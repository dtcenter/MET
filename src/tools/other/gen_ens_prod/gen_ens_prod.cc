// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gen_ens_prod.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    09/10/21  Halley Gotway  MET #1904 Initial version.
//   001    11/15/21  Halley Gotway  MET #1968 Ensemble -ctrl error check.
//   002    01/14/21  McCabe         MET #1695 All members in one file.
//   003    02/17/22  Halley Gotway  MET #1918 Add normalize config option.
//   004    07/06/22  Howard Soh     METplus-Internal #19 Rename main to met_main
//   005    10/03/22  Prestopnik     MET #2227 Remove using namespace std and netCDF from header files
//   006    04/29/24  Halley Gotway  MET #2870 Ignore MISSING keyword.
//   007    05/07/25  Halley Gotway  MET #3145 Add OpenMP.
//   008    01/14/26  Halley Gotway  MET #3294 Add EAS probabilities.
//
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <limits.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>

#include "main.h"
#include "gen_ens_prod.h"
#include "compute_eas.h"

#include "vx_nc_util.h"
#include "vx_data2d_nc_met.h"
#include "vx_regrid.h"
#include "vx_log.h"

#include "nc_obs_util.h"
#include "nc_point_obs_in.h"

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_grid(const Grid &);
static void process_ensemble();

static void get_climo_mean_stdev(GenEnsProdVarInfo *, int,
                                 bool, int, DataPlane &, DataPlane &);
static void get_ens_mean_stdev(GenEnsProdVarInfo *, DataPlane &, DataPlane &);
static bool get_data_plane(const char *, GrdFileType, VarInfo *, DataPlane &);

static void clear_counts();
static void track_counts(const GenEnsProdVarInfo *, const DataPlane &,
                         bool, const DataPlane &, const DataPlane &);

static void setup_nc_file();
static void write_ens_nc(GenEnsProdVarInfo *, int, const DataPlane &,
                         const DataPlane &, const DataPlane &);
static void write_eas_nc(GenEnsProdVarInfo *, const DataPlane &,
                         const DataPlane &, const DataPlane &);
static void write_ens_var_float(GenEnsProdVarInfo *, const float *,
                                const DataPlane &,
                                const char *, const char *);
static void write_ens_var_int(GenEnsProdVarInfo *, const int *,
                              const DataPlane &,
                              const char *, const char *);
static void write_ens_data_plane(GenEnsProdVarInfo *,
                                 const DataPlane &, const DataPlane &,
                                 const char *, const char *);

static void add_var_att_local(GenEnsProdVarInfo *, NcVar *, bool is_int,
                              const DataPlane &, const char *, const char *);

static void clean_up();
static void usage(int exit_code=1);

static void set_ens_files  (const StringArray &);
static void set_out_file   (const StringArray &);
static void set_config_file(const StringArray &);
static void set_ctrl_file  (const StringArray &);

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the ensemble fields
   process_ensemble();

   // Close output files and deallocate memory
   clean_up();

   return 0;
}

////////////////////////////////////////////////////////////////////////

string get_tool_name() {
   return "gen_ens_prod";
}

////////////////////////////////////////////////////////////////////////

static void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;
   const char *method_name = "process_command_line() -> ";

   //
   // Check for zero arguments
   //
   if(argc == 1) usage();

   //
   // Parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // Set the usage function
   //
   cline.set_usage(usage);

   //
   // Add the options function calls
   //
   cline.add(set_ens_files,   "-ens",   -1);
   cline.add(set_out_file,    "-out",    1);
   cline.add(set_config_file, "-config", 1);
   cline.add(set_ctrl_file,   "-ctrl",   1);

   //
   // Parse the command line
   //
   cline.parse();

   // Check for error, there should be zero arguments left
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set
   n_ens_files = ens_files.n();
   if(n_ens_files == 0) {
      mlog << Error << "\n" << method_name
           << "the ensemble file list must be set using the "
           << "\"-ens\" option.\n\n";
      exit(1);
   }
   if(out_file.empty()) {
      mlog << Error << "\n" << method_name
           << "the output file must be set using the "
           << "\"-out\" option.\n\n";
      exit(1);
   }
   if(config_file.empty()) {
      mlog << Error << "\n" << method_name
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      exit(1);
   }

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

   // Get the ensemble file type from the files
   if(etype == FileType_None) {
      etype = parse_file_list_type(ens_files);
   }

   // UGrid not supported
   if(etype == FileType_UGrid) {
      mlog << Error << "\n" << method_name
           << grdfiletype_to_string(etype)
           << " ensemble files are not supported\n\n";
      exit(1);
   }

   // Process the configuration
   conf_info.process_config(etype, &ens_files, ctrl_file.nonempty());

   // Resize vectors to store threshold counts
   thresh_cnt_na.resize(conf_info.get_max_n_cat());
   thresh_nbrhd_cnt_na.resize(conf_info.get_max_n_cat());

   for(int i=0; i<conf_info.get_max_n_cat(); i++) {
      thresh_nbrhd_cnt_na[i].resize(conf_info.get_n_nbrhd());
   }

   // List the input ensemble files
   mlog << Debug(1) << "Ensemble Files["
        << n_ens_files << "]:\n";
   for(int i=0; i<n_ens_files; i++) {
      mlog << "   " << ens_files[i]  << "\n";
   }

   // List the control member file
   mlog << Debug(1) << "Ensemble Control: "
        << (ctrl_file.empty() ? "None" : ctrl_file.c_str()) << "\n";

   // Check for control in the list of ensemble files
   if(ctrl_file.nonempty() && ens_files.has(ctrl_file) && n_ens_files != 1) {
      mlog << Error << "\n" << method_name
           << "the ensemble control file should not appear in the list "
           << "of ensemble member files:\n" << ctrl_file << "\n\n";
      exit(1);
   }

   // Check for missing non-python ensemble files
   for(int i=0; i<n_ens_files; i++) {
      if(!file_exists(ens_files[i].c_str()) &&
         !is_python_grdfiletype(etype)) {
         log_missing_file(method_name, "input ensemble file", ens_files[i]);
         ens_file_vld.add(0);
      }
      else {
         ens_file_vld.add(1);
      }
   }

   if(conf_info.control_id.nonempty() && ctrl_file.empty()) {
      mlog << Warning << "\n" << method_name
           << "control_id is set in the config file but "
           << "control file is not provided with -ctrl argument\n\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

static void process_grid(const Grid &fcst_grid) {
   Grid obs_grid;

   // Parse regridding logic
   RegridInfo ri;
   if(conf_info.get_n_var() > 0) {
      ri = conf_info.ens_input[0]->get_var_info()->regrid();
   }
   else {
      mlog << Error << "\nprocess_grid() -> "
           << "at least one ensemble field must be specified!\n\n";
      exit(1);
   }

   // Determine the verification grid
   grid = parse_vx_grid(ri, &fcst_grid, &fcst_grid);
   nxy  = grid.nx() * grid.ny();

   return;
}

////////////////////////////////////////////////////////////////////////

static void process_ensemble() {
   DataPlane ens_dp;
   DataPlane ctrl_dp;
   DataPlane cmn_dp;
   DataPlane csd_dp;
   DataPlane emn_dp;
   DataPlane esd_dp;
   unixtime max_init_ut = bad_data_ll;

   // Loop through each of the ensemble fields to be processed
   vector<GenEnsProdVarInfo*>::const_iterator var_it = conf_info.ens_input.begin();
   for(int i_var=0; var_it != conf_info.ens_input.end(); var_it++, i_var++) {

      // Need to reinitialize counts and sums for each ensemble field
      bool need_reset = true;

      // When normalizing relative to climatology with MET_ENS_MEMBER_ID set,
      // read climatology separately for each member
      bool set_climo_ens_mem_id =
         (conf_info.ens_member_ids.n() > 1) &&
         ((*var_it)->normalize == NormalizeType::ClimoAnom ||
          (*var_it)->normalize == NormalizeType::ClimoStdAnom);

      // Print out the normalization flag
      ConcatString cs;
      if((*var_it)->normalize != NormalizeType::None) {
         cs << " with normalize = "
            << normalizetype_to_string((*var_it)->normalize);
      }
      cs << "\n";

      mlog << Debug(2) << "\n" << sep_str << "\n\n"
           << "Processing ensemble field: "
           << (*var_it)->raw_magic_str << cs;

      int n_ens_inputs = (*var_it)->inputs_n();
      int n_ens_vld = 0;
      for(int i_ens=0; i_ens < n_ens_inputs; i_ens++) {

         // Get file and VarInfo to process
         ConcatString ens_file((*var_it)->get_file(i_ens));
         auto var_info = (*var_it)->get_var_info(i_ens);

         // Skip bad data files
         if(!ens_file_vld[(*var_it)->get_file_index(i_ens)]) continue;

         mlog << Debug(3)
              << "\nReading ensemble field \""
              << var_info->magic_str() << "\".\n";

         // Read data and track the valid data count
         if(!get_data_plane(ens_file.c_str(), etype,
                            var_info, ens_dp)) {
            mlog << Warning << "\nprocess_ensemble() -> "
                 << "ensemble field \"" << var_info->magic_str()
                 << "\" not found in file \"" << ens_file << "\"\n\n";
            continue;
         }
         else {
            n_ens_vld++;
         }

         // Reinitialize for the current variable
         if(need_reset) {

            need_reset = false;

            // Reset the running sums and counts
            clear_counts();

            // Read climatology data for this field
            get_climo_mean_stdev((*var_it), i_var,
                                 set_climo_ens_mem_id,
                                 i_ens, cmn_dp, csd_dp);

            // Compute the ensemble summary data, if needed
            if((*var_it)->normalize == NormalizeType::FcstAnom ||
               (*var_it)->normalize == NormalizeType::FcstStdAnom) {
               get_ens_mean_stdev((*var_it), emn_dp, esd_dp);
            }
            else {
               emn_dp.erase();
               esd_dp.erase();
            }

            // Read ensemble control member data, if provided
            if(ctrl_file.nonempty()) {
               VarInfo *ctrl_info = (*var_it)->get_ctrl(i_ens);

               mlog << Debug(3) << "\n"
                    << "Reading control field: "
                    << ctrl_info->magic_str() << "\n";

               // Error out if missing
               if(!get_data_plane(ctrl_file.c_str(), etype,
                                  ctrl_info, ctrl_dp)) {
                  mlog << Error << "\nprocess_ensemble() -> "
                       << "control member ensemble field \""
                       << ctrl_info->magic_str()
                       << "\" not found in file \"" << ctrl_file << "\"\n\n";
                  exit(1);
               }

               // Read climo data with MET_ENS_MEMBER_ID set
               if(set_climo_ens_mem_id) {
                  get_climo_mean_stdev((*var_it), i_var,
                                       set_climo_ens_mem_id, i_ens,
                                       cmn_dp, csd_dp);
               }

               // Normalize, if requested
               if((*var_it)->normalize != NormalizeType::None) {
                  normalize_data(ctrl_dp, (*var_it)->normalize,
                                 &cmn_dp, &csd_dp, &emn_dp, &esd_dp);
               }

               // Apply current data to the running sums and counts
               track_counts(*var_it, ctrl_dp, true, cmn_dp, csd_dp);

            } // end if ctrl_file

            mlog << Debug(3)
                 << "Found " << (ctrl_dp.is_empty() ? 0 : 1)
                 << " control member, " << (cmn_dp.is_empty() ? 0 : 1)
                 << " climatology mean, and " << (csd_dp.is_empty() ? 0 : 1)
                 << " climatology standard deviation field(s) for \""
                 << var_info->magic_str() << "\".\n";

         } // end if need_reset

         // Read climo data with MET_ENS_MEMBER_ID set
         if(set_climo_ens_mem_id) {
             get_climo_mean_stdev((*var_it), i_var,
                                  set_climo_ens_mem_id, i_ens,
                                  cmn_dp, csd_dp);
         }

         // Normalize, if requested
         if((*var_it)->normalize != NormalizeType::None) {
            normalize_data(ens_dp, (*var_it)->normalize,
                           &cmn_dp, &csd_dp, &emn_dp, &esd_dp);
         }

         // Apply current data to the running sums and counts
         track_counts(*var_it, ens_dp, false, cmn_dp, csd_dp);

         // Keep track of the maximum initialization time
         if(is_bad_data(max_init_ut) || ens_dp.init() > max_init_ut) {
            max_init_ut = ens_dp.init();
         }

      } // end for it

      // Check for too much missing data
      if(((double) n_ens_vld/n_ens_inputs) < conf_info.vld_ens_thresh) {
         mlog << Error << "\nprocess_ensemble() -> "
              << n_ens_vld << " of " << n_ens_inputs
              << " (" << (double)n_ens_vld/n_ens_inputs << ")"
              << " fields found for \"" << (*var_it)->get_var_info()->magic_str()
              << "\" does not meet the threshold specified by \""
              << conf_key_ens_ens_thresh << "\" (" << conf_info.vld_ens_thresh
              << ") in the configuration file.\n\n";
         exit(1);
      }

      // Write out the ensemble information to a NetCDF file
      ens_dp.set_init(max_init_ut);
      write_ens_nc(*var_it, n_ens_vld, ens_dp, cmn_dp, csd_dp);

   } // end for var_it

   return;
}

////////////////////////////////////////////////////////////////////////

static void get_climo_mean_stdev(GenEnsProdVarInfo *ens_info, int i_var,
                                 bool set_ens_mem_id, int i_ens,
                                 DataPlane &cmn_dp, DataPlane &csd_dp) {

   // Set the MET_ENS_MEMBER_ID environment variable
   if(set_ens_mem_id) {
      setenv(met_ens_member_id, ens_info->get_ens_member_id(i_ens).c_str(), 1);
   }

   mlog << Debug(4)
        << "Reading climatology mean data for ensemble field \""
        << ens_info->get_var_info(i_ens)->magic_str() << "\".\n";

   cmn_dp = read_climo_data_plane(
               conf_info.conf.lookup_dictionary(conf_key_ens),
               conf_key_climo_mean,
               i_var, ens_valid_ut, grid,
               "ensemble climatology mean");

   mlog << Debug(4)
        << "Reading climatology standard deviation data for ensemble field \""
        << ens_info->get_var_info(i_ens)->magic_str() << "\".\n";

   csd_dp = read_climo_data_plane(
               conf_info.conf.lookup_dictionary(conf_key_ens),
               conf_key_climo_stdev,
               i_var, ens_valid_ut, grid,
               "ensemble climatology standard deviation");

   // Unset the MET_ENS_MEMBER_ID environment variable
   if(set_ens_mem_id) {
      unsetenv(met_ens_member_id);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

static void get_ens_mean_stdev(GenEnsProdVarInfo *ens_info,
                               DataPlane &emn_dp, DataPlane &esd_dp) {
   VarInfo *var_info;
   DataPlane ens_dp;

   // Check for null pointer
   if(!ens_info) {
      mlog << Error << "\nget_ens_mean_stdev() -> "
           << "null pointer!\n\n";
      exit(1);
   }

   mlog << Debug(2)
        << "Computing the ensemble mean and standard deviation for "
        << ens_info->raw_magic_str << ".\n";

   nxy = 0;
   vector<double> emn_cnt;
   vector<double> emn_sum;
   vector<double> esd_cnt;
   vector<double> esd_sum;
   vector<double> esd_ssq;

#pragma omp declare reduction(vec_double_plus : vector<double> :          \
                              transform(omp_out.begin(), omp_out.end(),   \
                                         omp_in.begin(), omp_out.begin(), \
                                        plus<double>()))                  \
                    initializer(omp_priv = decltype(omp_orig)(omp_orig.size()))

   // Loop over the ensemble inputs
   for(int i_ens=0; i_ens<ens_info->inputs_n(); i_ens++) {

      // Get file and VarInfo to process
      ConcatString ens_file(ens_info->get_file(i_ens));
      var_info = ens_info->get_var_info(i_ens);

      // Skip bad data files
      if(!ens_file_vld[ens_info->get_file_index(i_ens)]) continue;

      // Read data and track the valid data count
      if(!get_data_plane(ens_file.c_str(), etype,
                         var_info, ens_dp)) {
         mlog << Warning << "\nget_ens_mean_stdev() -> "
              << "ensemble field \"" << var_info->magic_str()
              << "\" not found in file \"" << ens_file << "\"\n\n";
         continue;
      }

      // Initialize sums, if needed
      if(emn_cnt.empty()) {
         nxy = ens_dp.nx()*ens_dp.ny();
         emn_cnt.resize(nxy, 0.0);
         emn_sum.resize(nxy, 0.0);
         esd_cnt.resize(nxy, 0.0);
         esd_sum.resize(nxy, 0.0);
         esd_ssq.resize(nxy, 0.0);
      }

#pragma omp parallel default(none) \
      shared(nxy, ens_dp, emn_cnt, emn_sum) \
      shared(esd_cnt, esd_sum, esd_ssq)
      {      

         // Update the counts and sums
#pragma omp for schedule(static) \
                reduction(vec_double_plus: emn_cnt, emn_sum) \
                reduction(vec_double_plus: esd_cnt, esd_sum, esd_ssq)
         for(int j=0; j<nxy; j++) {

            double ens = ens_dp.buf()[j];

            // Skip bad data
            if(is_bad_data(ens) || is_bad_data(emn_sum[j])) continue;

            // Update counts and sums
            emn_cnt[j] += 1;
            emn_sum[j] += ens;
            esd_cnt[j] += 1;
            esd_sum[j] += ens;
            esd_ssq[j] += ens*ens;

         } // end for j
      } // End omp parallel
   } // end for i_ens

   // Read ensemble control member data, if provided
   if(ctrl_file.nonempty()) {
      var_info = ens_info->get_ctrl(0);

      // Error out if missing
      if(!get_data_plane(ctrl_file.c_str(), etype,
                         var_info, ens_dp)) {
         mlog << Error << "\nget_ens_mean_stdev() -> "
              << "control member ensemble field \""
              << var_info->magic_str()
              << "\" not found in file \"" << ctrl_file << "\"\n\n";
         exit(1);
      }

#pragma omp parallel default(none) \
      shared(nxy, ens_dp, emn_cnt, emn_sum)
      {      

         // Update the counts and sums
#pragma omp for schedule(static) \
                reduction(vec_double_plus: emn_cnt, emn_sum)
         for(int j=0; j<nxy; j++) {

            double ens = ens_dp.buf()[j];

            // Skip bad data
            if(is_bad_data(ens) || is_bad_data(emn_sum[j])) continue;

            // Update counts and sums
            emn_cnt[j] += 1;
            emn_sum[j] += ens;

         } // end for j
      } // End omp parallel
   } // end if ctrl

   // Compute the ensemble mean and standard deviation
   emn_dp.set_size(ens_dp.nx(), ens_dp.ny());
   esd_dp.set_size(ens_dp.nx(), ens_dp.ny());

#pragma omp parallel default(none) \
   shared(nxy, emn_dp, esd_dp) \
   shared(emn_cnt, emn_sum) \
   shared(esd_cnt, esd_sum, esd_ssq)
   {      

#pragma omp for schedule(static)
      for(int j=0; j<nxy; j++) {

         // Ensemble mean
         if(is_bad_data(emn_sum[j]) ||
            is_eq(emn_cnt[j], 0.0)) {
            emn_dp.buf()[j] = bad_data_double;
         }
         else {
            emn_dp.buf()[j] = emn_sum[j] / emn_cnt[j];
         }

         // Ensemble standard deviation
         if(is_bad_data(esd_sum[j]) ||
            is_eq(esd_cnt[j], 0.0)) {
            esd_dp.buf()[j] = bad_data_double;
         }
         else {
            esd_dp.buf()[j] = compute_stdev(
                                 esd_sum[j], esd_ssq[j],
                                 nint(esd_cnt[j]));
         }
      } // end for j
   } // End omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////

static bool get_data_plane(const char *infile, GrdFileType ftype,
                           VarInfo *info, DataPlane &dp) {
   bool found;
   Met2dDataFile *mtddf = nullptr;

   // Read the current ensemble file
   if(!(mtddf = Met2dDataFileFactory::new_met_2d_data_file(infile, ftype))) {
      mlog << Error << "\nget_data_plane() -> "
           << "trouble reading file \"" << infile << "\"\n\n";
      exit(1);
   }

   // Read the gridded data field
   if((found = mtddf->data_plane(*info, dp))) {

      // Setup the verification grid, if necessary
      if(nxy == 0) process_grid(mtddf->grid());

      // Create the output file, if necessary
      if(nc_out == nullptr) setup_nc_file();

      // Regrid, if requested and necessary
      if(!(mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding field \"" << info->magic_str()
              << "\" to the verification grid using "
              << info->regrid().get_str() << ".\n";
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
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) nullptr; }

   return found;
}

////////////////////////////////////////////////////////////////////////

static void clear_counts() {

   cnt_na.set_const(0.0, nxy);
   min_na.set_const(bad_data_double, nxy);
   max_na.set_const(bad_data_double, nxy);
   sum_na.set_const(0.0, nxy);

   stdev_cnt_na.set_const(0.0, nxy);
   stdev_sum_na.set_const(0.0, nxy);
   stdev_ssq_na.set_const(0.0, nxy);

   for(int i=0; i<conf_info.get_max_n_cat(); i++) {
      thresh_cnt_na[i].set_const(0.0, nxy);
      for(int j=0; j<conf_info.get_n_nbrhd(); j++) {
         thresh_nbrhd_cnt_na[i][j].set_const(0.0, nxy);
      }
   }

   // Store ensemble data for EAS
   ens_eas_dp.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

static void track_counts(const GenEnsProdVarInfo *ens_info,
                         const DataPlane &ens_dp, bool is_ctrl,
                         const DataPlane &cmn_dp,
                         const DataPlane &csd_dp) {

   // Ensemble thresholds
   const int n_thr = ens_info->cat_ta.n();
   const SingleThresh *thr_buf = ens_info->cat_ta.buf();

   // Increment counts for each grid point
   for(int i=0; i<nxy; i++) {

      // Get current values
      double ens = ens_dp.data()[i];
      double cmn = (cmn_dp.is_empty() ? bad_data_double : cmn_dp.data()[i]);
      double csd = (csd_dp.is_empty() ? bad_data_double : csd_dp.data()[i]);

      // MET #2924 Use the same data for the forecast and observation climatologies
      ClimoPntInfo cpi(cmn, csd, cmn, csd);

      // Skip bad data values
      if(is_bad_data(ens)) continue;

      // Otherwise, update counts and sums
      else {

         // Valid data count
         cnt_na.buf()[i] += 1;

         // Ensemble sum
         sum_na.buf()[i] += ens;

         // Ensemble min and max
         if(ens <= min_na.buf()[i] || is_bad_data(min_na.buf()[i])) min_na.buf()[i] = ens;
         if(ens >= max_na.buf()[i] || is_bad_data(max_na.buf()[i])) max_na.buf()[i] = ens;

         // Standard deviation sum, sum of squares, and count, excluding control member
         if(!is_ctrl) {
            stdev_sum_na.buf()[i] += ens;
            stdev_ssq_na.buf()[i] += ens*ens;
            stdev_cnt_na.buf()[i] += 1;
         }

         // Event frequency
         for(int j=0; j<n_thr; j++) {
            if(thr_buf[j].check(ens, &cpi)) thresh_cnt_na[j].inc(i, 1);
         }
      } // end else
   } // end for i

   // Increment NMEP count anywhere fractional coverage > 0
   if(ens_info->nc_info.do_nmep) {
      DataPlane frac_dp;

      // Loop over thresholds
      for(int i=0; i<n_thr; i++) {

         // Loop over neighborhood sizes
         for(int j=0; j<conf_info.get_n_nbrhd(); j++) {

            // Compute fractional coverage
            fractional_coverage(ens_dp, frac_dp,
               conf_info.nbrhd_prob.width[j],
               conf_info.nbrhd_prob.shape, grid.wrap_lon(),
               thr_buf[i], &cmn_dp, &csd_dp, &cmn_dp, &csd_dp,
               conf_info.nbrhd_prob.vld_thresh);

            // Increment counts
            for(int k=0; k<nxy; k++) {
               if(frac_dp.data()[k] > 0) thresh_nbrhd_cnt_na[i][j].inc(k, 1);
            } // end for k

         } // end for j
      } // end for i
   } // end if do_nmep

   // Store ensemble data for EAS
   if(ens_info->nc_info.do_eas || ens_info->nc_info.do_eas_width) {
      ens_eas_dp.emplace_back(ens_dp);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

static void setup_nc_file() {

   // Create a new NetCDF file and open it
   nc_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file.text(), program_name,
                       conf_info.model.c_str());

   // Add the projection information
   write_netcdf_proj(nc_out, grid, lat_dim, lon_dim);

   // Add the lat/lon variables
   if(conf_info.ens_input[0]->nc_info.do_latlon) {
      write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

static void write_ens_nc(GenEnsProdVarInfo *ens_info, int n_ens_vld,
                         const DataPlane &ens_dp,
                         const DataPlane &cmn_dp,
                         const DataPlane &csd_dp) {

   // Allocate memory for storing ensemble data
   vector<float> ens_mean  (nxy);
   vector<float> ens_stdev (nxy);
   vector<float> ens_minus (nxy);
   vector<float> ens_plus  (nxy);
   vector<float> ens_min   (nxy);
   vector<float> ens_max   (nxy);
   vector<float> ens_range (nxy);
   vector<int  > ens_vld   (nxy);

   // Store the threshold for the ratio of valid data points
   double t = conf_info.vld_data_thresh;

#pragma omp parallel default(none) \
   shared(ens_vld, n_ens_vld, t) \
   shared(ens_mean, ens_stdev, ens_minus, ens_plus, ens_min, ens_max, ens_range) \
   shared(sum_na, cnt_na, min_na, max_na, stdev_sum_na, stdev_ssq_na, stdev_cnt_na)
   {

      // Store the data
#pragma omp for schedule(static)
      for(int i=0; i<cnt_na.n(); i++) {

         // Valid data count
         ens_vld[i] = nint(cnt_na[i]);

         // Check for too much missing data
         if((cnt_na[i]/n_ens_vld) < t) {
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
            ens_mean[i]  = (float) (sum_na[i]/cnt_na[i]);
            ens_stdev[i] = (float) compute_stdev(stdev_sum_na[i], stdev_ssq_na[i], nint(stdev_cnt_na[i]));
            ens_minus[i] =         ens_mean[i] - ens_stdev[i];
            ens_plus[i]  =         ens_mean[i] + ens_stdev[i];
            ens_min[i]   = (float) min_na[i];
            ens_max[i]   = (float) max_na[i];
            ens_range[i] = (is_eq(max_na[i], min_na[i]) ?
                            0.0 : (float) (max_na[i] - min_na[i]));
         }
      } // end for i
   } // End omp parallel

   // Add the ensemble mean, if requested
   if(ens_info->nc_info.do_mean) {
      write_ens_var_float(ens_info, ens_mean.data(), ens_dp,
                          "ENS_MEAN",
                          "Ensemble Mean");
   }

   // Add the ensemble standard deviation, if requested
   if(ens_info->nc_info.do_stdev) {
      write_ens_var_float(ens_info, ens_stdev.data(), ens_dp,
                          "ENS_STDEV",
                          "Ensemble Standard Deviation");
   }

   // Add the ensemble mean minus one standard deviation, if requested
   if(ens_info->nc_info.do_minus) {
      write_ens_var_float(ens_info, ens_minus.data(), ens_dp,
                          "ENS_MINUS",
                          "Ensemble Mean Minus 1 Standard Deviation");
   }

   // Add the ensemble mean plus one standard deviation, if requested
   if(ens_info->nc_info.do_plus) {
      write_ens_var_float(ens_info, ens_plus.data(), ens_dp,
                          "ENS_PLUS",
                          "Ensemble Mean Plus 1 Standard Deviation");
   }

   // Add the ensemble minimum value, if requested
   if(ens_info->nc_info.do_min) {
      write_ens_var_float(ens_info, ens_min.data(), ens_dp,
                          "ENS_MIN",
                          "Ensemble Minimum");
   }

   // Add the ensemble maximum value, if requested
   if(ens_info->nc_info.do_max) {
      write_ens_var_float(ens_info, ens_max.data(), ens_dp,
                          "ENS_MAX",
                          "Ensemble Maximum");
   }

   // Add the ensemble range, if requested
   if(ens_info->nc_info.do_range) {
      write_ens_var_float(ens_info, ens_range.data(), ens_dp,
                          "ENS_RANGE",
                          "Ensemble Range");
   }

   // Add the ensemble valid data count, if requested
   if(ens_info->nc_info.do_vld) {
      write_ens_var_int(ens_info, ens_vld.data(), ens_dp,
                        "ENS_VLD",
                        "Ensemble Valid Data Count");
   }

   // Add the ensemble relative frequencies and neighborhood probabilities, if requested
   if(ens_info->nc_info.do_freq ||
      ens_info->nc_info.do_nep) {

      DataPlane prob_dp;
      prob_dp.set_size(grid.nx(), grid.ny());

      // Loop through each threshold
      for(int i=0; i<ens_info->cat_ta.n(); i++) {

         // Initialize
         prob_dp.erase();

         // Compute the ensemble relative frequency
         for(int j=0; j<cnt_na.n(); j++) {
            prob_dp.buf()[j] = ((cnt_na[j]/n_ens_vld) < t ?
                                bad_data_double :
                                (thresh_cnt_na[i][j]/cnt_na[j]));
         }

         // Write ensemble relative frequency
         if(ens_info->nc_info.do_freq) {
            ConcatString type_str("ENS_FREQ_");
            type_str << ens_info->cat_ta[i].get_abbr_str().contents().c_str();
            write_ens_data_plane(ens_info, prob_dp, ens_dp, type_str.c_str(),
                                 "Ensemble Relative Frequency");
         }

         // Process the neighborhood ensemble probability
         if(ens_info->nc_info.do_nep) {
            GaussianInfo info;

            // Loop over the neighborhoods
            for(int j=0; j<conf_info.get_n_nbrhd(); j++) {

               DataPlane nbrhd_dp(smooth_field(prob_dp, InterpMthd::UW_Mean,
                                     conf_info.nbrhd_prob.width[j],
                                     conf_info.nbrhd_prob.shape, grid.wrap_lon(),
                                     conf_info.nbrhd_prob.vld_thresh, info));

               // Write neighborhood ensemble probability
               ConcatString type_str("ENS_NEP_");
               type_str << ens_info->cat_ta[i].get_abbr_str().contents().c_str() << "_"
                        << interpmthd_to_string(InterpMthd::Nbrhd).c_str()
                        << conf_info.nbrhd_prob.width[j]*conf_info.nbrhd_prob.width[j];
               write_ens_data_plane(ens_info, nbrhd_dp, ens_dp, type_str.c_str(),
                                    "Neighborhood Ensemble Probability");
            } // end for k
         } // end if do_nep
      } // end for i
   } // end if

   // Add the neighborhood maximum ensemble probabilities, if requested
   if(ens_info->nc_info.do_nmep) {

      DataPlane prob_dp;
      prob_dp.set_size(grid.nx(), grid.ny());

      // Loop through each threshold
      for(int i=0; i<ens_info->cat_ta.n(); i++) {

         // Loop through each neigbhorhood size
         for(int j=0; j<conf_info.get_n_nbrhd(); j++) {

            // Initialize
            prob_dp.erase();

            // Compute the neighborhood maximum ensemble probability
            for(int k=0; k<cnt_na.n(); k++) {
               prob_dp.buf()[k] = ((cnt_na[k]/n_ens_vld) < t ?
                                   bad_data_double :
                                   (thresh_nbrhd_cnt_na[i][j][k]/cnt_na[k]));
            }

            // Loop through the requested NMEP smoothers
            for(int k=0; k<conf_info.nmep_smooth.n_interp; k++) {

               DataPlane nbrhd_dp(smooth_field(prob_dp,
                                     string_to_interpmthd(conf_info.nmep_smooth.method[k].c_str()),
                                     conf_info.nmep_smooth.width[k],
                                     conf_info.nmep_smooth.shape, grid.wrap_lon(),
                                     conf_info.nmep_smooth.vld_thresh,
                                     conf_info.nmep_smooth.gaussian));

               // Write neighborhood maximum ensemble probability
               ConcatString type_str("ENS_NMEP_");
               type_str << ens_info->cat_ta[i].get_abbr_str().contents().c_str() << "_"
                        << interpmthd_to_string(InterpMthd::Nbrhd).c_str()
                        << conf_info.nbrhd_prob.width[j]*conf_info.nbrhd_prob.width[j] << "_"
                        << conf_info.nmep_smooth.method[k].c_str()
                        << conf_info.nmep_smooth.width[k]*conf_info.nmep_smooth.width[k];
               write_ens_data_plane(ens_info, nbrhd_dp, ens_dp, type_str.c_str(),
                                    "Neighborhood Maximum Ensemble Probability");
            } // end for k
         } // end for j
      } // end for i
   } // end if do_nmep

   // Compute and store EAS fractional coverage fields
   if(ens_info->nc_info.do_eas || ens_info->nc_info.do_eas_width) {
      write_eas_nc(ens_info, ens_dp, cmn_dp, csd_dp);
   }

   // Write the climo mean field, if requested
   if(ens_info->nc_info.do_climo && !cmn_dp.is_empty()) {
      write_ens_data_plane(ens_info, cmn_dp, ens_dp,
                           "CLIMO_MEAN",
                           "Climatology mean");
   }

   // Write the climo stdev field, if requested
   if(ens_info->nc_info.do_climo && !csd_dp.is_empty()) {
      write_ens_data_plane(ens_info, csd_dp, ens_dp,
                           "CLIMO_STDEV",
                           "Climatology standard deviation");
   }

   // Write the climo distribution percentile thresholds, if requested
   if(ens_info->nc_info.do_climo_cdp &&
      !cmn_dp.is_empty() && !csd_dp.is_empty()) {

      DataPlane cdp_dp;
      vector<Simple_Node> simp;
      ens_info->cat_ta.get_simple_nodes(simp);

      // Process all CDP thresholds except 0 and 100
      for(const auto &n : simp) {
         if(n.ptype() == perc_thresh_fcst_climo_dist &&
            !is_eq(n.pvalue(), 0.0) &&
            !is_eq(n.pvalue(), 100.0)) {
            ConcatString type_str("CLIMO_FCDP");
            type_str << nint(n.pvalue());
            cdp_dp = normal_cdf_inv(n.pvalue()/100.0, cmn_dp, csd_dp);
            write_ens_data_plane(ens_info, cdp_dp, ens_dp, type_str.c_str(),
                                 "Forecast climatology distribution percentile");
         }
         else if(n.ptype() == perc_thresh_obs_climo_dist &&
                 !is_eq(n.pvalue(), 0.0) &&
                 !is_eq(n.pvalue(), 100.0)) {
            ConcatString type_str("CLIMO_OCDP");
            type_str << nint(n.pvalue());
            cdp_dp = normal_cdf_inv(n.pvalue()/100.0, cmn_dp, csd_dp);
            write_ens_data_plane(ens_info, cdp_dp, ens_dp, type_str.c_str(),
                                 "Observation climatology distribution percentile");
         }
      } // end for
   }

   return;
}

////////////////////////////////////////////////////////////////////////

static void write_eas_nc(GenEnsProdVarInfo *ens_info,
                         const DataPlane &ens_dp,
                         const DataPlane &cmn_dp,
                         const DataPlane &csd_dp) {

   // Number of categorical thresholds
   const int n_thr = ens_info->cat_ta.n();

   // Loop through each categorical threshold
   for(int i_thr=0; i_thr<n_thr; i_thr++) {

      mlog << Debug(3) << "Computing ensemble agreement scale field "
           << "using the " << ens_info->cat_ta[i_thr].get_str()
           << " threshold and " << conf_info.get_n_eas()
           << " candidate widths (" << write_css(conf_info.eas_prob.width)
           << ").\n";

      // Apply threshold to convert to bitmap
      vector<DataPlane> ens_thresh_dp;
      for(const auto &x : ens_eas_dp) {
         DataPlane dp(x);
         dp.threshold(ens_info->cat_ta[i_thr], &cmn_dp, &csd_dp);
         ens_thresh_dp.emplace_back(dp);
      }

      // Compute Gaussian weights
      if(conf_info.eas_prob.gaussian.weights.empty()) {
         conf_info.eas_prob.gaussian.compute();
      }

      // Compute EAS probabilities
      DataPlane eas_prob_dp;
      DataPlane eas_width_dp;
      compute_eas(ens_thresh_dp, conf_info.eas_prob, grid,
                  eas_prob_dp, eas_width_dp);

      ConcatString type_cs;
      ConcatString thr_cs(ens_info->cat_ta[i_thr].get_abbr_str());

      // Write EAS probabilities
      if(ens_info->nc_info.do_eas) {
         type_cs << cs_erase << "ENS_EAS_" << thr_cs;
         write_ens_data_plane(ens_info, eas_prob_dp, ens_dp, type_cs.c_str(),
                              "Ensemble Agreement Scale Probability");
      }

      // Write EAS widths
      if(ens_info->nc_info.do_eas_width) {
         type_cs << cs_erase << "ENS_EAS_WIDTH_" << thr_cs;
         write_ens_data_plane(ens_info, eas_width_dp, ens_dp, type_cs.c_str(),
                              "Ensemble Agreement Scale Width");
      }
   } // end for i_thr
}

////////////////////////////////////////////////////////////////////////

static void write_ens_var_float(GenEnsProdVarInfo *ens_info,
                                const float *ens_data,
                                const DataPlane &dp,
                                const char *type_str,
                                const char *long_name_str) {

   // Append nc_var_str config file entry, if specified
   ConcatString var_str;
   ConcatString cs(ens_info->nc_var_str);
   if(!cs.empty()) var_str << "_" << cs;

   // Construct the variable name
   ConcatString ens_var_name;
   ens_var_name << ens_info->get_var_info()->name_attr() << "_"
                << ens_info->get_var_info()->level_attr()
                << var_str << "_" << type_str;

   // Skip variable names that have already been written
   if(nc_ens_var_sa.has(ens_var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_ens_var_sa.add(ens_var_name);

   auto ens_var = add_var(nc_out, (string) ens_var_name,
                          ncFloat, lat_dim, lon_dim);

   //
   // Construct the variable name attribute
   // For the ensemble mean, just use the variable name.
   // For all other fields, append the field type.
   //
   ConcatString name_str;
   if(strcmp(type_str, "ENS_MEAN") == 0) {
      name_str << cs_erase
               << ens_info->get_var_info()->name_attr();
   }
   else {
      name_str << cs_erase
               << ens_info->get_var_info()->name_attr() << "_"
               << type_str;
   }

   // Add the variable attributes
   add_var_att_local(ens_info, &ens_var, false, dp,
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

static void write_ens_var_int(GenEnsProdVarInfo *ens_info,
                              const int *ens_data,
                              const DataPlane &dp,
                              const char *type_str,
                              const char *long_name_str) {

   // Append nc_var_str config file entry, if specified
   ConcatString var_str;
   ConcatString cs(ens_info->nc_var_str);
   if(!cs.empty()) var_str << "_" << cs;

   // Construct the variable name
   ConcatString ens_var_name;
   ens_var_name << ens_info->get_var_info()->name_attr() << "_"
                << ens_info->get_var_info()->level_attr()
                << var_str << "_" << type_str;

   // Skip variable names that have already been written
   if(nc_ens_var_sa.has(ens_var_name)) return;

   // Otherwise, add to the list of previously defined variables
   nc_ens_var_sa.add(ens_var_name);

   int deflate_level = conf_info.get_compression_level();
   auto ens_var = add_var(nc_out, (string) ens_var_name,
                          ncInt, lat_dim, lon_dim, deflate_level);

   // Construct the variable name attribute
   ConcatString name_str;
   name_str << ens_info->get_var_info()->name_attr() << "_"
            << type_str;

   // Add the variable attributes
   add_var_att_local(ens_info, &ens_var, true, dp,
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

static void write_ens_data_plane(GenEnsProdVarInfo *ens_info,
                                 const DataPlane &ens_dp,
                                 const DataPlane &dp,
                                 const char *type_str,
                                 const char *long_name_str) {

   // Copy the data to float
   vector<float> ens_data(ens_dp.const_buf().size());
   copy(ens_dp.const_buf().begin(), ens_dp.const_buf().end(),
        ens_data.begin());

   // Write the output
   write_ens_var_float(ens_info, ens_data.data(), dp,
                       type_str, long_name_str);

   return;
}

////////////////////////////////////////////////////////////////////////

static void add_var_att_local(GenEnsProdVarInfo *ens_info,
                              NcVar *nc_var, bool is_int,
                              const DataPlane &dp,
                              const char *name_str,
                              const char *long_name_str) {
   ConcatString att_str;
   const VarInfo *info = ens_info->get_var_info();

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

static void clean_up() {

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // List the output NetCDF file
   mlog << Debug(1) << "Output file: " << out_file << "\n";

   // Close the output NetCDF file
   if(nc_out) { delete nc_out; nc_out = (NcFile *) nullptr; }

   // Clear the threshold count arrays
   thresh_cnt_na.clear();
   thresh_nbrhd_cnt_na.clear();
   ens_eas_dp.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

__attribute__((noreturn)) static void usage(int exit_code) {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-ens file_1 ... file_n | file_list\n"
        << "\t-out file\n"
        << "\t-config file\n"
        << "\t[-ctrl file]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-ens file_1 ... file_n\" is a list of ensemble "
        << "member file names (required).\n"

        << "\t\t\"-ens file_list\" is an ASCII file containing a list "
        << "of ensemble member file names (required).\n"

        << "\t\t\"-out file\" is the NetCDF output file for the derived "
        << "ensemble products (required).\n"

        << "\t\t\"-config file\" is a GenEnsProdConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-ctrl file\" is the gridded ensemble control data file "
        << "included in the mean but excluded from the spread (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n";

   exit (exit_code);
}

////////////////////////////////////////////////////////////////////////

static void set_ens_files(const StringArray & a) {
   ens_files.add(parse_file_list(a));
}

////////////////////////////////////////////////////////////////////////

static void set_out_file(const StringArray & a) {
   out_file = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_config_file(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_ctrl_file(const StringArray & a) {
   ctrl_file = a[0];
}

////////////////////////////////////////////////////////////////////////
