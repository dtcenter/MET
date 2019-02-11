// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __COMPUTE_CI_H__
#define  __COMPUTE_CI_H__

////////////////////////////////////////////////////////////////////////

#include "vx_gsl_prob.h"
#include "vx_util.h"
#include "met_stats.h"

////////////////////////////////////////////////////////////////////////

static const int large_sample_threshold = 30;
static const int wald_sample_threshold  = 100;

////////////////////////////////////////////////////////////////////////

extern void compute_normal_ci(double x, double alpha, double se,
   double &cl, double &cu);

extern void compute_proportion_ci(double p, int n, double alpha,
   double vif, double &p_cl, double &p_cu);

extern void compute_wald_ci(double p, int n, double alpha,
   double vif, double &p_cl, double &p_cu);

extern void compute_wilson_ci(double p, int n, double alpha,
   double vif, double &p_cl, double &p_cu);

extern void compute_woolf_ci(double odds, double alpha,
   int fy_oy, int fy_on, int fn_oy, int fn_on,
   double &odds_cl, double &odds_cu);

extern void compute_hk_ci(double hk, double alpha, double vif,
   int fy_oy, int fy_on, int fn_oy, int fn_on,
   double &hk_cl, double &hk_cu);

extern void compute_cts_stats_ci_bca(const gsl_rng *,
   const NumArray &, const NumArray &,
   int, CTSInfo *&, int, int, int, const char *);

extern void compute_mcts_stats_ci_bca(const gsl_rng *,
   const NumArray &, const NumArray &,
   int, MCTSInfo &, int, int, const char *);

extern void compute_cnt_stats_ci_bca(const gsl_rng *,
   const NumArray &, const NumArray &,
   const NumArray &, const NumArray &,
   int, int, int, CNTInfo &, const char *);

extern void compute_cts_stats_ci_perc(const gsl_rng *,
   const NumArray &, const NumArray &,
   int, double, CTSInfo *&, int, int, int, const char *);

extern void compute_mcts_stats_ci_perc(const gsl_rng *,
   const NumArray &, const NumArray &,
   int, double, MCTSInfo &, int, int, const char *);

extern void compute_cnt_stats_ci_perc(const gsl_rng *,
   const NumArray &, const NumArray &,
   const NumArray &, const NumArray &,
   int, int, int, double, CNTInfo &, const char *);

extern void compute_nbrcts_stats_ci_bca(const gsl_rng *,
   const NumArray &, const NumArray &,
   int, NBRCTSInfo *&, int, int, const char *);

extern void compute_nbrcnt_stats_ci_bca(const gsl_rng *,
   const NumArray &, const NumArray &,
   const NumArray &, const NumArray &, const NumArray &,
   int, NBRCNTInfo &, int, const char *);

extern void compute_nbrcts_stats_ci_perc(const gsl_rng *,
   const NumArray &, const NumArray &,
   int, double, NBRCTSInfo *&, int, int, const char *);

extern void compute_nbrcnt_stats_ci_perc(const gsl_rng *,
   const NumArray &, const NumArray &,
   const NumArray &, const NumArray &, const NumArray &,
   int, double, NBRCNTInfo &, int, const char *);

extern void compute_mean_stdev_ci_bca(const gsl_rng *, const NumArray &,
   int, double, CIInfo &, CIInfo &);

extern void compute_mean_stdev_ci_perc(const gsl_rng *, const NumArray &,
   int, double, double, CIInfo &, CIInfo &);

extern void compute_bca_interval(double, NumArray &, NumArray &,
   double, double &, double &);

extern void compute_perc_interval(double, NumArray &,
   double, double &, double &);

////////////////////////////////////////////////////////////////////////

#endif   // __COMPUTE_CI_H__

////////////////////////////////////////////////////////////////////////
