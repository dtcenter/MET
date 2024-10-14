// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <set>
#include <limits>

#include "pair_data_ensemble.h"
#include "ens_stats.h"
#include "obs_error.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_util.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
// Code for class PairDataEnsemble
//
////////////////////////////////////////////////////////////////////////

PairDataEnsemble::PairDataEnsemble() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairDataEnsemble::~PairDataEnsemble() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PairDataEnsemble::PairDataEnsemble(const PairDataEnsemble &pd) {

   init_from_scratch();

   assign(pd);
}

////////////////////////////////////////////////////////////////////////

PairDataEnsemble & PairDataEnsemble::operator=(const PairDataEnsemble &pd) {

   if(this == &pd) return *this;

   assign(pd);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::init_from_scratch() {

   e_na       = (NumArray *) nullptr;
   n_ens      = 0;
   ssvar_bins = (SSVARInfo *) nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::clear() {
   int i;

   PairBase::clear();

   obs_error_entry.clear();
   obs_error_flag = false;

   for(i=0; i<n_ens; i++) e_na[i].clear();
   if(e_na) { delete [] e_na; e_na = (NumArray *) nullptr; }

   v_na.clear();
   r_na.clear();

   crps_emp_na.clear();
   crps_emp_fair_na.clear();
   spread_md_na.clear();
   crpscl_emp_na.clear();
   crps_gaus_na.clear();
   crpscl_gaus_na.clear();

   ign_na.clear();
   pit_na.clear();

   ign_conv_oerr_na.clear();
   ign_corr_oerr_na.clear();

   n_ge_obs_na.clear();
   me_ge_obs_na.clear();
   n_lt_obs_na.clear();
   me_lt_obs_na.clear();

   n_ens = 0;
   n_pair = 0;
   ctrl_index = bad_data_int;
   skip_const = false;
   skip_ba.clear();

   rhist_na.clear();
   relp_na.clear();
   phist_na.clear();

   var_na.clear();
   var_oerr_na.clear();
   var_plus_oerr_na.clear();

   esum_na.clear();
   esumsq_na.clear();
   esumn_na.clear();

   mn_na.clear();
   mn_oerr_na.clear();

   if(ssvar_bins) { delete [] ssvar_bins; ssvar_bins = (SSVARInfo *) nullptr; }

   ssvar_bin_size = bad_data_double;
   phist_bin_size = bad_data_double;

   crpss_emp      = bad_data_double;
   crpss_gaus     = bad_data_double;

   me             = bad_data_double;
   mae            = bad_data_double;
   rmse           = bad_data_double;
   me_oerr        = bad_data_double;
   mae_oerr       = bad_data_double;
   rmse_oerr      = bad_data_double;

   bias_ratio     = bad_data_double;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::extend(int n) {
   int i;

   // Allocate memory for the number of observations.
   // Only applies to arrays sized by n_obs which does not include:
   //   rhist_na, relp_na, phist_na

   PairBase::extend(n);

   obs_error_entry.extend(n);

   for(i=0; i<n_ens; i++) e_na[i].extend(n);

   v_na.extend               (n);
   r_na.extend               (n);
   crps_emp_na.extend        (n);
   crps_emp_fair_na.extend   (n);
   spread_md_na.extend       (n);
   crpscl_emp_na.extend      (n);
   crps_gaus_na.extend       (n);
   crpscl_gaus_na.extend     (n);
   ign_na.extend             (n);
   pit_na.extend             (n);
   ign_conv_oerr_na.extend   (n);
   ign_corr_oerr_na.extend   (n);
   n_ge_obs_na.extend        (n);
   me_ge_obs_na.extend       (n);
   n_lt_obs_na.extend        (n);
   me_lt_obs_na.extend       (n);
   skip_ba.extend            (n);
   var_na.extend             (n);
   var_oerr_na.extend        (n);
   var_plus_oerr_na.extend   (n);
   esum_na.extend            (n);
   esumsq_na.extend          (n);
   esumn_na.extend           (n);
   mn_na.extend              (n);
   mn_oerr_na.extend         (n);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::assign(const PairDataEnsemble &pd) {
   int i;

   clear();

   set_mask_name(pd.mask_name.c_str());
   set_mask_area_ptr(pd.mask_area_ptr);
   set_msg_typ(pd.msg_typ.c_str());
   set_msg_typ_vals(pd.msg_typ_vals);

   set_interp_mthd(pd.interp_mthd);
   set_interp_wdth(pd.interp_wdth);
   set_interp_shape(pd.interp_shape);

   // PairBase
   n_obs          = pd.n_obs;
   sid_sa         = pd.sid_sa;
   lat_na         = pd.lat_na;
   lon_na         = pd.lon_na;
   x_na           = pd.x_na;
   y_na           = pd.y_na;
   wgt_na         = pd.wgt_na;
   vld_ta         = pd.vld_ta;
   lvl_na         = pd.lvl_na;
   elv_na         = pd.elv_na;
   o_na           = pd.o_na;
   o_qc_sa        = pd.o_qc_sa;

   cdf_info_ptr   = pd.cdf_info_ptr;

   fcmn_na        = pd.fcmn_na;
   fcsd_na        = pd.fcsd_na;
   ocmn_na        = pd.ocmn_na;
   ocsd_na        = pd.ocsd_na;
   ocdf_na        = pd.ocdf_na;

   // PairDataEnsemble
   v_na             = pd.v_na;
   r_na             = pd.r_na;

   crps_emp_na      = pd.crps_emp_na;
   crps_emp_fair_na = pd.crps_emp_fair_na;
   spread_md_na     = pd.spread_md_na;
   crpscl_emp_na    = pd.crpscl_emp_na;
   crps_gaus_na     = pd.crps_gaus_na;
   crpscl_gaus_na   = pd.crpscl_gaus_na;

   ign_na           = pd.ign_na;
   pit_na           = pd.pit_na;

   ign_conv_oerr_na = pd.ign_conv_oerr_na;
   ign_corr_oerr_na = pd.ign_corr_oerr_na;

   n_ge_obs_na    = pd.n_ge_obs_na;
   me_ge_obs_na   = pd.me_ge_obs_na;
   n_lt_obs_na    = pd.n_lt_obs_na;
   me_lt_obs_na   = pd.me_lt_obs_na;

   n_pair         = pd.n_pair;
   ctrl_index     = pd.ctrl_index;
   skip_const     = pd.skip_const;
   skip_ba        = pd.skip_ba;

   var_na           = pd.var_na;
   var_oerr_na      = pd.var_oerr_na;
   var_plus_oerr_na = pd.var_plus_oerr_na;

   esum_na        = pd.esum_na;
   esumsq_na      = pd.esumsq_na;
   esumn_na       = pd.esumn_na;

   mn_na          = pd.mn_na;
   mn_oerr_na     = pd.mn_oerr_na;

   rhist_na       = pd.rhist_na;
   relp_na        = pd.relp_na;
   phist_na       = pd.phist_na;

   if(pd.ssvar_bins){
      ssvar_bins = new SSVARInfo[pd.ssvar_bins[0].n_bin];
      for(i=0; i < pd.ssvar_bins[0].n_bin; i++){
         ssvar_bins[i] = pd.ssvar_bins[i];
      }
   } else ssvar_bins = 0;

   ssvar_bin_size = pd.ssvar_bin_size;
   phist_bin_size = pd.phist_bin_size;

   crpss_emp      = pd.crpss_emp;
   crpss_gaus     = pd.crpss_gaus;

   me             = pd.me;
   mae            = pd.mae;
   rmse           = pd.rmse;
   me_oerr        = pd.me_oerr;
   mae_oerr       = pd.mae_oerr;
   rmse_oerr      = pd.rmse_oerr;

   bias_ratio     = pd.bias_ratio;

   set_ens_size(pd.n_ens);

   for(i=0; i<n_ens; i++) e_na[i] = pd.e_na[i];

   obs_error_entry = pd.obs_error_entry;
   obs_error_flag  = pd.obs_error_flag;

   return;
}

////////////////////////////////////////////////////////////////////////

bool PairDataEnsemble::has_obs_error() const {
   return obs_error_flag;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::add_ens(int member, double v) {

   e_na[member].add(v);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::add_ens_var_sums(int i_obs, double v) {

   // Initialize new sums to 0
   if(i_obs >= esum_na.n()) {
      esum_na.add(0.0);
      esumsq_na.add(0.0);
      esumn_na.add(0.0);
   }

   // Track sums of the raw ensemble member values
   if(!is_bad_data(v)) {
      esum_na.inc(i_obs, v);
      esumsq_na.inc(i_obs, v*v);
      esumn_na.inc(i_obs, 1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::set_ens_size(int n) {

   // Allocate a NumArray to store ensemble values for each member
   n_ens = n;
   e_na  = new NumArray [n_ens];

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::add_obs_error_entry(ObsErrorEntry *e) {

   obs_error_entry.add(e);

   if(e != 0) obs_error_flag = true;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_pair_vals(const gsl_rng *rng_ptr) {
   int i, j, k, n_vld, n_bel, n_tie;
   int n_skip_const, n_skip_vld;
   NumArray src_na, dest_na, cur_ens, cur_clm;
   double mean, stdev, var_unperturbed, var_perturbed;

   // Check if the ranks have already been computed
   if(r_na.n() == o_na.n()) return;

   // Print the observation climo data being used
   bool ocmn_flag = set_climo_flag(o_na, ocmn_na);
   bool ocsd_flag = set_climo_flag(o_na, ocsd_na);

   if(ocmn_flag && cdf_info_ptr && cdf_info_ptr->cdf_ta.n() == 2) {
      mlog << Debug(3)
           << "Computing ensemble statistics relative to the "
           << "observation climatological mean.\n";
   }
   else if(ocmn_flag    &&
           ocsd_flag    &&
           cdf_info_ptr &&
           cdf_info_ptr->cdf_ta.n() > 2) {
      mlog << Debug(3)
           << "Computing ensemble statistics relative to a "
           << cdf_info_ptr->cdf_ta.n() - 2
           << "-member observation climatological ensemble.\n";
   }
   else {
      mlog << Debug(3)
           << "No reference observation climatology data provided.\n";
   }

   // Compute the rank for each observation
   for(i=0, n_pair=0, n_skip_const=0, n_skip_vld=0; i<o_na.n(); i++) {

      // Initialize
      cur_ens.erase();

      // Compute the number of ensemble values above and below the observation
      for(j=0, n_vld = n_bel = n_tie = 0; j<n_ens; j++) {

         // Skip bad data
         if(e_na[j].n() > i && !is_bad_data(e_na[j][i])) {

            // Increment the valid count
            n_vld++;

            // Store the current ensemble value
            cur_ens.add(e_na[j][i]);

            // Track running counts and sums
            if(is_eq(e_na[j][i], o_na[i])) n_tie++;
            else if(e_na[j][i] < o_na[i])  n_bel++;
         }

      } // end for j

      // Store the number of valid ensemble values
      v_na.add(n_vld);

      // Skip points missing ensemble data
      if(n_vld != n_ens) {
         n_skip_vld++;
         skip_ba.add(true);
      }
      // Skip points with constant value, if requested
      else if(skip_const && n_tie == n_ens) {
         n_skip_const++;
         skip_ba.add(true);
      }
      // Increment the n_pair counter
      else {
         n_pair++;
         skip_ba.add(false);
      }

      // Store bad data values if skipping this point
      if(skip_ba[i]) {
         var_na.add(bad_data_double);
         mn_oerr_na.add(bad_data_double);
         var_oerr_na.add(bad_data_double);
         var_plus_oerr_na.add(bad_data_double);
         r_na.add(bad_data_int);
         crps_emp_na.add(bad_data_double);
         crps_emp_fair_na.add(bad_data_double);
         spread_md_na.add(bad_data_double);
         crpscl_emp_na.add(bad_data_double);
         crps_gaus_na.add(bad_data_double);
         crpscl_gaus_na.add(bad_data_double);
         ign_na.add(bad_data_double);
         pit_na.add(bad_data_double);
         ign_conv_oerr_na.add(bad_data_double);
         ign_corr_oerr_na.add(bad_data_double);
         n_ge_obs_na.add(bad_data_double);
         me_ge_obs_na.add(bad_data_double);
         n_lt_obs_na.add(bad_data_double);
         me_lt_obs_na.add(bad_data_double);
      }
      // Otherwise, compute scores
      else {

         // Compute the variance of the unperturbed ensemble members
         var_unperturbed = compute_variance(esum_na[i], esumsq_na[i], esumn_na[i]);
         var_na.add(var_unperturbed);

         // Process the observation error information
         ObsErrorEntry * e = (has_obs_error() ? obs_error_entry[i] : 0);
         if(e) {

            // Get observation error variance
            double oerr_var = e->variance();

            // Compute the observation error log scores
            double v_conv, v_corr;
            compute_obs_error_log_scores(
               compute_mean(esum_na[i], esumn_na[i]),
               compute_stdev(esum_na[i], esumsq_na[i], esumn_na[i]),
               o_na[i], oerr_var,
               v_conv, v_corr);
            ign_conv_oerr_na.add(v_conv);
            ign_corr_oerr_na.add(v_corr);

            // Compute perturbed ensemble mean and variance
            // Exclude the control member from the variance
            mn_oerr_na.add(cur_ens.mean());
            var_oerr_na.add(cur_ens.variance(ctrl_index));

            // Compute the variance plus observation error variance
            if(is_bad_data(var_unperturbed) ||
               is_bad_data(oerr_var)) {
               var_plus_oerr_na.add(bad_data_double);
            }
            else {
               var_plus_oerr_na.add(var_unperturbed + oerr_var);
            }
         }
         // If no observation error specified, store bad data values
         else {
            ign_conv_oerr_na.add(bad_data_double);
            ign_corr_oerr_na.add(bad_data_double);
            mn_oerr_na.add(bad_data_double);
            var_oerr_na.add(bad_data_double);
            var_plus_oerr_na.add(bad_data_double);
         }

         // With no ties, the rank is the number below plus 1
         if(n_tie == 0) {
            r_na.add(n_bel+1);
         }
         // With ties present, randomly assign the rank in:
         //    [n_bel+1, n_bel+n_tie+1]
         else {

            // Initialize
            dest_na.clear();
            src_na.clear();
            for(k=n_bel+1; k<=n_bel+n_tie+1; k++) src_na.add(k);

            // Randomly choose one of the ranks
            ran_choose(rng_ptr, src_na, dest_na, 1);

            // Store the rank
            r_na.add(nint(dest_na[0]));
         }

         // Derive ensemble from observation climo mean and standard deviation
         derive_climo_vals(cdf_info_ptr, ocmn_na[i], ocsd_na[i], cur_clm);

         // Store empirical CRPS stats
         // For crps_emp use temporary, local variable so we can use it
         // for the crps_emp_fair calculation
         double crps_emp = compute_crps_emp(o_na[i], cur_ens);
         crps_emp_na.add(crps_emp);
         crps_emp_fair_na.add(crps_emp - cur_ens.wmean_abs_diff());
         spread_md_na.add(cur_ens.mean_abs_diff());
         crpscl_emp_na.add(compute_crps_emp(o_na[i], cur_clm));

         // Ensemble mean and standard deviation
         // Exclude the control member from the standard deviation
         mean  = cur_ens.mean();
         stdev = cur_ens.stdev(ctrl_index);

         // Store Gaussian CRPS stats
         crps_gaus_na.add(compute_crps_gaus(o_na[i], mean, stdev));
         crpscl_gaus_na.add(compute_crps_gaus(o_na[i], ocmn_na[i], ocsd_na[i]));
         ign_na.add(compute_ens_ign(o_na[i], mean, stdev));
         pit_na.add(compute_ens_pit(o_na[i], mean, stdev));

         // Compute the Bias Ratio terms 
         int n_ge_obs, n_lt_obs;
         double me_ge_obs, me_lt_obs;
         compute_bias_ratio_terms(
            o_na[i], cur_ens,
            n_ge_obs, me_ge_obs,
            n_lt_obs, me_lt_obs);

         // Store the Bias Ratio terms 
         n_ge_obs_na.add(n_ge_obs);
         me_ge_obs_na.add(me_ge_obs);
         n_lt_obs_na.add(n_lt_obs);
         me_lt_obs_na.add(me_lt_obs);
      }
   } // end for i

   if(n_skip_vld > 0) {
      mlog << Debug(2)
           << "Skipping " << n_skip_vld << " of " << o_na.n()
           << " points due to missing ensemble values.\n";
   }

   if(skip_const) {
      mlog << Debug(2)
           << "Skipping " << n_skip_const << " of " << o_na.n()
           << " points with constant value.\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_rhist() {
   int i, rank;

   // Clear the ranked histogram
   rhist_na.clear();

   // Initialize the histogram counts to 0
   for(i=0; i<=n_ens; i++) rhist_na.add(0);

   // The compute_pair_vals() routine should have already been called.
   // Loop through the ranks and populate the histogram.
   for(i=0; i<r_na.n(); i++) {

      // Get the current rank
      rank = nint(r_na[i]);

      // Increment the histogram counts
      if(!is_bad_data(rank)) rhist_na.set(rank-1, rhist_na[rank-1]+1);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_relp() {
   int i, j, n;
   double d, min_d;
   NumArray min_ens;

   // Clear the RELP histogram
   relp_na.clear();

   // Allocate space
   min_ens.extend(n_ens);

   // Initialize counts to 0
   for(i=0; i<n_ens; i++) relp_na.add(0);

   // Loop through the observations and update the counts
   for(i=0; i<o_na.n(); i++) {

      if(skip_ba[i]) continue;

      // Search for the minimum difference
      for(j=0, min_d=1.0e10; j<n_ens; j++) {

         // Absolute value of the difference
         d = abs(e_na[j][i] - o_na[i]);

         // Store the closest member
         if(d < min_d) {
            min_ens.erase();
            min_ens.add(j);
            min_d = d;
         }
         // Store all members tied for closest
         else if(is_eq(d, min_d)) {
            min_ens.add(j);
         }
      } // end for j

      // Increment fractional RELP counts for each closest member
      for(j=0, n=min_ens.n(); j<n; j++) {
         relp_na.set(min_ens[j], relp_na[(min_ens[j])] + (double) 1.0/n);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_phist() {
   int i, bin;

   // Clear the PIT histogram
   phist_na.clear();

   // Initialize the histogram counts to 0
   for(i=0; i<ceil(1.0/phist_bin_size); i++) phist_na.add(0);

   // The compute_pair_vals() routine should have already been called.
   // Loop through the PIT values and populate the histogram.
   for(i=0; i<pit_na.n(); i++) {

      if(skip_ba[i] || is_bad_data(pit_na[i])) continue;

      if(pit_na[i] < 0.0 || pit_na[i] > 1.0) {
         mlog << Warning << "\nPairDataEnsemble::compute_phist() -> "
              << "probability integral transform value ("
              << pit_na[i] << ") is outside of valid range [0, 1].\n\n";
         continue;
      }

      // Determine the bin
      bin = (is_eq(pit_na[i], 1.0) ?
             phist_na.n() - 1 : floor(pit_na[i]/phist_bin_size));

      // Increment the histogram counts
      phist_na.set(bin, phist_na[bin]+1);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

// Comparison method for ssvar bins
struct ssvar_bin_comp {
  bool operator() (const string& lhs, const string& rhs) const {
     return atof(lhs.data()) < atof(rhs.data());
  }
};

void PairDataEnsemble::compute_ssvar() {
   int i, j;
   double var;
   ssvar_bin_map bins;
   NumArray cur;

   // SSVAR requires valid ensemble mean input
   // HiRA stores the ensemble mean as bad data
   if(mn_na.is_const(bad_data_double)) return;

   // Check number of points
   if(o_na.n() != mn_na.n()) {
      mlog << Error << "\nPairDataEnsemble::compute_ssvar() -> "
           << "the number of ensemble mean points ("
           << mn_na.n()
           << ") should match the number of observation points ("
           << o_na.n() << ")!\n\n";
      exit(1);
   }
   for(j=0; j<n_ens; j++) {
      if(o_na.n() != e_na[j].n()) {
         mlog << Error << "\nPairDataEnsemble::compute_ssvar() -> "
              << "the number of ensemble member " << j+1 << " points ("
              << e_na[j].n()
              << ") should match the number of observation points ("
              << o_na.n() << ")!\n\n";
         exit(1);
      }
   }

   // Compute the variance of ensemble member values at each point
   for(i=0; i<o_na.n(); i++) {

      // Check if point should be skipped
      if(skip_ba[i]) continue;

      // Store ensemble values for the current observation
      // Exclude the control member from the variance
      for(j=0, cur.erase(); j<n_ens; j++) {
         if(j == ctrl_index) continue;
         cur.add(e_na[j][i]);
      }

      // Build a variance point
      ens_ssvar_pt pt;
      pt.var = cur.variance();
      pt.f   = mn_na[i];
      pt.o   = o_na[i];
      pt.w   = wgt_na[i];

      // Determine the bin for the current point and add it to the list
      // Bins are defined starting at 0 and are left-closed, right-open
      j = floor(pt.var/ssvar_bin_size);
      string ssvar_min = str_format("%.5e", j*ssvar_bin_size).contents();
      if( !bins.count(ssvar_min) ){
         ssvar_pt_list pts;
         pts.push_back(pt);
         bins[ssvar_min] = pts;

         // Print warning for too many bins
         if(bins.size() == n_warn_ssvar_bins) {
            mlog << Warning << "\nPairDataEnsemble::compute_ssvar() -> "
                 << "writing at least " << n_warn_ssvar_bins
                 << " SSVAR output lines. Increase the ssvar_bin_size "
                 << "config file setting for this variable to reduce "
                 << "the number of variance bins.\n\n";
         }

      } else {
         bins[ssvar_min].push_back(pt);
      }

   } // end for i

   // Sort the bins
   set<string,ssvar_bin_comp> sorted_bins;
   for(auto &x : bins) sorted_bins.insert(x.first);

   // Report the number of bins built
   int n_bin = sorted_bins.size();
   mlog << Debug(4) << "PairDataEnsemble::compute_ssvar() - "
        << "Built " << n_bin << " variance spread/skill bins from "
        << o_na.n() << " observations\n";

   // Check for no bins
   if(n_bin == 0) return;

   // Build a list of SSVARInfo objects
   ssvar_bins = new SSVARInfo[n_bin];
   i=0;
   for( set<string>::iterator set_it = sorted_bins.begin();
        set_it != sorted_bins.end(); set_it++, i++ ){

      ssvar_pt_list* pts = &( bins[*set_it] );
      var = 0;
      double f = 0, o = 0, fo = 0, ff = 0, oo = 0, w = 0;

      for(j=0; j < (int)pts->size(); j++){
         var += (*pts)[j].w * (*pts)[j].var;
         f   += (*pts)[j].w * (*pts)[j].f;
         o   += (*pts)[j].w * (*pts)[j].o;
         fo  += (*pts)[j].w * (*pts)[j].f * (*pts)[j].o;
         ff  += (*pts)[j].w * (*pts)[j].f * (*pts)[j].f;
         oo  += (*pts)[j].w * (*pts)[j].o * (*pts)[j].o;
         w   += (*pts)[j].w;
      }

      ssvar_bins[i].n_bin    = n_bin;
      ssvar_bins[i].bin_i    = i;
      ssvar_bins[i].bin_n    = pts->size();

      ssvar_bins[i].var_min  = atof( (*set_it).data() );
      ssvar_bins[i].var_max  = ssvar_bins[i].var_min + ssvar_bin_size;
      ssvar_bins[i].var_mean = var / w;

      ssvar_bins[i].sl1l2_info.scount = pts->size();
      ssvar_bins[i].sl1l2_info.fbar   = f  / w;
      ssvar_bins[i].sl1l2_info.obar   = o  / w;
      ssvar_bins[i].sl1l2_info.fobar  = fo / w;
      ssvar_bins[i].sl1l2_info.ffbar  = ff / w;
      ssvar_bins[i].sl1l2_info.oobar  = oo / w;

      if( i < 100 ){
         mlog << Debug(4) << "  SSVAR[ "
              << "bin_i: " << ssvar_bins[i].bin_i << "  "
              << "bin_n: " << ssvar_bins[i].bin_n << "  "
              << "var: (" << ssvar_bins[i].var_min << ", "
                          << ssvar_bins[i].var_max << ")  "
              << "fbar: " << ssvar_bins[i].sl1l2_info.fbar << "  "
              << "obar: " << ssvar_bins[i].sl1l2_info.obar << " ]\n";
      } else if( i == 100 ){
         mlog << Debug(4) << "  SSVAR message 101 through "
              << n_bin << " omitted\n";
      }

   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Apply conditional observation threshold and return a subset of pairs.
// The compute_pair_vals() functions should have already been called.
// After retrieving the subset, the compute statistics functions should
// be called again.
//
////////////////////////////////////////////////////////////////////////

PairDataEnsemble PairDataEnsemble::subset_pairs_obs_thresh(const SingleThresh &ot) const {

   // Check for no work to be done
   if(ot.get_type() == thresh_na) return *this;

   int i, j;
   PairDataEnsemble pd;

   // Set the ensemble size and allocate memory
   pd.set_ens_size(n_ens);
   pd.extend(n_obs);
   pd.phist_bin_size  = phist_bin_size;
   pd.ssvar_bin_size  = ssvar_bin_size;
   pd.obs_error_entry = obs_error_entry;
   pd.obs_error_flag  = obs_error_flag;
   pd.cdf_info_ptr    = cdf_info_ptr;

   bool fcmn_flag = set_climo_flag(o_na, fcmn_na);
   bool fcsd_flag = set_climo_flag(o_na, fcsd_na);
   bool ocmn_flag = set_climo_flag(o_na, ocmn_na);
   bool ocsd_flag = set_climo_flag(o_na, ocsd_na);
   bool wgt_flag  = set_climo_flag(o_na, wgt_na);

   // Loop over the pairs
   for(i=0; i<n_obs; i++) {

      // Store climo data
      ClimoPntInfo cpi(fcmn_na[i], fcsd_na[i], ocmn_na[i], ocsd_na[i]);

      // Check for bad data and apply observation threshold
      if(is_bad_data(o_na[i])                   ||
         skip_ba[i]                             ||
         (fcmn_flag && is_bad_data(fcmn_na[i])) ||
         (fcsd_flag && is_bad_data(fcsd_na[i])) ||
         (ocmn_flag && is_bad_data(ocmn_na[i])) ||
         (ocsd_flag && is_bad_data(ocsd_na[i])) ||
         (wgt_flag && is_bad_data(wgt_na[i]))   ||
         !ot.check(o_na[i], &cpi)) continue;

      // Add data for the current observation but only include data
      // required for ensemble output line types.
      //
      // Include in subset:
      //   wgt_na, o_na, fcmn_na, fcsd_na, ocmn_na, ocsd_na, ocdf_na, v_na, r_na,
      //   crps_emp_na, crps_emp_fair_na, spread_md_na,
      //   crpscl_emp_na, crps_gaus_na, crpscl_gaus_na,
      //   ign_na, pit_na,
      //   ign_conv_oerr, ign_corr_oerr,
      //   n_gt_obs_na, me_gt_obs_na, n_lt_obs_na, me_lt_obs_na,
      //   var_na, var_oerr_na, var_plus_oerr_na,
      //   mn_na, mn_oerr_na, e_na
      //
      // Exclude from subset:
      //   sid_sa, lat_na, lon_na, x_na, y_na, vld_ta, lvl_ta, elv_ta,
      //   o_qc_sa, esum_na, esumsq_na, esumn_na

      pd.wgt_na.add(wgt_na[i]);
      pd.o_na.add(o_na[i]);
      pd.fcmn_na.add(fcmn_na[i]);
      pd.fcsd_na.add(fcsd_na[i]);
      pd.ocmn_na.add(ocmn_na[i]);
      pd.ocsd_na.add(ocsd_na[i]);
      pd.ocdf_na.add(ocdf_na[i]);
      pd.v_na.add(v_na[i]);
      pd.r_na.add(r_na[i]);
      pd.crps_emp_na.add(crps_emp_na[i]);
      pd.crps_emp_fair_na.add(crps_emp_fair_na[i]);
      pd.spread_md_na.add(spread_md_na[i]);
      pd.crpscl_emp_na.add(crpscl_emp_na[i]);
      pd.crps_gaus_na.add(crps_gaus_na[i]);
      pd.crpscl_gaus_na.add(crpscl_gaus_na[i]);
      pd.ign_na.add(ign_na[i]);
      pd.pit_na.add(pit_na[i]);
      pd.ign_conv_oerr_na.add(ign_conv_oerr_na[i]);
      pd.ign_corr_oerr_na.add(ign_corr_oerr_na[i]);
      pd.n_ge_obs_na.add(n_ge_obs_na[i]);
      pd.me_ge_obs_na.add(me_ge_obs_na[i]);
      pd.n_lt_obs_na.add(n_lt_obs_na[i]);
      pd.me_lt_obs_na.add(me_lt_obs_na[i]);
      pd.skip_ba.add(false);
      pd.var_na.add(var_na[i]);
      pd.var_oerr_na.add(var_oerr_na[i]);
      pd.var_plus_oerr_na.add(var_plus_oerr_na[i]);
      pd.mn_na.add(mn_na[i]);
      pd.mn_oerr_na.add(mn_oerr_na[i]);

      for(j=0; j<n_ens; j++) pd.e_na[j].add(e_na[j][i]);

      // Increment counters
      pd.n_obs++;
      pd.n_pair++;

   } // end for i

   mlog << Debug(3)
        << "Using " << pd.n_obs << " of " << n_obs
        << " ensemble pairs for observation filtering threshold "
        << ot.get_str() << ".\n";

   return pd;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class VxPairDataEnsemble
//
////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble::VxPairDataEnsemble() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble::~VxPairDataEnsemble() {
   clear();
}

////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble::VxPairDataEnsemble(const VxPairDataEnsemble &vx_pd) {

   init_from_scratch();

   assign(vx_pd);
}

////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble & VxPairDataEnsemble::operator=(const VxPairDataEnsemble &vx_pd) {

   if(this == &vx_pd) return *this;

   assign(vx_pd);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::init_from_scratch() {

   VxPairBase::init_from_scratch();

   ens_info = (EnsVarInfo *) nullptr;
   obs_info = (VarInfo *)    nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::clear() {

   VxPairBase::clear();

   if(ens_info) { delete ens_info; ens_info = (EnsVarInfo *) nullptr; }
   if(obs_info) { delete obs_info; obs_info = (VarInfo *)    nullptr; }

   obs_error_info = (ObsErrorInfo *) nullptr;

   pd.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::assign(const VxPairDataEnsemble &vx_pd) {

   clear();

   VxPairBase::assign(vx_pd);

   set_ens_info(vx_pd.ens_info);
   set_obs_info(vx_pd.obs_info);

   obs_error_info = vx_pd.obs_error_info;

   set_size(vx_pd.n_msg_typ, vx_pd.n_mask, vx_pd.n_interp);

   pd = vx_pd.pd;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_ens_info(const EnsVarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(ens_info) { delete ens_info; ens_info = (EnsVarInfo *) nullptr; }

   // Perform a deep copy
   ens_info = new EnsVarInfo(*info);

   // Set the base pointer
   if(!fcst_info) set_fcst_info(ens_info->get_var_info());

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_size(int types, int masks, int interps) {

   VxPairBase::set_size(types, masks, interps);

   // Resize the PairDataPoint vector
   pd.resize(n_vx);

   // Set PairBase pointers to the PairDataEnsemble objects
   for(int i=0; i<n_vx; i++) pb_ptr[i] = &pd[i];

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_ens_size(int n) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairDataEnsemble::set_ens_size() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto it = pd.begin(); it != pd.end(); it++) {

      // Handle HiRA neighborhoods
      if(it->interp_mthd == InterpMthd::HiRA) {
         GridTemplateFactory gtf;
         GridTemplate* gt = gtf.buildGT(it->interp_shape,
                                        it->interp_wdth,
                                        false);
         it->set_ens_size(n*gt->size());
      }
      else {
         it->set_ens_size(n);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_ssvar_bin_size(double ssvar_bin_size) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairDataEnsemble::set_ssvar_bin_size() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pd) x.ssvar_bin_size = ssvar_bin_size;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_phist_bin_size(double phist_bin_size) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairDataEnsemble::set_phist_bin_size() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pd) x.phist_bin_size = phist_bin_size;

   return;
}


////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_ctrl_index(int index) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairDataEnsemble::set_ctrl_index() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pd) x.ctrl_index = index;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_skip_const(bool tf) {

   if(n_vx == 0) {
      mlog << Warning << "\nVxPairDataEnsemble::set_skip_const() -> "
           << "set_size() has not been called yet!\n\n";
   }

   for(auto &x : pd) x.skip_const = tf;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::add_point_obs(float *hdr_arr, int *hdr_typ_arr,
                                       const char *hdr_typ_str,
                                       const char *hdr_sid_str,
                                       unixtime hdr_ut,
                                       const char *obs_qty, float *obs_arr,
                                       const Grid &gr, const char *var_name) {

   // Check the observation VarInfo file type
   if(obs_info->file_type() != FileType_Gb1) {
      mlog << Error << "\nVxPairDataEnsemble::add_point_obs() -> "
           << "when processing point observations, the observation "
           << "VarInfo type must be GRIB.\n\n";
      exit(1);
   }

   // Create VarInfoGrib pointer
   VarInfoGrib *obs_info_grib = (VarInfoGrib *) obs_info;

   // Increment the number of tries count
   n_try++;

   // Point observation summary string for rejection log messages
   ConcatString pnt_obs_str;
   if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
      pnt_obs_str = point_obs_to_string(hdr_arr, hdr_typ_str, hdr_sid_str,
                                        hdr_ut, obs_qty, obs_arr, var_name);
   }

   // Check the station ID
   if(!is_keeper_sid(pnt_obs_str.c_str(), hdr_sid_str)) return;

   // Check observation variable
   if(!is_keeper_var(pnt_obs_str.c_str(), var_name, nint(obs_arr[1]))) return;

   // Check observation quality
   if(!is_keeper_qty(pnt_obs_str.c_str(), obs_qty)) return;

   // Check valid time
   if(!is_keeper_vld(pnt_obs_str.c_str(), hdr_ut)) return;

   // Check observation value
   double obs_v = obs_arr[4];
   if(!is_keeper_obs(pnt_obs_str.c_str(), obs_v)) return;

   // Check location
   double hdr_lat = hdr_arr[0];
   double hdr_lon = hdr_arr[1];
   double obs_x, obs_y;
   if(!is_keeper_grd(pnt_obs_str.c_str(), gr, hdr_lat, hdr_lon, obs_x, obs_y)) return;

   // TODO: Add topography filtering to Ensemble-Stat

   // Check topo
   double hdr_elv = hdr_arr[2];
   if(!is_keeper_topo(pnt_obs_str.c_str(), gr, obs_x, obs_y,
                      hdr_typ_str, hdr_elv)) return;

   // Check level
   double obs_lvl = obs_arr[2];
   double obs_hgt = obs_arr[3];
   if(!is_keeper_lvl(pnt_obs_str.c_str(), hdr_typ_str, obs_lvl, obs_hgt)) return;

   // When verifying a vertical level forecast against a surface message type,
   // set the observation level value to bad data so that it's not used in the
   // duplicate logic.
   if(obs_info->level().type() == LevelType_Vert &&
      msg_typ_sfc.reg_exp_match(hdr_typ_str)) {
      obs_lvl = bad_data_double;
   }

   // Set flags
   bool spfh_flag = fcst_info->is_specific_humidity() &&
                    obs_info->is_specific_humidity();

   // Store pointer to ObsErrorEntry
   ObsErrorEntry *oerr_ptr = (ObsErrorEntry *) nullptr;
   if(obs_error_info->flag) {

      // Use config file setting, if specified
      if(obs_error_info->entry.dist_type != DistType::None) {
         oerr_ptr = &(obs_error_info->entry);
      }
      // Otherwise, do a table lookup
      else {

         // Check for table entries for this variable and message type
         if(!obs_error_table.has(obs_info->name().c_str(), hdr_typ_str)) {
            mlog << Warning << "\nVxPairDataEnsemble::add_point_obs() -> "
                 << "Disabling observation error logic since the "
                 << "obs error table contains no entry for OBS_VAR("
                 << obs_info->name() << ") and MESSAGE_TYPE("
                 << hdr_typ_str << ").\nSpecify a custom obs error "
                 << "table using the MET_OBS_ERROR_TABLE environment "
                 << "variable.\n\n";
            obs_error_info->flag = false;
         }
         else {
            oerr_ptr = obs_error_table.lookup(
               obs_info->name().c_str(), hdr_typ_str, hdr_sid_str,
               hdr_typ_arr[0], hdr_typ_arr[1], hdr_typ_arr[2],
               obs_lvl, obs_hgt, obs_v);
         }
      }
   }

   // Apply observation error additive and multiplicative
   // bias correction, if requested
   if(obs_error_info->flag) {
      obs_v = add_obs_error_bc(obs_error_info->rng_ptr,
                               FieldType::Obs, oerr_ptr, obs_v);
   }

   // Loop through the message types
   for(int i_msg_typ=0; i_msg_typ<n_msg_typ; i_msg_typ++) {

      // Check message type
      if(!is_keeper_typ(pnt_obs_str.c_str(), i_msg_typ, hdr_typ_str)) continue;

      int x = nint(obs_x);
      int y = nint(obs_y);

      // Loop through the masking regions
      for(int i_mask=0; i_mask<n_mask; i_mask++) {

         // Check masking region
         if(!is_keeper_mask(pnt_obs_str.c_str(), i_msg_typ, i_mask, x, y,
                            hdr_sid_str, hdr_lat, hdr_lon)) continue;

         // Loop through the interpolation methods
         for(int i_interp=0; i_interp<n_interp; i_interp++) {

            // Check climatology values
            ClimoPntInfo cpi;
            if(!is_keeper_climo(pnt_obs_str.c_str(), i_msg_typ, i_mask, i_interp,
                                gr, obs_x, obs_y, obs_v, obs_lvl, obs_hgt,
                                cpi)) continue;

            // Add the observation value
            // Weight is from the nearest grid point
            int n = three_to_one(i_msg_typ, i_mask, i_interp);
            if(!pd[n].add_point_obs(hdr_sid_str, hdr_lat, hdr_lon,
                  obs_x, obs_y, hdr_ut, obs_lvl, obs_hgt,
                  obs_v, obs_qty, cpi, default_weight)) {

               if(mlog.verbosity_level() >= REJECT_DEBUG_LEVEL) {
                  mlog << Debug(REJECT_DEBUG_LEVEL)
                       << "For " << fcst_info->magic_str()
                       << " versus " << obs_info->magic_str()
                       << ", skipping observation since it is a duplicate:\n"
                       << pnt_obs_str << "\n";
               }

               inc_count(rej_dup, i_msg_typ, i_mask, i_interp);
               continue;
            }

            // Store the observation error pointer
            pd[n].add_obs_error_entry(oerr_ptr);

         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::add_ens(int member, bool mn, Grid &gr) {

   // Set flag for specific humidity
   bool spfh_flag = fcst_info->is_specific_humidity() &&
                    obs_info->is_specific_humidity();

   // Loop through all the PairDataEnsemble objects and interpolate
   for(auto it = pd.begin(); it != pd.end(); it++) {

      // Only apply HiRA to single levels
      if(it->interp_mthd == InterpMthd::HiRA &&
         fcst_dpa.n_planes() != 1 ) {

         mlog << Warning << "\nVxPairDataEnsemble::add_ens() -> "
              << "the \"" << interpmthd_to_string(it->interp_mthd)
              << "\" interpolation method only applies when verifying a "
              << "single level, not " << fcst_dpa.n_planes()
              << " levels.\n\n";
            continue;
      }

      // Process each of the observations
      NumArray fcst_na;
      for(int i_obs=0; i_obs<it->n_obs; i_obs++) {

         // Initialize
         fcst_na.erase();

         // Interpolate using the observation pressure level or height
         double to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                          it->lvl_na[i_obs] : it->elv_na[i_obs]);
         int lvl_blw, lvl_abv;

         // For a single forecast field
         if(fcst_dpa.n_planes() == 1) {
            lvl_blw = 0;
            lvl_abv = 0;
         }
         // For multiple forecast fields, find the levels above
         // and below the observation point.
         else {
            find_vert_lvl(fcst_dpa, to_lvl, lvl_blw, lvl_abv);
         }

         // Extract the HiRA neighborhood of values
         if(it->interp_mthd == InterpMthd::HiRA) {

            // For HiRA, set the ensemble mean to bad data
            if(mn) {
               fcst_na.erase();
               fcst_na.add(bad_data_double);
            }
            // Otherwise, retrieve all the neighborhood values
            // using a valid threshold of 0
            else {
               get_interp_points(fcst_dpa,
                  it->x_na[i_obs], it->y_na[i_obs],
                  it->interp_mthd, it->interp_wdth, it->interp_shape,
                  gr.wrap_lon(), 0, spfh_flag,
                  fcst_info->level().type(),
                  to_lvl, lvl_blw, lvl_abv,
                  fcst_na);
            }
         }
         // Otherwise, get a single interpolated ensemble value
         else {
            ClimoPntInfo cpi(it->fcmn_na[i_obs], it->fcsd_na[i_obs],
                             it->ocmn_na[i_obs], it->ocsd_na[i_obs]);

            fcst_na.add(compute_interp(fcst_dpa,
               it->x_na[i_obs], it->y_na[i_obs], it->o_na[i_obs], &cpi,
               it->interp_mthd, it->interp_wdth, it->interp_shape,
               gr.wrap_lon(), interp_thresh, spfh_flag,
               fcst_info->level().type(),
               to_lvl, lvl_blw, lvl_abv));
         }

         // Store the single ensemble value or HiRA neighborhood
         for(int i_fcst=0; i_fcst<fcst_na.n(); i_fcst++) {

            // Store the ensemble mean
            if(mn) {
               it->mn_na.add(fcst_na[i_fcst]);
            }
            // Store the ensemble member values
            else {

               // Track unperturbed ensemble variance sums
               // Exclude the control member from the variance
               if(member != it->ctrl_index) {
                  it->add_ens_var_sums(i_obs, fcst_na[i_fcst]);
               }

               // Apply observation error perturbation, if requested
               double fcst_v;
               if(obs_error_info->flag) {
                  fcst_v = add_obs_error_inc(
                              obs_error_info->rng_ptr, FieldType::Fcst,
                              it->obs_error_entry[i_obs],
                              it->o_na[i_obs], fcst_na[i_fcst]);
               }
               else {
                  fcst_v = fcst_na[i_fcst];
               }

               // Determine index of ensemble member
               int i_mem = member * fcst_na.n() + i_fcst;

               // Store perturbed ensemble member value
               it->add_ens(i_mem, fcst_v);
            }

         } // end for i_fcst
      } // end for i_obs
   } // end for PairDataEnsemble iterator

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the empirical continuous ranked probability score
//
////////////////////////////////////////////////////////////////////////

double compute_crps_emp(double obs, const NumArray &ens_na) {
   int i;
   double fcst = 0.0;
   NumArray evals;

   // Store valid ensemble member values
   evals.extend(ens_na.n());
   for(i=0; i<ens_na.n(); i++) {
      if(!is_bad_data(ens_na[i])) evals.add(ens_na[i]);
   }
   evals.sort_array();

   // Check for bad or no data
   if(is_bad_data(obs) || evals.n() == 0) return bad_data_double;

   // Initialize
   double obs_cdf  = 0.0;
   double fcst_cdf = 0.0;
   double prv_fcst = 0.0;
   double integral = 0.0;
   double wgt      = 1.0/evals.n();

   // Compute empirical CRPS
   for(i=0; i<evals.n(); i++) {
      fcst = evals[i];
      if(is_eq(obs_cdf, 0.0) && obs < fcst) {
         integral += (obs - prv_fcst) * pow(fcst_cdf, 2);
         integral += (fcst - obs) * pow(fcst_cdf - 1.0, 2);
         obs_cdf   = 1.0;
      }
      else {
         integral += (fcst - prv_fcst) * pow(fcst_cdf - obs_cdf, 2);
      }
      fcst_cdf += wgt;
      prv_fcst  = fcst;
   }

   // Handle obs being >= all ensemble members
   if(is_eq(obs_cdf, 0.0)) integral += (obs - fcst);

   return integral;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the Gaussian CRPS
//
////////////////////////////////////////////////////////////////////////

double compute_crps_gaus(double obs, double m, double s) {
   double v, z;

   if(is_bad_data(m) || is_bad_data(s) || is_eq(s, 0.0)) {
      v = bad_data_double;
   }
   else {
      z = (obs - m)/s;
      v = s*(z*(2.0*znorm(z) - 1) + 2.0*dnorm(z) - 1.0/sqrt(pi));
   }

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the ensemble ignorance score
//
////////////////////////////////////////////////////////////////////////

double compute_ens_ign(double obs, double m, double s) {
   double v;

   if(is_bad_data(m) || is_bad_data(s) || is_eq(s, 0.0)) {
      v = bad_data_double;
   }
   else {
      v = 0.5*log(2.0*pi*s*s) + (obs - m)*(obs - m)/(2.0*s*s);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the ensemble probability integral transform
//
////////////////////////////////////////////////////////////////////////

double compute_ens_pit(double obs, double m, double s) {
   double v;
   
   if(is_bad_data(m) || is_bad_data(s) || is_eq(s, 0.0)) {
      v = bad_data_double;
   }
   else {
      v = normal_cdf(obs, m, s);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the bias ratio terms
//
////////////////////////////////////////////////////////////////////////

void compute_bias_ratio_terms(double obs, const NumArray &e_na,
                              int &n_ge_obs, double &me_ge_obs,
                              int &n_lt_obs, double &me_lt_obs) {

   // Initialize
   n_ge_obs  = n_lt_obs  = 0;
   me_ge_obs = me_lt_obs = 0.0;

   // Loop over ensemble member values
   for(int i=0; i<e_na.n(); i++) {

      // Track counts and sums
      if(e_na[i] >= obs) {
         n_ge_obs  += 1;
         me_ge_obs += (e_na[i] - obs);
      }
      else {
         n_lt_obs  += 1;
         me_lt_obs += (e_na[i] - obs);
      }
   }

   // Convert sums to means
   if(n_ge_obs > 0) me_ge_obs /= n_ge_obs;
   else             me_ge_obs  = bad_data_double;
   if(n_lt_obs > 0) me_lt_obs /= n_lt_obs;
   else             me_lt_obs  = bad_data_double;

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_bias_ratio(double me_ge_obs, double me_lt_obs) {
   double v;

   // Compute bias ratio
   if(is_bad_data(me_ge_obs) ||
      is_bad_data(me_lt_obs) || is_eq(me_lt_obs, 0.0)) {
      v = bad_data_double;
   }
   else {
      v = me_ge_obs / abs(me_lt_obs);
   }

   return v;
}

////////////////////////////////////////////////////////////////////////

void compute_obs_error_log_scores(double emn, double esd,
                                  double obs, double oerr_var,
                                  double &v_conv, double &v_corr) {

   const char *method_name = "compute_obs_error_log_scores() -> ";

   // Check for bad input data
   if(is_bad_data(emn) ||
      is_bad_data(esd) ||
      is_bad_data(obs) ||
      is_bad_data(oerr_var)) {
      v_conv = v_corr = bad_data_double;
   }
   else {
      double sigma2 = esd * esd;
      double ov2    = oerr_var * oerr_var;

      // Error-convolved logarithmic scoring rule in
      // Ferro (2017, Eq 5) doi:10.1002/qj.3115
      // Scale by 2.0 * pi for consistency with ignorance score
      v_conv = 0.5 * log(2.0 * pi * (sigma2 + ov2)) +
               ((obs - emn) * (obs - emn)) /
               (2.0 * (sigma2 + ov2));

      // Error-corrected logarithmic scoring rule in
      // Ferro (2017, Eq 7) doi:10.1002/qj.3115
      // Scale by 2.0 * pi for consistency with ignorance score
      v_corr = 0.5 * log(2.0 * pi * sigma2) +
               ((obs - emn) * (obs - emn) - ov2) /
               (2.0 * sigma2);
   }

   if(mlog.verbosity_level() >= 10) {
      mlog << Debug(10) << method_name
           << "inputs (emn = " << emn
           << ", esd = " << esd
           << ", obs = " << obs
           << ", oerr_var = " << oerr_var
           << ") and outputs (ign_oerr_conv = " << v_conv
           << ", ign_oerr_corr = " << v_corr << ")\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////
