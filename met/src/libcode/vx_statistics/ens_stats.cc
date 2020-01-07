// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "ens_stats.h"
#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class SSVARInfo
//
////////////////////////////////////////////////////////////////////////

SSVARInfo::SSVARInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

SSVARInfo::~SSVARInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

SSVARInfo::SSVARInfo(const SSVARInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

SSVARInfo & SSVARInfo::operator=(const SSVARInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

SSVARInfo & SSVARInfo::operator+=(const SSVARInfo &c) {
   SSVARInfo ssvar_info;

   // Check for matching variance bounds
   if(!is_eq(var_min, c.var_min) || !is_eq(var_max, c.var_max)) {
      mlog << Error << "\nSSVARInfo::operator+=() -> "
           << "the variance bounds don't match ("
           << var_min << ", " << var_max << ") != ("
           << c.var_min << ", " << c.var_max << ").\n\n";
      exit(1);
   }

   // The bin index information is unknown when aggregating
   ssvar_info.n_bin = ssvar_info.bin_i = bad_data_int;

   // Increment the bin count
   ssvar_info.bin_n = bin_n + c.bin_n;

   // Store the variance range
   ssvar_info.var_min  = var_min;
   ssvar_info.var_max  = var_max;

   // Compute weighted averages
   ssvar_info.var_mean = (var_mean*bin_n + c.var_mean*c.bin_n)/ssvar_info.bin_n;
   ssvar_info.sl1l2_info  = sl1l2_info;
   ssvar_info.sl1l2_info += c.sl1l2_info;

   assign(ssvar_info);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void SSVARInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void SSVARInfo::clear() {

   n_bin = bin_i = bin_n = 0;
   var_min = var_max = var_mean = 0;
   sl1l2_info.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void SSVARInfo::assign(const SSVARInfo &c) {

   n_bin      = c.n_bin;
   bin_i      = c.bin_i;
   bin_n      = c.bin_n;
   var_min    = c.var_min;
   var_max    = c.var_max;
   var_mean   = c.var_mean;
   sl1l2_info = c.sl1l2_info;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class ECNTInfo
//
////////////////////////////////////////////////////////////////////////

ECNTInfo::ECNTInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ECNTInfo::~ECNTInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ECNTInfo::ECNTInfo(const ECNTInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

ECNTInfo & ECNTInfo::operator=(const ECNTInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ECNTInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ECNTInfo::clear() {

   othresh.clear();
   n_ens   = n_pair    = 0;
   crps    = crpss     = ign         = bad_data_double;
   me      = rmse      = spread      = bad_data_double;
   me_oerr = rmse_oerr = spread_oerr = bad_data_double;
   spread_plus_oerr    = bad_data_double;
   
   return;
}

////////////////////////////////////////////////////////////////////////

void ECNTInfo::assign(const ECNTInfo &c) {

   othresh          = c.othresh;

   n_ens            = c.n_ens;
   n_pair           = c.n_pair;

   crps             = c.crps;
   crpss            = c.crpss;
   ign              = c.ign;

   me               = c.me;
   rmse             = c.rmse;
   spread           = c.spread;

   me_oerr          = c.me_oerr;
   rmse_oerr        = c.rmse_oerr;
   spread_oerr      = c.spread_oerr;
   spread_plus_oerr = c.spread_plus_oerr;

   return;
}

////////////////////////////////////////////////////////////////////////

void ECNTInfo::set(const PairDataEnsemble &pd) {
   int i;
   double w, w_sum;
   double crps_climo;
   double fbar, obar, ffbar, oobar, fobar;
   NumArray cur;

   // Store the number of ensemble members
   n_ens = pd.n_ens;
   
   // Get the average CRPS value
   crps = pd.crps_na.wmean(pd.wgt_na);

   // Get the sum of the weights
   for(i=0, n_pair=0, w_sum=0.0; i<pd.wgt_na.n(); i++) {
      if(!pd.skip_ba[i]) {
         n_pair++;
         w_sum += pd.wgt_na[i];
      }
   }

   // Check for bad data
   if(is_bad_data(crps)            ||
      pd.cmn_na.n() != pd.o_na.n() ||
      pd.cmn_na.n() == 0           ||
      pd.cmn_na.has(bad_data_double)) {
      crpss = bad_data_double;
   }
   else {

      // Compute the climatological CRPS
      ffbar = oobar = fobar = 0.0;
      for(i=0; i<pd.n_obs; i++) {

         if(pd.skip_ba[i]) continue;

         // Track running sums
         w      = pd.wgt_na[i]/w_sum;
         ffbar += w * pd.cmn_na[i] * pd.cmn_na[i];
         oobar += w * pd.o_na[i]   * pd.o_na[i];
         fobar += w * pd.cmn_na[i] * pd.o_na[i];
      }
      crps_climo = ffbar + oobar - 2.0*fobar;

      // Compute skill score
      crpss = (is_eq(crps_climo, 0.0) ?
               bad_data_double : (crps_climo - crps)/crps_climo);
   }

   // Compute the average IGN value
   ign = pd.ign_na.wmean(pd.wgt_na);

   // Compute ME and RMSE values
   fbar = obar = ffbar = oobar = fobar = 0.0;
   for(i=0; i<pd.n_obs; i++) {

      if(pd.skip_ba[i]) continue;

      // Track running sums
      w      = pd.wgt_na[i]/w_sum;
      obar  += w *  pd.o_na[i];
      oobar += w *  pd.o_na[i] *  pd.o_na[i];
      fbar  += w * pd.mn_na[i];
      ffbar += w * pd.mn_na[i] * pd.mn_na[i];
      fobar += w * pd.mn_na[i] *  pd.o_na[i];
   }

   // Derive ME and RMSE from partial sums
   me   = fbar - obar;
   rmse = sqrt(ffbar + oobar - 2.0*fobar);

   // Compute the average spread value
   spread = pd.spread_na.wmean(pd.wgt_na);
   
   // If observation error was specified, compute ME_OERR and RMSE_OERR
   if(pd.has_obs_error()) {

      fbar = obar = ffbar = oobar = fobar = 0.0;
      for(i=0; i<pd.n_obs; i++) {

         if(pd.skip_ba[i]) continue;

         // Track running sums
         w      = pd.wgt_na[i]/w_sum;
         obar  += w *       pd.o_na[i];
         oobar += w *       pd.o_na[i] *       pd.o_na[i];
         fbar  += w * pd.mn_oerr_na[i];
         ffbar += w * pd.mn_oerr_na[i] * pd.mn_oerr_na[i];
         fobar += w * pd.mn_oerr_na[i] *       pd.o_na[i];
      }

      // Derive ME_OERR and RMSE_OERR from partial sums
      me_oerr   = fbar - obar;
      rmse_oerr = sqrt(ffbar + oobar - 2.0*fobar);
   }
   else {
      me_oerr   = bad_data_double;
      rmse_oerr = bad_data_double;
   }

   // Compute the average perturbed spread value
   spread_oerr = pd.spread_oerr_na.wmean(pd.wgt_na);

   // Compute the average spread plus oerr value
   spread_plus_oerr = pd.spread_plus_oerr_na.wmean(pd.wgt_na);

   return;
}

////////////////////////////////////////////////////////////////////////
