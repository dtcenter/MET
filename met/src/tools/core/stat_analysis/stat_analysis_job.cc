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
//   005    05/03/12  Halley Gotway   Switch to using vx_config library.
//   006    02/04/13  Halley Gotway   Add -by case option.
//   007    03/07/13  Halley Gotway   Add aggregate SSVAR lines.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <cmath>

#include "vx_log.h"

#include "stat_analysis_job.h"
#include "parse_stat_line.h"
#include "aggr_stat_line.h"

////////////////////////////////////////////////////////////////////////

void set_job_from_config(MetConfig &c, STATAnalysisJob &j) {
   BootInfo boot_info;

   //
   // Parse top-level configuration filtering options
   //
   
   j.model           = c.lookup_string_array(conf_key_model, false);
   j.fcst_lead       = c.lookup_seconds_array(conf_key_fcst_lead, false);
   j.obs_lead        = c.lookup_seconds_array(conf_key_obs_lead, false);
   j.fcst_valid_beg  = c.lookup_unixtime(conf_key_fcst_valid_beg, false);
   j.fcst_valid_end  = c.lookup_unixtime(conf_key_fcst_valid_end, false);
   j.fcst_valid_hour = c.lookup_seconds_array(conf_key_fcst_valid_hour, false);
   j.obs_valid_beg   = c.lookup_unixtime(conf_key_obs_valid_beg, false);
   j.obs_valid_end   = c.lookup_unixtime(conf_key_obs_valid_end, false);
   j.obs_valid_hour  = c.lookup_seconds_array(conf_key_obs_valid_hour, false);
   j.fcst_init_beg   = c.lookup_unixtime(conf_key_fcst_init_beg, false);
   j.fcst_init_end   = c.lookup_unixtime(conf_key_fcst_init_end, false);
   j.fcst_init_hour  = c.lookup_seconds_array(conf_key_fcst_init_hour, false);
   j.obs_init_beg    = c.lookup_unixtime(conf_key_obs_init_beg, false);
   j.obs_init_end    = c.lookup_unixtime(conf_key_obs_init_end, false);
   j.obs_init_hour   = c.lookup_seconds_array(conf_key_obs_init_hour, false);
   j.fcst_var        = c.lookup_string_array(conf_key_fcst_var, false);
   j.obs_var         = c.lookup_string_array(conf_key_obs_var, false);
   j.fcst_lev        = c.lookup_string_array(conf_key_fcst_lev, false);
   j.obs_lev         = c.lookup_string_array(conf_key_obs_lev, false);
   j.obtype          = c.lookup_string_array(conf_key_obtype, false);
   j.vx_mask         = c.lookup_string_array(conf_key_vx_mask, false);
   j.interp_mthd     = c.lookup_string_array(conf_key_interp_mthd, false);
   j.interp_pnts     = c.lookup_num_array(conf_key_interp_pnts, false);
   j.fcst_thresh     = c.lookup_thresh_array(conf_key_fcst_thresh, false);
   j.obs_thresh      = c.lookup_thresh_array(conf_key_obs_thresh, false);
   j.cov_thresh      = c.lookup_thresh_array(conf_key_cov_thresh, false);
   j.alpha           = c.lookup_num_array(conf_key_alpha, false);
   j.line_type       = c.lookup_string_array(conf_key_line_type, false);
   j.column          = c.lookup_string_array(conf_key_column, false);
   j.weight          = c.lookup_num_array(conf_key_weight, false);

   j.out_alpha       = c.lookup_double(conf_key_out_alpha, false);
   
   boot_info         = parse_conf_boot(&c);
   j.boot_interval   = boot_info.interval;
   j.boot_rep_prop   = boot_info.rep_prop;
   j.n_boot_rep      = boot_info.n_rep;
   j.set_boot_rng(boot_info.rng);
   j.set_boot_seed(boot_info.seed);
   
   j.rank_corr_flag  = (int) c.lookup_bool(conf_key_rank_corr_flag);
   j.vif_flag        = (int) c.lookup_bool(conf_key_vif_flag);
   
   //
   // No settings in the default job for column_min_name,
   // column_min_value, column_max_name, column_max_value, and 
   // column_case since those are strictly job command options.
   //

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
   // Print warning for column_case option
   //
   if(j.column_case.n_elements() > 0 &&
      j.job_type != stat_job_summary &&
      j.job_type != stat_job_aggr &&
      j.job_type != stat_job_aggr_stat) {
      mlog << Warning << "\nThe -by option is ignored for the \""
           << statjobtype_to_string(j.job_type) << "\" job type.\n\n";
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
      mlog << Error << "\ndo_job_filter() -> "
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
   int i, k, r, c;
   double v, min, v10, v25, v50, v75, v90, max;
   CIInfo mean_ci, stdev_ci;
   gsl_rng *rng_ptr = (gsl_rng *) 0;
   AsciiTable out_at;
   ConcatString key;
   NumArray value;
   StringArray sa;
   map<ConcatString, NumArray> summary_map;
   map<ConcatString, NumArray>::iterator it;

   //
   // Check that the -line_type option has been supplied only once
   //
   if(j.line_type.n_elements() != 1) {
      mlog << Error << "\ndo_job_summary() -> "
           << "this function may only be called when the "
           << "\"-line_type\" option has been used exactly once to "
           << "specify a single line type from which to select a "
           << "statistic to summarize: " << jobstring << "\n\n";
      throw(1);
   }

   //
   // Check that the -column option has been supplied
   //
   if(j.column.n_elements() == 0) {
      mlog << Error << "\ndo_job_summary() -> "
           << "this function may only be called when the "
           << "\"-column\" option has been used at least once to "
           << "specify the column(s) to summarize: " << jobstring
           << "\n\n";
      throw(1);
   }

   //
   // Determine the line type
   //
   lt = string_to_statlinetype(j.line_type[0]);
   
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

         //
         // Loop through the columns to be summarized
         //
         for(i=0; i<j.column.n_elements(); i++) {
        
            //
            // Build the key and get the current column value
            //
            key = j.column[i];
            key << ":" << j.get_case_info(line);
            v   = atof(line.get_item(determine_column_offset(lt, j.column[i])));

            //
            // Add value to existing map entry or add a new one
            //         
            if(!is_bad_data(v)) {
               if(summary_map.count(key) > 0) {
                  summary_map[key].add(v);
               }
               else {
                  value.clear();
                  value.add(v);
                  summary_map[key] = value;
               }
            }
         } // end for i

         n_out++;

      } // end if
   } // end while

   //
   // Check for no matching STAT lines
   //
   if(summary_map.size() == 0) {
      mlog << Warning << "\ndo_job_summary() -> "
           << "no valid data found in the STAT lines for job: "
           << jobstring << "\n\n";
      return;
   }

   //
   // Setup the output object
   //
   r = c = 0;
   out_at.set_size(summary_map.size() + 1,
                   2 + j.column_case.n_elements() + n_job_sum_columns);
   setup_table(out_at);

   //
   // Write header line
   //
   out_at.set_entry(r, c++, "COL_NAME:");
   out_at.set_entry(r, c++, "COLUMN");
   for(k=0; k<j.column_case.n_elements(); k++)
     out_at.set_entry(0, c++, j.column_case[k]);
   write_header_row(job_sum_columns, n_job_sum_columns,
                    0, out_at, r, c);

   //
   // Set up CIInfo objects
   //
   mean_ci.allocate_n_alpha(1);
   stdev_ci.allocate_n_alpha(1);
                    
   //
   // Loop over the summary table
   //
   for(it  = summary_map.begin(), r=1;
       it != summary_map.end();
       it++, r++) {

      //
      // Compute the summary information for this collection of values:
      // min, max, v10, v25, v50, 75, v90
      //
      min = it->second.percentile_array(0.00);
      v10 = it->second.percentile_array(0.10);
      v25 = it->second.percentile_array(0.25);
      v50 = it->second.percentile_array(0.50);
      v75 = it->second.percentile_array(0.75);
      v90 = it->second.percentile_array(0.90);
      max = it->second.percentile_array(1.00);

      //
      // Set up the random number generator and seed value
      //
      rng_set(rng_ptr, j.boot_rng, j.boot_seed);

      //
      // Compute a bootstrap confidence interval for the mean of this
      // array of values.
      //
      if(j.boot_interval == boot_bca_flag) {
         compute_mean_stdev_ci_bca(rng_ptr, it->second,
                                   j.n_boot_rep,
                                   j.out_alpha, mean_ci, stdev_ci);
      }
      else {
         compute_mean_stdev_ci_perc(rng_ptr, it->second,
                                    j.n_boot_rep, j.boot_rep_prop,
                                    j.out_alpha, mean_ci, stdev_ci);
      }

      //
      // Write the data row
      //
      c = 0;
      
      // Split the current map key
      sa = it->first.split(":");

      out_at.set_entry(r, c++, "SUMMARY:");
      out_at.set_entry(r, c++, sa[0]);

      // Write case column values
      for(i=1; i<sa.n_elements(); i++)
         out_at.set_entry(r, c++, sa[i]);

      out_at.set_entry(r, c++, it->second.n_elements());
      out_at.set_entry(r, c++, mean_ci.v);
      out_at.set_entry(r, c++, mean_ci.v_ncl[0]);
      out_at.set_entry(r, c++, mean_ci.v_ncu[0]);
      out_at.set_entry(r, c++, mean_ci.v_bcl[0]);
      out_at.set_entry(r, c++, mean_ci.v_bcu[0]);
      out_at.set_entry(r, c++, stdev_ci.v);
      out_at.set_entry(r, c++, stdev_ci.v_bcl[0]);
      out_at.set_entry(r, c++, stdev_ci.v_bcu[0]);
      out_at.set_entry(r, c++, min);
      out_at.set_entry(r, c++, v10);
      out_at.set_entry(r, c++, v25);
      out_at.set_entry(r, c++, v50);
      out_at.set_entry(r, c++, v75);
      out_at.set_entry(r, c++, v90);
      out_at.set_entry(r, c++, max);

   } // end for it

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
   AsciiTable out_at;

   map<ConcatString, AggrCTCInfo>   ctc_map;
   map<ConcatString, AggrMCTCInfo>  mctc_map;
   map<ConcatString, AggrPCTInfo>   pct_map;
   map<ConcatString, AggrPSumInfo>  psum_map;
   map<ConcatString, AggrISCInfo>   isc_map;
   map<ConcatString, AggrRHISTInfo> rhist_map;
   map<ConcatString, AggrSSVARInfo> ssvar_map;

   //
   // Check that the -line_type option has been supplied exactly once
   //
   if(j.line_type.n_elements() != 1) {
      mlog << Error << "\ndo_job_aggr() -> "
           << "this function may only be called when the \"-line_type\" "
           << "option has been used exactly once to specify the line "
           << "type for aggregation: " << jobstring << "\n\n";
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
      lt != stat_rhist  && lt != stat_isc    &&
      lt != stat_ssvar) {
      mlog << Error << "\ndo_job_aggr() -> "
           << "the \"-line_type\" option must be set to one of:\n"
           << "\tFHO, CTC, MCTC,\n"
           << "\tSL1L2, SAL1L2, VL1L2, VAL1L2,\n"
           << "\tPCT, NBRCTC, NBRCNT, ISC,\n"
           << "\tRHIST, SSVAR\n\n";
      throw(1);
   }

   //
   // Turn off the vif_flag since it doesn't apply
   //
   j.vif_flag = 0;

   //
   // Sum up the contingency table type lines:
   //    FHO, CTC, NBRCTC
   //
   if(lt == stat_fho ||
      lt == stat_ctc ||
      lt == stat_nbrctc) {
      aggr_ctc_lines(f, j, ctc_map, n_in, n_out);
      write_job_aggr_ctc(j, lt, ctc_map, out_at);
   }

   //
   // Sum up the multi-category contingency table type lines:
   //    MCTC
   //
   else if(lt == stat_mctc) {
      aggr_mctc_lines(f, j, mctc_map, n_in, n_out);
      write_job_aggr_mctc(j, lt, mctc_map, out_at);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT
   //
   else if(lt == stat_pct) {
      aggr_pct_lines(f, j, pct_map, n_in, n_out);
      write_job_aggr_pct(j, lt, pct_map, out_at);
   }

   //
   // Sum the partial sum line types:
   //    SL1L2, SAL1L2, VL1L2, VAL1L2, NBRCNT
   //
   else if(lt == stat_sl1l2  ||
           lt == stat_sal1l2 ||
           lt == stat_vl1l2  ||
           lt == stat_val1l2 ||
           lt == stat_nbrcnt) {
      aggr_psum_lines(f, j, psum_map, n_in, n_out);
      write_job_aggr_psum(j, lt, psum_map, out_at);
   }

   //
   // Sum the ISC line types
   //
   else if(lt == stat_isc) {
      aggr_isc_lines(f, j, isc_map, n_in, n_out);
      write_job_aggr_isc(j, lt, isc_map, out_at);
   }

   //
   // Sum the RHIST line types
   //
   else if(lt == stat_rhist) {
      aggr_rhist_lines(f, j, rhist_map, n_in, n_out);
      write_job_aggr_rhist(j, lt, rhist_map, out_at);
   }

   //
   // Sum the SSVAR line types
   //
   else if(lt == stat_ssvar) {
      aggr_ssvar_lines(f, j, ssvar_map, n_in, n_out);
      write_job_aggr_ssvar(j, lt, ssvar_map, out_at);
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
   // Write the ASCII Table and the job command line
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
   AsciiTable out_at;
   int i, n;

   map<ConcatString, AggrCTCInfo>   ctc_map;
   map<ConcatString, AggrMCTCInfo>  mctc_map;
   map<ConcatString, AggrPCTInfo>   pct_map;
   map<ConcatString, AggrPSumInfo>  psum_map;
   map<ConcatString, AggrWindInfo>  wind_map;
   map<ConcatString, AggrORANKInfo> orank_map;
   map<ConcatString, AggrMPRInfo>   mpr_map;

   //
   // Check that the -line_type option has been supplied only once
   //
   if(j.line_type.n_elements() != 1) {
      mlog << Error << "\ndo_job_aggr_stat() -> "
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
   // Valid combinations of input and output line types:
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

   //
   // Sum up the contingency table type lines:
   //    FHO, CTC -> CTS
   //    NBRCTC -> NBRCTS
   //
   if(((in_lt  == stat_fho ||
        in_lt  == stat_ctc) &&
        out_lt == stat_cts) ||
      (in_lt   == stat_nbrctc &&
       out_lt  == stat_nbrcts)) {
      aggr_ctc_lines(f, j, ctc_map, n_in, n_out);
      write_job_aggr_ctc(j, out_lt, ctc_map, out_at);
   }

   //
   // Sum up the multi-category contingency table type lines:
   //    MCTC -> MCTS
   //
   else if(in_lt  == stat_mctc &&
           out_lt == stat_mcts) {
      aggr_mctc_lines(f, j, mctc_map, n_in, n_out);
      write_job_aggr_mctc(j, out_lt, mctc_map, out_at);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT -> PSTD, PJC, PRC
   //
   else if(  in_lt == stat_pct &&
           (out_lt == stat_pstd ||
            out_lt == stat_pjc  ||
            out_lt == stat_prc)) {
      aggr_pct_lines(f, j, pct_map, n_in, n_out);
      write_job_aggr_pct(j, out_lt, pct_map, out_at);
   }

   //
   // Sum the scalar partial sum line types:
   //    SL1L2, SAL1L2 -> CNT
   //    NBRCTC -> NBRCNT
   //
   else if((in_lt  == stat_sl1l2 ||
            in_lt  == stat_sal1l2) &&
            out_lt == stat_cnt) {
      aggr_psum_lines(f, j, psum_map, n_in, n_out);
      write_job_aggr_psum(j, out_lt, psum_map, out_at);
   }

   //
   // Sum the vector partial sum line types:
   //    VL1L2, VAL1L2 -> WDIR
   //
   else if((in_lt  == stat_vl1l2 ||
            in_lt  == stat_val1l2) &&
            out_lt == stat_wdir) {
      aggr_wind_lines(f, j, wind_map, n_in, n_out);
      write_job_aggr_wind(j, out_lt, wind_map, out_at);
   }

   //
   // Sum the observation rank line types:
   //    ORANK -> RHIST
   //
   else if(in_lt  == stat_orank &&
           out_lt == stat_rhist) {
      aggr_orank_lines(f, j, orank_map, n_in, n_out);
      write_job_aggr_orank(j, out_lt, orank_map, out_at);
   }

   //
   // Read the matched pair lines:
   //    MPR -> FHO, CTC, CTS, MCTC, MCTS, CNT,
   //           SL1L2, SAL1L2, PCT, PSTD, PJC, PRC
   //
   else if(in_lt == stat_mpr &&
           (out_lt == stat_fho   || out_lt == stat_ctc    ||
            out_lt == stat_cts   || out_lt == stat_mctc   ||
            out_lt == stat_mcts  || out_lt == stat_cnt    ||
            out_lt == stat_sl1l2 || out_lt == stat_sal1l2 ||
            out_lt == stat_pct   || out_lt == stat_pstd   ||
            out_lt == stat_pjc   || out_lt == stat_prc)) {

      //
      // Check output threshold values for 2x2 contingency table
      //
      if(out_lt == stat_fho ||
         out_lt == stat_ctc ||
         out_lt == stat_cts) {

         if(j.out_fcst_thresh.n_elements() != 1 ||
            j.out_obs_thresh.n_elements()  != 1) {
            mlog << Error << "\ndo_job_aggr_stat() -> "
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
            mlog << Error << "\ndo_job_aggr_stat() -> "
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

         if(j.out_obs_thresh.n_elements() != 1) {
            mlog << Error << "\ndo_job_aggr_stat() -> "
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
      aggr_mpr_lines(f, j, mpr_map, n_in, n_out);
      write_job_aggr_mpr(j, out_lt, mpr_map, out_at, tmp_dir);
   }

   //
   // Invalid combination of input and output line types
   //
   else {
      mlog << Error << "\ndo_job_aggr_stat() -> "
           << "invalid combination of \"-line_type "
           << statlinetype_to_string(in_lt) << "\" and "
           << "\"-out_line_type " << statlinetype_to_string(out_lt)
           << "\"\n\n";
      throw(1);
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
   // Write the Ascii Table and the job command line
   //
   write_jobstring(jobstring, sa_out);
   write_table(out_at, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_hdr(STATAnalysisJob &j, int n_row, int n_col,
                        AsciiTable &at) {
   int i, c;

   //
   // Setup the output table
   //
   at.set_size(n_row, n_col);
   setup_table(at);

   //
   // Write the header information
   //
   c = 0;
   at.set_entry(0, c++, "COL_NAME:");
   for(i=0; i<j.column_case.n_elements(); i++) {
      at.set_entry(0, c++, j.column_case[i]);
   } // end for i

   return;
}   

////////////////////////////////////////////////////////////////////////

void write_job_aggr_ctc(STATAnalysisJob &j, STATLineType lt,
                        map<ConcatString, AggrCTCInfo> &m,
                        AsciiTable &at) {
   map<ConcatString, AggrCTCInfo>::iterator it;
   int n_row, n_col, r, c;
   NBRCTSInfo nbrcts_info;

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + j.column_case.n_elements();
        if(lt == stat_fho)    n_col += n_fho_columns;
   else if(lt == stat_ctc)    n_col += n_ctc_columns;
   else if(lt == stat_cts)    n_col += n_cts_columns;
   else if(lt == stat_nbrctc) n_col += n_nbrctc_columns;
   else if(lt == stat_nbrcts) n_col += n_nbrcts_columns;
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
        if(lt == stat_fho)    write_header_row(fho_columns,    n_fho_columns,    0, at, 0, c);
   else if(lt == stat_ctc)    write_header_row(ctc_columns,    n_ctc_columns,    0, at, 0, c);
   else if(lt == stat_cts)    write_header_row(cts_columns,    n_cts_columns,    0, at, 0, c);
   else if(lt == stat_nbrctc) write_header_row(nbrctc_columns, n_nbrctc_columns, 0, at, 0, c);
   else if(lt == stat_nbrcts) write_header_row(nbrcts_columns, n_nbrcts_columns, 0, at, 0, c);
   
   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // FHO output line
      //
      if(lt == stat_fho) {
         at.set_entry(r, c++, "FHO:");
         write_case_cols(it->first, at, r, c);
         write_fho_cols(it->second.cts_info, at, r, c);
      }
      //
      // CTC output line
      //
      else if(lt == stat_ctc) {
         at.set_entry(r, c++, "CTC:");
         write_case_cols(it->first, at, r, c);
         write_ctc_cols(it->second.cts_info, at, r, c);
      }
      //
      // CTS output line
      //
      else if(lt == stat_cts) {

         //
         // Store the alpha information in the CTSInfo object
         //
         it->second.cts_info.allocate_n_alpha(1);
         it->second.cts_info.alpha[0] = j.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // CTSInfo object
         //
         it->second.cts_info.compute_stats();
         it->second.cts_info.compute_ci();

         //
         // Write the data line
         //
         at.set_entry(r, c++, "CTS:");
         write_case_cols(it->first, at, r, c);
         write_cts_cols(it->second.cts_info, 0, at, r, c);
      }
      //
      // NBRCTC output line
      //
      else if(lt == stat_nbrctc) {

         nbrcts_info.clear();
         nbrcts_info.cts_info = it->second.cts_info;
        
         at.set_entry(r, c++, "NBRCTC:");
         write_case_cols(it->first, at, r, c);
         write_nbrctc_cols(nbrcts_info, at, r, c);
      }
      //
      // NBRCTS output line
      //
      else if(lt == stat_nbrcts) {

         nbrcts_info.clear();
         nbrcts_info.cts_info = it->second.cts_info;

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
         // Write the data line
         //
         at.set_entry(r, c++, "NBRCTS:");
         write_case_cols(it->first, at, r, c);
         write_nbrcts_cols(nbrcts_info, 0, at, r, c);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_mctc(STATAnalysisJob &j, STATLineType lt,
                         map<ConcatString, AggrMCTCInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrMCTCInfo>::iterator it;
   int n, n_row, n_col, r, c;

   //
   // Determine the maximum MCTC dimension
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.mcts_info.cts.nrows(), n);
   }

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + j.column_case.n_elements();
        if(lt == stat_mctc) n_col += get_n_mctc_columns(n);
   else if(lt == stat_mcts) n_col += n_mcts_columns;
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
        if(lt == stat_mctc) write_mctc_header_row(0, n, at, 0, c);
   else if(lt == stat_mcts) write_header_row(mcts_columns, n_mcts_columns, 0, at, 0, c);

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // MCTC output line
      //
      if(lt == stat_mctc) {
         at.set_entry(r, c++, "MCTC:");
         write_case_cols(it->first, at, r, c);
         write_mctc_cols(it->second.mcts_info, at, r, c);
      }
      //
      // MCTS output line
      //
      else if(lt == stat_mcts) {

         //
         // Store the alpha information in the CTSInfo object
         //
         it->second.mcts_info.allocate_n_alpha(1);
         it->second.mcts_info.alpha[0] = j.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // MCTSInfo object
         //
         it->second.mcts_info.compute_stats();
         it->second.mcts_info.compute_ci();

         //
         // Write the data line
         //
         at.set_entry(r, c++, "MCTS:");
         write_case_cols(it->first, at, r, c);
         write_mcts_cols(it->second.mcts_info, 0, at, r, c);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_pct(STATAnalysisJob &j, STATLineType lt,
                        map<ConcatString, AggrPCTInfo> &m,
                        AsciiTable &at) {
   map<ConcatString, AggrPCTInfo>::iterator it;
   int n, n_row, n_col, r, c;
   
   //
   // Determine the maximum PCT dimension
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.pct_info.pct.nrows() + 1, n);
   }

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + j.column_case.n_elements();
        if(lt == stat_pct)  n_col += get_n_pct_columns(n);
   else if(lt == stat_pstd) n_col += get_n_pstd_columns(n);
   else if(lt == stat_pjc)  n_col += get_n_pjc_columns(n);
   else if(lt == stat_prc)  n_col += get_n_prc_columns(n);
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
        if(lt == stat_pct)  write_pct_header_row (0, n, at, 0, c);
   else if(lt == stat_pstd) write_pstd_header_row(0, n, at, 0, c);
   else if(lt == stat_pjc)  write_pjc_header_row (0, n, at, 0, c);
   else if(lt == stat_prc)  write_prc_header_row (0, n, at, 0, c);
   
   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // PCT output line
      //
      if(lt == stat_pct) {
         at.set_entry(r, c++, "PCT:");
         write_case_cols(it->first, at, r, c);
         write_pct_cols(it->second.pct_info, at, r, c);
      }
      //
      // PSTD output line
      //
      else if(lt == stat_pstd) {

         //
         // Store the alpha information in the PCTInfo object
         //
         it->second.pct_info.allocate_n_alpha(1);
         it->second.pct_info.alpha[0] = j.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // MCTSInfo object
         //
         it->second.pct_info.compute_stats();
         it->second.pct_info.compute_ci();

         //
         // Write the data line
         //
         at.set_entry(r, c++, "PSTD:");
         write_case_cols(it->first, at, r, c);
         write_pstd_cols(it->second.pct_info, 0, at, r, c);
      }
      //
      // PJC output line
      //
      else if(lt == stat_pjc) {
         at.set_entry(r, c++, "PJC:");
         write_case_cols(it->first, at, r, c);
         write_pjc_cols(it->second.pct_info, at, r, c);
      }
      //
      // PRC output line
      //
      else if(lt == stat_prc) {
         at.set_entry(r, c++, "PRC:");
         write_case_cols(it->first, at, r, c);
         write_prc_cols(it->second.pct_info, at, r, c);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_psum(STATAnalysisJob &j, STATLineType lt,
                         map<ConcatString, AggrPSumInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrPSumInfo>::iterator it;
   int n_row, n_col, r, c;

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + j.column_case.n_elements();
        if(lt == stat_sl1l2)  n_col += n_sl1l2_columns;
   else if(lt == stat_sal1l2) n_col += n_sal1l2_columns;
   else if(lt == stat_vl1l2)  n_col += n_vl1l2_columns;
   else if(lt == stat_val1l2) n_col += n_val1l2_columns;
   else if(lt == stat_cnt)    n_col += n_cnt_columns;
   else if(lt == stat_nbrcnt) n_col += n_nbrcnt_columns;
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
        if(lt == stat_sl1l2)  write_header_row(sl1l2_columns,  n_sl1l2_columns,  0, at, 0, c);
   else if(lt == stat_sal1l2) write_header_row(sal1l2_columns, n_sal1l2_columns, 0, at, 0, c);
   else if(lt == stat_vl1l2)  write_header_row(vl1l2_columns,  n_vl1l2_columns,  0, at, 0, c);
   else if(lt == stat_val1l2) write_header_row(val1l2_columns, n_val1l2_columns, 0, at, 0, c);
   else if(lt == stat_cnt)    write_header_row(cnt_columns,    n_cnt_columns,    0, at, 0, c);
   else if(lt == stat_nbrcnt) write_header_row(nbrcnt_columns, n_nbrcnt_columns, 0, at, 0, c);

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // SL1L2 output line
      //
      if(lt == stat_sl1l2) {
         at.set_entry(r, c++, "SL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sl1l2_cols(it->second.sl1l2_info, at, r, c);
      }
      //
      // SAL1L2 output line
      //
      else if(lt == stat_sal1l2) {
         at.set_entry(r, c++, "SAL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sal1l2_cols(it->second.sl1l2_info, at, r, c);
      }
      //
      // VL1L2 output line
      //
      else if(lt == stat_vl1l2) {
         at.set_entry(r, c++, "VL1L2:");
         write_case_cols(it->first, at, r, c);
         write_vl1l2_cols(it->second.vl1l2_info, at, r, c);
      }
      //
      // VAL1L2 output line
      //
      else if(lt == stat_val1l2) {
         at.set_entry(r, c++, "VAL1L2:");
         write_case_cols(it->first, at, r, c);
         write_val1l2_cols(it->second.vl1l2_info, at, r, c);
      }
      //
      // CNT output line
      //
      else if(lt == stat_cnt) {

         it->second.cnt_info.clear();
         
         //
         // Allocate space for confidence intervals
         //
         it->second.cnt_info.allocate_n_alpha(1);
         it->second.cnt_info.alpha[0] = j.out_alpha;

         //
         // Compute CNTInfo statistics from the aggregated partial sums
         //
         if(it->second.sl1l2_info.scount > 0)
            compute_cntinfo(it->second.sl1l2_info, 0, it->second.cnt_info);
         else
            compute_cntinfo(it->second.sl1l2_info, 1, it->second.cnt_info);
         
         at.set_entry(r, c++, "CNT:");
         write_case_cols(it->first, at, r, c);
         write_cnt_cols(it->second.cnt_info, 0, at, r, c);
      }
      //
      // NBRCNT output line
      //
      else if(lt == stat_nbrcnt) {
         at.set_entry(r, c++, "NBRCNT:");
         write_case_cols(it->first, at, r, c);
         write_nbrcnt_cols(it->second.nbrcnt_info, 0, at, r, c);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_wind(STATAnalysisJob &j, STATLineType lt,
                         map<ConcatString, AggrWindInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrWindInfo>::iterator it;
   int i, n, n_row, n_col, r, c, count;
   double uf, vf, uo, vo, fbar, obar;
   double angle, me, mae;

   //
   // Setup the output table
   //
   n_row = 1 + (2 * m.size());
   n_col = 1 + j.column_case.n_elements() + n_job_wdir_columns;
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
   write_header_row(job_wdir_columns, n_job_wdir_columns, 0, at, 0, c);

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // Check for matching component lengths
      //
      if(it->second.uf_na.n_elements() != it->second.vf_na.n_elements() ||
         it->second.uo_na.n_elements() != it->second.vo_na.n_elements() ||
         it->second.uf_na.n_elements() != it->second.uo_na.n_elements()) {
         mlog << Error << "\nwrite_job_aggr_wind() -> "
              << "the number of U and V forecast and observation points "
              << "must be the same.\n\n";
         throw(1);
      }

      //
      // Store the number of vectors
      //
      n = it->second.uf_na.n_elements();

      //
      // Compute the mean forecast and observation angles
      // from the unit vectors
      //
      uf   = it->second.uf_na.sum()/it->second.uf_na.n_elements();
      vf   = it->second.vf_na.sum()/it->second.vf_na.n_elements();
      fbar = convert_u_v_to_wdir(uf, vf);
      uo   = it->second.uo_na.sum()/it->second.uo_na.n_elements();
      vo   = it->second.vo_na.sum()/it->second.vo_na.n_elements();
      obar = convert_u_v_to_wdir(uo, vo);

      mlog << Debug(4) << "write_job_aggr_wind() -> "
           << "ROW_MEAN_WDIR: average forecast direction (u, v) = ("
           << uf << ", "  << vf << ") = " << fbar << " degrees\n";

      mlog << Debug(4) << "write_job_aggr_wind() -> "
           << "ROW_MEAN_WDIR: average observed direction (u, v) = ("
           << uo << ", "  << vo << ") = " << obar << " degrees\n";

      //
      // Compute the mean error and the mean absolute error
      // from the unit vectors
      //
      me = mae = 0.0;
      for(i=0; i<n; i++) {

         angle = angle_difference(it->second.uf_na[i], it->second.vf_na[i],
                                  it->second.uo_na[i], it->second.vo_na[i]);

         if(mlog.verbosity_level() > 3) {
            mlog << Debug(4) << "write_job_aggr_wind() -> "
                 << "ROW_MEAN_WDIR: [" << i+1 << "] difference of forecast direction "
                 << convert_u_v_to_wdir(it->second.uf_na[i], it->second.vf_na[i])
                 << " - observed direction "
                 << convert_u_v_to_wdir(it->second.uo_na[i], it->second.vo_na[i])
                 << " = " << angle << " degrees\n";
         }

         // Check for bad data
         if(is_eq(angle, bad_data_double)) {
            me  = bad_data_double;
            mae = bad_data_double;
            break;
         }

         me  += angle;
         mae += fabs(angle);
      } // end for i
      if(!is_eq(me,  bad_data_double)) me  /= n;
      if(!is_eq(mae, bad_data_double)) mae /= n;

      //
      // Write the mean wind direction statistics
      //
      at.set_entry(r, 0, "ROW_MEAN_WDIR:");
      write_case_cols(it->first, at, r, c);      
      at.set_entry(r, c++, n);
      at.set_entry(r, c++, fbar);
      at.set_entry(r, c++, obar);
      at.set_entry(r, c++, me);
      at.set_entry(r, c++, mae);

      if(lt == stat_vl1l2) {
         uf    = it->second.vl1l2_info.ufbar;
         vf    = it->second.vl1l2_info.vfbar;
         uo    = it->second.vl1l2_info.uobar;
         vo    = it->second.vl1l2_info.vobar;
         count = it->second.vl1l2_info.vcount;
      }
      else if(lt == stat_val1l2) {
         uf    = it->second.vl1l2_info.ufabar;
         vf    = it->second.vl1l2_info.vfabar;
         uo    = it->second.vl1l2_info.uoabar;
         vo    = it->second.vl1l2_info.voabar;
         count = it->second.vl1l2_info.vacount;
      }

      //
      // Compute the aggregated forecast and observation angles
      //
      fbar = convert_u_v_to_wdir(uf, vf);
      obar = convert_u_v_to_wdir(uo, vo);
      me   = angle_difference(uf, vf, uo, vo);
      mae  = bad_data_double;

      mlog << Debug(4) << "write_job_aggr_wind() -> "
           << "AGGR_WDIR: aggregated forecast direction (u, v) = ("
           << uf << ", "  << vf << ") = " << fbar << " degrees\n";

      mlog << Debug(4) << "write_job_aggr_wind() -> "
           << "AGGR_WDIR: aggregated observed direction (u, v) = ("
           << uo << ", "  << vo << ") = " << obar << " degrees\n";

      //
      // Write the aggregated wind direction statistics
      //
      at.set_entry(r, 0, "AGGR_WDIR:");
      write_case_cols(it->first, at, r, c);
      at.set_entry(r, c++, count);
      at.set_entry(r, c++, fbar);
      at.set_entry(r, c++, obar);
      at.set_entry(r, c++, me);
      at.set_entry(r, c++, mae);

   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_rhist(STATAnalysisJob &j, STATLineType lt,
                          map<ConcatString, AggrRHISTInfo> &m,
                          AsciiTable &at) {
   map<ConcatString, AggrRHISTInfo>::iterator it;
   int n, n_row, n_col, r, c;

   //
   // Determine the maximum number of ranks
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.ens_pd.rhist_na.n_elements(), n);
   }
   
   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + j.column_case.n_elements() + get_n_rhist_columns(n);
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
   write_rhist_header_row(0, n, at, 0, c);

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // RHIST output line
      //
      at.set_entry(r, c++, "RHIST:");
      write_case_cols(it->first, at, r, c);
      write_rhist_cols(&(it->second.ens_pd), at, r, c);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_ssvar(STATAnalysisJob &j, STATLineType lt,
                          map<ConcatString, AggrSSVARInfo> &m,
                          AsciiTable &at) {
   map<ConcatString, AggrSSVARInfo>::iterator case_it;
   map<ConcatString, SSVARInfo>::iterator bin_it;
   int i, n, n_row, n_col, r, c;

   //
   // Determine the number of rows
   //
   for(case_it = m.begin(), n = 0; case_it != m.end(); case_it++) {
      n += case_it->second.ssvar_bins.size();
   } // end for case_it

   //
   // Setup the output table
   //
   n_row = 1 + n;
   n_col = 1 + j.column_case.n_elements() + n_ssvar_columns;
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
   write_header_row(ssvar_columns, n_ssvar_columns, 0, at, 0, c);

   //
   // Loop through the case map
   //
   for(case_it = m.begin(), r=1; case_it != m.end(); case_it++) {

      //
      // Loop through the bin map to determine the total count
      //
      for(bin_it = case_it->second.ssvar_bins.begin(), n=0;
          bin_it != case_it->second.ssvar_bins.end();
          bin_it++) {
         n += bin_it->second.bin_n;
      }

      //
      // Loop through the bin map and write the output
      //
      for(bin_it = case_it->second.ssvar_bins.begin(), i=0;
          bin_it != case_it->second.ssvar_bins.end();
          bin_it++, r++, i++) {

         //
         // Initialize
         //
         c = 0;

         //
         // SSVAR output line
         //
         at.set_entry(r, c++, "SSVAR:");
         write_case_cols(case_it->first, at, r, c);
         at.set_entry(r, c++, n);
         at.set_entry(r, c++, (int) case_it->second.ssvar_bins.size());
         at.set_entry(r, c++, i);
         at.set_entry(r, c++, bin_it->second.bin_n);
         at.set_entry(r, c++, bin_it->second.var_min);
         at.set_entry(r, c++, bin_it->second.var_max);
         at.set_entry(r, c++, bin_it->second.var_mean);
         at.set_entry(r, c++, bin_it->second.fbar);
         at.set_entry(r, c++, bin_it->second.obar);
         at.set_entry(r, c++, bin_it->second.fobar);
         at.set_entry(r, c++, bin_it->second.ffbar);
         at.set_entry(r, c++, bin_it->second.oobar);

      } // end for bin_it
   } // end for case_it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_orank(STATAnalysisJob &j, STATLineType lt,
                          map<ConcatString, AggrORANKInfo> &m,
                          AsciiTable &at) {
   map<ConcatString, AggrORANKInfo>::iterator it;
   int n, n_row, n_col, r, c;

   //
   // Determine the maximum number of ranks
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.ens_pd.rhist_na.n_elements(), n);
   }

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + j.column_case.n_elements() + get_n_rhist_columns(n);
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
   write_rhist_header_row(0, n, at, 0, c);

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // RHIST output line
      //
      at.set_entry(r, c++, "RHIST:");
      write_case_cols(it->first, at, r, c);
      write_rhist_cols(&(it->second.ens_pd), at, r, c);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_isc(STATAnalysisJob &j, STATLineType lt,
                        map<ConcatString, AggrISCInfo> &m,
                        AsciiTable &at) {
   map<ConcatString, AggrISCInfo>::iterator it;
   int i, n, n_row, n_col, r, c;
   
   //
   // Determine the maximum number of scales
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.isc_info.n_scale + 2, n);
   }

   //
   // Setup the output table
   //
   n_row = 1 + (n * m.size());
   n_col = 1 + j.column_case.n_elements() + n_isc_columns;
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
   write_header_row(isc_columns, n_isc_columns, 0, at, 0, c);

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++) {
     
      //
      // ISC output line
      //
      for(i=-1, c=0; i<=it->second.isc_info.n_scale; i++, r++, c=0) {
         at.set_entry(r, c++, "ISC:");
         write_case_cols(it->first, at, r, c);
         write_isc_cols(it->second.isc_info, i, at, r, c);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_mpr(STATAnalysisJob &j, STATLineType lt,
                        map<ConcatString, AggrMPRInfo> &m,
                        AsciiTable &at, const char *tmp_dir) {
   map<ConcatString, AggrMPRInfo>::iterator it;
   int n_row, n_col, r, c;

   CTSInfo   cts_info;
   MCTSInfo  mcts_info;
   CNTInfo   cnt_info;
   SL1L2Info sl1l2_info;
   PCTInfo   pct_info;

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + j.column_case.n_elements();
        if(lt == stat_fho)    n_col += n_fho_columns;
   else if(lt == stat_ctc)    n_col += n_ctc_columns;
   else if(lt == stat_cts)    n_col += n_cts_columns;
   else if(lt == stat_mctc)   n_col += get_n_mctc_columns(j.out_fcst_thresh.n_elements()+1);
   else if(lt == stat_mcts)   n_col += n_mcts_columns;
   else if(lt == stat_cnt)    n_col += n_cnt_columns;
   else if(lt == stat_sl1l2)  n_col += n_sl1l2_columns;
   else if(lt == stat_sal1l2) n_col += n_sal1l2_columns;
   else if(lt == stat_pct)    n_col += get_n_pct_columns(j.out_fcst_thresh.n_elements());
   else if(lt == stat_pstd)   n_col += get_n_pstd_columns(j.out_fcst_thresh.n_elements());
   else if(lt == stat_pjc)    n_col += get_n_pjc_columns(j.out_fcst_thresh.n_elements());
   else if(lt == stat_prc)    n_col += get_n_prc_columns(j.out_fcst_thresh.n_elements());
   write_job_aggr_hdr(j, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + j.column_case.n_elements();
        if(lt == stat_fho)    write_header_row(fho_columns,    n_fho_columns,    0, at, 0, c);
   else if(lt == stat_ctc)    write_header_row(ctc_columns,    n_ctc_columns,    0, at, 0, c);
   else if(lt == stat_cts)    write_header_row(cts_columns,    n_cts_columns,    0, at, 0, c);
   else if(lt == stat_mctc)   write_mctc_header_row(0, j.out_fcst_thresh.n_elements(), at, 0, c);
   else if(lt == stat_mcts)   write_header_row(mcts_columns,   n_mcts_columns,   0, at, 0, c);
   else if(lt == stat_cnt)    write_header_row(cnt_columns,    n_cnt_columns,    0, at, 0, c);
   else if(lt == stat_sl1l2)  write_header_row(sl1l2_columns,  n_sl1l2_columns,  0, at, 0, c);
   else if(lt == stat_sal1l2) write_header_row(sal1l2_columns, n_sal1l2_columns, 0, at, 0, c);
   else if(lt == stat_pct)    write_pct_header_row (0, j.out_fcst_thresh.n_elements(), at, 0, c);
   else if(lt == stat_pstd)   write_pstd_header_row(0, j.out_fcst_thresh.n_elements(), at, 0, c);
   else if(lt == stat_pjc)    write_pjc_header_row (0, j.out_fcst_thresh.n_elements(), at, 0, c);
   else if(lt == stat_prc)    write_prc_header_row (0, j.out_fcst_thresh.n_elements(), at, 0, c);

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Initialize
      //
      c = 0;

      //
      // FHO output line
      //
      if(lt == stat_fho) {
         mpr_to_ctc(j, it->second, cts_info);
         at.set_entry(r, c++, "FHO:");
         write_case_cols(it->first, at, r, c);
         write_fho_cols(cts_info, at, r, c);
      }
      //
      // CTC output line
      //
      else if(lt == stat_ctc) {
         mpr_to_ctc(j, it->second, cts_info);
         at.set_entry(r, c++, "CTC:");
         write_case_cols(it->first, at, r, c);
         write_ctc_cols(cts_info, at, r, c);
      }
      //
      // CTS output line
      //
      else if(lt == stat_cts) {
         mpr_to_cts(j, it->second, cts_info, tmp_dir);
         at.set_entry(r, c++, "CTS:");
         write_case_cols(it->first, at, r, c);
         write_cts_cols(cts_info, 0, at, r, c);
      }
      //
      // MCTC output line
      //
      else if(lt == stat_mctc) {
         mpr_to_mctc(j, it->second, mcts_info);
         at.set_entry(r, c++, "MCTC:");
         write_case_cols(it->first, at, r, c);
         write_mctc_cols(mcts_info, at, r, c);
      }
      //
      // MCTS output line
      //
      else if(lt == stat_mcts) {
         mpr_to_mcts(j, it->second, mcts_info, tmp_dir);
         at.set_entry(r, c++, "MCTS:");
         write_case_cols(it->first, at, r, c);
         write_mcts_cols(mcts_info, 0, at, r, c);
      }
      //
      // CNT output line
      //
      else if(lt == stat_cnt) {
         mpr_to_cnt(j, it->second, cnt_info, tmp_dir);
         at.set_entry(r, c++, "CNT:");
         write_case_cols(it->first, at, r, c);
         write_cnt_cols(cnt_info, 0, at, r, c);
      }
      //
      // SL1L2 output line
      //
      else if(lt == stat_sl1l2) {
         mpr_to_psum(j, it->second, sl1l2_info);
         at.set_entry(r, c++, "SL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sl1l2_cols(sl1l2_info, at, r, c);
      }
      //
      // SAL1L2 output line
      //
      else if(lt == stat_sal1l2) {
         mpr_to_psum(j, it->second, sl1l2_info);
         at.set_entry(r, c++, "SAL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sal1l2_cols(sl1l2_info, at, r, c);
      }
      //
      // PCT output line
      //
      else if(lt == stat_pct) {
         mpr_to_pct(j, it->second, pct_info);
         at.set_entry(r, c++, "PCT:");
         write_case_cols(it->first, at, r, c);
         write_pct_cols(pct_info, at, r, c);
      }
      //
      // PSTD output line
      //
      else if(lt == stat_pstd) {
         mpr_to_pct(j, it->second, pct_info);
         at.set_entry(r, c++, "PSTD:");
         write_case_cols(it->first, at, r, c);
         write_pstd_cols(pct_info, 0, at, r, c);
      }
      //
      // PJC output line
      //
      else if(lt == stat_pjc) {
         mpr_to_pct(j, it->second, pct_info);
         at.set_entry(r, c++, "PJC:");
         write_case_cols(it->first, at, r, c);
         write_pjc_cols(pct_info, at, r, c);
      }
      //
      // PRC output line
      //
      else if(lt == stat_prc) {
         mpr_to_pct(j, it->second, pct_info);        
         at.set_entry(r, c++, "PRC:");
         write_case_cols(it->first, at, r, c);
         write_prc_cols(pct_info, at, r, c);
      }
   } // end for it

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
   double go_index;
   AsciiTable out_at;

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
      mlog << Error << "\ncompute_ss_index() -> "
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
      mlog << Error << "\ncompute_ss_index() -> "
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
      mlog << Error << "\ncompute_ss_index() -> "
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

void write_case_cols(const ConcatString &cs, AsciiTable &at,
                     int &r, int &c) {
   StringArray sa = cs.split(":");

   for(int i=0; i<sa.n_elements(); i++) at.set_entry(r, c++, sa[i]);

   return;
}
           
////////////////////////////////////////////////////////////////////////
