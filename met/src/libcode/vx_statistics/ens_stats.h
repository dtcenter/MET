// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __ENS_STATS_H__
#define  __ENS_STATS_H__

////////////////////////////////////////////////////////////////////////

#include "pair_data_ensemble.h"
#include "met_stats.h"

#include "vx_config.h"
#include "vx_util.h"
#include "vx_grid.h"

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
// Class to store Ensemble Continuous Statistics
//
////////////////////////////////////////////////////////////////////////

class ECNTInfo {

   private:
      void init_from_scratch();
      void assign(const ECNTInfo &);

   public:

      ECNTInfo();
      ~ECNTInfo();
      ECNTInfo(const ECNTInfo &);
      ECNTInfo & operator=(const ECNTInfo &);

      // Filtering threshold
      SingleThresh othresh;

      // Number of ensemble members and pairs
      int n_ens, n_pair;

      double crps, crpss, ign;
      double me, rmse, spread;
      double me_oerr, rmse_oerr, spread_oerr;
      double spread_plus_oerr;

      // Compute statistics
      void set(const PairDataEnsemble &);
      
      void clear();
};

////////////////////////////////////////////////////////////////////////

#endif   // __ENS_STATS_H__

////////////////////////////////////////////////////////////////////////
