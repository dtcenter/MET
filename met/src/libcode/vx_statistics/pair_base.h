// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __PAIR_BASE_H__
#define  __PAIR_BASE_H__

////////////////////////////////////////////////////////////////////////

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
      DataPlane     *mask_dp_ptr; // Pointer to the masking field
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
      TimeArray   vld_ta;  // Valid time [n_obs]
      NumArray    lvl_na;  // Level [n_obs]
      NumArray    elv_na;  // Elevation [n_obs]
      NumArray    o_na;    // Observation value [n_obs]
      int         n_obs;   // Number of observations

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_mask_name(const char *);
      void set_mask_dp_ptr(DataPlane *);
      void set_msg_typ(const char *);

      void set_interp_mthd(const char *);
      void set_interp_mthd(InterpMthd);
      void set_interp_dpth(int);

      int  has_obs_rec(const char *, double, double, double, double,
                       double, double, int &);

      void add_obs(const char *, double, double, double, double,
                   unixtime, double, double, double);
      void add_obs(double, double, double);

      void set_obs(int, const char *, double, double, double, double,
                   unixtime, double, double, double);
      void set_obs(int, double, double, double);

};

////////////////////////////////////////////////////////////////////////

extern void   compute_crps_ign_pit(double, const NumArray &,
                                   double &, double &, double &);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_BASE_H__

////////////////////////////////////////////////////////////////////////
