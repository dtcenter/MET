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
#include <errno.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "compute_ci.h"
#include "compute_stats.h"

#include "vx_gsl_prob.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void write_cntinfo(ofstream &, const CNTInfo &);
static void write_ctsinfo(ofstream &, const CTSInfo &);
static void write_mctsinfo(ofstream &, const MCTSInfo &);
static void write_nbrcntinfo(ofstream &, const NBRCNTInfo &);
static void read_ldf(const ConcatString, int, NumArray &);

////////////////////////////////////////////////////////////////////////
//
// Compute a normal confidence interval.
//
////////////////////////////////////////////////////////////////////////

void compute_normal_ci(double v, double alpha, double se,
                       double &cl, double &cu) {
   double cv_normal_l, cv_normal_u;

   cv_normal_l = normal_cdf_inv(alpha/2.0, 0.0, 1.0);
   cv_normal_u = normal_cdf_inv(1.0 - (alpha/2.0), 0.0, 1.0);

   cl = v + cv_normal_l * se;
   cu = v + cv_normal_u * se;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for a proportion.
//
////////////////////////////////////////////////////////////////////////

void compute_proportion_ci(double p, int n, double alpha, double vif,
                           double &p_cl, double &p_cu) {

   //
   // Compute the confidence interval using the Wilson method for all
   // sizes of n, since it provides a better approximation
   //
   compute_wilson_ci(p, n, alpha, vif, p_cl, p_cu);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for a proportion using the Wald method.
// Taken from Statistical Methods in the Atmospheric Sciences 2nd Ed.
// Daniel S. Wilks, page 327
//
////////////////////////////////////////////////////////////////////////

void compute_wald_ci(double p, int n, double alpha, double vif,
                     double &p_cl, double &p_cu) {
   double v, cv_normal_l, cv_normal_u;

   if(is_bad_data(p)) {
      p_cl = p_cu = bad_data_double;
      return;
   }

   //
   // Compute the upper and lower critical values from the
   // normal distribution.
   //
   cv_normal_l = normal_cdf_inv(alpha/2.0, 0.0, 1.0);
   cv_normal_u = normal_cdf_inv(1.0 - (alpha/2.0), 0.0, 1.0);

   //
   // Compute the upper and lower bounds of the confidence interval
   //
   v = vif*p*(1.0-p)/n;

   if(v < 0.0) {
      p_cl = bad_data_double;
      p_cu = bad_data_double;
   }
   else {
      p_cl = p + cv_normal_l * sqrt(v);
      p_cu = p + cv_normal_u * sqrt(v);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for a proportion using the Wilson method.
// Taken from Statistical Methods in the Atmospheric Sciences 2nd Ed.
// Daniel S. Wilks, page 327
//
////////////////////////////////////////////////////////////////////////

void compute_wilson_ci(double p, int n_int, double alpha, double vif,
                       double &p_cl, double &p_cu) {
   double v, cv_normal_l, cv_normal_u;
   long long n = n_int;

   if(is_bad_data(p)) {
      p_cl = p_cu = bad_data_double;
      return;
   }

   //
   // Compute the upper and lower critical values from the
   // normal distribution.
   //
   cv_normal_l = normal_cdf_inv(alpha/2.0, 0.0, 1.0);
   cv_normal_u = normal_cdf_inv(1.0 - (alpha/2.0), 0.0, 1.0);

   //
   // Compute the upper and lower bounds of the confidence interval
   //
   v = vif*p*(1.0-p)/n + cv_normal_l*cv_normal_l/(4*n*n);
   if(v < 0.0) {
      p_cl = bad_data_double;
   }
   else {
      p_cl = ( p + (cv_normal_l*cv_normal_l)/(2.0*n) + cv_normal_l * sqrt(v) )
             / (1.0 + cv_normal_l*cv_normal_l/n);
   }

   v = vif*p*(1.0-p)/n + cv_normal_u*cv_normal_u/(4*n*n);
   if(v < 0.0) {
      p_cu = bad_data_double;
   }
   else {
      p_cu = ( p + (cv_normal_u*cv_normal_u)/(2.0*n) + cv_normal_u * sqrt(v) )
          / (1.0 + cv_normal_u*cv_normal_u/n);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for the odds ratio using Woolf's
// method.
//
////////////////////////////////////////////////////////////////////////

void compute_woolf_ci(double odds, double alpha,
                      int fy_oy, int fy_on, int fn_oy, int fn_on,
                      double &odds_cl, double &odds_cu) {
   double cv_normal_l, cv_normal_u, a, b;

   if(is_bad_data(odds) ||
      fy_oy == 0 || fy_on == 0 || fn_oy == 0 || fn_on == 0) {
      odds_cl = odds_cu = bad_data_double;
      return;
   }

   //
   // Compute the upper and lower critical values from the
   // normal distribution.
   //
   cv_normal_l = normal_cdf_inv(alpha/2.0, 0.0, 1.0);
   cv_normal_u = normal_cdf_inv(1.0 - (alpha/2.0), 0.0, 1.0);

   //
   // Compute the upper and lower bounds of the confidence interval
   //
   a = exp(cv_normal_l*sqrt(1.0/fy_oy + 1.0/fy_on + 1.0/fn_oy + 1.0/fn_on));
   b = exp(cv_normal_u*sqrt(1.0/fy_oy + 1.0/fy_on + 1.0/fn_oy + 1.0/fn_on));

   odds_cl = odds * a;
   odds_cu = odds * b;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for the Hanssen and Kuipers
// discriminant.
// Taken from Statistical Methods in the Atmospheric Sciences 2nd Ed.
// Daniel S. Wilks, page 328
//
////////////////////////////////////////////////////////////////////////

void compute_hk_ci(double hk, double alpha, double vif,
                   int fy_oy, int fy_on, int fn_oy, int fn_on,
                   double &hk_cl, double &hk_cu) {
   double cv_normal, stdev;
   double h, h_var, f_var;
   int h_n, f_n;

   //
   // Get the counts
   //
   h_n = fy_oy + fn_oy;
   f_n = fn_on + fy_on;

   if(is_bad_data(hk) || h_n == 0 || f_n == 0) {
      hk_cl = hk_cu = bad_data_double;
      return;
   }

   //
   // Compute the critical value for the normal distribution based
   // on the sample size
   //
   cv_normal = normal_cdf_inv(alpha/2.0, 0.0, 1.0);

   //
   // Compute the hit rate and false alarm rate
   //
   h = (double) fy_oy/h_n;

   //
   // Compute a variance for H and F
   //
   h_var = sqrt(h*(1.0-h)/h_n + cv_normal*cv_normal/(4.0*h_n*h_n))
         / (1.0 + cv_normal*cv_normal/h_n);

   f_var = sqrt(h*(1.0-h)/f_n + cv_normal*cv_normal/(4.0*f_n*f_n))
         / (1.0 + cv_normal*cv_normal/f_n);

   //
   // Compute the standard deviation for HK
   //
   stdev = sqrt(vif*(h_var*h_var + f_var*f_var));

   //
   // Compute the upper and lower bounds of the confidence interval
   //
   hk_cl = hk + cv_normal*stdev;
   hk_cu = hk - cv_normal*stdev;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the categorical statistics for the pairs provided.
// Compute bootstrap confidence intervals using the BCa method with
// the random number generator and number of replicates specified.
// Arguments:
//    gsl_rng is a pointer to the random number generator to be used.
//    pd is the PairDataPoint object with data to be bootstrapped.
//    b is the number of replicates to be used when bootstrapping.
//    CTSInfo is the object for holding the stats.
//
////////////////////////////////////////////////////////////////////////

void compute_cts_stats_ci_bca(const gsl_rng *rng_ptr,
                              const PairDataPoint &pd,
                              int b, CTSInfo *&cts_info, int n_cts,
                              bool cts_flag, bool rank_flag,
                              const char *tmp_dir) {
   int n = 0;
   int i, j, c;
   double s;
   NumArray i_na, ir_na, si_na, sr_na;
   CTSInfo *cts_tmp = (CTSInfo *) 0;

   //
   // Temp file streams for categorical statistics
   //
   ofstream *cts_i_out = (ofstream *) 0, *cts_r_out = (ofstream *) 0;
   ConcatString *cts_i_file = (ConcatString *) 0;
   ConcatString *cts_r_file = (ConcatString *) 0;
   ConcatString prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_cts_stats_ci_bca() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute categorical stats from the raw data for each threshold
   // with the normal_ci flag set
   //
   for(i=0; i<n_cts; i++) {
      compute_ctsinfo(pd, i_na, cts_flag, true, cts_info[i]);
   }

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the cts_flag is off
   //
   if(n<=1 || b<1 || !cts_flag) return;

   //
   // Allocate space to store categorical stats for each threshold
   // and for the output temp file streams
   //
   cts_tmp    = new CTSInfo      [n_cts];
   cts_i_out  = new ofstream     [n_cts];
   cts_r_out  = new ofstream     [n_cts];
   cts_i_file = new ConcatString [n_cts];
   cts_r_file = new ConcatString [n_cts];
   for(i=0; i<n_cts; i++) {
      cts_tmp[i].fthresh = cts_info[i].fthresh;
      cts_tmp[i].othresh = cts_info[i].othresh;
   }

   //
   // Build the temp file names
   //
   for(i=0; i<n_cts; i++) {
      prefix << cs_erase << tmp_dir << "/tmp_cts_i_" << i;
      cts_i_file[i] = make_temp_file_name(prefix.c_str(), NULL);

      prefix << cs_erase << tmp_dir << "/tmp_cts_r_" << i;
      cts_r_file[i] = make_temp_file_name(prefix.c_str(), NULL);
   }

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      for(i=0; i<n_cts; i++) {
         cts_i_out[i].open(cts_i_file[i].c_str());
         cts_r_out[i].open(cts_r_file[i].c_str());
         if(!cts_i_out[i] || !cts_r_out[i]) {
            mlog << Error << "\ncompute_cts_stats_ci_bca() -> "
                 << "can't open one or more temporary files for writing:\n"
                 << cts_i_file[i] << "\n"
                 << cts_r_file[i] << "\n\n";

            //
            // Attempt to delete temp files
            //
            for(i=0; i<n_cts; i++) {
               remove_temp_file(cts_i_file[i]);
               remove_temp_file(cts_r_file[i]);
            }

            // deallocate memory
            if(cts_tmp)    { delete [] cts_tmp;    cts_tmp    = (CTSInfo *)      0; }
            if(cts_i_out)  { delete [] cts_i_out;  cts_i_out  = (ofstream *)     0; }
            if(cts_r_out)  { delete [] cts_r_out;  cts_r_out  = (ofstream *)     0; }
            if(cts_i_file) { delete [] cts_i_file; cts_i_file = (ConcatString *) 0; }
            if(cts_r_file) { delete [] cts_r_file; cts_r_file = (ConcatString *) 0; }
            throw(1);
         }
      }

      //
      // Compute catgegorical stats from the raw data with the i-th data
      // point removed and write out to a temp file
      //
      for(i=0; i<n_cts; i++) {
         for(j=0; j<n; j++) {
            compute_i_ctsinfo(pd, j, false, cts_tmp[i]);
            write_ctsinfo(cts_i_out[i], cts_tmp[i]);
         }
      } // end for i

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, n);

         //
         // Compute categorical stats for each replicate with the
         // cts_flag set and the normal_ci_flag unset
         //
         for(j=0; j<n_cts; j++) {
            compute_ctsinfo(pd, ir_na, true, false, cts_tmp[j]);
            write_ctsinfo(cts_r_out[j], cts_tmp[j]);
         } // end for j
      }

      //
      // Close the temp files
      //
      for(i=0; i<n_cts; i++) {
         cts_i_out[i].close();
         cts_r_out[i].close();
      }

      //
      // Compute bootstrap intervals for each threshold value
      //
      for(i=0; i<n_cts; i++) {

         //
         // Initialize column counter
         //
         c = 1;

         //
         // Compute bootstrap interval for baser
         //
         s = cts_info[i].baser.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].baser.v_bcl[j],
                                 cts_info[i].baser.v_bcu[j]);

         //
         // Compute bootstrap interval for fmean
         //
         s = cts_info[i].fmean.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].fmean.v_bcl[j],
                                 cts_info[i].fmean.v_bcu[j]);

         //
         // Compute bootstrap interval for acc
         //
         s = cts_info[i].acc.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].acc.v_bcl[j],
                                 cts_info[i].acc.v_bcu[j]);

         //
         // Compute bootstrap interval for fbias
         //
         s = cts_info[i].fbias.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].fbias.v_bcl[j],
                                 cts_info[i].fbias.v_bcu[j]);

         //
         // Compute bootstrap interval for pody
         //
         s = cts_info[i].pody.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].pody.v_bcl[j],
                                 cts_info[i].pody.v_bcu[j]);

         //
         // Compute bootstrap interval for podn
         //
         s = cts_info[i].podn.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].podn.v_bcl[j],
                                 cts_info[i].podn.v_bcu[j]);

         //
         // Compute bootstrap interval for pofd
         //
         s = cts_info[i].pofd.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].pofd.v_bcl[j],
                                 cts_info[i].pofd.v_bcu[j]);

         //
         // Compute bootstrap interval for far
         //
         s = cts_info[i].far.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].far.v_bcl[j],
                                 cts_info[i].far.v_bcu[j]);

         //
         // Compute bootstrap interval for csi
         //
         s = cts_info[i].csi.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].csi.v_bcl[j],
                                 cts_info[i].csi.v_bcu[j]);

         //
         // Compute bootstrap interval for gss
         //
         s = cts_info[i].gss.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].gss.v_bcl[j],
                                 cts_info[i].gss.v_bcu[j]);

         //
         // Compute bootstrap interval for hk
         //
         s = cts_info[i].hk.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].hk.v_bcl[j],
                                 cts_info[i].hk.v_bcu[j]);

         //
         // Compute bootstrap interval for hss
         //
         s = cts_info[i].hss.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].hss.v_bcl[j],
                                 cts_info[i].hss.v_bcu[j]);

         //
         // Compute bootstrap interval for odds
         //
         s = cts_info[i].odds.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].odds.v_bcl[j],
                                 cts_info[i].odds.v_bcu[j]);

         //
         // Compute bootstrap interval for lodds
         //
         s = cts_info[i].lodds.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].lodds.v_bcl[j],
                                 cts_info[i].lodds.v_bcu[j]);

         //
         // Compute bootstrap interval for orss
         //
         s = cts_info[i].orss.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].orss.v_bcl[j],
                                 cts_info[i].orss.v_bcu[j]);

         //
         // Compute bootstrap interval for eds
         //
         s = cts_info[i].eds.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].eds.v_bcl[j],
                                 cts_info[i].eds.v_bcu[j]);

         //
         // Compute bootstrap interval for seds
         //
         s = cts_info[i].seds.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].seds.v_bcl[j],
                                 cts_info[i].seds.v_bcu[j]);

         //
         // Compute bootstrap interval for edi
         //
         s = cts_info[i].edi.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].edi.v_bcl[j],
                                 cts_info[i].edi.v_bcu[j]);

         //
         // Compute bootstrap interval for sedi
         //
         s = cts_info[i].sedi.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].sedi.v_bcl[j],
                                 cts_info[i].sedi.v_bcu[j]);

         //
         // Compute bootstrap interval for bagss
         //
         s = cts_info[i].bagss.v;
         read_ldf(cts_i_file[i], c,   si_na);
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].bagss.v_bcl[j],
                                 cts_info[i].bagss.v_bcu[j]);
      } // end for i

   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_cts_stats_ci_bca() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";


      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   for(i=0; i<n_cts; i++) {
      remove_temp_file(cts_i_file[i]);
      remove_temp_file(cts_r_file[i]);
   }

   //
   // Deallocate memory
   //
   if(cts_tmp)    { delete [] cts_tmp;    cts_tmp    = (CTSInfo *)      0; }
   if(cts_i_out)  { delete [] cts_i_out;  cts_i_out  = (ofstream *)     0; }
   if(cts_r_out)  { delete [] cts_r_out;  cts_r_out  = (ofstream *)     0; }
   if(cts_i_file) { delete [] cts_i_file; cts_i_file = (ConcatString *) 0; }
   if(cts_r_file) { delete [] cts_r_file; cts_r_file = (ConcatString *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the multi-category statistics for the pairs provided.
// Compute bootstrap confidence intervals using the BCa method with
// the random number generator and number of replicates specified.
// Arguments:
//    gsl_rng is a pointer to the random number generator to be used.
//    f_na and o_na are the arrays to be bootstrapped.
//    b is the number of replicates to be used when bootstrapping.
//    MCTSInfo is the object for holding the stats.
//
////////////////////////////////////////////////////////////////////////

void compute_mcts_stats_ci_bca(const gsl_rng *rng_ptr,
                               const PairDataPoint &pd,
                               int b, MCTSInfo &mcts_info,
                               bool mcts_flag, bool rank_flag,
                               const char *tmp_dir) {
   int n = 0;
   int i, c;
   double s;
   NumArray i_na, ir_na, si_na, sr_na;
   MCTSInfo mcts_tmp;

   //
   // Temp file streams for categorical statistics
   //
   ofstream mcts_i_out, mcts_r_out;
   ConcatString mcts_i_file, mcts_r_file, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_mcts_stats_ci_bca() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute mulit-category stats from the raw data for each threshold
   // with the normal_ci flag set
   //
   compute_mctsinfo(pd, i_na, mcts_flag, true, mcts_info);

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the mcts_flag is off
   //
   if(n<=1 || b<1 || !mcts_flag) return;

   //
   // Initialize the MCTSInfo temporary object.
   //
   mcts_tmp = mcts_info;

   //
   // Build the temp file names
   //
   prefix << cs_erase << tmp_dir << "/tmp_mcts_i";
   mcts_i_file = make_temp_file_name(prefix.c_str(), NULL);

   prefix << cs_erase << tmp_dir << "/tmp_mcts_r";
   mcts_r_file = make_temp_file_name(prefix.c_str(), NULL);

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      mcts_i_out.open(mcts_i_file.c_str());
      mcts_r_out.open(mcts_r_file.c_str());
      if(!mcts_i_out || !mcts_r_out) {
         mlog << Error << "\ncompute_mcts_stats_ci_bca() -> "
              << "can't open one or more temporary files for writing:\n"
              << mcts_i_file << "\n"
              << mcts_r_file << "\n\n";
         throw(1);
      }

      //
      // Compute catgegorical stats from the raw data with the i-th data
      // point removed and write out to a temp file
      //
      for(i=0; i<n; i++) {
         compute_i_mctsinfo(pd, i, false, mcts_tmp);
         write_mctsinfo(mcts_i_out, mcts_tmp);
      } // end for i

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, n);

         //
         // Compute categorical stats for each replicate with the
         // cts_flag set and the normal_ci_flag unset
         //
         compute_mctsinfo(pd, ir_na, true, false, mcts_tmp);
         write_mctsinfo(mcts_r_out, mcts_tmp);
      }

      //
      // Close the temp files
      //
      mcts_i_out.close();
      mcts_r_out.close();

      //
      // Initialize column counter
      //
      c = 1;

      //
      // Compute bootstrap interval for acc
      //
      s = mcts_info.acc.v;
      read_ldf(mcts_i_file, c,   si_na);
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              mcts_info.alpha[i],
                              mcts_info.acc.v_bcl[i],
                              mcts_info.acc.v_bcu[i]);

      //
      // Compute bootstrap interval for hk
      //
      s = mcts_info.hk.v;
      read_ldf(mcts_i_file, c,   si_na);
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              mcts_info.alpha[i],
                              mcts_info.hk.v_bcl[i],
                              mcts_info.hk.v_bcu[i]);

      //
      // Compute bootstrap interval for hss
      //
      s = mcts_info.hss.v;
      read_ldf(mcts_i_file, c,   si_na);
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              mcts_info.alpha[i],
                              mcts_info.hss.v_bcl[i],
                              mcts_info.hss.v_bcu[i]);

      //
      // Compute bootstrap interval for ger
      //
      s = mcts_info.ger.v;
      read_ldf(mcts_i_file, c,   si_na);
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              mcts_info.alpha[i],
                              mcts_info.ger.v_bcl[i],
                              mcts_info.ger.v_bcu[i]);

   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_mcts_stats_ci_bca() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      //
      // Attempt to delete temp files
      //
      remove_temp_file(mcts_i_file);
      remove_temp_file(mcts_r_file);

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   remove_temp_file(mcts_i_file);
   remove_temp_file(mcts_r_file);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the continuous statistics for the pairs provided.
// Compute bootstrap confidence intervals using the BCa method with
// the random number generator and number of replicates specified.
// Arguments:
//    gsl_rng is a pointer to the random number generator to be used.
//    f_na and o_na are the arrays to be bootstrapped.
//    precip_flag specifies if precip is being compared.
//    b is the number of replicates to be used when bootstrapping.
//    CNTInfo is the object for holding the stats.
//
////////////////////////////////////////////////////////////////////////

void compute_cnt_stats_ci_bca(const gsl_rng *rng_ptr,
                              const PairDataPoint &pd,
                              bool precip_flag, bool rank_flag,
                              int b, CNTInfo &cnt_info,
                              const char *tmp_dir) {
   int n =0;
   int i, c;
   double s;
   NumArray i_na, ir_na, si_na, sr_na;
   CNTInfo cnt_tmp;

   //
   // Temp file streams for continuous statistics
   //
   ofstream cnt_i_out, cnt_r_out;
   ConcatString cnt_i_file, cnt_r_file, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_cnt_stats_ci_bca() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute continuous stats from the raw data with the
   // normal_ci_flag set
   //
   compute_cntinfo(pd, i_na, precip_flag, rank_flag, true, cnt_info);

   //
   // Do not compute bootstrap CI's if n<=1 or b==0
   //
   if(n<=1 || b<1) return;

   //
   // Build the temp file names
   //
   prefix << cs_erase << tmp_dir << "/tmp_cnt_i";
   cnt_i_file = make_temp_file_name(prefix.c_str(), NULL);

   prefix << cs_erase << tmp_dir << "/tmp_cnt_r";
   cnt_r_file = make_temp_file_name(prefix.c_str(), NULL);

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      cnt_i_out.open(cnt_i_file.c_str());
      cnt_r_out.open(cnt_r_file.c_str());
      if(!cnt_i_out || !cnt_r_out) {
         mlog << Error << "\ncompute_cnt_stats_ci_bca() -> "
              << "can't open one or more temporary files for writing:\n"
              << cnt_i_file << "\n"
              << cnt_r_file << "\n\n";
         throw(1);
      }

      //
      // Compute continuous stats from the raw data with the i-th data
      // point removed and write out to a temp file
      //
      for(i=0; i<n; i++) {
         compute_i_cntinfo(pd, i, precip_flag, false, false, cnt_tmp);
         write_cntinfo(cnt_i_out, cnt_tmp);
      }

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, n);

         //
         // Compute continuous stats for each replicate with the
         // rank_flag and normal_ci_flag unset
         //
         compute_cntinfo(pd, ir_na, precip_flag, false, false, cnt_tmp);
         write_cntinfo(cnt_r_out, cnt_tmp);
      }

      //
      // Close the temp files
      //
      cnt_i_out.close();
      cnt_r_out.close();

      //
      // Initialize column counter
      //
      c = 1;

      //
      // Compute bootstrap interval for fbar
      //
      s = cnt_info.fbar.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.fbar.v_bcl[i],
                              cnt_info.fbar.v_bcu[i]);

      //
      // Compute bootstrap interval for fstdev
      //
      s = cnt_info.fstdev.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.fstdev.v_bcl[i],
                              cnt_info.fstdev.v_bcu[i]);

      //
      // Compute bootstrap interval for obar
      //
      s = cnt_info.obar.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.obar.v_bcl[i],
                              cnt_info.obar.v_bcu[i]);

      //
      // Compute bootstrap interval for ostdev
      //
      s = cnt_info.ostdev.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.ostdev.v_bcl[i],
                              cnt_info.ostdev.v_bcu[i]);


      //
      // Compute bootstrap interval for pr_corr
      //
      s = cnt_info.pr_corr.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.pr_corr.v_bcl[i],
                              cnt_info.pr_corr.v_bcu[i]);

      //
      // Compute bootstrap interval for anom_corr
      //
      s = cnt_info.anom_corr.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.anom_corr.v_bcl[i],
                              cnt_info.anom_corr.v_bcu[i]);

      //
      // Compute bootstrap interval for rmsfa
      //
      s = cnt_info.rmsfa.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.rmsfa.v_bcl[i],
                              cnt_info.rmsfa.v_bcu[i]);

      //
      // Compute bootstrap interval for rmsoa
      //
      s = cnt_info.rmsoa.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.rmsoa.v_bcl[i],
                              cnt_info.rmsoa.v_bcu[i]);

      //
      // Compute bootstrap interval for anom_corr_uncntr
      //
      s = cnt_info.anom_corr_uncntr.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.anom_corr_uncntr.v_bcl[i],
                              cnt_info.anom_corr_uncntr.v_bcu[i]);

      //
      // Compute bootstrap interval for me
      //
      s = cnt_info.me.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.me.v_bcl[i],
                              cnt_info.me.v_bcu[i]);

      //
      // Compute bootstrap interval for me2
      //
      s = cnt_info.me2.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.me2.v_bcl[i],
                              cnt_info.me2.v_bcu[i]);

      //
      // Compute bootstrap interval for estdev
      //
      s = cnt_info.estdev.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.estdev.v_bcl[i],
                              cnt_info.estdev.v_bcu[i]);

      //
      // Compute bootstrap interval for mbias
      //
      s = cnt_info.mbias.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.mbias.v_bcl[i],
                              cnt_info.mbias.v_bcu[i]);

      //
      // Compute bootstrap interval for mae
      //
      s = cnt_info.mae.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.mae.v_bcl[i],
                              cnt_info.mae.v_bcu[i]);

      //
      // Compute bootstrap interval for mse
      //
      s = cnt_info.mse.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.mse.v_bcl[i],
                              cnt_info.mse.v_bcu[i]);

      //
      // Compute bootstrap interval for msess
      //
      s = cnt_info.msess.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.msess.v_bcl[i],
                              cnt_info.msess.v_bcu[i]);

      //
      // Compute bootstrap interval for bcmse
      //
      s = cnt_info.bcmse.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.bcmse.v_bcl[i],
                              cnt_info.bcmse.v_bcu[i]);

      //
      // Compute bootstrap interval for rmse
      //
      s = cnt_info.rmse.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.rmse.v_bcl[i],
                              cnt_info.rmse.v_bcu[i]);

      //
      // Compute bootstrap interval for e10
      //
      s = cnt_info.e10.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.e10.v_bcl[i],
                              cnt_info.e10.v_bcu[i]);

      //
      // Compute bootstrap interval for e25
      //
      s = cnt_info.e25.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.e25.v_bcl[i],
                              cnt_info.e25.v_bcu[i]);

      //
      // Compute bootstrap interval for e50
      //
      s = cnt_info.e50.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.e50.v_bcl[i],
                              cnt_info.e50.v_bcu[i]);

      //
      // Compute bootstrap interval for e75
      //
      s = cnt_info.e75.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.e75.v_bcl[i],
                              cnt_info.e75.v_bcu[i]);

      //
      // Compute bootstrap interval for e90
      //
      s = cnt_info.e90.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.e90.v_bcl[i],
                              cnt_info.e90.v_bcu[i]);

      //
      // Compute bootstrap interval for eiqr
      //
      s = cnt_info.eiqr.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.eiqr.v_bcl[i],
                              cnt_info.eiqr.v_bcu[i]);

      //
      // Compute bootstrap interval for mad
      //
      s = cnt_info.mad.v;
      read_ldf(cnt_i_file, c,   si_na);
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              cnt_info.alpha[i],
                              cnt_info.mad.v_bcl[i],
                              cnt_info.mad.v_bcu[i]);
   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_cnt_stats_ci_bca() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      //
      // Attempt to delete temp files
      //
      remove_temp_file(cnt_i_file);
      remove_temp_file(cnt_r_file);

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   remove_temp_file(cnt_i_file);
   remove_temp_file(cnt_r_file);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the categorical statistics for the pairs provided.
// Compute bootstrap confidence intervals using the percentile method
// with the random number generator and number of replicates specified.
//
////////////////////////////////////////////////////////////////////////

void compute_cts_stats_ci_perc(const gsl_rng *rng_ptr,
                               const PairDataPoint &pd,
                               int b, double m_prop,
                               CTSInfo *&cts_info, int n_cts,
                               bool cts_flag, bool rank_flag,
                               const char *tmp_dir) {
   int n = 0;
   int i, j, m, c;
   double s;
   NumArray i_na, ir_na, sr_na;
   CTSInfo *cts_tmp = (CTSInfo *) 0;

   //
   // Temp file streams for categorical statistics
   //
   ofstream *cts_r_out = (ofstream *) 0;
   ConcatString *cts_r_file = (ConcatString *) 0, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_cts_stats_ci_perc() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Compute size of the replicate
   //
   m = nint(m_prop * n);

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute categorical stats from the raw data for each threshold
   // with the normal_ci flag set
   //
   for(i=0; i<n_cts; i++) {
      compute_ctsinfo(pd, i_na, cts_flag, true, cts_info[i]);
   }

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the cts_flag is off
   //
   if(n<=1 || b<1 || !cts_flag) return;

   //
   // Allocate space to store categorical stats for each threshold
   // and for the output temp file streams
   //
   cts_tmp    = new CTSInfo [n_cts];
   cts_r_out  = new ofstream [n_cts];
   cts_r_file = new ConcatString [n_cts];
   for(i=0; i<n_cts; i++) {
      cts_tmp[i].fthresh = cts_info[i].fthresh;
      cts_tmp[i].othresh = cts_info[i].othresh;
   }

   //
   // Build the temp file names
   //
   for(i=0; i<n_cts; i++) {
      prefix << cs_erase << tmp_dir << "/tmp_cts_r_" << i;
      cts_r_file[i] = make_temp_file_name(prefix.c_str(), NULL);
   }

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      for(i=0; i<n_cts; i++) {
         cts_r_out[i].open(cts_r_file[i].c_str());
         if(!cts_r_out[i]) {
            mlog << Error << "\ncompute_cts_stats_ci_perc() -> "
                 << "can't open the temporary file for writing:\n"
                 << cts_r_file[i] << "\n\n";
           //
           // Attempt to delete temp files
           //
           for(i=0; i<n_cts; i++) remove_temp_file(cts_r_file[i]);

           if(cts_tmp)    { delete [] cts_tmp;    cts_tmp    = (CTSInfo *)      0; }
           if(cts_r_out)  { delete [] cts_r_out;  cts_r_out  = (ofstream *)     0; }
           if(cts_r_file) { delete [] cts_r_file; cts_r_file = (ConcatString *) 0; }

            throw(1);
         }
      }

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, m);

         //
         // Compute categorical stats for each replicate with the
         // cts_flag set and the normal_ci_flag unset
         //
         for(j=0; j<n_cts; j++) {
            compute_ctsinfo(pd, ir_na, true, false, cts_tmp[j]);
            write_ctsinfo(cts_r_out[j], cts_tmp[j]);
         } // end for j
      }

      //
      // Close the temp files
      //
      for(i=0; i<n_cts; i++) cts_r_out[i].close();

      //
      // Compute bootstrap intervals for each threshold value
      //
      for(i=0; i<n_cts; i++) {

         //
         // Initialize column counter
         //
         c = 1;

         //
         // Compute bootstrap interval for baser
         //
         s = cts_info[i].baser.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].baser.v_bcl[j],
                                 cts_info[i].baser.v_bcu[j]);

         //
         // Compute bootstrap interval for fmean
         //
         s = cts_info[i].fmean.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].fmean.v_bcl[j],
                                 cts_info[i].fmean.v_bcu[j]);

         //
         // Compute bootstrap interval for acc
         //
         s = cts_info[i].acc.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].acc.v_bcl[j],
                                 cts_info[i].acc.v_bcu[j]);

         //
         // Compute bootstrap interval for fbias
         //
         s = cts_info[i].fbias.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].fbias.v_bcl[j],
                                 cts_info[i].fbias.v_bcu[j]);

         //
         // Compute bootstrap interval for pody
         //
         s = cts_info[i].pody.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].pody.v_bcl[j],
                                 cts_info[i].pody.v_bcu[j]);

         //
         // Compute bootstrap interval for podn
         //
         s = cts_info[i].podn.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].podn.v_bcl[j],
                                 cts_info[i].podn.v_bcu[j]);

         //
         // Compute bootstrap interval for pofd
         //
         s = cts_info[i].pofd.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].pofd.v_bcl[j],
                                 cts_info[i].pofd.v_bcu[j]);

         //
         // Compute bootstrap interval for far
         //
         s = cts_info[i].far.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].far.v_bcl[j],
                                 cts_info[i].far.v_bcu[j]);

         //
         // Compute bootstrap interval for csi
         //
         s = cts_info[i].csi.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].csi.v_bcl[j],
                                 cts_info[i].csi.v_bcu[j]);

         //
         // Compute bootstrap interval for gss
         //
         s = cts_info[i].gss.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].gss.v_bcl[j],
                                 cts_info[i].gss.v_bcu[j]);

         //
         // Compute bootstrap interval for hk
         //
         s = cts_info[i].hk.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].hk.v_bcl[j],
                                 cts_info[i].hk.v_bcu[j]);

         //
         // Compute bootstrap interval for hss
         //
         s = cts_info[i].hss.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].hss.v_bcl[j],
                                 cts_info[i].hss.v_bcu[j]);

         //
         // Compute bootstrap interval for odds
         //
         s = cts_info[i].odds.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].odds.v_bcl[j],
                                 cts_info[i].odds.v_bcu[j]);

         //
         // Compute bootstrap interval for lodds
         //
         s = cts_info[i].lodds.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].lodds.v_bcl[j],
                                 cts_info[i].lodds.v_bcu[j]);

         //
         // Compute bootstrap interval for orss
         //
         s = cts_info[i].orss.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].orss.v_bcl[j],
                                 cts_info[i].orss.v_bcu[j]);

         //
         // Compute bootstrap interval for eds
         //
         s = cts_info[i].eds.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].eds.v_bcl[j],
                                 cts_info[i].eds.v_bcu[j]);

         //
         // Compute bootstrap interval for seds
         //
         s = cts_info[i].seds.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].seds.v_bcl[j],
                                 cts_info[i].seds.v_bcu[j]);

         //
         // Compute bootstrap interval for edi
         //
         s = cts_info[i].edi.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].edi.v_bcl[j],
                                 cts_info[i].edi.v_bcu[j]);

         //
         // Compute bootstrap interval for sedi
         //
         s = cts_info[i].sedi.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].sedi.v_bcl[j],
                                 cts_info[i].sedi.v_bcu[j]);

         //
         // Compute bootstrap interval for bagss
         //
         s = cts_info[i].bagss.v;
         read_ldf(cts_r_file[i], c++, sr_na);
         for(j=0; j<cts_info[i].n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 cts_info[i].alpha[j],
                                 cts_info[i].bagss.v_bcl[j],
                                 cts_info[i].bagss.v_bcu[j]);
      } // end for i

   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_cts_stats_ci_perc() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   for(i=0; i<n_cts; i++) {
      remove_temp_file(cts_r_file[i]);
   }

   //
   // Deallocate memory
   //
   if(cts_tmp)    { delete [] cts_tmp;    cts_tmp    = (CTSInfo *)      0; }
   if(cts_r_out)  { delete [] cts_r_out;  cts_r_out  = (ofstream *)     0; }
   if(cts_r_file) { delete [] cts_r_file; cts_r_file = (ConcatString *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the muli-category statistics for the pairs provided.
// Compute bootstrap confidence intervals using the percentile method
// with the random number generator and number of replicates specified.
//
////////////////////////////////////////////////////////////////////////

void compute_mcts_stats_ci_perc(const gsl_rng *rng_ptr,
                                const PairDataPoint &pd,
                                int b, double m_prop,
                                MCTSInfo &mcts_info,
                                bool mcts_flag, bool rank_flag,
                                const char *tmp_dir) {
   int n = 0;
   int i, m, c;
   double s;
   NumArray i_na, ir_na, sr_na;
   MCTSInfo mcts_tmp;

   //
   // Temp file streams for categorical statistics
   //
   ofstream mcts_r_out;
   ConcatString mcts_r_file, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_mcts_stats_ci_perc() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Compute size of the replicate
   //
   m = nint(m_prop * n);

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute categorical stats from the raw data for each threshold
   // with the normal_ci flag set
   //
   compute_mctsinfo(pd, i_na, mcts_flag, true, mcts_info);

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the mcts_flag is off
   //
   if(n<=1 || b<1 || !mcts_flag) return;

   //
   // Initialize the MCTSInfo temporary object.
   //
   mcts_tmp = mcts_info;

   //
   // Build the temp file names
   //
   prefix << cs_erase << tmp_dir << "/tmp_mcts_r";
   mcts_r_file = make_temp_file_name(prefix.c_str(), NULL);

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp file
      //
      mcts_r_out.open(mcts_r_file.c_str());
      if(!mcts_r_out) {
         mlog << Error << "\ncompute_mcts_stats_ci_perc() -> "
              << "can't open the temporary file for writing:\n"
              << mcts_r_file << "\n\n";
         throw(1);
      }

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, m);

         //
         // Compute multi-category stats for each replicate with the
         // mcts_flag set and the normal_ci_flag unset
         //
         compute_mctsinfo(pd, ir_na, true, false, mcts_tmp);
         write_mctsinfo(mcts_r_out, mcts_tmp);
      }

      //
      // Close the temp file
      //
      mcts_r_out.close();

      //
      // Initialize column counter
      //
      c = 1;

      //
      // Compute bootstrap interval for acc
      //
      s = mcts_info.acc.v;
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               mcts_info.alpha[i],
                               mcts_info.acc.v_bcl[i],
                               mcts_info.acc.v_bcu[i]);

      //
      // Compute bootstrap interval for hk
      //
      s = mcts_info.hk.v;
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               mcts_info.alpha[i],
                               mcts_info.hk.v_bcl[i],
                               mcts_info.hk.v_bcu[i]);

      //
      // Compute bootstrap interval for hss
      //
      s = mcts_info.hss.v;
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               mcts_info.alpha[i],
                               mcts_info.hss.v_bcl[i],
                               mcts_info.hss.v_bcu[i]);

      //
      // Compute bootstrap interval for ger
      //
      s = mcts_info.ger.v;
      read_ldf(mcts_r_file, c++, sr_na);
      for(i=0; i<mcts_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               mcts_info.alpha[i],
                               mcts_info.ger.v_bcl[i],
                               mcts_info.ger.v_bcu[i]);
   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_mcts_stats_ci_perc() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      //
      // Attempt to delete temp file
      //
      remove_temp_file(mcts_r_file);

      exit(i_err);
   } // end catch block

   //
   // Delete temp file
   //
   remove_temp_file(mcts_r_file);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the continuous statistics for the pairs provided.
// Compute bootstrap confidence intervals using the percentile method
// with the random number generator and number of replicates specified.
//
////////////////////////////////////////////////////////////////////////

void compute_cnt_stats_ci_perc(const gsl_rng *rng_ptr,
                               const PairDataPoint &pd,
                               bool precip_flag, bool rank_flag,
                               int b, double m_prop, CNTInfo &cnt_info,
                               const char *tmp_dir) {
   int n = 0;
   int i, m, c;
   double s;
   NumArray i_na, ir_na, sr_na;
   CNTInfo cnt_tmp;

   //
   // Temp file streams for continuous statistics
   //
   ofstream cnt_r_out;
   ConcatString cnt_r_file, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_cnt_stats_ci_perc() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Compute size of the replicate
   //
   m = nint(m_prop * n);

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute continuous stats from the raw data with the
   // normal_ci_flag set
   //
   compute_cntinfo(pd, i_na, precip_flag, rank_flag, true, cnt_info);

   //
   // Do not compute bootstrap CI's if n<=1 or b== 0
   //
   if(n<=1 || b<1) return;

   //
   // Build the temp file names
   //
   prefix << cs_erase << tmp_dir << "/tmp_cnt_r";
   cnt_r_file = make_temp_file_name(prefix.c_str(), NULL);

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      cnt_r_out.open(cnt_r_file.c_str());
      if(!cnt_r_out) {
         mlog << Error << "\ncompute_cnt_stats_ci_perc() -> "
              << "can't open the temporary file for writing:\n"
              << cnt_r_file << "\n\n";
         throw(1);
      }

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, m);

         //
         // Compute continuous stats for each replicate with the
         // rank_flag and normal_ci_flag unset
         //
         compute_cntinfo(pd, ir_na, precip_flag, false, false, cnt_tmp);
         write_cntinfo(cnt_r_out, cnt_tmp);
      }

      //
      // Close the temp files
      //
      cnt_r_out.close();

      //
      // Initialize column counter
      //
      c = 1;

      //
      // Compute bootstrap interval for fbar
      //
      s = cnt_info.fbar.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.fbar.v_bcl[i],
                               cnt_info.fbar.v_bcu[i]);

      //
      // Compute bootstrap interval for fstdev
      //
      s = cnt_info.fstdev.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.fstdev.v_bcl[i],
                               cnt_info.fstdev.v_bcu[i]);

      //
      // Compute bootstrap interval for obar
      //
      s = cnt_info.obar.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.obar.v_bcl[i],
                               cnt_info.obar.v_bcu[i]);

      //
      // Compute bootstrap interval for ostdev
      //
      s = cnt_info.ostdev.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.ostdev.v_bcl[i],
                               cnt_info.ostdev.v_bcu[i]);


      //
      // Compute bootstrap interval for pr_corr
      //
      s = cnt_info.pr_corr.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.pr_corr.v_bcl[i],
                               cnt_info.pr_corr.v_bcu[i]);

      //
      // Compute bootstrap interval for anom_corr
      //
      s = cnt_info.anom_corr.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.anom_corr.v_bcl[i],
                               cnt_info.anom_corr.v_bcu[i]);

      //
      // Compute bootstrap interval for rmsfa
      //
      s = cnt_info.rmsfa.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.rmsfa.v_bcl[i],
                               cnt_info.rmsfa.v_bcu[i]);

      //
      // Compute bootstrap interval for rmsoa
      //
      s = cnt_info.rmsoa.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.rmsoa.v_bcl[i],
                               cnt_info.rmsoa.v_bcu[i]);

      //
      // Compute bootstrap interval for anom_corr_uncntr
      //
      s = cnt_info.anom_corr_uncntr.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.anom_corr_uncntr.v_bcl[i],
                               cnt_info.anom_corr_uncntr.v_bcu[i]);

      //
      // Compute bootstrap interval for me
      //
      s = cnt_info.me.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.me.v_bcl[i],
                               cnt_info.me.v_bcu[i]);

      //
      // Compute bootstrap interval for me2
      //
      s = cnt_info.me2.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.me2.v_bcl[i],
                               cnt_info.me2.v_bcu[i]);

      //
      // Compute bootstrap interval for estdev
      //
      s = cnt_info.estdev.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.estdev.v_bcl[i],
                               cnt_info.estdev.v_bcu[i]);

      //
      // Compute bootstrap interval for mbias
      //
      s = cnt_info.mbias.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.mbias.v_bcl[i],
                               cnt_info.mbias.v_bcu[i]);

      //
      // Compute bootstrap interval for mae
      //
      s = cnt_info.mae.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.mae.v_bcl[i],
                               cnt_info.mae.v_bcu[i]);

      //
      // Compute bootstrap interval for mse
      //
      s = cnt_info.mse.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.mse.v_bcl[i],
                               cnt_info.mse.v_bcu[i]);

      //
      // Compute bootstrap interval for msess
      //
      s = cnt_info.msess.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.msess.v_bcl[i],
                               cnt_info.msess.v_bcu[i]);

      //
      // Compute bootstrap interval for bcmse
      //
      s = cnt_info.bcmse.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.bcmse.v_bcl[i],
                               cnt_info.bcmse.v_bcu[i]);

      //
      // Compute bootstrap interval for rmse
      //
      s = cnt_info.rmse.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.rmse.v_bcl[i],
                               cnt_info.rmse.v_bcu[i]);

      //
      // Compute bootstrap interval for e10
      //
      s = cnt_info.e10.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.e10.v_bcl[i],
                               cnt_info.e10.v_bcu[i]);

      //
      // Compute bootstrap interval for e25
      //
      s = cnt_info.e25.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.e25.v_bcl[i],
                               cnt_info.e25.v_bcu[i]);

      //
      // Compute bootstrap interval for e50
      //
      s = cnt_info.e50.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.e50.v_bcl[i],
                               cnt_info.e50.v_bcu[i]);

      //
      // Compute bootstrap interval for e75
      //
      s = cnt_info.e75.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.e75.v_bcl[i],
                               cnt_info.e75.v_bcu[i]);

      //
      // Compute bootstrap interval for e90
      //
      s = cnt_info.e90.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.e90.v_bcl[i],
                               cnt_info.e90.v_bcu[i]);

      //
      // Compute bootstrap interval for eiqr
      //
      s = cnt_info.eiqr.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.eiqr.v_bcl[i],
                               cnt_info.eiqr.v_bcu[i]);

      //
      // Compute bootstrap interval for mad
      //
      s = cnt_info.mad.v;
      read_ldf(cnt_r_file, c++, sr_na);
      for(i=0; i<cnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                               cnt_info.alpha[i],
                               cnt_info.mad.v_bcl[i],
                               cnt_info.mad.v_bcu[i]);
   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_cnt_stats_ci_perc() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      //
      // Attempt to delete temp files
      //
      remove_temp_file(cnt_r_file);

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   remove_temp_file(cnt_r_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_nbrcts_stats_ci_bca(const gsl_rng *rng_ptr,
                                 const PairDataPoint &pd,
                                 int b, NBRCTSInfo *&nbrcts_info,
                                 int n_nbrcts, bool nbrcts_flag,
                                 const char *tmp_dir) {
   int n = 0;
   int i, j, c;
   double s;
   NumArray i_na, ir_na, si_na, sr_na;
   NBRCTSInfo *nbrcts_tmp = (NBRCTSInfo *) 0;

   //
   // Temp file streams for categorical statistics
   //
   ofstream *nbrcts_i_out = (ofstream *) 0, *nbrcts_r_out = (ofstream *) 0;
   ConcatString *nbrcts_i_file = (ConcatString *) 0, *nbrcts_r_file = (ConcatString *) 0, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_nbrcts_stats_ci_bca() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute categorical stats from the raw data for each threshold
   // with the normal_ci flag set
   //
   for(i=0; i<n_nbrcts; i++) {
      compute_ctsinfo(pd, i_na, nbrcts_flag, true,
                      nbrcts_info[i].cts_info);
   }

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the nbrcts_flag is off
   //
   if(n<=1 || b<1 || !nbrcts_flag) return;

   //
   // Allocate space to store categorical stats for each threshold
   // and for the output temp file streams
   //
   nbrcts_tmp    = new NBRCTSInfo [n_nbrcts];
   nbrcts_i_out  = new ofstream [n_nbrcts];
   nbrcts_r_out  = new ofstream [n_nbrcts];
   nbrcts_i_file = new ConcatString [n_nbrcts];
   nbrcts_r_file = new ConcatString [n_nbrcts];
   for(i=0; i<n_nbrcts; i++) {
      nbrcts_tmp[i].cts_info.fthresh = nbrcts_info[i].cts_info.fthresh;
      nbrcts_tmp[i].cts_info.othresh = nbrcts_info[i].cts_info.othresh;
      nbrcts_tmp[i].fthresh          = nbrcts_info[i].fthresh;
      nbrcts_tmp[i].othresh          = nbrcts_info[i].othresh;
      nbrcts_tmp[i].cthresh          = nbrcts_info[i].cthresh;
   }

   //
   // Build the temp file names
   //
   for(i=0; i<n_nbrcts; i++) {
      prefix << cs_erase << tmp_dir << "/tmp_nbrcts_i_" << i;
      nbrcts_i_file[i] = make_temp_file_name(prefix.c_str(), NULL);

      prefix << cs_erase << tmp_dir << "/tmp_nbrcts_r_" << i;
      nbrcts_r_file[i] = make_temp_file_name(prefix.c_str(), NULL);
   }

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      for(i=0; i<n_nbrcts; i++) {
         nbrcts_i_out[i].open(nbrcts_i_file[i].c_str());
         nbrcts_r_out[i].open(nbrcts_r_file[i].c_str());
         if(!nbrcts_i_out[i] || !nbrcts_r_out[i]) {
            mlog << Error << "\ncompute_nbrcts_stats_ci_bca() -> "
                 << "can't open one or more temporary files for writing:\n"
                 << nbrcts_i_file[i] << "\n"
                 << nbrcts_r_file[i] << "\n\n";

            //
            // Attempt to delete temp files
            //
            for(i=0; i<n_nbrcts; i++) {
               remove_temp_file(nbrcts_i_file[i]);
               remove_temp_file(nbrcts_r_file[i]);
            }

            // deallocate memory
            if(nbrcts_tmp)    { delete [] nbrcts_tmp;    nbrcts_tmp    = (NBRCTSInfo *)   0; }
             if(nbrcts_i_out)  { delete [] nbrcts_i_out;  nbrcts_i_out  = (ofstream *)     0; }
            if(nbrcts_r_out)  { delete [] nbrcts_r_out;  nbrcts_r_out  = (ofstream *)     0; }
            if(nbrcts_i_file) { delete [] nbrcts_i_file; nbrcts_i_file = (ConcatString *) 0; }
            if(nbrcts_r_file) { delete [] nbrcts_r_file; nbrcts_r_file = (ConcatString *) 0; }

            throw(1);
         }
      }

      //
      // Compute catgegorical stats from the raw data with the i-th data
      // point removed and write out to a temp file
      //
      for(i=0; i<n_nbrcts; i++) {
         for(j=0; j<n; j++) {
            compute_i_ctsinfo(pd, j, false, nbrcts_tmp[i].cts_info);
            write_ctsinfo(nbrcts_i_out[i], nbrcts_tmp[i].cts_info);
         }
      } // end for i

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, n);

         //
         // Compute categorical stats for each replicate with the
         // nbrcts_flag flag set and the normal_ci_flag unset
         //
         for(j=0; j<n_nbrcts; j++) {
            compute_ctsinfo(pd, ir_na, true, false,
                            nbrcts_tmp[j].cts_info);
            write_ctsinfo(nbrcts_r_out[j], nbrcts_tmp[j].cts_info);
         } // end for j
      }

      //
      // Close the temp files
      //
      for(i=0; i<n_nbrcts; i++) {
         nbrcts_i_out[i].close();
         nbrcts_r_out[i].close();
      }

      //
      // Compute bootstrap intervals for each threshold value
      //
      for(i=0; i<n_nbrcts; i++) {

         //
         // Initialize column counter
         //
         c = 1;

         //
         // Compute bootstrap interval for baser
         //
         s = nbrcts_info[i].cts_info.baser.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.baser.v_bcl[j],
                                 nbrcts_info[i].cts_info.baser.v_bcu[j]);

         //
         // Compute bootstrap interval for fmean
         //
         s = nbrcts_info[i].cts_info.fmean.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.fmean.v_bcl[j],
                                 nbrcts_info[i].cts_info.fmean.v_bcu[j]);

         //
         // Compute bootstrap interval for acc
         //
         s = nbrcts_info[i].cts_info.acc.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.acc.v_bcl[j],
                                 nbrcts_info[i].cts_info.acc.v_bcu[j]);

         //
         // Compute bootstrap interval for fbias
         //
         s = nbrcts_info[i].cts_info.fbias.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.fbias.v_bcl[j],
                                 nbrcts_info[i].cts_info.fbias.v_bcu[j]);

         //
         // Compute bootstrap interval for pody
         //
         s = nbrcts_info[i].cts_info.pody.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.pody.v_bcl[j],
                                 nbrcts_info[i].cts_info.pody.v_bcu[j]);

         //
         // Compute bootstrap interval for podn
         //
         s = nbrcts_info[i].cts_info.podn.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.podn.v_bcl[j],
                                 nbrcts_info[i].cts_info.podn.v_bcu[j]);

         //
         // Compute bootstrap interval for pofd
         //
         s = nbrcts_info[i].cts_info.pofd.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.pofd.v_bcl[j],
                                 nbrcts_info[i].cts_info.pofd.v_bcu[j]);

         //
         // Compute bootstrap interval for far
         //
         s = nbrcts_info[i].cts_info.far.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.far.v_bcl[j],
                                 nbrcts_info[i].cts_info.far.v_bcu[j]);

         //
         // Compute bootstrap interval for csi
         //
         s = nbrcts_info[i].cts_info.csi.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.csi.v_bcl[j],
                                 nbrcts_info[i].cts_info.csi.v_bcu[j]);

         //
         // Compute bootstrap interval for gss
         //
         s = nbrcts_info[i].cts_info.gss.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.gss.v_bcl[j],
                                 nbrcts_info[i].cts_info.gss.v_bcu[j]);

         //
         // Compute bootstrap interval for hk
         //
         s = nbrcts_info[i].cts_info.hk.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.hk.v_bcl[j],
                                 nbrcts_info[i].cts_info.hk.v_bcu[j]);

         //
         // Compute bootstrap interval for hss
         //
         s = nbrcts_info[i].cts_info.hss.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.hss.v_bcl[j],
                                 nbrcts_info[i].cts_info.hss.v_bcu[j]);

         //
         // Compute bootstrap interval for odds
         //
         s = nbrcts_info[i].cts_info.odds.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.odds.v_bcl[j],
                                 nbrcts_info[i].cts_info.odds.v_bcu[j]);

         //
         // Compute bootstrap interval for lodds
         //
         s = nbrcts_info[i].cts_info.lodds.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.lodds.v_bcl[j],
                                 nbrcts_info[i].cts_info.lodds.v_bcu[j]);

         //
         // Compute bootstrap interval for orss
         //
         s = nbrcts_info[i].cts_info.orss.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.orss.v_bcl[j],
                                 nbrcts_info[i].cts_info.orss.v_bcu[j]);

         //
         // Compute bootstrap interval for eds
         //
         s = nbrcts_info[i].cts_info.eds.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.eds.v_bcl[j],
                                 nbrcts_info[i].cts_info.eds.v_bcu[j]);

         //
         // Compute bootstrap interval for seds
         //
         s = nbrcts_info[i].cts_info.seds.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.seds.v_bcl[j],
                                 nbrcts_info[i].cts_info.seds.v_bcu[j]);

         //
         // Compute bootstrap interval for edi
         //
         s = nbrcts_info[i].cts_info.edi.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.edi.v_bcl[j],
                                 nbrcts_info[i].cts_info.edi.v_bcu[j]);

         //
         // Compute bootstrap interval for sedi
         //
         s = nbrcts_info[i].cts_info.sedi.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.sedi.v_bcl[j],
                                 nbrcts_info[i].cts_info.sedi.v_bcu[j]);

         //
         // Compute bootstrap interval for bagss
         //
         s = nbrcts_info[i].cts_info.bagss.v;
         read_ldf(nbrcts_i_file[i], c,   si_na);
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_bca_interval(s, si_na, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.bagss.v_bcl[j],
                                 nbrcts_info[i].cts_info.bagss.v_bcu[j]);
      } // end for i

   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_nbrcts_stats_ci_bca() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   for(i=0; i<n_nbrcts; i++) {
      remove_temp_file(nbrcts_i_file[i]);
      remove_temp_file(nbrcts_r_file[i]);
   }

   //
   // Deallocate memory
   //
   if(nbrcts_tmp)    { delete [] nbrcts_tmp;    nbrcts_tmp    = (NBRCTSInfo *)   0; }
   if(nbrcts_i_out)  { delete [] nbrcts_i_out;  nbrcts_i_out  = (ofstream *)     0; }
   if(nbrcts_r_out)  { delete [] nbrcts_r_out;  nbrcts_r_out  = (ofstream *)     0; }
   if(nbrcts_i_file) { delete [] nbrcts_i_file; nbrcts_i_file = (ConcatString *) 0; }
   if(nbrcts_r_file) { delete [] nbrcts_r_file; nbrcts_r_file = (ConcatString *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_nbrcnt_stats_ci_bca(const gsl_rng *rng_ptr,
                                 const PairDataPoint &pd,
                                 const PairDataPoint &pd_thr,
                                 int b, NBRCNTInfo &nbrcnt_info,
                                 bool nbrcnt_flag,
                                 const char *tmp_dir) {
   int n = 0;
   int i, c;
   double s;
   NumArray i_na, ir_na, si_na, sr_na;
   NBRCNTInfo nbrcnt_tmp;

   //
   // Temp file streams for continuous statistics
   //
   ofstream nbrcnt_i_out, nbrcnt_r_out;
   ConcatString nbrcnt_i_file, nbrcnt_r_file, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_nbrcnt_stats_ci_bca() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute continuous stats from the raw data
   //
   compute_nbrcntinfo(pd, pd_thr, i_na, nbrcnt_info, nbrcnt_flag);

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the nbrcnt_flag is off
   //
   if(n<=1 || b<1 || !nbrcnt_flag) return;

   //
   // Build the temp file names
   //
   prefix << cs_erase << tmp_dir << "/tmp_nbrcnt_i";
   nbrcnt_i_file = make_temp_file_name(prefix.c_str(), NULL);

   prefix << cs_erase << tmp_dir << "/tmp_nbrcnt_r";
   nbrcnt_r_file = make_temp_file_name(prefix.c_str(), NULL);

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      nbrcnt_i_out.open(nbrcnt_i_file.c_str());
      nbrcnt_r_out.open(nbrcnt_r_file.c_str());
      if(!nbrcnt_i_out || !nbrcnt_r_out) {
         mlog << Error << "\ncompute_nbrcnt_stats_ci_bca() -> "
              << "can't open one or more temporary files for writing:\n"
              << nbrcnt_i_file << "\n"
              << nbrcnt_r_file << "\n\n";
         throw(1);
      }

      //
      // Compute continuous stats from the raw data with the i-th data
      // point removed and write out to a temp file
      //
      for(i=0; i<n; i++) {
         compute_i_nbrcntinfo(pd, pd_thr, i, nbrcnt_tmp);
         write_nbrcntinfo(nbrcnt_i_out, nbrcnt_tmp);
      }

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, n);

         //
         // Compute continuous stats for each replicate
         //
         compute_nbrcntinfo(pd, pd_thr, ir_na, nbrcnt_tmp, 1);
         write_nbrcntinfo(nbrcnt_r_out, nbrcnt_tmp);
      }

      //
      // Close the temp files
      //
      nbrcnt_i_out.close();
      nbrcnt_r_out.close();

      //
      // Initialize column counter
      //
      c = 1;

      //
      // Compute bootstrap interval for FBS
      //
      s = nbrcnt_info.fbs.v;
      read_ldf(nbrcnt_i_file, c,   si_na);
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.fbs.v_bcl[i],
                              nbrcnt_info.fbs.v_bcu[i]);

      //
      // Compute bootstrap interval for FSS
      //
      s = nbrcnt_info.fss.v;
      read_ldf(nbrcnt_i_file, c,   si_na);
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.fss.v_bcl[i],
                              nbrcnt_info.fss.v_bcu[i]);

      //
      // Compute bootstrap interval for AFSS
      //
      s = nbrcnt_info.afss.v;
      read_ldf(nbrcnt_i_file, c,   si_na);
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.afss.v_bcl[i],
                              nbrcnt_info.afss.v_bcu[i]);

      //
      // Compute bootstrap interval for UFSS
      //
      s = nbrcnt_info.ufss.v;
      read_ldf(nbrcnt_i_file, c,   si_na);
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.ufss.v_bcl[i],
                              nbrcnt_info.ufss.v_bcu[i]);

      //
      // Compute bootstrap interval for F_RATE
      //
      s = nbrcnt_info.f_rate.v;
      read_ldf(nbrcnt_i_file, c,   si_na);
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.f_rate.v_bcl[i],
                              nbrcnt_info.f_rate.v_bcu[i]);

      //
      // Compute bootstrap interval for O_RATE
      //
      s = nbrcnt_info.o_rate.v;
      read_ldf(nbrcnt_i_file, c,   si_na);
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_bca_interval(s, si_na, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.o_rate.v_bcl[i],
                              nbrcnt_info.o_rate.v_bcu[i]);

   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_nbrcnt_stats_ci_bca() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      //
      // Attempt to delete temp files
      //
      remove_temp_file(nbrcnt_i_file);
      remove_temp_file(nbrcnt_r_file);

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   remove_temp_file(nbrcnt_i_file);
   remove_temp_file(nbrcnt_r_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_nbrcts_stats_ci_perc(const gsl_rng *rng_ptr,
                                  const PairDataPoint &pd,
                                  int b, double m_prop,
                                  NBRCTSInfo *&nbrcts_info,
                                  int n_nbrcts, bool nbrcts_flag,
                                  const char *tmp_dir) {
   int n = 0;
   int i, j, c;
   double s;
   NumArray i_na, ir_na, sr_na;
   NBRCTSInfo *nbrcts_tmp = ( NBRCTSInfo *) 0;

   //
   // Temp file streams for categorical statistics
   //
   ofstream *nbrcts_r_out = (ofstream *) 0;
   ConcatString *nbrcts_r_file = (ConcatString *) 0, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_nbrcts_stats_ci_perc() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute categorical stats from the raw data for each threshold
   // with the normal_ci flag set
   //
   for(i=0; i<n_nbrcts; i++) {
      compute_ctsinfo(pd, i_na, nbrcts_flag, true,
                      nbrcts_info[i].cts_info);
   }

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the nbrcts_flag is off
   //
   if(n<=1 || b<1 || !nbrcts_flag) return;

   //
   // Allocate space to store categorical stats for each threshold
   // and for the output temp file streams
   //
   nbrcts_tmp    = new NBRCTSInfo [n_nbrcts];
   nbrcts_r_out  = new ofstream [n_nbrcts];
   nbrcts_r_file = new ConcatString [n_nbrcts];
   for(i=0; i<n_nbrcts; i++) {
      nbrcts_tmp[i].cts_info.fthresh = nbrcts_info[i].cts_info.fthresh;
      nbrcts_tmp[i].cts_info.othresh = nbrcts_info[i].cts_info.othresh;
      nbrcts_tmp[i].fthresh          = nbrcts_info[i].fthresh;
      nbrcts_tmp[i].othresh          = nbrcts_info[i].othresh;
      nbrcts_tmp[i].cthresh          = nbrcts_info[i].cthresh;
   }

   //
   // Build the temp file names
   //
   for(i=0; i<n_nbrcts; i++) {
      prefix << cs_erase << tmp_dir << "/tmp_nbrcts_r_" << i;
      nbrcts_r_file[i] = make_temp_file_name(prefix.c_str(), NULL);
   }

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      for(i=0; i<n_nbrcts; i++) {
         nbrcts_r_out[i].open(nbrcts_r_file[i].c_str());
         if(!nbrcts_r_out[i]) {
            mlog << Error << "\ncompute_nbrcts_stats_ci_perc() -> "
                 << "can't open the temporary file for writing:\n"
                 << nbrcts_r_file[i] << "\n\n";

            //
          // Attempt to delete temp files
          //
          for(i=0; i<n_nbrcts; i++) remove_temp_file(nbrcts_r_file[i]);
         // Deallocate memory
         //
         if(nbrcts_tmp)    { delete [] nbrcts_tmp;    nbrcts_tmp    = (NBRCTSInfo *)   0; }
         if(nbrcts_r_out)  { delete [] nbrcts_r_out;  nbrcts_r_out  = (ofstream *)     0; }
         if(nbrcts_r_file) { delete [] nbrcts_r_file; nbrcts_r_file = (ConcatString *) 0; }

            throw(1);
         }
      }

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, n);

         //
         // Compute categorical stats for each replicate with the
         // nbrcts_flag set and the normal_ci_flag unset
         //
         for(j=0; j<n_nbrcts; j++) {
            compute_ctsinfo(pd, ir_na, true, false,
                            nbrcts_tmp[j].cts_info);
            write_ctsinfo(nbrcts_r_out[j], nbrcts_tmp[j].cts_info);
         } // end for j
      }

      //
      // Close the temp files
      //
      for(i=0; i<n_nbrcts; i++) nbrcts_r_out[i].close();

      //
      // Compute bootstrap intervals for each threshold value
      //
      for(i=0; i<n_nbrcts; i++) {

         //
         // Initialize column counter
         //
         c = 1;

         //
         // Compute bootstrap interval for baser
         //
         s = nbrcts_info[i].cts_info.baser.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.baser.v_bcl[j],
                                  nbrcts_info[i].cts_info.baser.v_bcu[j]);

         //
         // Compute bootstrap interval for fmean
         //
         s = nbrcts_info[i].cts_info.fmean.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.fmean.v_bcl[j],
                                  nbrcts_info[i].cts_info.fmean.v_bcu[j]);

         //
         // Compute bootstrap interval for acc
         //
         s = nbrcts_info[i].cts_info.acc.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.acc.v_bcl[j],
                                  nbrcts_info[i].cts_info.acc.v_bcu[j]);

         //
         // Compute bootstrap interval for fbias
         //
         s = nbrcts_info[i].cts_info.fbias.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.fbias.v_bcl[j],
                                  nbrcts_info[i].cts_info.fbias.v_bcu[j]);

         //
         // Compute bootstrap interval for pody
         //
         s = nbrcts_info[i].cts_info.pody.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.pody.v_bcl[j],
                                  nbrcts_info[i].cts_info.pody.v_bcu[j]);

         //
         // Compute bootstrap interval for podn
         //
         s = nbrcts_info[i].cts_info.podn.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.podn.v_bcl[j],
                                  nbrcts_info[i].cts_info.podn.v_bcu[j]);

         //
         // Compute bootstrap interval for pofd
         //
         s = nbrcts_info[i].cts_info.pofd.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.pofd.v_bcl[j],
                                  nbrcts_info[i].cts_info.pofd.v_bcu[j]);

         //
         // Compute bootstrap interval for far
         //
         s = nbrcts_info[i].cts_info.far.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.far.v_bcl[j],
                                  nbrcts_info[i].cts_info.far.v_bcu[j]);

         //
         // Compute bootstrap interval for csi
         //
         s = nbrcts_info[i].cts_info.csi.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.csi.v_bcl[j],
                                  nbrcts_info[i].cts_info.csi.v_bcu[j]);

         //
         // Compute bootstrap interval for gss
         //
         s = nbrcts_info[i].cts_info.gss.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.gss.v_bcl[j],
                                  nbrcts_info[i].cts_info.gss.v_bcu[j]);

         //
         // Compute bootstrap interval for hk
         //
         s = nbrcts_info[i].cts_info.hk.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.hk.v_bcl[j],
                                  nbrcts_info[i].cts_info.hk.v_bcu[j]);

         //
         // Compute bootstrap interval for hss
         //
         s = nbrcts_info[i].cts_info.hss.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.hss.v_bcl[j],
                                  nbrcts_info[i].cts_info.hss.v_bcu[j]);

         //
         // Compute bootstrap interval for odds
         //
         s = nbrcts_info[i].cts_info.odds.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                  nbrcts_info[i].cts_info.alpha[j],
                                  nbrcts_info[i].cts_info.odds.v_bcl[j],
                                  nbrcts_info[i].cts_info.odds.v_bcu[j]);

         //
         // Compute bootstrap interval for lodds
         //
         s = nbrcts_info[i].cts_info.lodds.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.lodds.v_bcl[j],
                                 nbrcts_info[i].cts_info.lodds.v_bcu[j]);

         //
         // Compute bootstrap interval for orss
         //
         s = nbrcts_info[i].cts_info.orss.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.orss.v_bcl[j],
                                 nbrcts_info[i].cts_info.orss.v_bcu[j]);

         //
         // Compute bootstrap interval for eds
         //
         s = nbrcts_info[i].cts_info.eds.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.eds.v_bcl[j],
                                 nbrcts_info[i].cts_info.eds.v_bcu[j]);

         //
         // Compute bootstrap interval for seds
         //
         s = nbrcts_info[i].cts_info.seds.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.seds.v_bcl[j],
                                 nbrcts_info[i].cts_info.seds.v_bcu[j]);

         //
         // Compute bootstrap interval for edi
         //
         s = nbrcts_info[i].cts_info.edi.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.edi.v_bcl[j],
                                 nbrcts_info[i].cts_info.edi.v_bcu[j]);

         //
         // Compute bootstrap interval for sedi
         //
         s = nbrcts_info[i].cts_info.sedi.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.sedi.v_bcl[j],
                                 nbrcts_info[i].cts_info.sedi.v_bcu[j]);

         //
         // Compute bootstrap interval for bagss
         //
         s = nbrcts_info[i].cts_info.bagss.v;
         read_ldf(nbrcts_r_file[i], c++, sr_na);
         for(j=0; j<nbrcts_info[i].cts_info.n_alpha; j++)
            compute_perc_interval(s, sr_na,
                                 nbrcts_info[i].cts_info.alpha[j],
                                 nbrcts_info[i].cts_info.bagss.v_bcl[j],
                                 nbrcts_info[i].cts_info.bagss.v_bcu[j]);
      } // end for i

   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_nbrcts_stats_ci_perc() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   for(i=0; i<n_nbrcts; i++) {
      remove_temp_file(nbrcts_r_file[i]);
   }

   //
   // Deallocate memory
   //
   if(nbrcts_tmp)    { delete [] nbrcts_tmp;    nbrcts_tmp    = (NBRCTSInfo *)   0; }
   if(nbrcts_r_out)  { delete [] nbrcts_r_out;  nbrcts_r_out  = (ofstream *)     0; }
   if(nbrcts_r_file) { delete [] nbrcts_r_file; nbrcts_r_file = (ConcatString *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_nbrcnt_stats_ci_perc(const gsl_rng *rng_ptr,
                                  const PairDataPoint &pd,
                                  const PairDataPoint &pd_thr,
                                  int b, double m_prop,
                                  NBRCNTInfo &nbrcnt_info,
                                  bool nbrcnt_flag,
                                  const char *tmp_dir) {
   int n = 0;
   int i, c;
   double s;
   NumArray i_na, ir_na, sr_na;
   NBRCNTInfo nbrcnt_tmp;

   //
   // Temp file streams for continuous statistics
   //
   ofstream nbrcnt_r_out;
   ConcatString nbrcnt_r_file, prefix;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(pd.f_na.n() != pd.o_na.n()) {
      mlog << Error << "\ncompute_nbrcnt_stats_ci_perc() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n";
      exit(1);
   }
   else {
      n = pd.f_na.n();
   }

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   //
   // Compute continuous stats from the raw data
   //
   compute_nbrcntinfo(pd, pd_thr, i_na, nbrcnt_info, nbrcnt_flag);

   //
   // Do not compute bootstrap CI's if n<=1, the number of replicates
   // is zero, or the nbrcnt_flag is off
   //
   if(n<=1 || b<1 || !nbrcnt_flag) return;

   //
   // Build the temp file names
   //
   prefix << cs_erase << tmp_dir << "/tmp_nbrcnt_r";
   nbrcnt_r_file = make_temp_file_name(prefix.c_str(), NULL);

   //
   // Enclose computations in a try block to catch any errors and
   // delete the temp files before exiting
   //
   try {

      //
      // Open up the temp files
      //
      nbrcnt_r_out.open(nbrcnt_r_file.c_str());
      if(!nbrcnt_r_out) {
         mlog << Error << "\ncompute_nbrcnt_stats_ci_perc() -> "
              << "can't open the temporary file for writing:\n"
              << nbrcnt_r_file << "\n\n";
         throw(1);
      }

      //
      // Resample the array of indices with replacement
      //
      for(i=0; i<b; i++) {

         ran_sample(rng_ptr, i_na, ir_na, n);

         //
         // Compute continuous stats for each replicate
         //
         compute_nbrcntinfo(pd, pd_thr, ir_na, nbrcnt_tmp, 1);
         write_nbrcntinfo(nbrcnt_r_out, nbrcnt_tmp);
      }

      //
      // Close the temp files
      //
      nbrcnt_r_out.close();

      //
      // Initialize column counter
      //
      c = 1;

      //
      // Compute bootstrap interval for FBS
      //
      s = nbrcnt_info.fbs.v;
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.fbs.v_bcl[i],
                              nbrcnt_info.fbs.v_bcu[i]);

      //
      // Compute bootstrap interval for FSS
      //
      s = nbrcnt_info.fss.v;
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.fss.v_bcl[i],
                              nbrcnt_info.fss.v_bcu[i]);

      //
      // Compute bootstrap interval for AFSS
      //
      s = nbrcnt_info.afss.v;
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.afss.v_bcl[i],
                              nbrcnt_info.afss.v_bcu[i]);

      //
      // Compute bootstrap interval for UFSS
      //
      s = nbrcnt_info.ufss.v;
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.ufss.v_bcl[i],
                              nbrcnt_info.ufss.v_bcu[i]);

      //
      // Compute bootstrap interval for F_RATE
      //
      s = nbrcnt_info.f_rate.v;
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.f_rate.v_bcl[i],
                              nbrcnt_info.f_rate.v_bcu[i]);

      //
      // Compute bootstrap interval for O_RATE
      //
      s = nbrcnt_info.o_rate.v;
      read_ldf(nbrcnt_r_file, c++, sr_na);
      for(i=0; i<nbrcnt_info.n_alpha; i++)
         compute_perc_interval(s, sr_na,
                              nbrcnt_info.alpha[i],
                              nbrcnt_info.o_rate.v_bcl[i],
                              nbrcnt_info.o_rate.v_bcu[i]);

   } // end try block

   //
   // Catch any thrown errors
   //
   catch(int i_err) {

      mlog << Error << "\ncompute_nbrcnt_stats_ci_perc() -> "
           << "encountered an error value of " << i_err
           << ".  Deleting temp files before exiting.\n\n";

      //
      // Attempt to delete temp files
      //
      remove_temp_file(nbrcnt_r_file);

      exit(i_err);
   } // end catch block

   //
   // Delete temp files
   //
   remove_temp_file(nbrcnt_r_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_mean_stdev_ci_bca(const gsl_rng *rng_ptr,
                               const NumArray &v_na,
                               int b, double alpha,
                               CIInfo &mean_ci, CIInfo &stdev_ci) {
   int n, i;
   NumArray i_na, ir_na;
   NumArray meani_na, meanr_na, stdevi_na, stdevr_na;
   CIInfo mean_tmp, stdev_tmp;

   //
   // Get the number of values in the array
   //
   n = v_na.n();

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   compute_mean_stdev(v_na, i_na, true, alpha, mean_ci, stdev_ci);

   //
   // Do not compute bootstrap CI's if n<=1 or the number of
   // replicates is zero
   //
   if(n<=1 || b<1) return;

   //
   // Compute the mean and standard deviation from the raw data
   // with the i-th data point removed
   //
   for(i=0; i<n; i++) {
      compute_i_mean_stdev(v_na, false, alpha, i,
                           mean_tmp, stdev_tmp);

      //
      // Store the mean and standard deviation for this replicate
      //
      if(!is_bad_data(mean_tmp.v))  meani_na.add(mean_tmp.v);
      if(!is_bad_data(stdev_tmp.v)) stdevi_na.add(stdev_tmp.v);
   }

   //
   // Resample the array of indices with replacement
   //
   for(i=0; i<b; i++) {

      ran_sample(rng_ptr, i_na, ir_na, n);

      //
      // Compute the mean and standard deviation for each replicate
      //
      compute_mean_stdev(v_na, ir_na, false, alpha, mean_tmp, stdev_tmp);

      //
      // Store the mean and standard deviation for this replicate
      //
      if(!is_bad_data(mean_tmp.v))  meanr_na.add(mean_tmp.v);
      if(!is_bad_data(stdev_tmp.v)) stdevr_na.add(stdev_tmp.v);
   }

   //
   // Compute bootstrap interval for the mean
   //
   compute_bca_interval(mean_ci.v, meani_na, meanr_na,
                        alpha, mean_ci.v_bcl[0], mean_ci.v_bcu[0]);

   //
   // Compute bootstrap interval for the standard deviation
   //
   compute_bca_interval(stdev_ci.v, stdevi_na, stdevr_na,
                        alpha, stdev_ci.v_bcl[0], stdev_ci.v_bcu[0]);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_mean_stdev_ci_perc(const gsl_rng *rng_ptr,
                                const NumArray &v_na,
                                int b, double m_prop, double alpha,
                                CIInfo &mean_ci, CIInfo &stdev_ci) {
   int n, i;
   NumArray i_na, ir_na;
   NumArray meanr_na, stdevr_na;
   CIInfo mean_tmp, stdev_tmp;

   //
   // Get the number of values in the array
   //
   n = v_na.n();

   //
   // Setup the index array
   //
   i_na.add_seq(0, n-1);

   compute_mean_stdev(v_na, i_na, true, alpha, mean_ci, stdev_ci);

   //
   // Do not compute bootstrap CI's if n<=1 or the number of
   // replicates is zero
   //
   if(n<=1 || b<1) return;

   //
   // Resample the array of indices with replacement
   //
   for(i=0; i<b; i++) {

      ran_sample(rng_ptr, i_na, ir_na, n);

      //
      // Compute the mean and standard deviation for each replicate
      //
      compute_mean_stdev(v_na, ir_na, false, alpha, mean_tmp, stdev_tmp);

      //
      // Store the mean and standard deviation for this replicate
      //
      if(!is_bad_data(mean_tmp.v))  meanr_na.add(mean_tmp.v);
      if(!is_bad_data(stdev_tmp.v)) stdevr_na.add(stdev_tmp.v);
   }

   //
   // Compute bootstrap interval for the mean
   //
   compute_perc_interval(mean_ci.v, meanr_na,
                         alpha, mean_ci.v_bcl[0], mean_ci.v_bcu[0]);

   //
   // Compute bootstrap interval for the standard deviation
   //
   compute_perc_interval(stdev_ci.v, stdevr_na,
                         alpha, stdev_ci.v_bcl[0], stdev_ci.v_bcu[0]);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for the statistic (s) from the array
// resampled statistics (s_hat) using the BCa method.
// Arguments:
//    s  is the statistic computed from the raw data.
//    si is an array of statistics recomputed n times with the i-th
//       raw data point removed each time.
//    sr is an array of statistics recomputed from b replicates of the
//       raw data.
//    alpha is the alpha confidence value to be used.
//    s_bcl and s_bcu hold the computed bootstrap confidence interval.
//
////////////////////////////////////////////////////////////////////////

void compute_bca_interval(double s, NumArray &si_na,
                                    NumArray &sr_na,
                          double alpha,
                          double &s_bcl, double &s_bcu) {
   int i, n, b, count;
   double cv_l, cv_u;
   double a_hat, z_hat, si_bar, p, diff, sum, num, den, a1, a2;

   n = si_na.n();
   b = sr_na.n();
   if(n == 0 || b == 0) {
      s_bcl = bad_data_double;
      s_bcu = bad_data_double;

      return;
   }

   //
   // Compute the critical values for the Normal distribution.
   //
   cv_l = normal_cdf_inv(alpha/2.0, 0.0, 1.0);
   cv_u = normal_cdf_inv(1.0 - (alpha/2.0), 0.0, 1.0);

   //
   // Estimate z_hat as the quantile of the standard normal
   // distribution that corresponds to the proportion of bootstrap
   // replications of s that are less than s.
   //
   for(i=0, count=0; i<b; i++) if(sr_na[i] < s) count++;
   p = (double) count/b;
   z_hat = normal_cdf_inv(p, 0.0, 1.0);

   //
   // Compute the mean of the si array.
   //
   for(i=0, sum=0; i<n; i++) sum += si_na[i];
   si_bar = (double) sum/n;

   //
   // Estimate the acceleration parameter (a_hat).
   //
   for(i=0, num=0, sum=0; i<n; i++) {
      diff = si_bar - si_na[i];
      num += pow(diff, 3.0);
      sum += pow(diff, 2.0);
   } // end for i
   den = 6.0*pow(sum, 1.5);

   if(is_eq(den, 0.0)) {
      s_bcl = bad_data_double;
      s_bcu = bad_data_double;
   }
   else {
      a_hat = num/den;

      a1 = normal_cdf(z_hat + (z_hat + cv_l)/(1.0 - a_hat*(z_hat + cv_l)),
                      0.0, 1.0);
      a2 = normal_cdf(z_hat + (z_hat + cv_u)/(1.0 - a_hat*(z_hat + cv_u)),
                      0.0, 1.0);

      //
      // Compute the lower and upper bootstrap confidence limits by
      // sorting the array of replicates and taking the a1-th and the
      // a2-th percentile of the data.
      //
      s_bcl = sr_na.percentile_array(a1);
      s_bcu = sr_na.percentile_array(a2);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute a confidence interval for the statistic (s) from the array
// resampled statistics (s_hat) using the percentile method.
//
////////////////////////////////////////////////////////////////////////

void compute_perc_interval(double s, NumArray &sr_na,
                           double alpha, double &s_bcl, double &s_bcu) {
   double a1, a2;

   if(sr_na.n() == 0) {
      s_bcl = bad_data_double;
      s_bcu = bad_data_double;

      return;
   }

   //
   // Choose a1 and a2 as the percentiles to be selected from the
   // array of replicates of the statistic
   //
   a1 = alpha/2.0;
   a2 = 1.0 - alpha/2.0;

   //
   // Compute the lower and upper bootstrap confidence limits by
   // sorting the array of replicates and taking the a1-th and the
   // a2-th percentile of the data.
   //
   s_bcl = sr_na.percentile_array(a1);
   s_bcu = sr_na.percentile_array(a2);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_cntinfo(ofstream &tmp_out, const CNTInfo &c) {
   char line[max_line_len];

   snprintf(line, max_line_len,
           "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
           c.fbar.v,    c.fstdev.v,    c.obar.v,          c.ostdev.v,
           c.pr_corr.v, c.anom_corr.v,
           c.rmsfa.v,   c.rmsoa.v,     c.anom_corr_uncntr.v,
           c.me.v,      c.me2.v,       c.estdev.v,        c.mbias.v,
           c.mae.v,     c.mse.v,       c.msess.v,         c.bcmse.v,
           c.rmse.v,    c.e10.v,       c.e25.v,           c.e50.v,
           c.e75.v,     c.e90.v,       c.eiqr.v,          c.mad.v);

   tmp_out << line << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ctsinfo(ofstream &tmp_out, const CTSInfo &c) {
   char line[max_line_len];

   snprintf(line, max_line_len,
           "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
           c.baser.v,   c.fmean.v,   c.acc.v,     c.fbias.v,
           c.pody.v,    c.podn.v,    c.pofd.v,    c.far.v,
           c.csi.v,     c.gss.v,     c.hk.v,      c.hss.v,
           c.odds.v,    c.lodds.v,   c.orss.v,    c.eds.v,
           c.seds.v,    c.edi.v,     c.sedi.v,    c.bagss.v
          );

   tmp_out << line << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mctsinfo(ofstream &tmp_out, const MCTSInfo &c) {
   char line[max_line_len];

   snprintf(line, max_line_len, "%f %f %f %f",
           c.acc.v, c.hk.v, c.hss.v, c.ger.v);

   tmp_out << line << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nbrcntinfo(ofstream &tmp_out, const NBRCNTInfo &c) {
   char line[max_line_len];

   snprintf(line, max_line_len,
           "%f %f %f %f %f %f",
           c.fbs.v, c.fss.v, c.afss.v, c.ufss.v, c.f_rate.v, c.o_rate.v);

   tmp_out << line << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void read_ldf(const ConcatString file_name, int col, NumArray &na) {
   LineDataFile ldf_in;
   DataLine line;
   double v;

   //
   // Open up the input file for reading
   //
   if(!ldf_in.open(file_name.c_str())) {
      mlog << Error << "\nread_ldf() -> "
           << "can't open file: " << file_name << "\n\n"
          ;
      throw(1);
   }

   //
   // Clear out the NumArray
   //
   na.clear();

   //
   // Read the data in the column specified
   //
   while(ldf_in >> line) {
      v = atof(line.get_item(col-1));
      if(!is_bad_data(v)) na.add(v);
   }

   //
   // Close the input file
   //
   ldf_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////
