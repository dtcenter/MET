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
//
// Code for class ERPSInfo
//
////////////////////////////////////////////////////////////////////////

ERPSInfo::ERPSInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ERPSInfo::~ERPSInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ERPSInfo::ERPSInfo(const ERPSInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

ERPSInfo & ERPSInfo::operator=(const ERPSInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

ERPSInfo & ERPSInfo::operator+=(const ERPSInfo &c) {
   ERPSInfo r_info;

   if(n_ens != c.n_ens) {
      mlog << Error << "\nERPSInfo::operator+=() -> "
           << "the ensemble size must remain constant (" << n_ens
           << " != " << c.n_ens << ")!\n\n";
      exit(1);
   }

   r_info.fthresh = fthresh;
   r_info.othresh = othresh;
   r_info.n_ens   = n_ens;
   r_info.n_pair  = n_pair + c.n_pair;

   if(r_info.n_pair > 0) {

      // Aggregate RPS as a weighted average
      r_info.rps = (rps*n_pair + c.rps*c.n_pair) / r_info.n_pair;

      // Compute RPSS with external climatology
      if(!is_bad_data(rpscl) && !is_bad_data(c.rpscl)) {
         r_info.rpscl = (rpscl*n_pair + c.rpscl*c.n_pair) / r_info.n_pair;
         r_info.rpss  = (!is_eq(rpscl, 0.0) ?
                         1.0 - (r_info.rps / r_info.rpscl) :
                         bad_data_double);
      }
      else {
         r_info.rpscl = bad_data_double;
         r_info.rpss  = bad_data_double;
      }

      // The RPS components cannot be aggregated as a weighted average
      r_info.rps_rel   = bad_data_double;
      r_info.rps_res   = bad_data_double;
      r_info.rps_unc   = bad_data_double;
      r_info.rpss_smpl = bad_data_double;
   }

   assign(r_info);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ERPSInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ERPSInfo::clear() {

   othresh.clear();
   n_ens     = n_pair  = 0;
   rps_rel   = rps_res = rps_unc = bad_data_double;
   rps       = rpscl   = rpss    = bad_data_double;
   rpss_smpl = bad_data_double;
   
   return;
}

////////////////////////////////////////////////////////////////////////

void ERPSInfo::assign(const ERPSInfo &c) {

   fthresh   = c.fthresh;
   othresh   = c.othresh;

   n_ens     = c.n_ens;
   n_pair    = c.n_pair;

   rps_rel   = c.rps_rel;
   rps_res   = c.rps_res;
   rps_unc   = c.rps_unc;

   rps       = c.rps;
   rpscl     = c.rpscl;
   rpss      = c.rpss;

   rpss_smpl = c.rpss_smpl;

   return;
}

////////////////////////////////////////////////////////////////////////

void ERPSInfo::set(const PairDataEnsemble &pd) {
   int i, j, k, n_event;
   double p;
   bool cmn_flag;
   NumArray p_thresh, climo_prob;

   // Store the dimensions
   n_ens  = pd.n_ens;
   n_pair = pd.n_pair;

   // Check that thresholds are actually defined
   if(fthresh.n() == 0) {
      mlog << Error << "\nERPSInfo::set(const PairDataEnsemble &) -> "
           << "no thresholds provided to compute the ERPS line type! "
           << "Specify thresholds using the \"" << conf_key_rps_thresh
           << "\" configuration file option.\n\n";
      exit(1);
   }

   // Check RPS threshold formatting: monotonically increasing
   fthresh.check_bin_thresh();

   // Flag to process climo
   cmn_flag = set_climo_flag(pd.o_na, pd.cmn_na);

   // Setup probability thresholds, equally spaced by ensemble size
   for(i=0; i<=n_ens; i++) p_thresh.add((double) i/n_ens);

   // Setup forecast probabilistic contingency table
   Nx2ContingencyTable fcst_pct;
   fcst_pct.set_size(n_ens);
   fcst_pct.set_thresholds(p_thresh.vals());

   // Setup climatology probabilistic contingency table
   Nx2ContingencyTable climo_pct;
   climo_pct.set_size(n_ens);
   climo_pct.set_thresholds(p_thresh.vals());

   // Initialize
   rps_rel = rps_res = rps_unc = 0.0;
   rps     = rpscl   = 0.0;

   // Loop over the fthresh entries and populate PCT tables for each
   for(i=0; i<fthresh.n(); i++) {

      // Initialize PCT counts
      fcst_pct.zero_out();
      climo_pct.zero_out();

      // Derive climatological probabilities
      if(cmn_flag) climo_prob = derive_climo_prob(pd.cmn_na, pd.csd_na,
                                                  fthresh[i]);

      // Loop over the observations
      for(j=0; j<pd.n_obs; j++) {

         // Loop over ensemble members and count events
         for(k=0, n_event=0; k<n_ens; k++) {
            if(fthresh[i].check(pd.e_na[k][j])) n_event++;
         }

         // Update the forecast PCT counts
         p = (double) n_event/n_ens;
         if(fthresh[i].check(pd.o_na[j])) fcst_pct.inc_event(p);
         else                             fcst_pct.inc_nonevent(p);

         // Update the climatology PCT counts
         if(cmn_flag) {
            p = climo_prob[j];
            if(fthresh[i].check(pd.o_na[j])) climo_pct.inc_event(p);
            else                             climo_pct.inc_nonevent(p);
         }
      } // end for j

      // Increment sums
      rps_rel   += fcst_pct.reliability() / fthresh.n();
      rps_res   += fcst_pct.resolution()  / fthresh.n();
      rps_unc   += fcst_pct.uncertainty() / fthresh.n();
      rps       += fcst_pct.brier_score() / fthresh.n();
      if(cmn_flag) rpscl += climo_pct.brier_score() / fthresh.n();

   } // end for i
   
   // Compute RPSS with sample climatology
   rpss_smpl = (!is_eq(rps_unc, 0.0) ?
               (rps_res - rps_rel) / rps_unc :
               bad_data_double);

   // Compute RPSS with external climatology
   rpss      = (cmn_flag && !is_eq(rpscl, 0.0) ?
                1.0 - (rps / rpscl) :
                bad_data_double);

   return;
}

////////////////////////////////////////////////////////////////////////
