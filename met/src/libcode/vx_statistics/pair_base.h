// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "GridTemplate.h"

#include "vx_util.h"
#include "vx_data2d.h"

struct ob_val_t {
  unixtime ut;
  double val;
  string qc;
};

struct station_values_t {
  string sid;
  double lat;
  double lon;
  double x;
  double y;
  double wgt;
  unixtime ut;
  double lvl;
  double elv;
  double cmn;
  double csd;
  double summary_val;
  vector<ob_val_t> obs;
};

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
      MaskPlane     *mask_area_ptr;  // Pointer to the masking MaskPlane
                                     // which is not allocated
      StringArray   *mask_sid_ptr;   // Pointer to masking station ID list
                                     // which is not allocated
      MaskLatLon    *mask_llpnt_ptr; // Pointer to Lat/Lon thresholds
                                     // which is not allocated

      ConcatString msg_typ;          // Name of the verifying message type
      StringArray  msg_typ_vals;     // Message type values to be included

      // Interpolation method and shape used
      InterpMthd interp_mthd;
      int        interp_wdth;
      GridTemplateFactory::GridTemplates interp_shape;

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
      NumArray    cdf_na;  // Climatology cumulative distribution function [n_obs]

      unixtime    fcst_ut; // Forecast valid time

      bool       check_unique;   // Check for duplicates, keeping unique obs
      ObsSummary obs_summary;    // Summarize multiple observations
      int        obs_perc_value; // Percentile value for ObsSummary_Perc

      StringArray map_key;
      map<string,station_values_t> map_val; // Storage for single obs values

      //////////////////////////////////////////////////////////////////

      void clear();

      void extend(int);    // Allocate memory for expected size

      void set_mask_name(const char *);
      void set_mask_area_ptr(MaskPlane *);
      void set_mask_sid_ptr(StringArray *);
      void set_mask_llpnt_ptr(MaskLatLon *);

      void set_msg_typ(const char *);
      void set_msg_typ_vals(const StringArray &);

      void set_interp_mthd(const char *);
      void set_interp_mthd(InterpMthd);
      void set_interp_wdth(int);
      void set_interp_shape(GridTemplateFactory::GridTemplates);

      void set_fcst_ut(unixtime ut);
      void set_check_unique(bool check);
      void set_obs_summary(ObsSummary s);
      void set_obs_perc_value(int i);

      int  has_obs_rec(const char *, double, double, double, double,
                       double, double, int &);

      ob_val_t compute_nearest(string sng_key);
      ob_val_t compute_min(string sng_key);
      ob_val_t compute_max(string sng_key);
      ob_val_t compute_uw_mean(string sng_key);
      ob_val_t compute_dw_mean(string sng_key);
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

      void add_climo(double, double, double);
      void set_climo(int, double, double, double);

      double process_obs(VarInfo *, double);

      void print_obs_summary();

      void calc_obs_summary();

};

////////////////////////////////////////////////////////////////////////
//
// Miscellanous utility functions
//
////////////////////////////////////////////////////////////////////////

extern void find_vert_lvl(const DataPlaneArray &, const double,
                          int &, int &);

extern double compute_interp(const DataPlaneArray &dpa,
                      const double obs_x, const double obs_y, const double obs_v,
                      const InterpMthd method, const int width,
                      const GridTemplateFactory::GridTemplates shape,
                      const double thresh,
                      const bool spfh_flag, const LevelType lvl_typ,
                      const double to_lvl, const int i_blw, const int i_abv,
                      const SingleThresh *cat_thresh = 0);

extern void get_interp_points(const DataPlaneArray &dpa,
                      const double obs_x, const double obs_y,
                      const InterpMthd method, const int width,
                      const GridTemplateFactory::GridTemplates shape,
                      const double thresh,
                      const bool spfh_flag, const LevelType lvl_typ,
                      const double to_lvl, const int i_blw, const int i_abv,
                      NumArray &interp_points);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_BASE_H__

////////////////////////////////////////////////////////////////////////
