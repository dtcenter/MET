// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __VX_MET_STATS_H__
#define  __VX_MET_STATS_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util/vx_util.h"
#include "vx_data_grids/grid.h"
#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_contable/vx_contable.h"

////////////////////////////////////////////////////////////////////////

static const int pair_data_alloc_jump = 1000;

// Exponent used in distance weighted mean calculations
static const int dw_mean_pow = 2;

////////////////////////////////////////////////////////////////////////
//
// Class to store Grib Code Information
//
////////////////////////////////////////////////////////////////////////

//
// Enumeration for level types
//
enum LevelType {

    NoLevel    = 0,
    AccumLevel = 1,
    VertLevel  = 2,
    PresLevel  = 3,
    RecNumber  = 4
};

class GCInfo {

   private:
      void init_from_scratch();
      void assign(const GCInfo &);

   public:

      GCInfo();
      ~GCInfo();
      GCInfo(const GCInfo &);
      GCInfo & operator=(const GCInfo &);
      int operator==(const GCInfo &);

      //////////////////////////////////////////////////////////////////

      ConcatString abbr_str; // GRIB code abbreviation string
      ConcatString lvl_str;  // Level defined in config file
      ConcatString info_str; // abbr_str/lvl_str

      int          code;     // GRIB code itself

      LevelType    lvl_type; // Type of level specified
      int          lvl_1;    // First level defined
      int          lvl_2;    // Second level in case of a range

      // Flag to indicate whether this field should be used in
      // vector calculations
      int          vflag;

      // Flag to indicate whether this is a probability field
      int          pflag;

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_gcinfo(const char *, int);

      void set_abbr_str(const char *);
      void set_lvl_str(const char *);
      void set_info_str(const char *);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Fcst-Climo-Obs pair data
//
////////////////////////////////////////////////////////////////////////

class PairData {

   private:

      void init_from_scratch();
      void assign(const PairData &);

   public:

      PairData();
      ~PairData();
      PairData(const PairData &);
      PairData & operator=(const PairData &);

      //////////////////////////////////////////////////////////////////

      // Masking area applied to the forecast and climo fields
      ConcatString mask_name;
      WrfData     *mask_wd_ptr; // Pointer to the masking field
                                // which is not allocated

      // The verifying message type
      ConcatString msg_typ;

      // Interpolation method and width used
      InterpMthd interp_mthd;
      int        interp_wdth;

      // Forecast, climotological, and observation data pairs
      NumArray f_na;
      NumArray c_na;
      NumArray o_na;

      // Observation latitude, longitude, pressure, and elevation
      NumArray lat_na;
      NumArray lon_na;
      NumArray lvl_na;
      NumArray elv_na;
      int      n_pair;

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_mask_name(const char *);
      void set_mask_wd_ptr(WrfData *);

      void set_msg_typ(const char *);

      void set_interp_mthd(const char *);
      void set_interp_mthd(InterpMthd);
      void set_interp_wdth(int);

      void add_pair(double, double, double, double,
                    double, double, double);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store a variety of PairData objects for each Grib Code
//
////////////////////////////////////////////////////////////////////////

class GCPairData {

   private:

      void init_from_scratch();
      void assign(const GCPairData &);

   public:

      GCPairData();
      ~GCPairData();
      GCPairData(const GCPairData &);
      GCPairData & operator=(const GCPairData &);

      //////////////////////////////////////////////////////////////////
      //
      // The grib code information to be verified
      //
      //////////////////////////////////////////////////////////////////

      GCInfo fcst_gci;         // Forecast field
      GCInfo obs_gci;          // Observation field

      double interp_thresh;    // Threshold between 0 and 1 used when
                               // interpolating the forecasts to the
                               // observation location.

      //////////////////////////////////////////////////////////////////
      //
      // Forecast and climatological fields falling between the
      // requested levels.  Store the number of fields, the pressure
      // level corresponding to each field, and pointers to the data.
      //
      //////////////////////////////////////////////////////////////////

      int      n_fcst;         // Number of fcst fields
      NumArray fcst_lvl;       // Array of vertical levels
      WrfData  **fcst_wd_ptr;  // Array of pointers to the fcst data

      int      n_climo;        // Number of climo fields
      NumArray climo_lvl;      // Array of vertical levels
      WrfData  **climo_wd_ptr; // Array of pointers to the climo data

      //////////////////////////////////////////////////////////////////

      unixtime beg_ut;         // Beginning of valid time window
      unixtime end_ut;         // End of valid time window

      //////////////////////////////////////////////////////////////////

      int      n_msg_typ;      // Number of verifying message types

      int      n_mask;         // Total number of masking regions
                               // of masking WrfData fields or SIDs

      int      n_interp;       // Number of interpolation techniques

      //////////////////////////////////////////////////////////////////

      PairData ***pd;          // 3-Dim Array of PairData objects
                               // as [n_msg_typ][n_mask][n_interp]

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_fcst_gci(const GCInfo &);
      void set_obs_gci(const GCInfo &);
      void set_interp_thresh(double);

      void set_n_fcst(int);
      void set_fcst_lvl(int, double);
      void set_fcst_wd_ptr(int, WrfData *);

      void set_n_climo(int);
      void set_climo_lvl(int, double);
      void set_climo_wd_ptr(int, WrfData *);

      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      // Should call set_pd_size prior to setting up the msg_typ, mask
      // or interpolation stuff
      void set_pd_size(int, int, int);

      void set_msg_typ(int, const char *);

      void set_mask_wd(int, const char *, WrfData *);

      void set_interp(int, const char *, int);
      void set_interp(int, InterpMthd, int);

      void add_obs(float *, char *, char *, unixtime, float *, Grid &);
      void find_vert_lvl(int, double, int &, int &);

      int  get_n_pair();

      double compute_interp(int, double, double, int, double, int, int);
      double compute_horz_interp(WrfData *, double, double, int, int);
      double compute_vert_pinterp(double, double, double, double, double);
      double compute_vert_zinterp(double, double, double, double, double);
};

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

      double v;

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
      SingleThresh       cts_fcst_thresh;
      SingleThresh       cts_obs_thresh;

      CIInfo baser, fmean, acc, fbias;
      CIInfo pody, podn, pofd;
      CIInfo far, csi, gss, hk, hss, odds;

      void clear();
      void allocate_n_alpha(int);
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

      CIInfo me, estdev, mbias, mae, mse, bcmse, rmse;
      CIInfo e10, e25, e50, e75, e90;

      int n_ranks, frank_ties, orank_ties;

      double fobar, ffbar, oobar;

      void clear();
      void allocate_n_alpha(int);
      void compute_ci();
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

      // Wind speed thresholds
      SingleThresh wind_fcst_thresh;
      SingleThresh wind_obs_thresh;

      // VL1L2 Quantities
      double ufbar, vfbar, uobar, vobar;
      double uvfobar;
      double uvffbar, uvoobar;
      int    vcount;

      // VAL1L2 Quantities
      double ufabar, vfabar, uoabar, voabar;
      double uvfoabar;
      double uvffabar, uvooabar;
      int    vacount;

      void zero_out();
      void clear();
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

      // Fraction threshold applied
      SingleThresh raw_fcst_thresh;
      SingleThresh raw_obs_thresh;
      SingleThresh frac_thresh;

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

      // Neighborhood width used
      int nbr_wdth;

      // CNTInfo object
      CNTInfo cnt_info;

      // Raw threshold applied to define the fractions
      SingleThresh raw_fcst_thresh;
      SingleThresh raw_obs_thresh;

      // Fractions Brier Score
      CIInfo fbs;

      // Fractions Skill Score
      CIInfo fss;

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
      SingleThresh       cts_thresh;

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

      // Multiple thresholds for the probabilistic forecast
      ThreshArray         pct_fcst_thresh;

      // Single threshold for the scalar observation
      SingleThresh        pct_obs_thresh;

      CIInfo brier;

      void clear();
      void allocate_n_alpha(int);
      void compute_stats();
      void compute_ci();
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

extern int    compute_rank(const WrfData &, WrfData &, double *, int &);

extern void   compute_cntinfo(const SL1L2Info &, int, CNTInfo &);

extern void   compute_cntinfo(const NumArray &, const NumArray &,
                              const NumArray &,
                              int, int, int, int, int,
                              CNTInfo &);
extern void   compute_i_cntinfo(const NumArray &, const NumArray &,
                                int, int, int, int, int,
                                CNTInfo &);

extern void   compute_ctsinfo(const NumArray &, const NumArray &,
                              const NumArray &, int, int, CTSInfo &);
extern void   compute_i_ctsinfo(const NumArray &, const NumArray &,
                                int, int, CTSInfo &);

extern void   compute_pctinfo(const NumArray &, const NumArray &,
                              int, PCTInfo &);

extern void   compute_nbrcntinfo(const NumArray &, const NumArray &,
                                 const NumArray &,
                                 NBRCNTInfo &, int);
extern void   compute_i_nbrcntinfo(const NumArray &, const NumArray &,
                                   int, NBRCNTInfo &);

extern void   compute_mean_stdev(const NumArray &, const NumArray &,
                                 int, double,
                                 CIInfo &, CIInfo &);
extern void   compute_i_mean_stdev(const NumArray &,
                                   int, double, int,
                                   CIInfo &, CIInfo &);

extern int    is_precip_code(int);

////////////////////////////////////////////////////////////////////////

#endif   // __VX_MET_STATS_H__

////////////////////////////////////////////////////////////////////////
