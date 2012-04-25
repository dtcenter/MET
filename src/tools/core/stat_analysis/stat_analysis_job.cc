// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
//   004    08/16/11  Halley Gotway   Reimplementation of GO Index job
//                    with addition of generalized Skill Score Index
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>

#include "vx_log.h"

#include "stat_analysis_Conf.h"

#include "stat_analysis_job.h"
#include "parse_stat_line.h"
#include "aggr_stat_line.h"

////////////////////////////////////////////////////////////////////////

void set_job_from_config(stat_analysis_Conf &c, STATAnalysisJob &j) {
   int i, n;
   Result r;

   //
   // Get info from config file and store in the job
   //

   //
   // model
   //
   n = c.n_model_elements();

   for(i=0; i<n; i++) {
      r = c.model(i);
      j.model.add(r.sval());
   }

   //
   // fcst_lead
   //
   n = c.n_fcst_lead_elements();

   for(i=0; i<n; i++) {
      r = c.fcst_lead(i);
      j.fcst_lead.add(timestring_to_sec(r.sval()));
   }

   //
   // obs_lead
   //
   n = c.n_obs_lead_elements();

   for(i=0; i<n; i++) {
      r = c.obs_lead(i);
      j.obs_lead.add(timestring_to_sec(r.sval()));
   }

   //
   // fcst_valid_beg
   //
   r = c.fcst_valid_beg();

   if(strlen(r.sval()) > 0)
      j.fcst_valid_beg = timestring_to_unix(r.sval());

   //
   // fcst_valid_end
   //
   r = c.fcst_valid_end();

   if(strlen(r.sval()) > 0)
      j.fcst_valid_end = timestring_to_unix(r.sval());

   //
   // obs_valid_beg
   //
   r = c.obs_valid_beg();

   if(strlen(r.sval()) > 0)
      j.obs_valid_beg = timestring_to_unix(r.sval());

   //
   // obs_valid_end
   //
   r = c.obs_valid_end();

   if(strlen(r.sval()) > 0)
      j.obs_valid_end = timestring_to_unix(r.sval());

   //
   // fcst_init_beg
   //
   r = c.fcst_init_beg();

   if(strlen(r.sval()) > 0)
      j.fcst_init_beg = timestring_to_unix(r.sval());

   //
   // fcst_init_end
   //
   r = c.fcst_init_end();

   if(strlen(r.sval()) > 0)
      j.fcst_init_end = timestring_to_unix(r.sval());

   //
   // obs_init_beg
   //
   r = c.obs_init_beg();

   if(strlen(r.sval()) > 0)
      j.obs_init_beg = timestring_to_unix(r.sval());

   //
   // obs_init_end
   //
   r = c.obs_init_end();

   if(strlen(r.sval()) > 0)
      j.obs_init_end = timestring_to_unix(r.sval());

   //
   // fcst_init_hour
   //
   n = c.n_fcst_init_hour_elements();

   for(i=0; i<n; i++) {
      r = c.fcst_init_hour(i);
      j.fcst_init_hour.add(timestring_to_sec(r.sval()));
   }

   //
   // obs_init_hour
   //
   n = c.n_obs_init_hour_elements();

   for(i=0; i<n; i++) {
      r = c.obs_init_hour(i);
      j.obs_init_hour.add(timestring_to_sec(r.sval()));
   }

   //
   // fcst_var
   //
   n = c.n_fcst_var_elements();

   for(i=0; i<n; i++) {
      r = c.fcst_var(i);
      j.fcst_var.add(r.sval());
   }

   //
   // obs_var
   //
   n = c.n_obs_var_elements();

   for(i=0; i<n; i++) {
      r = c.obs_var(i);
      j.obs_var.add(r.sval());
   }

   //
   // fcst_lev
   //
   n = c.n_fcst_lev_elements();

   for(i=0; i<n; i++) {
      r = c.fcst_lev(i);
      j.fcst_lev.add(r.sval());
   }

   //
   // obs_lev
   //
   n = c.n_obs_lev_elements();

   for(i=0; i<n; i++) {
      r = c.obs_lev(i);
      j.obs_lev.add(r.sval());
   }

   //
   // obtype
   //
   n = c.n_obtype_elements();

   for(i=0; i<n; i++) {
      r = c.obtype(i);
      j.obtype.add(r.sval());
   }

   //
   // vx_mask
   //
   n = c.n_vx_mask_elements();

   for(i=0; i<n; i++) {
      r = c.vx_mask(i);
      j.vx_mask.add(r.sval());
   }

   //
   // interp_mthd
   //
   n = c.n_interp_mthd_elements();

   for(i=0; i<n; i++) {
      r = c.interp_mthd(i);
      j.interp_mthd.add(r.sval());
   }

   //
   // interp_pnts
   //
   n = c.n_interp_pnts_elements();

   for(i=0; i<n; i++) {
      r = c.interp_pnts(i);
      j.interp_pnts.add(r.ival());
   }

   //
   // fcst_thresh
   //
   n = c.n_fcst_thresh_elements();

   for(i=0; i<n; i++) {
      r = c.fcst_thresh(i);
      j.fcst_thresh.add(r.sval());
   }

   //
   // obs_thresh
   //
   n = c.n_obs_thresh_elements();

   for(i=0; i<n; i++) {
      r = c.obs_thresh(i);
      j.obs_thresh.add(r.sval());
   }

   //
   // cov_thresh
   //
   n = c.n_cov_thresh_elements();

   for(i=0; i<n; i++) {
      r = c.cov_thresh(i);
      j.cov_thresh.add(r.sval());
   }

   //
   // alpha
   //
   n = c.n_alpha_elements();

   for(i=0; i<n; i++) {
      r = c.alpha(i);
      j.alpha.add(r.dval());
   }

   //
   // line_type
   //
   n = c.n_line_type_elements();

   for(i=0; i<n; i++) {
      r = c.line_type(i);
      j.line_type.add(r.sval());
   }

   //
   // column
   //
   n = c.n_column_elements();

   for(i=0; i<n; i++) {
      r = c.column(i);
      j.column.add(r.sval());
   }

   //
   // weight
   //
   n = c.n_weight_elements();

   for(i=0; i<n; i++) {
      r = c.weight(i);
      j.weight.add(r.dval());
   }

   //
   // No settings in the default job for column_min_name,
   // column_min_value, column_max_name, and column_max_value since
   // those are strictly job command options.
   //

   //
   // out_out_alpha
   //
   j.out_alpha = c.out_alpha().dval();

   //
   // boot_interval
   //
   j.boot_interval = c.boot_interval().ival();

   //
   // boot_rep_prop
   //
   j.boot_rep_prop = c.boot_rep_prop().dval();

   //
   // n_boot_rep
   //
   j.n_boot_rep = c.n_boot_rep().ival();

   //
   // boot_rng
   //
   j.set_boot_rng(c.boot_rng().sval());

   //
   // boot_seed
   //
   j.set_boot_seed(c.boot_seed().sval());

   //
   // rank_corr_flag
   //
   j.rank_corr_flag = c.rank_corr_flag().ival();

   //
   // vif_flag
   //
   j.vif_flag = c.vif_flag().ival();

   return;
}

////////////////////////////////////////////////////////////////////////

void do_job(const ConcatString &jobstring, STATAnalysisJob &j,
            int n_job, const ConcatString &tmp_dir,
            const ConcatString &tmp_path, ofstream *sa_out) {
   LineDataFile f;
   int n_in, n_out;

   //
   // Open up the temp file for reading the intermediate STAT line data
   //
   if(!f.open(tmp_path)) {
      mlog << Error << "\ndo_job() -> "
           << "can't open the temporary file \"" << tmp_path
           << "\" for reading!\n\n";
      throw(1);
   }

   //
   // Initialize n_in and n_out to keep track of the number of lines
   // read and retained.
   //
   n_in = n_out = 0;

   mlog << Debug(2) << "\nProcessing Job " << n_job << ": "
        << jobstring << "\n";

   //
   // If the -dump_row option was supplied, open the file
   //
   if(j.dump_row) {

      mlog << Debug(1) << "Creating STAT output file \"" << j.dump_row
           << "\"\n";
      j.open_dump_row_file();
   }

   //
   // Switch on the job type
   //
   switch(j.job_type) {

      case(stat_job_filter):
         do_job_filter(jobstring, f, j, n_in, n_out, sa_out);
         break;

      case(stat_job_summary):
         do_job_summary(jobstring, f, j, n_in, n_out, sa_out);
         break;

      case(stat_job_aggr):
         do_job_aggr(jobstring, f, j, n_in, n_out, sa_out);
         break;

      case(stat_job_aggr_stat):
         do_job_aggr_stat(jobstring, f, j, n_in, n_out, sa_out, tmp_dir);
         break;

      case(stat_job_go_index):
         do_job_go_index(jobstring, f, j, n_in, n_out, sa_out);
         break;

      case(stat_job_ss_index):
         do_job_ss_index(jobstring, f, j, n_in, n_out, sa_out);
         break;

      default:
         mlog << Error << "\ndo_job() -> "
              << "jobtype value of " << j.job_type
              << " not currently supported!\n\n";
         throw(1);
   }

   mlog << Debug(2) << "Job " << n_job << " used " << n_out << " out of "
        << n_in << " STAT lines.\n";

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

void do_job_filter(const ConcatString &jobstring, LineDataFile &f,
                   STATAnalysisJob &j, int &n_in, int &n_out,
                   ofstream *sa_out) {
   ConcatString out_line;
   STATLine line;

   //
   // Check that the -dump_row option has been supplied
   //
   if(!j.dump_row) {
      mlog << Error << "\ndo_job_filter()-> "
           << "this function may only be called when using the "
           << "-dump_row option in the job command line: "
           << jobstring << "\n\n";
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
   out_line << "FILTER:        " << jobstring;

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

void do_job_summary(const ConcatString &jobstring, LineDataFile &f,
                    STATAnalysisJob &j, int &n_in, int &n_out,
                    ofstream *sa_out) {
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
      mlog << Error << "\ndo_job_summary()-> "
           << "this function may only be called when the "
           << "\"-line_type\" option has been used exactly once to "
           << "specify a single line type from which to select a "
           << "statistic to summarize: " << jobstring << "\n\n";
      throw(1);
   }

   //
   // Check that the -column option has been supplied
   //
   if(j.column.n_elements() != 1) {
      mlog << Error << "\ndo_job_summary()-> "
           << "this function may only be called when the "
           << "\"-column\" option has been used to specify a "
           << "single column from which to select a statistic "
           << "to summarize: " << jobstring << "\n\n";
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
   offset = determine_column_offset(lt, j.column[0]);

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
      mlog << Warning << "\ndo_job_summary() -> "
           << "no valid data found in the STAT lines for job: "
           << jobstring << "\n\n";
      return;
   }

   //
   // Compute the summary information for this collection of values:
   // min, max, v10, v25, v50, 75, v90
   //
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

void do_job_aggr(const ConcatString &jobstring, LineDataFile &f,
                 STATAnalysisJob &j, int &n_in, int &n_out,
                 ofstream *sa_out) {
   STATLine line;
   STATLineType lt;

   CTSInfo          cts_info;
   MCTSInfo         mcts_info;
   CNTInfo          cnt_info;
   SL1L2Info        sl1l2_info;
   VL1L2Info        vl1l2_info;
   PCTInfo          pct_info;
   NBRCTSInfo       nbrcts_info;
   NBRCNTInfo       nbrcnt_info;
   ISCInfo          isc_info;
   PairDataEnsemble ens_pd;

   nbrcnt_info.allocate_n_alpha(1);

   AsciiTable out_at;
   int i, n_thresh, n_cat;

   //
   // Check that the -line_type option has been supplied only once
   //
   if(j.line_type.n_elements() != 1) {
      mlog << Error << "\ndo_job_aggr()-> "
           << "this function may only be called when the "
           << "\"-line_type\" option has been used exactly once to "
           << "specify a single line type over which to perform the "
           << "aggregation: " << jobstring << "\n\n";
      throw(1);
   }

   //
   // Determine the line type specified for this job
   //
   lt = string_to_statlinetype(j.line_type[0]);

   //
   // Check that a valid line type has been selected
   //
   if(lt != stat_fho    && lt != stat_ctc    &&
      lt != stat_mctc   && lt != stat_sl1l2  &&
      lt != stat_sal1l2 && lt != stat_vl1l2  &&
      lt != stat_val1l2 && lt != stat_pct    &&
      lt != stat_nbrctc && lt != stat_nbrcnt &&
      lt != stat_rhist  && lt != stat_isc) {
      mlog << Error << "\ndo_job_aggr()-> "
           << "the \"-line_type\" option must be set to one of:\n"
           << "\tFHO, CTC, MCTC,\n"
           << "\tSL1L2, SAL1L2, VL1L2, VAL1L2,\n"
           << "\tPCT, NBRCTC, NBRCNT, ISC, RHIST\n\n";
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
                          lt, n_in, n_out);
   }

   //
   // Sum up the mulit-category contingency table type lines:
   //    MCTC
   //
   else if(lt == stat_mctc) {
      aggr_mctc_lines(jobstring, f, j, mcts_info,
                      lt, n_in, n_out);
   }

   //
   // Sum the partial sum line types:
   //    SL1L2, SAL1L2, VL1L2, VAL1L2
   //
   else if(lt == stat_sl1l2  ||
           lt == stat_sal1l2 ||
           lt == stat_vl1l2  ||
           lt == stat_val1l2 ||
           lt == stat_nbrcnt) {
      aggr_partial_sum_lines(jobstring, f, j,
                             sl1l2_info, vl1l2_info, cnt_info,
                             nbrcnt_info, lt, n_in, n_out);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT
   //
   else if(lt == stat_pct) {
      aggr_nx2_contable_lines(jobstring, f, j, pct_info,
                              lt, n_in, n_out);
   }

   //
   // Sum up the neighborhood contingency table lines:
   //    NBRCTC
   //
   else if(lt == stat_nbrctc) {
      aggr_contable_lines(jobstring, f, j, cts_info,
                          lt, n_in, n_out);
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
         mlog << Error << "\ndo_job_aggr()-> "
              << "when aggregating ISC lines you must select at least "
              << "one tile name to be used with the \"-vx_mask\" "
              << "option: " << jobstring << "\n\n";
         throw(1);
      }

      aggr_isc_lines(jobstring, f, j, isc_info, n_in, n_out);
   }

   //
   // Sum the RHIST line types
   //
   else if(lt == stat_rhist) {
      aggr_rhist_lines(jobstring, f, j, ens_pd, n_in, n_out);
   }

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      mlog << Warning << "\ndo_job_aggr() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n\n";
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

      case(stat_nbrcnt):

         //
         // Get the column names
         //
         out_at.set_size(2, n_nbrcnt_columns+1);
         setup_table(out_at);
         out_at.set_entry(0, 0,  "COL_NAME:");
         write_header_row(nbrcnt_columns, n_nbrcnt_columns, 0,
                          out_at, 0, 1);

         //
         // Write the NBRCTC row
         //
         out_at.set_entry(1, 0,  "NBRCNT:");
         write_nbrcnt_cols(nbrcnt_info, 0, out_at, 1, 1);

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

         mlog << Error << "\ndo_job_aggr() -> "
              << "line type value of "
               << statlinetype_to_string(line.type())
              << " not currently supported for the aggregation "
              << "job!\n\n";
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

void do_job_aggr_stat(const ConcatString &jobstring, LineDataFile &f,
                      STATAnalysisJob &j, int &n_in, int &n_out,
                      ofstream *sa_out, const ConcatString &tmp_dir) {
   STATLine line;
   STATLineType in_lt, out_lt;

   CTSInfo          cts_info;
   MCTSInfo         mcts_info;
   CNTInfo          cnt_info;
   SL1L2Info        sl1l2_info;
   VL1L2Info        vl1l2_info;
   PCTInfo          pct_info;
   NBRCTSInfo       nbrcts_info;
   NBRCNTInfo       nbrcnt_info;
   ISCInfo          isc_info;
   NumArray         f_na, o_na, c_na;
   NumArray         uf_na, vf_na, uo_na, vo_na;
   PairDataEnsemble ens_pd;

   int fcst_gc, obs_gc;
   AsciiTable out_at;
   int i, n;

   //
   // Check that the -line_type option has been supplied only once
   //
   if(j.line_type.n_elements() != 1) {
      mlog << Error << "\ndo_job_aggr_stat()-> "
           << "this function may only be called when the "
           << "\"-line_type\" option has been used exactly once to "
           << "specify a single line type over which to perform the "
           << "aggregation: " << jobstring << "\n\n";
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

      mlog << Error << "\ndo_job_aggr_stat()-> "
           << "invalid combination of \"-line_type "
           << statlinetype_to_string(in_lt) << "\" and "
           << "\"-out_line_type " << statlinetype_to_string(out_lt)
           << "\"\n\n";
      throw(1);
   }

   //
   // Sum up the contingency table type lines:
   //    FHO, CTC
   //
   if(in_lt == stat_fho ||
      in_lt == stat_ctc) {
      aggr_contable_lines(jobstring, f, j, cts_info,
                          in_lt, n_in, n_out);
   }

   //
   // Sum up the multi-category contingency table type lines:
   //    MCTC
   //
   else if(in_lt == stat_mctc) {
      aggr_mctc_lines(jobstring, f, j, mcts_info,
                      in_lt, n_in, n_out);
   }

   //
   // Sum the scalar partial sum line types:
   //    SL1L2, SAL1L2
   //
   else if(in_lt == stat_sl1l2  ||
           in_lt == stat_sal1l2) {
      aggr_partial_sum_lines(jobstring, f, j,
                             sl1l2_info, vl1l2_info, cnt_info,
                             nbrcnt_info, in_lt, n_in, n_out);
   }

   //
   // Sum the vector partial sum line types:
   //    VL1L2, VAL1L2
   //
   else if(in_lt == stat_vl1l2 ||
           in_lt == stat_val1l2) {
      aggr_vl1l2_wdir(jobstring, f, j, vl1l2_info,
                      uf_na, vf_na, uo_na, vo_na,
                      in_lt, n_in, n_out);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT
   //
   else if(in_lt == stat_pct) {
      aggr_nx2_contable_lines(jobstring, f, j, pct_info,
                              in_lt, n_in, n_out);
   }

   //
   // Sum up the neighborhood contingency table lines:
   //    NBRCTC
   //
   else if(in_lt == stat_nbrctc) {
      aggr_contable_lines(jobstring, f, j, cts_info,
                          in_lt, n_in, n_out);
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
            mlog << Error << "\ndo_job_aggr_stat()-> "
                 << "when \"-out_line_type\" is set to FHO, CTC, or "
                 << "CTS the \"-out_fcst_thresh\" and "
                 << "\"-out_obs_thresh\" options must be specified "
                 << "exactly once.\n\n";
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
            mlog << Error << "\ndo_job_aggr_stat()-> "
                 << "when \"-out_line_type\" is set to MCTC or MCTS "
                 << "the \"-out_fcst_thresh\" and "
                 << "\"-out_obs_thresh\" options must be specified "
                 << "the same number of times and at least twice.\n\n";
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

               mlog << Error << "\ndo_job_aggr_stat() -> "
                    << "when \"-out_line_type\" is set to MCTC or MCTS "
                    << "the thresholds must be monotonically "
                    << "increasing and be of the same inequality type "
                    << "(lt, le, gt, or ge).\n\n";
               throw(1);
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
            mlog << Error << "\ndo_job_aggr_stat()-> "
                 << "when \"-out_line_type\" is set to PCT, PSTD, "
                 << "PJC, or PRC, the \"-out_obs_thresh\" option "
                 << "must be specified exactly once.\n\n";
            throw(1);
         }

         n = j.out_fcst_thresh.n_elements();

         // Check that the first threshold is 0 and the last is 1.
         if(n < 3 ||
            !is_eq(j.out_fcst_thresh[0].thresh,   0.0) ||
            !is_eq(j.out_fcst_thresh[n-1].thresh, 1.0)) {

            mlog << Error << "\ndo_job_aggr_stat() -> "
                 << "When verifying a probability field, you must "
                 << "use the \"-out_fcst_thresh\" option to select "
                 << "at least 3 probability thresholds beginning with "
                 << "0.0 and ending with 1.0.\n\n";
            throw(1);
         }

         for(i=0; i<n; i++) {

            // Check that all threshold types are >=
            if(j.out_fcst_thresh[i].type != thresh_ge) {
               mlog << Error << "\ndo_job_aggr_stat() -> "
                    << "When verifying a probability field, all "
                    << "forecast probability thresholds must be set "
                    << "as greater than or equal to with \"ge\" or "
                    << "\"=\".\n\n";
               throw(1);
            }

            // Check that all thresholds are in [0, 1].
            if(j.out_fcst_thresh[i].thresh < 0.0 ||
               j.out_fcst_thresh[i].thresh > 1.0) {

               mlog << Error << "\ndo_job_aggr_stat() -> "
                    << "When verifying a probability field, all "
                    << "forecast probability thresholds must be "
                    << "between 0 and 1.\n\n";
               throw(1);
            }
         } // end for i
      }

      //
      // Parse the input MPR lines
      //
      read_mpr_lines(jobstring, f, j, fcst_gc, obs_gc,
                     f_na, o_na, c_na,
                     n_in, n_out);

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
      aggr_orank_lines(jobstring, f, j, ens_pd, n_in, n_out);
   }

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      mlog << Warning << "\ndo_job_aggr_stat() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n\n";
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
         mlog << Error << "\nwrite_job_cnt() -> "
              << "unexpected line type value of " << in_lt
              << "\n\n";
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

      mlog << Error << "\nwrite_job_wdir()-> "
           << "the number of U and V forecast and observation points "
           << "must be the same.\n\n";
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

   mlog << Debug(4) << "write_job_wdir() -> "
        << "ROW_MEAN_WDIR: average forecast direction (u, v) = ("
        << uf << ", "  << vf << ") = " << fbar << " degrees\n";

   mlog << Debug(4) << "write_job_wdir() -> "
        << "ROW_MEAN_WDIR: average observed direction (u, v) = ("
        << uo << ", "  << vo << ") = " << obar << " degrees\n";
   
   //
   // Compute the mean error and the mean absolute error
   // from the unit vectors
   //
   me = mae = 0.0;
   for(i=0; i<n_out; i++) {

      angle = angle_difference(uf_na[i], vf_na[i],
                               uo_na[i], vo_na[i]);

      if(mlog.verbosity_level() > 3) {
         mlog << Debug(4) << "write_job_wdir() -> "
              << "ROW_MEAN_WDIR: [" << i+1 << "] difference of forecast direction "
              << convert_u_v_to_wdir(uf_na[i], vf_na[i]) << " - observed direction "
              << convert_u_v_to_wdir(uo_na[i], vo_na[i]) << " = "
              << angle << " degrees\n";
      }
                               
      // Check for bad data
      if(is_eq(angle, bad_data_double)) {
         me  = bad_data_double;
         mae = bad_data_double;
         break;
      }

      me   += angle;
      mae  += abs(angle);
   }
   if(!is_eq(me,  bad_data_double)) me  /= n_out;
   if(!is_eq(mae, bad_data_double)) mae /= n_out;

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
         mlog << Error << "\nwrite_job_wdir() -> "
              << "unexpected line type value of " << in_lt
              << "\n\n";
         throw(1);
   } // end switch

   //
   // Compute the aggregated forecast and observation angles
   //
   fbar = convert_u_v_to_wdir(uf, vf);
   obar = convert_u_v_to_wdir(uo, vo);
   me   = angle_difference(uf, vf, uo, vo);
   mae  = bad_data_double;

   mlog << Debug(4) << "write_job_wdir() -> "
        << "AGGR_WDIR: aggregated forecast direction (u, v) = ("
        << uf << ", "  << vf << ") = " << fbar << " degrees\n";

   mlog << Debug(4) << "write_job_wdir() -> "
        << "AGGR_WDIR: aggregated observed direction (u, v) = ("
        << uo << ", "  << vo << ") = " << obar << " degrees\n";
   
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
         mlog << Error << "\nwrite_job_pct() -> "
              << "unexpected line type value of " << in_lt
              << "\n\n";
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

         mlog << Error << "\nwrite_job_mpr() -> "
              << "line type value of " << j.out_line_type
              << " not currently supported for the aggregation "
              << "job!\n\n";
         throw(1);
   } // end switch

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_go_index() routine is used to compute the GO Index.
// The GO Index is a special case of the Skill Score Index consisting
// of a predefined set of variables, levels, lead times, statistics,
// and weights.
// For lead times of 12, 24, 36, and 48 hours, it contains RMSE for:
// - Wind Speed at the surface(b), 850(a), 400(a), 250(a) mb
// - Dewpoint Temperature at the surface(b), 850(b), 700(b), 400(b) mB
// - Temperature at the surface(b), 400(a) mB
// - Height at 400(a) mB
// - Sea Level Pressure(b)
// Where (a) means weights of 4, 3, 2, 1 for the lead times, and
//       (b) means weights of 8, 6, 4, 2 for the lead times.
// The RMSE values are dervied from the partial sums in the SL1L2 lines.
//
////////////////////////////////////////////////////////////////////////

void do_job_go_index(const ConcatString &jobstring, LineDataFile &f,
                     STATAnalysisJob &j, int &n_in, int &n_out,
                     ofstream *sa_out) {
   stat_analysis_Conf go_conf;
   double go_index;
   AsciiTable out_at;
   ConcatString config_file;

   //
   // Read in the STATAnalysis config file which defines the GO Index.
   //
   config_file = replace_path(go_index_config_file);
   mlog << Debug(3) << "Reading Config: " << config_file << "\n";
   go_conf.read(config_file);

   //
   // Parse the contents of the GO Index config file into the job.
   //
   set_job_from_config(go_conf, j);

   //
   // Compute the GO Index as a special case of the Skill Score Index
   //
   go_index = compute_ss_index(jobstring, f, j, n_in, n_out);

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      mlog << Warning << "\ndo_job_go_index() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n\n";
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
//
// The do_job_ss_index() routine is used to compute the generalized
// Skill Score Index.  This job can be configured to compute a weighted
// average of skill scores derived from a configurable set of variables,
// levels, lead times, and statistics.  The skill score index is
// computed using two models, a forecast model and a reference model.
// For each statistic in the index, a skill score is computed as:
//   SS = 1 - (S[model]*S[model])/(S[reference]*S[reference])
// Where S is the statistic.
// Next, a weighted average is computed over all the skill scores.
// Lastly, an index value is computed as:
//   Index = sqrt(1/(1-SS[avg]))
// Where SS[avg] is the weighted average of skill scores.
//
////////////////////////////////////////////////////////////////////////

void do_job_ss_index(const ConcatString &jobstring, LineDataFile &f,
                     STATAnalysisJob &j, int &n_in, int &n_out,
                     ofstream *sa_out) {
   double ss_index;
   AsciiTable out_at;

   //
   // Compute the Skill Score Index
   //
   ss_index = compute_ss_index(jobstring, f, j, n_in, n_out);

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      mlog << Warning << "\ndo_job_ss_index() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n\n";
      return;
   }

   //
   // Get the column names
   //
   out_at.set_size(2, n_job_sum_columns+1);
   setup_table(out_at);
   out_at.set_entry(0, 0,  "COL_NAME:");
   write_header_row(job_ss_columns, n_job_ss_columns, 0, out_at, 0, 1);

   //
   // Write the data row
   //
   out_at.set_entry(1, 0,  "SS_INDEX:");
   out_at.set_entry(1, 1,  ss_index);

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

void write_jobstring(const ConcatString &jobstring, ofstream *sa_out) {
   ConcatString out_line;

   out_line << "JOB_LIST:      " << jobstring;

   write_line(out_line, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_line(const ConcatString &str, ofstream *sa_out) {

   if(sa_out) *(sa_out) << str << "\n" << flush;
   else       cout      << str << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_ss_index(const ConcatString &jobstring, LineDataFile &f,
                        STATAnalysisJob &j, int &n_in, int &n_out) {
   STATLine line;
   SL1L2Info si;
   TTContingencyTable ct;
   CNTInfo fcst_cnt, ref_cnt;
   bool keep;
   int i, n_terms;
   double fcst_stat, ref_stat, ss, ss_sum, weight_sum;
   double ss_avg, ss_index;

   //
   // Check that the -model option has been supplied exactly 2 times.
   // The first is the forecast model and the second is the reference.
   //
   if(j.model.n_elements() != 2) {
      mlog << Error << "\ncompute_ss_index()-> "
           << "this job may only be called when the \"-model\" option "
           << "has been used exactly twice to specify the forecast "
           << "model followed by the reference model: "
           << jobstring << "\n\n";
      throw(1);
   }

   //
   // Use the length of the fcst_var array to infer the number of terms.
   //
   if((n_terms = j.fcst_var.n_elements()) < 1) {
      mlog << Error << "\ncompute_ss_index()-> "
           << "you must define the Skill Score Index to be computed "
           << "using the \"-fcst_var\", \"-fcst_lev\", \"-fcst_lead\", "
           << "\"-line_type\", \"-column\", and \"-weight\" options: "
           << jobstring << "\n\n";
      throw(1);
   }

   //
   // Check that the required elements are of the same length.
   //
   if(n_terms != j.fcst_lev.n_elements()  ||
      n_terms != j.fcst_lead.n_elements() ||
      n_terms != j.line_type.n_elements() ||
      n_terms != j.column.n_elements()    ||
      n_terms != j.weight.n_elements()) {
      mlog << Error << "\ncompute_ss_index()-> "
           << "all filtering parameters for defining the Skill Score "
           << "Index must be of the same length.  Check \"-fcst_var\", "
           << "\"-fcst_lev\", \"-fcst_lead\", \"-line_type\", "
           << "\"-column\", and \"-weight\" options: "
           << jobstring << "\n\n";
      throw(1);
   }

   //
   // Define arrays of jobs for each term in the Skill Score Index.
   // Separate arrays for the forecast and reference models.
   //
   STATAnalysisJob *fcst_job = (STATAnalysisJob *) 0, *ref_job = (STATAnalysisJob *) 0;
   fcst_job = new STATAnalysisJob [n_terms];
   ref_job  = new STATAnalysisJob [n_terms];

   //
   // Define arrays of objects to store the partial sums or contingency
   // table counts for each term in the Skill Score Index.
   //
   SL1L2Info *fcst_si = (SL1L2Info *) 0,  *ref_si = (SL1L2Info *) 0;
   CTSInfo   *fcst_cts = (CTSInfo *) 0, *ref_cts = (CTSInfo *) 0;
   fcst_si  = new SL1L2Info [n_terms];
   ref_si   = new SL1L2Info [n_terms];
   fcst_cts = new CTSInfo   [n_terms];
   ref_cts  = new CTSInfo   [n_terms];

   //
   // Define array of line types to be aggregated for each term in the
   // Skill Score Index.
   //
   STATLineType *job_lt = (STATLineType *) 0;
   job_lt = new STATLineType [n_terms];

   //
   // Arrays to keep track of the number of stat lines per term
   //
   NumArray n_fcst_lines, n_ref_lines;

   mlog << Debug(3) << "Forecast Model  = " << j.model[0] << "\n"
        << "Reference Model = " << j.model[1] << "\n";

   //
   // Set up the job for each term in the index.
   //
   for(i=0; i<n_terms; i++) {

      //
      // Initialize the counts
      //
      n_fcst_lines.add(0);
      n_ref_lines.add(0);

      //
      // Initialize to the full Skill Score Index job
      //
      fcst_job[i] = j;

      //
      // model
      //
      fcst_job[i].model.clear();
      fcst_job[i].model.add(j.model[0]);

      //
      // fcst_lead
      //
      if(j.fcst_lead.n_elements() == n_terms) {
         fcst_job[i].fcst_lead.clear();
         fcst_job[i].fcst_lead.add(j.fcst_lead[i]);
      }

      //
      // obs_lead
      //
      if(j.obs_lead.n_elements() == n_terms) {
         fcst_job[i].obs_lead.clear();
         fcst_job[i].obs_lead.add(j.obs_lead[i]);
      }

      //
      // fcst_init_hour
      //
      if(j.fcst_init_hour.n_elements() == n_terms) {
         fcst_job[i].fcst_init_hour.clear();
         fcst_job[i].fcst_init_hour.add(j.fcst_init_hour[i]);
      }

      //
      // obs_init_hour
      //
      if(j.obs_init_hour.n_elements() == n_terms) {
         fcst_job[i].obs_init_hour.clear();
         fcst_job[i].obs_init_hour.add(j.obs_init_hour[i]);
      }

      //
      // fcst_var
      //
      if(j.fcst_var.n_elements() == n_terms) {
         fcst_job[i].fcst_var.clear();
         fcst_job[i].fcst_var.add(j.fcst_var[i]);
      }

      //
      // obs_var
      //
      if(j.obs_var.n_elements() == n_terms) {
         fcst_job[i].obs_var.clear();
         fcst_job[i].obs_var.add(j.obs_var[i]);
      }

      //
      // fcst_lev
      //
      if(j.fcst_lev.n_elements() == n_terms) {
         fcst_job[i].fcst_lev.clear();
         fcst_job[i].fcst_lev.add(j.fcst_lev[i]);
      }

      //
      // obs_lev
      //
      if(j.obs_lev.n_elements() == n_terms) {
         fcst_job[i].obs_lev.clear();
         fcst_job[i].obs_lev.add(j.obs_lev[i]);
      }

      //
      // obtype
      //
      if(j.obtype.n_elements() == n_terms) {
         fcst_job[i].obtype.clear();
         fcst_job[i].obtype.add(j.obtype[i]);
      }

      //
      // vx_mask
      //
      if(j.vx_mask.n_elements() == n_terms) {
         fcst_job[i].vx_mask.clear();
         fcst_job[i].vx_mask.add(j.vx_mask[i]);
      }

      //
      // interp_mthd
      //
      if(j.interp_mthd.n_elements() == n_terms) {
         fcst_job[i].interp_mthd.clear();
         fcst_job[i].interp_mthd.add(j.interp_mthd[i]);
      }

      //
      // interp_pnts
      //
      if(j.interp_pnts.n_elements() == n_terms) {
         fcst_job[i].interp_pnts.clear();
         fcst_job[i].interp_pnts.add(j.interp_pnts[i]);
      }

      //
      // fcst_thresh
      //
      if(j.fcst_thresh.n_elements() == n_terms) {
         fcst_job[i].fcst_thresh.clear();
         fcst_job[i].fcst_thresh.add(j.fcst_thresh[i]);
      }

      //
      // obs_thresh
      //
      if(j.obs_thresh.n_elements() == n_terms) {
         fcst_job[i].obs_thresh.clear();
         fcst_job[i].obs_thresh.add(j.obs_thresh[i]);
      }

      //
      // line_type
      //
      if(j.line_type.n_elements() == n_terms) {
         fcst_job[i].line_type.clear();
         fcst_job[i].line_type.add(j.line_type[i]);

         job_lt[i] = string_to_statlinetype(j.line_type[i]);
         if(job_lt[i] != stat_sl1l2 && job_lt[i] != stat_ctc) {
            mlog << Error << "\ncompute_ss_index() -> "
                 << "a Skill Score Index can only be computed using "
                 << "statistics derived from SL1L2 or CTC line types."
                 << "\n\n";
            throw(1);
         }
      }

      //
      // column
      //
      if(j.column.n_elements() == n_terms) {
         fcst_job[i].column.clear();
         fcst_job[i].column.add(j.column[i]);
      }

      //
      // weight
      //
      if(j.weight.n_elements() == n_terms) {
         fcst_job[i].weight.clear();
         fcst_job[i].weight.add(j.weight[i]);
      }

      //
      // Set the reference model job identical to the forecast model
      // job but with a different model name.
      //
      ref_job[i] = fcst_job[i];
      ref_job[i].model.set(0, j.model[1]);

   } // end for i

   //
   // Process the STAT lines
   //
   n_in = n_out = 0;
   while(f >> line) {

      n_in++;

      //
      // Loop through the jobs to see if this line should be kept
      //
      keep = 0;
      for(i=0; i<n_terms; i++) {

         //
         // Check the forecast model job
         //
         if(fcst_job[i].is_keeper(line)) {
            keep = 1;
            n_fcst_lines.set(i, n_fcst_lines[i]+1);

            if(job_lt[i] == stat_sl1l2) {
               si.clear();
               parse_sl1l2_line(line, si);
               fcst_si[i] += si;
            }
            else if(job_lt[i] == stat_ctc) {
               ct.zero_out();
               parse_ctc_ctable(line, ct);
               fcst_cts[i].cts.set_fy_oy(fcst_cts[i].cts.fy_oy() +
                                         ct.fy_oy());
               fcst_cts[i].cts.set_fy_on(fcst_cts[i].cts.fy_on() +
                                         ct.fy_on());
               fcst_cts[i].cts.set_fn_oy(fcst_cts[i].cts.fn_oy() +
                                         ct.fn_oy());
               fcst_cts[i].cts.set_fn_on(fcst_cts[i].cts.fn_on() +
                                         ct.fn_on());
            }
         } // end if fcst_job

         //
         // Check the reference model job
         //
         if(ref_job[i].is_keeper(line)) {
            keep = 1;
            n_ref_lines.set(i, n_ref_lines[i]+1);

            if(job_lt[i] == stat_sl1l2) {
               si.clear();
               parse_sl1l2_line(line, si);
               ref_si[i] += si;
            }
            else if(job_lt[i]== stat_ctc) {
               ct.zero_out();
               parse_ctc_ctable(line, ct);
               ref_cts[i].cts.set_fy_oy(ref_cts[i].cts.fy_oy() +
                                        ct.fy_oy());
               ref_cts[i].cts.set_fy_on(ref_cts[i].cts.fy_on() +
                                        ct.fy_on());
               ref_cts[i].cts.set_fn_oy(ref_cts[i].cts.fn_oy() +
                                        ct.fn_oy());
               ref_cts[i].cts.set_fn_on(ref_cts[i].cts.fn_on() +
                                        ct.fn_on());
            }
         } // end if ref_job
      } // end for i

      //
      // Write line to dump file
      //
      if(keep) {
         if(j.dr_out) *(j.dr_out) << line;
         n_out++;
      }

   } // end while

   //
   // Loop through the terms and compute a skill score for each.
   //
   ss = ss_sum = weight_sum = 0.0;
   for(i=0; i<n_terms; i++) {

      //
      // Compute continuous stats for the current term
      //
      if(job_lt[i] == stat_sl1l2) {
         fcst_cnt.clear();
         compute_cntinfo(fcst_si[i], 0, fcst_cnt);
         ref_cnt.clear();
         compute_cntinfo(ref_si[i], 0, ref_cnt);
      }
      //
      // Compute categorical stats for the current term
      //
      else if(job_lt[i]== stat_ctc) {
         fcst_cts[i].compute_stats();
         ref_cts[i].compute_stats();
      }

      //
      // Extract the statistic to be used in defining the skill score.
      // Continuous (only stats derived from SL1L2 lines):
      //    PR_CORR, ME, ESTDEV, MBIAS, MSE, BCRMSE, RMSE
      // Categorical:
      //    BASER, FMEAN, ACC, FBIAS, PODY, PODN, POFD, FAR, CSI, GSS,
      //    HK, HSS, ODDS
      //

      fcst_stat = ref_stat = bad_data_double;

      if(strcasecmp(fcst_job[i].column[0], "PR_CORR") == 0) {
         fcst_stat = fcst_cnt.pr_corr.v;
         ref_stat  = ref_cnt.pr_corr.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "ME") == 0) {
         fcst_stat = fcst_cnt.me.v;
         ref_stat  = ref_cnt.me.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "ESTDEV") == 0) {
         fcst_stat = fcst_cnt.estdev.v;
         ref_stat  = ref_cnt.estdev.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "MBIAS") == 0) {
         fcst_stat = fcst_cnt.mbias.v;
         ref_stat  = ref_cnt.mbias.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "MSE") == 0) {
         fcst_stat = fcst_cnt.mse.v;
         ref_stat  = ref_cnt.mse.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "BCRMSE") == 0) {
         fcst_stat = fcst_cnt.bcmse.v;
         ref_stat  = ref_cnt.bcmse.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "RMSE") == 0) {
         fcst_stat = fcst_cnt.rmse.v;
         ref_stat  = ref_cnt.rmse.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "BASER") == 0) {
         fcst_stat = fcst_cts[i].baser.v;
         ref_stat  = ref_cts[i].baser.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "FMEAN") == 0) {
         fcst_stat = fcst_cts[i].fmean.v;
         ref_stat  = ref_cts[i].fmean.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "ACC") == 0) {
         fcst_stat = fcst_cts[i].acc.v;
         ref_stat  = ref_cts[i].acc.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "FBIAS") == 0) {
         fcst_stat = fcst_cts[i].fbias.v;
         ref_stat  = ref_cts[i].fbias.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "PODY") == 0) {
         fcst_stat = fcst_cts[i].pody.v;
         ref_stat  = ref_cts[i].pody.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "PODN") == 0) {
         fcst_stat = fcst_cts[i].podn.v;
         ref_stat  = ref_cts[i].podn.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "POFD") == 0) {
         fcst_stat = fcst_cts[i].pofd.v;
         ref_stat  = ref_cts[i].pofd.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "FAR") == 0) {
         fcst_stat = fcst_cts[i].far.v;
         ref_stat  = ref_cts[i].far.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "CSI") == 0) {
         fcst_stat = fcst_cts[i].csi.v;
         ref_stat  = ref_cts[i].csi.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "GSS") == 0) {
         fcst_stat = fcst_cts[i].gss.v;
         ref_stat  = ref_cts[i].gss.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "HK") == 0) {
         fcst_stat = fcst_cts[i].hk.v;
         ref_stat  = ref_cts[i].hk.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "HSS") == 0) {
         fcst_stat = fcst_cts[i].hss.v;
         ref_stat  = ref_cts[i].hss.v;
      }
      else if(strcasecmp(fcst_job[i].column[0], "ODDS") == 0) {
         fcst_stat = fcst_cts[i].odds.v;
         ref_stat  = ref_cts[i].odds.v;
      }

      //
      // Check for conditions when a skill score cannot be computed.
      //
      if(nint(n_fcst_lines[i]) == 0 || nint(n_ref_lines[i]) == 0   ||
         is_bad_data(fcst_stat)     || is_bad_data(ref_stat) ||
         is_eq(ref_stat, 0.0)) {
         ss = bad_data_double;
      }
      //
      // Compute the skill score and keep a running sum of the skill
      // scores and weights.
      //
      else {
         ss = 1.0 - (fcst_stat*fcst_stat)/(ref_stat*ref_stat);
         ss_sum     += ss*fcst_job[i].weight[0];
         weight_sum += fcst_job[i].weight[0];
      }

      mlog << Debug(3) << "Skill Score Index Term " << i+1
           << ": fcst_var = " << fcst_job[i].fcst_var[0]
           << ", fcst_lev = " << fcst_job[i].fcst_lev[0]
           << ", fcst_lead_sec = " << fcst_job[i].fcst_lead[0]
           << ", line_type = " << fcst_job[i].line_type[0]
           << ", column = " << fcst_job[i].column[0]
           << ", n_fcst = " << n_fcst_lines[i]
           << ", n_ref = " << n_ref_lines[i]
           << ", fcst = " << fcst_stat
           << ", ref = " << ref_stat
           << ", skill = " << ss
           << ", weight = " << fcst_job[i].weight[0] << "\n";

      //
      // Check the the number of aggregated lines differ.
      //
      if(nint(n_fcst_lines[i]) != nint(n_ref_lines[i])) {
         mlog << Warning << "\ncompute_ss_index() -> "
              << "the number of aggregated forecast and reference lines "
              << "differ (" << n_fcst_lines[i] << " != " << n_ref_lines[i]
              << ") for term " << i+1 << ".\n\n";
      }

      if(is_bad_data(ss)) {
         mlog << Warning << "\ncompute_ss_index() -> "
              << "can't compute skill score for term " << i+1 << ".\n\n";
      }

   } // end for i

   //
   // Compute the weighted average of the skill scores.
   //
   if(is_eq(weight_sum, 0.0)) ss_avg = bad_data_double;
   else                       ss_avg = ss_sum/weight_sum;

   //
   // Compute the Skill Score Index value.
   //
   if(is_bad_data(ss_avg) ||
      is_eq(ss_avg, 1.0)) ss_index = bad_data_double;
   else                   ss_index = sqrt(1.0/(1.0 - ss_avg));

   mlog << Debug(3) << "Skill Score Index Weighted Average = " << ss_avg << "\n"
        << "Skill Score Index Value = " << ss_index << "\n";

   //
   // Clean up allocated memory.
   //
   if(fcst_job) { delete [] fcst_job; fcst_job = (STATAnalysisJob *) 0; }
   if(ref_job)  { delete [] ref_job;  ref_job  = (STATAnalysisJob *) 0; }
   if(fcst_si)  { delete [] fcst_si;  fcst_si  = (SL1L2Info *)       0; }
   if(ref_si)   { delete [] ref_si;   ref_si   = (SL1L2Info *)       0; }
   if(fcst_cts) { delete [] fcst_cts; fcst_cts = (CTSInfo *)         0; }
   if(ref_cts)  { delete [] ref_cts;  ref_cts  = (CTSInfo *)         0; }
   if(job_lt)   { delete [] job_lt;   job_lt   = (STATLineType *)    0; }

   return(ss_index);
}

////////////////////////////////////////////////////////////////////////

