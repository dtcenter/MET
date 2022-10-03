// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   skill_score_index_job.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    07/28/21  Halley Gotway   New
//   001    09/28/22  Prestopnik      MET #2227 Remove namespace std
//
//
////////////////////////////////////////////////////////////////////////

#ifndef  __SKILL_SCORE_INDEX_JOB_H__
#define  __SKILL_SCORE_INDEX_JOB_H__

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <vector>
#include <cmath>

#include "met_stats.h"
#include "stat_job.h"
#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// Class used to compute the Skill Score Index Job
//
////////////////////////////////////////////////////////////////////////

class SSIndexJobInfo {

   private:
      void init_from_scratch();
      void assign(const SSIndexJobInfo &);

   public:

      SSIndexJobInfo();
      ~SSIndexJobInfo();
      SSIndexJobInfo(const SSIndexJobInfo &);
      SSIndexJobInfo & operator=(const SSIndexJobInfo &);

      // Index name
      ConcatString ss_index_name;

      // Valid data threshold
      double ss_index_vld_thresh;

      // Forecast and reference model names
      ConcatString fcst_model, ref_model;

      // Array of unique initialization times
      TimeArray init_time;

      // Number of terms
      int n_term;

      // Array of input lines counts for each term
      NumArray n_fcst_lines, n_ref_lines;

      // Input line type for each term
      std::vector<STATLineType> job_lt;

      // Vectors of jobs for each term
      std::vector<STATAnalysisJob> fcst_job;
      std::vector<STATAnalysisJob> ref_job;

      // Vectors of partial sums and contingency tables for each term
      std::vector<SL1L2Info> fcst_sl1l2;
      std::vector<SL1L2Info> ref_sl1l2;
      std::vector<CTSInfo>   fcst_cts;
      std::vector<CTSInfo>   ref_cts;

      // Add jobs for each term
      void add_term(const STATAnalysisJob &,
                    const STATAnalysisJob &);

      // Process STAT lines
      bool is_keeper(const STATLine &);
      bool add(STATLine &);
   
      // Compute the skill score index value
      SSIDXData compute_ss_index();

      void clear();
};

////////////////////////////////////////////////////////////////////////

#endif   //  __SKILL_SCORE_INDEX_JOB__

////////////////////////////////////////////////////////////////////////
