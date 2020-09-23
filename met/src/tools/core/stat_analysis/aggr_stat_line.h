// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   aggr_stat_line.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add aggr_rhist_lines and
//                    aggr_orank_lines.
//   002    03/07/13  Halley Gotway   Add aggregate SSVAR lines.
//   003    06/03/14  Halley Gotway   Add aggregate PHIST lines.
//   004    07/28/14  Halley Gotway   Add aggregate_stat for MPR to WDIR.
//   005    02/05/15  Halley Gotway   Add StatHdrInfo to keep track of
//                    unique header entries for each aggregation.
//   006    03/30/15  Halley Gotway   Add ramp job type.
//   007    06/09/17  Halley Gotway   Add aggregate RELP lines.
//   008    06/09/17  Halley Gotway   Add aggregate GRAD lines.
//   009    03/01/18  Halley Gotway   Update summary job type.
//   010    04/25/18  Halley Gotway   Add ECNT line type.
//   011    04/01/19  Fillmore        Add FCST and OBS units.
//   012    01/24/20  Halley Gotway   Add aggregate RPS lines.
//
////////////////////////////////////////////////////////////////////////

#ifndef  __AGGR_STAT_LINE_H__
#define  __AGGR_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_analysis_util.h"
#include "vx_gsl_prob.h"
#include "vx_util.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"
#include "ens_stats.h"

////////////////////////////////////////////////////////////////////////

static const int min_time_series = 10;

////////////////////////////////////////////////////////////////////////

struct StatHdrInfo {
   StringArray model, desc;
   StringArray fcst_var, fcst_units, fcst_lev;
   StringArray obs_var, obs_units, obs_lev;
   StringArray obtype, vx_mask, interp_mthd;
   StringArray fcst_thresh, obs_thresh, cov_thresh;
   NumArray fcst_lead, obs_lead, interp_pnts, alpha;
   unixtime fcst_valid_beg, fcst_valid_end;
   unixtime obs_valid_beg, obs_valid_end;

   StatHdrInfo();

   void clear();
   void add(const STATLine &line);
   void check_shc(const ConcatString &cur_case);
   StatHdrColumns   get_shc(const ConcatString &cur_case,
                            const StringArray  &case_cols,
                            const StringArray  &hdr_cols,
                            const StringArray  &hdr_vals,
                            const STATLineType lt);
   ConcatString get_shc_str(const ConcatString &cur_case,
                            const StringArray  &case_cols,
                            const StringArray  &case_vals,
                            const StringArray  &hdr_cols,
                            const StringArray  &hdr_vals,
                            const char         *col_name,
                            const StringArray  &col_vals,
                            bool               warning);
};

struct AggrSummaryInfo {
   StatHdrInfo hdr;
   map<ConcatString, NumArray> val;
   map<ConcatString, NumArray> wgt;
};

struct AggrCTCInfo {
   StatHdrInfo hdr;
   CTSInfo cts_info;
   NumArray valid_ts, baser_ts, fmean_ts, acc_ts;
   NumArray pody_ts, podn_ts, pofd_ts, far_ts, csi_ts, hk_ts;
};

struct AggrMCTCInfo {
   StatHdrInfo hdr;
   MCTSInfo mcts_info;
   NumArray valid_ts, acc_ts;
};

struct AggrPCTInfo {
   StatHdrInfo hdr;
   PCTInfo pct_info;
   NumArray valid_ts, baser_ts, brier_ts;
};

struct AggrPSumInfo {
   StatHdrInfo hdr;
   SL1L2Info  sl1l2_info;
   VL1L2Info  vl1l2_info;
   CNTInfo    cnt_info;
   NBRCNTInfo nbrcnt_info;
   NumArray valid_ts, fbar_ts, obar_ts, me_ts;
};

struct AggrGRADInfo {
   StatHdrInfo hdr;
   GRADInfo grad_info;
};

struct AggrWindInfo {
   StatHdrInfo hdr;
   VL1L2Info vl1l2_info;
   StringArray hdr_sa;
   PairDataPoint pd_u, pd_v;
};

struct AggrMPRInfo {
   StatHdrInfo hdr;
   ConcatString fcst_var, obs_var;
   PairDataPoint pd;
};

struct AggrISCInfo {
   StatHdrInfo hdr;
   ISCInfo isc_info;
   NumArray *total_na, *mse_na, *fen_na, *oen_na, *baser_na, *fbias_na;
};

struct AggrENSInfo {
   StatHdrInfo hdr;
   PairDataEnsemble ens_pd;
   NumArray crps_climo_na;
   NumArray me_na, mse_na, me_oerr_na, mse_oerr_na;
};

struct AggrRPSInfo {
   StatHdrInfo hdr;
   RPSInfo rps_info;
};

// Define struct used to perform comparisons for SSVAR bins
struct ssvar_bin_cmp {
  bool operator()(const ConcatString & cs1, const ConcatString & cs2) const {

    // Check for string equality
    if( cs1 == cs2) return(0);

    // Otherwise, parse list of numbers and compare each element
    StringArray sa1 = cs1.split(":");
    StringArray sa2 = cs2.split(":");
    for(int i=0; i<min(sa1.n_elements(), sa2.n_elements()); i++) {
       if(!is_eq(atof(sa1[i].c_str()), atof(sa2[i].c_str()))) {
          return(atof(sa1[i].c_str()) < atof(sa2[i].c_str()));
       }
    }
    return(-1);
  }
};

struct AggrSSVARInfo {
   StatHdrInfo hdr;
   map<ConcatString, SSVARInfo, ssvar_bin_cmp> ssvar_bins;
};

struct AggrTimeSeriesInfo {
   StatHdrInfo hdr;
   ConcatString fcst_var, obs_var;
   NumArray f_na, o_na;
   TimeArray init_ts, valid_ts;

   AggrTimeSeriesInfo();
   void clear();
   void sort();
};

////////////////////////////////////////////////////////////////////////

extern void aggr_summary_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrSummaryInfo> &,
               int &, int &);

extern void aggr_ctc_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrCTCInfo> &,
               int &, int &);

extern void aggr_mctc_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrMCTCInfo> &,
               int &, int &);

extern void aggr_pct_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrPCTInfo> &,
               int &, int &);

extern void aggr_psum_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrPSumInfo> &,
               int &, int &);

extern void aggr_grad_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrGRADInfo> &,
               int &, int &);

extern void aggr_wind_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrWindInfo> &,
               int &, int &);

extern void aggr_mpr_wind_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrWindInfo> &,
               int &, int &);

extern void aggr_mpr_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrMPRInfo> &,
               int &, int &);

extern void aggr_isc_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrISCInfo> &,
               int &, int &);

extern void aggr_ecnt_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrENSInfo> &,
               int &, int &);

extern void aggr_rps_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrRPSInfo> &,
               int &, int &);

extern void aggr_rhist_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrENSInfo> &,
               int &, int &);

extern void aggr_phist_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrENSInfo> &,
               int &, int &);

extern void aggr_relp_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrENSInfo> &,
               int &, int &);

extern void aggr_orank_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrENSInfo> &,
               int &, int &);

extern void aggr_ssvar_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrSSVARInfo> &,
               int &, int &);

extern void aggr_time_series_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrTimeSeriesInfo> &,
               int &, int &);

////////////////////////////////////////////////////////////////////////

extern void mpr_to_ctc(
               STATAnalysisJob &, const AggrMPRInfo &,
               CTSInfo &);

extern void mpr_to_cts(
               STATAnalysisJob &, const AggrMPRInfo &,
               CTSInfo &, const char *, gsl_rng *);

extern void mpr_to_mctc(
               STATAnalysisJob &, const AggrMPRInfo &,
               MCTSInfo &);

extern void mpr_to_mcts(
               STATAnalysisJob &, const AggrMPRInfo &,
               MCTSInfo &, const char *, gsl_rng *);

extern void mpr_to_cnt(
               STATAnalysisJob &, const AggrMPRInfo &,
               CNTInfo &, const char *, gsl_rng *);

extern void mpr_to_psum(STATAnalysisJob &, const AggrMPRInfo &,
               SL1L2Info &);

extern void mpr_to_pct(
               STATAnalysisJob &, const AggrMPRInfo &,
               PCTInfo &);

////////////////////////////////////////////////////////////////////////

extern double compute_vif(NumArray &);

////////////////////////////////////////////////////////////////////////

#endif   //  __AGGR_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////
