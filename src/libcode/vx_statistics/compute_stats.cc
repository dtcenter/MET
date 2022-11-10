// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

#include "pair_data_point.h"
#include "compute_stats.h"
#include "compute_ci.h"
#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

void compute_cntinfo(const SL1L2Info &s, bool aflag, CNTInfo &cnt_info) {
   double fbar, obar, ffbar, fobar, oobar, den;
   int n;

   // Set the quantities that can't be derived from SL1L2Info to bad data
   cnt_info.sp_corr.set_bad_data();
   cnt_info.kt_corr.set_bad_data();
   cnt_info.e10.set_bad_data();
   cnt_info.e25.set_bad_data();
   cnt_info.e50.set_bad_data();
   cnt_info.e75.set_bad_data();
   cnt_info.e90.set_bad_data();
   cnt_info.eiqr.set_bad_data();
   cnt_info.mad.set_bad_data();
   cnt_info.n_ranks    = 0;
   cnt_info.frank_ties = 0;
   cnt_info.orank_ties = 0;

   // Get partial sums
   n     = (aflag ? s.sacount : s.scount);
   fbar  = (aflag ? s.fabar   : s.fbar);
   obar  = (aflag ? s.oabar   : s.obar);
   fobar = (aflag ? s.foabar  : s.fobar);
   ffbar = (aflag ? s.ffabar  : s.ffbar);
   oobar = (aflag ? s.ooabar  : s.oobar);

   // Number of matched pairs
   cnt_info.n = n;

   // Forecast mean and standard deviation
   cnt_info.fbar.v   = fbar;
   cnt_info.fstdev.v = compute_stdev(fbar*n, ffbar*n, n);

   // Observation mean and standard deviation
   cnt_info.obar.v   = obar;
   cnt_info.ostdev.v = compute_stdev(obar*n, oobar*n, n);

   // Multiplicative bias
   cnt_info.mbias.v = (is_eq(obar, 0.0) ? bad_data_double : fbar/obar);

   // Correlation coefficient

   // Handle SAL1L2 data
   if(aflag) {
      cnt_info.pr_corr.v          = bad_data_double;
      cnt_info.anom_corr.v        = compute_corr( fbar*n,  obar*n,
                                                 ffbar*n, oobar*n,
                                                 fobar*n, n);
      cnt_info.rmsfa.v            = sqrt(ffbar);
      cnt_info.rmsoa.v            = sqrt(oobar);
      cnt_info.anom_corr_uncntr.v = compute_anom_corr_uncntr(ffbar, oobar,
                                                             fobar);
   }
   // Handle SL1L2 data
   else {
      cnt_info.pr_corr.v          = compute_corr( fbar*n,  obar*n,
                                                 ffbar*n, oobar*n,
                                                 fobar*n, n);
      cnt_info.anom_corr.v        = bad_data_double;
      cnt_info.rmsfa.v            = bad_data_double;
      cnt_info.rmsoa.v            = bad_data_double;
      cnt_info.anom_corr_uncntr.v = bad_data_double;
   }

   // Compute mean error
   cnt_info.me.v = fbar - obar;

   // Compute mean error squared
   cnt_info.me2.v = cnt_info.me.v * cnt_info.me.v;

   // Compute mean absolute error
   cnt_info.mae.v = s.mae;

   // Compute mean squared error
   cnt_info.mse.v = ffbar + oobar - 2.0*fobar;

   // Compute mean squared error skill score
   den = cnt_info.ostdev.v * cnt_info.ostdev.v;
   if(!is_eq(den, 0.0)) {
      cnt_info.msess.v = 1.0 - cnt_info.mse.v / den;
   }
   else {
      cnt_info.msess.v = bad_data_double;
   }

   // Compute standard deviation of the mean error
   cnt_info.estdev.v = compute_stdev(cnt_info.me.v*n,
                                     cnt_info.mse.v*n, n);

   // Compute bias corrected mean squared error (decomposition of MSE)
   cnt_info.bcmse.v = cnt_info.mse.v - (fbar - obar)*(fbar - obar);

   // Compute root mean squared error
   cnt_info.rmse.v = sqrt(cnt_info.mse.v);

   // Compute Scatter Index (SI)
   if(!is_eq(cnt_info.obar.v, 0.0)) {
      cnt_info.si.v = cnt_info.rmse.v / cnt_info.obar.v;
   }
   else {
      cnt_info.si.v = bad_data_double;
   }
   
   // Compute normal confidence intervals
   cnt_info.compute_ci();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Assume that the input f_na and o_na contain only valid data.
// Assume that c_na is either empty or contains only valid data.
//
////////////////////////////////////////////////////////////////////////

void compute_cntinfo(const PairDataPoint &pd, const NumArray &i_na,
                     bool precip_flag, bool rank_flag, bool normal_ci_flag,
                     CNTInfo &cnt_info) {
   int i, j, n;
   double f, o, c, wgt, wgt_sum;
   double f_bar, o_bar, ff_bar, oo_bar, fo_bar;
   double fa_bar, oa_bar, ffa_bar, ooa_bar, foa_bar;
   double err, err_bar, abs_err_bar, err_sq_bar, den;
   bool cmn_flag;

   //
   // Allocate memory to store the differences
   //
   NumArray err_na, dev_na;
   err_na.extend(pd.f_na.n());
   dev_na.extend(pd.f_na.n());

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n() || pd.f_na.n() == 0) {
      mlog << Error << "\ncompute_cntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "non-zero length!\n\n";
      throw(1);
   }

   //
   // Flag to process climo
   //
   cmn_flag = set_climo_flag(pd.f_na, pd.cmn_na);

   //
   // Get the sum of the weights
   //
   wgt_sum = pd.wgt_na.sum();

   //
   // Compute the continuous statistics from the fcst and obs arrays
   //
   n       = 0;
   f_bar   = o_bar       = ff_bar     = oo_bar  = fo_bar  = 0.0;
   fa_bar  = oa_bar      = ffa_bar    = ooa_bar = foa_bar = 0.0;
   err_bar = abs_err_bar = err_sq_bar = 0.0;
   for(i=0; i<i_na.n(); i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      f   = pd.f_na[j];
      o   = pd.o_na[j];
      c   = (cmn_flag ? pd.cmn_na[j] : bad_data_double);
      wgt = pd.wgt_na[i]/wgt_sum;

      //
      // Should be no bad data, but checking to be sure
      //
      if(is_bad_data(f) ||
         is_bad_data(o) ||
         (cmn_flag && is_bad_data(c))) continue;

      //
      // Compute the error
      //
      err = f-o;
      err_na.add(err);

      f_bar       += wgt*f;
      o_bar       += wgt*o;
      ff_bar      += wgt*f*f;
      oo_bar      += wgt*o*o;
      fo_bar      += wgt*f*o;
      err_bar     += wgt*err;
      abs_err_bar += wgt*fabs(err);
      err_sq_bar  += wgt*err*err;
      n++;

      if(cmn_flag) {
         fa_bar  += wgt*(f-c);
         oa_bar  += wgt*(o-c);
         foa_bar += wgt*(f-c)*(o-c);
         ffa_bar += wgt*(f-c)*(f-c);
         ooa_bar += wgt*(o-c)*(o-c);
      }
   } // end for i

   //
   // Store the sample size
   //
   if((cnt_info.n = n) == 0) return;

   //
   // Compute forecast mean and standard deviation
   //
   cnt_info.fbar.v   = f_bar;
   cnt_info.fstdev.v = compute_stdev(f_bar*n, ff_bar*n, n);

   //
   // Compute observation mean and standard deviation
   //
   cnt_info.obar.v   = o_bar;
   cnt_info.ostdev.v = compute_stdev(o_bar*n, oo_bar*n, n);

   //
   // Compute multiplicative bias
   //
   if(is_eq(cnt_info.obar.v, 0.0))
      cnt_info.mbias.v = bad_data_double;
   else
      cnt_info.mbias.v = cnt_info.fbar.v/cnt_info.obar.v;

   //
   // Compute Pearson correlation coefficient
   //
   cnt_info.pr_corr.v = compute_corr( f_bar*n,  o_bar*n,
                                     ff_bar*n, oo_bar*n,
                                     fo_bar*n, n);

   //
   // Process anomaly scores
   //
   if(cmn_flag) {

      //
      // Compute Anomaly Correlation
      //
      cnt_info.anom_corr.v        = compute_corr( fa_bar*n,  oa_bar*n,
                                                 ffa_bar*n, ooa_bar*n,
                                                 foa_bar*n, n);

      //
      // Compute RMSFA and RMSOA
      //
      cnt_info.rmsfa.v            = sqrt(ffa_bar);
      cnt_info.rmsoa.v            = sqrt(ooa_bar);

      //
      // Compute Raw (uncentered) Anomaly Correlation
      //
      cnt_info.anom_corr_uncntr.v = compute_anom_corr_uncntr(ffa_bar, ooa_bar,
                                                             foa_bar);
   }
   else {
      cnt_info.anom_corr.v        = bad_data_double;
      cnt_info.rmsfa.v            = bad_data_double;
      cnt_info.rmsoa.v            = bad_data_double;
      cnt_info.anom_corr_uncntr.v = bad_data_double;
   }

   //
   // Compute percentiles of the error
   //
   cnt_info.e10.v  = err_na.percentile_array(0.10);
   cnt_info.e25.v  = err_na.percentile_array(0.25);
   cnt_info.e50.v  = err_na.percentile_array(0.50);
   cnt_info.e75.v  = err_na.percentile_array(0.75);
   cnt_info.e90.v  = err_na.percentile_array(0.90);
   cnt_info.eiqr.v = cnt_info.e75.v - cnt_info.e25.v;

   //
   // Compute the median absolute deviation
   //
   for(i=0; i<err_na.n(); i++) {
      dev_na.add(fabs(err_na[i] - cnt_info.e50.v));
   }
   cnt_info.mad.v = dev_na.percentile_array(0.50);

   //
   // Compute mean error and standard deviation of the mean error
   //
   cnt_info.me.v     = err_bar;
   cnt_info.estdev.v = compute_stdev(n*err_bar, n*err_sq_bar, n);

   //
   // Compute mean error squared
   //
   cnt_info.me2.v = cnt_info.me.v * cnt_info.me.v;

   //
   // Compute mean absolute error
   //
   cnt_info.mae.v = abs_err_bar;

   //
   // Compute mean squared error
   //
   cnt_info.mse.v = err_sq_bar;

   //
   // Compute mean squared error skill score
   //
   den = cnt_info.ostdev.v * cnt_info.ostdev.v;
   cnt_info.msess.v = (is_eq(den, 0.0) ? bad_data_double :
                       1.0 - (cnt_info.mse.v / den));

   //
   // Compute bias corrected mean squared error (decomposition of MSE)
   //
   f = cnt_info.fbar.v;
   o = cnt_info.obar.v;
   cnt_info.bcmse.v = cnt_info.mse.v - (f-o)*(f-o);

   //
   // Compute root mean squared error
   //
   cnt_info.rmse.v = sqrt(err_sq_bar);

   //
   // Compute Scatter Index (SI)
   //
   if(!is_eq(cnt_info.obar.v, 0.0)) {
      cnt_info.si.v = cnt_info.rmse.v / cnt_info.obar.v;
   }
   else {
      cnt_info.si.v = bad_data_double;
   }

   
   //
   // Only compute the Kendall Tau and Spearman's Rank corrleation
   // coefficients if the rank_flag is set.
   //
   if(rank_flag) {
      int concordant, discordant, extra_f, extra_o;
      int n_f_rank, n_o_rank, n_f_rank_ties, n_o_rank_ties;
      NumArray f_na2, o_na2, f_na_rank, o_na_rank, wgt_na2;

      //
      // Allocate memory
      //
      f_na2.extend(pd.f_na.n());
      o_na2.extend(pd.f_na.n());
      wgt_na2.extend(pd.f_na.n());

      for(i=0; i<n; i++) {

         //
         // Get the index to be used from the index num array
         //
         j = nint(i_na[i]);

         //
         // Skip (0, 0) cases for precipitation
         //
         if(precip_flag &&
            is_eq(pd.f_na[j], 0.0) &&
            is_eq(pd.o_na[j], 0.0)) continue;

         f_na2.add(pd.f_na[j]);
         o_na2.add(pd.o_na[j]);
         wgt_na2.add(pd.wgt_na[j]);
      } // end for i

      //
      // Compute ranks of the remaining raw data values
      // in the fcst and obs arrays
      //
      f_na_rank = f_na2;
      o_na_rank = o_na2;
      n_f_rank  = f_na_rank.rank_array(n_f_rank_ties);
      n_o_rank  = o_na_rank.rank_array(n_o_rank_ties);

      if(n_f_rank != n_o_rank) {
         mlog << Error << "\ncompute_cntinfo() -> "
              << "n_f_rank does not equal n_o_rank!\n\n";
         throw(1);
      }
      else {
         n = n_f_rank;
      }

      //
      // Store the number of ranks and ties
      //
      cnt_info.n_ranks    = n;
      cnt_info.frank_ties = n_f_rank_ties;
      cnt_info.orank_ties = n_o_rank_ties;

      //
      // Get the sum of the weights
      //
      wgt_sum = wgt_na2.sum();

      //
      // Compute sums for the ranks for use in computing Spearman's
      // Rank correlation coefficient
      //
      f_bar = o_bar = ff_bar = oo_bar = fo_bar = 0.0;
      for(i=0; i<n_f_rank; i++) {

         f   = f_na_rank[i];
         o   = o_na_rank[i];
         wgt = wgt_na2[i]/wgt_sum;

         f_bar  += wgt*f;
         o_bar  += wgt*o;
         ff_bar += wgt*f*f;
         oo_bar += wgt*o*o;
         fo_bar += wgt*f*o;
      } // end for i

      //
      // Compute Spearman's Rank correlation coefficient
      //
      cnt_info.sp_corr.v = compute_corr( f_bar*n,  o_bar*n,
                                        ff_bar*n, oo_bar*n,
                                        fo_bar*n, n);

      //
      // Compute Kendall Tau Rank correlation coefficient:
      // For each pair of ranked data points (fi, oi), compare it to all other pairs
      // of ranked data points (fj, oj) where j > i.  If the relative ordering of the
      // ranks of the f's is the same as the relative ordering of the ranks of the o's,
      // count the comparison as concordant.  If the previous is not the case, count
      // the comparison as discordant.  If there is a tie between the o's, count the
      // comparison as extra_f.  A tie between the f's counts as an extra_o.  If there
      // is a tie in both the f's and o's, don't count the comparison as anything.
      //
      concordant = discordant = extra_f = extra_o = 0;
      for(i=0; i<n; i++) {
         for(j=i+1; j<n; j++) {

            //
            // Check for agreement in the relative ordering of ranks
            //
            if(      (f_na_rank[i] > f_na_rank[j] && o_na_rank[i] > o_na_rank[j]) ||
                     (f_na_rank[i] < f_na_rank[j] && o_na_rank[i] < o_na_rank[j]) ) concordant++;
            //
            // Check for disagreement in the relative ordering of ranks
            //
            else if( (f_na_rank[i] > f_na_rank[j] && o_na_rank[i] < o_na_rank[j]) ||
                     (f_na_rank[i] < f_na_rank[j] && o_na_rank[i] > o_na_rank[j]) ) discordant++;
            //
            // Check for ties in the forecast rank
            //
            else if(is_eq(f_na_rank[i], f_na_rank[j]) && !is_eq(o_na_rank[i], o_na_rank[j])) extra_o++;
            //
            // Check for ties in the observation rank
            //
            else if(!is_eq(f_na_rank[i], f_na_rank[j]) && is_eq(o_na_rank[i], o_na_rank[j])) extra_f++;
         }
      }
      den = sqrt((double) concordant+discordant+extra_f)*
            sqrt((double) concordant+discordant+extra_o);
      if(is_eq(den, 0.0)) cnt_info.kt_corr.v = bad_data_double;
      else                cnt_info.kt_corr.v = (concordant - discordant)/den;
   } // end if rank_flag

   //
   // Compute normal confidence intervals if the normal_ci_flag is set
   //
   if(normal_ci_flag) cnt_info.compute_ci();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the CNTInfo object from the pairs but remove the i-th pair.
//
////////////////////////////////////////////////////////////////////////

void compute_i_cntinfo(const PairDataPoint &pd, int skip,
                       bool precip_flag, bool rank_flag,
                       bool normal_ci_flag, CNTInfo &cnt_info) {
   int n;
   NumArray i_na;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_i_cntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }
   else {
      n = pd.f_na.n();
   }

   if(skip < 0 || skip > n) {
      mlog << Error << "\ncompute_i_cntinfo() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n";
      throw(1);
   }

   //
   // Exclude the i-th element
   //
   i_na.add_seq(0, skip-1);
   i_na.add_seq(skip+1, n-1);

   compute_cntinfo(pd, i_na, precip_flag, rank_flag, normal_ci_flag,
                   cnt_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_ctsinfo(const PairDataPoint &pd, const NumArray &i_na,
                     bool cts_flag, bool normal_ci_flag,
                     CTSInfo &cts_info) {
   int i, j, n;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_ctsinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }

   //
   // Loop over the length of the index array
   //
   n = i_na.n();

   //
   // Reset the CTS object
   //
   cts_info.cts.zero_out();

   //
   // Loop through the pair data and fill in the contingency table
   //
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      //
      // Add this pair to the contingency table
      //
      cts_info.add(pd.f_na[j], pd.o_na[j], pd.cmn_na[j], pd.csd_na[j]);

   } // end for i

   //
   // Only compute the categorical stats if reqeusted
   //
   if(cts_flag) {

      cts_info.compute_stats();

      //
      // Only compute the normal confidence intervals if requested
      //
      if(normal_ci_flag) cts_info.compute_ci();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_i_ctsinfo(const PairDataPoint &pd, int skip,
                       bool normal_ci_flag, CTSInfo &cts_info) {
   int n;
   NumArray i_na;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_i_ctsinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }
   else {
      n = pd.f_na.n();
   }

   if(skip < 0 || skip > n) {
      mlog << Error << "\ncompute_i_ctsinfo() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n";
      throw(1);
   }

   //
   // Exclude the i-th element
   //
   i_na.add_seq(0, skip-1);
   i_na.add_seq(skip+1, n-1);

   compute_ctsinfo(pd, i_na, 1, normal_ci_flag, cts_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_mctsinfo(const PairDataPoint &pd, const NumArray &i_na,
                      bool mcts_flag, bool normal_ci_flag,
                      MCTSInfo &mcts_info) {
   int i, j, n;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_mctsinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }

   //
   // Loop over the length of the index array
   //
   n = i_na.n();

   //
   // Reset the MCTS object
   //
   mcts_info.cts.zero_out();

   //
   // Loop through the pair data and fill in the contingency table
   //
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      //
      // Add this pair to the contingency table
      //
      mcts_info.add(pd.f_na[j], pd.o_na[j], pd.cmn_na[j], pd.csd_na[j]);

   } // end for i

   //
   // Only compute the categorical stats if reqeusted
   //
   if(mcts_flag) {

      mcts_info.compute_stats();

      //
      // Only compute the normal confidence intervals if requested
      //
      if(normal_ci_flag) mcts_info.compute_ci();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_i_mctsinfo(const PairDataPoint &pd, int skip,
                        bool normal_ci_flag, MCTSInfo &mcts_info) {
   int n;
   NumArray i_na;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_i_mctsinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }
   else {
      n = pd.f_na.n();
   }

   if(skip < 0 || skip > n) {
      mlog << Error << "\ncompute_i_mctsinfo() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n";
      throw(1);
   }

   //
   // Exclude the i-th element
   //
   i_na.add_seq(0, skip-1);
   i_na.add_seq(skip+1, n-1);

   compute_mctsinfo(pd, i_na, 1, normal_ci_flag, mcts_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_pctinfo(const PairDataPoint &pd, bool pstd_flag,
                     PCTInfo &pct_info, const NumArray *cprob_in) {
   int i, n_thresh, n_pair;
   NumArray p_thresh, climo_prob;
   bool cmn_flag;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_pctinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }
   n_pair = pd.f_na.n();

   // Flag to process climo
   cmn_flag = (set_climo_flag(pd.f_na, pd.cmn_na) ||
               (cprob_in && cprob_in->n() > 0));

   // Use input climatological probabilities or derive them
   if(cmn_flag) {
      if(cprob_in) climo_prob = *cprob_in;
      else         climo_prob = derive_climo_prob(pd.cdf_info_ptr,
                                                  pd.cmn_na, pd.csd_na,
                                                  pct_info.othresh);
   }

   //
   // Store the probability threshold values
   //
   n_thresh = pct_info.fthresh.n();
   for(i=0; i<n_thresh; i++) p_thresh.add(pct_info.fthresh[i].get_value());

   //
   // Set up the forecast Nx2ContingencyTable
   //
   pct_info.pct.clear();
   pct_info.pct.set_size(n_thresh-1);
   pct_info.pct.set_thresholds(p_thresh.vals());

   //
   // Set up the climatology Nx2ContingencyTable
   //
   pct_info.climo_pct.clear();
   pct_info.climo_pct.set_size(n_thresh-1);
   pct_info.climo_pct.set_thresholds(p_thresh.vals());

   //
   // Loop through the pair data and fill in the contingency table
   //
   for(i=0; i<n_pair; i++) {

      //
      // Check the observation thresholds and increment accordingly
      //
      if(pct_info.othresh.check(pd.o_na[i], pd.cmn_na[i], pd.csd_na[i])) {
         pct_info.pct.inc_event(pd.f_na[i]);
         if(cmn_flag) pct_info.climo_pct.inc_event(climo_prob[i]);
      }
      else {
         pct_info.pct.inc_nonevent(pd.f_na[i]);
         if(cmn_flag) pct_info.climo_pct.inc_nonevent(climo_prob[i]);
      }
   } // end for i

   //
   // Only compute the probabilistic stats if reqeusted
   //
   if(pstd_flag) {
      pct_info.compute_stats();
      pct_info.compute_ci();
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute partial sums info (n, ffbar, oobar, fobar) for FBS and FSS.
// Compute F_RATE and O_RATE for AFSS and UFSS.
//
////////////////////////////////////////////////////////////////////////

void compute_nbrcntinfo(const PairDataPoint &pd,
                        const PairDataPoint &pd_thr,
                        const NumArray &i_na,
                        NBRCNTInfo &nbrcnt_info, bool nbrcnt_flag) {
   int i, j, n;
   double f, o, wgt, wgt_sum, ff_bar, oo_bar, fo_bar;
   double f_thr_bar, o_thr_bar;

   //
   // Check that the input arrays have the same length
   //
   if(pd.f_na.n() != pd.o_na.n()     ||
      pd.f_na.n() != pd_thr.f_na.n() ||
      pd.f_na.n() != pd_thr.o_na.n() ||
      pd.f_na.n() == 0) {
      mlog << Error << "\ncompute_nbrcntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "non-zero length!\n\n";
      throw(1);
   }

   //
   // Loop over the length of the index array
   //
   n = i_na.n();

   //
   // Get the sum of the weights
   //
   wgt_sum = pd.wgt_na.sum();

   //
   // Compute the continuous statistics from the fcst and obs arrays
   //
   ff_bar = oo_bar = fo_bar = 0.0;
   f_thr_bar = o_thr_bar = 0.0;
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      f   = pd.f_na[j];
      o   = pd.o_na[j];
      wgt = pd.wgt_na[i]/wgt_sum;

      ff_bar += wgt*f*f;
      oo_bar += wgt*o*o;
      fo_bar += wgt*f*o;

      f_thr_bar += wgt*pd_thr.f_na[j];
      o_thr_bar += wgt*pd_thr.o_na[j];
   } // end for i

   //
   // Store the sample size
   //
   nbrcnt_info.sl1l2_info.scount = n;

   //
   // Compute the f*o, f*f, and o*o means
   //
   nbrcnt_info.sl1l2_info.fobar = fo_bar;
   nbrcnt_info.sl1l2_info.ffbar = ff_bar;
   nbrcnt_info.sl1l2_info.oobar = oo_bar;

   //
   // Compute f_rate and o_rate
   //
   nbrcnt_info.f_rate.v = f_thr_bar;
   nbrcnt_info.o_rate.v = o_thr_bar;

   //
   // Only compute stats if requested
   //
   if(nbrcnt_flag) {
      nbrcnt_info.compute_stats();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_i_nbrcntinfo(const PairDataPoint &pd,
                          const PairDataPoint &pd_thr, int skip,
                          NBRCNTInfo &nbrcnt_info) {
   int n;
   NumArray i_na;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_i_nbrcntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }
   else {
      n = pd.f_na.n();
   }

   if(skip < 0 || skip > n) {
      mlog << Error << "\ncompute_i_nbrcntinfo() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n"
          ;
      throw(1);
   }

   //
   // Exclude the i-th element
   //
   i_na.add_seq(0, skip-1);
   i_na.add_seq(skip+1, n-1);

   compute_nbrcntinfo(pd, pd_thr, i_na, nbrcnt_info, 1);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_mean_stdev(const NumArray &v_na, const NumArray &i_na,
                        bool normal_ci_flag, double alpha,
                        CIInfo &mean_ci, CIInfo &stdev_ci) {
   int i, j, n;
   double v, sum, sum_sq;
   double cv_normal_l, cv_normal_u;

   //
   // Loop over the length of the index array
   //
   n = i_na.n();

   //
   // Loop over the values provided
   //
   sum = sum_sq = 0.0;
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      v = v_na[j];

      sum    += v;
      sum_sq += v*v;
   } // end for i

   //
   // Compute the mean
   //
   if(n == 0) mean_ci.v = bad_data_double;
   else       mean_ci.v = sum/n;

   //
   // Compute the standard deviation
   //
   stdev_ci.v = compute_stdev(sum, sum_sq, n);

   //
   // Compute the normal confidence interval for the mean
   // if the normal_ci_flag is set
   //
   if(normal_ci_flag) {

      //
      // Check for the degenerate case
      //
      if(n <= 1 || is_bad_data(stdev_ci.v)) {
         mean_ci.v_ncl[0] = mean_ci.v_ncu[0] = bad_data_double;
      }
      else {

         //
         // Compute the critical values for the Normal or Student's-T
         // distribution based on the sample size
         //
         if(n >= large_sample_threshold) {
            cv_normal_l = normal_cdf_inv(alpha/2.0, 0.0, 1.0);
            cv_normal_u = normal_cdf_inv(1.0 - (alpha/2.0), 0.0, 1.0);
         }
         //
         // If the number of samples is less than the large sample threshold,
         // use the T-distribution
         //
         else {
            cv_normal_l = students_t_cdf_inv(alpha/2.0, n-1);
            cv_normal_u = students_t_cdf_inv(1.0 - (alpha/2.0), n-1);
         }

         //
         // Compute confidence interval for the mean
         //
         mean_ci.v_ncl[0] = mean_ci.v +
                            cv_normal_l*stdev_ci.v/sqrt((double) n);
         mean_ci.v_ncu[0] = mean_ci.v +
                            cv_normal_u*stdev_ci.v/sqrt((double) n);
      } // end else
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_i_mean_stdev(const NumArray &v_na,
                          bool normal_ci_flag, double alpha, int skip,
                          CIInfo &mean_ci, CIInfo &stdev_ci) {
   int i, n, count;
   NumArray v_na_i, i_na_i;

   n = v_na.n();

   if(skip < 0 || skip > n) {
      mlog << Error << "\ncompute_i_mean_stdev() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n"
          ;
      exit(1);
   }

   //
   // Copy over the array and index values except for the one to
   // be skipped
   //
   for(i=0, count=0; i<n; i++) {
      if(i == skip) continue;
      v_na_i.add(v_na[i]);
      i_na_i.add(count);
      count++;
   }

   compute_mean_stdev(v_na_i, i_na_i, normal_ci_flag, alpha,
                      mean_ci, stdev_ci);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute an unweighted mean of scalar partial sums.
//
////////////////////////////////////////////////////////////////////////

void compute_sl1l2_mean(const SL1L2Info *sl1l2_info, int n,
                        SL1L2Info &sl1l2_mean) {
   int i, n_sl1l2, n_sal1l2;

   // Initialize
   sl1l2_mean.clear();
   if(n == 0) return;

   // Store thresholds using the first array entry
   sl1l2_mean.fthresh = sl1l2_info[0].fthresh;
   sl1l2_mean.othresh = sl1l2_info[0].othresh;
   sl1l2_mean.logic   = sl1l2_info[0].logic;

   // Compute running sums
   for(i=n_sl1l2=n_sal1l2=0; i<n; i++) {

      if(sl1l2_info[i].scount > 0) {
         n_sl1l2++;
         sl1l2_mean.scount += sl1l2_info[i].scount;
         sl1l2_mean.fbar   += sl1l2_info[i].fbar;
         sl1l2_mean.obar   += sl1l2_info[i].obar;
         sl1l2_mean.ffbar  += sl1l2_info[i].ffbar;
         sl1l2_mean.oobar  += sl1l2_info[i].oobar;
         sl1l2_mean.mae    += sl1l2_info[i].mae;
      }

      if(sl1l2_info[i].sacount > 0) {
         n_sal1l2++;
         sl1l2_mean.sacount += sl1l2_info[i].sacount;
         sl1l2_mean.fabar   += sl1l2_info[i].fabar;
         sl1l2_mean.oabar   += sl1l2_info[i].oabar;
         sl1l2_mean.ffabar  += sl1l2_info[i].ffabar;
         sl1l2_mean.ooabar  += sl1l2_info[i].ooabar;
      }
   } // end for i

   // Compute means
   if(sl1l2_mean.scount > 0) {
      sl1l2_mean.fbar  /= n_sl1l2;
      sl1l2_mean.obar  /= n_sl1l2;
      sl1l2_mean.ffbar /= n_sl1l2;
      sl1l2_mean.oobar /= n_sl1l2;
      sl1l2_mean.mae   /= n_sl1l2;
   }
   if(sl1l2_mean.sacount > 0) {
      sl1l2_mean.fabar  /= n_sal1l2;
      sl1l2_mean.oabar  /= n_sal1l2;
      sl1l2_mean.ffabar /= n_sal1l2;
      sl1l2_mean.ooabar /= n_sal1l2;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute an unweighted mean of continuous statistics.
//
////////////////////////////////////////////////////////////////////////

void compute_cnt_mean(const CNTInfo *cnt_info, int n,
                      CNTInfo &cnt_mean) {
   int i;
   NumArray na;

   // Initialize
   cnt_mean.clear();
   if(n == 0) return;

   // Store thresholds using the first array entry
   cnt_mean.fthresh = cnt_info[0].fthresh;
   cnt_mean.othresh = cnt_info[0].othresh;
   cnt_mean.logic   = cnt_info[0].logic;

   // Allocate one alpha value but compute no confidence intervals
   cnt_mean.allocate_n_alpha(1);
   cnt_mean.alpha[0] = bad_data_double;

   // Compute the sum of the counts
   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].n);
   cnt_mean.n = na.sum();

   // Compute means, skipping n_ranks, frank_ties, and orank_ties

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].fbar.v);
   cnt_mean.fbar.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].fstdev.v);
   cnt_mean.fstdev.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].obar.v);
   cnt_mean.obar.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].ostdev.v);
   cnt_mean.ostdev.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].pr_corr.v);
   cnt_mean.pr_corr.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].sp_corr.v);
   cnt_mean.sp_corr.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].kt_corr.v);
   cnt_mean.kt_corr.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].anom_corr.v);
   cnt_mean.anom_corr.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].rmsfa.v);
   cnt_mean.rmsfa.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].rmsoa.v);
   cnt_mean.rmsoa.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].anom_corr_uncntr.v);
   cnt_mean.anom_corr_uncntr.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].me.v);
   cnt_mean.me.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].me2.v);
   cnt_mean.me2.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].estdev.v);
   cnt_mean.estdev.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].mbias.v);
   cnt_mean.mbias.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].mae.v);
   cnt_mean.mae.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].mse.v);
   cnt_mean.mse.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].msess.v);
   cnt_mean.msess.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].bcmse.v);
   cnt_mean.bcmse.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].rmse.v);
   cnt_mean.rmse.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].si.v);
   cnt_mean.si.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].e10.v);
   cnt_mean.e10.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].e25.v);
   cnt_mean.e25.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].e50.v);
   cnt_mean.e50.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].e75.v);
   cnt_mean.e75.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].e90.v);
   cnt_mean.e90.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].eiqr.v);
   cnt_mean.eiqr.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].mad.v);
   cnt_mean.mad.v = na.mean();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute an unweighted mean of probabilistic statistics.
//
////////////////////////////////////////////////////////////////////////

void compute_pct_mean(const PCTInfo *cnt_info, int n,
                      PCTInfo &pct_mean, bool sum_total) {
   int i;
   NumArray na;

   // Initialize
   pct_mean.clear();
   if(n == 0) return;

   // Initialize with the first array entry
   pct_mean.fthresh   = cnt_info[0].fthresh;
   pct_mean.othresh   = cnt_info[0].othresh;
   pct_mean.pct       = cnt_info[0].pct;
   pct_mean.climo_pct = cnt_info[0].climo_pct;

   // Allocate one alpha value but compute no confidence intervals
   pct_mean.allocate_n_alpha(1);
   pct_mean.alpha[0] = bad_data_double;

   // Compute the sum or mean of the totals
   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].total);
   if(sum_total) pct_mean.total = na.sum();
   else          pct_mean.total = na.mean();

   // Compute means of statistics

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].baser.v);
   pct_mean.baser.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].reliability);
   pct_mean.reliability = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].resolution);
   pct_mean.resolution = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].uncertainty);
   pct_mean.uncertainty = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].roc_auc);
   pct_mean.roc_auc = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].brier.v);
   pct_mean.brier.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].briercl.v);
   pct_mean.briercl.v = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].bss);
   pct_mean.bss = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(cnt_info[i].bss_smpl);
   pct_mean.bss_smpl = na.mean();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute an unweighted mean of continuous ensemble statistics.
//
////////////////////////////////////////////////////////////////////////

void compute_ecnt_mean(const ECNTInfo *ecnt_info, int n,
                       ECNTInfo &ecnt_mean) {
   int i;
   NumArray na;

   // Initialize
   ecnt_mean.clear();
   if(n == 0) return;

   // Store thresholds using the first array entry
   ecnt_mean.othresh = ecnt_info[0].othresh;

   // Store number of ensemble members using the first entry
   ecnt_mean.n_ens = ecnt_info[0].n_ens;

   // Compute the sum of the pairs
   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].n_pair);
   ecnt_mean.n_pair = na.sum();

   // Compute unweighted mean for each statistic
   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].crps_emp);
   ecnt_mean.crps_emp = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].crpscl_emp);
   ecnt_mean.crpscl_emp = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].crpss_emp);
   ecnt_mean.crpss_emp = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].crps_gaus);
   ecnt_mean.crps_gaus = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].crpscl_gaus);
   ecnt_mean.crpscl_gaus = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].crpss_gaus);
   ecnt_mean.crpss_gaus = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].ign);
   ecnt_mean.ign = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].me);
   ecnt_mean.me = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].rmse);
   ecnt_mean.rmse = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].spread);
   ecnt_mean.spread = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].me_oerr);
   ecnt_mean.me_oerr = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].rmse_oerr);
   ecnt_mean.rmse_oerr = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].spread_oerr);
   ecnt_mean.spread_oerr = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].spread_plus_oerr);
   ecnt_mean.spread_plus_oerr = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].bias_ratio);
   ecnt_mean.bias_ratio = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].n_ge_obs);
   ecnt_mean.n_ge_obs = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].me_ge_obs);
   ecnt_mean.me_ge_obs = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].n_lt_obs);
   ecnt_mean.n_lt_obs = na.mean();

   for(i=0,na.erase(); i<n; i++) na.add(ecnt_info[i].me_lt_obs);
   ecnt_mean.me_lt_obs = na.mean();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute aggregated SEEPS.
//
////////////////////////////////////////////////////////////////////////

void compute_aggregated_seeps(const PairDataPoint *pd, SeepsAggScore *seeps) {
   static const char *method_name = "compute_seeps_agg() -> ";

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd->f_na.n() != pd->o_na.n()) {
      mlog << Error << "\n" << method_name
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      throw(1);
   }

   SeepsScore *seeps_mpr;
   int count, count_diagonal;
   int c12, c13, c21, c23, c31, c32;
   double score_sum, obs_sum, fcst_sum;
   vector<SeepsScore *> seeps_mprs;

   score_sum = obs_sum = fcst_sum = 0.0;
   count = count_diagonal = c12 = c13 = c21 = c23 = c31 = c32 = 0;
   for(int i=0; i<pd->n_obs; i++) {
      if (i >= pd->seeps_mpr.size()) break;
      seeps_mpr = pd->seeps_mpr[i];
      if (!seeps_mpr || is_eq(seeps_mpr->score, bad_data_double)) continue;

      count++;
      obs_sum  += pd->o_na[i];   // Observation Value
      fcst_sum += pd->f_na[i];   // Forecast Value
      score_sum += seeps_mpr->score;
      if (seeps_mpr->fcst_cat == 0) {
         if (seeps_mpr->obs_cat == 1) c12++;
         else if(seeps_mpr->obs_cat == 2) c13++;
         else count_diagonal++;
      }
      else if (seeps_mpr->fcst_cat == 1) {
         if (seeps_mpr->obs_cat == 0) c21++;
         else if(seeps_mpr->obs_cat == 2) c23++;
         else count_diagonal++;
      }
      else if (seeps_mpr->fcst_cat == 2) {
         if (seeps_mpr->obs_cat == 0) c31++;
         else if (seeps_mpr->obs_cat == 1) c32++;
         else count_diagonal++;
      }
      seeps_mprs.push_back(seeps_mpr);
   }
   if (count > 0) {
      double *density_vector;
      double pvf[SEEPS_MATRIX_SIZE];
      double weighted_score, weight_sum, weight[count];

      seeps->n_obs = count;
      seeps->mean_fcst = fcst_sum / count;
      seeps->mean_obs = obs_sum / count;
      seeps->score = score_sum / count;
      density_vector = compute_seeps_density_vector(pd, seeps);

      for (int i=0; i<SEEPS_MATRIX_SIZE; i++) pvf[i] = 0.;
      if (density_vector != NULL) {
         //IDL: w = 1/d
         weight_sum = 0.;
         for (int i=0; i<count; i++) {
            if (is_eq(density_vector[i], 0)) weight[i] = 0;
            else {
               weight[i] = 1 / density_vector[i];
               weight_sum += weight[i];
            }
         }
         weighted_score = 0.;
         if (!is_eq(weight_sum, 0)) {
            //IDL: w = w/sum(w)
            for (int i=0; i<count; i++) weight[i] /= weight_sum;

            //IDL: for i=0l, n-1l do begin
            for (int i=0; i<seeps_mprs.size(); i++) {
               seeps_mpr = seeps_mprs[i];
               //IDL: s = s + c(4+cat(i) * w{i)
               weighted_score += seeps_mpr->score * weight[i];
               //IDL: svf(cat{i)) = svf(cat{i)) + c(4+cat(i) * w{i)
               //IDL: pvf(cat{i)) = pvf(cat{i)) + w{i)
               pvf[seeps_mpr->s_idx] += weight[i];
            }
         }

         if (density_vector != NULL) delete [] density_vector;
      }
      seeps_mprs.clear();

      // The weight for s12 to s32 should come from climo file, but not available yet
      seeps->pv1 = pvf[0] + pvf[3] + pvf[6];    // sum by column for obs
      seeps->pv2 = pvf[1] + pvf[4] + pvf[7];    // sum by column for obs
      seeps->pv3 = pvf[2] + pvf[5] + pvf[8];    // sum by column for obs
      seeps->pf1 = pvf[0] + pvf[1] + pvf[2];    // sum by row for forecast
      seeps->pf2 = pvf[3] + pvf[4] + pvf[5];    // sum by row for forecast
      seeps->pf3 = pvf[6] + pvf[7] + pvf[8];    // sum by row for forecast
      seeps->s12 = c12 * seeps->pf1 * seeps->pv2;
      seeps->s13 = c13 * seeps->pf1 * seeps->pv3;
      seeps->s21 = c21 * seeps->pf2 * seeps->pv1;
      seeps->s23 = c23 * seeps->pf2 * seeps->pv3;
      seeps->s31 = c31 * seeps->pf3 * seeps->pv1;
      seeps->s32 = c32 * seeps->pf3 * seeps->pv2;
      seeps->weighted_score = weighted_score;

      mlog << Debug(7) << method_name
           << "SEEPS score=" << seeps->score << " weighted_score=" << weighted_score
           << " pv1=" << seeps->pv1 << " pv2=" << seeps->pv2 << " pv3=" << seeps->pv3
           << " pf1=" << seeps->pf1 << " pf2=" << seeps->pf2 << " pf3=" << seeps->pf3 << "\n";
   }
   else {
      mlog << Debug(5) << method_name
           << "no SEEPS_MPR available\n";
   }
   seeps->c12 = c12;
   seeps->c13 = c13;
   seeps->c21 = c21;
   seeps->c23 = c23;
   seeps->c31 = c31;
   seeps->c32 = c32;

   if (count != (c12+c13+c21+c23+c31+c32+count_diagonal)){
      mlog << Debug(6) << method_name
           << "INFO check count: all=" << count << " s12=" << c12<< " s13=" << c13
           << " s21=" << c21 << " s23=" << c23
           << " s31=" << c31 << " s32=" << c32 << "\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_aggregated_seeps_grid(const DataPlane &fcst_dp, const DataPlane &obs_dp,
                                   DataPlane &seeps_dp, SeepsAggScore *seeps,
                                   int month, int hour) {
   int seeps_count, count_diagonal;
   int nx = fcst_dp.nx();
   int ny = fcst_dp.ny();
   double pvf[SEEPS_MATRIX_SIZE];
   int c12, c13, c21, c23, c31, c32;
   double obs_sum, fcst_sum;
   double seeps_score, seeps_score_sum, weighted_score;
   float weight = 1 / (nx * ny);
   SeepsScore *seeps_mpr;
   static const char *method_name = "compute_aggregated_seeps_grid() -> ";

   seeps_dp.set_size(nx, ny);
   obs_sum = fcst_sum = weighted_score = seeps_score_sum = 0.;
   seeps_count = count_diagonal = c12 = c13 = c21 = c23 = c31 = c32 = 0;

   SeepsClimoGrid *seeps_climo = get_seeps_climo_grid(month);
   for (int i=0; i<SEEPS_MATRIX_SIZE; i++) pvf[i] = 0.;

   for (int ix=0; ix<nx; ix++) {
      for (int iy=0; iy<ny; iy++) {
         float fcst_value = fcst_dp.get(ix, iy);
         float obs_value = obs_dp.get(ix, iy);
         if (is_eq(fcst_value, -9999.0) || is_eq(obs_value, -9999.0)) seeps_score = bad_data_float;
         else {
            if (is_eq(fcst_value, 0.) && is_eq(obs_value, 0.)) {
               seeps_score = 0;
               pvf[0] += weight;
            }
            else {
               obs_sum += obs_value;
               fcst_sum += fcst_value;
               seeps_mpr = seeps_climo->get_record(ix, iy, fcst_value, obs_value);
               if (seeps_mpr != NULL) {
                  if (seeps_mpr->fcst_cat == 0) {
                     if (seeps_mpr->obs_cat == 1) c12++;
                     else if(seeps_mpr->obs_cat == 2) c13++;
                     else count_diagonal++;
                  }
                  else if (seeps_mpr->fcst_cat == 1) {
                     if (seeps_mpr->obs_cat == 0) c21++;
                     else if(seeps_mpr->obs_cat == 2) c23++;
                     else count_diagonal++;
                  }
                  else if (seeps_mpr->fcst_cat == 2) {
                     if (seeps_mpr->obs_cat == 0) c31++;
                     else if (seeps_mpr->obs_cat == 1) c32++;
                     else count_diagonal++;
                  }
                  seeps_score = seeps_mpr->score;
                  seeps_score_sum += seeps_score;
                  //IDL: s = s + c(4+cat(i) * w{i)
                  weighted_score += seeps_score * weight;
                  //IDL: svf(cat{i)) = svf(cat{i)) + c(4+cat(i) * w{i)
                  //IDL: pvf(cat{i)) = pvf(cat{i)) + w{i)
                  pvf[seeps_mpr->s_idx] += weight;
                  delete seeps_mpr;
               }
            }
            seeps_count++;
         }
         seeps_dp.set(seeps_score, ix, iy);
      }
   }

   seeps->n_obs = seeps_count;
   if (seeps_count > 0) {
      seeps->mean_fcst = fcst_sum / seeps_count;
      seeps->mean_obs = obs_sum / seeps_count;

      seeps->pv1 = pvf[0] + pvf[3] + pvf[6];    // sum by column for obs
      seeps->pv2 = pvf[1] + pvf[4] + pvf[7];    // sum by column for obs
      seeps->pv3 = pvf[2] + pvf[5] + pvf[8];    // sum by column for obs
      seeps->pf1 = pvf[0] + pvf[1] + pvf[2];    // sum by row for forecast
      seeps->pf2 = pvf[3] + pvf[4] + pvf[5];    // sum by row for forecast
      seeps->pf3 = pvf[6] + pvf[7] + pvf[8];    // sum by row for forecast
      seeps->s12 = c12 * seeps->pf1 * seeps->pv2;
      seeps->s13 = c13 * seeps->pf1 * seeps->pv3;
      seeps->s21 = c21 * seeps->pf2 * seeps->pv1;
      seeps->s23 = c23 * seeps->pf2 * seeps->pv3;
      seeps->s31 = c31 * seeps->pf3 * seeps->pv1;
      seeps->s32 = c32 * seeps->pf3 * seeps->pv2;
      seeps->score = seeps_score_sum / seeps_count;
   }
   seeps->c12 = c12;
   seeps->c13 = c13;
   seeps->c21 = c21;
   seeps->c23 = c23;
   seeps->c31 = c31;
   seeps->c32 = c32;
   seeps->weighted_score = weighted_score;
   mlog << Debug(6) << method_name
        << "SEEPS score=" << seeps->score << " weighted_score=" << weighted_score
        << " pv1=" << seeps->pv1 << " pv2=" << seeps->pv2 << " pv3=" << seeps->pv3
        << " pf1=" << seeps->pf1 << " pf2=" << seeps->pf2 << " pf3=" << seeps->pf3 << "\n";
}

////////////////////////////////////////////////////////////////////////
// IDL code:
//
// my_pi = 3.1415926
// r0 = density_radius*my_pi/180
// lat =  reform(float(data_geo(0,*)))
// lon =  reform( ((float(data_geo(1,*)+360.) mod 360.)))
// n = n_elements(lat)
// lat = lat * my_pi / 180
// slat = sin(lat)
// clat = cos(lat)
// lon = lon * my_pi / 180
// slon = sin(lon)
// clon = cos(lon)
// r=(clat#transpose(clat))*(slon#transpose(slon)) + (clon#transpose(clon))*(slat#transpose(slat))
// r=r*((r lt 1.) and (r gt -1.)) + (r ge 1.) - (r le -1.)
// r=acos(r)
// if r0 gt 0.0 then r = exp( -(r / r0) ^ 2) * ( r le 4. * r0) else r = (r*0.) + 1
// r=sum(r,1)
//
// SUM function (not built-in)
// PRINT, array2
// ; PV-WAVE prints the following:
// ; 0.00000      1.00000
// ; 2.00000      3.00000
// PRINT, SUM(array2, 0)
// ; PV-WAVE prints: 1.00000      5.00000
// PRINT, SUM(array2, 1)
// ; PV-WAVE prints: 2.00000      4.00000


double *compute_seeps_density_vector(const PairDataPoint *pd, SeepsAggScore *seeps) {
   int seeps_idx;
   SeepsScore *seeps_mpr;
   int seeps_cnt = seeps->n_obs;
   const double density_radidus_rad = density_radius * rad_per_deg;
   double rlat[seeps_cnt], rlon[seeps_cnt];
   double clat[seeps_cnt], clon[seeps_cnt], slat[seeps_cnt], slon[seeps_cnt];
   double clat_m[seeps_cnt][seeps_cnt], clon_m[seeps_cnt][seeps_cnt];
   double slat_m[seeps_cnt][seeps_cnt], slon_m[seeps_cnt][seeps_cnt];
   double clat_slon[seeps_cnt][seeps_cnt], clon_slat[seeps_cnt][seeps_cnt];
   double density_m[seeps_cnt][seeps_cnt];
   static const char *method_name = "compute_seeps_density_vector() -> ";

   if (seeps_cnt == 0) {
      mlog << Debug(1) << method_name
           << "no SEEPS_MPR available.\n";
      return NULL;
   }
   // Get lat/lon & convert them to radian and get sin/cos values
   seeps_idx = 0;
   double *density_vector = new double[seeps_cnt];
   for(int i=0; i<pd->n_obs; i++) {
      if (i >= pd->seeps_mpr.size()) break;
      seeps_mpr = pd->seeps_mpr[i];
      if (!seeps_mpr || is_eq(seeps_mpr->score, bad_data_double)) continue;

      rlat[seeps_idx] = pd->lat_na[i] * rad_per_deg;    // radian of lat
      rlon[seeps_idx] = fmod((pd->lon_na[i] + 360.), 360.) * rad_per_deg;    // radian of long
      clat[seeps_idx] = cos(rlat[seeps_idx]);
      clon[seeps_idx] = cos(rlon[seeps_idx]);
      slat[seeps_idx] = sin(rlat[seeps_idx]);
      slon[seeps_idx] = sin(rlon[seeps_idx]);

      seeps_idx++;
   }

   // prooducs n by n matrix by multipling transposed vector
   int v_count;
   double density, mask1, mask2, mask3, temp;
   // Initialize
   v_count = 0;
   if (seeps_idx < seeps_cnt) seeps_cnt = seeps_idx;
   for(int i=0; i<seeps_cnt; i++) {
      for(int j=0; j<seeps_cnt; j++) {
         clat_slon[i][j] = clon_slat[i][j] = 0.;
      }
      density_vector[i] = 0.;
   }
   for(int j=0; j<seeps_cnt; j++) {
      for(int i=0; i<seeps_cnt; i++) {
         // Make n by n matrix: clat_m = clat#transpose(clat), slat_m = slat#transpose(slat) by IDL
         clat_m[i][j] = clat[i] * clat[j];
         slat_m[i][j] = slat[i] * slat[i];
         clon_m[i][j] = clon[i] * clon[i];
         slon_m[i][j] = slon[i] * slon[i];
         //IDL: r=(clat#transpose(clat))*(slon#transpose(slon)) + (clon#transpose(clon))*(slat#transpose(slat))
         density = clat_m[i][j] * slon_m[i][j] + clon_m[i][j] * slat_m[i][j];
         //IDL: r * ((r lt 1.) and (r gt -1.)) + (r ge 1.) - (r le -1.)
         mask1 = (density < 1.0 && density > -1.0) ? 1. : 0.;
         mask2 = (density >= 1.0 ) ? 1. : 0.;
         mask3 = (density <= -1.0) ? 1. : 0.;
         density = density * mask1 + mask2 - mask3;
         //IDL: r = acos(r)
         density = acos(density);
         //IDL: if r0 gt 0.0 then r = exp(-(r/r0)^2) * (r le 4. * r0) else r = (r*0.)+1.
         if (density_radidus_rad <= 0.) density = 1.0;
         else {
            mask3 = (density <= 4.0) ? 1. : 0.;
            temp = density / density_radidus_rad;
            density = exp(-(temp * temp)) * mask3 * density_radidus_rad;
         }
         density_vector[j] += density;
      }
      if (!is_eq(density_vector[j], 0.)) v_count++;
   }
   if (v_count == 0) {
      mlog << Debug(4) << method_name
           << "no non-zero values at density_vector\n";
   }
   mlog << Debug(10) << method_name
        << " non zero count=" << v_count
        << " density_vector[0]=" << density_vector[0]
        << " density_vector[" << (seeps_cnt-1) << "]=" << density_vector[seeps_cnt-1] << "\n";

   return density_vector;
}

////////////////////////////////////////////////////////////////////////
