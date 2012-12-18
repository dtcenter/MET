// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   series_analysis.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/10/12  Halley Gotway   New
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
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "series_analysis.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_scores();
static void get_series_data(int, VarInfo *, VarInfo *,
                            DataPlane &, DataPlane &);
static void get_series_entry(int, VarInfo *, const ConcatString &,
                             const GrdFileType, DataPlane &);
static void get_series_entry(int, VarInfo *, const StringArray &,
                             const GrdFileType, DataPlane &);


// static void setup_first_pass(const DataPlane &);
// static void setup_txt_files (unixtime, int);
// static void setup_table     (AsciiTable &);
// static void setup_nc_file   (unixtime, int);
// 
// static void build_outfile_name(unixtime, int, const char *,
//                                ConcatString &);
// 
// static void do_cts   (CTSInfo *&, int,
//                       const NumArray &, const NumArray &);
// static void do_mcts  (MCTSInfo &, int,
//                       const NumArray &, const NumArray &);
// static void do_cnt   (CNTInfo &, int,
//                       const NumArray &, const NumArray &);
// static void do_vl1l2 (VL1L2Info *&, int,
//                       const NumArray &, const NumArray &,
//                       const NumArray &, const NumArray &);
// static void do_pct   (PCTInfo   *&, int,
//                       const NumArray &, const NumArray &);
// static void do_nbrcts(NBRCTSInfo *&, int, int, int,
//                       const NumArray &, const NumArray &);
// static void do_nbrcnt(NBRCNTInfo &, int, int, int,
//                       const NumArray &, const NumArray &);
// 
// static void write_nc(const DataPlane &, const DataPlane &,
//                      int, InterpMthd, int);
// static void add_var_att(NcVar *, const char *, const char *);
// 
// static void finish_txt_files();
// 
static void clean_up();

static void usage();
static void set_fcst_files(const StringArray &);
static void set_obs_files(const StringArray &);
static void set_both_files(const StringArray &);
static void set_out_file(const StringArray &);
static void set_config_file(const StringArray &);
static void set_log_file(const StringArray &);
static void set_verbosity(const StringArray &);
static void parse_file_list(const StringArray &, StringArray &);
static void parse_ascii_file_list(const char *, StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_fcst_files,  "-fcst",  -1);
   cline.add(set_obs_files,   "-obs",   -1);
   cline.add(set_both_files,  "-both",  -1);
   cline.add(set_config_file, "-config", 1);
   cline.add(set_out_file,    "-out",    1);
   cline.add(set_log_file,    "-log",    1);
   cline.add(set_verbosity,   "-v",      1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be zero arguments left.
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set.
   if(fcst_files.n_elements() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the forecast file list must be set using the "
           << "\"-fcst\" or \"-both\" option.\n\n";
      usage();
   }
   if(obs_files.n_elements() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the observation file list must be set using the "
           << "\"-obs\" or \"-both\" option.\n\n";
      usage();
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      usage();
   }
   if(out_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the output NetCDF file must be set using the "
           << "\"-out\" option.\n\n";
      usage();
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file, config_file);

   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_files[0], ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_files[0] << "\"\n\n";
      exit(1);
   }

   // Read observation file
   if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(obs_files[0], otype))) {
      mlog << Error << "\nTrouble reading observation file \""
           << obs_files[0] << "\"\n\n";
      exit(1);
   }

   // Store the input data file types
   ftype = fcst_mtddf->file_type();
   otype = obs_mtddf->file_type();

   // Process the configuration
   conf_info.process_config(ftype, otype);

   // Check that the grids match
   if(!(fcst_mtddf->grid() == obs_mtddf->grid())) {

      mlog << Error << "\nprocess_command_line() -> "
           << "The forecast and observation grids do not match: "
           << fcst_mtddf->grid().serialize() << " != "
           << obs_mtddf->grid().serialize() << "\n\n";
      exit(1);
   }
   // If they do, store the grid
   else {
      grid = fcst_mtddf->grid();
      conf_info.process_masks(grid);
   }

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.boot_rng, conf_info.boot_seed);

   // List the lengths of the series options
   mlog << Debug(1)
        << "Length of configuration \"fcst.field\" = " << conf_info.get_n_fcst() << "\n"
        << "Length of configuration \"obs.field\"  = " << conf_info.get_n_obs() << "\n"
        << "Length of forecast file list         = " << fcst_files.n_elements() << "\n"
        << "Length of observation file list      = " << obs_files.n_elements() << "\n";

   // Deteremine the length of the series to be analyzed.  Series is
   // defined by the first parameter of length greater than one:
   // - Configuration fcst.field
   // - Configuration obs.field
   // - Forecast file list
   // - Observation file list
   if(conf_info.get_n_fcst() > 1) {
      series_type = SeriesType_Fcst_Conf;
      n_series = conf_info.get_n_fcst();
      mlog << Debug(1)
           << "Series defined by the \"fcst.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(conf_info.get_n_obs() > 1) {
      series_type = SeriesType_Obs_Conf;
      n_series = conf_info.get_n_obs();
      mlog << Debug(1)
           << "Series defined by the \"obs.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(fcst_files.n_elements() > 1) {
      series_type = SeriesType_Fcst_Files;
      n_series = fcst_files.n_elements();
      mlog << Debug(1)
           << "Series defined by the forecast file list of length "
           << n_series << ".\n";
   }
   else if(obs_files.n_elements() > 1) {
      series_type = SeriesType_Obs_Files;
      n_series = obs_files.n_elements();
      mlog << Debug(1)
           << "Series defined by the observation file list of length "
           << n_series << ".\n";
   }
   else {
      mlog << Error << "\nprocess_command_line() -> "
           << "cannot define series when the \"fcst.field\" and "
           << "\"obs.field\" configuration entries and the \"-fcst\" and "
           << "\"-obs\" command line options all have length one.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

// JHG, should detect if this is a time series

void process_scores() {
   int nxny, i, x, y, i_read, n_reads, i_series, i_point;
   VarInfo *fcst_info = (VarInfo *) 0;
   VarInfo *obs_info  = (VarInfo *) 0;
   NumArray *f_na = (NumArray *) 0;
   NumArray *o_na = (NumArray *) 0;
   DataPlane fcst_dp, obs_dp;

   // Determine the block size
   nxny    = grid.nx() * grid.ny();
   n_reads = ceil((double) nxny / conf_info.block_size);

   // Allocate space to store the pairs for each grid point
   f_na = new NumArray [conf_info.block_size];
   o_na = new NumArray [conf_info.block_size];

   mlog << Debug(2)
        << "Computing statistics using a block size of "
        << conf_info.block_size << ", requiring " << n_reads
        << " passes through the " << grid.nx() << " x "
        << grid.ny() << " grid.\n";

   // Loop over the data reads
   for(i_read=0; i_read<n_reads; i_read++) {
     
      // Starting grid point
      i_point = i_read*conf_info.block_size;

      mlog << Debug(3)
           << "Processing data pass number " << i_read + 1 << " of "
           << n_reads << " for grid points " << i_point + 1 << " to "
           << i_point + conf_info.block_size << ".\n";
      
      // Loop over the series variable
      for(i_series=0; i_series<n_series; i_series++) {

         // Store the current VarInfo objects
         fcst_info = (conf_info.get_n_fcst() > 1 ?
                      conf_info.fcst_info[i_series] :
                      conf_info.fcst_info[0]);
         obs_info  = (conf_info.get_n_obs() > 1 ?
                      conf_info.obs_info[i_series] :
                      conf_info.obs_info[0]);

         // Retrieve the data planes for the current series entry
         get_series_data(i_series, fcst_info, obs_info, fcst_dp, obs_dp);
         
         // Store matched pairs for each grid point
         for(i=0; i<conf_info.block_size && (i_point+i)<nxny; i++) {

            // Convert n to x, y
            fcst_dp.one_to_two(i_point + i, x, y);

            // Store valid fcst/obs pairs where the mask is on
            if(!is_bad_data(fcst_dp(x,y)) &&
               !is_bad_data(obs_dp(x,y)) &&
               !is_bad_data(conf_info.mask_dp(x,y))) {
               f_na[i].add(fcst_dp(x, y));
               o_na[i].add(obs_dp(x, y));
            }

         } // end for i

      } // end for i_series

      // Compute statistics for each grid point in the block
      for(i=0; i<conf_info.block_size && (i_point+i)<nxny; i++) {

         // JHG, work here!
         fcst_dp.one_to_two(i_point + i, x, y);
         mlog << Debug(1) << "[" << i+1 << " of "
              << conf_info.block_size << " (" << x << "," << y
              << ")] mean(f_na) = " << f_na[i].mean()
              << ", mean(o_na) = " << o_na[i].mean() << "\n";
      }

      // Empty out the NumArray objects
      for(i=0; i<conf_info.block_size; i++) {
         f_na[i].empty();
         o_na[i].empty();
      }
     
   } // end for i_read

   // Deallocate and clean up
   if(f_na) { delete [] f_na; f_na = (NumArray *) 0; }
   if(o_na) { delete [] o_na; o_na = (NumArray *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_series_data(int i_series,
                     VarInfo *fcst_info, VarInfo *obs_info,
                     DataPlane &fcst_dp, DataPlane &obs_dp) {
  
   mlog << Debug(3)
        << "Processing series entry " << i_series + 1 << " of "
        << n_series << ": " << fcst_info->magic_str()
        << " versus " << obs_info->magic_str() << "\n";
  
   // Switch on the series type
   switch(series_type) {

      case SeriesType_Fcst_Conf:
         get_series_entry(i_series, fcst_info, fcst_files, ftype, fcst_dp);
         if(conf_info.get_n_obs() == 1) {
            obs_info->set_valid(fcst_dp.valid());
         }
         get_series_entry(i_series, obs_info, obs_files, otype, obs_dp);
         break;

      case SeriesType_Obs_Conf:
         get_series_entry(i_series, obs_info, obs_files, otype, obs_dp);
         if(conf_info.get_n_fcst() == 1) {
            fcst_info->set_valid(obs_dp.valid());
         }
         get_series_entry(i_series, fcst_info, fcst_files, ftype, fcst_dp);
         break;
         
      case SeriesType_Fcst_Files:
         get_series_entry(i_series, fcst_info, fcst_files[i_series], ftype, fcst_dp);
         obs_info->set_valid(fcst_dp.valid());
         get_series_entry(i_series, obs_info, obs_files, otype, obs_dp);
         break;

      case SeriesType_Obs_Files:
         get_series_entry(i_series, obs_info, obs_files[i_series], otype, obs_dp);
         fcst_info->set_valid(obs_dp.valid());
         get_series_entry(i_series, fcst_info, fcst_files, ftype, fcst_dp);
         break;

      default:
         mlog << Error << "\nget_series_data() -> "
              << "unexpected SeriesType value: "
              << series_type << "\n\n";
         exit(1);
         break;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_series_entry(int i_series, VarInfo *info,
                      const ConcatString &file, const GrdFileType type,
                      DataPlane &dp) {
   StringArray sa;

   sa.add(file);

   get_series_entry(i_series, info, sa, type, dp);

   return;
}

////////////////////////////////////////////////////////////////////////

// JHG, implement an optional quiet argument for data_plane() in
// Met2dDataFile heirarchy

void get_series_entry(int i_series, VarInfo *info,
                      const StringArray &files, const GrdFileType type,
                      DataPlane &dp) {
   int i, j;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   bool found = false;

   // Search for match in the file list
   for(i=0; i<files.n_elements(); i++) {

      // Start the search with the value of i_series
      j = (i_series + i) % files.n_elements();
     
      // Open the data file
      mtddf = mtddf_factory.new_met_2d_data_file(files[j], type);

      // Check that the grid does not change
      if(mtddf->grid() != grid) {
         mlog << Error << "\nget_series_entry() -> "
              << "the grid has changed in file \"" << files[j] << "\":\n"
              << "Old: " << grid.serialize() << "\n"
              << "New: " << mtddf->grid().serialize() << "\n\n";
         exit(1);
      }
      
      // Attempt to read the gridded data from the current file
      found = mtddf->data_plane(*info, dp);
      
      // Close the data file
      delete mtddf; mtddf = (Met2dDataFile *) 0;

      // Break out of the loop if found
      if(found) {
         mlog << Debug(3)
              << "Found data for " << info->magic_str()
              << " in " << files[j] << "\n";
         break;
      }
   }

   // Check if the data plane was found
   if(!found) {
      mlog << Error << "\nget_series_entry() -> "
           << "Could not find data for " << info->magic_str()
           << " in file list:\n";
      for(i=0; i<files.n_elements(); i++)
         mlog << Error << "   " << files[i] << "\n";
      mlog << Error << "\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
/*
void process_scores() {
   int i, j, k, m, n;
   bool status;
   int max_scal_t, max_prob_t, wind_t, frac_t;

   DataPlane fcst_dp,        obs_dp;
   DataPlane fcst_dp_smooth, obs_dp_smooth;

   NumArray f_na, o_na;

   // Objects to handle vector winds
   DataPlane fcst_u_wind_dp,        obs_u_wind_dp;
   DataPlane fcst_u_wind_dp_smooth, obs_u_wind_dp_smooth;
   NumArray f_u_wind_na, o_u_wind_na;

   CNTInfo     cnt_info;
   CTSInfo    *cts_info = (CTSInfo    *) 0;
   MCTSInfo    mcts_info;
   VL1L2Info  *vl1l2_info = (VL1L2Info  *) 0;
   NBRCNTInfo  nbrcnt_info;
   NBRCTSInfo *nbrcts_info = (NBRCTSInfo *) 0;
   PCTInfo    *pct_info = (PCTInfo    *) 0;

   // Allocate enough space for the CTSInfo objects
   max_scal_t  = conf_info.get_max_n_scal_thresh();
   cts_info    = new CTSInfo [max_scal_t];

   // Allocate enough space for the VL1L2Info objects
   wind_t      = conf_info.get_n_wind_thresh();
   vl1l2_info  = new VL1L2Info [wind_t];

   // Allocate enough space for the NBRCTS objects
   frac_t      = conf_info.nbrhd_cov_ta.n_elements();
   nbrcts_info = new NBRCTSInfo [frac_t];

   // Allocate enough space for the PCTInfo objects
   max_prob_t  = conf_info.get_max_n_prob_obs_thresh();
   pct_info    = new PCTInfo [max_prob_t];

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Read the gridded data from the input forecast file
      status = fcst_mtddf->data_plane(*conf_info.fcst_info[i], fcst_dp);

      if(!status) {
         mlog << Warning << "\nprocess_scores() -> "
              << conf_info.fcst_info[i]->magic_str()
              << " not found in file: " << fcst_file
              << "\n\n";
         continue;
      }

      // For probability fields, check to see if they need to be
      // rescaled from [0, 100] to [0, 1]
      if(conf_info.fcst_info[i]->p_flag()) rescale_probability(fcst_dp);

      // Set the forecast lead time
      shc.set_fcst_lead_sec(fcst_dp.lead());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(fcst_dp.valid());
      shc.set_fcst_valid_end(fcst_dp.valid());

      // Read the gridded data from the input observation file
      status = obs_mtddf->data_plane(*conf_info.obs_info[i], obs_dp);

      if(!status) {
         mlog << Warning << "\nprocess_scores() -> "
              << conf_info.obs_info[i]->magic_str()
              << " not found in file: " << obs_file
              << "\n\n";
         continue;
      }

      // Set the observation lead time
      shc.set_obs_lead_sec(obs_dp.lead());

      // Set the observation valid time
      shc.set_obs_valid_beg(obs_dp.valid());
      shc.set_obs_valid_end(obs_dp.valid());

      // Check that the valid times match
      if(fcst_dp.valid() != obs_dp.valid()) {

         mlog << Warning << "\nprocess_scores() -> "
              << "Forecast and observation valid times do not match "
              << unix_to_yyyymmdd_hhmmss(fcst_dp.valid()) << " != "
              << unix_to_yyyymmdd_hhmmss(obs_dp.valid()) << " for "
              << conf_info.fcst_info[i]->magic_str() << " versus "
              << conf_info.obs_info[i]->magic_str() << ".\n\n";
      }

      // Check that the accumulation intervals match
      if(conf_info.fcst_info[i]->level().type() == LevelType_Accum &&
         conf_info.obs_info[i]->level().type()  == LevelType_Accum &&
         fcst_dp.accum() != obs_dp.accum()) {

         mlog << Warning << "\nprocess_scores() -> "
              << "Forecast and observation accumulation times "
              << "do not match " << sec_to_hhmmss(fcst_dp.accum())
              << " != " << sec_to_hhmmss(obs_dp.accum())
              << " for " << conf_info.fcst_info[i]->magic_str() << " versus "
              << conf_info.obs_info[i]->magic_str() << ".\n\n";
      }

      // Setup the first pass through the data
      if(is_first_pass) setup_first_pass(fcst_dp);

      // Setup the obtype column
      if(conf_info.fcst_info[i]->is_precipitation() &&
         conf_info.obs_info[i]->is_precipitation()) {
         shc.set_msg_typ("MC_PCP");
      }
      else {
         shc.set_msg_typ("ANALYS");
      }

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.fcst_info[i]->name());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.fcst_info[i]->level_name());

      // Store the observation variable name
      shc.set_obs_var(conf_info.obs_info[i]->name());

      // Set the observation level name
      shc.set_obs_lev(conf_info.obs_info[i]->level_name());

      mlog << Debug(2) << "\n" << sep_str << "\n\n";

      // If verifying vector winds, store the U-wind fields
      if(conf_info.fcst_info[i]->is_u_wind() &&
         conf_info.obs_info[i]->is_u_wind() &&
         conf_info.fcst_info[i]->v_flag() &&
         conf_info.obs_info[i]->v_flag()) {

         fcst_u_wind_dp = fcst_dp;
         obs_u_wind_dp  = obs_dp;
      }

      // Loop through and apply each of the smoothing operations
      for(j=0; j<conf_info.get_n_interp(); j++) {

         // Store the interpolation method and width being applied
         shc.set_interp_mthd(conf_info.interp_mthd[j]);
         shc.set_interp_wdth(conf_info.interp_wdth[j]);

         // If requested in the config file, smooth the forecast field
         if(conf_info.interp_field == FieldType_Fcst ||
            conf_info.interp_field == FieldType_Both) {
            smooth_field(fcst_dp, fcst_dp_smooth,
                         conf_info.interp_mthd[j],
                         conf_info.interp_wdth[j],
                         conf_info.interp_thresh);
         }
         // Do not smooth the forecast field
         else {
            fcst_dp_smooth = fcst_dp;
         }

         // If requested in the config file, smooth the observation field
         if(conf_info.interp_field == FieldType_Obs ||
            conf_info.interp_field == FieldType_Both) {
            smooth_field(obs_dp, obs_dp_smooth,
                         conf_info.interp_mthd[j],
                         conf_info.interp_wdth[j],
                         conf_info.interp_thresh);
         }
         // Do not smooth the observation field
         else {
            obs_dp_smooth = obs_dp;
         }

         // Loop through the masks to be applied
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Apply the current mask to the fields
            apply_mask(fcst_dp_smooth, obs_dp_smooth,
                       conf_info.mask_dp[k],
                       f_na, o_na);

            // Set the mask name
            shc.set_mask(conf_info.mask_name[k]);

            mlog << Debug(2) << "Processing "
                 << conf_info.fcst_info[i]->magic_str()
                 << " versus "
                 << conf_info.obs_info[i]->magic_str()
                 << ", for interpolation method "
                 << shc.get_interp_mthd_str() << "("
                 << shc.get_interp_pnts_str()
                 << "), over region " << shc.get_mask()
                 << ", using " << f_na.n_elements() << " pairs.\n";

            // Continue if no pairs were found
            if(f_na.n_elements() == 0) continue;

            // Compute CTS scores
            if(!conf_info.fcst_info[i]->p_flag()      &&
                conf_info.fcst_ta[i].n_elements() > 0 &&
               (conf_info.output_flag[i_fho] != STATOutputType_None ||
                conf_info.output_flag[i_ctc] != STATOutputType_None ||
                conf_info.output_flag[i_cts] != STATOutputType_None)) {

               // Initialize
               for(m=0; m<max_scal_t; m++) cts_info[m].clear();

               // Compute CTS
               do_cts(cts_info, i, f_na, o_na);

               // Loop through all of the thresholds
               for(m=0; m<conf_info.fcst_ta[i].n_elements(); m++) {

                  // Write out FHO
                  if(conf_info.output_flag[i_fho] != STATOutputType_None &&
                     cts_info[m].cts.n() > 0) {

                     write_fho_row(shc, cts_info[m],
                        conf_info.output_flag[i_fho] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_fho], i_txt_row[i_fho]);
                  }

                  // Write out CTC
                  if(conf_info.output_flag[i_ctc] != STATOutputType_None &&
                     cts_info[m].cts.n() > 0) {

                     write_ctc_row(shc, cts_info[m],
                        conf_info.output_flag[i_ctc] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_ctc], i_txt_row[i_ctc]);
                  }

                  // Write out CTS
                  if(conf_info.output_flag[i_cts] != STATOutputType_None &&
                     cts_info[m].cts.n() > 0) {

                     write_cts_row(shc, cts_info[m],
                        conf_info.output_flag[i_cts] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_cts], i_txt_row[i_cts]);
                  }
               } // end for m
            } // end Compute CTS

            // Compute MCTS scores
            if(!conf_info.fcst_info[i]->p_flag()      &&
                conf_info.fcst_ta[i].n_elements() > 1 &&
               (conf_info.output_flag[i_mctc] != STATOutputType_None ||
                conf_info.output_flag[i_mcts] != STATOutputType_None)) {

               // Initialize
               mcts_info.clear();

               // Compute MCTS
               do_mcts(mcts_info, i, f_na, o_na);

               // Write out MCTC
               if(conf_info.output_flag[i_mctc] != STATOutputType_None &&
                  mcts_info.cts.total() > 0) {

                  write_mctc_row(shc, mcts_info,
                     conf_info.output_flag[i_mctc] == STATOutputType_Both,
                     stat_at, i_stat_row,
                     txt_at[i_mctc], i_txt_row[i_mctc]);
               }

               // Write out MCTS
               if(conf_info.output_flag[i_mcts] != STATOutputType_None &&
                  mcts_info.cts.total() > 0) {

                  write_mcts_row(shc, mcts_info,
                     conf_info.output_flag[i_mcts] == STATOutputType_Both,
                     stat_at, i_stat_row,
                     txt_at[i_mcts], i_txt_row[i_mcts]);
               }
            } // end Compute MCTS

            // Compute CNT scores
            if(!conf_info.fcst_info[i]->p_flag() &&
               (conf_info.output_flag[i_cnt] != STATOutputType_None ||
                conf_info.output_flag[i_sl1l2] != STATOutputType_None)) {

               // Initialize
               cnt_info.clear();

               // Compute CNT
               do_cnt(cnt_info, i, f_na, o_na);

               // Write out CNT
               if(conf_info.output_flag[i_cnt] != STATOutputType_None &&
                  cnt_info.n > 0) {

                  write_cnt_row(shc, cnt_info,
                     conf_info.output_flag[i_cnt] == STATOutputType_Both,
                     stat_at, i_stat_row,
                     txt_at[i_cnt], i_txt_row[i_cnt]);
               }

               // Write out SL1L2 as long as the vflag is not set
               if(!conf_info.fcst_info[i]->v_flag() &&
                  conf_info.output_flag[i_sl1l2] != STATOutputType_None &&
                  cnt_info.n > 0) {

                  write_sl1l2_row(shc, cnt_info,
                     conf_info.output_flag[i_sl1l2] == STATOutputType_Both,
                     stat_at, i_stat_row,
                     txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
               }

            } // end Compute CNT

            // Compute VL1L2 partial sums for UGRD,VGRD
            if(!conf_info.fcst_info[i]->p_flag() &&
                conf_info.fcst_info[i]->v_flag() &&
               conf_info.output_flag[i_vl1l2] != STATOutputType_None &&
               i > 0 &&
               conf_info.fcst_info[i]->is_v_wind() &&
               conf_info.fcst_info[i-1]->is_u_wind()) {

               // Store the forecast variable name
               shc.set_fcst_var(ugrd_vgrd_abbr_str);

               // Store the observation variable name
               shc.set_obs_var(ugrd_vgrd_abbr_str);

               // Initialize
               for(m=0; m<wind_t; m++) vl1l2_info[m].clear();

               // Apply current smoothing method to the UGRD fields
               smooth_field(fcst_u_wind_dp, fcst_u_wind_dp_smooth,
                            conf_info.interp_mthd[j],
                            conf_info.interp_wdth[j],
                            conf_info.interp_thresh);

                smooth_field(obs_u_wind_dp, obs_u_wind_dp_smooth,
                             conf_info.interp_mthd[j],
                             conf_info.interp_wdth[j],
                             conf_info.interp_thresh);

               // Apply the current mask to the UGRD fields
               apply_mask(fcst_u_wind_dp_smooth, obs_u_wind_dp_smooth,
                          conf_info.mask_dp[k],
                          f_u_wind_na, o_u_wind_na);

               // Compute VL1L2
               do_vl1l2(vl1l2_info, i,
                        f_na, o_na, f_u_wind_na, o_u_wind_na);

               // Loop through all of the wind speed thresholds
               for(m=0; m<conf_info.fcst_wind_ta.n_elements(); m++) {

                  // Write out VL1L2
                  if(conf_info.output_flag[i_vl1l2] != STATOutputType_None &&
                     vl1l2_info[m].vcount > 0) {

                     write_vl1l2_row(shc, vl1l2_info[m],
                        conf_info.output_flag[i_vl1l2] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_vl1l2], i_txt_row[i_vl1l2]);
                  }
               } // end for m

               // Reset the forecast variable name
               shc.set_fcst_var(conf_info.fcst_info[i]->name());

               // Reset the observation variable name
               shc.set_obs_var(conf_info.obs_info[i]->name());

            } // end Compute VL1L2

            // Compute PCT counts and scores
            if(conf_info.fcst_info[i]->p_flag() &&
               (conf_info.output_flag[i_pct] != STATOutputType_None  ||
                conf_info.output_flag[i_pstd] != STATOutputType_None ||
                conf_info.output_flag[i_pjc] != STATOutputType_None  ||
                conf_info.output_flag[i_prc] != STATOutputType_None)) {

               // Initialize
               for(m=0; m<max_prob_t; m++) pct_info[m].clear();

               // Compute PCT
               do_pct(pct_info, i, f_na, o_na);

               // Loop through all of the thresholds
               for(m=0; m<max_prob_t; m++) {

                  // Write out PCT
                  if(conf_info.output_flag[i_pct] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_pct_row(shc, pct_info[m],
                        conf_info.output_flag[i_pct] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_pct], i_txt_row[i_pct]);
                  }

                  // Write out PSTD
                  if(conf_info.output_flag[i_pstd] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_pstd_row(shc, pct_info[m],
                        conf_info.output_flag[i_pstd] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_pstd], i_txt_row[i_pstd]);
                  }

                  // Write out PJC
                  if(conf_info.output_flag[i_pjc] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_pjc_row(shc, pct_info[m],
                        conf_info.output_flag[i_pjc] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_pjc], i_txt_row[i_pjc]);
                  }

                  // Write out PRC
                  if(conf_info.output_flag[i_prc] != STATOutputType_None &&
                     pct_info[m].pct.n() > 0) {

                     write_prc_row(shc, pct_info[m],
                        conf_info.output_flag[i_prc] == STATOutputType_Both,
                        stat_at, i_stat_row,
                        txt_at[i_prc], i_txt_row[i_prc]);
                  }
               }
            } // end Compute PCT

         } // end for k

         // Write out the smoothed forecast, observation, and difference
         // fields in netCDF format for each GRIB code if requested in
         // the config file
         if(conf_info.nc_pairs_flag)
            write_nc(fcst_dp_smooth, obs_dp_smooth, i,
                     conf_info.interp_mthd[j],
                     conf_info.interp_wdth[j]);

      } // end for j

      // Loop through and apply the Neighborhood methods for each of the
      // neighborhood widths if requested in the config file
      if(!conf_info.fcst_info[i]->p_flag() &&
         (conf_info.output_flag[i_nbrctc] != STATOutputType_None ||
          conf_info.output_flag[i_nbrcts] != STATOutputType_None ||
          conf_info.output_flag[i_nbrcnt] != STATOutputType_None)) {

         // Initialize
         for(j=0; j<frac_t; j++) nbrcts_info[j].clear();

         // Loop through and apply each of the neighborhood widths
         for(j=0; j<conf_info.get_n_nbrhd_wdth(); j++) {

            // Store the interpolation method and width being applied
            shc.set_interp_mthd(InterpMthd_Nbrhd);
            shc.set_interp_wdth(conf_info.nbrhd_wdth[j]);

            // Loop through and apply each of the raw threshold values
            for(k=0; k<conf_info.fcst_ta[i].n_elements(); k++) {

               // Compute the thresholded fractional coverage field
               fractional_coverage(fcst_dp, fcst_dp_smooth,
                                   conf_info.nbrhd_wdth[j],
                                   conf_info.fcst_ta[i][k],
                                   conf_info.nbrhd_thresh);

               fractional_coverage(obs_dp, obs_dp_smooth,
                                   conf_info.nbrhd_wdth[j],
                                   conf_info.obs_ta[i][k],
                                   conf_info.nbrhd_thresh);

               // Loop through the masks to be applied
               for(m=0; m<conf_info.get_n_mask(); m++) {

                  // Apply the current mask to the fields
                  apply_mask(fcst_dp_smooth, obs_dp_smooth,
                             conf_info.mask_dp[m],
                             f_na, o_na);

                  mlog << Debug(2) << "Processing "
                       << conf_info.fcst_info[i]->magic_str()
                       << " versus "
                       << conf_info.obs_info[i]->magic_str()
                       << ", for interpolation method "
                       << shc.get_interp_mthd_str() << "("
                       << shc.get_interp_pnts_str()
                       << "), raw thresholds of "
                       << conf_info.fcst_ta[i][k].get_str()
                       << " and "
                       << conf_info.obs_ta[i][k].get_str()
                       << ", over region " << shc.get_mask()
                       << ", using " << f_na.n_elements()
                       << " pairs.\n";

                  // Continue if no pairs were found
                  if(f_na.n_elements() == 0) continue;

                  // Set the mask name
                  shc.set_mask(conf_info.mask_name[m]);

                  // Compute NBRCTS scores
                  if(conf_info.output_flag[i_nbrctc] != STATOutputType_None ||
                     conf_info.output_flag[i_nbrcts] != STATOutputType_None) {

                     // Initialize
                     for(n=0; n<conf_info.nbrhd_cov_ta.n_elements(); n++) {
                        nbrcts_info[n].clear();
                     }

                     do_nbrcts(nbrcts_info, i, j, k, f_na, o_na);

                     // Loop through all of the thresholds
                     for(n=0; n<conf_info.nbrhd_cov_ta.n_elements(); n++) {

                        // Only write out if n > 0
                        if(nbrcts_info[n].cts_info.cts.n() > 0) {

                           // Write out NBRCTC
                           if(conf_info.output_flag[i_nbrctc] != STATOutputType_None) {

                              write_nbrctc_row(shc, nbrcts_info[n],
                                 conf_info.output_flag[i_nbrctc] == STATOutputType_Both,
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrctc], i_txt_row[i_nbrctc]);
                           }

                           // Write out NBRCTS
                           if(conf_info.output_flag[i_nbrcts] != STATOutputType_None) {

                              write_nbrcts_row(shc, nbrcts_info[n],
                                 conf_info.output_flag[i_nbrcts] == STATOutputType_Both,
                                 stat_at, i_stat_row,
                                 txt_at[i_nbrcts], i_txt_row[i_nbrcts]);
                           }
                        } // end if
                     } // end for n
                  } // end compute NBRCTS

                  // Compute NBRCNT scores
                  if(conf_info.output_flag[i_nbrcnt] != STATOutputType_None) {

                     // Initialize
                     nbrcnt_info.clear();

                     do_nbrcnt(nbrcnt_info, i, j, k, f_na, o_na);

                     // Write out NBRCNT
                     if(nbrcnt_info.cnt_info.n > 0 &&
                        conf_info.output_flag[i_nbrcnt]) {

                        write_nbrcnt_row(shc, nbrcnt_info,
                           conf_info.output_flag[i_nbrcnt] == STATOutputType_Both,
                           stat_at, i_stat_row,
                           txt_at[i_nbrcnt], i_txt_row[i_nbrcnt]);
                     }
                  } // end compute NBRCNT
               } // end for m
            } // end for k
         } // end for j
      } // end if
   } // end for i

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Deallocate memory
   if(cts_info)    { delete [] cts_info;    cts_info    = (CTSInfo *)    0; }
   if(vl1l2_info)  { delete [] vl1l2_info;  vl1l2_info  = (VL1L2Info *)  0; }
   if(nbrcts_info) { delete [] nbrcts_info; nbrcts_info = (NBRCTSInfo *) 0; }
   if(pct_info)    { delete [] pct_info;    pct_info    = (PCTInfo *)    0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const DataPlane &dp) {

   // Unset the flag
   is_first_pass = false;
  
   // Process masks Grids and Polylines in the config file
   conf_info.process_masks(grid);

   // Create output text files as requested in the config file
   setup_txt_files(dp.valid(), dp.lead());

   // If requested, create a NetCDF file to store the matched pairs and
   // difference fields for each GRIB code and masking region
   if(conf_info.nc_pairs_flag) setup_nc_file(dp.valid(), dp.lead());

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files(unixtime valid_ut, int lead_sec) {
   int  i, max_col, max_prob_col, max_mctc_col, n_prob, n_cat;
   ConcatString base_name;

   // Create output file names for the stat file and optional text files
   build_outfile_name(valid_ut, lead_sec, "", base_name);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   n_prob = conf_info.get_max_n_prob_fcst_thresh();
   n_cat  = conf_info.get_max_n_scal_thresh() + 1;

   max_prob_col = get_n_pjc_columns(n_prob);
   max_mctc_col = get_n_mctc_columns(n_cat);

   // Maximum number of standard STAT columns
   max_col = max_stat_col + n_header_columns + 1;

   // Maximum number of probabilistic columns
   if(max_prob_col > max_col)
      max_col = max_prob_col + n_header_columns + 1;

   // Maximum number of multi-category contingency table columns
   if(max_mctc_col > max_stat_col)
      max_col = max_mctc_col + n_header_columns + 1;

   // Initialize file stream
   stat_out   = (ofstream *) 0;

   // Build the file name
   stat_file << base_name << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file);

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

   // Loop through output file type
   for(i=0; i<n_txt; i++) {

      // Only set it up if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Initialize file stream
         txt_out[i] = (ofstream *) 0;

         // Build the file name
         txt_file[i] << base_name << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i]);

         // Get the maximum number of columns for this line type
         switch(i) {

            case(i_mctc):
               max_col = get_n_mctc_columns(n_cat) + n_header_columns + 1;
               break;

            case(i_pct):
               max_col = get_n_pct_columns(n_prob)  + n_header_columns + 1;
               break;

            case(i_pstd):
               max_col = get_n_pstd_columns(n_prob) + n_header_columns + 1;
               break;

            case(i_pjc):
               max_col = get_n_pjc_columns(n_prob)  + n_header_columns + 1;
               break;

            case(i_prc):
               max_col = get_n_prc_columns(n_prob)  + n_header_columns + 1;
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

            case(i_pct):
               write_pct_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case(i_pstd):
               write_pstd_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case(i_pjc):
               write_pjc_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case(i_prc):
               write_prc_header_row(1, n_prob, txt_at[i], 0, 0);
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

   // Left-justify all columns
   at.set_table_just(LeftJust);

   // Set the precision
   at.set_precision(default_precision);

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(unixtime valid_ut, int lead_sec) {

   // Create output NetCDF file name
   build_outfile_name(valid_ut, lead_sec, "_pairs.nc", out_file);

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_file, NcFile::Replace);

   if(!nc_out->is_valid()) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file, program_name);
   nc_out->add_att("Difference", "Forecast Value - Observation Value");

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = nc_out->add_dim("lat", (long) grid.ny());
   lon_dim = nc_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, lat_dim, lon_dim, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime valid_ut, int lead_sec,
                        const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   int l_hr, l_min, l_sec;
   ConcatString date_str;

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      str << "_" << conf_info.output_prefix;

   // Append the timing information
   sec_to_hms(lead_sec, l_hr, l_min, l_sec);
   unix_to_mdyhms(valid_ut, mon, day, yr, hr, min, sec);
   date_str.format("%.2i%.2i%.2iL_%.4i%.2i%.2i_%.2i%.2i%.2iV",
           l_hr, l_min, l_sec, yr, mon, day, hr, min, sec);
   str << "_" << date_str;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cts(CTSInfo *&cts_info, int i_vx,
            const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_cts;

   //
   // Set up the CTSInfo thresholds and alpha values
   //
   n_cts = conf_info.fcst_ta[i_vx].n_elements();
   for(i=0; i<n_cts; i++) {
      cts_info[i].cts_fcst_thresh = conf_info.fcst_ta[i_vx][i];
      cts_info[i].cts_obs_thresh  = conf_info.obs_ta[i_vx][i];
      cts_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         cts_info[i].alpha[j] = conf_info.ci_alpha[j];
      }
   }

   mlog << Debug(2) << "Computing Categorical Statistics.\n";

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_cts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         cts_info, n_cts,
         conf_info.output_flag[i_cts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         cts_info, n_cts,
         conf_info.output_flag[i_cts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(MCTSInfo &mcts_info, int i_vx,
             const NumArray &f_na, const NumArray &o_na) {
   int i;

   //
   // Set up the MCTSInfo size, thresholds, and alpha values
   //
   mcts_info.cts.set_size(conf_info.fcst_ta[i_vx].n_elements() + 1);
   mcts_info.cts_fcst_ta = conf_info.fcst_ta[i_vx];
   mcts_info.cts_obs_ta  = conf_info.obs_ta[i_vx];
   mcts_info.allocate_n_alpha(conf_info.get_n_ci_alpha());

   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      mcts_info.alpha[i] = conf_info.ci_alpha[i];
   }

   mlog << Debug(2) << "Computing Multi-Category Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_mcts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         mcts_info,
         conf_info.output_flag[i_mcts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         mcts_info,
         conf_info.output_flag[i_mcts] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt(CNTInfo &cnt_info, int i_vx,
            const NumArray &f_na, const NumArray &o_na) {
   int i;

   //
   // Set up the CNTInfo alpha values
   //
   cnt_info.allocate_n_alpha(conf_info.get_n_ci_alpha());
   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      cnt_info.alpha[i] = conf_info.ci_alpha[i];
   }

   mlog << Debug(2) << "Computing Continuous Statistics.\n";

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_cnt_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.fcst_info[i_vx]->is_precipitation() &
         conf_info.obs_info[i_vx]->is_precipitation(),
         conf_info.n_boot_rep,
         cnt_info,
         conf_info.output_flag[i_cnt] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_cnt_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.fcst_info[i_vx]->is_precipitation() &
         conf_info.obs_info[i_vx]->is_precipitation(),
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         cnt_info,
         conf_info.output_flag[i_cnt] != STATOutputType_None,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_vl1l2(VL1L2Info *&v_info, int i_vx,
              const NumArray &vf_na, const NumArray &vo_na,
              const NumArray &uf_na, const NumArray &uo_na) {
   int i, j, n_thresh;
   double uf, vf, uo, vo, fwind, owind;
   SingleThresh fst, ost;

   // Check that the number of pairs are the same
   if(vf_na.n_elements() != vo_na.n_elements() ||
      uf_na.n_elements() != uo_na.n_elements() ||
      vf_na.n_elements() != uf_na.n_elements()) {

      mlog << Error << "\ndo_vl1l2() -> "
           << "the number of UGRD pairs != the number of VGRD pairs: "
           << vf_na.n_elements() << " != " << uf_na.n_elements()
           << "\n\n";
      exit(1);
   }

   // Initialize all of the VL1L2Info objects
   n_thresh = conf_info.get_n_wind_thresh();
   for(i=0; i<n_thresh; i++) {
      v_info[i].zero_out();
      v_info[i].wind_fcst_thresh = conf_info.fcst_wind_ta[i];
      v_info[i].wind_obs_thresh  = conf_info.obs_wind_ta[i];
   }

   // Loop through the pair data and compute sums
   for(i=0; i<vf_na.n_elements(); i++) {

      // Retrieve the U,V values
      uf = uf_na[i];
      vf = vf_na[i];
      uo = uo_na[i];
      vo = vo_na[i];

      // Compute wind speeds
      fwind = convert_u_v_to_wind(uf, vf);
      owind = convert_u_v_to_wind(uo, vo);

      // Skip bad data values in the forecast or observation fields
      if(is_bad_data(uf)    || is_bad_data(vf) ||
         is_bad_data(uo)    || is_bad_data(vo) ||
         is_bad_data(fwind) || is_bad_data(owind)) continue;

      // Loop through each of wind speed thresholds to be used
      for(j=0; j<n_thresh; j++) {

         fst = v_info[j].wind_fcst_thresh;
         ost = v_info[j].wind_obs_thresh;

         // Apply both wind speed thresholds
         if(fst.type != thresh_na && ost.type  != thresh_na) {
            if(!fst.check(fwind) || !ost.check(owind)) continue;
         }
         // Apply only the fcst wind speed threshold
         else if(fst.type != thresh_na && ost.type  == thresh_na) {
            if(!fst.check(fwind)) continue;
         }
         // Apply only the obs wind speed threshold
         else if(fst.type == thresh_na && ost.type  != thresh_na) {
            if(!ost.check(owind)) continue;
         }

         // Add this pair to the VL1L2 counts

         // VL1L2 sums
         v_info[j].vcount  += 1;
         v_info[j].ufbar   += uf;
         v_info[j].vfbar   += vf;
         v_info[j].uobar   += uo;
         v_info[j].vobar   += vo;
         v_info[j].uvfobar += uf*uo+vf*vo;
         v_info[j].uvffbar += uf*uf+vf*vf;
         v_info[j].uvoobar += uo*uo+vo*vo;
      } // end for j
   } // end for i

   // Compute means for the VL1L2Info objects
   for(i=0; i<n_thresh; i++) {

      mlog << Debug(2) << "Computing Vector Partial Sums, "
           << "for forecast wind speed "
           << v_info[i].wind_fcst_thresh.get_str()
           << ", for observation wind speed "
           << v_info[i].wind_obs_thresh.get_str()
           << ", using " << v_info[i].vcount << " pairs.\n";

      if(v_info[i].vcount == 0) {
         // Set to bad data
         v_info[i].ufbar   = bad_data_double;
         v_info[i].vfbar   = bad_data_double;
         v_info[i].uobar   = bad_data_double;
         v_info[i].vobar   = bad_data_double;
         v_info[i].uvfobar = bad_data_double;
         v_info[i].uvffbar = bad_data_double;
         v_info[i].uvoobar = bad_data_double;
      }
      else {
         // Compute the mean VL1L2 values
         v_info[i].ufbar   /= v_info[i].vcount;
         v_info[i].vfbar   /= v_info[i].vcount;
         v_info[i].uobar   /= v_info[i].vcount;
         v_info[i].vobar   /= v_info[i].vcount;
         v_info[i].uvfobar /= v_info[i].vcount;
         v_info[i].uvffbar /= v_info[i].vcount;
         v_info[i].uvoobar /= v_info[i].vcount;
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(PCTInfo *&pct_info, int i_vx,
            const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_pct;

   mlog << Debug(2) << "Computing Probabilistic Statistics.\n";

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Set up the PCTInfo thresholds and alpha values
   //
   n_pct = conf_info.obs_ta[i_vx].n_elements();
   for(i=0; i<n_pct; i++) {

      // Use all of the selected forecast thresholds
      pct_info[i].pct_fcst_thresh = conf_info.fcst_ta[i_vx];

      // Process the observation thresholds one at a time
      pct_info[i].pct_obs_thresh  = conf_info.obs_ta[i_vx][i];

      pct_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         pct_info[i].alpha[j] = conf_info.ci_alpha[j];
      }

      //
      // Compute the probabilistic counts and statistics
      //
      compute_pctinfo(f_na, o_na, conf_info.output_flag[i_pstd],
                      pct_info[i]);
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_nbrcts(NBRCTSInfo *&nbrcts_info,
               int i_vx, int i_wdth, int i_thresh,
               const NumArray &f_na, const NumArray &o_na) {
   int i, j, n_nbrcts;

   //
   // Set up the NBRCTSInfo thresholds and alpha values
   //
   n_nbrcts = conf_info.nbrhd_cov_ta.n_elements();
   for(i=0; i<n_nbrcts; i++) {

      nbrcts_info[i].raw_fcst_thresh =
         conf_info.fcst_ta[i_vx][i_thresh];
      nbrcts_info[i].raw_obs_thresh =
         conf_info.obs_ta[i_vx][i_thresh];
      nbrcts_info[i].frac_thresh =
         conf_info.nbrhd_cov_ta[i];

      nbrcts_info[i].cts_info.cts_fcst_thresh =
         conf_info.nbrhd_cov_ta[i];
      nbrcts_info[i].cts_info.cts_obs_thresh =
         conf_info.nbrhd_cov_ta[i];
      nbrcts_info[i].allocate_n_alpha(conf_info.get_n_ci_alpha());

      for(j=0; j<conf_info.get_n_ci_alpha(); j++) {
         nbrcts_info[i].cts_info.alpha[j] = conf_info.ci_alpha[j];
      }
   }

   //
   // Keep track of the neighborhood width
   //
   for(i=0; i<n_nbrcts; i++) {
      nbrcts_info[i].nbr_wdth = conf_info.nbrhd_wdth[i_wdth];
   }

   mlog << Debug(2) << "Computing Neighborhood Categorical Statistics.\n";

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_nbrcts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         nbrcts_info, n_nbrcts,
         conf_info.output_flag[i_nbrcts] != STATOutputType_None,
         conf_info.tmp_dir);
   }
   else {
      compute_nbrcts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         nbrcts_info, n_nbrcts,
         conf_info.output_flag[i_nbrcts] != STATOutputType_None,
         conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_nbrcnt(NBRCNTInfo &nbrcnt_info,
               int i_vx, int i_wdth, int i_thresh,
               const NumArray &f_na, const NumArray &o_na) {
   int i;

   //
   // Set up the NBRCNTInfo threshold and alpha values
   //
   nbrcnt_info.raw_fcst_thresh =
      conf_info.fcst_ta[i_vx][i_thresh];
   nbrcnt_info.raw_obs_thresh =
      conf_info.obs_ta[i_vx][i_thresh];

   nbrcnt_info.allocate_n_alpha(conf_info.get_n_ci_alpha());
   for(i=0; i<conf_info.get_n_ci_alpha(); i++) {
      nbrcnt_info.cnt_info.alpha[i] = conf_info.ci_alpha[i];
   }

   //
   // Keep track of the neighborhood width
   //
   nbrcnt_info.nbr_wdth = conf_info.nbrhd_wdth[i_wdth];

   mlog << Debug(2) << "Computing Neighborhood Continuous Statistics.\n";

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_nbrcnt_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         nbrcnt_info,
         conf_info.output_flag[i_nbrcnt] != STATOutputType_None,
         conf_info.tmp_dir);
   }
   else {
      compute_nbrcnt_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         nbrcnt_info,
         conf_info.output_flag[i_nbrcnt] != STATOutputType_None,
         conf_info.tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc(const DataPlane &fcst_dp, const DataPlane &obs_dp,
              int i_vx, InterpMthd mthd, int wdth) {
   int i, n, x, y;
   int fcst_flag, obs_flag, diff_flag;
   float *fcst_data = (float *) 0;
   float *obs_data  = (float *) 0;
   float *diff_data = (float *) 0;
   double fval, oval;
   NcVar *fcst_var = (NcVar *) 0, *obs_var = (NcVar *) 0, *diff_var = (NcVar *) 0;
   ConcatString fcst_var_name, obs_var_name, diff_var_name;
   ConcatString att_str, mthd_str;

   // Get the interpolation method string
   mthd_str = interpmthd_to_string(mthd);

   // Allocate memory for the forecast, observation, and difference
   // fields
   fcst_data = new float [grid.nx()*grid.ny()];
   obs_data  = new float [grid.nx()*grid.ny()];
   diff_data = new float [grid.nx()*grid.ny()];

   // Compute the difference field for each of the masking regions
   for(i=0; i<conf_info.get_n_mask(); i++) {

      // Build the forecast variable name
      fcst_var_name << cs_erase << "FCST_"
                    << conf_info.fcst_info[i_vx]->name() << "_"
                    << conf_info.fcst_info[i_vx]->level_name() << "_"
                    << conf_info.mask_name[i];
      
      // Append smoothing information
      if((wdth > 1) &&
         (conf_info.interp_field == FieldType_Fcst ||
          conf_info.interp_field == FieldType_Both)) {
         fcst_var_name << "_" << mthd_str << "_" << wdth*wdth;
      }

      // Build the observation variable name
      obs_var_name << cs_erase << "OBS_"
                   << conf_info.obs_info[i_vx]->name() << "_"
                   << conf_info.obs_info[i_vx]->level_name() << "_"
                   << conf_info.mask_name[i];

      // Append smoothing information
      if((wdth > 1) &&
         (conf_info.interp_field == FieldType_Obs ||
          conf_info.interp_field == FieldType_Both)) {
         obs_var_name << "_" << mthd_str << "_" << wdth*wdth;
      }

      // Build the difference variable name
      diff_var_name << cs_erase << "DIFF_"
                    << conf_info.fcst_info[i_vx]->name() << "_"
                    << conf_info.fcst_info[i_vx]->level_name() << "_"
                    << conf_info.obs_info[i_vx]->name() << "_"
                    << conf_info.obs_info[i_vx]->level_name() << "_"
                    << conf_info.mask_name[i];
      
      // Append smoothing information
      if(wdth > 1) {
         diff_var_name << "_"
                       << mthd_str << "_"
                       << wdth*wdth;
      }

      // Figure out if the forecast, observation, and difference
      // fields should be written out.
      fcst_flag = !fcst_var_sa.has(fcst_var_name);
      obs_flag  = !obs_var_sa.has(obs_var_name);

      // Don't write the difference field for probability forecasts
      diff_flag = (!diff_var_sa.has(diff_var_name) &&
                   !conf_info.fcst_info[i_vx]->p_flag());

      // Set up the forecast variable if not already defined
      if(fcst_flag) {

         // Define the forecast variable
         fcst_var = nc_out->add_var(fcst_var_name, ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         fcst_var_sa.add(fcst_var_name);

         // Add variable attributes for the forecast field
         add_var_att(fcst_var, "name", shc.get_fcst_var());
         att_str << cs_erase << conf_info.fcst_info[i_vx]->name()
                 << " at "
                 << conf_info.fcst_info[i_vx]->level_name();
         add_var_att(fcst_var, "long_name", att_str);
         add_var_att(fcst_var, "level", shc.get_fcst_lev());
         add_var_att(fcst_var, "units", conf_info.fcst_info[i_vx]->units());
         write_netcdf_var_times(fcst_var, fcst_dp);
         fcst_var->add_att("_FillValue", bad_data_float);
         add_var_att(fcst_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(fcst_var, "smoothing_method", mthd_str);
         fcst_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end fcst_flag

      // Set up the observation variable if not already defined
      if(obs_flag) {

         // Define the observation variable
         obs_var  = nc_out->add_var(obs_var_name,  ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         obs_var_sa.add(obs_var_name);

         // Add variable attributes for the observation field
         add_var_att(obs_var, "name", shc.get_obs_var());
         att_str << cs_erase << conf_info.obs_info[i_vx]->name()
                 << " at "
                 << conf_info.obs_info[i_vx]->level_name();
         add_var_att(obs_var, "long_name", att_str);
         add_var_att(obs_var, "level", shc.get_obs_lev());
         add_var_att(obs_var, "units", conf_info.obs_info[i_vx]->units());
         write_netcdf_var_times(obs_var, obs_dp);
         obs_var->add_att("_FillValue", bad_data_float);
         add_var_att(obs_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(obs_var, "smoothing_method", mthd_str);
         obs_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end obs_flag

      // Set up the difference variable if not already defined
      if(diff_flag) {

         // Define the difference variable
         diff_var = nc_out->add_var(diff_var_name, ncFloat,
                                    lat_dim, lon_dim);

         // Add to the list of previously defined variables
         diff_var_sa.add(diff_var_name);

         // Add variable attributes for the difference field
         att_str << cs_erase << "Forecast " << shc.get_fcst_var()
                 << " minus Observed " << shc.get_obs_var();
         add_var_att(diff_var, "name", att_str);
         att_str << cs_erase << conf_info.fcst_info[i_vx]->name() << " at "
                 << conf_info.fcst_info[i_vx]->level_name() << " and "
                 << conf_info.obs_info[i_vx]->name() << " at "
                 << conf_info.obs_info[i_vx]->level_name();
         add_var_att(diff_var, "long_name", att_str);
         att_str << cs_erase << shc.get_fcst_lev() << " and " << shc.get_obs_lev();
         add_var_att(diff_var, "level", att_str);
         att_str << cs_erase << conf_info.fcst_info[i_vx]->units() << " and "
                 << conf_info.obs_info[i_vx]->units();
         add_var_att(diff_var, "units", att_str);
         diff_var->add_att("_FillValue", bad_data_float);
         write_netcdf_var_times(diff_var, fcst_dp);
         diff_var->add_att("_FillValue", bad_data_float);
         add_var_att(diff_var, "masking_region", conf_info.mask_name[i]);
         add_var_att(diff_var, "smoothing_method", mthd_str);
         diff_var->add_att("smoothing_neighborhood", wdth*wdth);
      } // end diff_flag

      // Store the forecast, observation, and difference values
      for(x=0; x<grid.nx(); x++) {
         for(y=0; y<grid.ny(); y++) {

            n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

            // Check whether the mask is on or off for this point
            if(!conf_info.mask_dp[i].s_is_on(x, y)) {
               fcst_data[n] = bad_data_float;
               obs_data[n]  = bad_data_float;
               diff_data[n] = bad_data_float;
               continue;
            }

            // Retrieve the forecast and observation values
            fval = fcst_dp.get(x, y);
            oval = obs_dp.get(x, y);

            // Set the forecast data
            if(is_bad_data(fval)) fcst_data[n] = bad_data_float;
            else                  fcst_data[n] = fval;

            // Set the observation data
            if(is_bad_data(oval)) obs_data[n]  = bad_data_float;
            else                  obs_data[n]  = oval;

            // Set the difference data
            if(is_bad_data(fval) ||
               is_bad_data(oval)) diff_data[n] = bad_data_float;
            else                  diff_data[n] = fval - oval;
         } // end for y
      } // end for x

      // Write out the forecast field
      if(fcst_flag) {
         if(!fcst_var->put(&fcst_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nc() -> "
                 << "error with the fcst_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n";
            exit(1);
         }
      }

      // Write out the observation field
      if(obs_flag) {
         if(!obs_var->put(&obs_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nc() -> "
                 << "error with the obs_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n";
            exit(1);
         }
      }

      // Write out the difference field
      if(diff_flag) {
         if(!diff_var->put(&diff_data[0], grid.ny(), grid.nx())) {
            mlog << Error << "\nwrite_nc() -> "
                 << "error with the diff_var->put for fields "
                 << shc.get_fcst_var() << " and " << shc.get_obs_var()
                 << " and masking region " << conf_info.mask_name[i]
                 << "\n\n";
            exit(1);
         }
      }

   } // end for i

   // Deallocate and clean up
   if(fcst_data) { delete [] fcst_data; fcst_data = (float *) 0; }
   if(obs_data)  { delete [] obs_data;  obs_data  = (float *) 0; }
   if(diff_data) { delete [] diff_data; diff_data = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void add_var_att(NcVar *var, const char *att_name, const char *att_value) {

   if(att_value) var->add_att(att_name, att_value);
   else          var->add_att(att_name, na_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file);
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i]);
         }
      }
   }

   return;
}
*/
////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the output NetCDF file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_file << "\n";

      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Deallocate memory for data files
   if(fcst_mtddf) { delete fcst_mtddf; fcst_mtddf = (Met2dDataFile *) 0; }
   if(obs_mtddf)  { delete obs_mtddf;  obs_mtddf  = (Met2dDataFile *) 0; }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-fcst file_1 ... file_n\n"
        << "\t-obs file_1 ... file_n\n"
        << "\t[-both file_1 ... file_n]\n"
        << "\t-out file\n"
        << "\t-config file\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-fcst file_1 ... file_n\" are the gridded "
        << "forecast files or ASCII files containing lists of file "
        << "names to be used (required).\n"

        << "\t\t\"-obs file_1 ... file_n\" are the gridded "
        << "observation files or ASCII files containing lists of file "
        << "names to be used (required).\n"

        << "\t\t\"-both file_1 ... file_n\" sets the \"-fcst\" and "
        << "\"-obs\" options to the same set of files (optional).\n"

        << "\t\t\"-out file\" is the NetCDF output file containing "
        << "computed statistics (required).\n"

        << "\t\t\"-config file\" is a SeriesAnalysisConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\n\tNOTE: All fields must be on the same grid.\n\n";

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_fcst_files(const StringArray & a) {
   parse_file_list(a, fcst_files);
}

////////////////////////////////////////////////////////////////////////

void set_obs_files(const StringArray & a) {
   parse_file_list(a, obs_files);
}

////////////////////////////////////////////////////////////////////////

void set_both_files(const StringArray & a) {
   set_fcst_files(a);
   set_obs_files(a);
}

////////////////////////////////////////////////////////////////////////

void set_out_file(const StringArray & a) {
   out_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_config_file(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_log_file(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

void parse_file_list(const StringArray & a, StringArray & list) {
   int i;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   GrdFileType type = FileType_None;

   // Check for empty list
   if(a.n_elements() == 0) {
      mlog << Error << "\nparse_file_list() -> "
           << "empty list!\n\n";
      exit(1);
   }

   // Attempt to read the first file as a gridded data file
   mtddf = mtddf_factory.new_met_2d_data_file(a[0], type);

   // If the read was successful, store the list of gridded files.
   // Otherwise, process entries as ASCII files.
   if(mtddf)                            list.add(a);
   else for(i=0; i<a.n_elements(); i++) parse_ascii_file_list(a[0], list);

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_ascii_file_list(const char *file_list, StringArray & list) {
   ifstream f_in;
   char file_name[PATH_MAX];

   mlog << Debug(1)
        << "Reading ASCII file list: " << file_list << "\n";
   
   // Open the ensemble file list
   f_in.open(file_list);
   if(!f_in) {
      mlog << Error << "\nparse_ascii_file_list() -> "
           << "can't open the ASCII file list \"" << file_list
           << "\" for reading\n\n";
      exit(1);
   }

   // Read and store the file names
   while(f_in >> file_name) list.add(file_name);

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////
