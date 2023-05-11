// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __PAIR_DATA_POINT_H__
#define  __PAIR_DATA_POINT_H__

////////////////////////////////////////////////////////////////////////

#include "pair_base.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "seeps.h"

////////////////////////////////////////////////////////////////////////
//
// Class to store matched pair data:
//    forecast, observation, and climatology values
//
////////////////////////////////////////////////////////////////////////

class PairDataPoint : public PairBase {

   private:

      void init_from_scratch();
      void assign(const PairDataPoint &);

      SeepsClimo   *seeps_climo;

   public:

      PairDataPoint();
      ~PairDataPoint();
      PairDataPoint(const PairDataPoint &);
      PairDataPoint & operator=(const PairDataPoint &);

      //////////////////////////////////////////////////////////////////

      // Forecast values
      NumArray f_na; // Forecast [n_obs]
      std::vector<SeepsScore *> seeps_mpr;
      SeepsAggScore seeps;

      //////////////////////////////////////////////////////////////////

      void clear();
      void erase();

      void extend(int);

      bool add_point_pair(const char *, double, double, double, double,
                          unixtime, double, double, double, double,
                          const char *, double, double, double);
      void load_seeps_climo();
      void set_seeps_thresh(const SingleThresh &p1_thresh);
      void set_seeps_score(SeepsScore *, int index=-1);

      void set_point_pair(int, const char *, double, double, double, double,
                          unixtime, double, double, double, double,
                          const char *, double, double, double, SeepsScore *);

      bool add_grid_pair(double, double, double, double, double);

      bool add_grid_pair(const NumArray &f_in,   const NumArray &o_in,
                         const NumArray &cmn_in, const NumArray &csd_in,
                         const NumArray &w_in);

      PairDataPoint subset_pairs_cnt_thresh(const SingleThresh &ft,
                                            const SingleThresh &ot,
                                            const SetLogic type) const;
      SeepsScore *compute_seeps(const char *, double, double, unixtime);

};

////////////////////////////////////////////////////////////////////////
//
// Class to store a variety of PairDataPoint objects for each
// verification task
//
////////////////////////////////////////////////////////////////////////

class VxPairDataPoint {

   private:

      void init_from_scratch();
      void assign(const VxPairDataPoint &);

   public:

      VxPairDataPoint();
      ~VxPairDataPoint();
      VxPairDataPoint(const VxPairDataPoint &);
      VxPairDataPoint & operator=(const VxPairDataPoint &);

      //////////////////////////////////////////////////////////////////
      //
      // Information about the fields to be compared
      //
      //////////////////////////////////////////////////////////////////

      VarInfo     *fcst_info;    // Forecast field, allocated by VarInfoFactory
      VarInfo     *climo_info;   // Climatology field, allocated by VarInfoFactory
      VarInfoGrib *obs_info;     // Observation field, allocated by VarInfoFactory

      ConcatString desc;         // User description from config file

      double interp_thresh;      // Threshold between 0 and 1 used when
                                 // interpolating the forecasts to the
                                 // observation location.

      //////////////////////////////////////////////////////////////////
      //
      // Forecast and climatology fields falling between the requested
      // levels.  Store the fields in a data plane array.
      //
      //////////////////////////////////////////////////////////////////

      DataPlaneArray fcst_dpa;     // Forecast data plane array
      DataPlaneArray climo_mn_dpa; // Climatology mean data plane array
      DataPlaneArray climo_sd_dpa; // Climatology standard deviation data plane array

      //////////////////////////////////////////////////////////////////

      unixtime fcst_ut;          // Forecast valid time
      unixtime beg_ut;           // Beginning of valid time window
      unixtime end_ut;           // End of valid time window

      //////////////////////////////////////////////////////////////////

      StringArray sid_inc_filt;  // Station ID inclusion list
      StringArray sid_exc_filt;  // Station ID exclusion list
      StringArray obs_qty_inc_filt;  // Observation quality include markers
      StringArray obs_qty_exc_filt;  // Observation quality exclude markers
      
      //////////////////////////////////////////////////////////////////

      StringArray mpr_column;    // Names of MPR columns or diffs of columns
      ThreshArray mpr_thresh;    // Filtering thresholds for the MPR columns

      //////////////////////////////////////////////////////////////////

      StringArray msg_typ_sfc;   // List of surface message types
      StringArray msg_typ_lnd;   // List of surface land message types
      StringArray msg_typ_wtr;   // List of surface water message types

      SurfaceInfo sfc_info;      // Land/sea mask and topography info

      //////////////////////////////////////////////////////////////////

      int      n_msg_typ;        // Number of verifying message types

      int      n_mask;           // Total number of masking regions
                                 // of masking DataPlane fields or SIDs

      int      n_interp;         // Number of interpolation techniques

      //////////////////////////////////////////////////////////////////

      PairDataPoint ***pd;       // 3-Dim Array of PairDataPoint objects
                                 // as [n_msg_typ][n_mask][n_interp]

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

      //  3-Dim Arrays for observation rejection reason codes
      int ***rej_typ;            // Reject based on message type
      int ***rej_mask;           // Reject based on masking region
      int ***rej_fcst;           // Reject forecast bad data
      int ***rej_cmn;            // Reject climo mean bad data
      int ***rej_csd;            // Reject climo stdev bad data
      int ***rej_mpr;            // Reject based on MPR filtering logic
      int ***rej_dup;            // Reject based on duplicates logic

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_fcst_info(VarInfo *);
      void set_climo_info(VarInfo *);
      void set_obs_info(VarInfoGrib *);

      void set_desc(const char *);

      void set_interp_thresh(double);

      void set_fcst_dpa(const DataPlaneArray &);
      void set_climo_mn_dpa(const DataPlaneArray &);
      void set_climo_sd_dpa(const DataPlaneArray &);

      void set_fcst_ut(const unixtime);
      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      void set_sid_inc_filt(const StringArray &);
      void set_sid_exc_filt(const StringArray &);
      void set_obs_qty_inc_filt(const StringArray &);
      void set_obs_qty_exc_filt(const StringArray &);
      
      // Call set_pd_size before set_msg_typ, set_mask_area, and set_interp
      void set_pd_size(int, int, int);

      void set_msg_typ(int, const char *);
      void set_msg_typ_vals(int, const StringArray &);
      void set_mask_area(int, const char *, MaskPlane *);
      void set_mask_sid(int, const char *, StringArray *);
      void set_mask_llpnt(int, const char *, MaskLatLon *);

      void set_interp(int i_interp, const char *interp_mthd_str, int width,
                      GridTemplateFactory::GridTemplates shape);
      void set_interp(int i_interp, InterpMthd mthd,
                      int width, GridTemplateFactory::GridTemplates shape);

      void set_mpr_thresh(const StringArray &, const ThreshArray &);

      void load_seeps_climo();
      void set_seeps_thresh(const SingleThresh &p1_thresh);

      void set_climo_cdf_info_ptr(const ClimoCDFInfo *);

      void set_msg_typ_sfc(const StringArray &);
      void set_msg_typ_lnd(const StringArray &);
      void set_msg_typ_wtr(const StringArray &);

      void set_sfc_info(const SurfaceInfo &);

      void add_point_obs(float *, const char *, const char *, unixtime,
                         const char *, float *, Grid &, const char * = 0,
                         const DataPlane * = 0);

      void add_prec_point_obs(float *, const char *, const char *, unixtime,
                              const char *, float *, Grid &, int month, int hour,
                              const char * = 0, const DataPlane * = 0);

      int  get_n_pair() const;

      void set_duplicate_flag(DuplicateType duplicate_flag);

      void set_obs_summary(ObsSummary obs_summary);

      void set_obs_perc_value(int percentile);

      void print_obs_summary();

      void calc_obs_summary();

      // Member functions for incrementing the counts
      void inc_count(int ***&, int);
      void inc_count(int ***&, int, int);
      void inc_count(int ***&, int, int, int);
};


////////////////////////////////////////////////////////////////////////
//
// Miscellanous functions
//
////////////////////////////////////////////////////////////////////////

extern bool check_fo_thresh(double, double, double, double,
                        const SingleThresh &, const SingleThresh &,
                        const SetLogic);

extern bool check_mpr_thresh(double, double, double, double,
                        const StringArray &, const ThreshArray &,
                        ConcatString * = 0);

extern double get_mpr_column_value(double, double, double, double,
                        const char *);

extern void apply_mpr_thresh_mask(DataPlane &, DataPlane &,
                        DataPlane &, DataPlane &,
                        const StringArray &, const ThreshArray &);

extern bool check_seeps_thresh(double, double,
                        const StringArray &, const ThreshArray &,
                        ConcatString * = 0);

extern double get_seeps_column_value(double, double,
                        const char *);

extern void apply_seeps_thresh_mask(DataPlane &, DataPlane &,
                        const StringArray &, const ThreshArray &);

// Apply conditional thresholds to subset the wind pairs
extern void subset_wind_pairs(const PairDataPoint &,
                        const PairDataPoint &, const SingleThresh &,
                        const SingleThresh &, const SetLogic,
                        PairDataPoint &, PairDataPoint &);

// Subset pairs for a specific climatology CDF bin
extern PairDataPoint subset_climo_cdf_bin(const PairDataPoint &,
                        const ThreshArray &, int i_bin);

// Write the point observation in the MET point format for logging
extern ConcatString point_obs_to_string(
                       float *hdr_arr, const char *hdr_typ_str,
                       const char *hdr_sid_str, unixtime hdr_ut,
                       const char *obs_qty, float *obs_arr,
                       const char *var_name);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_DATA_POINT_H__

////////////////////////////////////////////////////////////////////////
