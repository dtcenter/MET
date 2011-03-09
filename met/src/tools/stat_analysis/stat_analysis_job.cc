// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   stat_analysis_job.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add aggregate RHIST lines and
//                    aggregate_stat ORANK to RHIST.
//   002    06/09/10  Halley Gotway   Add aggregate MCTC lines and
//                    aggregate_stat MCTC to MCTS.
//   003    06/21/10  Halley Gotway   Add support for vif_flag.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "stat_analysis_job.h"
#include "parse_stat_line.h"
#include "aggr_stat_line.h"

////////////////////////////////////////////////////////////////////////

void do_job(const char *jobstring, STATAnalysisJob &j, int n_job,
            const char *tmp_dir, const char *tmp_path,
            ofstream *sa_out, int verbosity) {
   LineDataFile f;
   int n_in, n_out;

   //
   // Open up the temp file for reading the intermediate STAT line data
   //
   if(!f.open(tmp_path)) {
      cerr << "\n\nERROR: do_job() -> "
           << "can't open the temporary file \"" << tmp_path
           << "\" for reading!\n\n" << flush;

      throw(1);
   }

   //
   // Initialize n_in and n_out to keep track of the number of lines
   // read and retained.
   //
   n_in = n_out = 0;

   if(verbosity > 1) {
      cout << "\nProcessing Job " << n_job << ": "
           << jobstring << "\n" << flush;
   }

   //
   // If the -dump_row option was supplied, open the file
   //
   if(j.dump_row) {

      if(verbosity > 0) {
         cout << "Creating STAT output file \"" << j.dump_row
              << "\"\n" << flush;
      }
      j.open_dump_row_file();
   }

   //
   // Switch on the job type
   //
   switch(j.job_type) {

      case(stat_job_filter):
         do_job_filter(jobstring, f, j, n_in, n_out, sa_out,
                       verbosity);
         break;

      case(stat_job_summary):
         do_job_summary(jobstring, f, j, n_in, n_out, sa_out,
                        verbosity);
         break;

      case(stat_job_aggr):
         do_job_aggr(jobstring, f, j, n_in, n_out, sa_out,
                     verbosity);
         break;

      case(stat_job_aggr_stat):
         do_job_aggr_stat(jobstring, f, j, n_in, n_out, sa_out,
                          tmp_dir, verbosity);
         break;

      case(stat_job_go_index):
         do_job_go_index(jobstring, f, j, n_in, n_out, sa_out,
                         verbosity);
         break;

      default:
         cerr << "\n\nERROR: do_job() -> "
              << "jobtype value of " << j.job_type
              << " not currently supported!\n\n" << flush;

         throw(1);
   }

   if(verbosity > 1) {
      cout << "Job " << n_job << " used " << n_out << " out of "
           << n_in << " STAT lines.\n" << flush;
   }

   //
   // If an output file was created, close it
   //
   if(j.dr_out) j.close_dump_row_file();

   //
   // Close the input file stream
   //
   f.close();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_filter() routine should only be called when the -dump_row
// option has been selected to dump out the filtered STAT lines.
//
////////////////////////////////////////////////////////////////////////

void do_job_filter(const char *jobstring, LineDataFile &f,
                   STATAnalysisJob &j, int &n_in, int &n_out,
                   ofstream *sa_out, int verbosity) {
   char out_line[max_line_len];
   STATLine line;

   //
   // Check that the -dump_row option has been supplied
   //
   if(!j.dump_row) {
      cerr << "\n\nERROR: do_job_filter()-> "
           << "this function may only be called when using the "
           << "-dump_row option in the job command line: "
           << jobstring << "\n\n" << flush;

      throw(1);
   }

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   //
   // Build a simple output line
   //
   sprintf(out_line, "%-15s %s", "FILTER:", jobstring);

   //
   // Write the filter line generated
   //
   write_line(out_line, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_summary() routine should only be called when the
// -line_type option has been used exactly once and the -column option
// has been used to specify the column of data to summarize.
//
////////////////////////////////////////////////////////////////////////

void do_job_summary(const char *jobstring, LineDataFile &f,
                    STATAnalysisJob &j, int &n_in, int &n_out,
                    ofstream *sa_out, int verbosity) {
   STATLine line;
   STATLineType lt;
   NumArray v_array;
   int offset;
   double v, min, v10, v25, v50, v75, v90, max;
   CIInfo mean_ci, stdev_ci;
   gsl_rng *rng_ptr = (gsl_rng *) 0;
   AsciiTable out_at;

   //
   // Check that the -line_type option has been supplied only once
   //
   if(j.line_type.n_elements() != 1) {
      cerr << "\n\nERROR: do_job_summary()-> "
           << "this function may only be called when the "
           << "\"-line_type\" option has been used exactly once to "
           << "specify a single line type from which to select a "
           << "statistic to summarize: " << jobstring << "\n\n"
           << flush;

         throw(1);
   }

   //
   // Check that the -column option has been supplied
   //
   if(j.column == (char *) 0) {
      cerr << "\n\nERROR: do_job_summary()-> "
           << "this function may only be called when the "
           << "\"-column\" option has been used to specify a "
           << "single column from which to select a statistic "
           << "to summarize: " << jobstring << "\n\n"
           << flush;

         throw(1);
   }

   //
   // Determine the line type
   //
   lt = string_to_statlinetype(j.line_type[0]);

   //
   // Based on the line type and the column name selected, determine
   // the column offset to use.
   //
   offset = determine_column_offset(lt, j.column);

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         v = atof(line.get_item(offset));

         if(!is_bad_data(v)) v_array.add(v);

         n_out++;
      }
   } // end while

   //
   // Check for no matching STAT lines
   //
   if(v_array.n_elements() == 0) {
      cout << "WARNING: do_job_summary() -> "
           << "no valid data found in the STAT lines for job: "
           << jobstring << "\n" << flush;
      return;
   }

   //
   // Compute the summary information for this collection of values:
   // min, max, v10, v25, v50, 75, v90
   //
   v_array.sort_array();
   min = v_array.percentile_array(0.00);
   v10 = v_array.percentile_array(0.10);
   v25 = v_array.percentile_array(0.25);
   v50 = v_array.percentile_array(0.50);
   v75 = v_array.percentile_array(0.75);
   v90 = v_array.percentile_array(0.90);
   max = v_array.percentile_array(1.00);

   //
   // Set up CIInfo objects
   //
   mean_ci.allocate_n_alpha(1);
   stdev_ci.allocate_n_alpha(1);

   //
   // Set up the random number generator and seed value
   //
   rng_set(rng_ptr, j.boot_rng, j.boot_seed);

   //
   // Compute a bootstrap confidence interval for the mean of this
   // array of values.
   //
   if(j.boot_interval == boot_bca_flag) {
      compute_mean_stdev_ci_bca(rng_ptr, v_array,
                                j.n_boot_rep,
                                j.out_alpha, mean_ci, stdev_ci);
   }
   else {
      compute_mean_stdev_ci_perc(rng_ptr, v_array,
                                 j.n_boot_rep, j.boot_rep_prop,
                                 j.out_alpha, mean_ci, stdev_ci);
   }

   //
   // Get the column names
   //
   out_at.set_size(2, n_job_sum_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(job_sum_columns, n_job_sum_columns,
                    0, out_at, 0, 1);

   //
   // Write the data row
   //
   out_at.set_entry(1, 0,  "SUMMARY:");
   out_at.set_entry(1, 1,  v_array.n_elements());
   out_at.set_entry(1, 2,  mean_ci.v);
   out_at.set_entry(1, 3,  mean_ci.v_ncl[0]);
   out_at.set_entry(1, 4,  mean_ci.v_ncu[0]);
   out_at.set_entry(1, 5,  mean_ci.v_bcl[0]);
   out_at.set_entry(1, 6,  mean_ci.v_bcu[0]);
   out_at.set_entry(1, 7,  stdev_ci.v);
   out_at.set_entry(1, 8,  stdev_ci.v_bcl[0]);
   out_at.set_entry(1, 9,  stdev_ci.v_bcu[0]);
   out_at.set_entry(1, 10, min);
   out_at.set_entry(1, 11, v10);
   out_at.set_entry(1, 12, v25);
   out_at.set_entry(1, 13, v50);
   out_at.set_entry(1, 14, v75);
   out_at.set_entry(1, 15, v90);
   out_at.set_entry(1, 16, max);

   //
   // Write the Ascii Table and the job command line
   //
   write_jobstring(jobstring, sa_out);
   write_table(out_at, sa_out);

   //
   // Deallocate memory for the random number generator
   //
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_aggr() routine should only be called when the -line_type
// option has been used exactly once.
//
////////////////////////////////////////////////////////////////////////

void do_job_aggr(const char *jobstring, LineDataFile &f,
                 STATAnalysisJob &j, int &n_in, int &n_out,
                 ofstream *sa_out, int verbosity) {
   STATLine line;
   STATLineType lt;

   CTSInfo     cts_info;
   MCTSInfo    mcts_info;
   CNTInfo     cnt_info;
   SL1L2Info   sl1l2_info;
   VL1L2Info   vl1l2_info;
   PCTInfo     pct_info;
   NBRCTSInfo  nbrcts_info;
   ISCInfo     isc_info;
   EnsPairData ens_pd;

   AsciiTable out_at;
   int i, n_thresh, n_cat;

   //
   // Check that the -line_type option has been supplied only once
   //
   if(j.line_type.n_elements() != 1) {
      cerr << "\n\nERROR: do_job_aggr()-> "
           << "this function may only be called when the "
           << "\"-line_type\" option has been used exactly once to "
           << "specify a single line type over which to perform the "
           << "aggregation: " << jobstring << "\n\n"
           << flush;

         throw(1);
   }

   //
   // Determine the line type specified for this job
   //
   lt = string_to_statlinetype(j.line_type[0]);

   //
   // Check that a valid line type has been selected
   //
   if(lt != stat_fho    && lt != stat_ctc   &&
      lt != stat_mctc   && lt != stat_sl1l2 &&
      lt != stat_sal1l2 && lt != stat_vl1l2 &&
      lt != stat_val1l2 && lt != stat_pct   &&
      lt != stat_nbrctc && lt != stat_isc   &&
      lt != stat_rhist) {
      cerr << "\n\nERROR: do_job_aggr()-> "
           << "the \"-line_type\" option must be set to one of:\n"
           << "\tFHO, CTC, MCTC,\n"
           << "\tSL1L2, SAL1L2, VL1L2, VAL1L2,\n"
           << "\tPCT, NBRCTC, ISC, RHIST\n\n"
           << flush;

      throw(1);
   }

   //
   // Turn off the vif_flag since it doesn't apply
   //
   j.vif_flag = 0;

   //
   // Sum up the contingency table type lines:
   //    FHO, CTC
   //
   if(lt == stat_fho ||
      lt == stat_ctc) {
      aggr_contable_lines(jobstring, f, j, cts_info,
                          lt, n_in, n_out, verbosity);
   }

   //
   // Sum up the mulit-category contingency table type lines:
   //    MCTC
   //
   else if(lt == stat_mctc) {
      aggr_mctc_lines(jobstring, f, j, mcts_info,
                      lt, n_in, n_out, verbosity);
   }

   //
   // Sum the partial sum line types:
   //    SL1L2, SAL1L2, VL1L2, VAL1L2
   //
   else if(lt == stat_sl1l2  ||
           lt == stat_sal1l2 ||
           lt == stat_vl1l2  ||
           lt == stat_val1l2) {
      aggr_partial_sum_lines(jobstring, f, j,
                             sl1l2_info, vl1l2_info, cnt_info,
                             lt, n_in, n_out, verbosity);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT
   //
   else if(lt == stat_pct) {
      aggr_nx2_contable_lines(jobstring, f, j, pct_info,
                              lt, n_in, n_out, verbosity);
   }

   //
   // Sum up the neighborhood contingency table lines:
   //    NBRCTC
   //
   else if(lt == stat_nbrctc) {
      aggr_contable_lines(jobstring, f, j, cts_info,
                          lt, n_in, n_out, verbosity);
      nbrcts_info.cts_info = cts_info;
   }

   //
   // Sum the ISC line types
   //
   else if(lt == stat_isc) {

      //
      // Check that the -line_type option has been supplied only once
      //
      if(j.vx_mask.n_elements() == 0) {
         cerr << "\n\nERROR: do_job_aggr()-> "
              << "when aggregating ISC lines you must select at least "
              << "one tile name to be used with the \"-vx_mask\" "
              << "option: " << jobstring << "\n\n"
              << flush;

            throw(1);
      }

      aggr_isc_lines(jobstring, f, j, isc_info, n_in, n_out, verbosity);
   }

   //
   // Sum the RHIST line types
   //
   else if(lt == stat_rhist) {
      aggr_rhist_lines(jobstring, f, j, ens_pd, n_in, n_out, verbosity);
   }

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      cout << "WARNING: do_job_aggr() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n" << flush;
      return;
   }

   //
   // Switch on the line type to determine the format of the output
   // line
   //
   switch(lt) {

      case(stat_fho):

         //
         // Get the column names
         //
         out_at.set_size(2, n_fho_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(fho_columns, n_fho_columns, 0,
                          out_at, 0, 1);

         //
         // Write the FHO row
         //
         out_at.set_entry(1, 0,  "FHO:");
         write_fho_cols(cts_info, out_at, 1, 1);

         break;

      case(stat_ctc):

         //
         // Get the column names
         //
         out_at.set_size(2, n_ctc_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(ctc_columns, n_ctc_columns, 0,
                          out_at, 0, 1);

         //
         // Write the CTC row
         //
         out_at.set_entry(1, 0,  "CTC:");
         write_ctc_cols(cts_info, out_at, 1, 1);

         break;

      case(stat_mctc):

         //
         // Get the column names
         //
         n_cat = mcts_info.cts.nrows();
         out_at.set_size(2, get_n_mctc_columns(n_cat)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(mctc_columns, n_mctc_columns, 0,
                          out_at, 0, 1);

         //
         // Write the MCTC row
         //
         out_at.set_entry(1, 0,  "MCTC:");
         write_mctc_cols(mcts_info, out_at, 1, 1);

         break;

      case(stat_sl1l2):

         //
         // Get the column names
         //
         out_at.set_size(2, n_sl1l2_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(sl1l2_columns, n_sl1l2_columns, 0,
                          out_at, 0, 1);

         //
         // Write the SL1L2 row
         //
         out_at.set_entry(1, 0,  "SL1L2:");
         write_sl1l2_cols(sl1l2_info, out_at, 1, 1);

         break;

      case(stat_sal1l2):

         //
         // Get the column names
         //
         out_at.set_size(2, n_sal1l2_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(sal1l2_columns, n_sal1l2_columns, 0,
                          out_at, 0, 1);

         //
         // Write the SAL1L2 row
         //
         out_at.set_entry(1, 0,  "SAL1L2:");
         write_sal1l2_cols(sl1l2_info, out_at, 1, 1);

         break;

      case(stat_vl1l2):

         //
         // Get the column names
         //
         out_at.set_size(2, n_vl1l2_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(vl1l2_columns, n_vl1l2_columns, 0,
                          out_at, 0, 1);

         //
         // Write the VL1L2 row
         //
         out_at.set_entry(1, 0,  "VL1L2:");
         write_vl1l2_cols(vl1l2_info, out_at, 1, 1);

         break;

      case(stat_val1l2):

         //
         // Get the column names
         //
         out_at.set_size(2, n_val1l2_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(val1l2_columns, n_val1l2_columns, 0,
                          out_at, 0, 1);

         //
         // Write the VAL1L2 row
         //
         out_at.set_entry(1, 0,  "VAL1L2:");
         write_val1l2_cols(vl1l2_info, out_at, 1, 1);

         break;

      case(stat_pct):

         //
         // Get the column names
         //
         n_thresh = pct_info.pct.nrows()+1;
         out_at.set_size(2, get_n_pct_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_pct_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PCT row
         //
         out_at.set_entry(1, 0,  "PCT:");
         write_pct_cols(pct_info, out_at, 1, 1);

         break;

      case(stat_nbrctc):

         //
         // Get the column names
         //
         out_at.set_size(2, n_nbrctc_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(nbrctc_columns, n_nbrctc_columns, 0,
                          out_at, 0, 1);

         //
         // Write the NBRCTC row
         //
         out_at.set_entry(1, 0,  "NBRCTC:");
         write_nbrctc_cols(nbrcts_info, out_at, 1, 1);

         break;

      case(stat_isc):

         //
         // Allocate rows for the binary field, all scales, and the
         // header row.
         //
         out_at.set_size(isc_info.n_scale+3, n_isc_columns+1);
         setup_table(out_at);

         //
         // Get the column names
         //
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(isc_columns, n_isc_columns, 0,
                          out_at, 0, 1);

         //
         // Write the ISC rows
         //
         out_at.set_entry(1, 0,  "ISC:");

         for(i=-1; i<=isc_info.n_scale; i++) {
            write_isc_cols(isc_info, i, out_at, i+2, 1);
         }

         break;

      case(stat_rhist):

         //
         // Get the column names
         //
         out_at.set_size(2, get_n_rhist_columns(ens_pd.rhist_na.n_elements())+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_rhist_header_row(0, ens_pd.rhist_na.n_elements(), out_at, 0, 1);

         //
         // Write the RHIST row
         //
         out_at.set_entry(1, 0,  "RHIST:");
         write_rhist_cols(&ens_pd, out_at, 1, 1);

         break;

      default:

         cerr << "\n\nERROR: do_job_aggr() -> "
              << "line type value of "
               << statlinetype_to_string(line.type())
              << " not currently supported for the aggregation "
              << "job!\n\n" << flush;

         throw(1);
   } // end switch

   //
   // Write the Ascii Table and the job command line
   //
   write_jobstring(jobstring, sa_out);
   write_table(out_at, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_aggr_stat() routine should only be called when the
// -line_type and -out_line_type options have been used exactly once.
//
////////////////////////////////////////////////////////////////////////

void do_job_aggr_stat(const char *jobstring, LineDataFile &f,
                      STATAnalysisJob &j, int &n_in, int &n_out,
                      ofstream *sa_out, const char *tmp_dir,
                      int verbosity) {
   STATLine line;
   STATLineType in_lt, out_lt;

   CTSInfo     cts_info;
   MCTSInfo    mcts_info;
   CNTInfo     cnt_info;
   SL1L2Info   sl1l2_info;
   VL1L2Info   vl1l2_info;
   PCTInfo     pct_info;
   NBRCTSInfo  nbrcts_info;
   ISCInfo     isc_info;
   NumArray    f_na, o_na, c_na;
   NumArray    uf_na, vf_na, uo_na, vo_na;
   EnsPairData ens_pd;

   int fcst_gc, obs_gc;
   AsciiTable out_at;
   int i, n;

   //
   // Check that the -line_type option has been supplied only once
   //
   if(j.line_type.n_elements() != 1) {
      cerr << "\n\nERROR: do_job_aggr_stat()-> "
           << "this function may only be called when the "
           << "\"-line_type\" option has been used exactly once to "
           << "specify a single line type over which to perform the "
           << "aggregation: " << jobstring << "\n\n"
           << flush;

         throw(1);
   }

   //
   // Determine the input and output line types for this job
   //
   in_lt  = string_to_statlinetype(j.line_type[0]);
   out_lt = j.out_line_type;

   //
   // Check for a valid combination of input and output line types
   //    -line_type FHO,   CTC,    -out_line_type CTS
   //    -line_type MCTC,          -out_line_type MCTS
   //    -line_type SL1L2, SAL1L2, -out_line_type CNT
   //    -line_type VL1L2, VAL1L2, -out_line_type WDIR (wind direction)
   //    -line_type PCT,           -out_line_type PSTD, PJC, PRC
   //    -line_type NBRCTC,        -out_line_type NBRCTS
   //    -line_type MPR,           -out_line_type FHO, CTC, CTS,
   //                                             MCTC, MCTS, CNT,
   //                                             SL1L2, SAL1L2,
   //                                             PCT, PSTD, PJC, PRC
   //    -line_type ORANK,         -out_line_type RHIST
   //
   if     ( (in_lt  == stat_fho   || in_lt  == stat_ctc) &&
            (out_lt == stat_cts)
          ) i = 1;
   else if( (in_lt  == stat_mctc) &&
            (out_lt == stat_mcts)
          ) i = 1;
   else if( (in_lt  == stat_sl1l2 || in_lt  == stat_sal1l2) &&
            (out_lt == stat_cnt)
          )  i = 1;
   else if( (in_lt  == stat_vl1l2 || in_lt  == stat_val1l2) &&
            (out_lt == stat_wdir)
          )  i = 1;
   else if( (in_lt  == stat_pct) &&
            (out_lt == stat_pstd  || out_lt == stat_pjc    ||
             out_lt == stat_prc)
          ) i = 1;
   else if( (in_lt  == stat_nbrctc) &&
            (out_lt == stat_nbrcts)
          ) i = 1;
   else if( (in_lt  == stat_mpr) &&
            (out_lt == stat_fho   || out_lt == stat_ctc    ||
             out_lt == stat_cts   || out_lt == stat_mctc   ||
             out_lt == stat_mcts  || out_lt == stat_cnt    ||
             out_lt == stat_sl1l2 || out_lt == stat_sal1l2 ||
             out_lt == stat_pct   || out_lt == stat_pstd   ||
             out_lt == stat_pjc   || out_lt == stat_prc)
          )  i = 1;
   else if( (in_lt  == stat_orank) &&
            (out_lt == stat_rhist)
          ) i = 1;
   else {

      cerr << "\n\nERROR: do_job_aggr_stat()-> "
           << "invalid combination of \"-line_type "
           << statlinetype_to_string(in_lt) << "\" and "
           << "\"-out_line_type " << statlinetype_to_string(out_lt)
           << "\"\n\n" << flush;

      throw(1);
   }

   //
   // Sum up the contingency table type lines:
   //    FHO, CTC
   //
   if(in_lt == stat_fho ||
      in_lt == stat_ctc) {
      aggr_contable_lines(jobstring, f, j, cts_info,
                          in_lt, n_in, n_out, verbosity);
   }

   //
   // Sum up the multi-category contingency table type lines:
   //    MCTC
   //
   else if(in_lt == stat_mctc) {
      aggr_mctc_lines(jobstring, f, j, mcts_info,
                      in_lt, n_in, n_out, verbosity);
   }

   //
   // Sum the scalar partial sum line types:
   //    SL1L2, SAL1L2
   //
   else if(in_lt == stat_sl1l2  ||
           in_lt == stat_sal1l2) {
      aggr_partial_sum_lines(jobstring, f, j,
                             sl1l2_info, vl1l2_info, cnt_info,
                             in_lt, n_in, n_out, verbosity);
   }

   //
   // Sum the vector partial sum line types:
   //    VL1L2, VAL1L2
   //
   else if(in_lt == stat_vl1l2 ||
           in_lt == stat_val1l2) {
      aggr_vl1l2_wdir(jobstring, f, j, vl1l2_info,
                      uf_na, vf_na, uo_na, vo_na,
                      in_lt, n_in, n_out, verbosity);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT
   //
   else if(in_lt == stat_pct) {
      aggr_nx2_contable_lines(jobstring, f, j, pct_info,
                              in_lt, n_in, n_out, verbosity);
   }

   //
   // Sum up the neighborhood contingency table lines:
   //    NBRCTC
   //
   else if(in_lt == stat_nbrctc) {
      aggr_contable_lines(jobstring, f, j, cts_info,
                          in_lt, n_in, n_out, verbosity);
      nbrcts_info.cts_info = cts_info;
   }

   //
   // Read the matched pair lines:
   //    MPR
   //
   else if(in_lt == stat_mpr) {

      //
      // Check output threshold values for 2x2 contingency table
      //
      if(out_lt == stat_fho ||
         out_lt == stat_ctc ||
         out_lt == stat_cts) {

         if(j.out_fcst_thresh.n_elements() != 1 ||
            j.out_obs_thresh.n_elements()  != 1) {
            cerr << "\n\nERROR: do_job_aggr_stat()-> "
                 << "when \"-out_line_type\" is set to FHO, CTC, or "
                 << "CTS the \"-out_fcst_thresh\" and "
                 << "\"-out_obs_thresh\" options must be specified "
                 << "exactly once.\n\n" << flush;

            throw(1);
         }
      }

      //
      // Check output threshold values for NxN contingency table
      //
      if(out_lt == stat_mctc ||
         out_lt == stat_mcts) {

         if(j.out_fcst_thresh.n_elements() <= 1 ||
            j.out_fcst_thresh.n_elements() != j.out_obs_thresh.n_elements()) {
            cerr << "\n\nERROR: do_job_aggr_stat()-> "
                 << "when \"-out_line_type\" is set to MCTC or MCTS "
                 << "the \"-out_fcst_thresh\" and "
                 << "\"-out_obs_thresh\" options must be specified "
                 << "the same number of times and at least twice.\n\n"
                 << flush;

            throw(1);
         }

         for(i=0; i<j.out_fcst_thresh.n_elements()-1; i++) {

            if(j.out_fcst_thresh[i].thresh >  j.out_fcst_thresh[i+1].thresh ||
               j.out_obs_thresh[i].thresh  >  j.out_obs_thresh[i+1].thresh  ||
               j.out_fcst_thresh[i].type   != j.out_fcst_thresh[i+1].type   ||
               j.out_obs_thresh[i].type    != j.out_obs_thresh[i+1].type    ||
               j.out_fcst_thresh[i].type   == thresh_eq              ||
               j.out_fcst_thresh[i].type   == thresh_ne              ||
               j.out_obs_thresh[i].type    == thresh_eq              ||
               j.out_obs_thresh[i].type    == thresh_ne) {

               cerr << "\n\nERROR: do_job_aggr_stat() -> "
                    << "when \"-out_line_type\" is set to MCTC or MCTS "
                    << "the thresholds must be monotonically "
                    << "increasing and be of the same inequality type "
                    << "(lt, le, gt, or ge).\n\n" << flush;
               exit(1);
            }
         } // end for
      }

      //
      // Check for output threshold values
      //
      if(out_lt == stat_pct  ||
         out_lt == stat_pstd ||
         out_lt == stat_pjc  ||
         out_lt == stat_prc) {

         if(j.out_obs_thresh.n_elements()  != 1) {
            cerr << "\n\nERROR: do_job_aggr_stat()-> "
                 << "when \"-out_line_type\" is set to PCT, PSTD, "
                 << "PJC, or PRC, the \"-out_obs_thresh\" option "
                 << "must be specified exactly once.\n\n" << flush;

            throw(1);
         }

         n = j.out_fcst_thresh.n_elements();

         // Check that the first threshold is 0 and the last is 1.
         if(n < 3 ||
            !is_eq(j.out_fcst_thresh[0].thresh,   0.0) ||
            !is_eq(j.out_fcst_thresh[n-1].thresh, 1.0)) {

            cerr << "\n\nERROR: do_job_aggr_stat() -> "
                 << "When verifying a probability field, you must "
                 << "use the \"-out_fcst_thresh\" option to select "
                 << "at least 3 probability thresholds beginning with "
                 << "0.0 and ending with 1.0.\n\n"
                 << flush;
            throw(1);
         }

         for(i=0; i<n; i++) {

            // Check that all threshold types are >=
            if(j.out_fcst_thresh[i].type != thresh_ge) {
               cerr << "\n\nERROR: do_job_aggr_stat() -> "
                    << "When verifying a probability field, all "
                    << "forecast probability thresholds must be set "
                    << "as greater than or equal to with \"ge\" or "
                    << "\"=\".\n\n"
                    << flush;
               throw(1);
            }

            // Check that all thresholds are in [0, 1].
            if(j.out_fcst_thresh[i].thresh < 0.0 ||
               j.out_fcst_thresh[i].thresh > 1.0) {

               cerr << "\n\nERROR: do_job_aggr_stat() -> "
                    << "When verifying a probability field, all "
                    << "forecast probability thresholds must be "
                    << "between 0 and 1.\n\n"
                    << flush;
               throw(1);
            }
         } // end for i
      }

      //
      // Parse the input MPR lines
      //
      read_mpr_lines(jobstring, f, j, fcst_gc, obs_gc,
                     f_na, o_na, c_na,
                     n_in, n_out, verbosity);

      //
      // When -out_line_type FHO, CTC
      //
      if(out_lt == stat_fho ||
         out_lt == stat_ctc) {
         aggr_mpr_lines_ctc(j, f_na, o_na, cts_info);
      }
      //
      // When -out_line_type CTS
      //
      else if(out_lt == stat_cts) {
         aggr_mpr_lines_cts(j, f_na, o_na, cts_info, tmp_dir);
      }
      //
      // When -out_line_type MCTC
      //
      else if(out_lt == stat_mctc) {
         aggr_mpr_lines_mctc(j, f_na, o_na, mcts_info);
      }
      //
      // When -out_line_type MCTS
      //
      else if(out_lt == stat_mcts) {
         aggr_mpr_lines_mcts(j, f_na, o_na, mcts_info, tmp_dir);
      }
      //
      // When -out_line_type CNT
      //
      else if(out_lt == stat_cnt) {
         aggr_mpr_lines_cnt(j, fcst_gc, obs_gc, f_na, o_na, cnt_info, tmp_dir);
      }
      //
      // When -out_line_type SL1L2, SAL1L2
      //
      else if(out_lt == stat_sl1l2 ||
              out_lt == stat_sal1l2) {
         aggr_mpr_lines_psums(j, f_na, o_na, c_na, sl1l2_info);
      }
      //
      // When -out_line_type PCT, PSTD, PJC, PRC
      //
      else if(out_lt == stat_pct  ||
              out_lt == stat_pstd ||
              out_lt == stat_pjc  ||
              out_lt == stat_prc) {
         aggr_mpr_lines_pct(j, f_na, o_na, pct_info);
      }
   }

   //
   // Compute a ranked histogram from the observation ranks lines.
   //
   else if(in_lt == stat_orank) {
      aggr_orank_lines(jobstring, f, j, ens_pd,
                       n_in, n_out, verbosity);
   }

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      cout << "WARNING: do_job_aggr_stat() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n" << flush;
      return;
   }

   //
   // Write output line for each supported job type
   //
   if(in_lt == stat_fho ||
      in_lt == stat_ctc) {
      write_job_cts(j, in_lt, cts_info, out_at);
   }
   else if(in_lt == stat_mctc) {
      write_job_mcts(j, in_lt, mcts_info, out_at);
   }
   else if(in_lt == stat_sl1l2 ||
           in_lt == stat_sal1l2) {
      write_job_cnt(j, in_lt, sl1l2_info, cnt_info, out_at);
   }
   else if(in_lt == stat_vl1l2  ||
           in_lt == stat_val1l2) {
      write_job_wdir(j, in_lt, vl1l2_info,
                     uf_na, vf_na, uo_na, vo_na, out_at);
   }
   else if(in_lt == stat_pct) {
      write_job_pct(j, in_lt, pct_info, out_at);
   }
   else if(in_lt == stat_nbrctc) {
      write_job_nbrcts(j, in_lt, nbrcts_info, out_at);
   }
   else if(in_lt == stat_mpr) {
      write_job_mpr(j, in_lt, cts_info, mcts_info, cnt_info,
                    sl1l2_info, pct_info, out_at);
   }
   else if(in_lt == stat_orank) {

      //
      // Get the column names
      //
      out_at.set_size(2, get_n_rhist_columns(ens_pd.rhist_na.n_elements())+1);
      setup_table(out_at);
      out_at.set_entry(0, 0,  "COL_NAME:");
      write_rhist_header_row(0, ens_pd.rhist_na.n_elements(), out_at, 0, 1);

      //
      // Write the RHIST row
      //
      out_at.set_entry(1, 0,  "RHIST:");
      write_rhist_cols(&ens_pd, out_at, 1, 1);
   }

   //
   // Write the Ascii Table and the job command line
   //
   write_jobstring(jobstring, sa_out);
   write_table(out_at, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_cts(STATAnalysisJob &j, STATLineType in_lt,
                   CTSInfo &cts_info, AsciiTable &out_at) {

   //
   // Store the alpha information in the CTSInfo object
   //
   cts_info.allocate_n_alpha(1);
   cts_info.alpha[0] = j.out_alpha;

   //
   // Compute the stats and confidence intervals for this
   // CTSInfo object
   //
   cts_info.compute_stats();
   cts_info.compute_ci();

   //
   // Get the column names
   //
   out_at.set_size(2, n_cts_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(cts_columns, n_cts_columns, 0,
                    out_at, 0, 1);

   //
   // Write the CTS row
   //
   out_at.set_entry(1, 0,  "CTS:");
   write_cts_cols(cts_info, 0, out_at, 1, 1);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_mcts(STATAnalysisJob &j, STATLineType in_lt,
                    MCTSInfo &mcts_info, AsciiTable &out_at) {

   //
   // Store the alpha information in the CTSInfo object
   //
   mcts_info.allocate_n_alpha(1);
   mcts_info.alpha[0] = j.out_alpha;

   //
   // Compute the stats and confidence intervals for this
   // MCTSInfo object
   //
   mcts_info.compute_stats();
   mcts_info.compute_ci();

   //
   // Get the column names
   //
   out_at.set_size(2, n_mcts_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(mcts_columns, n_mcts_columns, 0,
                    out_at, 0, 1);

   //
   // Write the MCTS row
   //
   out_at.set_entry(1, 0,  "MCTS:");
   write_mcts_cols(mcts_info, 0, out_at, 1, 1);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_cnt(STATAnalysisJob &j, STATLineType in_lt,
                   SL1L2Info &sl1l2_info, CNTInfo &cnt_info,
                   AsciiTable &out_at) {

   //
   // Allocate space for confidence intervals
   //
   cnt_info.allocate_n_alpha(1);
   cnt_info.alpha[0] = j.out_alpha;

   //
   // Compute CNTInfo statistics from the aggregated partial sums
   //
   switch(in_lt) {

      case(stat_sl1l2):
         compute_cntinfo(sl1l2_info, 0, cnt_info);
         break;

      case(stat_sal1l2):
         compute_cntinfo(sl1l2_info, 1, cnt_info);
         break;

      default:
         cerr << "\n\nERROR: write_job_cnt() -> "
              << "unexpected line type value of " << in_lt
              << "\n\n" << flush;

         throw(1);
   } // end switch

   //
   // Get the column names
   //
   out_at.set_size(2, n_cnt_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(cnt_columns, n_cnt_columns, 0,
                    out_at, 0, 1);

   //
   // Write the CNT row
   //
   out_at.set_entry(1, 0,  "CNT:");
   write_cnt_cols(cnt_info, 0, out_at, 1, 1);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_wdir(STATAnalysisJob &j, STATLineType in_lt,
                    VL1L2Info &vl1l2_info,
                    NumArray &uf_na, NumArray &vf_na,
                    NumArray &uo_na, NumArray &vo_na,
                    AsciiTable &out_at) {
   int i, n, n_out;
   double uf, vf, uo, vo, angle;
   double fbar, obar, me, mae;

   if(uf_na.n_elements() != vf_na.n_elements() ||
      uo_na.n_elements() != vo_na.n_elements() ||
      uf_na.n_elements() != uo_na.n_elements()) {

      cerr << "\n\nERROR: write_job_wdir()-> "
           << "the number of U and V forecast and observation points "
           << "must be the same.\n\n" << flush;
      throw(1);
   }

   n_out = uf_na.n_elements();

   //
   // Get the column names
   //
   out_at.set_size(3, n_job_wdir_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(job_wdir_columns, n_job_wdir_columns, 0,
                    out_at, 0, 1);

   //
   // Compute the mean forecast and observation angles
   // from the unit vectors
   //
   uf   = uf_na.sum()/uf_na.n_elements();
   vf   = vf_na.sum()/vf_na.n_elements();
   fbar = convert_u_v_to_wdir(uf, vf);
   uo   = uo_na.sum()/uo_na.n_elements();
   vo   = vo_na.sum()/vo_na.n_elements();
   obar = convert_u_v_to_wdir(uo, vo);

   //
   // Compute the mean error and the mean absolute error
   // from the unit vectors
   //
   me = mae = 0.0;
   for(i=0; i<n_out; i++) {

      angle = angle_difference(uf_na[i], vf_na[i],
                               uo_na[i], vo_na[i]);

      // Check for bad data
      if(is_eq(angle, bad_data_double)) {
         me  = bad_data_double;
         mae = bad_data_double;
         break;
      }

      me   += angle;
      mae  += abs(angle);
   }
   if(me  != bad_data_double) me  /= n_out;
   if(mae != bad_data_double) mae /= n_out;

   //
   // Write the mean wind direction statistics
   //
   out_at.set_entry(1, 0, "ROW_MEAN_WDIR:");
   out_at.set_entry(1, 1, n_out);
   out_at.set_entry(1, 2, fbar);
   out_at.set_entry(1, 3, obar);
   out_at.set_entry(1, 4, me);
   out_at.set_entry(1, 5, mae);

   switch(in_lt) {

      case(stat_vl1l2):
         uf = vl1l2_info.ufbar;
         vf = vl1l2_info.vfbar;
         uo = vl1l2_info.uobar;
         vo = vl1l2_info.vobar;
         n  = vl1l2_info.vcount;
         break;

      case(stat_val1l2):
         uf = vl1l2_info.ufabar;
         vf = vl1l2_info.vfabar;
         uo = vl1l2_info.uoabar;
         vo = vl1l2_info.voabar;
         n  = vl1l2_info.vacount;
         break;

      default:
         cerr << "\n\nERROR: write_job_wdir() -> "
              << "unexpected line type value of " << in_lt
              << "\n\n" << flush;

         throw(1);
   } // end switch

   //
   // Compute the aggregated forecast and observation angles
   //
   fbar = convert_u_v_to_wdir(uf, vf);
   obar = convert_u_v_to_wdir(uo, vo);
   me   = angle_difference(uf, vf, uo, vo);
   mae  = bad_data_double;

   //
   // Write the aggregated wind direction statistics
   //
   out_at.set_entry(2, 0, "AGGR_WDIR:");
   out_at.set_entry(2, 1, n);
   out_at.set_entry(2, 2, fbar);
   out_at.set_entry(2, 3, obar);
   out_at.set_entry(2, 4, me);
   out_at.set_entry(2, 5, mae);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_pct(STATAnalysisJob &j, STATLineType in_lt,
                   PCTInfo &pct_info, AsciiTable &out_at) {
   int n_thresh = pct_info.pct.nrows()+1;;

   switch(j.out_line_type) {

      case(stat_pstd):

         //
         // Store the alpha information in the PCTInfo object
         //
         pct_info.allocate_n_alpha(1);
         pct_info.alpha[0] = j.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // PCTInfo object
         //
         pct_info.compute_stats();
         pct_info.compute_ci();

         //
         // Get the column names
         //
         out_at.set_size(2, get_n_pstd_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_pstd_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PSTD row
         //
         out_at.set_entry(1, 0,  "PSTD:");
         write_pstd_cols(pct_info, 0, out_at, 1, 1);

         break;

      case(stat_pjc):

         //
         // Get the column names
         //
         out_at.set_size(2, get_n_pjc_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_pjc_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PSTD row
         //
         out_at.set_entry(1, 0,  "PJC:");
         write_pjc_cols(pct_info, out_at, 1, 1);

         break;

     case(stat_prc):

         //
         // Get the column names
         //
         out_at.set_size(2, get_n_prc_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_prc_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PRC row
         //
         out_at.set_entry(1, 0,  "PRC:");
         write_prc_cols(pct_info, out_at, 1, 1);

         break;

      default:
         cerr << "\n\nERROR: write_job_pct() -> "
              << "unexpected line type value of " << in_lt
              << "\n\n" << flush;

         throw(1);
   } // end switch

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_nbrcts(STATAnalysisJob &j, STATLineType in_lt,
                      NBRCTSInfo &nbrcts_info, AsciiTable &out_at) {

   //
   // Store the alpha information in the NBRCTSInfo object
   //
   nbrcts_info.cts_info.allocate_n_alpha(1);
   nbrcts_info.cts_info.alpha[0] = j.out_alpha;

   //
   // Compute the stats and confidence intervals for this
   // NBRCTSInfo object
   //
   nbrcts_info.cts_info.compute_stats();
   nbrcts_info.cts_info.compute_ci();

   //
   // Get the column names
   //
   out_at.set_size(2, n_nbrcts_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(nbrcts_columns, n_nbrcts_columns, 0,
                    out_at, 0, 1);

   //
   // Write the NBRCTS row
   //
   out_at.set_entry(1, 0,  "NBRCTS:");
   write_nbrcts_cols(nbrcts_info, 0, out_at, 1, 1);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_mpr(STATAnalysisJob &j, STATLineType in_lt,
                   CTSInfo &cts_info, MCTSInfo &mcts_info,
                   CNTInfo &cnt_info, SL1L2Info &sl1l2_info,
                   PCTInfo &pct_info, AsciiTable &out_at) {
   int n_thresh, n_cat;

   //
   // Construct the output line
   //
   switch(j.out_line_type) {

      case(stat_fho):

         //
         // Get the column names
         //
         out_at.set_size(2, n_fho_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(fho_columns, n_fho_columns, 0,
                          out_at, 0, 1);

         //
         // Write the FHO row
         //
         out_at.set_entry(1, 0,  "FHO:");
         write_fho_cols(cts_info, out_at, 1, 1);

         break;

      case(stat_ctc):

         //
         // Get the column names
         //
         out_at.set_size(2, n_ctc_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(ctc_columns, n_ctc_columns, 0,
                          out_at, 0, 1);

         //
         // Write the CTC row
         //
         out_at.set_entry(1, 0,  "CTC:");
         write_ctc_cols(cts_info, out_at, 1, 1);

         break;

      case(stat_cts):

         //
         // Get the column names
         //
         out_at.set_size(2, n_cts_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(cts_columns, n_cts_columns, 0,
                          out_at, 0, 1);

         //
         // Write the CTS row
         //
         out_at.set_entry(1, 0,  "CTS:");
         write_cts_cols(cts_info, 0, out_at, 1, 1);

         break;

      case(stat_mctc):

         //
         // Get the column names
         //
         n_cat = mcts_info.cts.nrows();
         out_at.set_size(2, get_n_mctc_columns(n_cat)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(mctc_columns, n_mctc_columns, 0,
                          out_at, 0, 1);

         //
         // Write the MCTC row
         //
         out_at.set_entry(1, 0,  "MCTC:");
         write_mctc_cols(mcts_info, out_at, 1, 1);

         break;

      case(stat_mcts):

         //
         // Get the column names
         //
         out_at.set_size(2, n_mcts_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(mcts_columns, n_mcts_columns, 0,
                          out_at, 0, 1);

         //
         // Write the MCTS row
         //
         out_at.set_entry(1, 0,  "MCTS:");
         write_mcts_cols(mcts_info, 0, out_at, 1, 1);

         break;

      case(stat_cnt):

         //
         // Get the column names
         //
         out_at.set_size(2, n_cnt_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(cnt_columns, n_cnt_columns, 0,
                          out_at, 0, 1);

         //
         // Write the CNT row
         //
         out_at.set_entry(1, 0,  "CNT:");
         write_cnt_cols(cnt_info, 0, out_at, 1, 1);

         break;

      case(stat_sl1l2):

         //
         // Get the column names
         //
         out_at.set_size(2, n_sl1l2_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(sl1l2_columns, n_sl1l2_columns, 0,
                          out_at, 0, 1);

         //
         // Write the SL1L2 row
         //
         out_at.set_entry(1, 0,  "SL1L2:");
         write_sl1l2_cols(sl1l2_info, out_at, 1, 1);

         break;

      case(stat_sal1l2):

         //
         // Get the column names
         //
         out_at.set_size(2, n_sal1l2_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(sal1l2_columns, n_sal1l2_columns, 0,
                          out_at, 0, 1);

         //
         // Write the SAL1L2 row
         //
         out_at.set_entry(1, 0,  "SAL1L2:");
         write_sal1l2_cols(sl1l2_info, out_at, 1, 1);

         break;

      case(stat_pct):

         //
         // Get the column names
         //
         n_thresh = pct_info.pct.nrows()+1;
         out_at.set_size(2, get_n_pct_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_pct_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PCT row
         //
         out_at.set_entry(1, 0,  "PCT:");
         write_pct_cols(pct_info, out_at, 1, 1);

         break;

      case(stat_pstd):

         //
         // Get the column names
         //
         n_thresh = pct_info.pct.nrows()+1;
         out_at.set_size(2, get_n_pstd_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_pstd_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PSTD row
         //
         out_at.set_entry(1, 0,  "PSTD:");
         write_pstd_cols(pct_info, 0, out_at, 1, 1);

         break;

      case(stat_pjc):

         //
         // Get the column names
         //
         n_thresh = pct_info.pct.nrows()+1;
         out_at.set_size(2, get_n_pjc_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_pjc_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PJC row
         //
         out_at.set_entry(1, 0,  "PJC:");
         write_pjc_cols(pct_info, out_at, 1, 1);

         break;

      case(stat_prc):

         //
         // Get the column names
         //
         n_thresh = pct_info.pct.nrows()+1;
         out_at.set_size(2, get_n_prc_columns(n_thresh)+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_prc_header_row(0, n_thresh, out_at, 0, 1);

         //
         // Write the PRC row
         //
         out_at.set_entry(1, 0,  "PRC:");
         write_prc_cols(pct_info, out_at, 1, 1);

         break;

      default:

         cerr << "\n\nERROR: write_job_mpr() -> "
              << "line type value of " << j.out_line_type
              << " not currently supported for the aggregation "
              << "job!\n\n" << flush;

         throw(1);
   } // end switch

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_go_index() routine is used to compute the GO Index.
// The GO Index is a weighted sum of the RMSE's for 12 different
// forecast parameters with lead times of 12, 24, 36, and 48 hours.
//
// The weights are based on lead time and are one of two sets of
// values:
// a: 4, 3, 2, 1 for lead times of 12, 24, 36, and 48 hours
// b: 8, 6, 4, 2 for lead times of 12, 24, 36, and 48 hours
//
// The 12 forecast parameters and corresponding sets of weights are
// as follows:
// (4) Wind Speed at the surface (b), 850 mb (a), 400 mb (a),
//     and 250 mb (a)
// (4) Dewpoint at the surface (b), 850 mb (b), 700 mb (b),
//     and 400 mb (b)
// (2) Temperature at the surface(b) and 400 mb (a)
// (1) Geopotential height at 400 mb (a)
// (1) Sea Level Pressure (b)
//
// The RMSE values are computed from the partial sums in the SL1L2
// STAT lines as follows:
// rmse = sqrt( (sum(f*f) + sum(o*o) - 2*sum(f*o))/N )
//
////////////////////////////////////////////////////////////////////////

void do_job_go_index(const char *jobstring, LineDataFile &f,
                     STATAnalysisJob &j, int &n_in, int &n_out,
                     ofstream *sa_out, int verbosity) {
   STATLine line;
   double go_index;
   AsciiTable out_at;

   //
   // When computing the GO Index, a single initialization time must
   // be set for the job. i.e. fcst_init_beg = fcst_init_end
   //
   if(j.fcst_init_beg == 0 ||
      j.fcst_init_end == 0 ||
      j.fcst_init_beg != j.fcst_init_end) {
      cerr << "\n\nERROR: do_job_go_index() -> "
           << "when computing the GO Index, the -fcst_init_beg and "
           << "-fcst_init_end job command line options must be set to "
           << "the same value.\n\n"
           << "\n\n" << flush;

      throw(1);
   }

   //
   // Compute GO Index
   //
   go_index = compute_go_index(jobstring, f, j, n_in, n_out, verbosity);

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      cout << "WARNING: do_job_go_index() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n" << flush;
      return;
   }

   //
   // Get the column names
   //
   out_at.set_size(2, n_job_sum_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(job_go_columns, n_job_go_columns, 0, out_at, 0, 1);

   //
   // Write the data row
   //
   out_at.set_entry(1, 0,  "GO_INDEX:");
   out_at.set_entry(1, 1,  go_index);

   //
   // Write the Ascii Table and the job command line
   //
   write_jobstring(jobstring, sa_out);
   write_table(out_at, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Left-justify all columns
   at.set_table_just(LeftJust);

   // Right-justify the first column
   at.set_column_just(0, RightJust);

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

void write_table(AsciiTable &at, ofstream *sa_out) {

   if(sa_out) *(sa_out) << at << "\n" << flush;
   else       cout      << at << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_jobstring(const char *jobstring, ofstream *sa_out) {
   char out_line[max_line_len];

   sprintf(out_line, "%-15s %s", "JOB_LIST:", jobstring);

   write_line(out_line, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_line(const char *str, ofstream *sa_out) {

   if(sa_out) *(sa_out) << str << "\n" << flush;
   else       cout      << str << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_go_index(const char *jobstring, LineDataFile &f,
                        STATAnalysisJob &j, int &n_in, int &n_out,
                        int verbosity) {
   STATLine line;
   SL1L2Info s_info;
   int i;
   double go_index, go_sum, weight_sum;

   //
   // SL1L2Info objects to hold the counts for each variable and level
   // combination and each of the 4 lead times (12, 24, 36, and 48 hr)
   //
   SL1L2Info ws_sfc[4],      ws_850[4];
   double    ws_sfc_rmse[4], ws_850_rmse[4];
   SL1L2Info ws_400[4],      ws_250[4];
   double    ws_400_rmse[4], ws_250_rmse[4];
   SL1L2Info td_sfc[4],      td_850[4];
   double    td_sfc_rmse[4], td_850_rmse[4];
   SL1L2Info td_700[4],      td_400[4];
   double    td_700_rmse[4], td_400_rmse[4];
   SL1L2Info tt_sfc[4],      tt_400[4];
   double    tt_sfc_rmse[4], tt_400_rmse[4];
   SL1L2Info ht_400[4],      slp[4];
   double    ht_400_rmse[4], slp_rmse[4];

   const double a_weight[4] = { 4.0, 3.0, 2.0, 1.0 };
   const double b_weight[4] = { 8.0, 6.0, 4.0, 2.0 };

   //
   // Initialize the RMSE values
   //
   for(i=0; i<4; i++) {
      ws_sfc_rmse[i] = ws_850_rmse[i] = 0.0;
      ws_400_rmse[i] = ws_250_rmse[i] = 0.0;
      td_sfc_rmse[i] = td_850_rmse[i] = 0.0;
      td_700_rmse[i] = td_400_rmse[i] = 0.0;
      tt_sfc_rmse[i] = tt_400_rmse[i] = 0.0;
      ht_400_rmse[i] = slp_rmse   [i] = 0.0;
   }

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // The GO Index is computed only from STAT line type SL1L2.
         //
         if(string_to_statlinetype(line.line_type()) != stat_sl1l2 )
            continue;

         //
         // Determine the lead time.  The GO Index is only computed
         // using lead times of 12, 24, 36, and 48 hours.
         //
         if(     line.fcst_lead() == sec_per_hour * 12) i = 0;
         else if(line.fcst_lead() == sec_per_hour * 24) i = 1;
         else if(line.fcst_lead() == sec_per_hour * 36) i = 2;
         else if(line.fcst_lead() == sec_per_hour * 48) i = 3;
         else                                           continue;

         //
         // The GO Index is computed from the following 12 combinations
         // of variable type and level:
         // Wind Speed:    (4) WIND SFC (Z10), WIND P850, WIND P400,
         //                    WIND P250
         // Dew Point:     (4) DPT  SFC (Z2),  DPT  P850, DPT  P700,
         //                    DPT  P400
         // Temperature:   (2) TMP  SFC (Z2),  TMP  P400
         // Height:        (1) HGT  P400
         // Sea Level Prs: (1) PRMSL SFC (Z0)
         //

         //
         // Parse the line into an SL1L2Info object
         //
         parse_sl1l2_line(line, s_info);

         //
         // Handle the WIND variable type
         //
         if(strcmp(line.fcst_var(), "WIND") == 0 &&
            strcmp(line.obs_var(),  "WIND") == 0) {

            if(strcmp(line.fcst_lev(), "Z10") == 0 &&
               strcmp(line.obs_lev(),  "Z10") == 0)
               ws_sfc[i] += s_info;

            else if(strcmp(line.fcst_lev(), "P850") == 0 &&
                    strcmp(line.obs_lev(),  "P850") == 0)
               ws_850[i] += s_info;

            else if(strcmp(line.fcst_lev(), "P400") == 0 &&
                    strcmp(line.obs_lev(),  "P400") == 0)
               ws_400[i] += s_info;

            else if(strcmp(line.fcst_lev(), "P250") == 0 &&
                    strcmp(line.obs_lev(),  "P250") == 0)
               ws_250[i] += s_info;

            else
               continue;
         }

         //
         // Handle the DPT variable type
         //
         else if(strcmp(line.fcst_var(), "DPT") == 0 &&
                 strcmp(line.obs_var(),  "DPT") == 0) {

            if(strcmp(line.fcst_lev(), "Z2") == 0 &&
               strcmp(line.obs_lev(),  "Z2") == 0)
               td_sfc[i] += s_info;

            else if(strcmp(line.fcst_lev(), "P850") == 0 &&
                    strcmp(line.obs_lev(),  "P850") == 0)
               td_850[i] += s_info;

            else if(strcmp(line.fcst_lev(), "P700") == 0 &&
                    strcmp(line.obs_lev(),  "P700") == 0)
               td_700[i] += s_info;

            else if(strcmp(line.fcst_lev(), "P400") == 0 &&
                    strcmp(line.obs_lev(),  "P400") == 0)
               td_400[i] += s_info;

            else
               continue;
         }

         //
         // Handle the TMP variable type
         //
         else if(strcmp(line.fcst_var(), "TMP") == 0 &&
                 strcmp(line.obs_var(),  "TMP") == 0) {

            if(strcmp(line.fcst_lev(), "Z2") == 0 &&
               strcmp(line.obs_lev(),  "Z2") == 0)
               tt_sfc[i] += s_info;

            else if(strcmp(line.fcst_lev(), "P400") == 0 &&
                    strcmp(line.obs_lev(),  "P400") == 0)
               tt_400[i] += s_info;

            else
               continue;
         }

         //
         // Handle the HGT variable type
         //
         else if(strcmp(line.fcst_var(), "HGT") == 0 &&
                 strcmp(line.obs_var(),  "HGT") == 0) {

            if(strcmp(line.fcst_lev(), "P400") == 0 &&
               strcmp(line.obs_lev(),  "P400") == 0)
               ht_400[i] += s_info;

            else
               continue;
         }

         //
         // Handle the PRMSL variable type
         //
         else if(strcmp(line.fcst_var(), "PRMSL") == 0 &&
                 strcmp(line.obs_var(),  "PRMSL") == 0) {

            if(strcmp(line.fcst_lev(), "Z0") == 0 &&
               strcmp(line.obs_lev(),  "Z0") == 0)
               slp[i] += s_info;

            else
               continue;
         }

         //
         // Handle all other variable types
         //
         else
            continue;

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         n_out++;
      }
   } // end while

   //
   // Compute the RMSE values for each SL1L2Info object
   //
   for(i=0; i<4; i++) {
      ws_sfc_rmse[i] = compute_sl1l2_rmse(ws_sfc[i],
                                          "WIND", "Z10",  (i+1)*12);
      ws_850_rmse[i] = compute_sl1l2_rmse(ws_850[i],
                                          "WIND",  "P850", (i+1)*12);
      ws_400_rmse[i] = compute_sl1l2_rmse(ws_400[i],
                                          "WIND",  "P400", (i+1)*12);
      ws_250_rmse[i] = compute_sl1l2_rmse(ws_250[i],
                                          "WIND",  "P250", (i+1)*12);
      td_sfc_rmse[i] = compute_sl1l2_rmse(td_sfc[i],
                                          "DPT",   "Z2",   (i+1)*12);
      td_850_rmse[i] = compute_sl1l2_rmse(td_850[i],
                                          "DPT",   "P850", (i+1)*12);
      td_700_rmse[i] = compute_sl1l2_rmse(td_700[i],
                                          "DPT",   "P700", (i+1)*12);
      td_400_rmse[i] = compute_sl1l2_rmse(td_400[i],
                                          "DPT",   "P400", (i+1)*12);
      tt_sfc_rmse[i] = compute_sl1l2_rmse(tt_sfc[i],
                                          "TMP",   "Z2",   (i+1)*12);
      tt_400_rmse[i] = compute_sl1l2_rmse(tt_400[i],
                                          "TMP",   "P400", (i+1)*12);
      ht_400_rmse[i] = compute_sl1l2_rmse(ht_400[i],
                                          "HGT",   "P400", (i+1)*12);
      slp_rmse[i]    = compute_sl1l2_rmse(slp[i],
                                          "PRMSL", "Z0",   (i+1)*12);
   }

   //
   // Compute GO Index value
   //
   go_sum     = 0.0;
   weight_sum = 0.0;
   go_index   = 0.0;

   for(i=0; i<4; i++) {

      if(ws_sfc[i].scount > 0) {
         go_sum     += ws_sfc_rmse[i] * b_weight[i];
         weight_sum += b_weight[i];
      }

      if(ws_850[i].scount > 0) {
         go_sum     += ws_850_rmse[i] * a_weight[i];
         weight_sum += a_weight[i];
      }

      if(ws_400[i].scount > 0) {
         go_sum     += ws_400_rmse[i] * a_weight[i];
         weight_sum += a_weight[i];
      }

      if(ws_250[i].scount > 0) {
         go_sum     += ws_250_rmse[i] * a_weight[i];
         weight_sum += a_weight[i];
      }

      if(td_sfc[i].scount > 0) {
         go_sum     += td_sfc_rmse[i] * b_weight[i];
         weight_sum += b_weight[i];
      }

      if(td_850[i].scount > 0) {
         go_sum     += td_850_rmse[i] * b_weight[i];
         weight_sum += b_weight[i];
      }

      if(td_700[i].scount > 0) {
         go_sum     += td_700_rmse[i] * b_weight[i];
         weight_sum += b_weight[i];
      }

      if(td_400[i].scount > 0) {
         go_sum     += td_400_rmse[i] * b_weight[i];
         weight_sum += b_weight[i];
      }

      if(tt_sfc[i].scount > 0) {
         go_sum     += tt_sfc_rmse[i] * b_weight[i];
         weight_sum += b_weight[i];
      }

      if(tt_400[i].scount > 0) {
         go_sum     += tt_400_rmse[i] * a_weight[i];
         weight_sum += a_weight[i];
      }

      if(ht_400[i].scount > 0) {
         go_sum     += ht_400_rmse[i] * a_weight[i];
         weight_sum += a_weight[i];
      }

      if(slp[i].scount > 0) {
         go_sum     += slp_rmse[i] * b_weight[i];
         weight_sum += b_weight[i];
      }
   }
   if(is_eq(weight_sum, 0.0)) go_index = bad_data_double;
   else                       go_index = go_sum/weight_sum;

   return(go_index);
}

////////////////////////////////////////////////////////////////////////

double compute_sl1l2_rmse(SL1L2Info &s_info, const char *var,
                        const char *lvl, int lead_hr) {
   int n;
   double ff_sum, oo_sum, fo_sum, rmse;

   n = s_info.scount;

   if(n == 0) {
      cout << "WARNING: compute_sl1l2_rmse() -> "
           << "no matching STAT lines for GO Index variable \"" << var
           << "\" at level \"" << lvl << "\" for lead time of \""
           << lead_hr << "\" hours.\n" << flush;
      return(0.0);
   }

   ff_sum = s_info.ffbar*s_info.scount;
   oo_sum = s_info.oobar*s_info.scount;
   fo_sum = s_info.fobar*s_info.scount;

   rmse = sqrt( (ff_sum + oo_sum - 2.0*fo_sum)/(double) n);

   return(rmse);
}

////////////////////////////////////////////////////////////////////////
