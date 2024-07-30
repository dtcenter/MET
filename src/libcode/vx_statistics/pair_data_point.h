// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

      SeepsClimo *seeps_climo;

   public:

      PairDataPoint();
      ~PairDataPoint();
      PairDataPoint(const PairDataPoint &);
      PairDataPoint & operator=(const PairDataPoint &);

      //////////////////////////////////////////////////////////////////

      // Forecast values
      NumArray f_na; // Forecast [n_obs]
      std::vector<SeepsScore *> seeps_mpr;
      SeepsAggScore seeps_agg;

      //////////////////////////////////////////////////////////////////

      void clear();
      void erase();

      void extend(int);

      bool add_point_pair(const char *, double, double, double, double,
                          unixtime, double, double, double, double,
                          const char *, const ClimoPntInfo &, double);
      void load_seeps_climo(const ConcatString &seeps_climo_name);
      void set_seeps_thresh(const SingleThresh &p1_thresh);
      void set_seeps_score(SeepsScore *, int index=-1);

      void set_point_pair(int, const char *, double, double, double, double,
                          unixtime, double, double, double, double,
                          const char *, const ClimoPntInfo &,
                          double, const SeepsScore *);

      bool add_grid_pair(double, double, const ClimoPntInfo &, double);

      bool add_grid_pair(const NumArray &f_in,   const NumArray &o_in,
                         const NumArray &fcmn_in, const NumArray &fcsd_in,
                         const NumArray &ocmn_in, const NumArray &ocsd_in,
                         const NumArray &w_in);

      PairDataPoint subset_pairs_cnt_thresh(const SingleThresh &ft,
                                            const SingleThresh &ot,
                                            const SetLogic type) const;
      SeepsScore *compute_seeps(const char *, double, double, unixtime);

};

////////////////////////////////////////////////////////////////////////
//
// Class to store PairDataPoint objects for point verification
//
////////////////////////////////////////////////////////////////////////

class VxPairDataPoint : public VxPairBase {

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

      // 3-Dim vector of PairDataPoint objects [n_msg_typ][n_mask][n_interp]
      std::vector<PairDataPoint> pd;

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_size(int, int, int);

      void load_seeps_climo(const ConcatString &seeps_climo_name);
      void set_seeps_thresh(const SingleThresh &p1_thresh);

      void add_point_obs(float *, const char *, const char *, unixtime,
                         const char *, float *, Grid &, const char * = 0,
                         const DataPlane * = 0);
};


////////////////////////////////////////////////////////////////////////
//
// Miscellanous functions
//
////////////////////////////////////////////////////////////////////////

extern bool check_fo_thresh(double, double, const ClimoPntInfo &,
                        const SingleThresh &, const SingleThresh &,
                        const SetLogic);

extern bool check_mpr_thresh(double, double, const ClimoPntInfo &,
                        const StringArray &, const ThreshArray &,
                        ConcatString * = 0);

extern double get_mpr_column_value(double, double, const ClimoPntInfo &,
                        const char *);

extern void apply_mpr_thresh_mask(DataPlane &, DataPlane &,
                        DataPlane &, DataPlane &,
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

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_DATA_POINT_H__

////////////////////////////////////////////////////////////////////////
