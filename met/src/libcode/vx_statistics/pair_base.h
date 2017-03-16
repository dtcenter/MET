// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __PAIR_BASE_H__
#define  __PAIR_BASE_H__

////////////////////////////////////////////////////////////////////////

#include <map>
#include <utility>

#include "vx_util.h"
#include "vx_data2d.h"

struct ob_val_t {
  int ut;
  double val;
  string qc;
};

struct station_values_t {
  string sid;
  int ut;
  double cmn;
  double csd;
  double wgt;
  int x;
  int y;
  vector<ob_val_t> obs;
};

enum obs_summary_enum {
  OBS_SUMMARY_NONE,
  OBS_SUMMARY_NEAREST,
  OBS_SUMMARY_MIN,
  OBS_SUMMARY_MAX,
  OBS_SUMMARY_UWMEAN,
  OBS_SUMMARY_DWMEAN,
  OBS_SUMMARY_MEDIAN,
  OBS_SUMMARY_PERC
};

static bool sort_obs(ob_val_t a, ob_val_t b) { return a.val<b.val; }

////////////////////////////////////////////////////////////////////////
//
// Base class for matched pair data
//
////////////////////////////////////////////////////////////////////////

class PairBase {

   private:

      void init_from_scratch();

   public:

      PairBase();
      virtual ~PairBase();

      //////////////////////////////////////////////////////////////////

      // Masking area applied to the forecast and climo fields
      ConcatString  mask_name;
      DataPlane     *mask_dp_ptr;  // Pointer to the masking data plane
                                   // which is not allocated
      StringArray   *mask_sid_ptr; // Pointer to masking station ID list
                                   // which is not allocated

      // The verifying message type
      ConcatString msg_typ;

      // Interpolation method and width used
      InterpMthd interp_mthd;
      int        interp_dpth;

      // Observation Information
      StringArray sid_sa;  // Station ID [n_obs]
      NumArray    lat_na;  // Latitude [n_obs]
      NumArray    lon_na;  // Longitude [n_obs]
      NumArray    x_na;    // X [n_obs]
      NumArray    y_na;    // Y [n_obs]
      NumArray    wgt_na;  // Weight [n_obs]
      TimeArray   vld_ta;  // Valid time [n_obs]
      NumArray    lvl_na;  // Level [n_obs]
      NumArray    elv_na;  // Elevation [n_obs]
      NumArray    o_na;    // Observation value [n_obs]
      StringArray o_qc_sa; // Observation quality control [n_obs]
      int         n_obs;   // Number of observations

      // Climatology Information
      NumArray    cmn_na;  // Climatology mean [n_obs]
      NumArray    csd_na;  // Climatology standard deviation [n_obs]

      unixtime    fcst_ut; // Forecast valid time

      bool check_unique;   // Check for duplicates, keeping unique obs
      obs_summary_enum obs_select; // Summarize multiple observations
      int obs_perc_value; // percentile value to use in PERC obs_select
      map<string,station_values_t> map_val;  // Storage for single obs values      

      //////////////////////////////////////////////////////////////////

      void clear();

      void extend(int);    // Allocate memory for expected size

      void set_mask_name(const char *);
      void set_mask_dp_ptr(DataPlane *);
      void set_mask_sid_ptr(StringArray *);
      void set_msg_typ(const char *);

      void set_interp_mthd(const char *);
      void set_interp_mthd(InterpMthd);
      void set_interp_dpth(int);

      void set_fcst_ut(unixtime ut);
      void set_check_unique(bool check);
      void set_obs_summary(obs_summary_enum o);
      void set_obs_perc_value(int i);      

      int  has_obs_rec(const char *, double, double, double, double,
                       double, double, int &);

      ob_val_t compute_nearest(string sng_key);
      ob_val_t compute_min(string sng_key);
      ob_val_t compute_max(string sng_key);
      ob_val_t compute_uwmean(string sng_key);
      ob_val_t compute_dwmean(string sng_key);
      ob_val_t compute_median(string sng_key);
      ob_val_t compute_percentile(string sng_key, int perc);
      
      bool add_obs(const char *, double, double, double, double,
                   unixtime, double, double, double, const char *,
                   double, double,
                   double wgt = default_grid_weight);

      void add_obs(double, double, double, double, double,
                   double wgt = default_grid_weight);

      void set_obs(int, const char *, double, double, double, double,
                   unixtime, double, double, double,
                   const char *, double, double,
                   double wgt = default_grid_weight);

      void set_obs(int, double, double, double, double, double,
                   double wgt = default_grid_weight);

      void calc_obs_summary();

      void print_duplicate_report();
};

////////////////////////////////////////////////////////////////////////
//
// Miscellanous utility functions
//
////////////////////////////////////////////////////////////////////////

extern void find_vert_lvl(const DataPlaneArray &, const double,
                          int &, int &);

extern double compute_interp(const DataPlaneArray &,
                             const double, const double, const double,
                             const InterpMthd, const int, const double,
                             const bool, const LevelType,
                             const double, const int, const int,
                             const SingleThresh *cat_thresh = 0);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_BASE_H__

////////////////////////////////////////////////////////////////////////
