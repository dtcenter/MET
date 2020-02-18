// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
//   008    05/08/13  Halley Gotway   Fix bug in write_job_aggr_wind().
//   009    05/27/14  Halley Gotway   Range check the columns being read.
//   010    06/03/14  Halley Gotway   Add aggregate PHIST lines.
//   011    07/28/14  Halley Gotway   Add aggregate_stat for MPR to WDIR.
//   012    02/12/15  Halley Gotway   Write STAT output lines.
//   013    03/30/15  Halley Gotway   Add ramp job type.
//   014    06/09/17  Halley Gotway   Add aggregate RELP lines.
//   015    06/28/17  Halley Gotway   Add aggregate_stat for CTC, PCT,
//                    or MPR to ECLV lines.
//   016    10/09/17  Halley Gotway   Add aggregate GRAD lines.
//   017    03/01/18  Halley Gotway   Update summary job type.
//   018    04/04/19  Fillmore        Added FCST and OBS units.
//   019    01/24/20  Halley Gotway   Add aggregate RPS lines.
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

#include "vx_time_series.h"
#include "vx_log.h"

#include "stat_analysis_job.h"
#include "parse_stat_line.h"
#include "aggr_stat_line.h"

////////////////////////////////////////////////////////////////////////

void set_job_from_config(MetConfig &c, STATAnalysisJob &job) {
   BootInfo boot_info;

   //
   // Parse top-level configuration filtering options
   //

   job.model           = c.lookup_string_array(conf_key_model, false);
   job.desc            = c.lookup_string_array(conf_key_desc, false);
   job.fcst_lead       = c.lookup_seconds_array(conf_key_fcst_lead, false);
   job.obs_lead        = c.lookup_seconds_array(conf_key_obs_lead, false);
   job.fcst_valid_beg  = c.lookup_unixtime(conf_key_fcst_valid_beg, false);
   job.fcst_valid_end  = c.lookup_unixtime(conf_key_fcst_valid_end, false);
   job.fcst_valid_hour = c.lookup_seconds_array(conf_key_fcst_valid_hour, false);
   job.obs_valid_beg   = c.lookup_unixtime(conf_key_obs_valid_beg, false);
   job.obs_valid_end   = c.lookup_unixtime(conf_key_obs_valid_end, false);
   job.obs_valid_hour  = c.lookup_seconds_array(conf_key_obs_valid_hour, false);
   job.fcst_init_beg   = c.lookup_unixtime(conf_key_fcst_init_beg, false);
   job.fcst_init_end   = c.lookup_unixtime(conf_key_fcst_init_end, false);
   job.fcst_init_hour  = c.lookup_seconds_array(conf_key_fcst_init_hour, false);
   job.obs_init_beg    = c.lookup_unixtime(conf_key_obs_init_beg, false);
   job.obs_init_end    = c.lookup_unixtime(conf_key_obs_init_end, false);
   job.obs_init_hour   = c.lookup_seconds_array(conf_key_obs_init_hour, false);
   job.fcst_var        = c.lookup_string_array(conf_key_fcst_var, false);
   job.obs_var         = c.lookup_string_array(conf_key_obs_var, false);
   job.fcst_units      = c.lookup_string_array(conf_key_fcst_units, false);
   job.obs_units       = c.lookup_string_array(conf_key_obs_units, false);
   job.fcst_lev        = c.lookup_string_array(conf_key_fcst_lev, false);
   job.obs_lev         = c.lookup_string_array(conf_key_obs_lev, false);
   job.obtype          = c.lookup_string_array(conf_key_obtype, false);
   job.vx_mask         = c.lookup_string_array(conf_key_vx_mask, false);
   job.interp_mthd     = c.lookup_string_array(conf_key_interp_mthd, false);
   job.interp_pnts     = c.lookup_num_array(conf_key_interp_pnts, false);
   job.fcst_thresh     = c.lookup_thresh_array(conf_key_fcst_thresh, false);
   job.obs_thresh      = c.lookup_thresh_array(conf_key_obs_thresh, false);
   job.cov_thresh      = c.lookup_thresh_array(conf_key_cov_thresh, false);
   job.alpha           = c.lookup_num_array(conf_key_alpha, false);
   job.line_type       = c.lookup_string_array(conf_key_line_type, false);
   job.column          = c.lookup_string_array(conf_key_column, false);
   job.weight          = c.lookup_num_array(conf_key_weight, false);

   job.out_alpha       = c.lookup_double(conf_key_out_alpha, false);

   boot_info         = parse_conf_boot(&c);
   job.boot_interval   = boot_info.interval;
   job.boot_rep_prop   = boot_info.rep_prop;
   job.n_boot_rep      = boot_info.n_rep;
   job.set_boot_rng(boot_info.rng.c_str());
   job.set_boot_seed(boot_info.seed.c_str());

   job.rank_corr_flag  = (int) c.lookup_bool(conf_key_rank_corr_flag);
   job.vif_flag        = (int) c.lookup_bool(conf_key_vif_flag);

   job.wmo_sqrt_stats   = c.lookup_string_array(conf_key_wmo_sqrt_stats, false);
   job.wmo_fisher_stats = c.lookup_string_array(conf_key_wmo_fisher_stats, false);

   //
   // No settings in the default job for column_min_name/value,
   // column_max_name/value, column_str_name/value, and
   // by_column since those are strictly job command options.
   //

   return;
}

////////////////////////////////////////////////////////////////////////

void do_job(const ConcatString &jobstring, STATAnalysisJob &job,
            int n_job, const ConcatString &tmp_dir,
            const ConcatString &tmp_path, ofstream *sa_out) {
   LineDataFile f;
   int n_in, n_out;
   gsl_rng *rng_ptr = (gsl_rng *) 0;

   //
   // Open up the temp file for reading the intermediate STAT line data
   //
   if(!f.open(tmp_path.c_str())) {
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
   if(job.dump_row) {
      mlog << Debug(1) << "Creating dump row output file \"" << job.dump_row
           << "\"\n";
      job.open_dump_row_file();
   }

   //
   // If the -out_stat option was supplied, open the file
   //
   if(job.stat_file) {
      mlog << Debug(1) << "Creating STAT output file \"" << job.stat_file
           << "\"\n";
      job.open_stat_file();
   }

   //
   // Print warning for by_column option
   //
   if(job.by_column.n() > 0              &&
      job.job_type != stat_job_summary   &&
      job.job_type != stat_job_aggr      &&
      job.job_type != stat_job_aggr_stat &&
      job.job_type != stat_job_ramp) {
      mlog << Warning << "\nThe -by option is ignored for the \""
           << statjobtype_to_string(job.job_type) << "\" job type.\n\n";
   }

   //
   // Set up the random number generator and seed value
   // for the summary and aggregate stat jobs.
   //
   if(job.job_type == stat_job_summary ||
      job.job_type == stat_job_aggr_stat) {
      rng_set(rng_ptr, job.boot_rng, job.boot_seed);
   }

   //
   // Switch on the job type
   //
   switch(job.job_type) {

      case(stat_job_filter):
         do_job_filter(jobstring, f, job, n_in, n_out, sa_out);
         break;

      case(stat_job_summary):
         do_job_summary(jobstring, f, job, n_in, n_out, sa_out, rng_ptr);
         break;

      case(stat_job_aggr):
         do_job_aggr(jobstring, f, job, n_in, n_out, sa_out);
         break;

      case(stat_job_aggr_stat):
         do_job_aggr_stat(jobstring, f, job, n_in, n_out, sa_out, tmp_dir, rng_ptr);
         break;

      case(stat_job_go_index):
         do_job_go_index(jobstring, f, job, n_in, n_out, sa_out);
         break;

      case(stat_job_ss_index):
         do_job_ss_index(jobstring, f, job, n_in, n_out, sa_out);
         break;

      case(stat_job_ramp):
         do_job_ramp(jobstring, f, job, n_in, n_out, sa_out);
         break;

      default:
         mlog << Error << "\ndo_job() -> "
              << "Invalid -job type requested!\n\n";
         throw(1);
   }

   mlog << Debug(2) << "Job " << n_job << " used " << n_out << " out of "
        << n_in << " STAT lines.\n";

   //
   // If an output dump row file was created, close it
   //
   if(job.dr_out) job.close_dump_row_file();

   //
   // If an output STAT file was created, close it
   //
   if(job.stat_out) job.close_stat_file();

   //
   // Close the input file stream
   //
   f.close();

   //
   // Deallocate memory for the random number generator
   //
   if(rng_ptr) rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_filter() routine should only be called when the -dump_row
// option has been selected to dump out the filtered STAT lines.
//
////////////////////////////////////////////////////////////////////////

void do_job_filter(const ConcatString &jobstring, LineDataFile &f,
                   STATAnalysisJob &job, int &n_in, int &n_out,
                   ofstream *sa_out) {
   ConcatString out_line;
   STATLine line;

   //
   // Check that the -dump_row option has been supplied
   //
   if(!job.dump_row) {
      mlog << Error << "\ndo_job_filter() -> "
           << "this function may only be called when using the "
           << "-dump_row option in the job command line: "
           << jobstring << "\n\n";
      throw(1);
   }

   mlog << Debug(3) << "Filter Test jobstring:\n" << jobstring << "\n";

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(job.is_keeper(line)) {

         job.dump_stat_line(line);

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
// For do_job_summary() the -line_type and and -column options specify
// the data to be summarized in one of two ways:
// (1) Use -line_type once and -column one or more times.
// (2) Format -column options as LINE_TYPE:COLUMN.
//
////////////////////////////////////////////////////////////////////////

void do_job_summary(const ConcatString &jobstring, LineDataFile &f,
                    STATAnalysisJob &job, int &n_in, int &n_out,
                    ofstream *sa_out, gsl_rng *rng_ptr) {
   map<ConcatString, AggrSummaryInfo> summary_map;
   AsciiTable out_at;

   //
   // Check that the -column option has been supplied
   //
   if(job.column.n() == 0) {
      mlog << Error << "\ndo_job_summary() -> "
           << "the \"-column\" option must be used at least once: "
           << jobstring << "\n\n";
      throw(1);
   }

   //
   // Parse the input stat lines
   //
   aggr_summary_lines(f, job, summary_map, n_in, n_out);

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      mlog << Warning << "\ndo_job_summary() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n\n";
      return;
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
   write_job_summary(job, summary_map, out_at, rng_ptr);
   write_jobstring(jobstring, sa_out);
   write_table(out_at, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The do_job_aggr() routine should only be called when the -line_type
// option has been used exactly once.
//
////////////////////////////////////////////////////////////////////////

void do_job_aggr(const ConcatString &jobstring, LineDataFile &f,
                 STATAnalysisJob &job, int &n_in, int &n_out,
                 ofstream *sa_out) {
   STATLine line;
   STATLineType lt;
   AsciiTable out_at;

   map<ConcatString, AggrCTCInfo>   ctc_map;
   map<ConcatString, AggrMCTCInfo>  mctc_map;
   map<ConcatString, AggrPCTInfo>   pct_map;
   map<ConcatString, AggrPSumInfo>  psum_map;
   map<ConcatString, AggrGRADInfo>  grad_map;
   map<ConcatString, AggrISCInfo>   isc_map;
   map<ConcatString, AggrENSInfo>   ens_map;
   map<ConcatString, AggrRPSInfo>   rps_map;
   map<ConcatString, AggrSSVARInfo> ssvar_map;

   //
   // Check that the -line_type option has been supplied exactly once
   //
   if(job.line_type.n() != 1) {
      mlog << Error << "\ndo_job_aggr() -> "
           << "this function may only be called when the \"-line_type\" "
           << "option has been used exactly once to specify the line "
           << "type for aggregation: " << jobstring << "\n\n";
      throw(1);
   }

   //
   // Determine the line type specified for this job
   //
   lt = string_to_statlinetype(job.line_type[0].c_str());

   //
   // Check that a valid line type has been selected
   //
   if(lt != stat_fho    && lt != stat_ctc    &&
      lt != stat_mctc   && lt != stat_sl1l2  &&
      lt != stat_sal1l2 && lt != stat_vl1l2  &&
      lt != stat_val1l2 && lt != stat_pct    &&
      lt != stat_nbrctc && lt != stat_nbrcnt &&
      lt != stat_grad   && lt != stat_ecnt   &&
      lt != stat_rps    && lt != stat_rhist  &&
      lt != stat_phist  && lt != stat_relp   &&
      lt != stat_ssvar  && lt != stat_isc) {
      mlog << Error << "\ndo_job_aggr() -> "
           << "the \"-line_type\" option must be set to one of:\n"
           << "\tFHO, CTC, MCTC,\n"
           << "\tSL1L2, SAL1L2, VL1L2, VAL1L2,\n"
           << "\tPCT, NBRCTC, NBRCNT, GRAD, ISC,\n"
           << "\tECNT, RPS, RHIST, PHIST, RELP, SSVAR\n\n";
      throw(1);
   }

   //
   // Turn off the vif_flag since it doesn't apply
   //
   job.vif_flag = 0;

   //
   // Sum up the contingency table type lines:
   //    FHO, CTC, NBRCTC
   //
   if(lt == stat_fho ||
      lt == stat_ctc ||
      lt == stat_nbrctc) {
      aggr_ctc_lines(f, job, ctc_map, n_in, n_out);
      write_job_aggr_ctc(job, lt, ctc_map, out_at);
   }

   //
   // Sum up the multi-category contingency table type lines:
   //    MCTC
   //
   else if(lt == stat_mctc) {
      aggr_mctc_lines(f, job, mctc_map, n_in, n_out);
      write_job_aggr_mctc(job, lt, mctc_map, out_at);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT
   //
   else if(lt == stat_pct) {
      aggr_pct_lines(f, job, pct_map, n_in, n_out);
      write_job_aggr_pct(job, lt, pct_map, out_at);
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
      aggr_psum_lines(f, job, psum_map, n_in, n_out);
      write_job_aggr_psum(job, lt, psum_map, out_at);
   }

   //
   // Sum the gradient line type:
   //    GRAD
   //
   else if(lt == stat_grad) {
      aggr_grad_lines(f, job, grad_map, n_in, n_out);
      write_job_aggr_grad(job, lt, grad_map, out_at);
   }

   //
   // Sum the ISC line types
   //
   else if(lt == stat_isc) {
      aggr_isc_lines(f, job, isc_map, n_in, n_out);
      write_job_aggr_isc(job, lt, isc_map, out_at);
   }

   //
   // Sum the ECNT line types
   //
   else if(lt == stat_ecnt) {
      aggr_ecnt_lines(f, job, ens_map, n_in, n_out);
      write_job_aggr_ecnt(job, lt, ens_map, out_at);
   }

   //
   // Sum the RPS line types
   //
   else if(lt == stat_rps) {
      aggr_rps_lines(f, job, rps_map, n_in, n_out);
      write_job_aggr_rps(job, lt, rps_map, out_at);
   }

   //
   // Sum the RHIST line types
   //
   else if(lt == stat_rhist) {
      aggr_rhist_lines(f, job, ens_map, n_in, n_out);
      write_job_aggr_rhist(job, lt, ens_map, out_at);
   }

   //
   // Sum the PHIST line types
   //
   else if(lt == stat_phist) {
      aggr_phist_lines(f, job, ens_map, n_in, n_out);
      write_job_aggr_phist(job, lt, ens_map, out_at);
   }

   //
   // Sum the RELP line types
   //
   else if(lt == stat_relp) {
      aggr_relp_lines(f, job, ens_map, n_in, n_out);
      write_job_aggr_relp(job, lt, ens_map, out_at);
   }

   //
   // Sum the SSVAR line types
   //
   else if(lt == stat_ssvar) {
      aggr_ssvar_lines(f, job, ssvar_map, n_in, n_out);
      write_job_aggr_ssvar(job, lt, ssvar_map, out_at);
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
                      STATAnalysisJob &job, int &n_in, int &n_out,
                      ofstream *sa_out, const ConcatString &tmp_dir,
                      gsl_rng *rng_ptr) {
   STATLine line;
   STATLineType in_lt, out_lt;
   AsciiTable out_at;
   int i, n;

   map<ConcatString, AggrCTCInfo>  ctc_map;
   map<ConcatString, AggrMCTCInfo> mctc_map;
   map<ConcatString, AggrPCTInfo>  pct_map;
   map<ConcatString, AggrPSumInfo> psum_map;
   map<ConcatString, AggrWindInfo> wind_map;
   map<ConcatString, AggrENSInfo>  orank_map;
   map<ConcatString, AggrMPRInfo>  mpr_map;

   //
   // Check that the -line_type and -out_line_type options have been
   // supplied only once
   //
   if(job.line_type.n()     != 1 ||
      job.out_line_type.n() != 1) {
      mlog << Error << "\ndo_job_aggr_stat() -> "
           << "this function may only be called when the "
           << "\"-line_type\" and \"-out_line_type\" options have been "
           << "used exactly once: " << jobstring << "\n\n";
      throw(1);
   }

   //
   // Determine the input and output line types for this job
   //
   in_lt  = string_to_statlinetype(job.line_type[0].c_str());
   out_lt = string_to_statlinetype(job.out_line_type[0].c_str());

   //
   // Valid combinations of input and output line types:
   //    -line_type FHO,   CTC,    -out_line_type CTS, ECLV
   //    -line_type MCTC,          -out_line_type MCTS
   //    -line_type SL1L2, SAL1L2, -out_line_type CNT
   //    -line_type VL1L2          -out_line_type VCNT
   //    -line_type VL1L2, VAL1L2, -out_line_type WDIR (wind direction)
   //    -line_type PCT,           -out_line_type PSTD, PJC, PRC, ECLV
   //    -line_type NBRCTC,        -out_line_type NBRCTS
   //    -line_type MPR,           -out_line_type FHO, CTC, CTS,
   //                                             MCTC, MCTS, CNT,
   //                                             SL1L2, SAL1L2,
   //                                             PCT, PSTD, PJC, PRC, ECLV,
   //                                             WDIR (wind direction)
   //    -line_type ORANK,         -out_line_type RHIST, PHIST, RELP, SSVAR
   //

   //
   // Sum up the contingency table type lines:
   //    FHO, CTC -> CTS, ECLV
   //    NBRCTC -> NBRCTS
   //
   if(((in_lt  == stat_fho ||
        in_lt  == stat_ctc) &&
       (out_lt == stat_cts ||
        out_lt == stat_eclv)) ||
      (in_lt   == stat_nbrctc &&
       out_lt  == stat_nbrcts)) {
      aggr_ctc_lines(f, job, ctc_map, n_in, n_out);
      write_job_aggr_ctc(job, out_lt, ctc_map, out_at);
   }

   //
   // Sum up the multi-category contingency table type lines:
   //    MCTC -> MCTS
   //
   else if(in_lt  == stat_mctc &&
           out_lt == stat_mcts) {
      aggr_mctc_lines(f, job, mctc_map, n_in, n_out);
      write_job_aggr_mctc(job, out_lt, mctc_map, out_at);
   }

   //
   // Sum up the Nx2 contingency table lines:
   //    PCT -> PSTD, PJC, PRC, ECLV
   //
   else if(  in_lt == stat_pct &&
           (out_lt == stat_pstd ||
            out_lt == stat_pjc  ||
            out_lt == stat_prc  ||
            out_lt == stat_eclv)) {
      aggr_pct_lines(f, job, pct_map, n_in, n_out);
      write_job_aggr_pct(job, out_lt, pct_map, out_at);
   }

   //
   // Sum the scalar partial sum line types:
   //    SL1L2, SAL1L2 -> CNT
   //    NBRCTC -> NBRCNT
   //
   else if((in_lt  == stat_sl1l2 ||
            in_lt  == stat_sal1l2) &&
            out_lt == stat_cnt) {
      aggr_psum_lines(f, job, psum_map, n_in, n_out);
      write_job_aggr_psum(job, out_lt, psum_map, out_at);
   }

   //
   // Sum the vector partial sum line types:
   //    VL1L2 -> VCNT
   //
   else if(in_lt  == stat_vl1l2 &&
           out_lt == stat_vcnt) {
      aggr_psum_lines(f, job, psum_map, n_in, n_out);
      write_job_aggr_psum(job, out_lt, psum_map, out_at);
   }

   //
   // Sum the vector partial sum line types:
   //    VL1L2, VAL1L2 -> WDIR
   //
   else if((in_lt  == stat_vl1l2 ||
            in_lt  == stat_val1l2) &&
            out_lt == stat_wdir) {
      aggr_wind_lines(f, job, wind_map, n_in, n_out);
      write_job_aggr_wind(job, in_lt, wind_map, out_at);
   }

   //
   // Sum the UGRD and VGRD matched pair lines:
   //    MPR -> WDIR
   //
   else if(in_lt  == stat_mpr &&
           (out_lt == stat_wdir ||
            out_lt == stat_vl1l2 ||
            out_lt == stat_vcnt)) {

      mlog << Debug(4) << "do_job_aggr_stat() -> "
           << "For MPR wind aggregation, searching for UGRD and VGRD MPR lines.\n";

      job.fcst_var.clear();
      job.fcst_var.add(ugrd_abbr_str);
      job.fcst_var.add(vgrd_abbr_str);

      aggr_mpr_wind_lines(f, job, wind_map, n_in, n_out);
      if(out_lt == stat_wdir) {
         write_job_aggr_wind(job, in_lt, wind_map, out_at);
      }
      else {
         write_job_aggr_mpr_wind(job, out_lt, wind_map, out_at);
      }
   }

   //
   // Sum the observation rank line types:
   //    ORANK -> ECNT, RPS, RHIST, PHIST, RELP, SSVAR
   //
   else if(in_lt == stat_orank &&
           (out_lt == stat_ecnt  || out_lt == stat_rps   ||
            out_lt == stat_rhist || out_lt == stat_phist ||
            out_lt == stat_relp  || out_lt == stat_ssvar)) {

      //
      // Check forecast thresholds for RPS
      //
      if(out_lt == stat_rps) {

         if(job.out_fcst_thresh.n() == 0) {
            mlog << Error << "\ndo_job_aggr_stat() -> "
                 << "when \"-out_line_type\" is set to RPS, the "
                 << "\"-out_fcst_thresh\" option must be used to specify "
                 << "monotonically increasing thresholds of interet.\n\n";
            throw(1);
         }
      }

      aggr_orank_lines(f, job, orank_map, n_in, n_out);
      write_job_aggr_orank(job, out_lt, orank_map, out_at, rng_ptr);
   }

   //
   // Read the matched pair lines:
   //    MPR -> FHO, CTC, CTS, MCTC, MCTS, CNT,
   //           SL1L2, SAL1L2, PCT, PSTD, PJC, PRC, ECLV
   //
   else if(in_lt == stat_mpr &&
           (out_lt == stat_fho   || out_lt == stat_ctc    ||
            out_lt == stat_cts   || out_lt == stat_mctc   ||
            out_lt == stat_mcts  || out_lt == stat_cnt    ||
            out_lt == stat_sl1l2 || out_lt == stat_sal1l2 ||
            out_lt == stat_pct   || out_lt == stat_pstd   ||
            out_lt == stat_pjc   || out_lt == stat_prc    ||
            out_lt == stat_eclv)) {

      //
      // Check output threshold values for 2x2 contingency table
      //
      if(out_lt == stat_fho ||
         out_lt == stat_ctc ||
         out_lt == stat_cts ||
         out_lt == stat_eclv) {

         if(job.out_fcst_thresh.n() != 1 ||
            job.out_obs_thresh.n()  != 1) {
            mlog << Error << "\ndo_job_aggr_stat() -> "
                 << "when \"-out_line_type\" is set to FHO, CTC, "
                 << "CTS, or ECLV, the \"-out_thresh\" option or "
                 << "\"-out_fcst_thresh\" and \"-out_obs_thresh\" "
                 << "options must specify exactly one threshold.\n\n";
            throw(1);
         }
      }

      //
      // Check output threshold values for NxN contingency table
      //
      if(out_lt == stat_mctc ||
         out_lt == stat_mcts) {

         if(job.out_fcst_thresh.n() <= 1 ||
            job.out_fcst_thresh.n() != job.out_obs_thresh.n()) {
            mlog << Error << "\ndo_job_aggr_stat() -> "
                 << "when \"-out_line_type\" is set to MCTC or MCTS "
                 << "the \"-out_thresh\" option or \"-out_fcst_thresh\" and "
                 << "\"-out_obs_thresh\" options must specify "
                 << "the same number of thresholds and at least two.\n\n";
            throw(1);
         }

         for(i=0; i<job.out_fcst_thresh.n()-1; i++) {

            if(job.out_fcst_thresh[i].get_value() >  job.out_fcst_thresh[i+1].get_value() ||
               job.out_obs_thresh[i].get_value()  >  job.out_obs_thresh[i+1].get_value()  ||
               job.out_fcst_thresh[i].get_type()  != job.out_fcst_thresh[i+1].get_type()  ||
               job.out_obs_thresh[i].get_type()   != job.out_obs_thresh[i+1].get_type()   ||
               job.out_fcst_thresh[i].get_type()  == thresh_eq                          ||
               job.out_fcst_thresh[i].get_type()  == thresh_ne                          ||
               job.out_obs_thresh[i].get_type()   == thresh_eq                          ||
               job.out_obs_thresh[i].get_type()   == thresh_ne) {

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

         if(job.out_obs_thresh.n() != 1) {
            mlog << Error << "\ndo_job_aggr_stat() -> "
                 << "when \"-out_line_type\" is set to PCT, PSTD, "
                 << "PJC, or PRC, the \"-out_obs_thresh\" option "
                 << "must be specified exactly once.\n\n";
            throw(1);
         }

         // Check for special case of a single probability threshold
         if(job.out_fcst_thresh.n() == 1) {
            job.out_fcst_thresh = string_to_prob_thresh(job.out_fcst_thresh[0].get_str().c_str());
         }

         n = job.out_fcst_thresh.n();

         // Check that the first threshold is 0 and the last is 1.
         if(n < 3 ||
            !is_eq(job.out_fcst_thresh[0].get_value(),   0.0) ||
            !is_eq(job.out_fcst_thresh[n-1].get_value(), 1.0)) {

            mlog << Error << "\ndo_job_aggr_stat() -> "
                 << "When verifying a probability field, you must "
                 << "use the \"-out_fcst_thresh\" option to select "
                 << "at least 3 probability thresholds beginning with "
                 << "0.0 and ending with 1.0.\n\n";
            throw(1);
         }

         for(i=0; i<n; i++) {

            // Check that all threshold types are >=
            if(job.out_fcst_thresh[i].get_type() != thresh_ge) {
               mlog << Error << "\ndo_job_aggr_stat() -> "
                    << "When verifying a probability field, all "
                    << "forecast probability thresholds must be set "
                    << "as greater than or equal to with \"ge\" or "
                    << "\"=\".\n\n";
               throw(1);
            }

            // Check that all thresholds are in [0, 1].
            if(job.out_fcst_thresh[i].get_value() < 0.0 ||
               job.out_fcst_thresh[i].get_value() > 1.0) {

               mlog << Error << "\ndo_job_aggr_stat_stat() -> "
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
      aggr_mpr_lines(f, job, mpr_map, n_in, n_out);
      write_job_aggr_mpr(job, out_lt, mpr_map, out_at, tmp_dir.c_str(), rng_ptr);
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
   // Write the ASCII Table and the job command line
   //
   write_jobstring(jobstring, sa_out);
   write_table(out_at, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_summary(STATAnalysisJob &job,
                       map<ConcatString, AggrSummaryInfo> &m,
                       AsciiTable &at, gsl_rng *rng_ptr) {
   map<ConcatString, AggrSummaryInfo>::iterator it;
   map<ConcatString, NumArray>::iterator val_it, wgt_it;
   int i, r, c;
   double min, v10, v25, v50, v75, v90, max, iqr, range;
   double wmo_mean, wmo_wmean;
   ConcatString wmo_method;
   StringArray sa;
   CIInfo mean_ci, stdev_ci;

   //
   // Setup the output table
   //
   for(it = m.begin(), r = 0; it != m.end(); it++) {
      r += (int) it->second.val.size();
   }
   at.set_size(r + 1,
               3 + job.by_column.n() + n_job_summary_columns);
   setup_table(at, 3 + job.by_column.n(), job.get_precision());

   //
   // Initialize
   //
   r = c = 0;

   //
   // Write header line
   //
   at.set_entry(r, c++, (string)"COL_NAME:");
   at.set_entry(0, c++, (string)"LINE_TYPE");
   at.set_entry(0, c++, (string)"COLUMN");
   for(i=0; i<job.by_column.n(); i++) {
      at.set_entry(0, c++, job.by_column[i]);
   }
   write_header_row(job_summary_columns, n_job_summary_columns,
                    0, at, r, c);

   //
   // Set up CIInfo objects
   //
   mean_ci.allocate_n_alpha(1);
   stdev_ci.allocate_n_alpha(1);

   mlog << Debug(2) << "Computing output for " << (int) m.size()
        << " case(s).\n";

   //
   // Loop over the summary table
   //
   for(it = m.begin(), r = 1; it != m.end(); it++) {

      //
      // Print info about multiple header entries
      //
      it->second.hdr.check_shc(it->first);

      //
      // Loop over the statistics for current case
      //
      for(val_it  = it->second.val.begin(), wgt_it  = it->second.wgt.begin();
          val_it != it->second.val.end() && wgt_it != it->second.wgt.end();
          val_it++, wgt_it++) {

         //
         // Skip empty rows
         //
         if(val_it->second.n() == 0) continue;

         //
         // Compute the summary information for these values:
         // min, max, v10, v25, v50, 75, v90, iqr, range
         //
         min = val_it->second.percentile_array(0.00);
         v10 = val_it->second.percentile_array(0.10);
         v25 = val_it->second.percentile_array(0.25);
         v50 = val_it->second.percentile_array(0.50);
         v75 = val_it->second.percentile_array(0.75);
         v90 = val_it->second.percentile_array(0.90);
         max = val_it->second.percentile_array(1.00);

         iqr   = (is_bad_data(v75) || is_bad_data(v25) ? bad_data_double : v75 - v25);
         range = (is_bad_data(max) || is_bad_data(min) ? bad_data_double : max - min);

         //
         // Compute a bootstrap confidence interval for the mean.
         //
         if(job.boot_interval == boot_bca_flag) {
            compute_mean_stdev_ci_bca(rng_ptr, val_it->second,
                                      job.n_boot_rep,
                                      job.out_alpha, mean_ci, stdev_ci);
         }
         else {
            compute_mean_stdev_ci_perc(rng_ptr, val_it->second,
                                       job.n_boot_rep, job.boot_rep_prop,
                                       job.out_alpha, mean_ci, stdev_ci);
         }

         //
         // Compute WMO means
         //
         if(job.wmo_fisher_stats.has(val_it->first)) {
            wmo_method = "FISHER";
            wmo_mean   = val_it->second.mean_fisher();
            wmo_wmean  = val_it->second.wmean_fisher(wgt_it->second);
         }
         else if(job.wmo_sqrt_stats.has(val_it->first)) {
            wmo_method = "SQRT";
            wmo_mean   = val_it->second.mean_sqrt();
            wmo_wmean  = val_it->second.wmean_sqrt(wgt_it->second);
         }
         else {
            wmo_method = "MEAN";
            wmo_mean   = val_it->second.mean();
            wmo_wmean  = val_it->second.wmean(wgt_it->second);
         }

         //
         // Write the data row
         //
         c = 0;
         at.set_entry(r, c++, (string)"SUMMARY:");

         //
         // Line type and column name
         //
         sa = val_it->first.split(":");
         for(i=0; i<sa.n(); i++) at.set_entry(r, c++, sa[i]);

         //
         // Case columns
         //
         sa = it->first.split(":");
         for(i=0; i<sa.n(); i++) at.set_entry(r, c++, sa[i]);

         //
         // Data columns
         //
         at.set_entry(r, c++, val_it->second.n());
         at.set_entry(r, c++, mean_ci.v);
         at.set_entry(r, c++, mean_ci.v_ncl[0]);
         at.set_entry(r, c++, mean_ci.v_ncu[0]);
         at.set_entry(r, c++, mean_ci.v_bcl[0]);
         at.set_entry(r, c++, mean_ci.v_bcu[0]);
         at.set_entry(r, c++, stdev_ci.v);
         at.set_entry(r, c++, stdev_ci.v_bcl[0]);
         at.set_entry(r, c++, stdev_ci.v_bcu[0]);
         at.set_entry(r, c++, min);
         at.set_entry(r, c++, v10);
         at.set_entry(r, c++, v25);
         at.set_entry(r, c++, v50);
         at.set_entry(r, c++, v75);
         at.set_entry(r, c++, v90);
         at.set_entry(r, c++, max);
         at.set_entry(r, c++, iqr);
         at.set_entry(r, c++, range);
         at.set_entry(r, c++, wmo_method);
         at.set_entry(r, c++, wmo_mean);
         at.set_entry(r, c++, wmo_wmean);

         //
         // Increment the row counter
         //
         r++;

      } // end for val_it, wgt_it
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_hdr(STATAnalysisJob &job, int n_row, int n_col,
                        AsciiTable &at) {
   int i, c;

   //
   // Setup the output table
   //
   at.set_size(n_row, n_col);
   setup_table(at, 1 + job.by_column.n(), job.get_precision());

   //
   // Write the header information
   //
   c = 0;
   at.set_entry(0, c++, (string)"COL_NAME:");
   for(i=0; i<job.by_column.n(); i++) {
      at.set_entry(0, c++, job.by_column[i]);
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_ctc(STATAnalysisJob &job, STATLineType lt,
                        map<ConcatString, AggrCTCInfo> &m,
                        AsciiTable &at) {
   map<ConcatString, AggrCTCInfo>::iterator it;
   int n_row, n_col, n_pnt, r, c;
   NBRCTSInfo nbrcts_info;
   StatHdrColumns shc;

   n_pnt = job.out_eclv_points.n();

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + job.by_column.n();
        if(lt == stat_fho)    n_col += n_fho_columns;
   else if(lt == stat_ctc)    n_col += n_ctc_columns;
   else if(lt == stat_cts)    n_col += n_cts_columns;
   else if(lt == stat_eclv)   n_col += get_n_eclv_columns(n_pnt);
   else if(lt == stat_nbrctc) n_col += n_nbrctc_columns;
   else if(lt == stat_nbrcts) n_col += n_nbrcts_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the output header row
   //
   c = 1 + job.by_column.n();
        if(lt == stat_fho)    write_header_row(fho_columns,    n_fho_columns,    0, at, 0, c);
   else if(lt == stat_ctc)    write_header_row(ctc_columns,    n_ctc_columns,    0, at, 0, c);
   else if(lt == stat_cts)    write_header_row(cts_columns,    n_cts_columns,    0, at, 0, c);
   else if(lt == stat_eclv)   write_eclv_header_row(        0, n_pnt,               at, 0, c);
   else if(lt == stat_nbrctc) write_header_row(nbrctc_columns, n_nbrctc_columns, 0, at, 0, c);
   else if(lt == stat_nbrcts) write_header_row(nbrcts_columns, n_nbrcts_columns, 0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, n_pnt);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         if(lt == stat_cts || lt == stat_nbrcts) shc.set_alpha(job.out_alpha);
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // FHO output line
      //
      if(lt == stat_fho) {
         at.set_entry(r, c++, (string)"FHO:");
         write_case_cols(it->first, at, r, c);
         write_fho_cols(it->second.cts_info, at, r, c);
         if(job.stat_out) write_fho_cols(it->second.cts_info, job.stat_at, r, n_header_columns);
      }
      //
      // CTC output line
      //
      else if(lt == stat_ctc) {
         at.set_entry(r, c++, (string)"CTC:");
         write_case_cols(it->first, at, r, c);
         write_ctc_cols(it->second.cts_info, at, r, c);
         if(job.stat_out) write_ctc_cols(it->second.cts_info, job.stat_at, r, n_header_columns);
      }
      //
      // CTS output line
      //
      else if(lt == stat_cts) {

         //
         // Store the alpha information in the CTSInfo object
         //
         it->second.cts_info.allocate_n_alpha(1);
         it->second.cts_info.alpha[0] = job.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // CTSInfo object
         //
         it->second.cts_info.compute_stats();
         it->second.cts_info.compute_ci();

         //
         // Write the data line
         //
         at.set_entry(r, c++, (string)"CTS:");
         write_case_cols(it->first, at, r, c);
         write_cts_cols(it->second.cts_info, 0, at, r, c);
         if(job.stat_out) write_cts_cols(it->second.cts_info, 0, job.stat_at, r, n_header_columns);
      }
      //
      // ECLV output line
      //
      else if(lt == stat_eclv) {
         at.set_entry(r, c++, (string)"ECLV:");
         write_case_cols(it->first, at, r, c);
         write_eclv_cols(it->second.cts_info.cts, job.out_eclv_points, at, r, c);
         if(job.stat_out) write_eclv_cols(it->second.cts_info.cts, job.out_eclv_points, job.stat_at, r, n_header_columns);
      }
      //
      // NBRCTC output line
      //
      else if(lt == stat_nbrctc) {

         nbrcts_info.clear();
         nbrcts_info.cts_info = it->second.cts_info;

         at.set_entry(r, c++, (string)"NBRCTC:");
         write_case_cols(it->first, at, r, c);
         write_nbrctc_cols(nbrcts_info, at, r, c);
         if(job.stat_out) write_nbrctc_cols(nbrcts_info, job.stat_at, r, n_header_columns);
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
         nbrcts_info.cts_info.alpha[0] = job.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // NBRCTSInfo object
         //
         nbrcts_info.cts_info.compute_stats();
         nbrcts_info.cts_info.compute_ci();

         //
         // Write the data line
         //
         at.set_entry(r, c++, (string)"NBRCTS:");
         write_case_cols(it->first, at, r, c);
         write_nbrcts_cols(nbrcts_info, 0, at, r, c);
         if(job.stat_out) write_nbrcts_cols(nbrcts_info, 0, job.stat_at, r, n_header_columns);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_mctc(STATAnalysisJob &job, STATLineType lt,
                         map<ConcatString, AggrMCTCInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrMCTCInfo>::iterator it;
   int n, n_row, n_col, r, c;
   StatHdrColumns shc;

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
   n_col = 1 + job.by_column.n();
        if(lt == stat_mctc) n_col += get_n_mctc_columns(n);
   else if(lt == stat_mcts) n_col += n_mcts_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
        if(lt == stat_mctc) write_mctc_header_row(0, n, at, 0, c);
   else if(lt == stat_mcts) write_header_row(mcts_columns, n_mcts_columns, 0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, n);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         if(lt == stat_mcts) shc.set_alpha(job.out_alpha);
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // MCTC output line
      //
      if(lt == stat_mctc) {
         at.set_entry(r, c++, (string)"MCTC:");
         write_case_cols(it->first, at, r, c);
         write_mctc_cols(it->second.mcts_info, at, r, c);
         if(job.stat_out) write_mctc_cols(it->second.mcts_info, job.stat_at, r, n_header_columns);
      }
      //
      // MCTS output line
      //
      else if(lt == stat_mcts) {

         //
         // Store the alpha information in the CTSInfo object
         //
         it->second.mcts_info.allocate_n_alpha(1);
         it->second.mcts_info.alpha[0] = job.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // MCTSInfo object
         //
         it->second.mcts_info.compute_stats();
         it->second.mcts_info.compute_ci();

         //
         // Write the data line
         //
         at.set_entry(r, c++, (string)"MCTS:");
         write_case_cols(it->first, at, r, c);
         write_mcts_cols(it->second.mcts_info, 0, at, r, c);
         if(job.stat_out) write_mcts_cols(it->second.mcts_info, 0, job.stat_at, r, n_header_columns);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_pct(STATAnalysisJob &job, STATLineType lt,
                        map<ConcatString, AggrPCTInfo> &m,
                        AsciiTable &at) {
   map<ConcatString, AggrPCTInfo>::iterator it;
   int n, n_row, n_col, n_pnt, r, c, i;
   StatHdrColumns shc;

   //
   // Determine the maximum PCT dimension
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.pct_info.pct.nrows() + 1, n);
   }

   n_pnt = job.out_eclv_points.n();

   //
   // Setup the output table
   //
   if(lt == stat_eclv) n_row = 1 + m.size() * n;
   else                n_row = 1 + m.size();
   n_col = 1 + job.by_column.n();
        if(lt == stat_pct)  n_col += get_n_pct_columns(n);
   else if(lt == stat_pstd) n_col += get_n_pstd_columns(n);
   else if(lt == stat_pjc)  n_col += get_n_pjc_columns(n);
   else if(lt == stat_prc)  n_col += get_n_prc_columns(n);
   else if(lt == stat_eclv) n_col += get_n_eclv_columns(n_pnt);
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
        if(lt == stat_pct)  write_pct_header_row (0, n, at, 0, c);
   else if(lt == stat_pstd) write_pstd_header_row(0, n, at, 0, c);
   else if(lt == stat_pjc)  write_pjc_header_row (0, n, at, 0, c);
   else if(lt == stat_prc)  write_prc_header_row (0, n, at, 0, c);
   else if(lt == stat_eclv) write_eclv_header_row(0, n_pnt, at, 0, c);

   //
   // Setup the output STAT file
   //
   if(lt == stat_eclv) job.setup_stat_file(n_row, n_pnt);
   else                job.setup_stat_file(n_row, n);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         if(lt == stat_pstd) shc.set_alpha(job.out_alpha);
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // PCT output line
      //
      if(lt == stat_pct) {
         at.set_entry(r, c++, (string)"PCT:");
         write_case_cols(it->first, at, r, c);
         write_pct_cols(it->second.pct_info, at, r, c);
         if(job.stat_out) write_pct_cols(it->second.pct_info, job.stat_at, r, n_header_columns);
      }
      //
      // PSTD output line
      //
      else if(lt == stat_pstd) {

         //
         // Store the alpha information in the PCTInfo object
         //
         it->second.pct_info.allocate_n_alpha(1);
         it->second.pct_info.alpha[0] = job.out_alpha;

         //
         // Compute the stats and confidence intervals for this
         // PCTInfo object
         //
         it->second.pct_info.compute_stats();
         it->second.pct_info.compute_ci();

         //
         // Write the data line
         //
         at.set_entry(r, c++, (string)"PSTD:");
         write_case_cols(it->first, at, r, c);
         write_pstd_cols(it->second.pct_info, 0, at, r, c);
         if(job.stat_out) write_pstd_cols(it->second.pct_info, 0, job.stat_at, r, n_header_columns);
      }
      //
      // PJC output line
      //
      else if(lt == stat_pjc) {
         at.set_entry(r, c++, (string)"PJC:");
         write_case_cols(it->first, at, r, c);
         write_pjc_cols(it->second.pct_info, at, r, c);
         if(job.stat_out) write_pjc_cols(it->second.pct_info, job.stat_at, r, n_header_columns);
      }
      //
      // PRC output line
      //
      else if(lt == stat_prc) {
         at.set_entry(r, c++, (string)"PRC:");
         write_case_cols(it->first, at, r, c);
         write_prc_cols(it->second.pct_info, at, r, c);
         if(job.stat_out) write_prc_cols(it->second.pct_info, job.stat_at, r, n_header_columns);
      }
      //
      // ECLV output lines
      //
      else if(lt == stat_eclv) {
         ThreshArray prob_ta = string_to_prob_thresh(shc.get_fcst_thresh_str().c_str());
         for(i=0; i<it->second.pct_info.pct.nrows(); i++, r++) {
            c = 0;
            at.set_entry(r, c++, (string)"ECLV:");
            write_case_cols(it->first, at, r, c);
            write_eclv_cols(it->second.pct_info.pct.ctc_by_row(i),
                            job.out_eclv_points, at, r, c);
            if(job.stat_out) {
               shc.set_fcst_thresh(prob_ta[i]);
               write_header_cols(shc, job.stat_at, r);
               write_eclv_cols(it->second.pct_info.pct.ctc_by_row(i),
                               job.out_eclv_points, job.stat_at, r, n_header_columns);
            }
         }
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_psum(STATAnalysisJob &job, STATLineType lt,
                         map<ConcatString, AggrPSumInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrPSumInfo>::iterator it;
   int n_row, n_col, r, c;
   StatHdrColumns shc;

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + job.by_column.n();
        if(lt == stat_sl1l2)  n_col += n_sl1l2_columns;
   else if(lt == stat_sal1l2) n_col += n_sal1l2_columns;
   else if(lt == stat_vl1l2)  n_col += n_vl1l2_columns;
   else if(lt == stat_val1l2) n_col += n_val1l2_columns;
   else if(lt == stat_cnt)    n_col += n_cnt_columns;
   else if(lt == stat_vcnt)   n_col += n_vcnt_columns;
   else if(lt == stat_nbrcnt) n_col += n_nbrcnt_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
        if(lt == stat_sl1l2)  write_header_row(sl1l2_columns,  n_sl1l2_columns,  0, at, 0, c);
   else if(lt == stat_sal1l2) write_header_row(sal1l2_columns, n_sal1l2_columns, 0, at, 0, c);
   else if(lt == stat_vl1l2)  write_header_row(vl1l2_columns,  n_vl1l2_columns,  0, at, 0, c);
   else if(lt == stat_val1l2) write_header_row(val1l2_columns, n_val1l2_columns, 0, at, 0, c);
   else if(lt == stat_cnt)    write_header_row(cnt_columns,    n_cnt_columns,    0, at, 0, c);
   else if(lt == stat_vcnt)   write_header_row(vcnt_columns,   n_vcnt_columns,   0, at, 0, c);
   else if(lt == stat_nbrcnt) write_header_row(nbrcnt_columns, n_nbrcnt_columns, 0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         if(lt == stat_cnt || lt == stat_nbrcnt) shc.set_alpha(job.out_alpha);
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // SL1L2 output line
      //
      if(lt == stat_sl1l2) {
         at.set_entry(r, c++, (string)"SL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sl1l2_cols(it->second.sl1l2_info, at, r, c);
         if(job.stat_out) write_sl1l2_cols(it->second.sl1l2_info, job.stat_at, r, n_header_columns);
      }
      //
      // SAL1L2 output line
      //
      else if(lt == stat_sal1l2) {
         at.set_entry(r, c++, (string)"SAL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sal1l2_cols(it->second.sl1l2_info, at, r, c);
         if(job.stat_out) write_sal1l2_cols(it->second.sl1l2_info, job.stat_at, r, n_header_columns);
      }
      //
      // VL1L2 output line
      //
      else if(lt == stat_vl1l2) {
         at.set_entry(r, c++, (string)"VL1L2:");
         write_case_cols(it->first, at, r, c);
         write_vl1l2_cols(it->second.vl1l2_info, at, r, c);
         if(job.stat_out) write_vl1l2_cols(it->second.vl1l2_info, job.stat_at, r, n_header_columns);
      }
      //
      // VAL1L2 output line
      //
      else if(lt == stat_val1l2) {
         at.set_entry(r, c++, (string)"VAL1L2:");
         write_case_cols(it->first, at, r, c);
         write_val1l2_cols(it->second.vl1l2_info, at, r, c);
         if(job.stat_out) write_val1l2_cols(it->second.vl1l2_info, job.stat_at, r, n_header_columns);
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
         it->second.cnt_info.alpha[0] = job.out_alpha;

         //
         // Compute CNTInfo statistics from the aggregated partial sums
         //
         if(it->second.sl1l2_info.scount > 0)
            compute_cntinfo(it->second.sl1l2_info, 0, it->second.cnt_info);
         else
            compute_cntinfo(it->second.sl1l2_info, 1, it->second.cnt_info);

         at.set_entry(r, c++, (string)"CNT:");
         write_case_cols(it->first, at, r, c);
         write_cnt_cols(it->second.cnt_info, 0, at, r, c);
         if(job.stat_out) write_cnt_cols(it->second.cnt_info, 0, job.stat_at, r, n_header_columns);
      }
      //
      // VCNT output line
      //
      else if(lt == stat_vcnt) {
         at.set_entry(r, c++, (string)"VCNT:");
         write_case_cols(it->first, at, r, c);
         write_vcnt_cols(it->second.vl1l2_info, at, r, c);
         if(job.stat_out) write_vcnt_cols(it->second.vl1l2_info, job.stat_at, r, n_header_columns);
      }
      //
      // NBRCNT output line
      //
      else if(lt == stat_nbrcnt) {

         //
         // Allocate space for confidence intervals
         //
         it->second.nbrcnt_info.allocate_n_alpha(1);

         at.set_entry(r, c++, "NBRCNT:");
         write_case_cols(it->first, at, r, c);
         write_nbrcnt_cols(it->second.nbrcnt_info, 0, at, r, c);
         if(job.stat_out) write_nbrcnt_cols(it->second.nbrcnt_info, 0, job.stat_at, r, n_header_columns);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_grad(STATAnalysisJob &job, STATLineType lt,
                         map<ConcatString, AggrGRADInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrGRADInfo>::iterator it;
   int n_row, n_col, r, c;
   StatHdrColumns shc;

   //
   // Setup the output table
   //
   n_row  = 1 + m.size();
   n_col  = 1 + job.by_column.n();
   n_col += n_grad_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_header_row(grad_columns,  n_grad_columns,  0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) write_header_cols(shc, job.stat_at, r);

      //
      // Initialize
      //
      c = 0;

      //
      // GRAD output line
      //
      at.set_entry(r, c++, "GRAD:");
      write_case_cols(it->first, at, r, c);
      write_grad_cols(it->second.grad_info, at, r, c);
      if(job.stat_out) write_grad_cols(it->second.grad_info, job.stat_at, r, n_header_columns);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_wind(STATAnalysisJob &job, STATLineType lt,
                         map<ConcatString, AggrWindInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrWindInfo>::iterator it;
   int i, n, n_row, n_col, r, c;
   int count = 0;
   double uf, vf, uo, vo, fbar, obar;
   double angle, me, mae;

   //
   // Setup the output table
   //
   n_row = 1 + (2 * m.size());
   n_col = 1 + job.by_column.n() + n_job_wdir_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_header_row(job_wdir_columns, n_job_wdir_columns, 0, at, 0, c);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Check for matching component lengths
      //
      if(it->second.pd_u.f_na.n() != it->second.pd_v.f_na.n() ||
         it->second.pd_u.o_na.n() != it->second.pd_v.o_na.n() ||
         it->second.pd_u.f_na.n() != it->second.pd_u.o_na.n()) {
         mlog << Error << "\nwrite_job_aggr_wind() -> "
              << "the number of U and V forecast and observation points "
              << "must be the same.\n\n";
         throw(1);
      }

      //
      // Store the number of vectors
      //
      n = it->second.pd_u.f_na.n();

      //
      // Compute the mean forecast and observation angles
      // from the unit vectors
      //
      if(n > 0) {
         uf   = it->second.pd_u.f_na.sum()/it->second.pd_u.f_na.n();
         vf   = it->second.pd_v.f_na.sum()/it->second.pd_v.f_na.n();
         fbar = convert_u_v_to_wdir(uf, vf);
         uo   = it->second.pd_u.o_na.sum()/it->second.pd_u.o_na.n();
         vo   = it->second.pd_v.o_na.sum()/it->second.pd_v.o_na.n();
         obar = convert_u_v_to_wdir(uo, vo);
      }
      else {
         uf = vf = fbar = uo = vo = obar = bad_data_double;
      }

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
      me = mae = (n > 0 ? 0.0 : bad_data_double);
      for(i=0; i<n; i++) {

         angle = angle_difference(it->second.pd_u.f_na[i], it->second.pd_v.f_na[i],
                                  it->second.pd_u.o_na[i], it->second.pd_v.o_na[i]);

         if(mlog.verbosity_level() > 3) {
            mlog << Debug(4) << "write_job_aggr_wind() -> "
                 << "ROW_MEAN_WDIR: [" << i+1 << "] difference of forecast direction "
                 << convert_u_v_to_wdir(it->second.pd_u.f_na[i], it->second.pd_v.f_na[i])
                 << " - observed direction "
                 << convert_u_v_to_wdir(it->second.pd_u.o_na[i], it->second.pd_v.o_na[i])
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
      c = 0;
      at.set_entry(r, c++, "ROW_MEAN_WDIR:");
      write_case_cols(it->first, at, r, c);
      at.set_entry(r, c++, n);
      at.set_entry(r, c++, fbar);
      at.set_entry(r, c++, obar);
      at.set_entry(r, c++, me);
      at.set_entry(r, c++, mae);

      //
      // Increment row counter
      //
      r++;

      if(lt == stat_vl1l2 || lt == stat_mpr) {
         uf    = it->second.vl1l2_info.uf_bar;
         vf    = it->second.vl1l2_info.vf_bar;
         uo    = it->second.vl1l2_info.uo_bar;
         vo    = it->second.vl1l2_info.vo_bar;
         count = it->second.vl1l2_info.vcount;
      }
      else if(lt == stat_val1l2) {
         uf    = it->second.vl1l2_info.ufa_bar;
         vf    = it->second.vl1l2_info.vfa_bar;
         uo    = it->second.vl1l2_info.uoa_bar;
         vo    = it->second.vl1l2_info.voa_bar;
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
      c = 0;
      at.set_entry(r, c++, "AGGR_WDIR:");
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

void write_job_aggr_ecnt(STATAnalysisJob &job, STATLineType lt,
                          map<ConcatString, AggrENSInfo> &m,
                          AsciiTable &at) {
   map<ConcatString, AggrENSInfo>::iterator it;
   int n_row, n_col, r, c;
   StatHdrColumns shc;
   ECNTInfo ecnt_info;

   //
   // Setup the output table
   //
   n_row  = 1 + m.size();
   n_col  = 1 + job.by_column.n();
   n_col += n_ecnt_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_header_row(ecnt_columns,  n_ecnt_columns,  0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) write_header_cols(shc, job.stat_at, r);

      //
      // Initialize
      //
      c = 0;

      //
      // Compute ECNT stats
      //
      ecnt_info.set(it->second.ens_pd);

      //
      // ECNT output line
      //
      at.set_entry(r, c++, "ECNT:");
      write_case_cols(it->first, at, r, c);
      write_ecnt_cols(ecnt_info, at, r, c);
      if(job.stat_out) write_ecnt_cols(ecnt_info, job.stat_at, r, n_header_columns);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_rps(STATAnalysisJob &job, STATLineType lt,
                        map<ConcatString, AggrRPSInfo> &m,
                        AsciiTable &at) {
   map<ConcatString, AggrRPSInfo>::iterator it;
   int n_row, n_col, r, c;
   StatHdrColumns shc;
   RPSInfo rps_info;

   //
   // Setup the output table
   //
   n_row  = 1 + m.size();
   n_col  = 1 + job.by_column.n();
   n_col += n_rps_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_header_row(rps_columns,  n_rps_columns,  0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) write_header_cols(shc, job.stat_at, r);

      //
      // Initialize
      //
      c = 0;

      //
      // RPS output line
      //
      at.set_entry(r, c++, "RPS:");
      write_case_cols(it->first, at, r, c);
      write_rps_cols(it->second.rps_info, at, r, c);
      if(job.stat_out) write_rps_cols(it->second.rps_info, job.stat_at, r, n_header_columns);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_rhist(STATAnalysisJob &job, STATLineType lt,
                          map<ConcatString, AggrENSInfo> &m,
                          AsciiTable &at) {
   map<ConcatString, AggrENSInfo>::iterator it;
   int n, n_row, n_col, r, c;
   StatHdrColumns shc;

   //
   // Determine the maximum number of ranks
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.ens_pd.rhist_na.n(), n);
   }

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + job.by_column.n() + get_n_rhist_columns(n);
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_rhist_header_row(0, n, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, n);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         write_header_cols(shc, job.stat_at, r);
      }

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
      if(job.stat_out) write_rhist_cols(&(it->second.ens_pd), job.stat_at, r, n_header_columns);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_phist(STATAnalysisJob &job, STATLineType lt,
                           map<ConcatString, AggrENSInfo> &m,
                          AsciiTable &at) {
   map<ConcatString, AggrENSInfo>::iterator it;
   int n, n_row, n_col, r, c;
   StatHdrColumns shc;

   //
   // Determine the maximum number of bins
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.ens_pd.phist_na.n(), n);
   }

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + job.by_column.n() + get_n_phist_columns(n);
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_phist_header_row(0, n, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, n);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // PHIST output line
      //
      at.set_entry(r, c++, "PHIST:");
      write_case_cols(it->first, at, r, c);
      write_phist_cols(&(it->second.ens_pd), at, r, c);
      if(job.stat_out) write_phist_cols(&(it->second.ens_pd), job.stat_at, r, n_header_columns);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_relp(STATAnalysisJob &job, STATLineType lt,
                         map<ConcatString, AggrENSInfo> &m,
                         AsciiTable &at) {
   map<ConcatString, AggrENSInfo>::iterator it;
   int n, n_row, n_col, r, c;
   StatHdrColumns shc;

   //
   // Determine the maximum number of RELP values
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
      n = max(it->second.ens_pd.relp_na.n(), n);
   }

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + job.by_column.n() + get_n_relp_columns(n);
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_relp_header_row(0, n, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, n);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // RELP output line
      //
      at.set_entry(r, c++, "RELP:");
      write_case_cols(it->first, at, r, c);
      write_relp_cols(&(it->second.ens_pd), at, r, c);
      if(job.stat_out) write_relp_cols(&(it->second.ens_pd), job.stat_at, r, n_header_columns);
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_ssvar(STATAnalysisJob &job, STATLineType lt,
                          map<ConcatString, AggrSSVARInfo> &m,
                          AsciiTable &at) {
   map<ConcatString, AggrSSVARInfo>::iterator case_it;
   map<ConcatString, SSVARInfo>::iterator bin_it;
   int i, n, n_row, n_col, r, c;
   CNTInfo cnt_info;
   StatHdrColumns shc;

   //
   // Allocate space for confidence intervals and derive continuous
   // statistics
   //
   cnt_info.allocate_n_alpha(1);
   cnt_info.alpha[0] = job.out_alpha;

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
   n_col = 1 + job.by_column.n() + n_ssvar_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_header_row(ssvar_columns, n_ssvar_columns, 0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the case map
   //
   for(case_it = m.begin(), r=1; case_it != m.end(); case_it++) {

      //
      // Write the output STAT header columns
      //
      shc = case_it->second.hdr.get_shc(case_it->first, job.by_column,
                                        job.hdr_name, job.hdr_value, lt);

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
         // Compute CNTInfo statistics from the aggregated partial sums
         //
         compute_cntinfo(bin_it->second.sl1l2_info, 0, cnt_info);

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
         at.set_entry(r, c++, bin_it->second.sl1l2_info.fbar);
         at.set_entry(r, c++, bin_it->second.sl1l2_info.obar);
         at.set_entry(r, c++, bin_it->second.sl1l2_info.fobar);
         at.set_entry(r, c++, bin_it->second.sl1l2_info.ffbar);
         at.set_entry(r, c++, bin_it->second.sl1l2_info.oobar);
         at.set_entry(r, c++, cnt_info.fbar.v_ncl[0]);
         at.set_entry(r, c++, cnt_info.fbar.v_ncu[0]);
         at.set_entry(r, c++, cnt_info.fstdev.v);
         at.set_entry(r, c++, cnt_info.fstdev.v_ncl[0]);
         at.set_entry(r, c++, cnt_info.fstdev.v_ncu[0]);
         at.set_entry(r, c++, cnt_info.obar.v_ncl[0]);
         at.set_entry(r, c++, cnt_info.obar.v_ncu[0]);
         at.set_entry(r, c++, cnt_info.ostdev.v);
         at.set_entry(r, c++, cnt_info.ostdev.v_ncl[0]);
         at.set_entry(r, c++, cnt_info.ostdev.v_ncu[0]);
         at.set_entry(r, c++, cnt_info.pr_corr.v);
         at.set_entry(r, c++, cnt_info.pr_corr.v_ncl[0]);
         at.set_entry(r, c++, cnt_info.pr_corr.v_ncu[0]);
         at.set_entry(r, c++, cnt_info.me.v);
         at.set_entry(r, c++, cnt_info.me.v_ncl[0]);
         at.set_entry(r, c++, cnt_info.me.v_ncu[0]);
         at.set_entry(r, c++, cnt_info.estdev.v);
         at.set_entry(r, c++, cnt_info.estdev.v_ncl[0]);
         at.set_entry(r, c++, cnt_info.estdev.v_ncu[0]);
         at.set_entry(r, c++, cnt_info.mbias.v);
         at.set_entry(r, c++, cnt_info.mse.v);
         at.set_entry(r, c++, cnt_info.bcmse.v);
         at.set_entry(r, c++, cnt_info.rmse.v);

         //
         // Write the output STAT line
         //
         if(job.stat_out) {
            c = n_header_columns;
            write_header_cols(shc, job.stat_at, r);
            job.stat_at.set_entry(r, c++, n);
            job.stat_at.set_entry(r, c++, (int) case_it->second.ssvar_bins.size());
            job.stat_at.set_entry(r, c++, i);
            job.stat_at.set_entry(r, c++, bin_it->second.bin_n);
            job.stat_at.set_entry(r, c++, bin_it->second.var_min);
            job.stat_at.set_entry(r, c++, bin_it->second.var_max);
            job.stat_at.set_entry(r, c++, bin_it->second.var_mean);
            job.stat_at.set_entry(r, c++, bin_it->second.sl1l2_info.fbar);
            job.stat_at.set_entry(r, c++, bin_it->second.sl1l2_info.obar);
            job.stat_at.set_entry(r, c++, bin_it->second.sl1l2_info.fobar);
            job.stat_at.set_entry(r, c++, bin_it->second.sl1l2_info.ffbar);
            job.stat_at.set_entry(r, c++, bin_it->second.sl1l2_info.oobar);
            job.stat_at.set_entry(r, c++, cnt_info.fbar.v_ncl[0]);
            job.stat_at.set_entry(r, c++, cnt_info.fbar.v_ncu[0]);
            job.stat_at.set_entry(r, c++, cnt_info.fstdev.v);
            job.stat_at.set_entry(r, c++, cnt_info.fstdev.v_ncl[0]);
            job.stat_at.set_entry(r, c++, cnt_info.fstdev.v_ncu[0]);
            job.stat_at.set_entry(r, c++, cnt_info.obar.v_ncl[0]);
            job.stat_at.set_entry(r, c++, cnt_info.obar.v_ncu[0]);
            job.stat_at.set_entry(r, c++, cnt_info.ostdev.v);
            job.stat_at.set_entry(r, c++, cnt_info.ostdev.v_ncl[0]);
            job.stat_at.set_entry(r, c++, cnt_info.ostdev.v_ncu[0]);
            job.stat_at.set_entry(r, c++, cnt_info.pr_corr.v);
            job.stat_at.set_entry(r, c++, cnt_info.pr_corr.v_ncl[0]);
            job.stat_at.set_entry(r, c++, cnt_info.pr_corr.v_ncu[0]);
            job.stat_at.set_entry(r, c++, cnt_info.me.v);
            job.stat_at.set_entry(r, c++, cnt_info.me.v_ncl[0]);
            job.stat_at.set_entry(r, c++, cnt_info.me.v_ncu[0]);
            job.stat_at.set_entry(r, c++, cnt_info.estdev.v);
            job.stat_at.set_entry(r, c++, cnt_info.estdev.v_ncl[0]);
            job.stat_at.set_entry(r, c++, cnt_info.estdev.v_ncu[0]);
            job.stat_at.set_entry(r, c++, cnt_info.mbias.v);
            job.stat_at.set_entry(r, c++, cnt_info.mse.v);
            job.stat_at.set_entry(r, c++, cnt_info.bcmse.v);
            job.stat_at.set_entry(r, c++, cnt_info.rmse.v);
         }
      } // end for bin_it
   } // end for case_it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_orank(STATAnalysisJob &job, STATLineType lt,
                          map<ConcatString, AggrENSInfo> &m,
                          AsciiTable &at, gsl_rng *rng_ptr) {
   map<ConcatString, AggrENSInfo>::iterator it;
   int i, n, n_row, n_col, r, c;
   StatHdrColumns shc;

   //
   // Determine the maximum number of:
   // - Ranks for RHIST and PHIST
   // - SSVAR bins
   // - Ensemble members for RELP
   //
   for(it = m.begin(), n = 0; it != m.end(); it++) {
           if(lt == stat_rhist) n  = max(it->second.ens_pd.rhist_na.n(), n);
      else if(lt == stat_phist) n  = max(it->second.ens_pd.phist_na.n(), n);
      else if(lt == stat_relp)  n  = max(it->second.ens_pd.n_ens, n);
      else if(lt == stat_ssvar) {
         it->second.ens_pd.compute_ssvar();
         if(it->second.ens_pd.ssvar_bins) n += it->second.ens_pd.ssvar_bins[0].n_bin;
      }
   }

   //
   // Setup the output table
   //
   n_row = 0;
   n_col = 1 + job.by_column.n();
   if(lt == stat_ecnt) {
      n_row  = 1 + m.size();
      n_col += n_ecnt_columns;
   }
   if(lt == stat_rps) {
      n_row  = 1 + m.size();
      n_col += n_rps_columns;
   }
   else if(lt == stat_rhist) {
      n_row  = 1 + m.size();
      n_col += get_n_rhist_columns(n);
   }
   else if(lt == stat_phist) {
      n_row  = 1 + m.size();
      n_col += get_n_phist_columns(n);
   }
   else if(lt == stat_relp) {
      n_row  = 1 + m.size();
      n_col += get_n_relp_columns(n);
   }
   else if(lt == stat_ssvar) {
      n_row  = 1 + n;
      n_col += n_ssvar_columns;
   }
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
        if(lt == stat_ecnt)  write_header_row(ecnt_columns, n_ecnt_columns, 0, at, 0, c);
   else if(lt == stat_rps)   write_header_row(rps_columns, n_rps_columns, 0, at, 0, c);
   else if(lt == stat_rhist) write_rhist_header_row(0, n, at, 0, c);
   else if(lt == stat_phist) write_phist_header_row(0, n, at, 0, c);
   else if(lt == stat_relp)  write_relp_header_row (0, n, at, 0, c);
   else if(lt == stat_ssvar) write_header_row(ssvar_columns, n_ssvar_columns, 0, at, 0, c);

   //
   // Setup the output STAT file
   //
        if(lt == stat_ecnt)  job.setup_stat_file(n_row, 0);
   else if(lt == stat_rps)   job.setup_stat_file(n_row, 0);
   else if(lt == stat_rhist) job.setup_stat_file(n_row, n);
   else if(lt == stat_phist) job.setup_stat_file(n_row, n);
   else if(lt == stat_relp)  job.setup_stat_file(n_row, n);
   else if(lt == stat_ssvar) job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);

      //
      // Set SSVAR alpha value
      //
      if(lt == stat_ssvar) shc.set_alpha(job.out_alpha);

      //
      // Initialize
      //
      c = 0;

      //
      // ECNT output line
      //
      if(lt == stat_ecnt) {
         ECNTInfo ecnt_info;
         ecnt_info.set(it->second.ens_pd);
         at.set_entry(r, c++, "ECNT:");
         write_case_cols(it->first, at, r, c);
         write_ecnt_cols(ecnt_info, at, r, c);
         if(job.stat_out) {
            write_header_cols(shc, job.stat_at, r);
            write_ecnt_cols(ecnt_info, job.stat_at, r, n_header_columns);
         }
         // Increment row counter
         r++;
      }

      //
      // RPS output line
      //

      else if(lt == stat_rps) {
         RPSInfo rps_info;
         rps_info.fthresh = job.out_fcst_thresh;
         rps_info.set(it->second.ens_pd);
         at.set_entry(r, c++, "RPS:");
         write_case_cols(it->first, at, r, c);
         write_rps_cols(rps_info, at, r, c);
         if(job.stat_out) {
            shc.set_fcst_thresh(job.out_fcst_thresh);
            write_header_cols(shc, job.stat_at, r);
            write_rps_cols(rps_info, job.stat_at, r, n_header_columns);
         }
         // Increment row counter
         r++;
      }

      //
      // RHIST output line
      //
      else if(lt == stat_rhist) {
         it->second.ens_pd.compute_rhist();
         at.set_entry(r, c++, "RHIST:");
         write_case_cols(it->first, at, r, c);
         write_rhist_cols(&(it->second.ens_pd), at, r, c);
         if(job.stat_out) {
            write_header_cols(shc, job.stat_at, r);
            write_rhist_cols(&(it->second.ens_pd), job.stat_at, r, n_header_columns);
         }
         // Increment row counter
         r++;
      }

      //
      // PHIST output line
      //
      else if(lt == stat_phist) {
         at.set_entry(r, c++, "PHIST:");
         write_case_cols(it->first, at, r, c);
         write_phist_cols(&(it->second.ens_pd), at, r, c);
         if(job.stat_out) {
            write_header_cols(shc, job.stat_at, r);
            write_phist_cols(&(it->second.ens_pd), job.stat_at, r, n_header_columns);
         }
         // Increment row counter
         r++;
      }

      //
      // RELP output line
      //
      else if(lt == stat_relp) {
         it->second.ens_pd.compute_relp();
         at.set_entry(r, c++, "RELP:");
         write_case_cols(it->first, at, r, c);
         write_relp_cols(&(it->second.ens_pd), at, r, c);
         if(job.stat_out) {
            write_header_cols(shc, job.stat_at, r);
            write_relp_cols(&(it->second.ens_pd), job.stat_at, r, n_header_columns);
         }
         // Increment row counter
         r++;
      }

      //
      // SSVAR output lines
      //
      else if(lt == stat_ssvar) {

         if(!it->second.ens_pd.ssvar_bins) continue;

         //
         // Write a line for each ssvar bin
         //
         for(i=0; i<it->second.ens_pd.ssvar_bins[0].n_bin; i++) {
            c = 0;
            at.set_entry(r, c++, "SSVAR:");
            write_case_cols(it->first, at, r, c);
            write_ssvar_cols(&(it->second.ens_pd), i, job.out_alpha, at, r, c);
            if(job.stat_out) {
               write_header_cols(shc, job.stat_at, r);
               write_ssvar_cols(&(it->second.ens_pd), i, job.out_alpha, job.stat_at, r, n_header_columns);
            }
            // Increment row counter
            r++;
         }
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_isc(STATAnalysisJob &job, STATLineType lt,
                        map<ConcatString, AggrISCInfo> &m,
                        AsciiTable &at) {
   map<ConcatString, AggrISCInfo>::iterator it;
   int i, n, n_row, n_col, r, c;
   StatHdrColumns shc;

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
   n_col = 1 + job.by_column.n() + n_isc_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
   write_header_row(isc_columns, n_isc_columns, 0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++) {

      //
      // Format the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);

      //
      // ISC output line
      //
      for(i=-1, c=0; i<=it->second.isc_info.n_scale; i++, r++, c=0) {
         at.set_entry(r, c++, "ISC:");
         write_case_cols(it->first, at, r, c);
         write_isc_cols(it->second.isc_info, i, at, r, c);
         if(job.stat_out) {
            write_header_cols(shc, job.stat_at, r);
            write_isc_cols(it->second.isc_info, i, job.stat_at, r, n_header_columns);
         }
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_mpr(STATAnalysisJob &job, STATLineType lt,
                        map<ConcatString, AggrMPRInfo> &m,
                        AsciiTable &at, const char *tmp_dir,
                        gsl_rng *rng_ptr) {
   map<ConcatString, AggrMPRInfo>::iterator it;
   int n, n_row, n_col, r, c;
   StatHdrColumns shc;

   CTSInfo   cts_info;
   MCTSInfo  mcts_info;
   CNTInfo   cnt_info;
   SL1L2Info sl1l2_info;
   PCTInfo   pct_info;

   //
   // Setup the output table
   //
   n     = 0;
   n_row = 1 + m.size();
   n_col = 1 + job.by_column.n();
        if(lt == stat_fho)    { n_col += n_fho_columns; }
   else if(lt == stat_ctc)    { n_col += n_ctc_columns; }
   else if(lt == stat_cts)    { n_col += n_cts_columns; }
   else if(lt == stat_eclv)   { n      = job.out_eclv_points.n();
                                n_col += get_n_eclv_columns(n); }
   else if(lt == stat_mctc)   { n      = job.out_fcst_thresh.n()+1;
                                n_col += get_n_mctc_columns(n); }
   else if(lt == stat_mcts)   { n_col += n_mcts_columns; }
   else if(lt == stat_cnt)    { n_col += n_cnt_columns; }
   else if(lt == stat_sl1l2)  { n_col += n_sl1l2_columns; }
   else if(lt == stat_sal1l2) { n_col += n_sal1l2_columns; }
   else if(lt == stat_pct)    { n      = job.out_fcst_thresh.n();
                                n_col += get_n_pct_columns(n); }
   else if(lt == stat_pstd)   { n      = job.out_fcst_thresh.n();
                                n_col += get_n_pstd_columns(n); }
   else if(lt == stat_pjc)    { n      = job.out_fcst_thresh.n();
                                n_col += get_n_pjc_columns(n); }
   else if(lt == stat_prc)    { n      = job.out_fcst_thresh.n();
                                n_col += get_n_prc_columns(n); }
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
        if(lt == stat_fho)    write_header_row(fho_columns,    n_fho_columns,    0, at, 0, c);
   else if(lt == stat_ctc)    write_header_row(ctc_columns,    n_ctc_columns,    0, at, 0, c);
   else if(lt == stat_cts)    write_header_row(cts_columns,    n_cts_columns,    0, at, 0, c);
   else if(lt == stat_eclv)   write_eclv_header_row(0, n, at, 0, c);
   else if(lt == stat_mctc)   write_mctc_header_row(0, n, at, 0, c);
   else if(lt == stat_mcts)   write_header_row(mcts_columns,   n_mcts_columns,   0, at, 0, c);
   else if(lt == stat_cnt)    write_header_row(cnt_columns,    n_cnt_columns,    0, at, 0, c);
   else if(lt == stat_sl1l2)  write_header_row(sl1l2_columns,  n_sl1l2_columns,  0, at, 0, c);
   else if(lt == stat_sal1l2) write_header_row(sal1l2_columns, n_sal1l2_columns, 0, at, 0, c);
   else if(lt == stat_pct)    write_pct_header_row (0, n, at, 0, c);
   else if(lt == stat_pstd)   write_pstd_header_row(0, n, at, 0, c);
   else if(lt == stat_pjc)    write_pjc_header_row (0, n, at, 0, c);
   else if(lt == stat_prc)    write_prc_header_row (0, n, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, n);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Process percentile thresholds.
      //
      job.set_perc_thresh(it->second.pd.f_na, it->second.pd.o_na, it->second.pd.cmn_na);

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         if(lt == stat_cts || lt == stat_nbrcts || lt == stat_mcts ||
            lt == stat_cnt || lt == stat_nbrcnt || lt == stat_pstd) {
            shc.set_alpha(job.out_alpha);
         }
         if(job.out_fcst_thresh.n() > 0 || job.out_obs_thresh.n() > 0) {
            shc.set_fcst_thresh(job.out_fcst_thresh);
            shc.set_obs_thresh(job.out_obs_thresh);
            if(lt == stat_cnt || lt == stat_sl1l2) {
               shc.set_thresh_logic(job.out_cnt_logic);
            }
         }
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // FHO output line
      //
      if(lt == stat_fho) {
         mpr_to_ctc(job, it->second, cts_info);
         at.set_entry(r, c++, "FHO:");
         write_case_cols(it->first, at, r, c);
         write_fho_cols(cts_info, at, r, c);
         if(job.stat_out) write_fho_cols(cts_info, job.stat_at, r, n_header_columns);
      }
      //
      // CTC output line
      //
      else if(lt == stat_ctc) {
         mpr_to_ctc(job, it->second, cts_info);
         at.set_entry(r, c++, "CTC:");
         write_case_cols(it->first, at, r, c);
         write_ctc_cols(cts_info, at, r, c);
         if(job.stat_out) write_ctc_cols(cts_info, job.stat_at, r, n_header_columns);
      }
      //
      // CTS output line
      //
      else if(lt == stat_cts) {
         mpr_to_cts(job, it->second, cts_info, tmp_dir, rng_ptr);
         at.set_entry(r, c++, "CTS:");
         write_case_cols(it->first, at, r, c);
         write_cts_cols(cts_info, 0, at, r, c);
         if(job.stat_out) write_cts_cols(cts_info, 0, job.stat_at, r, n_header_columns);
      }
      //
      // ECLV output line
      //
      else if(lt == stat_eclv) {
         mpr_to_ctc(job, it->second, cts_info);
         at.set_entry(r, c++, "ECLV:");
         write_case_cols(it->first, at, r, c);
         write_eclv_cols(cts_info.cts, job.out_eclv_points, at, r, c);
         if(job.stat_out) write_eclv_cols(cts_info.cts, job.out_eclv_points, job.stat_at, r, n_header_columns);
      }
      //
      // MCTC output line
      //
      else if(lt == stat_mctc) {
         mpr_to_mctc(job, it->second, mcts_info);
         at.set_entry(r, c++, "MCTC:");
         write_case_cols(it->first, at, r, c);
         write_mctc_cols(mcts_info, at, r, c);
         if(job.stat_out) write_mctc_cols(mcts_info, job.stat_at, r, n_header_columns);
      }
      //
      // MCTS output line
      //
      else if(lt == stat_mcts) {
         mpr_to_mcts(job, it->second, mcts_info, tmp_dir, rng_ptr);
         at.set_entry(r, c++, "MCTS:");
         write_case_cols(it->first, at, r, c);
         write_mcts_cols(mcts_info, 0, at, r, c);
         if(job.stat_out) write_mcts_cols(mcts_info, 0, job.stat_at, r, n_header_columns);
      }
      //
      // CNT output line
      //
      else if(lt == stat_cnt) {
         mpr_to_cnt(job, it->second, cnt_info, tmp_dir, rng_ptr);
         at.set_entry(r, c++, "CNT:");
         write_case_cols(it->first, at, r, c);
         write_cnt_cols(cnt_info, 0, at, r, c);
         if(job.stat_out) write_cnt_cols(cnt_info, 0, job.stat_at, r, n_header_columns);
      }
      //
      // SL1L2 output line
      //
      else if(lt == stat_sl1l2) {
         mpr_to_psum(job, it->second, sl1l2_info);
         at.set_entry(r, c++, "SL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sl1l2_cols(sl1l2_info, at, r, c);
         if(job.stat_out) write_sl1l2_cols(sl1l2_info, job.stat_at, r, n_header_columns);
      }
      //
      // SAL1L2 output line
      //
      else if(lt == stat_sal1l2) {
         mpr_to_psum(job, it->second, sl1l2_info);
         at.set_entry(r, c++, "SAL1L2:");
         write_case_cols(it->first, at, r, c);
         write_sal1l2_cols(sl1l2_info, at, r, c);
         if(job.stat_out) write_sal1l2_cols(sl1l2_info, job.stat_at, r, n_header_columns);
      }
      //
      // PCT output line
      //
      else if(lt == stat_pct) {
         mpr_to_pct(job, it->second, pct_info);
         at.set_entry(r, c++, "PCT:");
         write_case_cols(it->first, at, r, c);
         write_pct_cols(pct_info, at, r, c);
         if(job.stat_out) write_pct_cols(pct_info, job.stat_at, r, n_header_columns);
      }
      //
      // PSTD output line
      //
      else if(lt == stat_pstd) {
         mpr_to_pct(job, it->second, pct_info);
         at.set_entry(r, c++, "PSTD:");
         write_case_cols(it->first, at, r, c);
         write_pstd_cols(pct_info, 0, at, r, c);
         if(job.stat_out) write_pstd_cols(pct_info, 0, job.stat_at, r, n_header_columns);
      }
      //
      // PJC output line
      //
      else if(lt == stat_pjc) {
         mpr_to_pct(job, it->second, pct_info);
         at.set_entry(r, c++, "PJC:");
         write_case_cols(it->first, at, r, c);
         write_pjc_cols(pct_info, at, r, c);
         if(job.stat_out) write_pjc_cols(pct_info, job.stat_at, r, n_header_columns);
      }
      //
      // PRC output line
      //
      else if(lt == stat_prc) {
         mpr_to_pct(job, it->second, pct_info);
         at.set_entry(r, c++, "PRC:");
         write_case_cols(it->first, at, r, c);
         write_prc_cols(pct_info, at, r, c);
         if(job.stat_out) write_prc_cols(pct_info, job.stat_at, r, n_header_columns);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_aggr_mpr_wind(STATAnalysisJob &job, STATLineType lt,
                             map<ConcatString, AggrWindInfo> &m,
                             AsciiTable &at) {
   map<ConcatString, AggrWindInfo>::iterator it;
   int n_row, n_col, r, c;
   StatHdrColumns shc;

   //
   // Setup the output table
   //
   n_row = 1 + m.size();
   n_col = 1 + job.by_column.n();
        if(lt == stat_vl1l2) n_col += n_vl1l2_columns;
   else if(lt == stat_vcnt)  n_col += n_vcnt_columns;
   write_job_aggr_hdr(job, n_row, n_col, at);

   //
   // Write the rest of the header row
   //
   c = 1 + job.by_column.n();
        if(lt == stat_vl1l2) write_header_row(vl1l2_columns, n_vl1l2_columns, 0, at, 0, c);
   else if(lt == stat_vcnt)  write_header_row(vcnt_columns,  n_vcnt_columns,  0, at, 0, c);

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_row, 0);

   mlog << Debug(2) << "Computing output for "
        << (int) m.size() << " case(s).\n";

   //
   // Loop through the map
   //
   for(it = m.begin(), r=1; it != m.end(); it++, r++) {

      //
      // Write the output STAT header columns
      //
      shc = it->second.hdr.get_shc(it->first, job.by_column,
                                   job.hdr_name, job.hdr_value, lt);
      if(job.stat_out) {
         if(lt == stat_cnt || lt == stat_nbrcnt) shc.set_alpha(job.out_alpha);
         write_header_cols(shc, job.stat_at, r);
      }

      //
      // Initialize
      //
      c = 0;

      //
      // VL1L2 output line
      //
      if(lt == stat_vl1l2) {
         at.set_entry(r, c++, "VL1L2:");
         write_case_cols(it->first, at, r, c);
         write_vl1l2_cols(it->second.vl1l2_info, at, r, c);
         if(job.stat_out) write_vl1l2_cols(it->second.vl1l2_info, job.stat_at, r, n_header_columns);
      }
      //
      // VCNT output line
      //
      else if(lt == stat_vcnt) {
         at.set_entry(r, c++, "VCNT:");
         write_case_cols(it->first, at, r, c);
         write_vcnt_cols(it->second.vl1l2_info, at, r, c);
         if(job.stat_out) write_vcnt_cols(it->second.vl1l2_info, job.stat_at, r, n_header_columns);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Add a switch to apply the swinging door algorithm and include that
// switch in the ramp job definition columns.
//
////////////////////////////////////////////////////////////////////////

void write_job_ramp(STATAnalysisJob &job,
                    map<ConcatString, AggrTimeSeriesInfo> &m,
                    AsciiTable &ctc_at, AsciiTable &cts_at,
                    AsciiTable &mpr_at) {
   map<ConcatString, AggrTimeSeriesInfo>::iterator it;
   NumArray framps, oramps, fdat, odat;
   NumArray vals_part, ramp_part, dat_part;
   TimeArray ts, ts_part, beg, end;
   int i, k, f, o, c;
   int cur_dt, min_dt, i_min_dt, i_swap;
   int n_row, n_col;
   bool sort_vld;
   unixtime init_ut, valid_ut;
   CTSInfo cts_info;
   StatHdrColumns ctc_shc, cts_shc;
   ConcatString cs;

   // Initialize to one header row
   int n_stat_row = 1;
   int r_stat, r_ctc, r_cts, r_mpr;

   //
   // Setup CTC output table
   //
   if(job.out_line_type.has(stat_ctc_str)) {
      n_row = 1 + m.size();
      n_stat_row += m.size();
      n_col = 1 + job.by_column.n() +
              n_job_ramp_columns + n_ctc_columns;
      write_job_aggr_hdr(job, n_row, n_col, ctc_at);

      //
      // Write the ramp job definition and CTC header columns
      //
      c = 1 + job.by_column.n();
      write_header_row(job_ramp_columns, n_job_ramp_columns, 0, ctc_at, 0, c);
      c += n_job_ramp_columns;
      write_header_row(ctc_columns, n_ctc_columns, 0, ctc_at, 0, c);
   }

   //
   // Setup CTS output table
   //
   if(job.out_line_type.has(stat_cts_str)) {
      n_row = 1 + m.size();
      n_stat_row += m.size();
      n_col = 1 + job.by_column.n() +
              n_job_ramp_columns + n_cts_columns;
      write_job_aggr_hdr(job, n_row, n_col, cts_at);

      //
      // Write the ramp job definition and CTS header columns
      //
      c = 1 + job.by_column.n();
      write_header_row(job_ramp_columns, n_job_ramp_columns, 0, cts_at, 0, c);
      c += n_job_ramp_columns;
      write_header_row(cts_columns, n_cts_columns, 0, cts_at, 0, c);
   }

   //
   // Setup MPR output table
   //
   if(job.out_line_type.has(stat_mpr_str)) {
      for(it = m.begin(), n_row = 1; it != m.end(); it++) {
         n_row += it->second.valid_ts.n();
      }
      n_col = 1 + job.by_column.n() +
              n_job_ramp_columns + n_job_ramp_mpr_columns;
      write_job_aggr_hdr(job, n_row, n_col, mpr_at);

      //
      // Write the ramp job definition and RAMP_MPR header columns
      //
      c = 1 + job.by_column.n();
      write_header_row(job_ramp_columns, n_job_ramp_columns, 0, mpr_at, 0, c);
      c += n_job_ramp_columns;
      write_header_row(job_ramp_mpr_columns, n_job_ramp_mpr_columns, 0, mpr_at, 0, c);
   }

   //
   // Setup the output STAT file
   //
   job.setup_stat_file(n_stat_row, 0);

   mlog << Debug(2) << "Applying ramp detection logic for "
        << (int) m.size() << " case(s).\n";

   //
   // Initialize output stats
   //
   cts_info.allocate_n_alpha(1);
   cts_info.alpha[0] = job.out_alpha;

   //
   // Loop through the map
   //
   for(it = m.begin(), r_stat = r_ctc = r_cts = r_mpr = 1; it != m.end(); it++) {

      //
      // Nothing to do
      //
      if(it->second.valid_ts.n() == 0) continue;

      //
      // Initialize
      //
      framps.clear();
      oramps.clear();
      fdat.clear();
      odat.clear();

      //
      // Sort the ramp data
      //
      it->second.sort();

      //
      // Check for series of valid (true) or initialization (false) times
      //
      sort_vld = (it->second.valid_ts.n() ==
                  it->second.f_na.n());
      ts       = (sort_vld ? it->second.valid_ts : it->second.init_ts);

      //
      // Subset the time series down to consistent time steps
      //
      ts.equal_dt(beg, end);

      if(beg.n() > 1) {
         mlog << Debug(4)
              << "Processing time series in " << beg.n()
              << " parts due to inconsistent time step.\n";
      }

      //
      // Process each consistent time segement
      //
      for(i=0; i<beg.n(); i++) {

         //
         // Get the current segment
         //
         ts_part = ts.subset(ts.index(beg[i]), ts.index(end[i]));

         cur_dt = (beg[i] == end[i] ? 0 :
                   (end[i] - beg[i]) / (ts_part.n() - 1));

         mlog << Debug(4)
              << "Processing time series from "
              << unix_to_yyyymmdd_hhmmss(beg[i]) << " to "
              << unix_to_yyyymmdd_hhmmss(end[i]) << " with time step of "
              << sec_to_hhmmss(cur_dt) << ".\n";

         //
         // Compute forecast ramps
         //
         cs << cs_erase << it->second.fcst_var << " " << job.column[0];
         mlog << Debug(4)
              << "Computing " << cs << " "
              << (sort_vld ? "valid" : "initialization")
              <<  " time series " << timeseriestype_to_string(job.ramp_type)
              << " ramps for case \"" << it->first << "\".\n";

         vals_part = it->second.f_na.subset(ts.index(beg[i]), ts.index(end[i]));

         if(job.ramp_type == TimeSeriesType_DyDt) {
            compute_dydt_ramps(cs.c_str(), vals_part, ts_part,
               job.ramp_time_fcst, job.ramp_exact_fcst, job.ramp_thresh_fcst,
               ramp_part, dat_part);
         }
         else {
            compute_swing_ramps(cs.c_str(), vals_part, ts_part,
               job.swing_width, job.ramp_thresh_fcst,
               ramp_part, dat_part);
         }

         //
         // Store data for current segment
         //
         framps.add(ramp_part);
         fdat.add(dat_part);

         //
         // Compute observed ramps
         //
         cs << cs_erase << it->second.obs_var << " " << job.column[1];
         mlog << Debug(4)
              << "Computing " << cs << " "
              << (sort_vld ? "valid" : "initialization")
              <<  " time series " << timeseriestype_to_string(job.ramp_type)
              << " ramps for case \"" << it->first << "\".\n";

         vals_part = it->second.o_na.subset(ts.index(beg[i]), ts.index(end[i]));

         if(job.ramp_type == TimeSeriesType_DyDt) {
            compute_dydt_ramps(cs.c_str(), vals_part, ts_part,
               job.ramp_time_obs, job.ramp_exact_obs, job.ramp_thresh_obs,
               ramp_part, dat_part);
         }
         else {
            compute_swing_ramps(cs.c_str(), vals_part, ts_part,
               job.swing_width, job.ramp_thresh_obs,
               ramp_part, dat_part);
         }

         //
         // Store data for current segment
         //
         oramps.add(ramp_part);
         odat.add(dat_part);
      } // end for i

      //
      // Populate the ramp contingency table
      //
      cts_info.cts.zero_out();
      for(i=0; i<ts.n(); i++) {

         // Initialize
         i_swap = bad_data_int;

         // Get current case
         f = nint(framps[i]);
         o = nint(oramps[i]);

         //
         // When f and o are both valid but disagree, search the matching time window.
         // Try to switch misses to hits and false alarms to correct negatives.
         //
         if(!is_bad_data(f) && !is_bad_data(o) && f != o &&
            (job.ramp_window_end - job.ramp_window_beg) > 0) {

            //
            // Search for points in the time window
            //
            for(k=0, min_dt=bad_data_int, i_min_dt=bad_data_int;
                k<ts.n(); k++) {

               //
               // Skip points before the time window
               //
               if(ts[k] < ts[i] + job.ramp_window_beg) continue;

               //
               // Break out of the loop after the time window
               //
               if(ts[k] > ts[i] + job.ramp_window_end) break;

               //
               // Skip forecasts that don't agree with the current observation
               //
               if(framps[k] != o) continue;

               //
               // Find closest match in time
               //
               cur_dt = labs(ts[i] - ts[k]);
               if(is_bad_data(min_dt))  { min_dt = cur_dt; i_min_dt = k; }
               else if(cur_dt < min_dt) { min_dt = cur_dt; i_min_dt = k; }
            } // end for k

            //
            // Switch the category if a match was found in the window
            //
            if(!is_bad_data(min_dt)) {
               f = o;
               i_swap = i_min_dt;
               mlog << Debug(4)
                    << "Switching "
                    << (o == 1 ? "FN_OY" : "FY_ON") << " to "
                    << (o == 1 ? "FY_OY" : "FN_ON")
                    << " at " << unix_to_yyyymmdd_hhmmss(ts[i])
                    << " since forecast "  << (o == 1 ? "event" : "non-event")
                    << " at " << unix_to_yyyymmdd_hhmmss(ts[i_swap])
                    << " fell within the ramp time window.\n";
            }
         }

         //
         // Update the contingency table counts
         //
         if(!is_bad_data(f) && !is_bad_data(o)) {
                 if(f == 1 && o == 1) cts_info.cts.inc_fy_oy();
            else if(f == 1 && o == 0) cts_info.cts.inc_fy_on();
            else if(f == 0 && o == 1) cts_info.cts.inc_fn_oy();
            else if(f == 0 && o == 0) cts_info.cts.inc_fn_on();
         }

         //
         // Write the ramp MPR output lines
         //
         if(job.out_line_type.has(stat_mpr_str)) {
            c = 0;
            mpr_at.set_entry(r_mpr, c++, "RAMP_MPR:");
            write_case_cols(it->first, mpr_at, r_mpr, c);
            write_job_ramp_cols(job, mpr_at, r_mpr, c);

            //
            // Pick which index to write
            //
            k = (is_bad_data(i_swap) ? i : i_swap);

            //
            // Write ramp MPR columns:
            //   TOTAL, INDEX,
            //   INIT,  LEAD, VALID,
            //   FPRV,  FCUR, FDLT, FRAMP,
            //   OPRV,  OCUR, ODLT, ORAMP,
            //   CATEGORY
            //
            mpr_at.set_entry(r_mpr, c++, it->second.valid_ts.n());
            mpr_at.set_entry(r_mpr, c++, i+1);
            init_ut  = (it->second.init_ts.n() > 1 ?
                        it->second.init_ts[i] : it->second.init_ts[0]);
            valid_ut = (it->second.valid_ts.n() > 1 ?
                       it->second.valid_ts[i] : it->second.valid_ts[0]);
            mpr_at.set_entry(r_mpr, c++, unix_to_yyyymmdd_hhmmss(init_ut));
            mpr_at.set_entry(r_mpr, c++, sec_to_hhmmss((int) valid_ut - init_ut));
            mpr_at.set_entry(r_mpr, c++, unix_to_yyyymmdd_hhmmss(valid_ut));
            if(job.ramp_type == TimeSeriesType_DyDt) {
               mpr_at.set_entry(r_mpr, c++, fdat[k]);
               mpr_at.set_entry(r_mpr, c++, it->second.f_na[k]);
               mpr_at.set_entry(r_mpr, c++, (is_bad_data(fdat[k]) || is_bad_data(it->second.f_na[k]) ?
                                             bad_data_double : it->second.f_na[k] - fdat[k]));
            }
            else {
               mpr_at.set_entry(r_mpr, c++, bad_data_double);
               mpr_at.set_entry(r_mpr, c++, it->second.f_na[k]);
               mpr_at.set_entry(r_mpr, c++, fdat[k]);
            }
            mpr_at.set_entry(r_mpr, c++, (is_bad_data(f) ? na_str : bool_to_string(f)));
            if(job.ramp_type == TimeSeriesType_DyDt) {
               mpr_at.set_entry(r_mpr, c++, odat[k]);
               mpr_at.set_entry(r_mpr, c++, it->second.o_na[k]);
               mpr_at.set_entry(r_mpr, c++, (is_bad_data(odat[k]) || is_bad_data(it->second.o_na[k]) ?
                                             bad_data_double : it->second.o_na[k] - odat[k]));
            }
            else {
               mpr_at.set_entry(r_mpr, c++, bad_data_double);
               mpr_at.set_entry(r_mpr, c++, it->second.o_na[k]);
               mpr_at.set_entry(r_mpr, c++, odat[k]);
            }
            mpr_at.set_entry(r_mpr, c++, (is_bad_data(o) ? na_str : bool_to_string(o)));
            if(is_bad_data(f) || is_bad_data(o)) {
               cs = na_str;
            }
            else {
               cs << cs_erase
                  << "F"  << (f == 1 ? "Y" : "N")
                  << "_O" << (o == 1 ? "Y" : "N");
            }
            mpr_at.set_entry(r_mpr, c++, cs);

            // Increment ramp MPR row counter
            r_mpr++;
         }
      } // end for i

      //
      // Copy over the output STAT header columns
      //
      ctc_shc = it->second.hdr.get_shc(it->first, job.by_column,
                                       job.hdr_name, job.hdr_value,
                                       stat_ctc);

      //
      // Compute contingency table statistics, if requested
      //
      if(job.out_line_type.has(stat_cts_str)) {
         cts_shc = it->second.hdr.get_shc(it->first, job.by_column,
                                          job.hdr_name, job.hdr_value,
                                          stat_cts);
         cts_shc.set_alpha(job.out_alpha);
         cts_info.compute_stats();
         cts_info.compute_ci();
      }

      //
      // Update FCST_VAR, if not user-defined
      //
      if(!job.hdr_name.has("FCST_VAR")) {
         cs << cs_erase << ctc_shc.get_fcst_var() << "_" << job.column[0] << "_RAMP";
         ctc_shc.set_fcst_var(cs);
         cts_shc.set_fcst_var(cs);
      }

      //
      // Update OBS_VAR, if not user-defined
      //
      if(!job.hdr_name.has("OBS_VAR")) {
         cs << cs_erase << ctc_shc.get_obs_var() << "_" << job.column[1] << "_RAMP";
         ctc_shc.set_obs_var(cs);
         cts_shc.set_obs_var(cs);
      }

      //
      // Set FCST_THRESH, if not user-defined
      //
      if(!job.hdr_name.has("FCST_THRESH")) {
         ctc_shc.set_fcst_thresh(job.ramp_thresh_fcst);
         cts_shc.set_fcst_thresh(job.ramp_thresh_fcst);
      }

      //
      // Set OBS_THRESH, if not user-defined
      //
      if(!job.hdr_name.has("OBS_THRESH")) {
         ctc_shc.set_obs_thresh(job.ramp_thresh_obs);
         cts_shc.set_obs_thresh(job.ramp_thresh_obs);
      }

      //
      // Write the CTC and CTS lines to output STAT file
      //
      if(job.stat_out) {

         // CTC line type
         if(job.out_line_type.has(stat_ctc_str)) {
            write_header_cols(ctc_shc, job.stat_at, r_stat);
            write_ctc_cols(cts_info, job.stat_at, r_stat, n_header_columns);
            r_stat++;
         }

         // CTS line type
         if(job.out_line_type.has(stat_cts_str)) {
            write_header_cols(cts_shc, job.stat_at, r_stat);
            write_cts_cols(cts_info, 0, job.stat_at, r_stat, n_header_columns);
            r_stat++;
         }
      }

      //
      // Write the ramp CTC output lines
      //
      if(job.out_line_type.has(stat_ctc_str)) {
         c = 0;
         ctc_at.set_entry(r_ctc, c++, "RAMP_CTC:");
         write_case_cols(it->first, ctc_at, r_ctc, c);
         write_job_ramp_cols(job, ctc_at, r_ctc, c);
         write_ctc_cols(cts_info, ctc_at, r_ctc, c);
         r_ctc++;
      }

      //
      // Write the ramp CTS output lines
      //
      if(job.out_line_type.has(stat_cts_str)) {
         c = 0;
         cts_at.set_entry(r_cts, c++, "RAMP_CTS:");
         write_case_cols(it->first, cts_at, r_cts, c);
         write_job_ramp_cols(job, cts_at, r_cts, c);
         write_cts_cols(cts_info, 0, cts_at, r_cts, c);
         r_cts++;
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void write_job_ramp_cols(const STATAnalysisJob &job, AsciiTable &at,
                         int &r, int &c) {

   //
   // Ramp Job Defintion columns:
   //    TYPE,
   //    FCOLUMN,    OCOLUMN,
   //    FTIME,      OTIME,
   //    FEXACT,     OEXACT,
   //    FTHRESH,    OTHRESH,
   //    WINDOW_BEG, WINDOW_END
   //
   at.set_entry(r, c++, timeseriestype_to_string(job.ramp_type));
   at.set_entry(r, c++, job.column[0]);
   at.set_entry(r, c++, job.column[1]);
   at.set_entry(r, c++, sec_to_hhmmss(job.ramp_time_fcst));
   at.set_entry(r, c++, sec_to_hhmmss(job.ramp_time_obs));
   at.set_entry(r, c++, bool_to_string(job.ramp_exact_fcst));
   at.set_entry(r, c++, bool_to_string(job.ramp_exact_obs));
   at.set_entry(r, c++, job.ramp_thresh_fcst.get_str());
   at.set_entry(r, c++, job.ramp_thresh_obs.get_str());
   at.set_entry(r, c++, sec_to_hhmmss(job.ramp_window_beg));
   at.set_entry(r, c++, sec_to_hhmmss(job.ramp_window_end));

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
                     STATAnalysisJob &job, int &n_in, int &n_out,
                     ofstream *sa_out) {
   double go_index;
   AsciiTable out_at;

   //
   // Compute the GO Index as a special case of the Skill Score Index
   //
   go_index = compute_ss_index(jobstring, f, job, n_in, n_out);

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
   out_at.set_size(2, 2);
   setup_table(out_at, 1, job.get_precision());
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
                     STATAnalysisJob &job, int &n_in, int &n_out,
                     ofstream *sa_out) {
   double ss_index;
   AsciiTable out_at;

   //
   // Compute the Skill Score Index
   //
   ss_index = compute_ss_index(jobstring, f, job, n_in, n_out);

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
   out_at.set_size(2, 2);
   setup_table(out_at, 1, job.get_precision());
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
//
// The do_job_ramp() routine applies rapid intensification/weakening
// logic to a time series of forecast and observation values and
// derives contingency table counts and statistics by thresholding the
// changes between times.
//
////////////////////////////////////////////////////////////////////////

void do_job_ramp(const ConcatString &jobstring, LineDataFile &f,
                 STATAnalysisJob &job, int &n_in, int &n_out,
                 ofstream *sa_out) {
   STATLine line;
   AsciiTable ctc_at, cts_at, mpr_at;

   map<ConcatString, AggrTimeSeriesInfo> ramp_map;

   //
   // Determine the input line type, use default if necessary
   //
   if(job.line_type.n() == 0) {
      job.line_type.add(default_ramp_line_type);
   }
   if(job.line_type.n() != 1) {
      mlog << Error << "\ndo_job_ramp() -> "
           << "the \"-line_type\" option may be used at most once to "
           << "specify the input line type: " << jobstring << "\n\n";
      throw(1);
   }

   //
   // Determine the output line type(s), use default if necessary
   //
   if(job.out_line_type.n() == 0) {
      job.out_line_type.add_css(default_ramp_out_line_type);
   }
   if(!job.out_line_type.has(stat_ctc_str) &&
      !job.out_line_type.has(stat_cts_str) &&
      !job.out_line_type.has(stat_mpr_str)) {
      mlog << Error << "\ndo_job_ramp() -> "
           << "the \"-out_line_type\" option must be set to CTC, CTS, and/or MPR: "
           << jobstring << "\n\n";
      throw(1);
   }

   //
   // Determine the ramp columns, use defaults if necessary
   //
   if(job.column.n() == 0) {
      job.column.add(default_ramp_fcst_col);
      job.column.add(default_ramp_obs_col);
   }
   if(job.column.n() != 2) {
      mlog << Error << "\ndo_job_ramp() -> "
           << "the \"-column\" option may be used exactly twice to "
           << "specify the forecast and observation values for the "
           << "ramp job: " << jobstring
           << "\n\n";
      throw(1);
   }

   //
   // Check the ramp type
   //
   if(job.ramp_type != TimeSeriesType_DyDt &&
      job.ramp_type != TimeSeriesType_Swing) {
      mlog << Error << "\ndo_job_ramp() -> "
           << "unsupported \"-ramp_type\" option: "
           << jobstring << "\n\n";
      throw(1);
   }

   //
   // Check swing_width for the swinging door algorithm
   //
   if(job.ramp_type == TimeSeriesType_Swing &&
      is_bad_data(job.swing_width)) {
      mlog << Error << "\ndo_job_ramp() -> "
           << "the \"-swing_width\" option is required for \"-ramp_type SWING\": "
           << jobstring << "\n\n";
      throw(1);
   }

   //
   // Determine the ramp thresholds, no defaults
   //
   if(job.ramp_thresh_fcst.get_type() == thresh_na ||
      job.ramp_thresh_obs.get_type()  == thresh_na) {
      mlog << Error << "\ndo_job_ramp() -> "
           << "the \"-ramp_thresh\" or \"-ramp_thresh_fcst\" and "
           << "\"-ramp_thresh_obs\" options must be used to define the "
           << "ramp events: " << jobstring << "\n\n";
      throw(1);
   }

   //
   // Parse the input stat lines
   //
   aggr_time_series_lines(f, job, ramp_map, n_in, n_out);
   write_job_ramp(job, ramp_map, ctc_at, cts_at, mpr_at);

   //
   // Check for no matching STAT lines
   //
   if(n_out == 0) {
      mlog << Warning << "\ndo_job_ramp() -> "
           << "no matching STAT lines found for job: " << jobstring
           << "\n\n";
      return;
   }

   //
   // Write the ASCII Table and the job command line
   //
   write_jobstring(jobstring, sa_out);
   if(job.out_line_type.has(stat_ctc_str)) write_table(ctc_at, sa_out);
   if(job.out_line_type.has(stat_cts_str)) write_table(cts_at, sa_out);
   if(job.out_line_type.has(stat_mpr_str)) write_table(mpr_at, sa_out);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at, int n_hdr_cols, int prec) {
   int i;

   // Left-justify header columns and right-justify data columns
   for(i=0;          i<n_hdr_cols; i++) at.set_column_just(i, LeftJust);
   for(i=n_hdr_cols; i<at.ncols(); i++) at.set_column_just(i, RightJust);

   // Right-justify the first column
   at.set_column_just(0, RightJust);

   // Set the precision
   at.set_precision(prec);

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
                        STATAnalysisJob &job, int &n_in, int &n_out) {
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
   if(job.model.n() != 2) {
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
   if((n_terms = job.fcst_var.n()) < 1) {
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
   if(n_terms != job.fcst_lev.n()  ||
      n_terms != job.fcst_lead.n() ||
      n_terms != job.line_type.n() ||
      n_terms != job.column.n()    ||
      n_terms != job.weight.n()) {
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

   mlog << Debug(3) << "Forecast Model  = " << job.model[0] << "\n"
        << "Reference Model = " << job.model[1] << "\n";

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
      fcst_job[i] = job;

      //
      // model
      //
      fcst_job[i].model.clear();
      fcst_job[i].model.add(job.model[0]);

      //
      // fcst_lead
      //
      if(job.fcst_lead.n() == n_terms) {
         fcst_job[i].fcst_lead.clear();
         fcst_job[i].fcst_lead.add(job.fcst_lead[i]);
      }

      //
      // obs_lead
      //
      if(job.obs_lead.n() == n_terms) {
         fcst_job[i].obs_lead.clear();
         fcst_job[i].obs_lead.add(job.obs_lead[i]);
      }

      //
      // fcst_init_hour
      //
      if(job.fcst_init_hour.n() == n_terms) {
         fcst_job[i].fcst_init_hour.clear();
         fcst_job[i].fcst_init_hour.add(job.fcst_init_hour[i]);
      }

      //
      // obs_init_hour
      //
      if(job.obs_init_hour.n() == n_terms) {
         fcst_job[i].obs_init_hour.clear();
         fcst_job[i].obs_init_hour.add(job.obs_init_hour[i]);
      }

      //
      // fcst_var
      //
      if(job.fcst_var.n() == n_terms) {
         fcst_job[i].fcst_var.clear();
         fcst_job[i].fcst_var.add(job.fcst_var[i]);
      }

      //
      // obs_var
      //
      if(job.obs_var.n() == n_terms) {
         fcst_job[i].obs_var.clear();
         fcst_job[i].obs_var.add(job.obs_var[i]);
      }

      //
      // fcst_lev
      //
      if(job.fcst_lev.n() == n_terms) {
         fcst_job[i].fcst_lev.clear();
         fcst_job[i].fcst_lev.add(job.fcst_lev[i]);
      }

      //
      // obs_lev
      //
      if(job.obs_lev.n() == n_terms) {
         fcst_job[i].obs_lev.clear();
         fcst_job[i].obs_lev.add(job.obs_lev[i]);
      }

      //
      // obtype
      //
      if(job.obtype.n() == n_terms) {
         fcst_job[i].obtype.clear();
         fcst_job[i].obtype.add(job.obtype[i]);
      }

      //
      // vx_mask
      //
      if(job.vx_mask.n() == n_terms) {
         fcst_job[i].vx_mask.clear();
         fcst_job[i].vx_mask.add(job.vx_mask[i]);
      }

      //
      // interp_mthd
      //
      if(job.interp_mthd.n() == n_terms) {
         fcst_job[i].interp_mthd.clear();
         fcst_job[i].interp_mthd.add(job.interp_mthd[i]);
      }

      //
      // interp_pnts
      //
      if(job.interp_pnts.n() == n_terms) {
         fcst_job[i].interp_pnts.clear();
         fcst_job[i].interp_pnts.add(job.interp_pnts[i]);
      }

      //
      // fcst_thresh
      //
      if(job.fcst_thresh.n() == n_terms) {
         fcst_job[i].fcst_thresh.clear();
         fcst_job[i].fcst_thresh.add(job.fcst_thresh[i]);
      }

      //
      // obs_thresh
      //
      if(job.obs_thresh.n() == n_terms) {
         fcst_job[i].obs_thresh.clear();
         fcst_job[i].obs_thresh.add(job.obs_thresh[i]);
      }

      //
      // line_type
      //
      if(job.line_type.n() == n_terms) {
         fcst_job[i].line_type.clear();
         fcst_job[i].line_type.add(job.line_type[i]);

         job_lt[i] = string_to_statlinetype(job.line_type[i].c_str());
         if(job_lt[i] != stat_sl1l2 && job_lt[i] != stat_ctc) {
            if ( fcst_job )  { delete [] fcst_job;  fcst_job = 0; }
            if ( ref_job  )  { delete [] ref_job;   ref_job  = 0; }
            if ( fcst_si  )  { delete [] fcst_si;   fcst_si  = 0; }
            if ( ref_si   )  { delete [] ref_si;    ref_si   = 0; }
            if ( fcst_cts )  { delete [] fcst_cts;  fcst_cts = 0; }
            if ( ref_cts  )  { delete [] ref_cts;   ref_cts  = 0; }
            if ( job_lt   )  { delete [] job_lt;    job_lt   = 0; }
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
      if(job.column.n() == n_terms) {
         fcst_job[i].column.clear();
         fcst_job[i].column.add(job.column[i]);
      }

      //
      // weight
      //
      if(job.weight.n() == n_terms) {
         fcst_job[i].weight.clear();
         fcst_job[i].weight.add(job.weight[i]);
      }

      //
      // Set the reference model job identical to the forecast model
      // job but with a different model name.
      //
      ref_job[i] = fcst_job[i];
      ref_job[i].model.set(0, job.model[1]);

   } // end for i

   //
   // Process the STAT lines
   //
   n_in = n_out = 0;
   while(f >> line) {

      if(line.is_header()) continue;

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
         job.dump_stat_line(line);
         n_out++;
      }

   } // end while

   //
   // Loop through the terms and compute a skill score for each.
   //
   ss_sum = weight_sum = 0.0;
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

      if(strcasecmp(fcst_job[i].column[0].c_str(), "PR_CORR") == 0) {
         fcst_stat = fcst_cnt.pr_corr.v;
         ref_stat  = ref_cnt.pr_corr.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "ME") == 0) {
         fcst_stat = fcst_cnt.me.v;
         ref_stat  = ref_cnt.me.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "ESTDEV") == 0) {
         fcst_stat = fcst_cnt.estdev.v;
         ref_stat  = ref_cnt.estdev.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "MBIAS") == 0) {
         fcst_stat = fcst_cnt.mbias.v;
         ref_stat  = ref_cnt.mbias.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "MSE") == 0) {
         fcst_stat = fcst_cnt.mse.v;
         ref_stat  = ref_cnt.mse.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "BCRMSE") == 0) {
         fcst_stat = fcst_cnt.bcmse.v;
         ref_stat  = ref_cnt.bcmse.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "RMSE") == 0) {
         fcst_stat = fcst_cnt.rmse.v;
         ref_stat  = ref_cnt.rmse.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "BASER") == 0) {
         fcst_stat = fcst_cts[i].baser.v;
         ref_stat  = ref_cts[i].baser.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "FMEAN") == 0) {
         fcst_stat = fcst_cts[i].fmean.v;
         ref_stat  = ref_cts[i].fmean.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "ACC") == 0) {
         fcst_stat = fcst_cts[i].acc.v;
         ref_stat  = ref_cts[i].acc.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "FBIAS") == 0) {
         fcst_stat = fcst_cts[i].fbias.v;
         ref_stat  = ref_cts[i].fbias.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "PODY") == 0) {
         fcst_stat = fcst_cts[i].pody.v;
         ref_stat  = ref_cts[i].pody.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "PODN") == 0) {
         fcst_stat = fcst_cts[i].podn.v;
         ref_stat  = ref_cts[i].podn.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "POFD") == 0) {
         fcst_stat = fcst_cts[i].pofd.v;
         ref_stat  = ref_cts[i].pofd.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "FAR") == 0) {
         fcst_stat = fcst_cts[i].far.v;
         ref_stat  = ref_cts[i].far.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "CSI") == 0) {
         fcst_stat = fcst_cts[i].csi.v;
         ref_stat  = ref_cts[i].csi.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "GSS") == 0) {
         fcst_stat = fcst_cts[i].gss.v;
         ref_stat  = ref_cts[i].gss.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "HK") == 0) {
         fcst_stat = fcst_cts[i].hk.v;
         ref_stat  = ref_cts[i].hk.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "HSS") == 0) {
         fcst_stat = fcst_cts[i].hss.v;
         ref_stat  = ref_cts[i].hss.v;
      }
      else if(strcasecmp(fcst_job[i].column[0].c_str(), "ODDS") == 0) {
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

   for(int i=0; i<sa.n(); i++) at.set_entry(r, c++, sa[i]);

   return;
}

////////////////////////////////////////////////////////////////////////
