// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   parse_stat_line.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add parse_rhist_line and
//                    parse_orank_line.
//   002    06/09/10  Halley Gotway   Add parse_mctc_ctable.
//   003    03/07/13  Halley Gotway   Add parse_ssvar_line.
//   004    05/19/14  Halley Gotway   Add OBS_QC to MPR and ORANK lines.
//   005    06/03/14  Halley Gotway   Add PHIST line type.
//   006    06/09/17  Halley Gotway   Add RELP line type.
//   008    10/09/17  Halley Gotway   Add GRAD line type.
//   009    04/25/18  Halley Gotway   Add ECNT line type.
//   010    01/24/20  Halley Gotway   Add RPS line type.
//
////////////////////////////////////////////////////////////////////////

#ifndef  __PARSE_STAT_LINE_H__
#define  __PARSE_STAT_LINE_H__

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
#include "vx_util.h"
#include "vx_statistics.h"

////////////////////////////////////////////////////////////////////////

// Matched Pair (MPR) data structure
struct MPRData {
   ConcatString fcst_var;
   ConcatString obs_var;
   int total, index;
   ConcatString obs_sid, obs_qc;
   double obs_lat, obs_lon, obs_lvl, obs_elv;
   double fcst, obs, climo_mean, climo_stdev, climo_cdf;
};

// Ensemble continuous statistics (ECNT) data structure
struct ECNTData {
   int total, n_ens;
   double crps, crpss, ign;
   double me, rmse, spread;
   double me_oerr, rmse_oerr, spread_oerr;
   double spread_plus_oerr;
};

// Ranked Histogram (RHIST) data structure
struct RHISTData {
   int total, n_rank;
   NumArray rhist_na;
};

// Probability Integral Transform Histogram (PHIST) data structure
struct PHISTData {
   int total, n_bin;
   double bin_size;
   NumArray phist_na;
};

// Relative Position (RELP) data structure
struct RELPData {
   int total, n_ens;
   NumArray relp_na;
};

// Observation Rank (ORANK) data structure
struct ORANKData {
   int total, index;
   ConcatString obs_sid, obs_qc;
   double obs_lat, obs_lon, obs_lvl, obs_elv;
   double obs, pit, climo;
   double ens_mean, spread, ens_mean_oerr, spread_oerr;
   double spread_plus_oerr;
   int rank, n_ens_vld, n_ens;
   NumArray ens_na;
};

////////////////////////////////////////////////////////////////////////

extern void parse_fho_ctable   (STATLine &, TTContingencyTable &);
extern void parse_ctc_ctable   (STATLine &, TTContingencyTable &);
extern void parse_mctc_ctable  (STATLine &, ContingencyTable &);
extern void parse_nbrctc_ctable(STATLine &, TTContingencyTable &);
extern void parse_nx2_ctable   (STATLine &, Nx2ContingencyTable &);

extern void parse_sl1l2_line   (STATLine &, SL1L2Info &);
extern void parse_sal1l2_line  (STATLine &, SL1L2Info &);
extern void parse_vl1l2_line   (STATLine &, VL1L2Info &);
extern void parse_val1l2_line  (STATLine &, VL1L2Info &);

extern void parse_mpr_line     (STATLine &, MPRData &);
extern void parse_isc_line     (STATLine &, ISCInfo &, int &);
extern void parse_nbrcnt_line  (STATLine &, NBRCNTInfo &);
extern void parse_grad_line    (STATLine &, GRADInfo &);

extern void parse_ecnt_line    (STATLine &, ECNTData &);
extern void parse_rps_line     (STATLine &, RPSInfo &);
extern void parse_rhist_line   (STATLine &, RHISTData &);
extern void parse_phist_line   (STATLine &, PHISTData &);
extern void parse_relp_line    (STATLine &, RELPData &);
extern void parse_orank_line   (STATLine &, ORANKData &);
extern void parse_ssvar_line   (STATLine &, SSVARInfo &);

////////////////////////////////////////////////////////////////////////

#endif   //  __PARSE_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////
