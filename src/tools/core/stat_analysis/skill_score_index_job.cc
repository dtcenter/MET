// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   skill_score_index_job.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    08/25/21  Halley Gotway   New
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

#include "skill_score_index_job.h"
#include "parse_stat_line.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class SSIndexJobInfo
//
////////////////////////////////////////////////////////////////////////

SSIndexJobInfo::SSIndexJobInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

SSIndexJobInfo::~SSIndexJobInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

SSIndexJobInfo::SSIndexJobInfo(const SSIndexJobInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

SSIndexJobInfo & SSIndexJobInfo::operator=(const SSIndexJobInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void SSIndexJobInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void SSIndexJobInfo::clear() {

   // Initialize
   ss_index_name.clear();
   ss_index_vld_thresh = 0.0;
   fcst_model.clear();
   ref_model.clear();
   n_term = 0;
   n_fcst_lines.clear();
   n_ref_lines.clear();
   job_lt.clear();
   fcst_job.clear();
   ref_job.clear();
   fcst_sl1l2.clear();
   ref_sl1l2.clear();
   fcst_cts.clear();
   ref_cts.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void SSIndexJobInfo::assign(const SSIndexJobInfo &c) {

   ss_index_name = c.ss_index_name;
   ss_index_vld_thresh = c.ss_index_vld_thresh;
   fcst_model = c.fcst_model;
   ref_model = c.ref_model;
   n_term = c.n_term;
   n_fcst_lines = c.n_fcst_lines;
   n_ref_lines = c.n_ref_lines;
   job_lt = c.job_lt;
   fcst_job = c.fcst_job;
   ref_job = c.ref_job;
   fcst_sl1l2 = c.fcst_sl1l2;
   ref_sl1l2 = c.ref_sl1l2;
   fcst_cts = c.fcst_cts;
   ref_cts = c.ref_cts;

   return;
}

////////////////////////////////////////////////////////////////////////

void SSIndexJobInfo::add_term(const STATAnalysisJob &fcst_term,
                              const STATAnalysisJob &ref_term) {

   // Increment the counter
   n_term++;

   // Add the jobs
   fcst_job.push_back(fcst_term);
   ref_job.push_back(ref_term);

   // Add the input line type for each job
   job_lt.push_back(string_to_statlinetype(fcst_term.line_type[0].c_str()));

   // Add partial sums
   SL1L2Info sl1l2_info;
   fcst_sl1l2.push_back(sl1l2_info);
   ref_sl1l2.push_back(sl1l2_info);

   // Add contingency tables
   CTSInfo cts_info;
   fcst_cts.push_back(cts_info);
   ref_cts.push_back(cts_info);

   // Add line counters
   n_fcst_lines.add(0);
   n_ref_lines.add(0);

   return;
}

////////////////////////////////////////////////////////////////////////

bool SSIndexJobInfo::is_keeper(const STATLine &line) {
   bool keep = false;

   // Check each forecast and reference job
   for(int i=0; i<n_term; i++) {
      if(fcst_job[i].is_keeper(line) ||
         ref_job[i].is_keeper(line)) {
         keep = true;
         break;
      }
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

bool SSIndexJobInfo::add(STATLine &line) {
   bool keep = false;
   SL1L2Info sl1l2;
   TTContingencyTable ctc;

   // Check each forecast and reference job
   for(int i=0; i<n_term; i++) {

      // Check the forecast model job
      if(fcst_job[i].is_keeper(line)) {
         keep = true;
         n_fcst_lines.inc(i, 1);

         // Aggregate SL1L2
         if(job_lt[i] == stat_sl1l2) {
            parse_sl1l2_line(line, sl1l2);
            fcst_sl1l2[i] += sl1l2;
         }
         // Aggregate CTC
         else if(job_lt[i] == stat_ctc) {
            parse_ctc_ctable(line, ctc);
            fcst_cts[i].cts += ctc;
         }
      }

      // Check the reference model job
      if(!keep && ref_job[i].is_keeper(line)) {
         keep = true;
         n_ref_lines.inc(i, 1);

         // Aggregate SL1L2
         if(job_lt[i] == stat_sl1l2) {
            parse_sl1l2_line(line, sl1l2);
            ref_sl1l2[i] += sl1l2;
         }
         // Aggregate CTC
         else if(job_lt[i] == stat_ctc) {
            parse_ctc_ctable(line, ctc);
            ref_cts[i].cts += ctc;
         }
      }

      // Break out of the loop after the first job match
      if(keep) break;
   }

   // Track the unique intialization times
   if(keep && !init_time.has(line.fcst_init_beg())) {
      init_time.add(line.fcst_init_beg());
   }

   return(keep);
}

////////////////////////////////////////////////////////////////////////

SSIDXData SSIndexJobInfo::compute_ss_index() {
   int i, n_vld, n_diff;
   double fcst_stat, ref_stat;
   double ss, ss_sum, weight_sum, ss_avg;
   CNTInfo fcst_cnt, ref_cnt;
   SSIDXData data;

   mlog << Debug(3)
        << "Computing " << ss_index_name << " for " << init_time.n()
        << " initialization time(s): " << write_css(init_time) << "\n";

   // Compute a skill score for each term
   for(i=0, n_vld=0, n_diff=0, ss_sum=weight_sum=0.0; i<n_term; i++) {

      // Continuous stats
      if(job_lt[i] == stat_sl1l2) {
         compute_cntinfo(fcst_sl1l2[i], 0, fcst_cnt);
         fcst_stat = fcst_cnt.get_stat(fcst_job[i].column[0].c_str());
         compute_cntinfo(ref_sl1l2[i], 0, ref_cnt);
         ref_stat = ref_cnt.get_stat(fcst_job[i].column[0].c_str());
      }
      // Categorical stats
      else if(job_lt[i] == stat_ctc) {
         fcst_cts[i].compute_stats();
         fcst_stat = fcst_cts[i].get_stat(fcst_job[i].column[0].c_str());
         ref_cts[i].compute_stats();
         ref_stat = ref_cts[i].get_stat(fcst_job[i].column[0].c_str());
      }
      else {
         mlog << Error << "\nSSIndexJobInfo::compute_ss_index() -> "
              << "unxpected line type of " << statlinetype_to_string(job_lt[i])
              << "!\n\n";
         exit(1);
      }

      // Check for conditions when a skill score cannot be computed
      if(nint(n_fcst_lines[i]) == 0 || nint(n_ref_lines[i]) == 0   ||
         is_bad_data(fcst_stat)     || is_bad_data(ref_stat)       ||
         is_eq(ref_stat, 0.0)) {
         ss = bad_data_double;
      }
      // Compute the skill score and keep a running sum of the skill
      // scores and weights
      else {
         ss          = 1.0 - (fcst_stat*fcst_stat)/(ref_stat*ref_stat);
         ss_sum     += ss*fcst_job[i].weight[0];
         weight_sum += fcst_job[i].weight[0];
      }

      // Format log message strings
      ConcatString term_cs, fcst_cs, ref_cs, ss_cs;
      term_cs << fcst_job[i].fcst_var[0] << "/"
              << fcst_job[i].fcst_lev[0] << " at "
              << sec_to_hhmmss(fcst_job[i].fcst_lead[0]) << " lead";
      if(is_bad_data(fcst_stat)) fcst_cs << na_str;
      else                       fcst_cs << fcst_stat;
      if(is_bad_data(ref_stat))  ref_cs  << na_str;
      else                       ref_cs  << ref_stat;
      if(is_bad_data(ss))        ss_cs   << na_str;
      else                       ss_cs   << ss;

      // Print debug info for each term
      mlog << Debug(3) << ss_index_name << " Term " << i+1
           << ": " << term_cs << ", "
           << n_fcst_lines[i] << " fcst "
           << fcst_job[i].line_type[0] << "->"
           << fcst_job[i].column[0] << " = " << fcst_cs << ", "
           << n_ref_lines[i] << " ref "
           << fcst_job[i].line_type[0] << "->"
           << fcst_job[i].column[0] << " = " << ref_cs
           << ", skill = " << ss_cs
           << ", weight = " << fcst_job[i].weight[0] << "\n";

      // Check if the number of aggregated lines differ
      if(nint(n_fcst_lines[i]) != nint(n_ref_lines[i])) {
         n_diff++;
         mlog << Debug(3) << "\nSSIndexJobInfo::compute_ss_index() -> "
              << "the number of aggregated forecast and reference lines "
              << "differ (" << n_fcst_lines[i] << " != " << n_ref_lines[i]
              << ") for term " << i+1 << " (" << term_cs << ").\n\n";
      }

      if(is_bad_data(ss)) {
         mlog << Debug(3) << "\nSSIndexJobInfo::compute_ss_index() -> "
              << "cannot compute the skill score for term " << i+1
              << " (" << term_cs << ").\n\n";
      }
      else {
         n_vld++;
      }

   } // end for i

   // Print warning about differing numbers of input lines
   if(n_diff > 0) {
      mlog << Warning << "\nSSIndexJobInfo::compute_ss_index() -> "
           << "the number of aggregated forecast and reference lines "
           << "differ for " << n_diff << " of the " << n_term << " terms. "
           << "Rerun at verbosity level 3 (-v 3) for details.\n\n";
   }

   // Print warning about bad data in skill scores
   if(n_vld != n_term) {
      mlog << Warning << "\nSSIndexJobInfo::compute_ss_index() -> "
           << "cannot compute the skill score for " << n_term - n_vld
           << " of the " << n_term << " terms. "
           << "Rerun at verbosity level 3 (-v 3) for details.\n\n";
   }

   // Check the required ratio of valid terms
   if(((double) n_vld / n_term) < ss_index_vld_thresh) {
      mlog << Warning << "\nSSIndexJobInfo::compute_ss_index() -> "
           << "skipping " << ss_index_name
           << " since the ratio of valid terms " << n_vld << "/"
           << n_term << " < " << ss_index_vld_thresh << "!\n\n";
      data.n_vld = 0;
      return(data);
   }

   // Compute the weighted average of the skill scores
   if(is_eq(weight_sum, 0.0)) ss_avg = bad_data_double;
   else                       ss_avg = ss_sum/weight_sum;

   // Store the output data
   data.ss_index_name = ss_index_name;
   data.fcst_model    = fcst_model;
   data.ref_model     = ref_model;
   data.init_time     = init_time;
   data.n_term        = n_term;
   data.n_vld         = n_vld;
   if(is_bad_data(ss_avg) || is_eq(ss_avg, 1.0)) {
      data.ss_index = bad_data_double;
   }
   else {
      data.ss_index = sqrt(1.0/(1.0 - ss_avg));
   }

   mlog << Debug(3)
        << ss_index_name << " Weighted Average = " << ss_avg << "\n"
        << ss_index_name << " Skill Score Value = " << data.ss_index << "\n";

   return(data);
}

////////////////////////////////////////////////////////////////////////
