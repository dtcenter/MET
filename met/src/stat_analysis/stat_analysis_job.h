// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   stat_analysis_job.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_ANALYSIS_JOB_H__
#define  __STAT_ANALYSIS_JOB_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_met_util/vx_met_util.h"
#include "vx_analysis_util/stat_job.h"

////////////////////////////////////////////////////////////////////////

extern void do_job(const char *, STATAnalysisJob &, int,
                   const char *, ofstream *, int);

extern void do_job_filter(const char *, LineDataFile &,
               STATAnalysisJob &, int &, int &, ofstream *, int);

extern void do_job_summary(const char *, LineDataFile &,
               STATAnalysisJob &, int &, int &, ofstream *, int);

extern void do_job_aggr(const char *, LineDataFile &,
               STATAnalysisJob &, int &, int &, ofstream *, int);

extern void do_job_aggr_stat(const char *, LineDataFile &,
               STATAnalysisJob &, int &, int &, ofstream *, int);

extern void do_job_go_index(const char *, LineDataFile &,
               STATAnalysisJob &, int &, int &, ofstream *, int);

////////////////////////////////////////////////////////////////////////

extern void write_job_cts(STATAnalysisJob &, STATLineType,
               CTSInfo &, AsciiTable &);

extern void write_job_cnt(STATAnalysisJob &, STATLineType,
               SL1L2Info &, AsciiTable &);

extern void write_job_wdir(STATAnalysisJob &, STATLineType,
               VL1L2Info &,
               NumArray &, NumArray &, NumArray &, NumArray &,
               AsciiTable &);

extern void write_job_pct(STATAnalysisJob &, STATLineType,
               PCTInfo &, AsciiTable &);

extern void write_job_nbrcts(STATAnalysisJob &, STATLineType,
               NBRCTSInfo &, AsciiTable &);

extern void write_job_mpr(STATAnalysisJob &, STATLineType,
               CTSInfo &, CNTInfo &, SL1L2Info &, PCTInfo &,
               AsciiTable &);

////////////////////////////////////////////////////////////////////////

extern void setup_table    (AsciiTable &);
extern void write_table    (AsciiTable&,  ofstream *);
extern void write_jobstring(const char *, ofstream *);
extern void write_line     (const char *, ofstream *);

////////////////////////////////////////////////////////////////////////

extern double compute_go_index(const char *, LineDataFile &,
                 STATAnalysisJob &, int &, int &, int);

extern double compute_sl1l2_rmse(SL1L2Info &,
                 const char *, const char *, int);

////////////////////////////////////////////////////////////////////////

#endif   //  __STAT_ANALYSIS_JOB_H__

////////////////////////////////////////////////////////////////////////
