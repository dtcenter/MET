// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __COMPUTE_STATS_H__
#define  __COMPUTE_STATS_H__

////////////////////////////////////////////////////////////////////////

#include "met_stats.h"
#include "ens_stats.h"
#include "pair_data_point.h"
#include "pair_data_ensemble.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// Utility functions for computing statistics.
//
////////////////////////////////////////////////////////////////////////

extern void   compute_cntinfo(const SL1L2Info &, bool, CNTInfo &);

extern void   compute_cntinfo(const PairDataPoint &, const NumArray &,
                              bool, bool, bool, CNTInfo &);
extern void   compute_i_cntinfo(const PairDataPoint &, int,
                                bool, bool, bool, CNTInfo &);

extern void   compute_ctsinfo(const PairDataPoint &, const NumArray &,
                              bool, bool, CTSInfo &);
extern void   compute_i_ctsinfo(const PairDataPoint &, int,
                                bool, CTSInfo &);

extern void   compute_mctsinfo(const PairDataPoint &, const NumArray &,
                               bool, bool, MCTSInfo &);
extern void   compute_i_mctsinfo(const PairDataPoint &, int,
                                 bool, MCTSInfo &);

extern void   compute_pctinfo(const PairDataPoint &, bool, PCTInfo &,
                              const NumArray *cprob_in = 0);

extern void   compute_nbrcntinfo(const PairDataPoint &,
                                 const PairDataPoint &,
                                 const NumArray &,
                                 NBRCNTInfo &, bool);
extern void   compute_i_nbrcntinfo(const PairDataPoint &,
                                   const PairDataPoint &,
                                   int, NBRCNTInfo &);

extern void   compute_mean_stdev(const NumArray &, const NumArray &,
                                 bool, double,
                                 CIInfo &, CIInfo &);

extern void   compute_i_mean_stdev(const NumArray &,
                                   bool, double, int,
                                   CIInfo &, CIInfo &);

////////////////////////////////////////////////////////////////////////
//
// Compute means of statistics for climatological bins.
//
////////////////////////////////////////////////////////////////////////

extern void compute_sl1l2_mean(const SL1L2Info *, int, SL1L2Info &);
extern void compute_cnt_mean  (const CNTInfo *,   int, CNTInfo &);
extern void compute_pct_mean  (const PCTInfo *,   int, PCTInfo &);
extern void compute_ecnt_mean (const ECNTInfo *,  int, ECNTInfo &);

////////////////////////////////////////////////////////////////////////

#endif   // __COMPUTE_STATS_H__

////////////////////////////////////////////////////////////////////////
