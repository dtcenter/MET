// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
//   001    08/16/11  Halley Gotway   Reimplementation of GO Index job
//                    with addition of generalized Skill Score Index
//   002    05/03/12  Halley Gotway   Switch to using vx_config library.
//   003    02/04/13  Halley Gotway   Add -by case option.
//   004    03/07/13  Halley Gotway   Add aggregate SSVAR lines.
//   005    06/09/17  Halley Gotway   Add aggregate RELP lines.
//   006    10/09/17  Halley Gotway   Add aggregate GRAD lines.
//   007    03/01/18  Halley Gotway   Update summary job type.
//   008    01/24/20  Halley Gotway   Add aggregate RPS lines.
//   009    09/28/22  Prestopnik      MET #2227 Remove namespace std
//
////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_ANALYSIS_JOB_H__
#define  __STAT_ANALYSIS_JOB_H__

////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_analysis_util.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"
#include "aggr_stat_line.h"

////////////////////////////////////////////////////////////////////////

extern void set_job_from_config(MetConfig &, STATAnalysisJob &);

extern void do_job(const ConcatString &, STATAnalysisJob &, int,
               const ConcatString &, const ConcatString &,
               std::ofstream *);

extern void do_job_filter(const ConcatString &, LineDataFile &,
               STATAnalysisJob &, int &, int &, std::ofstream *);

extern void do_job_summary(const ConcatString &, LineDataFile &,
               STATAnalysisJob &, int &, int &, std::ofstream *, gsl_rng *);

extern void do_job_aggr(const ConcatString &, LineDataFile &,
               STATAnalysisJob &, int &, int &, std::ofstream *);

extern void do_job_aggr_stat(const ConcatString &, LineDataFile &,
               STATAnalysisJob &, int &, int &, std::ofstream *,
               const ConcatString &, gsl_rng *);

extern void do_job_ss_index(const ConcatString &, LineDataFile &,
               STATAnalysisJob &, int &, int &, std::ofstream *);

extern void do_job_ramp(const ConcatString &, LineDataFile &,
               STATAnalysisJob &, int &, int &, std::ofstream *);

////////////////////////////////////////////////////////////////////////

extern void write_job_aggr_hdr(STATAnalysisJob &,
               int, int, AsciiTable &);

extern void write_job_summary(STATAnalysisJob &,
               std::map<ConcatString, AggrSummaryInfo> &, AsciiTable &,
               gsl_rng *);

extern void write_job_aggr_ctc(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrCTCInfo> &, AsciiTable &);

extern void write_job_aggr_mctc(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrMCTCInfo> &, AsciiTable &);

extern void write_job_aggr_pct(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrPCTInfo> &, AsciiTable &);

extern void write_job_aggr_psum(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrPSumInfo> &, AsciiTable &);

extern void write_job_aggr_grad(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrGRADInfo> &, AsciiTable &);

extern void write_job_aggr_wind(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrWindInfo> &, AsciiTable &);

extern void write_job_aggr_ecnt(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrENSInfo> &, AsciiTable &);

extern void write_job_aggr_rps(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrRPSInfo> &, AsciiTable &);

extern void write_job_aggr_rhist(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrENSInfo> &, AsciiTable &);

extern void write_job_aggr_phist(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrENSInfo> &, AsciiTable &);

extern void write_job_aggr_relp(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrENSInfo> &, AsciiTable &);

extern void write_job_aggr_ssvar(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrSSVARInfo> &, AsciiTable &);

extern void write_job_aggr_seeps(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrSEEPSInfo> &, AsciiTable &);

extern void write_job_aggr_seeps_mpr(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrSEEPSMPRInfo> &, AsciiTable &);

extern void write_job_aggr_orank(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrENSInfo> &, AsciiTable &,
               gsl_rng *);

extern void write_job_aggr_isc(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrISCInfo> &, AsciiTable &);

extern void write_job_aggr_mpr(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrMPRInfo> &, AsciiTable &,
               const char *, gsl_rng *);

extern void write_job_aggr_mpr_wind(STATAnalysisJob &, STATLineType,
               std::map<ConcatString, AggrWindInfo> &, AsciiTable &);

extern void write_job_ramp(STATAnalysisJob &,
               std::map<ConcatString, AggrTimeSeriesInfo> &,
               AsciiTable &, AsciiTable &, AsciiTable &);

extern void write_job_ramp_cols(const STATAnalysisJob &, AsciiTable &,
               int &, int &);

extern void write_job_ss_index(STATAnalysisJob &,
               AggrSSIndexInfo &, AsciiTable &);

////////////////////////////////////////////////////////////////////////

extern void setup_table    (AsciiTable &, int, int);
extern void write_table    (AsciiTable &,  std::ofstream *);
extern void write_jobstring(const ConcatString &, std::ofstream *);
extern void write_line     (const ConcatString &, std::ofstream *);

////////////////////////////////////////////////////////////////////////

extern void compute_ss_index(LineDataFile &, STATAnalysisJob &,
               AggrSSIndexInfo &, int &, int &);

extern void write_case_cols(const ConcatString &, AsciiTable &,
               int &, int &);

////////////////////////////////////////////////////////////////////////

#endif   //  __STAT_ANALYSIS_JOB_H__

////////////////////////////////////////////////////////////////////////
