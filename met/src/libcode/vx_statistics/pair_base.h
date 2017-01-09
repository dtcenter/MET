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
      bool check_single;   // Check for duplicates, keeping single obs

      map<string,int>         map_unique;      // Storage for unique obs
      multimap<string,string> map_unique_sid;  // Storage for unique obs sids
      map<string,string>      map_single;      // Storage for single obs
      multimap<string,string> map_single_val;  // Storage for single obs values

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_mask_name(const char *);
      void set_mask_dp_ptr(DataPlane *);
      void set_mask_sid_ptr(StringArray *);
      void set_msg_typ(const char *);

      void set_interp_mthd(const char *);
      void set_interp_mthd(InterpMthd);
      void set_interp_dpth(int);

      void set_fcst_ut(unixtime ut);
      void set_check_unique(bool check);
      void set_check_single(bool check);

      int  has_obs_rec(const char *, double, double, double, double,
                       double, double, int &);

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

      void print_duplicate_report();
};

////////////////////////////////////////////////////////////////////////

extern void compute_crps_ign_pit(double, const NumArray &,
                                 double &, double &, double &);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_BASE_H__

////////////////////////////////////////////////////////////////////////
