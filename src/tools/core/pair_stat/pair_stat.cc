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
static void setup_pairs();

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
static void set_out(const StringArray &);

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

   // Default output file base
   out_base = "./pair_stat";

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
   cline.add(set_out,    "-out", 1);

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

   // Process the masks
   conf_info.process_masks();

   // Setup the VxPairDataPoint objects
   conf_info.set_vx_pd();

   // List the input pair files
   mlog << Debug(1)
        << "Reading " << pairs_files.n() << " \""
        << pairsformat_to_string(pairs_format) << "\" format pairs file(s): "
        << write_css(pairs_files) << "\n";

   // Use the first verification task to set the random number generator
   // and seed value for bootstrap confidence intervals
   rng_set(rng_ptr,
           conf_info.vx_opt[0].boot_info.rng.c_str(),
           conf_info.vx_opt[0].boot_info.seed.c_str());

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
   str << cs_erase << out_base;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void process_mpr_pairs(const ConcatString &file_name, PairsFormat format) {
   LineDataFile f;
   const char *method_name = "process_mpr_pairs() -> ";

   // TODO: Add support for -format python
   if(format == PairsFormat::Python) { 
      mlog << Error << "\nprocess_mpr_pairs() -> "
           << "the \"-format python\" option is not supported yet!\n\n";
      exit(1);
   }

   //
   // Open the input file
   //
   if(!f.open(file_name.c_str())) {
      mlog << Warning << "\n" << method_name
           << "can't open matched pair file \"" << file_name
           << "\" for reading!\n\n";
      return;
   }

   //
   // Count the number read and kept
   //
   int n_read = 0;
   int n_keep = 0;

   //
   // Process the STAT lines
   //
   STATLine line;
   while(f >> line) {

      // Skip header and non-MPR lines
      if(line.is_header() || line.type() != STATLineType::mpr) continue;

      n_read++;

      if(conf_info.add_mpr_line(line)) n_keep++;
   }
   
   mlog << Debug(3) << "Keeping " << n_keep << " of " << n_read
        << " MPR lines from \"" << file_name << "\".\n";
 
   return;
}

////////////////////////////////////////////////////////////////////////

void process_ioda_pairs(const ConcatString &file_name) {

   // TODO: Add support for -format ioda
   mlog << Error << "\nprocess_ioda_pairs() -> "
        << "the \"-format ioda\" option is not supported yet!\n\n";
   exit(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int n_cat, n_wind;
   ConcatString cs;

   // Initialize pointers
   CTSInfo   *cts_info   = (CTSInfo *)   nullptr;
   MCTSInfo   mcts_info;
   VL1L2Info *vl1l2_info = (VL1L2Info *) nullptr;

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
   int i_vx = -1;
   for(auto &vx : conf_info.vx_opt) {

      i_vx++;

      // Store masking region as the only "case" information
      StringArray empty_sa;
      StringArray case_cols;
      case_cols.add("VX_MASK");

      // Loop through the verification masking regions
      for(int i_mask=0; i_mask<vx.get_n_mask(); i_mask++) {

         // Retrieve the header based on the inputs
         ConcatString cur_case(vx.mask_name[i_mask]);
         StatHdrColumns in_shc = vx.vx_hdr[i_mask].get_shc(cur_case, case_cols,
                                    empty_sa, empty_sa, STATLineType::mpr);

         // Override header columns 
         if(conf_info.model.nonempty()) in_shc.set_model(conf_info.model.c_str());
         if(vx.vx_pd.desc.nonempty())   in_shc.set_desc(vx.vx_pd.desc.c_str());
         in_shc.set_mask(vx.mask_name[i_mask].c_str());
         shc = in_shc;

         PairDataPoint *pd_ptr = &vx.vx_pd.pd[i_mask];

         mlog << Debug(2)
              << "Processing " << vx.vx_pd.fcst_info->magic_str()
              << " versus " << vx.vx_pd.obs_info->magic_str()
              << ", over region " << pd_ptr->mask_name
              << ", using " << pd_ptr->n_obs << " matched pairs.\n";

         // Process percentile thresholds
         vx.set_perc_thresh(pd_ptr);

         // Write out the MPR lines
         if(vx.output_flag[i_mpr] != STATOutputType::None) {
            write_mpr_row(shc, pd_ptr,
               vx.output_flag[i_mpr],
               stat_at, i_stat_row,
               txt_at[i_mpr], i_txt_row[i_mpr], false);
            shc = in_shc;
         }

         // Write out the SEEPS MPR lines
         if(vx.output_flag[i_seeps_mpr] != STATOutputType::None) {
            write_seeps_mpr_row(shc, pd_ptr,
               vx.output_flag[i_seeps_mpr],
               stat_at, i_stat_row,
               txt_at[i_seeps_mpr], i_txt_row[i_seeps_mpr], false);
            shc = in_shc;
         }

         // Write out the SEEPS lines
         if(vx.output_flag[i_seeps] != STATOutputType::None) {
            compute_aggregated_seeps(pd_ptr, &pd_ptr->seeps_agg);
            write_seeps_row(shc, &pd_ptr->seeps_agg,
               vx.output_flag[i_seeps],
               stat_at, i_stat_row,
               txt_at[i_seeps], i_txt_row[i_seeps]);
            shc = in_shc;
         }

         // Compute CTS scores
         if(!vx.vx_pd.fcst_info->is_prob() &&
             vx.fcat_ta.n() > 0            &&
            (vx.output_flag[i_fho]  != STATOutputType::None ||
             vx.output_flag[i_ctc]  != STATOutputType::None ||
             vx.output_flag[i_cts]  != STATOutputType::None ||
             vx.output_flag[i_eclv] != STATOutputType::None)) {

            // Initialize
            for(int i_cat=0; i_cat<n_cat; i_cat++) cts_info[i_cat].clear();

            // Compute CTS Info
            do_cts(cts_info, i_vx, pd_ptr);

            // Loop through the categorical thresholds
            for(int i_cat=0; i_cat<vx.fcat_ta.n(); i_cat++) {

               if(cts_info[i_cat].cts.n_pairs() == 0) continue;

               // Write out FHO
               if(vx.output_flag[i_fho] != STATOutputType::None) {
                  write_fho_row(shc, cts_info[i_cat],
                     vx.output_flag[i_fho],
                     stat_at, i_stat_row,
                     txt_at[i_fho], i_txt_row[i_fho]);
                  shc = in_shc;
               }

               // Write out CTC
               if(vx.output_flag[i_ctc] != STATOutputType::None) {
                  write_ctc_row(shc, cts_info[i_cat],
                     vx.output_flag[i_ctc],
                     stat_at, i_stat_row,
                     txt_at[i_ctc], i_txt_row[i_ctc]);
                  shc = in_shc;
               }

               // Write out CTS
               if(vx.output_flag[i_cts] != STATOutputType::None) {
                  write_cts_row(shc, cts_info[i_cat],
                     vx.output_flag[i_cts],
                     stat_at, i_stat_row,
                     txt_at[i_cts], i_txt_row[i_cts]);
                  shc = in_shc;
               }

               // Write out ECLV
               if(vx.output_flag[i_eclv] != STATOutputType::None) {
                  write_eclv_row(shc, cts_info[i_cat], vx.eclv_points,
                     vx.output_flag[i_eclv],
                     stat_at, i_stat_row,
                     txt_at[i_eclv], i_txt_row[i_eclv]);
                  shc = in_shc;
               }
            } // end for i_cat 
         } // end Compute CTS scores

         // Compute MCTS scores
         if(!vx.vx_pd.fcst_info->is_prob() &&
             vx.fcat_ta.n() > 1            &&
            (vx.output_flag[i_mctc] != STATOutputType::None ||
             vx.output_flag[i_mcts] != STATOutputType::None)) {

            // Initialize
            mcts_info.clear();

            // Compute MCTS Info
            do_mcts(mcts_info, i_vx, pd_ptr);

            if(mcts_info.cts.n_pairs() == 0) continue;

            // Write out MCTC
            if(vx.output_flag[i_mctc] != STATOutputType::None) {
               write_mctc_row(shc, mcts_info,
                  vx.output_flag[i_mctc],
                  stat_at, i_stat_row,
                  txt_at[i_mctc], i_txt_row[i_mctc]);
               shc = in_shc;
            }

            // Write out MCTS
            if(vx.output_flag[i_mcts] != STATOutputType::None) {
               write_mcts_row(shc, mcts_info,
                  vx.output_flag[i_mcts],
                  stat_at, i_stat_row,
                  txt_at[i_mcts], i_txt_row[i_mcts]);
               shc = in_shc;
            }
         } // end Compute MCTS scores

         // Compute CNT, SL1L2, and SAL1L2 scores
         if(!vx.vx_pd.fcst_info->is_prob() &&
            (vx.output_flag[i_cnt]    != STATOutputType::None ||
             vx.output_flag[i_sl1l2]  != STATOutputType::None ||
             vx.output_flag[i_sal1l2] != STATOutputType::None)) {
             do_cnt_sl1l2(vx, pd_ptr);
         }

         // Compute VL1L2 and VAL1L2 partial sums for UGRD and VGRD
         if(!vx.vx_pd.fcst_info->is_prob()       &&
             vx.vx_pd.fcst_info->is_v_wind()     &&
             vx.vx_pd.fcst_info->uv_index() >= 0 &&
            (vx.output_flag[i_vl1l2]  != STATOutputType::None ||
             vx.output_flag[i_val1l2] != STATOutputType::None ||
             vx.output_flag[i_vcnt]   != STATOutputType::None)) {

            // Store the forecast variable name
            shc.set_fcst_var(ugrd_vgrd_abbr_str);

            // Store the observation variable name
            shc.set_obs_var(ugrd_vgrd_abbr_str);

            // Initialize
            for(int i_wind=0; i_wind<n_wind; i_wind++) vl1l2_info[i_wind].clear();

            // Get the index of the matching u-component
            int u_vx = vx.vx_pd.fcst_info->uv_index();

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
            for(int i_wind=0; i_wind<vx.fwind_ta.n(); i_wind++) {

               // Write out VL1L2
               if(vx.output_flag[i_vl1l2] != STATOutputType::None &&
                  vl1l2_info[i_wind].vcount > 0) {
                  write_vl1l2_row(shc, vl1l2_info[i_wind],
                     vx.output_flag[i_vl1l2],
                     stat_at, i_stat_row,
                     txt_at[i_vl1l2], i_txt_row[i_vl1l2]);
               }

               // Write out VAL1L2
               if(vx.output_flag[i_val1l2] != STATOutputType::None &&
                  vl1l2_info[i_wind].vacount > 0) {
                  write_val1l2_row(shc, vl1l2_info[i_wind],
                     vx.output_flag[i_val1l2],
                     stat_at, i_stat_row,
                     txt_at[i_val1l2], i_txt_row[i_val1l2]);
               }

               // Write out VCNT
               if(vx.output_flag[i_vcnt] != STATOutputType::None &&
                  vl1l2_info[i_wind].vcount > 0) {
                  write_vcnt_row(shc, vl1l2_info[i_wind],
                     vx.output_flag[i_vcnt],
                     stat_at, i_stat_row,
                     txt_at[i_vcnt], i_txt_row[i_vcnt]);
              }
           } // end for i_wind

           shc = in_shc;
        } // end Compute VL1L2 and VAL1L2

        // Compute PCT counts and scores
        if(vx.vx_pd.fcst_info->is_prob() &&
           (vx.output_flag[i_pct]  != STATOutputType::None ||
            vx.output_flag[i_pstd] != STATOutputType::None ||
            vx.output_flag[i_pjc]  != STATOutputType::None ||
            vx.output_flag[i_prc]  != STATOutputType::None ||
            vx.output_flag[i_eclv] != STATOutputType::None)) {
            do_pct(conf_info.vx_opt[i_vx], pd_ptr);
         }
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
        << "\t[-out base]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-pairs\" defines one or more input files containing "
        << "forecast/observation pairs (required).\n"
        << "\t\t  Set as a list of file names (file_1 ... file_n) or as an ASCII file list (file_list).\n"
        << "\t\t  May be used multiple times.\n"

        << "\t\t\"-format type\" defines the input pairs file format (required).\n"
        << "\t\t  May be set to \"mpr\", \"python\", or \"ioda\".\n"

        << "\t\t\"-config config_file\" is a PairStatConfig file containing "
        << "the desired configuration settings (required).\n"

        << "\t\t\"-out base\" overrides the default output "
        << "file base (./tc_gen) (optional).\n"

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

void set_out(const StringArray & a) {
   out_base = a[0];
}

////////////////////////////////////////////////////////////////////////

