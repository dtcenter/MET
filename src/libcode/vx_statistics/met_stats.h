// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __MET_STATS_H__
#define  __MET_STATS_H__

////////////////////////////////////////////////////////////////////////

#include "contable.h"

#include "vx_config.h"
#include "vx_util.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////
//
// Class to store a statistical value with its lower and upper
// confidence interval bounds.
//
////////////////////////////////////////////////////////////////////////

class CIInfo {

   private:
      void init_from_scratch();
      void assign(const CIInfo &);

   public:

      CIInfo();
      ~CIInfo();
      CIInfo(const CIInfo &);
      CIInfo & operator=(const CIInfo &);

      int n;

      // Value of the statistic
      double v;

      // Variance inflation factor for time series
      double vif;

      // Confidence interval computed using a normal approximation
      double *v_ncl;
      double *v_ncu;

      // Confidence interval computed using a bootstrap approach
      double *v_bcl;
      double *v_bcu;

      void clear();
      void set_bad_data();
      void allocate_n_alpha(int);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Contingency Table Counts and Statistics
//
////////////////////////////////////////////////////////////////////////

class CTSInfo {

   private:
      void init_from_scratch();
      void assign(const CTSInfo &);

   public:

      CTSInfo();
      ~CTSInfo();
      CTSInfo(const CTSInfo &);
      CTSInfo & operator=(const CTSInfo &);

      // Confidence interval alpha values
      int     n_alpha;
      double *alpha;

      TTContingencyTable cts;
      SingleThresh       fthresh;
      SingleThresh       othresh;

      CIInfo baser, fmean, acc, fbias;
      CIInfo pody, podn, pofd;
      CIInfo far, csi, gss, bagss, hk, hss, odds;
      CIInfo lodds, orss, eds, seds, edi, sedi;

      void clear();
      void allocate_n_alpha(int);
      void add(double, double);
      void compute_stats();
      void compute_ci();

      double get_stat(const char *);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Multi-Category Contingency Table Counts and Statistics
//
////////////////////////////////////////////////////////////////////////

class MCTSInfo {

   private:
      void init_from_scratch();
      void assign(const MCTSInfo &);

   public:

      MCTSInfo();
      ~MCTSInfo();
      MCTSInfo(const MCTSInfo &);
      MCTSInfo & operator=(const MCTSInfo &);

      // Confidence interval alpha values
      int     n_alpha;
      double *alpha;

      ContingencyTable cts;
      ThreshArray      fthresh;
      ThreshArray      othresh;

      CIInfo acc, hk, hss, ger;

      void clear();
      void allocate_n_alpha(int);
      void set_fthresh(const ThreshArray &);
      void set_othresh(const ThreshArray &);
      void add(double, double);
      void compute_stats();
      void compute_ci();
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Continuous Statistics
//
////////////////////////////////////////////////////////////////////////

class CNTInfo {

   private:
      void init_from_scratch();
      void assign(const CNTInfo &);

   public:

      CNTInfo();
      ~CNTInfo();
      CNTInfo(const CNTInfo &);
      CNTInfo & operator=(const CNTInfo &);

      // Filtering thresholds
      SingleThresh fthresh;
      SingleThresh othresh;
      SetLogic     logic;

      // Confidence interval alpha values
      int     n_alpha;
      double *alpha;

      // Number of points
      int n;

      CIInfo fbar, fstdev;
      CIInfo obar, ostdev;

      // Correlation Coefficients: Pearson's, Spearman's Rank,
      // and Kendall Tau Rank
      CIInfo pr_corr, sp_corr, kt_corr;

      // Anomaly correlation and RMS Anomalies
      CIInfo anom_corr, rmsfa, rmsoa;

      CIInfo me, me2, estdev, mbias, mae, mse, msess, bcmse, rmse;
      CIInfo e10, e25, e50, e75, e90, eiqr;
      CIInfo mad;

      int n_ranks, frank_ties, orank_ties;

      void clear();
      void allocate_n_alpha(int);
      void compute_ci();

      double get_stat(const char *);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Scalar L1L2 and Scalar Anomaly L1L2 Values
//
////////////////////////////////////////////////////////////////////////

class SL1L2Info {

   private:
      void init_from_scratch();
      void assign(const SL1L2Info &);

   public:

      SL1L2Info();
      ~SL1L2Info();
      SL1L2Info(const SL1L2Info &);
      SL1L2Info & operator=(const SL1L2Info &);
      SL1L2Info & operator+=(const SL1L2Info &);

      // Filtering thresholds
      SingleThresh fthresh;
      SingleThresh othresh;
      SetLogic     logic;

      // SL1L2 Quantities
      double fbar, obar;
      double fobar;
      double ffbar, oobar;
      int    scount;

      // SAL1L2 Quantities
      double fabar, oabar;
      double foabar;
      double ffabar, ooabar;
      int    sacount;

      // Mean absolute error
      double mae;

      // Compute sums
      void set(const NumArray &f_na, const NumArray &o_na,
               const NumArray &c_na, const NumArray &w_na);

      void zero_out();
      void clear();
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Vector L1L2 and Vector Anomaly L1L2 Values
//
////////////////////////////////////////////////////////////////////////

class VL1L2Info {

   private:
      void init_from_scratch();
      void assign(const VL1L2Info &);

   public:

      VL1L2Info();
      ~VL1L2Info();
      VL1L2Info(const VL1L2Info &);
      VL1L2Info & operator=(const VL1L2Info &);
      VL1L2Info & operator+=(const VL1L2Info &);

      void calc_ncep_stats();

         // Filtering thresholds

      SingleThresh fthresh;
      SingleThresh othresh;
      SetLogic     logic;

         // VL1L2 Quantities

      double uf_bar;
      double vf_bar;
      double uo_bar;
      double vo_bar;

      double uvfo_bar;
      double uvff_bar;
      double uvoo_bar;

         // New VL1L2 Quantities added from vector stats whitepaper

      double f_speed_bar;
      double o_speed_bar;

         // New VL1L2 Quantities added from vector stats whitepaper

      double FBAR;
      double OBAR;

      double FS_RMS;
      double OS_RMS;

      double  MSVE;
      double RMSVE;

      double FSTDEV;
      double OSTDEV;

      // double COV;

      double FDIR;
      double ODIR;

      double FBAR_SPEED;
      double OBAR_SPEED;

      double VDIFF_SPEED;
      double VDIFF_DIR;

      double SPEED_ERR;
      double SPEED_ABSERR;

      double DIR_ERR;
      double DIR_ABSERR;

         //
         //  extra VL1L2 quantities for NCEP
         //

      double f_bar;  //  fcst wind speed
      double o_bar;  //   obs wind speed

      double me;     //  mean error

      double mse;    //  mean squared error

      double rmse;   //  root mean squared error

      double speed_bias;

      int    vcount;

      // VAL1L2 Quantities
      double ufa_bar;
      double vfa_bar;
      double uoa_bar;
      double voa_bar;
      double uvfoa_bar;
      double uvffa_bar;
      double uvooa_bar;

      int    vacount;

      // Compute sums
      void set(const NumArray &uf_na, const NumArray &vf_na,
               const NumArray &uo_na, const NumArray &vo_na,
               const NumArray &uc_na, const NumArray &vc_na,
               const NumArray &w_na);

      void zero_out();
      void clear();

      double get_stat(const char *);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Neighborhood Methods Contingency Table Counts
// and Statistics
//
////////////////////////////////////////////////////////////////////////

class NBRCTSInfo {

   private:
      void init_from_scratch();
      void assign(const NBRCTSInfo &);

   public:

      NBRCTSInfo();
      ~NBRCTSInfo();
      NBRCTSInfo(const NBRCTSInfo &);
      NBRCTSInfo & operator=(const NBRCTSInfo &);

      // Neighborhood width used
      int nbr_wdth;

      // CTSInfo object
      CTSInfo cts_info;

      // Raw threshold applied to define the fractional coverage
      SingleThresh fthresh;
      SingleThresh othresh;

      // Fractional coverage threshold
      SingleThresh cthresh;

      void clear();
      void allocate_n_alpha(int);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Neighborhood Methods Continuous Statistics
//
////////////////////////////////////////////////////////////////////////

class NBRCNTInfo {

   private:
      void init_from_scratch();
      void assign(const NBRCNTInfo &);

   public:

      NBRCNTInfo();
      ~NBRCNTInfo();
      NBRCNTInfo(const NBRCNTInfo &);
      NBRCNTInfo & operator=(const NBRCNTInfo &);
      NBRCNTInfo & operator+=(const NBRCNTInfo &);

      // Neighborhood width used
      int nbr_wdth;

      // SL1L2 object
      SL1L2Info sl1l2_info;

      // Raw threshold applied to define the fractional coverage
      SingleThresh fthresh;
      SingleThresh othresh;

      // Confidence interval alpha values
      int     n_alpha;
      double *alpha;

      // Fractions Brier Score,
      // Fractions Skill Score,
      // Asymptotic Fractions Skill Score,
      // Uniform Fractions Skill Score,
      // Forecast Rate, and Observation Rate
      CIInfo fbs, fss, afss, ufss, f_rate, o_rate;

      void clear();
      void allocate_n_alpha(int);
      void compute_stats();
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Intensity Scale Statistics
//
////////////////////////////////////////////////////////////////////////

class ISCInfo {

   private:
      void init_from_scratch();
      void assign(const ISCInfo &);

   public:

      ISCInfo();
      ~ISCInfo();
      ISCInfo(const ISCInfo &);
      ISCInfo & operator=(const ISCInfo &);

      // ContingencyTable
      TTContingencyTable cts;
      SingleThresh       fthresh;
      SingleThresh       othresh;

      // Dimension of the tile
      int tile_dim;

      // Lower-left corner of the tile
      int tile_xll;
      int tile_yll;

      // Number of scales to be decomposed
      int n_scale;

      // Mean Squared Error for each scale (MSE)
      double  mse;
      double *mse_scale;

      // Intensity Scale Score for each scale (ISC)
      double  isc;
      double *isc_scale;

      // Engery in the forecast and observation fields for each scale
      double  fen;
      double *fen_scale;

      double  oen;
      double *oen_scale;

      // Base Rate and Frequency Bias from the contingency table
      int     total;
      double  baser;
      double  fbias;

      void clear();
      void zero_out();
      void allocate_n_scale(int);
      void compute_isc();
      void compute_isc(int);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Probability Contingency Table Counts and Statistics
//
////////////////////////////////////////////////////////////////////////

class PCTInfo {

   private:
      void init_from_scratch();
      void assign(const PCTInfo &);

   public:

      PCTInfo();
      ~PCTInfo();
      PCTInfo(const PCTInfo &);
      PCTInfo & operator=(const PCTInfo &);

      // Confidence interval alpha values
      int     n_alpha;
      double *alpha;

      Nx2ContingencyTable pct;
      Nx2ContingencyTable climo_pct;

      // Multiple thresholds for the probabilistic forecast
      ThreshArray fthresh;

      // Single threshold for the scalar observation
      SingleThresh othresh;

      CIInfo baser;
      CIInfo brier;
      CIInfo briercl; // Climatological brier score
      double bss;

      void clear();
      void allocate_n_alpha(int);
      void compute_stats();
      void compute_ci();
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Ensemble Spread/Skill Information
//
////////////////////////////////////////////////////////////////////////

class SSVARInfo {

   private:
      void init_from_scratch();
      void assign(const SSVARInfo &);

   public:

      SSVARInfo();
      ~SSVARInfo();
      SSVARInfo(const SSVARInfo &);
      SSVARInfo & operator=(const SSVARInfo &);
      SSVARInfo & operator+=(const SSVARInfo &);

      int n_bin;
      int bin_i;
      int bin_n;

      double var_min;
      double var_max;
      double var_mean;

      SL1L2Info sl1l2_info;

      void clear();
};

////////////////////////////////////////////////////////////////////////
//
// Class to store gradient statistics
//
////////////////////////////////////////////////////////////////////////

class GRADInfo {

   private:
      void init_from_scratch();
      void assign(const GRADInfo &);

   public:

      GRADInfo();
      ~GRADInfo();
      GRADInfo(const GRADInfo &);
      GRADInfo & operator=(const GRADInfo &);
      GRADInfo & operator+=(const GRADInfo &);

      // Gradient Size
      int dx;
      int dy;

      // Gradient Partial Sums
      int    total;
      double fgbar, ogbar, mgbar, egbar;

      // Gradient Statistics
      double s1()         const; // s1         = 100 * egbar / mgbar
      double s1_og()      const; // s1_og      = 100 * egbar / ogbar
      double fgog_ratio() const; // fgog_ratio = fgbar / ogbar

      // Compute sums
      void set(int grad_dx, int grad_dy,
               const NumArray &fgx_na, const NumArray &fgy_na,
               const NumArray &ogx_na, const NumArray &ogy_na,
               const NumArray &w_na);

      void clear();
};

////////////////////////////////////////////////////////////////////////
//
// Utility functions for parsing data from configuration files
//
////////////////////////////////////////////////////////////////////////

extern int    parse_message_type(const char *, char **&);
extern int    parse_dbl_list(const char *, double *&);
extern int    parse_int_list(const char *, int *&);

extern int    max_int(const int *, int);
extern int    min_int(const int *, int);

extern void   dbl_to_str(double, char *);
extern void   dbl_to_str(double, char *, int);

extern double compute_stdev(double, double, int);

extern double compute_corr(double, double, double, double, double, int);

extern double compute_afss(double, double);

extern double compute_ufss(double);

extern int    compute_rank(const DataPlane &, DataPlane &, double *, int &);

extern void   compute_cntinfo(const SL1L2Info &, int, CNTInfo &);

extern void   compute_cntinfo(const NumArray &, const NumArray &,
                              const NumArray &, const NumArray &,
                              const NumArray &,
                              int, int, int, CNTInfo &);
extern void   compute_i_cntinfo(const NumArray &, const NumArray &,
                                const NumArray &, const NumArray &,
                                int, int, int, int, CNTInfo &);

extern void   compute_ctsinfo(const NumArray &, const NumArray &,
                              const NumArray &,
                              int, int, CTSInfo &);
extern void   compute_i_ctsinfo(const NumArray &, const NumArray &,
                                int, int, CTSInfo &);

extern void   compute_mctsinfo(const NumArray &, const NumArray &,
                               const NumArray &, int, int, MCTSInfo &);
extern void   compute_i_mctsinfo(const NumArray &, const NumArray &,
                                 int, int, MCTSInfo &);

extern void   compute_pctinfo(const NumArray &, const NumArray &,
                              const NumArray &, int, PCTInfo &);

extern void   compute_nbrcntinfo(const NumArray &, const NumArray &,
                                 const NumArray &, const NumArray &,
                                 const NumArray &, const NumArray &,
                                 NBRCNTInfo &, int);
extern void   compute_i_nbrcntinfo(const NumArray &, const NumArray &,
                                   const NumArray &, const NumArray &,
                                   const NumArray &,
                                   int, NBRCNTInfo &);

extern void   compute_mean_stdev(const NumArray &, const NumArray &,
                                 int, double,
                                 CIInfo &, CIInfo &);
extern void   compute_i_mean_stdev(const NumArray &,
                                   int, double, int,
                                   CIInfo &, CIInfo &);

////////////////////////////////////////////////////////////////////////

#endif   // __MET_STATS_H__

////////////////////////////////////////////////////////////////////////
