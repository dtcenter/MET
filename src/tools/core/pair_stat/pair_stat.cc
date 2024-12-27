// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research led(UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   pair_stat.cc
//
//   Description:
//      Based on user-specified parameters, this tool reads already
//      computed forecast and observation pair data, subsets them by
//      attribute, time, and space, and computes many verification
//      scores and statistics, including confidence intervals, to
//      summarize the comparison.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11/07/24  Halley Gotway  MET #3006 New
//
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>

#include "main.h"
#include "pair_stat.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_regrid.h"
#include "vx_log.h"
#include "seeps.h"

#include "nc_obs_util.h"
#include "nc_point_obs_in.h"

#ifdef WITH_PYTHON
#include "data2d_nc_met.h"
#include "pointdata_python.h"
#endif

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void setup_first_pass(const DataPlane &, const Grid &);

static void setup_txt_files();
static void setup_table    (AsciiTable &);

static void build_outfile_name(unixtime, int, const char *,
                               ConcatString &);

static void process_mpr_pairs(const ConcatString &, PairsFormat);
static void process_ioda_pairs(const ConcatString &);
static void process_scores();

static void do_cts       (CTSInfo   *&, int, const PairDataPoint *);
static void do_mcts      (MCTSInfo   &, int, const PairDataPoint *);
static void do_cnt_sl1l2 (const PairStatVxOpt &, const PairDataPoint *);
static void do_vl1l2     (VL1L2Info *&, int, const PairDataPoint *, const PairDataPoint *);
static void do_pct       (const PairStatVxOpt &, const PairDataPoint *);

static void finish_txt_files();

static void clean_up();

static void usage();

static void set_pairs(const StringArray &);
static void set_format(const StringArray &);
static void set_config(const StringArray &);
static void set_outdir(const StringArray &);

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process each pairs file
   for(int i=0; i<pairs_files.n(); i++) {

      if(pairs_format == PairsFormat::MPR ||
         pairs_format == PairsFormat::Python) {
         process_mpr_pairs(pairs_files[i], pairs_format);
      }
      else if(pairs_format == PairsFormat::IODA) {
         process_ioda_pairs(pairs_files[i]);
      }
      else {
         mlog << Error
              << "Unsupported PairsFormat of \""
              << pairsformat_to_string(pairs_format) << "\"!\n\n";
         exit(1);
      }
   }

   // Process observation summaries and point weights
   for(int i=0; i<conf_info.get_n_vx(); i++) {
      conf_info.vx_opt[i].vx_pd.calc_obs_summary();
      conf_info.vx_opt[i].vx_pd.print_obs_summary();
      conf_info.vx_opt[i].vx_pd.set_point_weight(conf_info.point_weight_flag);
   }

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return 0;
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "pair_stat";
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   const char *method_name = "process_command_line() -> ";

   out_dir = ".";

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   CommandLine cline;
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_pairs,  "-pairs", -1);
   cline.add(set_format, "-format", 1);
   cline.add(set_config, "-config", 1);
   cline.add(set_outdir, "-outdir", 1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be no arguments left
   if(cline.n() != 0) usage();

   // Expand the input pairs file lists
   StringArray tmp_src(pairs_files);
   pairs_files.clear();

   for(int i=0; i<tmp_src.n(); i++) {

      // Attempt to parse each input as a file list
      StringArray sa(parse_ascii_file_list(tmp_src[i].c_str()));

      // Add list elements, if present, or the input file name, if not
      if(sa.n() > 0) pairs_files.add(sa);
      else           pairs_files.add(tmp_src[i]);
   }

   // Check for required argruments
   if(pairs_files.n() == 0) {
      mlog << Error << "\n" << method_name
           << "The \"-pairs\" command line option is required!\n\n";
      usage();
   }
   if(pairs_format == PairsFormat::None) {
      mlog << Error << "\n" << method_name
           << "The \"-format\" command line option is required!\n\n";
      usage();
   }
   if(config_file.empty()) {
      mlog << Error << "\n" << method_name
           << "The \"-config\" command line option is required!\n\n";
      usage();
   }

   // List of config files to be read
   StringArray config_files;

   // Add ConfigConstants
   config_files.add(replace_path(config_const_filename));

   // Add the IODA Data config file
   if(pairs_format == PairsFormat::IODA) {
      config_files.add(replace_path(ioda_data_config_filename));
   }
  
   // Add the default config file
   config_files.add(replace_path(default_config_filename));

   // Add the user config file
   config_files.add(config_file);

   // Read the config files
   conf_info.read_config(config_files);

   // Process the configuration
   conf_info.process_config(pairs_format);

   // List the input pair files
   mlog << Debug(1)
        << "Reading " << pairs_files.n() << " \""
        << pairsformat_to_string(pairs_format) << "\" format pairs file(s): "
        << write_css(pairs_files) << "\n";

   // Set the model name
   if(conf_info.model.empty()) {
      shc.set_model(na_str);
   }
   else {
      shc.set_model(conf_info.model.c_str());
   }

   // Use the first verification task to set the random number generator
   // and seed value for bootstrap confidence intervals
   rng_set(rng_ptr,
           conf_info.vx_opt[0].boot_info.rng.c_str(),
           conf_info.vx_opt[0].boot_info.seed.c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_first_pass(const DataPlane &dp, const Grid &data_grid) {

   // Unset the flag
   is_first_pass = false;

   // Determine the verification grid
   grid = parse_vx_grid(conf_info.vx_opt[0].vx_pd.fcst_info->regrid(),
                        &data_grid, &data_grid);

   // Process the masks
   conf_info.process_masks();

   // Process the geography data
   conf_info.process_geog();

   // Setup the VxPairDataPoint objects
   conf_info.set_vx_pd();

   // Store the lead and valid times
   if(fcst_valid_ut == (unixtime) 0) fcst_valid_ut = dp.valid();
   if(is_bad_data(fcst_lead_sec))    fcst_lead_sec = dp.lead();

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files() {
   int max_col, max_prob_col, max_mctc_col, max_orank_col;
   int n_prob, n_cat, n_eclv;
   ConcatString base_name;

   // Create output file names for the stat file and optional text files
   build_outfile_name(fcst_valid_ut, fcst_lead_sec, "", base_name);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   n_prob = conf_info.get_max_n_fprob_thresh();
   n_cat  = conf_info.get_max_n_cat_thresh() + 1;
   n_eclv = conf_info.get_max_n_eclv_points();

   max_prob_col  = get_n_pjc_columns(n_prob);
   max_mctc_col  = get_n_mctc_columns(n_cat);

   // Determine the maximum number of data columns
   max_col = (max_prob_col  > max_stat_col ? max_prob_col  : max_stat_col);
   if (max_mctc_col  > max_col) max_col = max_mctc_col;
   if (max_orank_col > max_col) max_col = max_orank_col;

   // Add the header columns
   max_col += n_header_columns + 1;

   // Initialize file stream
   stat_out = (ofstream *) nullptr;

   // Build the file name
   stat_file << base_name << stat_file_ext;

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

   // Loop through output file type
   for(int i=0; i<n_txt; i++) {

      // Only set it up if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType::Both) {

         // Initialize file stream
         txt_out[i] = (ofstream *) nullptr;

         // Build the file name
         txt_file[i] << base_name << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i].c_str());

         // Get the maximum number of columns for this line type
         switch(i) {

            case i_mctc:
               max_col = get_n_mctc_columns(n_cat) + n_header_columns + 1;
               break;

            case i_pct:
               max_col = get_n_pct_columns(n_prob) + n_header_columns + 1;
               break;

            case i_pstd:
               max_col = get_n_pstd_columns(n_prob) + n_header_columns + 1;
               break;

            case i_pjc:
               max_col = get_n_pjc_columns(n_prob) + n_header_columns + 1;
               break;

            case i_prc:
               max_col = get_n_prc_columns(n_prob) + n_header_columns + 1;
               break;

            case i_eclv:
               max_col = get_n_eclv_columns(n_eclv) + n_header_columns + 1;
               break;

            default:
               max_col = n_txt_columns[i] + n_header_columns + 1;
               break;
         } // end switch

         // Setup the text AsciiTable
         txt_at[i].set_size(conf_info.n_txt_row(i) + 1, max_col);
         setup_table(txt_at[i]);

         // Write the text header row
         switch(i) {

            case i_mctc:
               write_mctc_header_row(1, n_cat, txt_at[i], 0, 0);
               break;

            case i_pct:
               write_pct_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case i_pstd:
               write_pstd_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case i_pjc:
               write_pjc_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case i_prc:
               write_prc_header_row(1, n_prob, txt_at[i], 0, 0);
               break;

            case i_eclv:
               write_eclv_header_row(1, n_eclv, txt_at[i], 0, 0);
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

void build_outfile_name(unixtime valid_ut, int lead_sec,
                        const char *suffix, ConcatString &str) {

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      str << "_" << conf_info.output_prefix;

   // Append the timing information
   str << "_"
       << sec_to_hhmmss(lead_sec) << "L_"
       << unix_to_yyyymmdd_hhmmss(valid_ut) << "V";

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void process_mpr_pairs(const ConcatString &file_name, PairsFormat format) {

   return;
}

/* JHG
   int j;
   int n_fcst;
   DataPlaneArray fcst_dpa;
   DataPlaneArray fcmn_dpa, fcsd_dpa;
   DataPlaneArray ocmn_dpa, ocsd_dpa;
   unixtime file_ut, beg_ut, end_ut;

   // Loop through each of the fields to be verified and extract
   // the forecast and climatological fields for verification
   for(int i=0; i<conf_info.get_n_vx(); i++) {

      VarInfo *fcst_info = conf_info.vx_opt[i].vx_pd.fcst_info;
      VarInfo *obs_info  = conf_info.vx_opt[i].vx_pd.obs_info;

      // Read the gridded data from the input forecast file
      n_fcst = fcst_mtddf->data_plane_array(*fcst_info, fcst_dpa);
      mlog << Debug(2) << "\n" << sep_str << "\n\n"
           << "Reading data for " << fcst_info->magic_str() << ".\n";

      // Check for zero fields
      if(n_fcst == 0) {
         mlog << Warning << "\nprocess_fcst_climo_files() -> "
              << "no fields matching " << fcst_info->magic_str()
              << " found in file: " << fcst_file << "\n\n";
         continue;
      }

      // MET #2795, for multiple individual forecast levels, print a
      // warning if the observations levels are not fully covered.
      if(n_fcst > 1 &&
         !is_eq(fcst_info->level().lower(), fcst_info->level().upper()) &&
         (obs_info->level().lower() < fcst_info->level().lower() ||
          obs_info->level().upper() > fcst_info->level().upper())) {
         mlog << Warning << "\nprocess_fcst_climo_files() -> "
              << "The forecast level range (" << fcst_info->magic_str()
              << ") does not fully contain the observation level range ("
              << obs_info->magic_str() << "). No vertical interpolation "
              << "will be performed for observations falling outside "
              << "the range of forecast levels. Instead, they will be "
              << "matched to the single nearest forecast level.\n\n";
      }

      // Setup the first pass through the data
      if(is_first_pass) setup_first_pass(fcst_dpa[0], fcst_mtddf->grid());

      // Regrid, if necessary
      if(!(fcst_mtddf->grid() == grid)) {
         mlog << Debug(1)
              << "Regridding " << fcst_dpa.n_planes()
              << " forecast field(s) for " << fcst_info->magic_str()
              << " to the verification grid using "
              << fcst_info->regrid().get_str() << ".\n";

         // Loop through the forecast fields
         for(j=0; j<fcst_dpa.n_planes(); j++) {
            fcst_dpa[j] = met_regrid(fcst_dpa[j], fcst_mtddf->grid(), grid,
                                     fcst_info->regrid());
         }
      }

      // Rescale probabilities from [0, 100] to [0, 1]
      if(fcst_info->p_flag()) {
         for(j=0; j<fcst_dpa.n_planes(); j++) {
            rescale_probability(fcst_dpa[j]);
         }
      } // end for j

      // Read forecast climatology data
      fcmn_dpa = read_climo_data_plane_array(
                    conf_info.conf.lookup_dictionary(conf_key_fcst),
                    conf_key_climo_mean,
                    i, fcst_dpa[0].valid(), grid,
                    "forecast climatology mean");
      fcsd_dpa = read_climo_data_plane_array(
                    conf_info.conf.lookup_dictionary(conf_key_fcst),
                    conf_key_climo_stdev,
                    i, fcst_dpa[0].valid(), grid,
                    "forecast climatology standard deviation");

      // Read observation climatology data
      ocmn_dpa = read_climo_data_plane_array(
                    conf_info.conf.lookup_dictionary(conf_key_obs),
                    conf_key_climo_mean,
                    i, fcst_dpa[0].valid(), grid,
                    "observation climatology mean");
      ocsd_dpa = read_climo_data_plane_array(
                    conf_info.conf.lookup_dictionary(conf_key_obs),
                    conf_key_climo_stdev,
                    i, fcst_dpa[0].valid(), grid,
                    "observation climatology standard deviation");

      // Store data for the current verification task
      conf_info.vx_opt[i].vx_pd.set_fcst_dpa(fcst_dpa);
      conf_info.vx_opt[i].vx_pd.set_fcst_climo_mn_dpa(fcmn_dpa);
      conf_info.vx_opt[i].vx_pd.set_fcst_climo_sd_dpa(fcsd_dpa);
      conf_info.vx_opt[i].vx_pd.set_obs_climo_mn_dpa(ocmn_dpa);
      conf_info.vx_opt[i].vx_pd.set_obs_climo_sd_dpa(ocsd_dpa);

      // Get the valid time for the first field
      file_ut = fcst_dpa[0].valid();

      // If obs_valid_beg_ut and obs_valid_end_ut were set on the command
      // line, use them.  If not, use beg_ds and end_ds.
      if(obs_valid_beg_ut != (unixtime) 0 ||
         obs_valid_end_ut != (unixtime) 0) {
         beg_ut = obs_valid_beg_ut;
         end_ut = obs_valid_end_ut;
      }
      else {
         beg_ut = file_ut + conf_info.vx_opt[i].beg_ds;
         end_ut = file_ut + conf_info.vx_opt[i].end_ds;
      }

      // Store the valid times for this VxPairDataPoint object
      conf_info.vx_opt[i].vx_pd.set_fcst_ut(fcst_valid_ut);
      conf_info.vx_opt[i].vx_pd.set_beg_ut(beg_ut);
      conf_info.vx_opt[i].vx_pd.set_end_ut(end_ut);

      // Dump out the number of levels found
      mlog << Debug(2)
           << "For " << fcst_info->magic_str() << ", found "
           << n_fcst << " forecast levels, "
           << fcmn_dpa.n_planes() << " forecast climatology mean and "
           << fcsd_dpa.n_planes() << " standard deviation level(s), and "
           << ocmn_dpa.n_planes() << " observation climatology mean and "
           << ocsd_dpa.n_planes() << " standard deviation level(s).\n";

   } // end for i

   // Check for no data
   if(is_first_pass) {
      mlog << Error << "\nprocess_fcst_climo_files() -> "
           << "no requested forecast data found!  Exiting...\n\n";
      exit(1);
   }

   mlog << Debug(2)
        << "\n" << sep_str << "\n\n";

   return;
}
*/
////////////////////////////////////////////////////////////////////////

void process_ioda_pairs(const ConcatString &file_name) {

   return;
}

/* JHG
   int j, i_obs;
   float obs_arr[OBS_ARRAY_LEN], hdr_arr[HDR_ARRAY_LEN];
   float prev_obs_arr[OBS_ARRAY_LEN];
   ConcatString hdr_typ_str;
   ConcatString hdr_sid_str;
   ConcatString hdr_vld_str;
   ConcatString obs_qty_str;
   unixtime hdr_ut;
   NcFile *obs_in = (NcFile *) nullptr;
   const char *method_name = "process_obs_file() -> ";

   // Set flags for vectors
   bool vflag = conf_info.get_vflag();
   bool is_ugrd, is_vgrd;

   // Open the observation file as a NetCDF file.
   // The observation file must be in NetCDF format as the
   // output of the PB2NC or ASCII2NC tool.
   bool status;
   bool use_var_id = true;
   bool use_arr_vars = false;
   bool use_python = false;
   MetNcPointObsIn nc_point_obs;
   MetPointData *met_point_obs = nullptr;

   // Check for python format
   string python_command = obs_file[i_nc];
   bool use_xarray = (0 == python_command.find(conf_val_python_xarray));
   use_python = use_xarray || (0 == python_command.find(conf_val_python_numpy));

#ifdef WITH_PYTHON
   MetPythonPointDataFile met_point_file;
   if (use_python) {
      int offset = python_command.find("=");
      if (offset == std::string::npos) {
         mlog << Error << "\n" << method_name
              << "trouble parsing the python command " << python_command << ".\n\n";
         exit(1);
      }

      if(!met_point_file.open(python_command.substr(offset+1).c_str(), use_xarray)) {
         met_point_file.close();
         mlog << Error << "\n" << method_name
              << "trouble getting point observation file from python command "
              << python_command << ".\n\n";
         exit(1);
      }

      met_point_obs = met_point_file.get_met_point_data();
      use_var_id = met_point_file.is_using_var_id();
   }
   else {
#else
   if (use_python) python_compile_error(method_name);
#endif
      if( !nc_point_obs.open(obs_file[i_nc].c_str()) ) {
         nc_point_obs.close();
      
         mlog << Warning << "\n" << method_name
              << "can't open observation netCDF file: "
              << obs_file[i_nc] << "\n\n";
         return;
      }
      
      nc_point_obs.read_dim_headers();
      nc_point_obs.check_nc(obs_file[i_nc].c_str(), method_name);
      nc_point_obs.read_obs_data_table_lookups();
      met_point_obs = (MetPointData *)&nc_point_obs;
      use_var_id = nc_point_obs.is_using_var_id();
      use_arr_vars = nc_point_obs.is_using_obs_arr();
#ifdef WITH_PYTHON
   }
#endif

   // Perform GRIB table lookups, if needed
   is_vgrd = is_ugrd = false;

   int hdr_count = met_point_obs->get_hdr_cnt();
   int obs_count = met_point_obs->get_obs_cnt();
   mlog << Debug(2)
        << "Searching " << obs_count
        << " observations from " << hdr_count
        << " messages.\n";

   ConcatString var_name("");
   StringArray var_names;
   StringArray obs_qty_array = met_point_obs->get_qty_data();
   if(use_var_id) var_names = met_point_obs->get_var_names();

   const int buf_size = (obs_count > BUFFER_SIZE) ? BUFFER_SIZE : obs_count;
   int   obs_qty_idx_block[buf_size];
   float obs_arr_block[buf_size][OBS_ARRAY_LEN];

   // Process each observation in the file
   int block_size;
   int prev_grib_code = bad_data_int;
   for(int i_block_start_idx=0; i_block_start_idx<obs_count; i_block_start_idx+=buf_size) {
      block_size = (obs_count - i_block_start_idx);
      if (block_size > buf_size) block_size = buf_size;

#ifdef WITH_PYTHON
      if (use_python)
         status = met_point_obs->get_point_obs_data()->fill_obs_buf(
                             block_size, i_block_start_idx, (float *)obs_arr_block, obs_qty_idx_block);
      else
#endif
         status = nc_point_obs.read_obs_data(block_size, i_block_start_idx,
                                            (float *)obs_arr_block,
                                            obs_qty_idx_block, (char *)nullptr);
      if (!status) exit(1);

      int hdr_idx;
      for(int i_block_idx=0; i_block_idx<block_size; i_block_idx++) {
         i_obs = i_block_start_idx + i_block_idx;

         for (j=0; j<OBS_ARRAY_LEN; j++) {
            obs_arr[j] = obs_arr_block[i_block_idx][j];
         }

         int qty_offset = use_arr_vars ? i_obs : obs_qty_idx_block[i_block_idx];
         obs_qty_str = obs_qty_array[qty_offset];

         int headerOffset = met_point_obs->get_header_offset(obs_arr);

         // Range check the header offset
         if(headerOffset < 0 || headerOffset >= hdr_count) {
            mlog << Warning << "\n" << method_name
                 << "range check error for header index " << headerOffset
                 << " from observation number " << i_obs
                 << " of point observation file: " << obs_file[i_nc]
                 << "\n\n";
            continue;
         }

         // Read the corresponding header array for this observation
         // - the corresponding header type, header Station ID, and valid time
#ifdef WITH_PYTHON
         if (use_python)
            met_point_obs->get_header(headerOffset, hdr_arr, hdr_typ_str, hdr_sid_str, hdr_vld_str);
         else
#endif
            nc_point_obs.get_header(headerOffset, hdr_arr, hdr_typ_str,
                                    hdr_sid_str, hdr_vld_str);

         // Store the variable name
         int org_grib_code = met_point_obs->get_grib_code_or_var_index(obs_arr);
         int grib_code = org_grib_code;
         if (prev_grib_code != org_grib_code) {
            if (use_var_id && grib_code < var_names.n()) {
               var_name   = var_names[grib_code];
               grib_code = bad_data_int;
            }
            else {
               var_name = "";
            }

            // Check for wind components
            is_ugrd = ( use_var_id &&        var_name == ugrd_abbr_str ) ||
                      (!use_var_id && nint(grib_code) == ugrd_grib_code);
            is_vgrd = ( use_var_id &&        var_name == vgrd_abbr_str ) ||
                      (!use_var_id && nint(grib_code) == vgrd_grib_code);
            prev_grib_code = org_grib_code;
         }

         // If the current observation is UGRD, save it as the
         // previous.  If vector winds are to be computed, UGRD
         // must be followed by VGRD
         if(vflag && is_ugrd) {
            for(j=0; j<4; j++) prev_obs_arr[j] = obs_arr[j];
         }

         // If the current observation is VGRD and vector
         // winds are to be computed.  Make sure that the
         // previous observation was UGRD with the same header
         // and at the same vertical level.
         if(vflag && is_vgrd) {

            if(!met_point_obs->is_same_obs_values(obs_arr, prev_obs_arr)) {
               mlog << Error << "\n" << method_name
                    << "for observation index " << i_obs
                    << ", when computing VL1L2 and/or VAL1L2 vector winds "
                    << "each UGRD observation must be followed by a VGRD "
                    << "observation with the same header and at the same "
                    << "level.\n\n";
               exit(1);
            }
         }

         // Convert string to a unixtime
         hdr_ut = timestring_to_unix(hdr_vld_str.c_str());

         // Check each conf_info.vx_pd object to see if this observation
         // should be added
         for(j=0; j<conf_info.get_n_vx(); j++) {

            // Check for no forecast fields
            if(conf_info.vx_opt[j].vx_pd.fcst_dpa.n_planes() == 0) continue;

            // Attempt to add the observation to the conf_info.vx_pd object
            conf_info.vx_opt[j].vx_pd.add_point_obs(
                    hdr_arr, hdr_typ_str.c_str(), hdr_sid_str.c_str(),
                    hdr_ut, obs_qty_str.c_str(), obs_arr,
                    grid, var_name.c_str());
         }

         met_point_obs->set_grib_code_or_var_index(obs_arr, org_grib_code);
      }

   } // end for i_block_start_idx

   // Deallocate and clean up
#ifdef WITH_PYTHON
   if (use_python) met_point_file.close();
   else
#endif
   nc_point_obs.close();

   return;
}
*/
////////////////////////////////////////////////////////////////////////

void process_scores() {
   int n_cat, n_wind;
   ConcatString cs;

   // Initialize pointers
   CTSInfo       *cts_info   = (CTSInfo *)       nullptr;
   MCTSInfo       mcts_info;
   VL1L2Info     *vl1l2_info = (VL1L2Info *)     nullptr;

   mlog << Debug(2)
        << "\n" << sep_str << "\n\n";

   // Setup the output text files as requested in the config file
   setup_txt_files();

   // Store the maximum number of each threshold type
   n_cat  = conf_info.get_max_n_cat_thresh();
   n_wind = conf_info.get_max_n_wind_thresh();

   // Allocate space for output statistics types
   cts_info   = new CTSInfo   [n_cat];
   vl1l2_info = new VL1L2Info [n_wind];

   // Compute scores for each PairData object and write output
   for(int i_vx=0; i_vx<conf_info.get_n_vx(); i_vx++) {

      PairStatVxOpt *vx_ptr = &conf_info.vx_opt[i_vx];

      // Check for no forecast fields
      if(vx_ptr->vx_pd.fcst_dpa.n_planes() == 0) continue;

      // Store the description
      if(vx_ptr->vx_pd.desc.empty()) {
         shc.set_desc(na_str);
      }
      else {
         shc.set_desc(vx_ptr->vx_pd.desc.c_str());
      }

      // Store the forecast variable name
      shc.set_fcst_var(vx_ptr->vx_pd.fcst_info->name_attr());

      // Store the forecast variable units
      shc.set_fcst_units(vx_ptr->vx_pd.fcst_info->units_attr());

      // Set the forecast level name
      shc.set_fcst_lev(vx_ptr->vx_pd.fcst_info->level_attr().c_str());

      // Store the observation variable name
      shc.set_obs_var(vx_ptr->vx_pd.obs_info->name_attr());

      // Store the observation variable units
      cs = vx_ptr->vx_pd.obs_info->units_attr();
      if(cs.empty()) cs = na_string;
      shc.set_obs_units(cs);

      // Set the observation level name
      shc.set_obs_lev(vx_ptr->vx_pd.obs_info->level_attr().c_str());

      // Set the forecast lead time
      shc.set_fcst_lead_sec(vx_ptr->vx_pd.fcst_dpa[0].lead());

      // Set the forecast valid time
      shc.set_fcst_valid_beg(vx_ptr->vx_pd.fcst_dpa[0].valid());
      shc.set_fcst_valid_end(vx_ptr->vx_pd.fcst_dpa[0].valid());

      // Set the observation lead time
      shc.set_obs_lead_sec(0);

      // Set the observation valid time
      shc.set_obs_valid_beg(vx_ptr->vx_pd.beg_ut);
      shc.set_obs_valid_end(vx_ptr->vx_pd.end_ut);

      // Store the message type in the obtype column
      shc.set_obtype(na_str);

      // Loop through the verification masking regions
      for(int i_mask=0; i_mask<vx_ptr->get_n_mask(); i_mask++) {

         // Store the verification masking region
         shc.set_mask(vx_ptr->mask_name[i_mask].c_str());

         // Store the interpolation method as nearest
         shc.set_interp_mthd(InterpMthd::Nearest);
         shc.set_interp_wdth(1);

         PairDataPoint *pd_ptr = &vx_ptr->vx_pd.pd[i_mask];

         mlog << Debug(2)
              << "Processing " << vx_ptr->vx_pd.fcst_info->magic_str()
              << " versus " << vx_ptr->vx_pd.obs_info->magic_str()
              << ", over region " << pd_ptr->mask_name
              << "), using " << pd_ptr->n_obs << " matched pairs.\n";

         // List counts for reasons why observations were rejected
         cs << cs_erase
            << "Number of matched pairs   = " << pd_ptr->n_obs << "\n"
            << "Observations processed    = " << vx_ptr->vx_pd.n_try << "\n"
            << "Rejected: station id      = " << vx_ptr->vx_pd.rej_sid << "\n"
            << "Rejected: obs var name    = " << vx_ptr->vx_pd.rej_var << "\n"
            << "Rejected: valid time      = " << vx_ptr->vx_pd.rej_vld << "\n"
            << "Rejected: bad obs value   = " << vx_ptr->vx_pd.rej_obs << "\n"
            << "Rejected: off the grid    = " << vx_ptr->vx_pd.rej_grd << "\n"
            << "Rejected: topography      = " << vx_ptr->vx_pd.rej_topo << "\n"
            << "Rejected: level mismatch  = " << vx_ptr->vx_pd.rej_lvl << "\n"
            << "Rejected: quality marker  = " << vx_ptr->vx_pd.rej_qty << "\n"
            << "Rejected: message type    = " << vx_ptr->vx_pd.rej_typ[i_mask] << "\n"
            << "Rejected: masking region  = " << vx_ptr->vx_pd.rej_mask[i_mask] << "\n"
            << "Rejected: bad fcst value  = " << vx_ptr->vx_pd.rej_fcst[i_mask] << "\n"
            << "Rejected: bad climo mean  = " << vx_ptr->vx_pd.rej_cmn[i_mask] << "\n"
            << "Rejected: bad climo stdev = " << vx_ptr->vx_pd.rej_csd[i_mask] << "\n"
            << "Rejected: mpr filter      = " << vx_ptr->vx_pd.rej_mpr[i_mask] << "\n"
            << "Rejected: duplicates      = " << vx_ptr->vx_pd.rej_dup[i_mask] << "\n";

         // Print report based on the number of matched pairs
         if(pd_ptr->n_obs > 0) mlog << Debug(3) << cs;
         else                  mlog << Debug(2) << cs;

         // Process percentile thresholds
         vx_ptr->set_perc_thresh(pd_ptr);

         // Write out the MPR lines
         if(vx_ptr->output_flag[i_mpr] != STATOutputType::None) {
            write_mpr_row(shc, pd_ptr,
               vx_ptr->output_flag[i_mpr],
               stat_at, i_stat_row,
               txt_at[i_mpr], i_txt_row[i_mpr], false);

            // Reset the obtype column
            shc.set_obtype(na_str);

            // Reset the observation valid time
            shc.set_obs_valid_beg(vx_ptr->vx_pd.beg_ut);
            shc.set_obs_valid_end(vx_ptr->vx_pd.end_ut);
         }

         // Write out the SEEPS MPR lines
         if(vx_ptr->output_flag[i_seeps_mpr] != STATOutputType::None) {
            write_seeps_mpr_row(shc, pd_ptr,
               vx_ptr->output_flag[i_seeps_mpr],
               stat_at, i_stat_row,
               txt_at[i_seeps_mpr], i_txt_row[i_seeps_mpr], false);

            // Reset the obtype column
            shc.set_obtype(na_str);

            // Reset the observation valid time
            shc.set_obs_valid_beg(vx_ptr->vx_pd.beg_ut);
            shc.set_obs_valid_end(vx_ptr->vx_pd.end_ut);
         }

         // Write out the SEEPS lines
         if(vx_ptr->output_flag[i_seeps] != STATOutputType::None) {
            compute_aggregated_seeps(pd_ptr, &pd_ptr->seeps_agg);
            write_seeps_row(shc, &pd_ptr->seeps_agg,
               vx_ptr->output_flag[i_seeps],
               stat_at, i_stat_row,
               txt_at[i_seeps], i_txt_row[i_seeps]);
         }

         // Compute CTS scores
         if(!vx_ptr->vx_pd.fcst_info->is_prob() &&
             vx_ptr->fcat_ta.n() > 0            &&
            (vx_ptr->output_flag[i_fho]  != STATOutputType::None ||
             vx_ptr->output_flag[i_ctc]  != STATOutputType::None ||
             vx_ptr->output_flag[i_cts]  != STATOutputType::None ||
             vx_ptr->output_flag[i_eclv] != STATOutputType::None)) {

            // Initialize
            for(int i_cat=0; i_cat<n_cat; i_cat++) cts_info[i_cat].clear();

            // Compute CTS Info
            do_cts(cts_info, i_vx, pd_ptr);

            // Loop through the categorical thresholds
            for(int i_cat=0; i_cat<vx_ptr->fcat_ta.n(); i_cat++) {

               if(cts_info[i_cat].cts.n_pairs() == 0) continue;

               // Write out FHO
               if(vx_ptr->output_flag[i_fho] != STATOutputType::None) {
                  write_fho_row(shc, cts_info[i_cat],
                     vx_ptr->output_flag[i_fho],
                     stat_at, i_stat_row,
                     txt_at[i_fho], i_txt_row[i_fho]);
               }

               // Write out CTC
               if(vx_ptr->output_flag[i_ctc] != STATOutputType::None) {
                  write_ctc_row(shc, cts_info[i_cat],
                     vx_ptr->output_flag[i_ctc],
                     stat_at, i_stat_row,
                     txt_at[i_ctc], i_txt_row[i_ctc]);
               }

               // Write out CTS
               if(vx_ptr->output_flag[i_cts] != STATOutputType::None) {
                  write_cts_row(shc, cts_info[i_cat],
                     vx_ptr->output_flag[i_cts],
                     stat_at, i_stat_row,
                     txt_at[i_cts], i_txt_row[i_cts]);
               }

               // Write out ECLV
               if(vx_ptr->output_flag[i_eclv] != STATOutputType::None) {
                  write_eclv_row(shc, cts_info[i_cat], vx_ptr->eclv_points,
                     vx_ptr->output_flag[i_eclv],
                     stat_at, i_stat_row,
                     txt_at[i_eclv], i_txt_row[i_eclv]);
               }
            } // end for i_cat 
         } // end Compute CTS scores

         // Compute MCTS scores
         if(!vx_ptr->vx_pd.fcst_info->is_prob() &&
             vx_ptr->fcat_ta.n() > 1            &&
            (vx_ptr->output_flag[i_mctc] != STATOutputType::None ||
             vx_ptr->output_flag[i_mcts] != STATOutputType::None)) {

            // Initialize
            mcts_info.clear();

            // Compute MCTS Info
            do_mcts(mcts_info, i_vx, pd_ptr);

            if(mcts_info.cts.n_pairs() == 0) continue;

            // Write out MCTC
            if(vx_ptr->output_flag[i_mctc] != STATOutputType::None) {
               write_mctc_row(shc, mcts_info,
                  vx_ptr->output_flag[i_mctc],
                  stat_at, i_stat_row,
                  txt_at[i_mctc], i_txt_row[i_mctc]);
            }

            // Write out MCTS
            if(vx_ptr->output_flag[i_mcts] != STATOutputType::None) {
               write_mcts_row(shc, mcts_info,
                  vx_ptr->output_flag[i_mcts],
                  stat_at, i_stat_row,
                  txt_at[i_mcts], i_txt_row[i_mcts]);
            }
         } // end Compute MCTS scores

         // Compute CNT, SL1L2, and SAL1L2 scores
         if(!vx_ptr->vx_pd.fcst_info->is_prob() &&
            (vx_ptr->output_flag[i_cnt]    != STATOutputType::None ||
             vx_ptr->output_flag[i_sl1l2]  != STATOutputType::None ||
             vx_ptr->output_flag[i_sal1l2] != STATOutputType::None)) {
             do_cnt_sl1l2(*vx_ptr, pd_ptr);
         }

         // Compute VL1L2 and VAL1L2 partial sums for UGRD and VGRD
         if(!vx_ptr->vx_pd.fcst_info->is_prob()       &&
             vx_ptr->vx_pd.fcst_info->is_v_wind()     &&
             vx_ptr->vx_pd.fcst_info->uv_index() >= 0 &&
            (vx_ptr->output_flag[i_vl1l2]  != STATOutputType::None ||
             vx_ptr->output_flag[i_val1l2] != STATOutputType::None ||
             vx_ptr->output_flag[i_vcnt]   != STATOutputType::None)) {

            // Store the forecast variable name
            shc.set_fcst_var(ugrd_vgrd_abbr_str);

            // Store the observation variable name
            shc.set_obs_var(ugrd_vgrd_abbr_str);

            // Initialize
            for(int i_wind=0; i_wind<n_wind; i_wind++) vl1l2_info[i_wind].clear();

            // Get the index of the matching u-component
            int u_vx = vx_ptr->vx_pd.fcst_info->uv_index();

            // Check to make sure the masking regions match
            if(conf_info.vx_opt[i_vx].get_n_mask() !=
               conf_info.vx_opt[u_vx].get_n_mask()) {
               mlog << Warning << "\nprocess_scores() -> "
                    << "when computing VL1L2 and/or VAL1L2 vector "
                    << "partial sums, the U and V components must "
                    << "be processed using the same set of mask regions. "
                    << "Failing to do so will cause unexpected results!\n\n";
            }

            // Compute VL1L2 and VAL1L2
            do_vl1l2(vl1l2_info, i_vx,
                     &conf_info.vx_opt[u_vx].vx_pd.pd[i_mask],
                     &conf_info.vx_opt[i_vx].vx_pd.pd[i_mask]);

            // Loop through all of the wind speed thresholds
            for(int i_wind=0; i_wind<vx_ptr->fwind_ta.n(); i_wind++) {

               // Write out VL1L2
               if(vx_ptr->output_flag[i_vl1l2] != STATOutputType::None &&
                  vl1l2_info[i_wind].vcount > 0) {
                  write_vl1l2_row(shc, vl1l2_info[i_wind],
                     vx_ptr->output_flag[i_vl1l2],
                     stat_at, i_stat_row,
                     txt_at[i_vl1l2], i_txt_row[i_vl1l2]);
               }

               // Write out VAL1L2
               if(vx_ptr->output_flag[i_val1l2] != STATOutputType::None &&
                  vl1l2_info[i_wind].vacount > 0) {
                  write_val1l2_row(shc, vl1l2_info[i_wind],
                     vx_ptr->output_flag[i_val1l2],
                     stat_at, i_stat_row,
                     txt_at[i_val1l2], i_txt_row[i_val1l2]);
               }

               // Write out VCNT
               if(vx_ptr->output_flag[i_vcnt] != STATOutputType::None &&
                   vl1l2_info[i_wind].vcount > 0) {
                   write_vcnt_row(shc, vl1l2_info[i_wind],
                      vx_ptr->output_flag[i_vcnt],
                      stat_at, i_stat_row,
                      txt_at[i_vcnt], i_txt_row[i_vcnt]);
               }
            } // end for i_wind

            // Reset the forecast variable name
            shc.set_fcst_var(vx_ptr->vx_pd.fcst_info->name_attr());

            // Reset the observation variable name
            shc.set_obs_var(vx_ptr->vx_pd.obs_info->name_attr());

        } // end Compute VL1L2 and VAL1L2

        // Compute PCT counts and scores
        if(vx_ptr->vx_pd.fcst_info->is_prob() &&
           (vx_ptr->output_flag[i_pct]  != STATOutputType::None ||
            vx_ptr->output_flag[i_pstd] != STATOutputType::None ||
            vx_ptr->output_flag[i_pjc]  != STATOutputType::None ||
            vx_ptr->output_flag[i_prc]  != STATOutputType::None ||
            vx_ptr->output_flag[i_eclv] != STATOutputType::None)) {
            do_pct(conf_info.vx_opt[i_vx], pd_ptr);
         }

         // Reset the verification masking region
         shc.set_mask(vx_ptr->mask_name[i_mask].c_str());

      } // end for i_mask

      mlog << Debug(2) << "\n" << sep_str << "\n\n";

   } // end for i_vx

   // Deallocate memory
   if(cts_info)   { delete [] cts_info;   cts_info   = (CTSInfo *)   nullptr; }
   if(vl1l2_info) { delete [] vl1l2_info; vl1l2_info = (VL1L2Info *) nullptr; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cts(CTSInfo *&cts_info, int i_vx, const PairDataPoint *pd_ptr) {
   int i, j, n_cat;

   mlog << Debug(2)
        << "Computing Categorical Statistics.\n";

   //
   // Set up the CTSInfo thresholds and alpha values
   //
   n_cat = conf_info.vx_opt[i_vx].fcat_ta.n();
   for(i=0; i<n_cat; i++) {
      cts_info[i].cts.set_ec_value(conf_info.vx_opt[i_vx].hss_ec_value);
      cts_info[i].fthresh = conf_info.vx_opt[i_vx].fcat_ta[i];
      cts_info[i].othresh = conf_info.vx_opt[i_vx].ocat_ta[i];
      cts_info[i].allocate_n_alpha(conf_info.vx_opt[i_vx].get_n_ci_alpha());

      for(j=0; j<conf_info.vx_opt[i_vx].get_n_ci_alpha(); j++) {
         cts_info[i].alpha[j] = conf_info.vx_opt[i_vx].ci_alpha[j];
      }
   }

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n() == 0 || pd_ptr->o_na.n() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.vx_opt[i_vx].boot_info.interval == BootIntervalType::BCA) {
      compute_cts_stats_ci_bca(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         cts_info, n_cat,
         conf_info.vx_opt[i_vx].output_flag[i_cts] != STATOutputType::None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         conf_info.vx_opt[i_vx].boot_info.rep_prop,
         cts_info, n_cat,
         conf_info.vx_opt[i_vx].output_flag[i_cts] != STATOutputType::None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(MCTSInfo &mcts_info, int i_vx, const PairDataPoint *pd_ptr) {
   int i;

   mlog << Debug(2)
        << "Computing Multi-Category Statistics.\n";

   //
   // Set up the MCTSInfo size, thresholds, and alpha values
   //
   mcts_info.cts.set_size(conf_info.vx_opt[i_vx].fcat_ta.n() + 1);
   mcts_info.cts.set_ec_value(conf_info.vx_opt[i_vx].hss_ec_value);
   mcts_info.set_fthresh(conf_info.vx_opt[i_vx].fcat_ta);
   mcts_info.set_othresh(conf_info.vx_opt[i_vx].ocat_ta);
   mcts_info.allocate_n_alpha(conf_info.vx_opt[i_vx].get_n_ci_alpha());

   for(i=0; i<conf_info.vx_opt[i_vx].get_n_ci_alpha(); i++) {
      mcts_info.alpha[i] = conf_info.vx_opt[i_vx].ci_alpha[i];
   }

   //
   // If there are no matched pairs to process, return
   //
   if(pd_ptr->f_na.n() == 0 || pd_ptr->o_na.n() == 0) return;

   //
   // Compute the stats, normal confidence intervals, and bootstrap
   // bootstrap confidence intervals
   //
   if(conf_info.vx_opt[i_vx].boot_info.interval == BootIntervalType::BCA) {
      compute_mcts_stats_ci_bca(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         mcts_info,
         conf_info.vx_opt[i_vx].output_flag[i_mcts] != STATOutputType::None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, *pd_ptr,
         conf_info.vx_opt[i_vx].boot_info.n_rep,
         conf_info.vx_opt[i_vx].boot_info.rep_prop,
         mcts_info,
         conf_info.vx_opt[i_vx].output_flag[i_mcts] != STATOutputType::None,
         conf_info.vx_opt[i_vx].rank_corr_flag,
         conf_info.tmp_dir.c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt_sl1l2(const PairStatVxOpt &vx_opt, const PairDataPoint *pd_ptr) {
   int i, j, k, n_bin;
   PairDataPoint pd_thr, pd;
   SL1L2Info *sl1l2_info = (SL1L2Info *) nullptr;
   CNTInfo   *cnt_info   = (CNTInfo *)   nullptr;

   mlog << Debug(2)
        << "Computing Scalar Partial Sums and Continuous Statistics.\n";

   // Determine the number of observation climo CDF bins
   n_bin = (pd_ptr->ocmn_na.n_valid() > 0 &&
            pd_ptr->ocsd_na.n_valid() > 0 ?
            vx_opt.get_n_cdf_bin() : 1);

   if(n_bin > 1) {
      mlog << Debug(2)
           << "Applying " << n_bin << " climatology bins.\n";
   }

   // Set flags
   bool do_sl1l2    = (vx_opt.output_flag[i_sl1l2]  != STATOutputType::None ||
                       vx_opt.output_flag[i_sal1l2] != STATOutputType::None);
   bool do_cnt      = (vx_opt.output_flag[i_cnt]    != STATOutputType::None);
   bool precip_flag = (vx_opt.vx_pd.fcst_info->is_precipitation() &&
                       vx_opt.vx_pd.obs_info->is_precipitation());

   // Allocate memory
   cnt_info   = new CNTInfo   [n_bin];
   sl1l2_info = new SL1L2Info [n_bin];

   // Process each continuous filtering threshold
   for(i=0; i<vx_opt.fcnt_ta.n(); i++) {

      // Apply continuous filtering thresholds to subset pairs
      pd_thr = pd_ptr->subset_pairs_cnt_thresh(vx_opt.fcnt_ta[i],
                                               vx_opt.ocnt_ta[i],
                                               vx_opt.cnt_logic);

      // Check for no matched pairs to process
      if(pd_thr.n_obs == 0) continue;

      // Process the climo CDF bins
      for(j=0; j<n_bin; j++) {

         // Initialize
         if(do_sl1l2) sl1l2_info[j].clear();
         if(do_cnt)   cnt_info[j].clear();

         // Apply climo CDF bins logic to subset pairs
         if(n_bin > 1) pd = subset_climo_cdf_bin(pd_thr,
                               vx_opt.cdf_info.cdf_ta, j);
         else          pd = pd_thr;

         // Check for no matched pairs to process
         if(pd.n_obs == 0) continue;

         // Compute and write SL1L2 and SAL1L2 output
         if(do_sl1l2) {

            // Store thresholds
            sl1l2_info[j].fthresh = vx_opt.fcnt_ta[i];
            sl1l2_info[j].othresh = vx_opt.ocnt_ta[i];
            sl1l2_info[j].logic   = vx_opt.cnt_logic;

            // Compute partial sums
            sl1l2_info[j].set(pd);

            // Write out SL1L2
            if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
               vx_opt.output_flag[i_sl1l2] != STATOutputType::None &&
               sl1l2_info[j].scount > 0) {

               write_sl1l2_row(shc, sl1l2_info[j],
                  vx_opt.output_flag[i_sl1l2],
                  j, n_bin, stat_at, i_stat_row,
                  txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
            }

            // Write out SAL1L2
            if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
               vx_opt.output_flag[i_sal1l2] != STATOutputType::None &&
               sl1l2_info[j].sacount > 0) {

               write_sal1l2_row(shc, sl1l2_info[j],
                  vx_opt.output_flag[i_sal1l2],
                  j, n_bin, stat_at, i_stat_row,
                  txt_at[i_sal1l2], i_txt_row[i_sal1l2]);
            }
         } // end do_sl1l2

         // Compute and write CNT output
         if(do_cnt) {

            // Store thresholds
            cnt_info[j].fthresh = vx_opt.fcnt_ta[i];
            cnt_info[j].othresh = vx_opt.ocnt_ta[i];
            cnt_info[j].logic   = vx_opt.cnt_logic;

            // Setup the CNTInfo alpha values
            cnt_info[j].allocate_n_alpha(vx_opt.get_n_ci_alpha());
            for(k=0; k<vx_opt.get_n_ci_alpha(); k++) {
               cnt_info[j].alpha[k] = vx_opt.ci_alpha[k];
            }

            // Compute the stats, normal confidence intervals, and
            // bootstrap confidence intervals
            if(vx_opt.boot_info.interval == BootIntervalType::BCA) {
               compute_cnt_stats_ci_bca(rng_ptr, pd,
                  precip_flag, vx_opt.rank_corr_flag,
                  vx_opt.boot_info.n_rep,
                  cnt_info[j], conf_info.tmp_dir.c_str());
            }
            else {
               compute_cnt_stats_ci_perc(rng_ptr, pd,
                  precip_flag, vx_opt.rank_corr_flag,
                  vx_opt.boot_info.n_rep,
                  vx_opt.boot_info.rep_prop,
                  cnt_info[j], conf_info.tmp_dir.c_str());
            }

            // Write out CNT
            if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
               vx_opt.output_flag[i_cnt] != STATOutputType::None &&
               cnt_info[j].n > 0) {

               write_cnt_row(shc, cnt_info[j],
                  vx_opt.output_flag[i_cnt], j, n_bin,
                  stat_at, i_stat_row, txt_at[i_cnt], i_txt_row[i_cnt]);
            }
         } // end if do_cnt
      } // end for j (n_bin)

      // Write the mean of the climo CDF bins
      if(n_bin > 1) {

         // Compute SL1L2 climo CDF bin means
         if(vx_opt.output_flag[i_sl1l2]  != STATOutputType::None ||
            vx_opt.output_flag[i_sal1l2] != STATOutputType::None) {

            SL1L2Info sl1l2_mean;
            compute_sl1l2_mean(sl1l2_info, n_bin, sl1l2_mean);

            // Write out SL1L2
            if(vx_opt.output_flag[i_sl1l2]  != STATOutputType::None &&
               sl1l2_mean.scount > 0) {

               write_sl1l2_row(shc, sl1l2_mean,
                  vx_opt.output_flag[i_sl1l2],
                  -1, n_bin, stat_at, i_stat_row,
                  txt_at[i_sl1l2], i_txt_row[i_sl1l2]);
            }

            // Write out SAL1L2
            if(vx_opt.output_flag[i_sal1l2] != STATOutputType::None &&
               sl1l2_mean.sacount > 0) {

               write_sal1l2_row(shc, sl1l2_mean,
                  vx_opt.output_flag[i_sal1l2],
                  -1, n_bin, stat_at, i_stat_row,
                  txt_at[i_sal1l2], i_txt_row[i_sal1l2]);
            }
         }

         // Compute CNT climo CDF bin means
         if(vx_opt.output_flag[i_cnt] != STATOutputType::None) {

            CNTInfo cnt_mean;
            compute_cnt_mean(cnt_info, n_bin, cnt_mean);

            if(cnt_mean.n > 0) {

               write_cnt_row(shc, cnt_mean,
                  vx_opt.output_flag[i_cnt],
                  -1, n_bin, stat_at, i_stat_row,
                  txt_at[i_cnt], i_txt_row[i_cnt]);
            }
         }
      } // end if n_bin > 1

   } // end for i (fcnt_ta)

   // Dealloate memory
   if(sl1l2_info) { delete [] sl1l2_info; sl1l2_info = (SL1L2Info *) nullptr; }
   if(cnt_info)   { delete [] cnt_info;   cnt_info   = (CNTInfo *)   nullptr;  }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_vl1l2(VL1L2Info *&v_info, int i_vx,
              const PairDataPoint *pd_u_ptr, const PairDataPoint *pd_v_ptr) {
   int i, j;

   mlog << Debug(2)
        << "Computing Vector Partial Sums and Continuous Vector Statistics.\n";

   //
   // Check that the number of pairs are the same
   //
   if(pd_u_ptr->n_obs != pd_v_ptr->n_obs) {
      mlog << Error << "\ndo_vl1l2() -> "
           << "unequal number of UGRD and VGRD pairs ("
           << pd_u_ptr->n_obs << " != " << pd_v_ptr->n_obs
           << ")\n\n";
      exit(1);
   }

   //
   // Set all of the VL1L2Info objects
   //
   for(i=0; i<conf_info.vx_opt[i_vx].fwind_ta.n(); i++) {

      //
      // Store thresholds
      //
      v_info[i].zero_out();
      v_info[i].fthresh = conf_info.vx_opt[i_vx].fwind_ta[i];
      v_info[i].othresh = conf_info.vx_opt[i_vx].owind_ta[i];
      v_info[i].logic   = conf_info.vx_opt[i_vx].wind_logic;
      v_info[i].allocate_n_alpha(conf_info.vx_opt[i_vx].get_n_ci_alpha());

      for(j=0; j<conf_info.vx_opt[i_vx].get_n_ci_alpha(); j++) {
         v_info[i].alpha[j] = conf_info.vx_opt[i_vx].ci_alpha[j];
      }

      //
      // Compute partial sums
      //
      v_info[i].set(*pd_u_ptr, *pd_v_ptr);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(const PairStatVxOpt &vx_opt, const PairDataPoint *pd_ptr) {
   int i, j, k, n_bin;
   PairDataPoint pd;
   PCTInfo *pct_info = (PCTInfo *) nullptr;

   mlog << Debug(2)
        << "Computing Probabilistic Statistics.\n";

   // Determine the number of observation climo CDF bins
   n_bin = (pd_ptr->ocmn_na.n_valid() > 0 &&
            pd_ptr->ocsd_na.n_valid() > 0 ?
            vx_opt.get_n_cdf_bin() : 1);

   if(n_bin > 1) {
      mlog << Debug(2)
           << "Applying " << n_bin << " climatology bins.\n";
   }

   // Allocate memory
   pct_info = new PCTInfo [n_bin];

   // Process each probabilistic observation threshold
   for(i=0; i<vx_opt.ocat_ta.n(); i++) {

      // Process the climo CDF bins
      for(j=0; j<n_bin; j++) {

         // Initialize
         pct_info[j].clear();

         // Apply climo CDF bins logic to subset pairs
         if(n_bin > 1) pd = subset_climo_cdf_bin(*pd_ptr,
                               vx_opt.cdf_info.cdf_ta, j);
         else          pd = *pd_ptr;

         // Store thresholds
         pct_info[j].fthresh = vx_opt.fcat_ta;
         pct_info[j].othresh = vx_opt.ocat_ta[i];
         pct_info[j].allocate_n_alpha(vx_opt.get_n_ci_alpha());

         for(k=0; k<vx_opt.get_n_ci_alpha(); k++) {
            pct_info[j].alpha[k] = vx_opt.ci_alpha[k];
         }

         // Compute the probabilistic counts and statistics
         compute_pctinfo(pd, (STATOutputType::None!=vx_opt.output_flag[i_pstd]), pct_info[j]);

         // Check for no matched pairs to process
         if(pd.n_obs == 0) continue;

         // Write out PCT
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_pct] != STATOutputType::None) {
            write_pct_row(shc, pct_info[j],
               vx_opt.output_flag[i_pct],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_pct], i_txt_row[i_pct]);
         }

         // Write out PSTD
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_pstd] != STATOutputType::None) {
            write_pstd_row(shc, pct_info[j],
               vx_opt.output_flag[i_pstd],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_pstd], i_txt_row[i_pstd]);
         }

         // Write out PJC
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_pjc] != STATOutputType::None) {
            write_pjc_row(shc, pct_info[j],
               vx_opt.output_flag[i_pjc],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_pjc], i_txt_row[i_pjc]);
         }

         // Write out PRC
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_prc] != STATOutputType::None) {
            write_prc_row(shc, pct_info[j],
               vx_opt.output_flag[i_prc],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_prc], i_txt_row[i_prc]);
         }

         // Write out ECLV
         if((n_bin == 1 || vx_opt.cdf_info.write_bins) &&
            vx_opt.output_flag[i_eclv] != STATOutputType::None) {
            write_eclv_row(shc, pct_info[j], vx_opt.eclv_points,
               vx_opt.output_flag[i_eclv],
               j, n_bin, stat_at, i_stat_row,
               txt_at[i_eclv], i_txt_row[i_eclv]);
         }
      } // end for j (n_bin)

      // Write the mean of the climo CDF bins
      if(n_bin > 1) {

         PCTInfo pct_mean;
         compute_pct_mean(pct_info, n_bin, pct_mean);

         // Write out PSTD
         if(vx_opt.output_flag[i_pstd] != STATOutputType::None) {
            write_pstd_row(shc, pct_mean,
               vx_opt.output_flag[i_pstd],
               -1, n_bin, stat_at, i_stat_row,
               txt_at[i_pstd], i_txt_row[i_pstd]);
         }
      } // end if n_bin > 1

   } // end for i (ocnt_ta)

   // Dealloate memory
   if(pct_info) { delete [] pct_info; pct_info = (PCTInfo *) nullptr; }

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
      if(conf_info.output_flag[i] == STATOutputType::Both) {

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

   // Close the output text files that were open for writing
   finish_txt_files();

   // Deallocate memory for data files
   if(fcst_mtddf) { delete fcst_mtddf; fcst_mtddf = (Met2dDataFile *) nullptr; }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-pairs file_1 ... file_n | file_list\n"
        << "\t-format type\n"
        << "\t-config config_file\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-pairs\" defines one or more input files containing "
        << "forecast/observation pairs. May be set as a list of file names "
        << "(file_1 ... file_n) or as an ASCII file containing a list of "
        << "file names (file_list). May be used multiple times (required)."

        << "\t\t\"-format type\" defines the input pairs file format "
        << "and may be set to \"mpr\" or \"ioda\" (required).\n"

        << "\t\t\"-config config_file\" is a PairStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-outdir path\" overrides the default output "
        << "directory (" << out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_pairs(const StringArray & a) {
   pairs_files.add(a);
}

////////////////////////////////////////////////////////////////////////

void set_format(const StringArray & a) {
   pairs_format = string_to_pairsformat(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a) {
   out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

