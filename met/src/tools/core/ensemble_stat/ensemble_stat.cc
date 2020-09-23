// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   ensemble_stat.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    02/01/10  Halley Gotway   New
//   001    09/09/11  Halley Gotway   Call set_grid after reading
//                    gridded observation files.
//   002    10/13/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//   003    10/18/11  Halley Gotway   When the user requests verification
//                    in the config file, check that obs files have been
//                    specified on the command line.
//   004    11/14/11  Holmes          Added code to enable reading of
//                                    multiple config files.
//   005    12/09/11  Halley Gotway   When gridded observations are
//          missing, print a warning and continue rather than exiting
//          with error status.
//   006    05/08/12  Halley Gotway   Switch to using vx_config library.
//   007    05/08/12  Halley Gotway   Move -ens_valid, -ens_lead,
//                    and -obs_lead command line options to config file.
//   008    05/18/12  Halley Gotway   Modify logic to better handle
//                    missing files, fields, and data values.
//   009    08/30/12  Oldenburg       Add spread/skill functionality
//   010    06/03/14  Halley Gotway   Add PHIST line type
//   011    07/09/14  Halley Gotway   Add station id exclusion option.
//   012    11/12/14  Halley Gotway  Pass the obtype entry from the
//                    from the config file to the output files.
//   013    03/02/15  Halley Gotway  Add automated regridding.
//   014    09/11/15  Halley Gotway  Add climatology.
//   015    05/10/16  Halley Gotway  Add grid weighting.
//   016    05/20/16  Prestopnik J   Removed -version (now in command_line.cc)
//   017    08/09/16  Halley Gotway  Fixed n_ens_vld vs n_vx_vld bug.
//   018    05/15/17  Prestonik P    Add shape for regrid and interp.
//   019    05/15/17  Halley Gotway  Add RELP line type and skip_const.
//   020    02/21/18  Halley Gotway  Restructure config logic to make
//                    all options settable for each verification task.
//   021    03/21/18  Halley Gotway  Add obs_error perturbation.
//   022    04/05/18  Halley Gotway  Replace -ssvar_mean with -ens_mean.
//   023    08/15/18  Halley Gotway  Add mask.llpnt type.
//   024    04/01/19  Fillmore       Add FCST and OBS units.
//   025    04/15/19  Halley Gotway  Add percentile thresholds.
//   026    11/26/19  Halley Gotway  Add neighborhood probabilities.
//   027    12/11/19  Halley Gotway  Reorganize logic to support the use
//                    of python embedding.
//   028    12/17/19  Halley Gotway  Apply climatology bins to ensemble
//                    continuous statistics.
//   029    01/21/20  Halley Gotway  Add RPS output line type.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ensemble_stat.h"

#include "vx_nc_util.h"
#include "vx_data2d_nc_met.h"
#include "vx_regrid.h"
#include "vx_log.h"

#include "nc_obs_util.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line  (int, char **);
static void process_grid          (const Grid &);
static void process_n_vld         ();
static void process_ensemble      ();
static void process_vx            ();
static bool get_data_plane        (const char *, GrdFileType, VarInfo *,
                                   DataPlane &, bool do_regrid);
static bool get_data_plane_array  (const char *, GrdFileType, VarInfo *,
                                   DataPlaneArray &, bool do_regrid);

static void process_point_vx      ();
static void process_point_climo   ();
static void process_point_obs     (int);
static int  process_point_ens     (int, int &);
static void process_point_scores  ();

static void process_grid_vx       ();
static void process_grid_scores   (int,
               const DataPlane *, const DataPlane *,
               const DataPlane &, const DataPlane &,
               const DataPlane &, const DataPlane &,
               const DataPlane &, const MaskPlane &,
               ObsErrorEntry *,   PairDataEnsemble &);

static void do_ecnt              (const EnsembleStatVxOpt &,
                                  const SingleThresh &,
                                  const PairDataEnsemble *);
static void do_rps               (const EnsembleStatVxOpt &,
                                  const SingleThresh &,
                                  const PairDataEnsemble *);

static void clear_counts();
static void track_counts(int, const DataPlane &);

static ConcatString get_ens_mn_var_name(int);

static void setup_nc_file   (unixtime, const char *);
static void setup_txt_files ();
static void setup_table     (AsciiTable &);

static void build_outfile_name(unixtime, const char *,
                               ConcatString &);
static void write_ens_nc(int, DataPlane &);
static void write_ens_var_float(int, float *, DataPlane &,
                                const char *, const char *);
static void write_ens_var_int(int, int *, DataPlane &,
                              const char *, const char *);

static void write_orank_nc(PairDataEnsemble &, DataPlane &, int, int, int);
static void write_orank_var_float(int, int, int, float *, DataPlane &,
                                  const char *, const char *);
static void write_orank_var_int(int, int, int, int *, DataPlane &,
                                const char *, const char *);

static void add_var_att_local(VarInfo *, NcVar *, bool is_int, DataPlane &,
                              const char *, const char *);

static void finish_txt_files();
static void clean_up();

static void usage();

static void set_grid_obs(const StringArray &);
static void set_point_obs(const StringArray &);
static void set_ens_mean(const StringArray & a);
static void set_obs_valid_beg(const StringArray &);
static void set_obs_valid_end(const StringArray &);
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Check for valid ensemble data
   process_n_vld();

   // Process the ensemble fields
   process_ensemble();

   // Only perform verification if requested
   if(vx_flag) process_vx();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;
   CommandLine cline;
   ConcatString default_config_file;
   Met2dDataFile *ens_mtddf = (Met2dDataFile *) 0;
   Met2dDataFile *obs_mtddf = (Met2dDataFile *) 0;

   // Set default output directory
   out_dir = ".";

   //
   // check for zero arguments
   //
   if(argc == 1) usage();

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_grid_obs,      "-grid_obs", 1);
   cline.add(set_point_obs,     "-point_obs", 1);
   cline.add(set_ens_mean,      "-ens_mean", 1);
   cline.add(set_obs_valid_beg, "-obs_valid_beg", 1);
   cline.add(set_obs_valid_end, "-obs_valid_end", 1);
   cline.add(set_outdir,        "-outdir", 1);
   cline.add(set_logfile,       "-log", 1);
   cline.add(set_verbosity,     "-v", 1);
   cline.add(set_compress,      "-compress",  1);

   //
   // quietly support deprecated -ssvar_mean option
   //
   cline.add(set_ens_mean,      "-ssvar_mean", 1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be either a number followed by that
   // number of filenames or a single filename and a config filename.
   //
   if(cline.n() < 2) usage();

   if(cline.n() == 2) {

      //
      // It should be a filename and then a config filename
      //
      if(is_integer(cline[0].c_str()) == 0) {
         ens_file_list = parse_ascii_file_list(cline[0].c_str());
         n_ens = ens_file_list.n();
      }
      else {
         usage();
      }
   }
   else {

      //
      // More than two arguments. Check that the first is an integer
      // followed by that number of filenames and a config filename.
      //
      if(is_integer(cline[0].c_str()) == 1) {

         n_ens = atoi(cline[0].c_str());

         if(n_ens <= 0) {
            mlog << Error << "\nprocess_command_line() -> "
                 << "the number of ensemble member files must be >= 1 ("
                 << n_ens << ")\n\n";
            exit(1);
         }

         if((cline.n() - 2) == n_ens) {

            //
            // Add each of the ensemble members to the list of files.
            //
            for(i=1; i<=n_ens; i++) ens_file_list.add(cline[i]);
         }
         else {
            usage();
         }
      }
   }

   // Check for at least one valid input ensemble file
   if(ens_file_list.n() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "no valid input ensemble member files specified!\n\n";
      exit(1);
   }

   // Check that the end_ut >= beg_ut
   if(obs_valid_beg_ut != (unixtime) 0 &&
      obs_valid_end_ut != (unixtime) 0 &&
      obs_valid_beg_ut > obs_valid_end_ut) {

      mlog << Error << "\nprocess_command_line() -> "
           << "the ending time ("
           << unix_to_yyyymmdd_hhmmss(obs_valid_end_ut)
           << ") must be greater than the beginning time ("
           << unix_to_yyyymmdd_hhmmss(obs_valid_beg_ut)
           << ").\n\n";
      exit(1);
   }

   // Store the config file name
   config_file = cline[cline.n() - 1];

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file, config_file);

   // Get the ensemble file type from config, if present
   etype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_ens));

   // Read the first input ensemble file
   if(!(ens_mtddf = mtddf_factory.new_met_2d_data_file(ens_file_list[0].c_str(), etype))) {
      mlog << Error << "\nprocess_command_line() -> "
           << "trouble reading ensemble file \""
           << ens_file_list[0] << "\"\n\n";
      exit(1);
   }

   // Store the input ensemble file type
   etype = ens_mtddf->file_type();

   // Use a variable index from var_name instead of GRIB code
   bool use_var_id = false;

   // Determine the input observation file type
   if(point_obs_flag) {
      otype = FileType_Gb1;
      if(point_obs_file_list.n() > 0) {
         use_var_id = is_using_var_id(point_obs_file_list[0].c_str());
      }
   }
   else if(!grid_obs_flag) {
      otype = FileType_None;
   }
   else {

      // Get the observation file type from config, if present
      otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

      // Read the first gridded observation file
      if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(grid_obs_file_list[0].c_str(), otype))) {
         mlog << Error << "\nprocess_command_line() -> "
              << "trouble reading gridded observation file \""
              << grid_obs_file_list[0] << "\"\n\n";
         exit(1);
      }

      // Store the gridded observation file type
      otype = obs_mtddf->file_type();
   }

   // Process the configuration
   conf_info.process_config(etype, otype, grid_obs_flag, point_obs_flag, use_var_id);

   // Set the model name
   shc.set_model(conf_info.model.c_str());

   // Allocate arrays to store threshold counts
   thresh_count_na       = new NumArray   [conf_info.get_max_n_thresh()];
   thresh_nbrhd_count_na = new NumArray * [conf_info.get_max_n_thresh()];

   for(i=0; i<conf_info.get_max_n_thresh(); i++) {
      thresh_nbrhd_count_na[i] = new NumArray [conf_info.get_n_nbrhd()];
   }

   // List the input ensemble files
   mlog << Debug(1) << "Ensemble Files["
        << ens_file_list.n() << "]:\n";
   for(i=0; i<ens_file_list.n(); i++) {
      mlog << "   " << ens_file_list[i]  << "\n";
   }

   // List the input gridded observations files
   if(grid_obs_file_list.n() > 0) {
      mlog << Debug(1) << "Gridded Observation Files["
           << grid_obs_file_list.n() << "]:\n" ;
      for(i=0; i<grid_obs_file_list.n(); i++) {
         mlog << "   " << grid_obs_file_list[i] << "\n" ;
      }
   }

   // List the input point observations files
   if(point_obs_file_list.n() > 0) {
      mlog << Debug(1) << "Point Observation Files["
           << point_obs_file_list.n() << "]:\n" ;
      for(i=0; i<point_obs_file_list.n(); i++) {
         mlog << "   " << point_obs_file_list[i] << "\n" ;
      }
   }

   // Check for missing non-python ensemble files
   for(i=0; i<ens_file_list.n(); i++) {

      if(!file_exists(ens_file_list[i].c_str()) &&
         !is_python_grdfiletype(etype)) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "can't open input ensemble file: "
              << ens_file_list[i] << "\n\n";
         ens_file_vld.add(0);
      }
      else {
         ens_file_vld.add(1);
      }
   }

   // Set flag to indicate whether verification is to be performed
   if((point_obs_flag || grid_obs_flag) &&
      (conf_info.get_n_vx() > 0) &&
      (conf_info.output_flag[i_ecnt]  != STATOutputType_None ||
       conf_info.output_flag[i_rhist] != STATOutputType_None ||
       conf_info.output_flag[i_phist] != STATOutputType_None ||
       conf_info.output_flag[i_ssvar] != STATOutputType_None ||
       conf_info.output_flag[i_relp]  != STATOutputType_None ||
       conf_info.output_flag[i_orank] != STATOutputType_None)) vx_flag = 1;
   else                                                        vx_flag = 0;

   // Process ensemble mean information
   ens_mean_flag = false;
   bool need_ens_mean = (
      conf_info.output_flag[i_ecnt]  != STATOutputType_None ||
      conf_info.output_flag[i_orank] != STATOutputType_None ||
      conf_info.output_flag[i_ssvar] != STATOutputType_None);

   // User-specified ensemble mean file
   if(ens_mean_user.nonempty()) {

      if(!need_ens_mean) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "ignoring input -ens_mean file because no ensemble "
              << "mean is needed.\n\n";
      }
      else if(!file_exists(ens_mean_user.c_str())) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "can't open input ensemble mean file: "
              << ens_mean_user << "\n\n";
         ens_mean_user = "";
      }
      else if(!vx_flag) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "ignoring input -ens_mean file because no verification "
              << "has been requested\n\n";
      }
      else {
         ens_mean_flag = true;
      }
   }
   else if(need_ens_mean) {

      ens_mean_flag = true;

      if(!conf_info.nc_info.do_mean) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "enabling NetCDF ensemble mean computation to be used "
              << "in verificaiton.\n\n";
         conf_info.nc_info.do_mean = true;
      }
   }

   // Deallocate memory for data files
   if(ens_mtddf) { delete ens_mtddf; ens_mtddf = (Met2dDataFile *) 0; }
   if(obs_mtddf) { delete obs_mtddf; obs_mtddf = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid(const Grid &fcst_grid) {
   Grid obs_grid;

   // Parse regridding logic
   RegridInfo ri;
   if(conf_info.get_n_ens_var() > 0) {
      ri = conf_info.ens_info[0]->regrid();
   }
   else if(conf_info.get_n_vx() > 0) {
      ri = conf_info.vx_opt[0].vx_pd.fcst_info->regrid();
   }
   else {
      mlog << Error << "\nprocess_grid() -> "
           << "at least one ensemble field or verification field must "
           << "be provided!\n\n";
      exit(1);
   }

   // Read gridded observation data, if necessary
   if(ri.field == FieldType_Obs) {

      if(!grid_obs_flag) {
         mlog << Error << "\nprocess_grid() -> "
              << "attempting to regrid to the observation grid, but no "
              << "gridded observations provided!\n\n";
         exit(1);
      }

      DataPlane dp;
      Met2dDataFile *mtddf = (Met2dDataFile *) 0;
      if(!(mtddf = mtddf_factory.new_met_2d_data_file(
                                 grid_obs_file_list[0].c_str(), otype))) {
         mlog << Error << "\nprocess_grid() -> "
              << "trouble reading file \"" << grid_obs_file_list[0]
              << "\"\n\n";
         exit(1);
      }

      if(!mtddf->data_plane(*(conf_info.vx_opt[0].vx_pd.obs_info), dp)) {
         mlog << Error << "\nprocess_grid() -> "
              << "observation field \""
              << conf_info.vx_opt[0].vx_pd.obs_info->magic_str()
              << "\" not found in file \"" << grid_obs_file_list[0]
              << "\"\n\n";
         exit(1);
      }

      // Store the observation grid
      obs_grid = mtddf->grid();

      if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
   }
   else {
      obs_grid = fcst_grid;
   }

   // Determine the verification grid
   grid = parse_vx_grid(ri, &fcst_grid, &obs_grid);
   nxy  = grid.nx() * grid.ny();

   // Compute weight for each grid point
   parse_grid_weight(grid, conf_info.grid_weight_flag, wgt_dp);

   return;
}

////////////////////////////////////////////////////////////////////////

void process_n_vld() {
   int i, j, n_vld;
   DataPlane dp;
   DataPlaneArray dpa;

   // Initialize
   n_ens_vld.clear();
   n_vx_vld.clear();

   // Loop through the ensemble fields to be processed
   for(i=0; i<conf_info.get_n_ens_var(); i++) {

      // Loop through the ensemble files
      for(j=0, n_vld=0; j<ens_file_list.n(); j++) {

         // Check for valid file
         if(!ens_file_vld[j]) continue;

         // Check for valid data
         if(!get_data_plane(ens_file_list[j].c_str(), etype,
                            conf_info.ens_info[i], dp, false)) {
            mlog << Warning << "\nprocess_n_vld() -> "
                 << "ensemble field \""
                 << conf_info.ens_info[i]->magic_str()
                 << "\" not found in file \"" << ens_file_list[j]
                 << "\"\n\n";
         }
         else {

            // Increment the valid counter
            n_vld++;
         }
      } // end for j

      // Check for enough valid data
      if((double) n_vld/n_ens < conf_info.vld_ens_thresh) {
         mlog << Error << "\nprocess_n_vld() -> "
              << n_ens - n_vld << " missing ensemble fields exceeds the "
              << "maximum allowable specified by \"ens.ens_thresh\" "
              << "in the configuration file.\n\n";
         exit(1);
      }

      // Store the valid data count
      n_ens_vld.add(n_vld);

   } // end for i

   // Loop through the verification fields to be processed
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Loop through the ensemble files
      for(j=0, n_vld=0; j<ens_file_list.n(); j++) {

         // Check for valid file
         if(!ens_file_vld[j]) continue;

         // Check for valid data fields.
         // Call data_plane_array to handle multiple levels.
         if(!get_data_plane_array(ens_file_list[j].c_str(), etype,
                                  conf_info.vx_opt[i].vx_pd.fcst_info,
                                  dpa, false)) {
            mlog << Warning << "\nprocess_n_vld() -> "
                 << "no data found for forecast field \""
                 << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
                 << "\" in file \"" << ens_file_list[j]
                 << "\"\n\n";
         }
         else {

            // Store the lead times for the first verification field
            if(i==0) ens_lead_na.add(dpa[0].lead());

            // Increment the valid counter
            n_vld++;
         }
      } // end for j

      // Check for enough valid data
      if((double) n_vld/n_ens < conf_info.vld_ens_thresh) {
         mlog << Error << "\nprocess_n_vld() -> "
              << n_ens - n_vld << " missing forecast fields exceeds the "
              << "maximum allowable specified by \"ens.ens_thresh\" "
              << "in the configuration file.\n\n";
         exit(1);
      }

      // Store the valid data count
      n_vx_vld.add(n_vld);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

bool get_data_plane(const char *infile, GrdFileType ftype,
                    VarInfo *info, DataPlane &dp, bool do_regrid) {
   bool found;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // Read the current ensemble file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(infile, ftype))) {
      mlog << Error << "\nget_data_plane() -> "
           << "trouble reading file \"" << infile << "\"\n\n";
      exit(1);
   }

   // Read the gridded data field
   if((found = mtddf->data_plane(*info, dp))) {

      // Setup the verification grid, if necessary
      if(nxy == 0) process_grid(mtddf->grid());

      // Regrid, if requested and necessary
      if(do_regrid && !(mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding field \"" << info->magic_str()
              << "\" to the verification grid.\n";
         dp = met_regrid(dp, mtddf->grid(), grid, info->regrid());
      }

      // Store the valid time, if not already set
      if(ens_valid_ut == (unixtime) 0) {
         ens_valid_ut = dp.valid();
      }
      // Check to make sure that the valid time doesn't change
      else if(ens_valid_ut != dp.valid()) {
         mlog << Warning << "\nget_data_plane() -> "
              << "The valid time has changed, "
              << unix_to_yyyymmdd_hhmmss(ens_valid_ut)
              << " != " << unix_to_yyyymmdd_hhmmss(dp.valid())
              << " in \"" << infile << "\"\n\n";
      }

   } // end if found

   // Deallocate the data file pointer, if necessary
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(found);
}

////////////////////////////////////////////////////////////////////////

bool get_data_plane_array(const char *infile, GrdFileType ftype,
                          VarInfo *info, DataPlaneArray &dpa,
                          bool do_regrid) {
   int n, i;
   bool found;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // Read the current ensemble file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(infile, ftype))) {
      mlog << Error << "\nget_data_plane_array() -> "
           << "trouble reading file \"" << infile << "\"\n\n";
      exit(1);
   }

   // Read the gridded data field
   n = mtddf->data_plane_array(*info, dpa);

   // Check for at least one field
   if((found = (n > 0))) {

      // Setup the verification grid, if necessary
      if(nxy == 0) process_grid(mtddf->grid());

      // Regrid, if requested and necessary
      if(do_regrid && !(mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding " << dpa.n_planes()
              << " field(s) \"" << info->magic_str()
              << "\" to the verification grid.\n";

         // Loop through the forecast fields
         for(i=0; i<dpa.n_planes(); i++) {
            dpa[i] = met_regrid(dpa[i], mtddf->grid(), grid,
                                info->regrid());
         }
      }

      // Store the valid time, if not already set
      if(ens_valid_ut == (unixtime) 0) {
         ens_valid_ut = dpa[0].valid();
      }
      // Check to make sure that the valid time doesn't change
      else if(ens_valid_ut != dpa[0].valid()) {
         mlog << Warning << "\nget_data_plane_array() -> "
              << "The valid time has changed, "
              << unix_to_yyyymmdd_hhmmss(ens_valid_ut)
              << " != " << unix_to_yyyymmdd_hhmmss(dpa[0].valid())
              << " in \"" << infile << "\"\n\n";
      }

   } // end if found

   // Deallocate the data file pointer, if necessary
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return(found);
}

////////////////////////////////////////////////////////////////////////

void process_ensemble() {
   int i, j;
   bool reset;
   DataPlane ens_dp;
   unixtime max_init_ut = bad_data_ll;

   // Loop through each of the ensemble fields to be processed
   for(i=0; i<conf_info.get_n_ens_var(); i++) {

      mlog << Debug(2) << "\n" << sep_str << "\n\n"
           << "Processing ensemble field: "
           << conf_info.ens_info[i]->magic_str() << "\n";

      // Loop through each of the input forecast files
      for(j=0, reset=true; j<ens_file_list.n(); j++) {

         // Skip bad data files
         if(!ens_file_vld[j]) continue;

         // Read the current field
         if(!get_data_plane(ens_file_list[j].c_str(), etype,
                            conf_info.ens_info[i], ens_dp, true)) continue;

         // Create a NetCDF file to store the ensemble output
         if(nc_out == (NcFile *) 0) {
            setup_nc_file(ens_dp.valid(), "_ens.nc");
         }

         // Reset the running sums and counts
         if(reset) {
            clear_counts();
            reset = false;
         }

         // Apply current data to the running sums and counts
         track_counts(i, ens_dp);

         // Keep track of the maximum initialization time
         if(is_bad_data(max_init_ut) || ens_dp.init() > max_init_ut) {
            max_init_ut = ens_dp.init();
         }

      } // end for j

      // Write out the ensemble information to a NetCDF file
      ens_dp.set_init(max_init_ut);
      write_ens_nc(i, ens_dp);

      // Store the ensemble mean output file
      ens_mean_file =
         out_nc_file_list[out_nc_file_list.n() - 1];

   } // end for i

   // Close the output NetCDF file
   if(nc_out) {
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_vx() {

   if(conf_info.get_n_vx() > 0) {

      if(point_obs_file_list.n() == 0 &&
         grid_obs_file_list.n()  == 0) {
         mlog << Error << "\nprocess_vx() -> "
              << " when \"fcst.field\" is non-empty, you must use "
              << "\"-point_obs\" and/or \"-grid_obs\" to specify the "
              << "verifying observations.\n\n";
         exit(1);
      }

      // Process masks Grids and Polylines in the config file
      conf_info.process_masks(grid);

      // Setup the PairDataEnsemble objects
      conf_info.set_vx_pd(n_vx_vld);

      // Process the point observations
      if(point_obs_flag) process_point_vx();

      // Process the gridded observations
      if(grid_obs_flag) process_grid_vx();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_point_vx() {
   int i, n_miss;
   unixtime beg_ut, end_ut;

   // Set observation time window for each verification task
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // If obs_valid_beg_ut and obs_valid_end_ut were set on the command
      // line, use them.  If not, use beg_ds and end_ds.
      if(obs_valid_beg_ut != (unixtime) 0 ||
         obs_valid_end_ut != (unixtime) 0) {
         beg_ut = obs_valid_beg_ut;
         end_ut = obs_valid_end_ut;
      }
      else {
         beg_ut = ens_valid_ut + conf_info.vx_opt[i].beg_ds;
         end_ut = ens_valid_ut + conf_info.vx_opt[i].end_ds;
      }

      // Store the timing info
      conf_info.vx_opt[i].vx_pd.set_fcst_ut(ens_valid_ut);
      conf_info.vx_opt[i].vx_pd.set_beg_ut(beg_ut);
      conf_info.vx_opt[i].vx_pd.set_end_ut(end_ut);
   }

   // Process the climatology fields
   process_point_climo();

   // Process each point observation NetCDF file
   for(i=0; i<point_obs_file_list.n(); i++) {
      process_point_obs(i);
   }

   // Calculate and print observation summaries
   for(i=0; i<conf_info.get_n_vx(); i++) {
      conf_info.vx_opt[i].vx_pd.calc_obs_summary();
      conf_info.vx_opt[i].vx_pd.print_obs_summary();
   }

   // Process each ensemble file
   for(i=0, n_miss=0; i<ens_file_list.n(); i++) {

      // If the current forecast file is valid, process it
      if(!ens_file_vld[i]) {
         n_miss++;
         continue;
      }
      else {
         process_point_ens(i, n_miss);
      }

   } // end for i

   // Process the ensemble mean, if necessary
   if(ens_mean_flag) process_point_ens(-1, n_miss);

   // Compute the scores and write them out
   process_point_scores();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_point_climo() {
   int i;
   DataPlaneArray cmn_dpa, csd_dpa;

   // Loop through each of the fields to be verified and extract
   // the climatology fields for verification
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Read climatology data
      cmn_dpa = read_climo_data_plane_array(
                   conf_info.conf.lookup_array(conf_key_climo_mean_field, false),
                   i, ens_valid_ut, grid);
      csd_dpa = read_climo_data_plane_array(
                   conf_info.conf.lookup_array(conf_key_climo_stdev_field, false),
                   i, ens_valid_ut, grid);

      // Store climatology information
      conf_info.vx_opt[i].vx_pd.set_climo_mn_dpa(cmn_dpa);
      conf_info.vx_opt[i].vx_pd.set_climo_sd_dpa(csd_dpa);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void process_point_obs(int i_nc) {
   int i_obs, j;
   unixtime hdr_ut;
   NcFile *obs_in = (NcFile *) 0;

   mlog << Debug(2) << "\n" << sep_str << "\n\n"
        << "Processing point observation file: "
        << point_obs_file_list[i_nc] << "\n";

   // Open the observation file as a NetCDF file.
   obs_in = open_ncfile(point_obs_file_list[i_nc].c_str());

   if(IS_INVALID_NC_P(obs_in)) {
      delete obs_in;
      obs_in = (NcFile *) 0;

      mlog << Warning << "\nprocess_point_obs() -> "
           << "can't open observation netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      return;
   }

   // Read the dimensions and variables
   NetcdfObsVars obs_vars;
   read_nc_dims_vars(obs_vars, obs_in);

   bool use_var_id = obs_vars.use_var_id;
   if (use_var_id) {
      NcDim var_dim = get_nc_dim(obs_in,nc_dim_nvar);
      get_dim_size(&var_dim);
   }

   int exit_code = check_nc_dims_vars(obs_vars);

   if(exit_code == exit_code_no_dim) {
      mlog << Error << "\nprocess_point_obs() -> "
           << "can't read \"mxstr\", \"nobs\" or \"nmsg\" "
           << "dimensions from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      exit(1);
   }

   // Read the variables

   if(exit_code == exit_code_no_hdr_vars) {
      mlog << Error << "\nprocess_point_obs() -> "
           << "can't read \"hdr_typ\", \"hdr_sid\", "
           << "or \"hdr_vld\" variables from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      exit(1);
   }
   if(exit_code == exit_code_no_loc_vars) {
      mlog << Error << "\nprocess_point_obs() -> "
           << "can't read \"hdr_arr\", or \"hdr_lat\" variables from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      exit(1);
   }
   if(exit_code == exit_code_no_obs_vars) {
      mlog << Error << "\nprocess_point_obs() -> "
           << "can't read \"obs_arr\" or \"obs_val\" variables from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      exit(1);
   }

   if(IS_INVALID_NC(obs_vars.obs_qty_var))
      mlog << Debug(3) << "Quality marker information not found input file.\n";

   bool use_arr_vars = !IS_INVALID_NC(obs_vars.obs_arr_var);
   int hdr_count = GET_NC_SIZE(obs_vars.hdr_dim);
   int obs_count = GET_NC_SIZE(obs_vars.obs_dim);
   mlog << Debug(2) << "Searching " << (obs_count)
        << " observations from " << (hdr_count)
        << " header messages.\n";

   int qty_len = get_nc_string_length(obs_in, obs_vars.obs_qty_tbl_var,
                   (use_arr_vars ? nc_var_obs_qty : nc_var_obs_qty_tbl));

   NcHeaderData header_data = get_nc_hdr_data(obs_vars);

   int buf_size = ((obs_count > DEF_NC_BUFFER_SIZE) ? DEF_NC_BUFFER_SIZE : (obs_count));

   int   obs_qty_idx_block[buf_size];
   float obs_arr_block[buf_size][OBS_ARRAY_LEN];
   char  obs_qty_str_block[buf_size][qty_len];
   StringArray obs_qty_array;

   float obs_arr[OBS_ARRAY_LEN], hdr_arr[hdr_arr_len];
   int  hdr_typ_arr[hdr_typ_arr_len];
   ConcatString hdr_typ_str;
   ConcatString hdr_sid_str;
   ConcatString hdr_vld_str;
   ConcatString obs_qty_str;

   StringArray var_names;
   ConcatString var_name;

   if (use_var_id) {
      NcVar obs_var = get_nc_var(obs_in, nc_var_obs_var);
      if (!get_nc_data_to_array(&obs_var, &var_names)) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "trouble getting variable names from "
              << GET_NC_NAME(obs_var) << "\n\n";
         exit(1);
      }
   }

   if (!IS_INVALID_NC(obs_vars.obs_qty_tbl_var)) {
      if (!get_nc_data_to_array(&obs_vars.obs_qty_tbl_var, &obs_qty_array)) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "trouble getting obs_qty\n\n";
         exit(1);
      }
   }

   for(int i_start=0; i_start<obs_count; i_start+=buf_size) {
      buf_size = ((obs_count-i_start) > DEF_NC_BUFFER_SIZE) ? DEF_NC_BUFFER_SIZE : (obs_count-i_start);

      if (!read_nc_obs_data(obs_vars, buf_size, i_start, qty_len,
            (float *)obs_arr_block, obs_qty_idx_block, (char *)obs_qty_str_block)) {
         exit(1);
      }

      // Process each observation in the file
      for(int i_offset=0; i_offset<buf_size; i_offset++) {
         int hdr_idx;
         i_obs = i_start + i_offset;

         for (j=0; j<OBS_ARRAY_LEN; j++) {
            obs_arr[j] = obs_arr_block[i_offset][j];
         }

         if (use_arr_vars) {
            obs_qty_str = obs_qty_str_block[i_offset];
         }
         else {
            obs_qty_str = obs_qty_array[obs_qty_idx_block[i_offset]];
         }

         int headerOffset  = obs_arr[0];

         // Range check the header offset
         if(headerOffset < 0 || headerOffset >= hdr_count) {
            mlog << Warning << "\nprocess_point_obs() -> "
                 << "range check error for header index " << headerOffset
                 << " from observation number " << i_obs
                 << " of point observation file: "
                 << point_obs_file_list[i_nc] << "\n\n";
            continue;
         }

         // Read the corresponding header array for this observation
         hdr_arr[0] = header_data.lat_array[headerOffset];
         hdr_arr[1] = header_data.lon_array[headerOffset];
         hdr_arr[2] = header_data.elv_array[headerOffset];

         // Read the header integer types
         hdr_typ_arr[0] = (header_data.prpt_typ_array.n() > headerOffset ?
                           header_data.prpt_typ_array[headerOffset] : bad_data_int);
         hdr_typ_arr[1] = (header_data.irpt_typ_array.n() > headerOffset ?
                           header_data.irpt_typ_array[headerOffset] : bad_data_int);
         hdr_typ_arr[2] = (header_data.inst_typ_array.n() > headerOffset ?
                           header_data.inst_typ_array[headerOffset] : bad_data_int);

         // Read the corresponding header type for this observation
         hdr_idx = use_arr_vars ? headerOffset : header_data.typ_idx_array[headerOffset];
         hdr_typ_str = header_data.typ_array[hdr_idx];

         // Read the corresponding header Station ID for this observation
         hdr_idx = use_arr_vars ? headerOffset : header_data.sid_idx_array[headerOffset];
         hdr_sid_str = header_data.sid_array[hdr_idx];

         // Read the corresponding valid time for this observation
         hdr_idx = use_arr_vars ? headerOffset : header_data.vld_idx_array[headerOffset];
         hdr_vld_str = header_data.vld_array[hdr_idx];

         // Convert string to a unixtime
         hdr_ut = timestring_to_unix(hdr_vld_str.c_str());

         if (use_var_id && obs_arr[1] < var_names.n()) {
            var_name = var_names[obs_arr[1]];
         }
         else {
            var_name = "";
         }

         // Check each conf_info.vx_pd object to see if this observation
         // should be added
         for(j=0; j<conf_info.get_n_vx(); j++) {

            // Attempt to add the observation to the vx_pd object
            conf_info.vx_opt[j].vx_pd.add_point_obs(hdr_arr,
                                         hdr_typ_arr, hdr_typ_str.c_str(),
                                         hdr_sid_str.c_str(), hdr_ut,
                                         obs_qty_str.c_str(), obs_arr,
                                         grid, var_name.c_str());
         }
      }
   } // end for i_start

   // Deallocate and clean up
   if(obs_in) {
      delete obs_in;
      obs_in = (NcFile *) 0;
   }
   clear_header_data(&header_data);

   return;
}

////////////////////////////////////////////////////////////////////////

int process_point_ens(int i_ens, int &n_miss) {
   int i;
   DataPlaneArray fcst_dpa;
   NumArray fcst_lvl_na;
   VarInfo *info = (VarInfo *) 0;
   VarInfoNcMet ens_mean_info;

   ConcatString ens_file;
   bool is_ens_mean = (-1 == i_ens);
   const char *file_type = (is_ens_mean ? "mean" : "ensemble");

   // Determine the correct file to process
   if(!is_ens_mean) ens_file = ConcatString(ens_file_list[i_ens]);
   else             ens_file = (ens_mean_user.empty() ?
                                ens_mean_file : ens_mean_user);

   mlog << Debug(2) << "\n" << sep_str << "\n\n"
        << "Processing " << file_type << " file: " << ens_file << "\n";

   // Loop through each of the fields to be verified and extract
   // the forecast fields for verification
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Use the calculated mean file, if necessary
      if(is_ens_mean && ens_mean_user.empty()) {
         fcst_dpa.clear();
         ens_mean_info.clear();
         ens_mean_info.set_magic(get_ens_mn_var_name(i),
                                 (string)"(*,*)");
         info = &ens_mean_info;
      }
      else {
         info = conf_info.vx_opt[i].vx_pd.fcst_info;
      }

      // Read the gridded data from the input forecast file
      if(!get_data_plane_array(ens_file.c_str(), info->file_type(), info,
                               fcst_dpa, true)) {

         // Error out if unable to read the ensemble mean
         if(is_ens_mean) {
            mlog << Error << "\nprocess_point_ens() -> "
                 << "trouble reading the ensemble mean field \""
                 << info->magic_str() << "\" from file \""
                 << ens_file << "\"\n\n";
            exit(1);
         }

         n_miss++;
         continue;
      }

      // Dump out the number of levels found
      mlog << Debug(2) << "For " << info->magic_str()
           << " found " << fcst_dpa.n_planes() << " forecast levels.\n";

      // Store information for the raw forecast fields
      conf_info.vx_opt[i].vx_pd.set_fcst_dpa(fcst_dpa);

      // Compute forecast values for this ensemble member
      conf_info.vx_opt[i].vx_pd.add_ens(i_ens-n_miss, is_ens_mean);

   } // end for i

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_point_scores() {
   PairDataEnsemble *pd_ptr = (PairDataEnsemble *) 0;
   PairDataEnsemble pd;
   int i, j, k, l, m, n;

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Create output text files as requested in the config file
   setup_txt_files();

   // Store the forecast lead time
   shc.set_fcst_lead_sec(nint(ens_lead_na.min()));

   // Store the forecast valid time
   shc.set_fcst_valid_beg(ens_valid_ut);
   shc.set_fcst_valid_end(ens_valid_ut);

   // Loop through the Ensemble pair data objects, compute the scores
   // requested, and write the output.
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Set the description
      shc.set_desc(conf_info.vx_opt[i].vx_pd.desc.c_str());

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.vx_opt[i].vx_pd.fcst_info->name_attr());

      // Store the forecast variable units
      shc.set_fcst_units(conf_info.vx_opt[i].vx_pd.fcst_info->units_attr());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.vx_opt[i].vx_pd.fcst_info->level_attr().c_str());

      // Store the observation variable name
      shc.set_obs_var(conf_info.vx_opt[i].vx_pd.obs_info->name_attr());

      // Store the observation variable units
      shc.set_obs_units(conf_info.vx_opt[i].vx_pd.obs_info->units_attr());

      // Set the observation level name
      shc.set_obs_lev(conf_info.vx_opt[i].vx_pd.obs_info->level_attr().c_str());

      // Set the observation lead time
      shc.set_obs_lead_sec(0);

      // Set the observation valid time
      shc.set_obs_valid_beg(conf_info.vx_opt[i].vx_pd.beg_ut);
      shc.set_obs_valid_end(conf_info.vx_opt[i].vx_pd.end_ut);

      // Loop through the message types
      for(j=0; j<conf_info.vx_opt[i].get_n_msg_typ(); j++) {

         // Store the message type in the obtype column
         shc.set_obtype(conf_info.vx_opt[i].msg_typ[j].c_str());

         // Loop through the verification masking regions
         for(k=0; k<conf_info.vx_opt[i].get_n_mask(); k++) {

            // Store the verification masking region
            shc.set_mask(conf_info.vx_opt[i].mask_name[k].c_str());

            // Loop through the interpolation methods
            for(l=0; l<conf_info.vx_opt[i].get_n_interp(); l++) {

               // Store the interpolation method and width being applied
               shc.set_interp_mthd(conf_info.vx_opt[i].interp_info.method[l],
                                   conf_info.vx_opt[i].interp_info.shape);
               shc.set_interp_wdth(conf_info.vx_opt[i].interp_info.width[l]);

               pd_ptr = &conf_info.vx_opt[i].vx_pd.pd[j][k][l];

               mlog << Debug(2)
                    << "Processing point verification "
                    << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
                    << " versus "
                    << conf_info.vx_opt[i].vx_pd.obs_info->magic_str()
                    << ", for observation type " << pd_ptr->msg_typ
                    << ", over region " << pd_ptr->mask_name
                    << ", for interpolation method "
                    << shc.get_interp_mthd() << "("
                    << shc.get_interp_pnts_str()
                    << "), using " << pd_ptr->n_obs << " matched pairs.\n";

               // Continue if there are no points
               if(pd_ptr->n_obs == 0) continue;

               // Process percentile thresholds
               conf_info.vx_opt[i].set_perc_thresh(pd_ptr);

               // Compute observation ranks and pair values
               pd_ptr->compute_pair_vals(conf_info.rng_ptr);

               // Process each filtering threshold
               for(m=0; m<conf_info.vx_opt[i].othr_ta.n(); m++) {

                  // Set the header column
                  shc.set_obs_thresh(conf_info.vx_opt[i].othr_ta[m]);

                  // Subset pairs using the current obs_thresh
                  pd = pd_ptr->subset_pairs(conf_info.vx_opt[i].othr_ta[m]);

                  // Continue if there are no points
                  if(pd.n_obs == 0) continue;

                  // Compute ECNT scores
                  if(conf_info.output_flag[i_ecnt] != STATOutputType_None) {
                     do_ecnt(conf_info.vx_opt[i],
                             conf_info.vx_opt[i].othr_ta[m], &pd);
                  }

                  // Compute RPS scores
                  if(conf_info.output_flag[i_rps] != STATOutputType_None) {
                     do_rps(conf_info.vx_opt[i],
                            conf_info.vx_opt[i].othr_ta[m], &pd);
                  }

                  // Write RHIST counts
                  if(conf_info.output_flag[i_rhist] != STATOutputType_None) {

                     pd.compute_rhist();

                     if(pd.rhist_na.sum() > 0) {
                        write_rhist_row(shc, &pd,
                           conf_info.output_flag[i_rhist],
                           stat_at, i_stat_row,
                           txt_at[i_rhist], i_txt_row[i_rhist]);
                     }
                  }

                  // Write PHIST counts if greater than 0
                  if(conf_info.output_flag[i_phist] != STATOutputType_None) {

                     pd.compute_phist();

                     if(pd.phist_na.sum() > 0) {
                        write_phist_row(shc, &pd,
                           conf_info.output_flag[i_phist],
                           stat_at, i_stat_row,
                           txt_at[i_phist], i_txt_row[i_phist]);
                     }
                  }

                  // Write RELP counts
                  if(conf_info.output_flag[i_relp] != STATOutputType_None) {

                     pd.compute_relp();

                     if(pd.relp_na.sum() > 0) {
                        write_relp_row(shc, &pd,
                           conf_info.output_flag[i_relp],
                           stat_at, i_stat_row,
                           txt_at[i_relp], i_txt_row[i_relp]);
                     }
                  }

                  // Write SSVAR scores
                  if(conf_info.output_flag[i_ssvar] != STATOutputType_None) {

                     pd.compute_ssvar();

                     // Make sure there are bins to process
                     if(pd.ssvar_bins) {

                        // Add rows to the output AsciiTables for SSVAR
                        stat_at.add_rows(pd.ssvar_bins[0].n_bin *
                                         conf_info.vx_opt[i].ci_alpha.n());

                        if(conf_info.output_flag[i_ssvar] == STATOutputType_Both) {
                           txt_at[i_ssvar].add_rows(pd.ssvar_bins[0].n_bin *
                                                    conf_info.vx_opt[i].ci_alpha.n());
                        }

                        // Write the SSVAR data for each alpha value
                        for(n=0; n<conf_info.vx_opt[i].ci_alpha.n(); n++) {
                           write_ssvar_row(shc, &pd, conf_info.vx_opt[i].ci_alpha[n],
                              conf_info.output_flag[i_ssvar],
                              stat_at, i_stat_row,
                              txt_at[i_ssvar], i_txt_row[i_ssvar]);
                        }
                     }
                  }

               } // end for m

               // Write out the unfiltered ORANK lines
               if(conf_info.output_flag[i_orank] != STATOutputType_None) {

                  // Set the header column
                  shc.set_obs_thresh(na_str);

                  write_orank_row(shc, pd_ptr,
                     conf_info.output_flag[i_orank],
                     stat_at, i_stat_row,
                     txt_at[i_orank], i_txt_row[i_orank]);

                  // Reset the observation valid time
                  shc.set_obs_valid_beg(conf_info.vx_opt[i].vx_pd.beg_ut);
                  shc.set_obs_valid_end(conf_info.vx_opt[i].vx_pd.end_ut);
               }

            } // end for l
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid_vx() {
   int i, j, k, l, m, n_miss;
   bool found;
   MaskPlane  mask_mp;
   DataPlane *fcst_dp = (DataPlane *) 0;
   DataPlane *fraw_dp = (DataPlane *) 0;
   DataPlane  obs_dp, oraw_dp;
   DataPlane emn_dp, cmn_dp, csd_dp;
   PairDataEnsemble pd_all, pd;
   ObsErrorEntry *oerr_ptr = (ObsErrorEntry *) 0;

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Set the obtype column
   shc.set_obtype(conf_info.obtype.c_str());

   // Allocate space to store the forecast fields
   fcst_dp = new DataPlane [n_ens];
   fraw_dp = new DataPlane [n_ens];

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Set the forecast lead time
      shc.set_fcst_lead_sec(nint(ens_lead_na.min()));

      // Set the forecast valid time
      shc.set_fcst_valid_beg(ens_valid_ut);
      shc.set_fcst_valid_end(ens_valid_ut);

      // Set the description
      shc.set_desc(conf_info.vx_opt[i].vx_pd.desc.c_str());

      // Set the forecast variable name
      shc.set_fcst_var(conf_info.vx_opt[i].vx_pd.fcst_info->name_attr());

      // Store the forecast variable units
      shc.set_fcst_units(conf_info.vx_opt[i].vx_pd.fcst_info->units_attr());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.vx_opt[i].vx_pd.fcst_info->level_attr().c_str());

      // Set the ObsErrorEntry pointer
      if(conf_info.vx_opt[i].obs_error.flag) {

         // Use config file setting, if specified
         if(conf_info.vx_opt[i].obs_error.entry.dist_type != DistType_None) {
            mlog << Debug(3)
                 << "Observation error for gridded verification is "
                 << "defined in the configuration file.\n";
            oerr_ptr = &(conf_info.vx_opt[i].obs_error.entry);
         }
         // Otherwise, do a table lookup
         else {

            // Check for table entries for this variable and message type
            if(!obs_error_table.has(
                  conf_info.vx_opt[i].vx_pd.obs_info->name().c_str(),
                  conf_info.obtype.c_str())) {
               mlog << Warning << "\nprocess_grid_vx() -> "
                    << "Disabling observation error logic since the "
                    << "obs error table contains no entry for OBS_VAR("
                    << conf_info.vx_opt[i].vx_pd.obs_info->name()
                    << ") and MESSAGE_TYPE(" << conf_info.obtype
                    << ").\nSpecify a custom obs error table using the "
                    << "MET_OBS_ERROR_TABLE environment variable.\n\n";
               conf_info.vx_opt[i].obs_error.flag = false;
            }
            else {

               // Do a lookup for this variable and message type
               oerr_ptr = obs_error_table.lookup(
                  conf_info.vx_opt[i].vx_pd.obs_info->name().c_str(),
                  conf_info.obtype.c_str());

               // If match was found and includes a value range setting,
               // reset to NULL and lookup separately for grid point
               if(oerr_ptr) {
                  if(oerr_ptr->val_range.n() == 0) {
                     mlog << Debug(3)
                          << "Observation error for gridded verification is "
                          << "defined by a single table entry.\n";
                  }
                  else {
                     mlog << Debug(3)
                          << "Observation error for gridded verification is "
                          << "defined by a table lookup for each point.\n";
                     oerr_ptr = (ObsErrorEntry *) 0;
                  }
               }
            }
         }
      }

      // Loop through each of the input ensemble files
      for(j=0, n_miss=0; j<ens_file_list.n(); j++) {

         // Initialize
         fcst_dp[j].clear();

         // If the current ensemble file is valid, read the field
         if(ens_file_vld[j]) {
            found = get_data_plane(ens_file_list[j].c_str(), etype,
                                   conf_info.vx_opt[i].vx_pd.fcst_info,
                                   fcst_dp[j], true);
         }
         else {
            found = false;
         }

         // Count the number of missing files
         if(!found) {
            n_miss++;
            continue;
         }
      } // end for j

      // Read climatology data
      cmn_dp = read_climo_data_plane(
                  conf_info.conf.lookup_array(conf_key_climo_mean_field, false),
                  i, ens_valid_ut, grid);
      csd_dp = read_climo_data_plane(
                  conf_info.conf.lookup_array(conf_key_climo_stdev_field, false),
                  i, ens_valid_ut, grid);

      mlog << Debug(3)
           << "Found " << (cmn_dp.nx() == 0 ? 0 : 1)
           << " climatology mean field(s) and " << (csd_dp.nx() == 0 ? 0 : 1)
           << " climatology standard deviation field(s) for forecast "
           << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str() << ".\n";

      // If requested in the config file, create a NetCDF file to store
      // the verification matched pairs
      if(conf_info.nc_info.do_orank &&
         nc_out == (NcFile *) 0)
         setup_nc_file(fcst_dp[j].valid(), "_orank.nc");

      // Read the observation file
      for(j=0, n_miss=0; j<grid_obs_file_list.n(); j++) {

         found = get_data_plane(grid_obs_file_list[j].c_str(), otype,
                                conf_info.vx_opt[i].vx_pd.obs_info,
                                obs_dp, true);

         // If found, break out of the loop
         if(!found) n_miss++;
         else {
            mlog << Debug(2)
                 << "Reading gridded observation for "
                 << conf_info.vx_opt[i].vx_pd.obs_info->magic_str()
                 << " from file: " << grid_obs_file_list[j] << "\n";
            break;
         }
      }

      // Check if the observation field was found
      if(n_miss == grid_obs_file_list.n()) {
         mlog << Warning << "\nprocess_grid_vx() -> "
              << conf_info.vx_opt[i].vx_pd.obs_info->magic_str()
              << " not found in observation files.\n\n";
         continue;
      }

      // Set the observation variable name
      shc.set_obs_var(conf_info.vx_opt[i].vx_pd.obs_info->name_attr());

      // Store the observation variable units
      shc.set_obs_units(conf_info.vx_opt[i].vx_pd.obs_info->units_attr());

      // Set the observation level name
      shc.set_obs_lev(conf_info.vx_opt[i].vx_pd.obs_info->level_attr().c_str());

      // Set the observation lead time
      shc.set_obs_lead_sec(obs_dp.lead());

      // Set the observation valid time
      shc.set_obs_valid_beg(obs_dp.valid());
      shc.set_obs_valid_end(obs_dp.valid());

      // Process the ensemble mean, if necessary
      if(ens_mean_flag) {
         VarInfo *info = (VarInfo *) 0;
         VarInfoNcMet ens_mean_info;
         ConcatString mn_file = (ens_mean_user.empty() ?
                                 ens_mean_file : ens_mean_user);

         mlog << Debug(2) << "\n" << sep_str << "\n\n"
              << "Processing ensemble mean file: " << mn_file << "\n";

         // Use the calculated mean file, in necessary
         if(ens_mean_user.empty()) {
            ens_mean_info.set_magic(get_ens_mn_var_name(i),
                                    (string)"(*,*)");
            info = &ens_mean_info;
         }
         else {
            info = conf_info.vx_opt[i].vx_pd.fcst_info;
         }

         // Read the gridded data from the mean file
         found = get_data_plane(mn_file.c_str(), FileType_None,
                                info, emn_dp, true);

         if(!found) {
            mlog << Error << "\nprocess_grid_vx() -> "
                 << "trouble reading ensemble mean field \""
                 << info->magic_str() << "\" from file \""
                 << mn_file << "\"\n\n";
            exit(1);
         }
      }

      // Loop through and apply each of the smoothing operations
      for(j=0; j<conf_info.vx_opt[i].get_n_interp(); j++) {

         // Store current settings
         ConcatString mthd_str   = conf_info.vx_opt[i].interp_info.method[j];
         InterpMthd   mthd       = string_to_interpmthd(mthd_str.c_str());
         int          wdth       = conf_info.vx_opt[i].interp_info.width[j];
         GaussianInfo gaussian   = conf_info.vx_opt[i].interp_info.gaussian;
         double       vld_thresh = conf_info.vx_opt[i].interp_info.vld_thresh;
         GridTemplateFactory::GridTemplates shape = conf_info.vx_opt[i].interp_info.shape;
         FieldType    field      = conf_info.vx_opt[i].interp_info.field;

         // Check for allowable smoothing operation
         if(mthd == InterpMthd_DW_Mean ||
            mthd == InterpMthd_LS_Fit  ||
            mthd == InterpMthd_Bilin   ||
            mthd == InterpMthd_Nbrhd) {

            mlog << Warning << "\nprocess_grid_vx() -> "
                 << mthd_str << " smoothing option not supported for "
                 << "gridded observations.\n\n";
            continue;
         }

         // Set the interpolation method and width
         shc.set_interp_mthd(mthd, shape);
         shc.set_interp_wdth(wdth);

         // Smooth the ensemble mean field, if requested
         if(ens_mean_flag &&
            (field == FieldType_Fcst || field == FieldType_Both)) {
            emn_dp = smooth_field(emn_dp, mthd, wdth, shape,
                                  vld_thresh, gaussian);
         }

         // Smooth the observation field, if requested
         if(field == FieldType_Obs || field == FieldType_Both) {
            obs_dp = smooth_field(obs_dp, mthd, wdth, shape,
                                  vld_thresh, gaussian);
         }

         // Store a copy of the unperturbed observation field
         oraw_dp = obs_dp;

         // Apply observation error bias correciton, if requested
         if(conf_info.vx_opt[i].obs_error.flag) {
            mlog << Debug(3)
                 << "Applying observation error bias correction to "
                 << "gridded observation data.\n";
            obs_dp = add_obs_error_bc(conf_info.rng_ptr,
                        FieldType_Obs, oerr_ptr, oraw_dp, oraw_dp,
                        conf_info.vx_opt[i].vx_pd.obs_info->name().c_str(),
                        conf_info.obtype.c_str());
         }

         // Looop through the ensemble members
         for(k=0; k<ens_file_list.n(); k++) {

            // Smooth the forecast field, if requested
            if(field == FieldType_Fcst || field == FieldType_Both) {
               fcst_dp[k] = smooth_field(fcst_dp[k], mthd, wdth, shape,
                                         vld_thresh, gaussian);
            }

            // Store a copy of the unperturbed ensemble field
            fraw_dp[k] = fcst_dp[k];

            // Apply observation error perturbation, if requested
            // and data is present for this ensemble member
            if(conf_info.vx_opt[i].obs_error.flag &&
               !fraw_dp[k].is_empty()) {
               mlog << Debug(3)
                    << "Applying observation error perturbation to "
                    << "ensemble member " << k+1 << ".\n";
               fcst_dp[k] = add_obs_error_inc(conf_info.rng_ptr,
                               FieldType_Fcst, oerr_ptr, fraw_dp[k], oraw_dp,
                               conf_info.vx_opt[i].vx_pd.obs_info->name().c_str(),
                               conf_info.obtype.c_str());
            }
         } // end for k

         // Loop through the masks to be applied
         for(k=0; k<conf_info.vx_opt[i].get_n_mask_area(); k++) {

            // Set the mask name
            shc.set_mask(conf_info.vx_opt[i].mask_name_area[k].c_str());

            // Store the current mask
            mask_mp = conf_info.mask_area_map[conf_info.vx_opt[i].mask_name_area[k]];

            // Initialize
            pd_all.clear();
            pd_all.set_ens_size(n_vx_vld[i]);
            pd_all.skip_const = conf_info.vx_opt[i].vx_pd.pd[0][0][0].skip_const;

            // Apply the current mask to the fields and compute the pairs
            process_grid_scores(i,
                                fcst_dp, fraw_dp,
                                obs_dp, oraw_dp,
                                emn_dp, cmn_dp, csd_dp,
                                mask_mp, oerr_ptr,
                                pd_all);

            mlog << Debug(2)
                 << "Processing gridded verification "
                 << conf_info.vx_opt[i].vx_pd.fcst_info->magic_str()
                 << " versus "
                 << conf_info.vx_opt[i].vx_pd.obs_info->magic_str()
                 << ", for observation type " << shc.get_obtype()
                 << ", over region " << shc.get_mask()
                 << ", for interpolation method "
                 << shc.get_interp_mthd() << "("
                 << shc.get_interp_pnts_str()
                 << "), using " << pd_all.n_obs << " matched pairs.\n";

            // Continue if there are no points
            if(pd_all.n_obs == 0) continue;

            // Process percentile thresholds
            conf_info.vx_opt[i].set_perc_thresh(&pd_all);

            // Compute observation ranks and pair values
            pd_all.compute_pair_vals(conf_info.rng_ptr);

            // Write out the unfiltered observation rank field.
            if(conf_info.nc_info.do_orank) {
               write_orank_nc(pd_all, obs_dp, i, j, k);
            }

            // Process each filtering threshold
            for(l=0; l<conf_info.vx_opt[i].othr_ta.n(); l++) {

               // Set the header column
               shc.set_obs_thresh(conf_info.vx_opt[i].othr_ta[l]);

               // Subset pairs using the current obs_thresh
               pd = pd_all.subset_pairs(conf_info.vx_opt[i].othr_ta[l]);

               // Continue if there are no points
               if(pd.n_obs == 0) continue;

               // Create output text files as requested in the config file
               setup_txt_files();

               // Compute ECNT scores
               if(conf_info.output_flag[i_ecnt] != STATOutputType_None) {
                  do_ecnt(conf_info.vx_opt[i],
                          conf_info.vx_opt[i].othr_ta[l], &pd);
               }

               // Compute RPS scores
               if(conf_info.output_flag[i_rps] != STATOutputType_None) {
                  do_rps(conf_info.vx_opt[i],
                         conf_info.vx_opt[i].othr_ta[l], &pd);
               }

               // Write RHIST counts
               if(conf_info.output_flag[i_rhist] != STATOutputType_None) {

                  pd.compute_rhist();

                  if(pd.rhist_na.sum() > 0) {
                     write_rhist_row(shc, &pd,
                        conf_info.output_flag[i_rhist],
                        stat_at, i_stat_row,
                        txt_at[i_rhist], i_txt_row[i_rhist]);
                  }
               }

               // Write PHIST counts if greater than 0
               if(conf_info.output_flag[i_phist] != STATOutputType_None) {

                  pd.phist_bin_size = conf_info.vx_opt[i].vx_pd.pd[0][0][0].phist_bin_size;
                  pd.compute_phist();

                  if(pd.phist_na.sum() > 0) {
                     write_phist_row(shc, &pd,
                        conf_info.output_flag[i_phist],
                        stat_at, i_stat_row,
                        txt_at[i_phist], i_txt_row[i_phist]);
                  }
               }

               // Write RELP counts
               if(conf_info.output_flag[i_relp] != STATOutputType_None) {

                  pd.compute_relp();

                  if(pd.relp_na.sum() > 0) {
                     write_relp_row(shc, &pd,
                        conf_info.output_flag[i_relp],
                        stat_at, i_stat_row,
                        txt_at[i_relp], i_txt_row[i_relp]);
                  }
               }

               // Write SSVAR scores
               if(conf_info.output_flag[i_ssvar] != STATOutputType_None) {

                  pd.ssvar_bin_size = conf_info.vx_opt[i].vx_pd.pd[0][0][0].ssvar_bin_size;
                  pd.compute_ssvar();

                  // Make sure there are bins to process
                  if(pd.ssvar_bins) {

                     // Add rows to the output AsciiTables for SSVAR
                     stat_at.add_rows(pd.ssvar_bins[0].n_bin *
                                      conf_info.vx_opt[i].ci_alpha.n());

                     if(conf_info.output_flag[i_ssvar] == STATOutputType_Both) {
                        txt_at[i_ssvar].add_rows(pd.ssvar_bins[0].n_bin *
                                                 conf_info.vx_opt[i].ci_alpha.n());
                     }

                     // Write the SSVAR data for each alpha value
                     for(m=0; m<conf_info.vx_opt[i].ci_alpha.n(); m++) {
                        write_ssvar_row(shc, &pd, conf_info.vx_opt[i].ci_alpha[m],
                           conf_info.output_flag[i_ssvar],
                           stat_at, i_stat_row,
                           txt_at[i_ssvar], i_txt_row[i_ssvar]);
                     }
                  }
               }

            } // end for l

         } // end for k
      } // end for j
   } // end for i

   // Delete allocated DataPlane objects
   if(fcst_dp) { delete [] fcst_dp; fcst_dp = (DataPlane *) 0; }
   if(fraw_dp) { delete [] fraw_dp; fraw_dp = (DataPlane *) 0; }

   // Close the output NetCDF file
   if(nc_out) {
      delete nc_out; nc_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid_scores(int i_vx,
        const DataPlane *fcst_dp, const DataPlane *fraw_dp,
        const DataPlane &obs_dp,  const DataPlane &oraw_dp,
        const DataPlane &emn_dp,  const DataPlane &cmn_dp,
        const DataPlane &csd_dp,  const MaskPlane &mask_mp,
        ObsErrorEntry *oerr_ptr,  PairDataEnsemble &pd) {
   int i, j, x, y, n_miss;
   double cmn, csd;
   ObsErrorEntry *e = (ObsErrorEntry *) 0;

   // Allocate memory in one big chunk based on grid size
   pd.extend(grid.nx()*grid.ny());

   // Climatology flags
   bool emn_flag = (emn_dp.nx() == obs_dp.nx() &&
                    emn_dp.ny() == obs_dp.ny());
   bool cmn_flag = (cmn_dp.nx() == obs_dp.nx() &&
                    cmn_dp.ny() == obs_dp.ny());
   bool csd_flag = (csd_dp.nx() == obs_dp.nx() &&
                    csd_dp.ny() == obs_dp.ny());

   // Loop through the observation field
   for(x=0; x<obs_dp.nx(); x++) {
      for(y=0; y<obs_dp.ny(); y++) {

         // Skip any grid points containing bad data values or where the
         // verification masking region is turned off
         if(is_bad_data(obs_dp(x, y)) ||
            !mask_mp.s_is_on(x, y)) continue;

         // Get current climatology values
         cmn = (cmn_flag ? cmn_dp(x, y) : bad_data_double);
         csd = (csd_flag ? csd_dp(x, y) : bad_data_double);

         // Add the observation point
         pd.add_grid_obs(x, y, oraw_dp(x, y), cmn, csd, wgt_dp(x, y));

         // Get the observation error entry pointer
         if(oerr_ptr) {
            e = oerr_ptr;
         }
         else if(conf_info.vx_opt[i_vx].obs_error.flag) {
            e = obs_error_table.lookup(
                   conf_info.vx_opt[i_vx].vx_pd.obs_info->name().c_str(),
                   conf_info.obtype.c_str(), oraw_dp(x,y));
         }
         else {
            e = (ObsErrorEntry *) 0;
         }

         // Store the observation error entry pointer
         pd.add_obs_error_entry(e);

         // Add the ensemble mean value for this point
         pd.mn_na.add((emn_flag ? emn_dp(x, y) : bad_data_double));

      } // end for y
   } // end for x

   // Loop through the observation points
   for(i=0; i<pd.n_obs; i++) {

      x = nint(pd.x_na[i]);
      y = nint(pd.y_na[i]);

      // Loop through each of the ensemble members
      for(j=0, n_miss=0; j<n_ens; j++) {

         // Skip missing data
         if(fcst_dp[j].nx() == 0 || fcst_dp[j].ny() == 0) {
            n_miss++;
            continue;
         }
         else {
            // Store the ensemble value
            pd.add_ens(j-n_miss, fcst_dp[j](x, y));

            // Store the unperturbed ensemble value
            pd.add_ens_var_sums(i, fraw_dp[j](x, y));
         }
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_ecnt(const EnsembleStatVxOpt &vx_opt,
             const SingleThresh &othresh,
             const PairDataEnsemble *pd_ptr) {
   int i, n_bin;
   PairDataEnsemble pd;
   ECNTInfo *ecnt_info = (ECNTInfo *) 0;

   // Check for valid pointer
   if(!pd_ptr) return;

   // Determine the number of climo CDF bins
   n_bin = (pd_ptr->cmn_na.n_valid() > 0 &&
            pd_ptr->csd_na.n_valid() > 0 ?
            vx_opt.get_n_cdf_bin() : 1);

   // Allocate memory
   ecnt_info = new ECNTInfo [n_bin];

   // Process the climo CDF bins
   for(i=0; i<n_bin; i++) {

      // Apply climo CDF bins logic to subset pairs
      if(n_bin > 1) pd = subset_climo_cdf_bin(*pd_ptr,
                            vx_opt.cdf_info.cdf_ta, i);
      else          pd = *pd_ptr;

      // Store threshold
      ecnt_info[i].othresh = othresh;

      // Check for no matched pairs to process
      if(pd.n_obs == 0) continue;

      // Compute ensemble statistics
      ecnt_info[i].set(pd);

      // Write out ECNT
      if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
         vx_opt.output_flag[i_ecnt] != STATOutputType_None &&
         ecnt_info[i].n_pair > 0) {
         write_ecnt_row(shc, ecnt_info[i], vx_opt.output_flag[i_ecnt],
                        i, n_bin, stat_at, i_stat_row,
                        txt_at[i_ecnt], i_txt_row[i_ecnt]);
      }
   } // end for i (n_bin)

   // Write the mean of the climo CDF bins
   if(n_bin > 1) {

      // Compute ECNT climo CDF bin means
      if(vx_opt.output_flag[i_ecnt] != STATOutputType_None) {

         ECNTInfo ecnt_mean;
         compute_ecnt_mean(ecnt_info, n_bin, ecnt_mean);

         if(ecnt_mean.n_pair > 0) {
            write_ecnt_row(shc, ecnt_mean,
                           vx_opt.output_flag[i_ecnt],
                           -1, n_bin, stat_at, i_stat_row,
                           txt_at[i_ecnt], i_txt_row[i_ecnt]);
         }
      }
   } // end if n_bin > 1

   // Dealloate memory
   if(ecnt_info) { delete [] ecnt_info; ecnt_info = (ECNTInfo *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_rps(const EnsembleStatVxOpt &vx_opt,
            const SingleThresh &othresh,
            const PairDataEnsemble *pd_ptr) {
   RPSInfo rps_info;

   // Check for valid pointer
   if(!pd_ptr) return;

   // Store observation filering threshold
   rps_info.othresh = othresh;
   rps_info.set_prob_cat_thresh(vx_opt.prob_cat_ta);

   // If prob_cat_thresh is empty and climo data is available,
   // use climo_cdf thresholds instead
   if(rps_info.fthresh.n()      == 0 &&
      pd_ptr->cmn_na.n_valid()   > 0 &&
      pd_ptr->csd_na.n_valid()   > 0 &&
      vx_opt.cdf_info.cdf_ta.n() > 0) {
      rps_info.set_cdp_thresh(vx_opt.cdf_info.cdf_ta);
   }

   // Compute ensemble RPS statistics
   rps_info.set(*pd_ptr);

   // Write out RPS
   if(vx_opt.output_flag[i_rps] != STATOutputType_None &&
      rps_info.n_pair > 0) {
      write_rps_row(shc, rps_info, vx_opt.output_flag[i_rps],
                    stat_at, i_stat_row,
                    txt_at[i_rps], i_txt_row[i_rps]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clear_counts() {
   int i, j, k;

   // Allocate memory in one big chunk based on grid size, if needed
   count_na.extend(nxy);
   min_na.extend(nxy);
   max_na.extend(nxy);
   sum_na.extend(nxy);
   sum_sq_na.extend(nxy);
   for(i=0; i<conf_info.get_max_n_thresh(); i++) {
      thresh_count_na[i].extend(nxy);
      for(j=0; j<conf_info.get_n_nbrhd(); j++) {
         thresh_nbrhd_count_na[i][j].extend(nxy);
      }
   }

   // Erase existing values
   count_na.erase();
   min_na.erase();
   max_na.erase();
   sum_na.erase();
   sum_sq_na.erase();
   for(i=0; i<conf_info.get_max_n_thresh(); i++) {
      thresh_count_na[i].erase();
      for(j=0; j<conf_info.get_n_nbrhd(); j++) {
         thresh_nbrhd_count_na[i][j].erase();
      }
   }

   // Initialize arrays
   for(i=0; i<nxy; i++) {
      count_na.add(0);
      min_na.add(bad_data_double);
      max_na.add(bad_data_double);
      sum_na.add(0.0);
      sum_sq_na.add(0.0);
      for(j=0; j<conf_info.get_max_n_thresh(); j++) {
         thresh_count_na[j].add(0);
         for(k=0; k<conf_info.get_n_nbrhd(); k++) {
            thresh_nbrhd_count_na[j][k].add(0);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void track_counts(int i_vx, const DataPlane &dp) {
   int i, j, k;
   double v;

   // Pointers to data buffers for faster access
   const double *Data = dp.data();
   double *CountBuf   = count_na.buf();
   double *MinBuf     = min_na.buf();
   double *MaxBuf     = max_na.buf();
   double *SumBuf     = sum_na.buf();
   double *SumSqBuf   = sum_sq_na.buf();

   // Ensemble thresholds
   const int Nthresh = conf_info.ens_ta[i_vx].n();
   SingleThresh *ThreshBuf = conf_info.ens_ta[i_vx].buf();

   // Increment counts for each grid point
   for(i=0; i<nxy; i++) {

      // Get current value
      v = *Data++;

      // Skip the bad data value
      if(is_bad_data(v)) continue;

      // Otherwise, update counts and sums
      else {

         CountBuf[i] += 1;

         if(v <= MinBuf[i] || is_bad_data(MinBuf[i])) MinBuf[i] = v;
         if(v >= MaxBuf[i] || is_bad_data(MaxBuf[i])) MaxBuf[i] = v;

         SumBuf[i]   += v;
         SumSqBuf[i] += v*v;

         for(j=0; j<Nthresh; j++) {
            if(ThreshBuf[j].check(v)) thresh_count_na[j].inc(i, 1);
         }
      } // end else

   } // end for i

   // Increment NMEP count anywhere fractional coverage > 0
   if(conf_info.nc_info.do_nmep) {
      DataPlane frac_dp;

      // Loop over thresholds
      for(i=0; i<Nthresh; i++) {

         // Loop over neighborhood sizes
         for(j=0; j<conf_info.get_n_nbrhd(); j++) {

            // Compute fractional coverage
            fractional_coverage(dp, frac_dp,
               conf_info.nbrhd_prob.width[j],
               conf_info.nbrhd_prob.shape,
               ThreshBuf[i], conf_info.nbrhd_prob.vld_thresh);

            // Increment counts
            const double *Frac = frac_dp.data();
            for(k=0; k<nxy; k++) {
               if(Frac[k] > 0) thresh_nbrhd_count_na[i][j].inc(k, 1);
            } // end for k

         } // end for j
      } // end for i
   } // end if do_nmep

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString get_ens_mn_var_name(int i_vx) {
   ConcatString cs;

   cs << conf_info.vx_opt[i_vx].vx_pd.fcst_info->name_attr() << "_"
      << conf_info.vx_opt[i_vx].vx_pd.fcst_info->level_attr()
      << "_ENS_MEAN";
      cs.replace(",", "_",   false);
      cs.replace("*", "all", false);
   mlog << Debug(4) << "Generated mean field: " << cs << "\n";

   return(cs);
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(unixtime valid_ut, const char *suffix) {
   ConcatString out_nc_file;

   // Create output NetCDF file name
   build_outfile_name(ens_valid_ut, suffix, out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = open_ncfile(out_nc_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_nc_file.text(), program_name,
                       conf_info.model.c_str(), conf_info.obtype.c_str());

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = add_dim(nc_out, "lat", (long) grid.ny());
   lon_dim = add_dim(nc_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   if(conf_info.nc_info.do_latlon) {
      write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);
   }

   // Add grid weight variable
   if(conf_info.nc_info.do_weight) {
      write_netcdf_grid_weight(nc_out, &lat_dim, &lon_dim,
                               conf_info.grid_weight_flag, wgt_dp);
   }

   // Append to the list of output files
   out_nc_file_list.add(out_nc_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files() {
   int  i, n, n_phist_bin, n_vld, max_col;
   ConcatString tmp_str;

   // Check to see if the text files have already been set up
   if(stat_at.nrows() > 0 || stat_at.ncols() > 0) return;

   // Create output file names for the stat file and optional text files
   build_outfile_name(ens_valid_ut, "", tmp_str);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Compute the number of PHIST bins
   for(i=n_phist_bin=0; i<conf_info.get_n_vx(); i++) {
      n = ceil(1.0 / conf_info.vx_opt[i].vx_pd.pd[0][0][0].phist_bin_size);
      n_phist_bin = (n > n_phist_bin ? n : n_phist_bin);
   }

   // Store the maximum number of valid verification fields
   n_vld    = n_vx_vld.max();

   // Get the maximum number of data columns
   max_col  = max(get_n_orank_columns(n_vld+1),
                  get_n_rhist_columns(n_vld));
   max_col  = max(max_col, get_n_phist_columns(n_phist_bin));
   max_col  = max(max_col, n_ecnt_columns);
   max_col  = max(max_col, n_ssvar_columns);
   max_col  = max(max_col, get_n_relp_columns(n_vld));
   max_col += n_header_columns;

   // Initialize file stream
   stat_out = (ofstream *) 0;

   // Build the file name
   stat_file << tmp_str << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file.c_str());

   // Setup the STAT AsciiTable
   stat_at.set_size(conf_info.n_stat_row() + 1, max_col);
   setup_table(stat_at);

   // Write the text header row
   write_header_row((const char **) 0, 0, 1, stat_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_stat_row = 1;

   /////////////////////////////////////////////////////////////////////
   //
   // Setup each of the optional output text files
   //
   /////////////////////////////////////////////////////////////////////

   // Loop through output line type
   for(i=0; i<n_txt; i++) {

      // Only set it up if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Only create ORANK file when using point observations
         if(i == i_orank && !point_obs_flag) continue;

         // Initialize file stream
         txt_out[i] = (ofstream *) 0;

         // Build the file name
         txt_file[i] << tmp_str << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i].c_str());

         // Get the maximum number of columns for this line type
         switch(i) {

            case(i_rhist):
               max_col = get_n_rhist_columns(n_vld+1) + n_header_columns + 1;
               break;

            case(i_phist):
               max_col = get_n_phist_columns(n_phist_bin) + n_header_columns + 1;
               break;

            case(i_relp):
               max_col = get_n_relp_columns(n_vld) + n_header_columns + 1;
               break;

            case(i_orank):
               max_col = get_n_orank_columns(n_vld) + n_header_columns + 1;
               break;

            default:
               max_col = n_txt_columns[i]  + n_header_columns + 1;
               break;
         } // end switch

         // Setup the text AsciiTable
         txt_at[i].set_size(conf_info.n_txt_row(i) + 1, max_col);
         setup_table(txt_at[i]);

         // Write the text header row
         switch(i) {

            case(i_rhist):
               write_rhist_header_row(1, n_vld+1, txt_at[i], 0, 0);
               break;

            case(i_phist):
               write_phist_header_row(1, n_phist_bin, txt_at[i], 0, 0);
               break;

            case(i_relp):
               write_relp_header_row(1, n_vld, txt_at[i], 0, 0);
               break;

            case(i_orank):
               write_orank_header_row(1, n_vld, txt_at[i], 0, 0);
               break;

            default:
               write_header_row(txt_columns[i], n_txt_columns[i], 1,
                  txt_at[i], 0, 0);
               break;
         } // end switch

         // Initialize the row index to 1 to account for the header
         i_txt_row[i] = 1;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Justify the STAT AsciiTable objects
   justify_stat_cols(at);

   // Set the precision
   at.set_precision(conf_info.conf.output_precision());

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime ut, const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   char tmp_str[max_str_len];

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir.text() << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      str << "_" << conf_info.output_prefix;

   // Append the timing information
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   snprintf(tmp_str, sizeof(tmp_str), "%.4i%.2i%.2i_%.2i%.2i%.2iV",
            yr, mon, day, hr, min, sec);
   str << "_" << tmp_str;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_nc(int i_ens, DataPlane &dp) {
   int i, j, k, l;
   double t, v;
   char type_str[max_str_len];
   DataPlane prob_dp, smooth_dp;

   // Arrays for storing ensemble data
   float *ens_mean  = (float *) 0;
   float *ens_stdev = (float *) 0;
   float *ens_minus = (float *) 0;
   float *ens_plus  = (float *) 0;
   float *ens_min   = (float *) 0;
   float *ens_max   = (float *) 0;
   float *ens_range = (float *) 0;
   int   *ens_vld   = (int   *) 0;
   float *ens_prob  = (float *) 0;

   // Allocate memory for storing ensemble data
   ens_mean  = new float [grid.nx()*grid.ny()];
   ens_stdev = new float [grid.nx()*grid.ny()];
   ens_minus = new float [grid.nx()*grid.ny()];
   ens_plus  = new float [grid.nx()*grid.ny()];
   ens_min   = new float [grid.nx()*grid.ny()];
   ens_max   = new float [grid.nx()*grid.ny()];
   ens_range = new float [grid.nx()*grid.ny()];
   ens_vld   = new int   [grid.nx()*grid.ny()];
   ens_prob  = new float [grid.nx()*grid.ny()];

   // Store the threshold for the ratio of valid data points
   t = conf_info.vld_data_thresh;

   // Store the data
   for(i=0; i<count_na.n(); i++) {

      // Valid data count
      ens_vld[i] = nint(count_na[i]);

      // Check for too much missing data
      if((double) (count_na[i]/n_ens_vld[i_ens]) < t) {
         ens_mean[i]  = bad_data_float;
         ens_stdev[i] = bad_data_float;
         ens_minus[i] = bad_data_float;
         ens_plus[i]  = bad_data_float;
         ens_min[i]   = bad_data_float;
         ens_max[i]   = bad_data_float;
         ens_range[i] = bad_data_float;
      }
      else {

         // Compute ensemble summary
         ens_mean[i]  = (float) (sum_na[i]/count_na[i]);
         ens_stdev[i] = (float) compute_stdev(sum_na[i], sum_sq_na[i], ens_vld[i]);
         ens_minus[i] = (float) ens_mean[i] - ens_stdev[i];
         ens_plus[i]  = (float) ens_mean[i] + ens_stdev[i];
         ens_min[i]   = (float) min_na[i];
         ens_max[i]   = (float) max_na[i];
         v = max_na[i] - min_na[i];
         if(is_eq(v, 0.0)) v = 0;
         ens_range[i] = (float) v;
      }
   } // end for i

   // Add the ensemble mean, if requested
   if(conf_info.nc_info.do_mean) {
      write_ens_var_float(i_ens, ens_mean, dp,
                          "ENS_MEAN",
                          "Ensemble Mean");
   }

   // Add the ensemble standard deviation, if requested
   if(conf_info.nc_info.do_stdev) {
      write_ens_var_float(i_ens, ens_stdev, dp,
                          "ENS_STDEV",
                          "Ensemble Standard Deviation");
   }

   // Add the ensemble mean minus one standard deviation, if requested
   if(conf_info.nc_info.do_minus) {
      write_ens_var_float(i_ens, ens_minus, dp,
                          "ENS_MINUS",
                          "Ensemble Mean Minus 1 Standard Deviation");
   }

   // Add the ensemble mean plus one standard deviation, if requested
   if(conf_info.nc_info.do_plus) {
      write_ens_var_float(i_ens, ens_plus, dp,
                          "ENS_PLUS",
                          "Ensemble Mean Plus 1 Standard Deviation");
   }

   // Add the ensemble minimum value, if requested
   if(conf_info.nc_info.do_min) {
      write_ens_var_float(i_ens, ens_min, dp,
                          "ENS_MIN",
                          "Ensemble Minimum");
   }

   // Add the ensemble maximum value, if requested
   if(conf_info.nc_info.do_max) {
      write_ens_var_float(i_ens, ens_max, dp,
                          "ENS_MAX",
                          "Ensemble Maximum");
   }

   // Add the ensemble range, if requested
   if(conf_info.nc_info.do_range) {
      write_ens_var_float(i_ens, ens_range, dp,
                          "ENS_RANGE",
                          "Ensemble Range");
   }

   // Add the ensemble valid data count, if requested
   if(conf_info.nc_info.do_vld) {
      write_ens_var_int(i_ens, ens_vld, dp,
                        "ENS_VLD",
                        "Ensemble Valid Data Count");
   }

   // Add the ensemble relative frequencies and neighborhood probabilities, if requested
   if(conf_info.nc_info.do_freq || conf_info.nc_info.do_nep) {

      prob_dp.set_size(dp.nx(), dp.ny());

      // Loop through each threshold
      for(i=0; i<conf_info.ens_ta[i_ens].n(); i++) {

         // Initialize
         prob_dp.erase();

         // Output variable name
         snprintf(type_str, sizeof(type_str), "ENS_FREQ_%s",
                  conf_info.ens_ta[i_ens][i].get_abbr_str().contents().c_str());

         // Compute the ensemble relative frequency
         for(j=0; j<count_na.n(); j++) {

            // Check for too much missing data
            if((double) (count_na[j]/n_ens_vld[i_ens]) < t) {
               ens_prob[j] = bad_data_float;
            }
            else {
               ens_prob[j] = (float) (thresh_count_na[i][j]/count_na[j]);
            }

            // Also store value in a DataPlane object
            prob_dp.buf()[j] = ens_prob[j];

         } // end for j

         // Write the ensemble relative frequency
         if(conf_info.nc_info.do_freq) {
            write_ens_var_float(i_ens, ens_prob, dp,
                                type_str,
                                "Ensemble Relative Frequency");
         }

         // Write the neighborhood ensemble probability
         if(conf_info.nc_info.do_nep) {

            GaussianInfo info;
            // Compute the mean neighborhood probability
            for(j=0; j<conf_info.get_n_nbrhd(); j++) {

               smooth_dp = smooth_field(prob_dp, InterpMthd_UW_Mean,
                              conf_info.nbrhd_prob.width[j],
                              conf_info.nbrhd_prob.shape,
                              conf_info.nbrhd_prob.vld_thresh, info);

               for(k=0; k<count_na.n(); k++) {
                  ens_prob[k] = (float) smooth_dp.buf()[k];
               }

               // Output variable name
               snprintf(type_str, sizeof(type_str), "ENS_NEP_%s_%s%i",
                        conf_info.ens_ta[i_ens][i].get_abbr_str().contents().c_str(),
                        interpmthd_to_string(InterpMthd_Nbrhd).c_str(),
                        conf_info.nbrhd_prob.width[j]*conf_info.nbrhd_prob.width[j]);

               write_ens_var_float(i_ens, ens_prob, dp,
                                   type_str,
                                   "Neighborhood Ensemble Probability");

            } // end for j
         } // end if do_nep
      } // end for i
   } // end if

   // Add the neighborhood maximum ensemble probabilities, if requested
   if(conf_info.nc_info.do_nmep) {

      prob_dp.set_size(dp.nx(), dp.ny());

      // Loop through each threshold
      for(i=0; i<conf_info.ens_ta[i_ens].n(); i++) {

         // Loop through each neigbhorhood size
         for(j=0; j<conf_info.get_n_nbrhd(); j++) {

            // Initialize
            prob_dp.erase();

            // Compute the neighborhood maximum ensemble probability
            for(k=0; k<count_na.n(); k++) {

               // Check for too much missing data
               if((double) (count_na[k]/n_ens_vld[i_ens]) < t) {
                  prob_dp.buf()[k] = bad_data_double;
               }
               else {
                  prob_dp.buf()[k] = (double) (thresh_nbrhd_count_na[i][j][k]/count_na[k]);
               }
            } // end for k

            // Apply requested NMEP smoothers
            for(k=0; k<conf_info.nmep_smooth.n_interp; k++) {

               smooth_dp = smooth_field(prob_dp,
                              string_to_interpmthd(conf_info.nmep_smooth.method[k].c_str()),
                              conf_info.nmep_smooth.width[k],
                              conf_info.nmep_smooth.shape,
                              conf_info.nmep_smooth.vld_thresh,
                              conf_info.nmep_smooth.gaussian);

               for(l=0; l<count_na.n(); l++) {
                  ens_prob[l] = (float) smooth_dp.buf()[l];
               }

               // Output variable name
               snprintf(type_str, sizeof(type_str), "ENS_NMEP_%s_%s%i_%s%i",
                        conf_info.ens_ta[i_ens][i].get_abbr_str().contents().c_str(),
                        interpmthd_to_string(InterpMthd_Nbrhd).c_str(),
                        conf_info.nbrhd_prob.width[j]*conf_info.nbrhd_prob.width[j],
                        conf_info.nmep_smooth.method[k].c_str(),
                        conf_info.nmep_smooth.width[k]*conf_info.nmep_smooth.width[k]);

               write_ens_var_float(i_ens, ens_prob, dp,
                                   type_str,
                                   "Neighborhood Maximum Ensemble Probability");

            } // end for k
         } // end for j
      } // end for i
   } // end if do_nep

   // Deallocate and clean up
   if(ens_mean)  { delete [] ens_mean;  ens_mean  = (float *) 0; }
   if(ens_stdev) { delete [] ens_stdev; ens_stdev = (float *) 0; }
   if(ens_minus) { delete [] ens_minus; ens_minus = (float *) 0; }
   if(ens_plus)  { delete [] ens_plus;  ens_plus  = (float *) 0; }
   if(ens_min)   { delete [] ens_min;   ens_min   = (float *) 0; }
   if(ens_max)   { delete [] ens_max;   ens_max   = (float *) 0; }
   if(ens_range) { delete [] ens_range; ens_range = (float *) 0; }
   if(ens_vld)   { delete [] ens_vld;   ens_vld   = (int   *) 0; }
   if(ens_prob)  { delete [] ens_prob;  ens_prob  = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_float(int i_ens, float *ens_data, DataPlane &dp,
                         const char *type_str,
                         const char *long_name_str) {
   NcVar ens_var;
   ConcatString ens_var_name, var_str, name_str, cs;

   // Append nc_pairs_var_str config file entry
   cs = conf_info.ens_var_str[i_ens];
   if(cs.length() > 0) var_str << "_" << cs;

   // Construct the variable name
   ens_var_name << cs_erase
                << conf_info.ens_info[i_ens]->name_attr() << "_"
                << conf_info.ens_info[i_ens]->level_attr()
                << var_str << "_" << type_str;

   // Skip variable names that have already been written
   if(nc_ens_var_sa.has(ens_var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_ens_var_sa.add(ens_var_name);

   ens_var = add_var(nc_out, (string)ens_var_name, ncFloat, lat_dim, lon_dim);

   //
   // Construct the variable name attribute
   // For the ensemble mean, just use the variable name.
   // For all other fields, append the field type.
   //
   if(strcmp(type_str, "ENS_MEAN") == 0) {
      name_str << cs_erase
               << conf_info.ens_info[i_ens]->name_attr();
   }
   else {
      name_str << cs_erase
               << conf_info.ens_info[i_ens]->name_attr() << "_"
               << type_str;
   }

   // Add the variable attributes
   add_var_att_local(conf_info.ens_info[i_ens], &ens_var, false, dp,
                     name_str.c_str(), long_name_str);

   // Write the data
   if(!put_nc_data_with_dims(&ens_var, &ens_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_ens_var_float() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_int(int i_ens, int *ens_data, DataPlane &dp,
                       const char *type_str,
                       const char *long_name_str) {
   NcVar ens_var;
   ConcatString ens_var_name, var_str, name_str, cs;

   // Append nc_pairs_var_str config file entry
   cs = conf_info.ens_var_str[i_ens];
   if(cs.length() > 0) var_str << "_" << cs;

   // Construct the variable name
   ens_var_name << cs_erase
                << conf_info.ens_info[i_ens]->name_attr() << "_"
                << conf_info.ens_info[i_ens]->level_attr()
                << var_str << "_" << type_str;

   // Skip variable names that have already been written
   if(nc_ens_var_sa.has(ens_var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_ens_var_sa.add(ens_var_name);

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = conf_info.get_compression_level();
   ens_var = add_var(nc_out, (string)ens_var_name, ncInt, lat_dim, lon_dim, deflate_level);

   // Construct the variable name attribute
   name_str << cs_erase
            << conf_info.ens_info[i_ens]->name_attr() << "_"
            << type_str;

   // Add the variable attributes
   add_var_att_local(conf_info.ens_info[i_ens], &ens_var, true, dp,
                     name_str.c_str(), long_name_str);

   // Write the data
   if(!put_nc_data_with_dims(&ens_var, &ens_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_ens_var_int() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_nc(PairDataEnsemble &pd, DataPlane &dp,
                    int i_vx, int i_interp, int i_mask) {
   int i, n;

   // Arrays for storing observation rank data
   float *obs_v    = (float *) 0;
   int   *obs_rank = (int *)   0;
   float *obs_pit  = (float *) 0;
   int   *ens_vld  = (int *)   0;

   // Allocate memory for storing ensemble data
   obs_v    = new float [grid.nx()*grid.ny()];
   obs_rank = new int   [grid.nx()*grid.ny()];
   obs_pit  = new float [grid.nx()*grid.ny()];
   ens_vld  = new int   [grid.nx()*grid.ny()];

   // Initialize
   for(i=0; i<grid.nx()*grid.ny(); i++) {
      obs_v[i]    = bad_data_float;
      obs_rank[i] = bad_data_int;
      obs_pit[i]  = bad_data_float;
      ens_vld[i]  = bad_data_int;
   }

   // Loop over all the pairs
   for(i=0; i<pd.n_obs; i++) {

      n = DefaultTO.two_to_one(grid.nx(), grid.ny(),
                               nint(pd.x_na[i]), nint(pd.y_na[i]));

      // Store the observation value, rank, and number of valid ensembles
      obs_v[n]    = (float) pd.o_na[i];
      obs_rank[n] = nint(pd.r_na[i]);
      obs_pit[n]  = (float) pd.pit_na[i];
      ens_vld[n]  = nint(pd.v_na[i]);

   } // end for i

   // Add the observation values
   write_orank_var_float(i_vx, i_interp, i_mask, obs_v, dp,
                         "OBS",
                         "Observation Value");

   // Add the observation ranks
   write_orank_var_int(i_vx, i_interp, i_mask, obs_rank, dp,
                       "OBS_RANK",
                       "Observation Rank");

   // Add the probability integral transforms
   write_orank_var_float(i_vx, i_interp, i_mask, obs_pit, dp,
                         "OBS_PIT",
                         "Probability Integral Transform");

   // Add the number of valid ensemble members
   write_orank_var_int(i_vx, i_interp, i_mask, ens_vld, dp,
                       "ENS_VLD",
                       "Ensemble Valid Data Count");

   // Deallocate and clean up
   if(obs_v)    { delete [] obs_v;    obs_v    = (float *) 0; }
   if(obs_rank) { delete [] obs_rank; obs_rank = (int   *) 0; }
   if(obs_pit)  { delete [] obs_pit;  obs_pit  = (float *) 0; }
   if(ens_vld)  { delete [] ens_vld;  ens_vld  = (int   *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_var_float(int i_vx, int i_interp, int i_mask,
                           float *data, DataPlane &dp,
                           const char *type_str,
                           const char *long_name_str) {
   NcVar nc_var;
   int wdth;
   ConcatString mthd_str, var_name, var_str, name_str;

   // Append nc_var_str config file entry
   if(conf_info.vx_opt[i_vx].var_str.length() > 0) {
      var_str << "_" << conf_info.vx_opt[i_vx].var_str;
   }

   // Get the interpolation method string and width
   mthd_str = conf_info.vx_opt[i_vx].interp_info.method[i_interp];
   wdth     = conf_info.vx_opt[i_vx].interp_info.width[i_interp];

   // Build the orank variable name
   var_name << cs_erase
            << conf_info.vx_opt[i_vx].vx_pd.obs_info->name_attr() << "_"
            << conf_info.vx_opt[i_vx].vx_pd.obs_info->level_attr()
            << var_str << "_" << type_str << "_"
            << conf_info.vx_opt[i_vx].mask_name_area[i_mask];

   // Construct the variable name attribute
   name_str << cs_erase
            << conf_info.vx_opt[i_vx].vx_pd.obs_info->name_attr() << "_"
            << type_str << "_"
            << conf_info.vx_opt[i_vx].mask_name_area[i_mask];

   // Append smoothing information, except for the raw observations
   ConcatString type_cs(type_str);
   if(wdth > 1 &&
      (type_cs != "OBS" ||
       conf_info.vx_opt[i_vx].interp_info.field == FieldType_Obs ||
       conf_info.vx_opt[i_vx].interp_info.field == FieldType_Both)) {
      var_name << "_" << mthd_str << "_" << wdth*wdth;
      name_str << "_" << mthd_str << "_" << wdth*wdth;
   }

   // Skip variable names that have already been written
   if(nc_orank_var_sa.has(var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_orank_var_sa.add(var_name);

   // Define the variable
   nc_var = add_var(nc_out, (string)var_name, ncFloat, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att_local(conf_info.vx_opt[i_vx].vx_pd.fcst_info, &nc_var, false, dp,
                     name_str.c_str(), long_name_str);

   // Write the data
   if(!put_nc_data_with_dims(&nc_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_orank_var_float() -> "
           << "error in nc_var->put for the " << var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_var_int(int i_vx, int i_interp, int i_mask,
                         int *data, DataPlane &dp,
                         const char *type_str,
                         const char *long_name_str) {
   NcVar nc_var;
   int wdth;
   ConcatString mthd_str, var_name, var_str, name_str;

   // Append nc_var_str config file entry
   if(conf_info.vx_opt[i_vx].var_str.length() > 0) {
      var_str << "_" << conf_info.vx_opt[i_vx].var_str;
   }

   // Get the interpolation method string and width
   mthd_str = conf_info.vx_opt[i_vx].interp_info.method[i_interp];
   wdth     = conf_info.vx_opt[i_vx].interp_info.width[i_interp];

   // Build the orank variable name
   var_name << cs_erase
            << conf_info.vx_opt[i_vx].vx_pd.obs_info->name_attr() << "_"
            << conf_info.vx_opt[i_vx].vx_pd.obs_info->level_attr()
            << var_str << "_" << type_str << "_"
            << conf_info.vx_opt[i_vx].mask_name_area[i_mask];

   // Construct the variable name attribute
   name_str << cs_erase
            << conf_info.vx_opt[i_vx].vx_pd.obs_info->name_attr() << "_"
            << type_str << "_"
            << conf_info.vx_opt[i_vx].mask_name_area[i_mask];

   // Append smoothing information
   if(wdth > 1) {
      var_name << "_" << mthd_str << "_" << wdth*wdth;
      name_str << "_" << mthd_str << "_" << wdth*wdth;
   }

   // Skip variable names that have already been written
   if(nc_orank_var_sa.has(var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_orank_var_sa.add(var_name);

   // Define the variable
   nc_var = add_var(nc_out, (string)var_name, ncInt, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att_local(conf_info.vx_opt[i_vx].vx_pd.fcst_info, &nc_var, true, dp,
                     name_str.c_str(), long_name_str);

   // Write the data
   if(!put_nc_data_with_dims(&nc_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_orank_var_int() -> "
           << "error in nc_var->put for the " << var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void add_var_att_local(VarInfo *info, NcVar *nc_var, bool is_int, DataPlane &dp,
                       const char *name_str, const char *long_name_str) {
   ConcatString att_str;

   // Construct the long name
   att_str << cs_erase
           << info->name_attr() << " at "
           << info->level_attr() << " "
           << long_name_str;

   // Add variable attributes
   add_att(nc_var, "name", name_str);
   add_att(nc_var, "long_name", (string)att_str);
   add_att(nc_var, "level", (string)info->level_attr());
   add_att(nc_var, "units", (string)info->units_attr());

   if(is_int) add_att(nc_var, "_FillValue", bad_data_int);
   else       add_att(nc_var, "_FillValue", bad_data_float);

   // Write out times
   write_netcdf_var_times(nc_var, dp);

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file.c_str());
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i].c_str());
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {
   int i, j;

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Close the output text files that were open for writing
   if(vx_flag) finish_txt_files();

   // List the output NetCDF files
   for(i=0; i<out_nc_file_list.n(); i++) {
      mlog << Debug(1)
           << "Output file: " << out_nc_file_list[i] << "\n";
   }

   // Deallocate threshold count arrays
   if(thresh_count_na) {
      for(i=0; i<conf_info.get_max_n_thresh(); i++) {
         thresh_count_na[i].clear();
      }
      delete [] thresh_count_na;
      thresh_count_na = (NumArray *) 0;
   }
   if(thresh_nbrhd_count_na) {
      for(i=0; i<conf_info.get_max_n_thresh(); i++) {
         for(j=0; j<conf_info.get_n_nbrhd(); j++) {
            thresh_nbrhd_count_na[i][j].clear();
         }
         delete [] thresh_nbrhd_count_na[i];
         thresh_nbrhd_count_na[i] = (NumArray *) 0;
      }
      delete [] thresh_nbrhd_count_na;
      thresh_nbrhd_count_na = (NumArray **) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tn_ens ens_file_1 ... ens_file_n | ens_file_list\n"
        << "\tconfig_file\n"
        << "\t[-grid_obs file]\n"
        << "\t[-point_obs file]\n"
        << "\t[-ens_mean file]\n"
        << "\t[-obs_valid_beg time]\n"
        << "\t[-obs_valid_end time]\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"n_ens ens_file_1 ... ens_file_n\" is the number "
        << "of ensemble members followed by a list of ensemble member "
        << "file names (required).\n"

        << "\t\t\"ens_file_list\" is an ASCII file containing a list "
        << "of ensemble member file names (required).\n"

        << "\t\t\"config_file\" is an EnsembleStatConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-grid_obs file\" specifies a gridded observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-point_obs file\" specifies a NetCDF point observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-ens_mean file\" specifies an ensemble mean model data file "
        << "(optional).\n"

        << "\t\t\"-obs_valid_beg time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "beginning of the matching time window (optional).\n"

        << "\t\t\"-obs_valid_end time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "end of the matching time window (optional).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable ("
        << conf_info.get_compression_level() << ") (optional).\n\n" << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grid_obs(const StringArray & a)
{
   grid_obs_file_list.add(a[0]);
   grid_obs_flag = 1;
}

////////////////////////////////////////////////////////////////////////

void set_point_obs(const StringArray & a)
{
   point_obs_file_list.add(a[0]);
   point_obs_flag = 1;
}

////////////////////////////////////////////////////////////////////////

void set_ens_mean(const StringArray & a)
{
   ens_mean_user = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_beg(const StringArray & a)
{
   obs_valid_beg_ut = timestring_to_unix(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_end(const StringArray & a)
{
   obs_valid_end_ut = timestring_to_unix(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a)
{
   out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a)
{
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
