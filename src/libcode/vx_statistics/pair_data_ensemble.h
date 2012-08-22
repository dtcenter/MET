// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __PAIR_DATA_ENSEMBLE_H__
#define  __PAIR_DATA_ENSEMBLE_H__

////////////////////////////////////////////////////////////////////////

#include "pair_base.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_gsl_prob.h"

////////////////////////////////////////////////////////////////////////
//
// Class to store ensemble pair data
//
////////////////////////////////////////////////////////////////////////

class PairDataEnsemble : public PairBase {

   private:

      void init_from_scratch();
      void assign(const PairDataEnsemble &);

   public:

      PairDataEnsemble();
      ~PairDataEnsemble();
      PairDataEnsemble(const PairDataEnsemble &);
      PairDataEnsemble & operator=(const PairDataEnsemble &);

      //////////////////////////////////////////////////////////////////

      // Ensemble, valid count, and rank values
      NumArray *e_na;     // Ensemble values [n_pair][n_ens]
      NumArray  v_na;     // Number of valid ensemble values [n_pair]
      NumArray  r_na;     // Observation ranks [n_pair]
      NumArray  crps_na;  // Continuous Ranked Probability Score [n_pair]
      NumArray  ign_na;   // Ignorance Score [n_pair]
      NumArray  pit_na;   // Probability Integral Transform [n_pair]
      int       n_pair;
      int       n_ens;      

      NumArray  rhist_na; // Ranked Histogram [n_ens]

      //////////////////////////////////////////////////////////////////

      void clear();

      void add_ens(int, double);
      void set_size();
      void set_n_ens();

      void compute_rank(const gsl_rng *);
      void compute_rhist();
      void compute_stats();
};

////////////////////////////////////////////////////////////////////////
//
// Class to store a variety of PairDataEnsemble objects for each
// verification task
//
////////////////////////////////////////////////////////////////////////

class VxPairDataEnsemble {

   private:

      void init_from_scratch();
      void assign(const VxPairDataEnsemble &);

   public:

      VxPairDataEnsemble();
      ~VxPairDataEnsemble();
      VxPairDataEnsemble(const VxPairDataEnsemble &);
      VxPairDataEnsemble & operator=(const VxPairDataEnsemble &);

      //////////////////////////////////////////////////////////////////
      //
      // Information about the fields to be compared
      //
      //////////////////////////////////////////////////////////////////
      
      VarInfo *fcst_info;        // Forecast field, allocated by VarInfoFactory
      VarInfo *obs_info;         // Observation field, allocated by VarInfoFactory

      double interp_thresh;      // Threshold between 0 and 1 used when
                                 // interpolating the forecasts to the
                                 // observation location.

      //////////////////////////////////////////////////////////////////
      //
      // Forecast fields falling between the requested levels.
      // Store the fields in a data plane array.
      //
      //////////////////////////////////////////////////////////////////

      DataPlaneArray fcst_dpa;   // Forecast data plane array

      //////////////////////////////////////////////////////////////////

      unixtime fcst_ut;          // Ensemble valid time
      unixtime beg_ut;           // Beginning of valid time window
      unixtime end_ut;           // End of valid time window

      //////////////////////////////////////////////////////////////////

      StringArray obs_qty_filt;  // Observation quality markers

      //////////////////////////////////////////////////////////////////

      int      n_msg_typ;        // Number of verifying message types

      int      n_mask;           // Total number of masking regions
                                 // of masking DataPlane fields or SIDs

      int      n_interp;         // Number of interpolation techniques

      //////////////////////////////////////////////////////////////////

      PairDataEnsemble ***pd;    // 3-Dim Array of PairDataEnsemble objects
                                 // as [n_msg_typ][n_mask][n_interp]

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_fcst_info(VarInfo *);
      void set_obs_info(VarInfo *);
      void set_interp_thresh(double);

      void set_fcst_dpa(const DataPlaneArray &);

      void set_fcst_ut(const unixtime);
      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      void set_obs_qty_filt(const StringArray);

      // Call set_pd_size before set_msg_typ, set_mask_dp, and set_interp
      void set_pd_size(int, int, int);

      void set_msg_typ(int, const char *);
      void set_mask_dp(int, const char *, DataPlane *);
      void set_interp(int, const char *, int);
      void set_interp(int, InterpMthd, int);

      // Call set_ens_size before add_ens
      void set_ens_size();

      void add_obs(float *, const char *, const char *, unixtime,
                   const char *, float *, Grid &);
      void add_ens();

      void find_vert_lvl(double, int &, int &);

      int  get_n_pair();

      void set_duplicate_flag(DuplicateType duplicate_flag);
      void print_duplicate_report();

      double compute_interp(double, double, int, double, int, int);
};

////////////////////////////////////////////////////////////////////////

extern void   compute_crps_ign_pit(double, const NumArray &,
                                   double &, double &, double &);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_DATA_ENSEMBLE_H__

////////////////////////////////////////////////////////////////////////
