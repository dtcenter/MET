// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
#include "vx_data2d_grib.h"

////////////////////////////////////////////////////////////////////////

static const int REJECT_DEBUG_LEVEL = 9;

////////////////////////////////////////////////////////////////////////

struct ob_val_t {
   unixtime ut;
   double val;
   std::string qc;
};

struct station_values_t {

   void clear();

   std::string sid;
   double lat;
   double lon;
   double x;
   double y;
   double wgt;
   unixtime ut;
   double lvl;
   double elv;
   double fcmn;
   double fcsd;
   double ocmn;
   double ocsd;
   double summary_val;
   std::vector<ob_val_t> obs;
};

////////////////////////////////////////////////////////////////////////
//
// Base class for matched pair data
//
////////////////////////////////////////////////////////////////////////

class PairBase {

   private:

      void init_from_scratch();

      bool IsPointVx;

   public:

      PairBase();
      virtual ~PairBase();

      //////////////////////////////////////////////////////////////////

      // Masking area applied to the forecast and climo fields
      ConcatString   mask_name;
      MaskPlane     *mask_area_ptr;  // Pointer to the masking MaskPlane
                                     // which is not allocated
      MaskSID       *mask_sid_ptr;   // Pointer to masking station ID list
                                     // which is not allocated
      MaskLatLon    *mask_llpnt_ptr; // Pointer to Lat/Lon thresholds
                                     // which is not allocated

      const ClimoCDFInfo *cdf_info_ptr; // Pointer to climo distribution info
                                        // which is not allocated

      //////////////////////////////////////////////////////////////////

      ConcatString msg_typ;          // Name of the verifying message type
      StringArray  msg_typ_vals;     // Message type values to be included

      // Interpolation method and shape used
      InterpMthd interp_mthd;
      int        interp_wdth;
      GridTemplateFactory::GridTemplates interp_shape;

      // Point and Grid Observation Information
      NumArray    o_na;    // Observation value [n_obs]
      NumArray    x_na;    // X [n_obs]
      NumArray    y_na;    // Y [n_obs]
      NumArray    wgt_na;  // Weight [n_obs]

      // Point and Grid Climatology Information
      NumArray    fcmn_na; // Forecast climatology mean [n_obs]
      NumArray    fcsd_na; // Forecast climatology standard deviation [n_obs]
      NumArray    ocmn_na; // Observation climatology mean [n_obs]
      NumArray    ocsd_na; // Observation climatology standard deviation [n_obs]
      NumArray    ocdf_na; // Observation climatology cumulative distribution function [n_obs]

      // Point Observation Information
      StringArray sid_sa;  // Station ID [n_obs]
      NumArray    lat_na;  // Latitude [n_obs]
      NumArray    lon_na;  // Longitude [n_obs]
      TimeArray   vld_ta;  // Valid time [n_obs]
      NumArray    lvl_na;  // Level [n_obs]
      NumArray    elv_na;  // Elevation [n_obs]
      StringArray o_qc_sa; // Observation quality control [n_obs]

      int         n_obs;   // Number of observations
      unixtime    fcst_ut; // Forecast valid time

      bool       check_unique;   // Check for duplicates, keeping unique obs
      ObsSummary obs_summary;    // Summarize multiple observations
      int        obs_perc_value; // Percentile value for ObsSummary::Perc

      StringArray map_key;
      std::map<std::string,station_values_t> map_val; // Storage for single obs values

      //////////////////////////////////////////////////////////////////

      void clear();
      void erase();

      void extend(int); // Allocate memory for expected size

      bool is_point_vx() const;

      void set_mask_name(const std::string &);
      void set_mask_area_ptr(MaskPlane *);
      void set_mask_sid_ptr(MaskSID *);
      void set_mask_llpnt_ptr(MaskLatLon *);

      void set_climo_cdf_info_ptr(const ClimoCDFInfo *);

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

      ob_val_t compute_nearest(std::string sng_key);
      ob_val_t compute_min(std::string sng_key);
      ob_val_t compute_max(std::string sng_key);
      ob_val_t compute_uw_mean(std::string sng_key);
      ob_val_t compute_dw_mean(std::string sng_key);
      ob_val_t compute_median(std::string sng_key);
      ob_val_t compute_percentile(std::string sng_key, int perc);

      bool add_point_obs(const char *, double, double, double, double,
                         unixtime, double, double, double, const char *,
                         const ClimoPntInfo &, double);

      void set_point_obs(int, const char *, double, double, double, double,
                         unixtime, double, double, double,
                         const char *, const ClimoPntInfo &, double);

      void add_grid_obs(double, const ClimoPntInfo &, double);
      
      void add_grid_obs(double, double, double, const ClimoPntInfo &, double);

      void add_climo(double, const ClimoPntInfo &);

      void set_climo(int, double, const ClimoPntInfo &);

      void compute_climo_cdf();

      double process_obs(const VarInfo *, double) const;

      void print_obs_summary() const;

      void calc_obs_summary();

};

////////////////////////////////////////////////////////////////////////

inline bool PairBase::is_point_vx() const { return IsPointVx; }

////////////////////////////////////////////////////////////////////////
//
// Base class for verification tasks
//
////////////////////////////////////////////////////////////////////////

class VxPairBase {

   protected:

      void init_from_scratch();
      void assign(const VxPairBase &);
      void copy_var_info(const VarInfo *info, VarInfo *&copy);

   public:

      VxPairBase();
      ~VxPairBase();
      VxPairBase(const VxPairBase &);
      VxPairBase & operator=(const VxPairBase &);

      //////////////////////////////////////////////////////////////////
      //
      // Information about the fields to be compared
      //
      //////////////////////////////////////////////////////////////////

      VarInfo *fcst_info;        // Forecast field, allocated by VarInfoFactory
      VarInfo *obs_info;         // Observation field, allocated by VarInfoFactory

      VarInfo *fclm_info;        // Forecast climatology field, allocated by VarInfoFactory
      VarInfo *oclm_info;        // Observation climatology field, allocated by VarInfoFactory

      ConcatString desc;         // User description from config file

      double interp_thresh;      // Threshold between 0 and 1 used when
                                 // interpolating the forecasts to the
                                 // observation location.

      //////////////////////////////////////////////////////////////////
      //
      // Forecast and climatology fields falling between the requested
      // levels. Store the fields in a data plane array.
      //
      //////////////////////////////////////////////////////////////////

      DataPlaneArray fcst_dpa;   // Forecast data plane array
      DataPlaneArray fcmn_dpa;   // Forecast climatology mean data plane array
      DataPlaneArray fcsd_dpa;   // Forecast climatology standard deviation data plane array
      DataPlaneArray ocmn_dpa;   // Observation climatology mean data plane array
      DataPlaneArray ocsd_dpa;   // Observation climatology standard deviation data plane array

      //////////////////////////////////////////////////////////////////

      unixtime fcst_ut;          // Forecast valid time
      unixtime beg_ut;           // Beginning of valid time window
      unixtime end_ut;           // End of valid time window

      //////////////////////////////////////////////////////////////////

      StringArray sid_inc_filt;     // Station ID inclusion list
      StringArray sid_exc_filt;     // Station ID exclusion list
      StringArray obs_qty_inc_filt; // Observation quality include markers
      StringArray obs_qty_exc_filt; // Observation quality exclude markers

      //////////////////////////////////////////////////////////////////

      StringArray mpr_column;    // Names of MPR columns or diffs of columns
      ThreshArray mpr_thresh;    // Filtering thresholds for the MPR columns

      //////////////////////////////////////////////////////////////////

      StringArray msg_typ_sfc;   // List of surface message types
      StringArray msg_typ_lnd;   // List of surface land message types
      StringArray msg_typ_wtr;   // List of surface water message types

      SurfaceInfo sfc_info;      // Land/sea mask and topography info

      //////////////////////////////////////////////////////////////////

      int n_msg_typ;             // Number of verifying message types

      int n_mask;                // Total number of masking regions
                                 // of masking DataPlane fields or SIDs

      int n_interp;              // Number of interpolation techniques

      int n_vx;                  // n_msg_typ * n_mask * n_interp

      //////////////////////////////////////////////////////////////////

      // 3-Dim vector of PairBase pointers [n_msg_typ][n_mask][n_interp]
      std::vector<PairBase *> pb_ptr;

      //  Counts for observation rejection reason codes
      int n_try;                 // Number of observations processed
      int rej_sid;               // Reject based on SID inclusion and exclusion lists
      int rej_var;               // Reject based on observation variable name
      int rej_vld;               // Reject based on valid time
      int rej_obs;               // Reject observation bad data
      int rej_grd;               // Reject based on location
      int rej_topo;              // Reject based on topography
      int rej_lvl;               // Reject based on vertical level
      int rej_qty;               // Reject based on obs quality

      // 3-Dim vectors for observation rejection reason codes [n_msg_typ][n_mask][n_interp]
      std::vector<int> rej_typ;  // Reject based on message type
      std::vector<int> rej_mask; // Reject based on masking region
      std::vector<int> rej_fcst; // Reject forecast bad data
      std::vector<int> rej_cmn;  // Reject fcst or obs climo mean bad data
      std::vector<int> rej_csd;  // Reject fcst or obs climo stdev bad data
      std::vector<int> rej_mpr;  // Reject based on MPR filtering logic
      std::vector<int> rej_dup;  // Reject based on duplicates logic

      //////////////////////////////////////////////////////////////////

      void clear();

      int three_to_one(int, int, int) const;

      void set_fcst_info(const VarInfo *);
      void set_obs_info(const VarInfo *);

      void set_fcst_climo_info(const VarInfo *);
      void set_obs_climo_info(const VarInfo *);

      void set_desc(const char *);

      void set_interp_thresh(double);

      void set_fcst_dpa(const DataPlaneArray &);
      void set_fcst_climo_mn_dpa(const DataPlaneArray &);
      void set_fcst_climo_sd_dpa(const DataPlaneArray &);
      void set_obs_climo_mn_dpa(const DataPlaneArray &);
      void set_obs_climo_sd_dpa(const DataPlaneArray &);

      void set_fcst_ut(const unixtime);
      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      void set_sid_inc_filt(const StringArray &);
      void set_sid_exc_filt(const StringArray &);
      void set_obs_qty_inc_filt(const StringArray &);
      void set_obs_qty_exc_filt(const StringArray &);

      // Call set_size before set_msg_typ, set_mask_area, and set_interp
      void set_size(int, int, int);

      void set_msg_typ(int, const char *);
      void set_msg_typ_vals(int, const StringArray &);
      void set_mask_area(int, const char *, MaskPlane *);
      void set_mask_sid(int, const char *, MaskSID *);
      void set_mask_llpnt(int, const char *, MaskLatLon *);

      void set_interp(int i_interp, const char *interp_mthd_str, int width,
                      GridTemplateFactory::GridTemplates shape);
      void set_interp(int i_interp, InterpMthd mthd,
                      int width, GridTemplateFactory::GridTemplates shape);

      void set_mpr_thresh(const StringArray &, const ThreshArray &);

      void set_climo_cdf_info_ptr(const ClimoCDFInfo *);

      void set_msg_typ_sfc(const StringArray &);
      void set_msg_typ_lnd(const StringArray &);
      void set_msg_typ_wtr(const StringArray &);

      void set_sfc_info(const SurfaceInfo &);

      int  get_n_pair() const;

      void set_duplicate_flag(DuplicateType duplicate_flag);
      void set_obs_summary(ObsSummary obs_summary);
      void set_obs_perc_value(int percentile);

      void print_obs_summary() const;
      void calc_obs_summary();

      bool is_keeper_sid(const char *, const char *);
      bool is_keeper_var(const char *, const char *, int);
      bool is_keeper_qty(const char *, const char *);
      bool is_keeper_vld(const char *, unixtime);
      bool is_keeper_obs(const char *, double &);
      bool is_keeper_grd(const char *, const Grid &,
                         double, double,
                         double &, double &);
      bool is_keeper_topo(const char *, const Grid &,
                          double, double,
                          const char *, double);
      bool is_keeper_lvl(const char *, const char *, double, double);
      bool is_keeper_typ(const char *, int, const char *);
      bool is_keeper_mask(const char *, int, int, int, int,
                          const char *, double, double);
      bool is_keeper_climo(const char *, int, int, int,
                           const Grid &gr, double, double,
                           double, double, double,
                           ClimoPntInfo &);
      bool is_keeper_fcst(const char *, int, int, int,
                          const char *, const Grid &gr,
                          double, double, double,
                          double, double, double,
                          const ClimoPntInfo &, double &);

      // Member functions for incrementing the counts
      void inc_count(std::vector<int> &, int);
      void inc_count(std::vector<int> &, int, int);
      void inc_count(std::vector<int> &, int, int, int);
};

////////////////////////////////////////////////////////////////////////
//
// Miscellanous utility functions
//
////////////////////////////////////////////////////////////////////////

extern void find_vert_lvl(const DataPlaneArray &, const double,
                          int &, int &);

extern double compute_interp(const DataPlaneArray &dpa,
                             const double obs_x, const double obs_y,
                             const double obs_v, const ClimoPntInfo *cpi,
                             const InterpMthd method, const int width,
                             const GridTemplateFactory::GridTemplates shape,
                             const bool wrap_lon,
                             const double thresh,
                             const bool spfh_flag, const LevelType lvl_typ,
                             const double to_lvl, const int i_blw, const int i_abv,
                             const SingleThresh *cat_thresh = 0);

extern void get_interp_points(const DataPlaneArray &dpa,
                              const double obs_x, const double obs_y,
                              const InterpMthd method, const int width,
                              const GridTemplateFactory::GridTemplates shape,
                              const bool wrap_lon,
                              const double thresh,
                              const bool spfh_flag, const LevelType lvl_typ,
                              const double to_lvl, const int i_blw, const int i_abv,
                              NumArray &interp_points);

extern bool set_climo_flag(const NumArray &, const NumArray &);

extern void derive_climo_vals(const ClimoCDFInfo *,
                              double, double, NumArray &);

extern NumArray derive_climo_prob(const ClimoCDFInfo *,
                                  const NumArray &, const NumArray &,
                                  const SingleThresh &);

extern double derive_prob(const NumArray &, const SingleThresh &);

// Write the point observation in the MET point format for logging
extern ConcatString point_obs_to_string(
                       const float *hdr_arr, const char *hdr_typ_str,
                       const char *hdr_sid_str, unixtime hdr_ut,
                       const char *obs_qty, const float *obs_arr,
                       const char *var_name);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_BASE_H__

////////////////////////////////////////////////////////////////////////
