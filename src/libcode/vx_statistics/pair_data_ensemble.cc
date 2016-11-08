// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
#include <set>
#include <limits>

#include "pair_data_ensemble.h"

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

   if(this == &pd) return(*this);

   assign(pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::init_from_scratch() {

   e_na       = (NumArray *) 0;
   n_ens      = 0;
   ssvar_bins = (SSVARInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::clear() {
   int i;

   PairBase::clear();

   for(i=0; i<n_ens; i++) e_na[i].clear();
   if(e_na) { delete [] e_na; e_na = (NumArray *) 0; }
   n_ens = 0;

   v_na.clear();
   r_na.clear();
   crps_na.clear();
   ign_na.clear();
   pit_na.clear();
   rhist_na.clear();
   phist_na.clear();
   var_na.clear();
   mn_na.clear();

   if(ssvar_bins) { delete [] ssvar_bins; ssvar_bins = (SSVARInfo *) 0; }

   ssvar_bin_size = bad_data_double;
   phist_bin_size = bad_data_double;
   crpss          = bad_data_double;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::assign(const PairDataEnsemble &pd) {
   int i;

   clear();

   set_mask_name(pd.mask_name);
   set_mask_dp_ptr(pd.mask_dp_ptr);
   set_msg_typ(pd.msg_typ);

   set_interp_mthd(pd.interp_mthd);
   set_interp_dpth(pd.interp_dpth);

   sid_sa   = pd.sid_sa;
   lat_na   = pd.lat_na;
   lon_na   = pd.lon_na;
   x_na     = pd.x_na;
   y_na     = pd.y_na;
   vld_ta   = pd.vld_ta;
   lvl_na   = pd.lvl_na;
   elv_na   = pd.elv_na;
   o_na     = pd.o_na;
   v_na     = pd.v_na;
   r_na     = pd.r_na;
   crps_na  = pd.crps_na;
   ign_na   = pd.ign_na;
   pit_na   = pd.pit_na;
   rhist_na = pd.rhist_na;
   phist_na = pd.phist_na;
   var_na   = pd.var_na;
   mn_na    = pd.mn_na;

   n_obs    = pd.n_obs;

   if(pd.ssvar_bins){
      ssvar_bins = new SSVARInfo[pd.ssvar_bins[0].n_bin];
      for(i=0; i < pd.ssvar_bins[0].n_bin; i++){
         ssvar_bins[i] = pd.ssvar_bins[i];
      }
   } else ssvar_bins = 0;

   ssvar_bin_size = pd.ssvar_bin_size;
   phist_bin_size = pd.phist_bin_size;
   crpss          = pd.crpss;

   set_ens_size(pd.n_ens);

   for(i=0; i<n_ens; i++) e_na[i] = pd.e_na[i];

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::add_ens(int member, double v) {

   e_na[member].add(v);

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

void PairDataEnsemble::compute_rank(const gsl_rng *rng_ptr) {
   int i, j, k, n_vld, n_bel, n_tie;
   NumArray src_na, dest_na;

   // Compute the rank for each observation
   for(i=0; i<o_na.n_elements(); i++) {

      // Compute the number of ensemble values less than the observation
      for(j=0, n_vld=0, n_bel=0, n_tie=0; j<n_ens; j++) {

         // Skip bad data
         if(!is_bad_data(e_na[j][i])) {

            // Increment the valid count
            n_vld++;

            // Keep track of the number of ties and the number below
            if(is_eq(e_na[j][i], o_na[i])) n_tie++;
            else if(e_na[j][i] < o_na[i])  n_bel++;
         }
      } // end for j

      // Store the number of valid ensemble values
      v_na.add(n_vld);

      // Compute rank only when all ensemble members are valid
      if(n_vld == n_ens) {

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

      }
      // Can't compute the rank when there is data missing
      else {
         r_na.add(bad_data_int);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_rhist() {
   int i, rank;

   // Clear the ranked histogram
   rhist_na.clear();

   // Initialize the histogram counts to 0
   for(i=0; i<=n_ens; i++) rhist_na.add(0);

   // The compute_rank() routine should have already been called.
   // Loop through the ranks and populate the histogram.
   for(i=0; i<r_na.n_elements(); i++) {

      // Get the current rank
      rank = nint(r_na[i]);

      // Increment the histogram counts
      if(!is_bad_data(rank)) rhist_na.set(rank-1, rhist_na[rank-1]+1);

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

   // The compute_stats() routine should have already been called.
   // Loop through the PIT values and populate the histogram.
   for(i=0; i<pit_na.n_elements(); i++) {

      if(is_bad_data(pit_na[i])) continue;

      if(pit_na[i] < 0.0 || pit_na[i] > 1.0) {
         mlog << Warning << "PairDataEnsemble::compute_phist() -> "
              << "probability integral transform value ("
              << pit_na[i] << ") is outside of valid range [0, 1].\n\n";
         continue;
      }

      // Determine the bin
      bin = (is_eq(pit_na[i], 1.0) ?
             phist_na.n_elements() - 1 :
             floor(pit_na[i]/phist_bin_size));

      // Increment the histogram counts
      phist_na.set(bin, phist_na[bin]+1);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_stats() {
   int i, j;
   double crps, ign, pit, w, w_sum;
   double crps_climo, ccbar, oobar, cobar;
   NumArray cur;

   // Initialize
   crps_na.clear();
   ign_na.clear();
   pit_na.clear();

   // Loop through the pairs and compute CRPS for each
   for(i=0; i<n_obs; i++) {

      // Don't compute if any of the ensemble members are missing
      if(nint(v_na[i]) != n_ens) {
         crps_na.add(bad_data_double);
         ign_na.add(bad_data_double);
         pit_na.add(bad_data_double);
         continue;
      }

      // Store ensemble values for the current observation
      for(j=0, cur.clear(); j<n_ens; j++) cur.add(e_na[j][i]);

      // Compute the stats
      compute_crps_ign_pit(o_na[i], cur, crps, ign, pit);

      // Store the stats and weight for the current point
      crps_na.add(crps);
      ign_na.add(ign);
      pit_na.add(pit);

   } // end for i

   // Compute CRPS Skill Score

   // Get the average ensemble CRPS value
   crps = crps_na.wmean(wgt_na);

   // Check for bad data
   if(is_bad_data(crps) ||
      cmn_na.n_elements() != o_na.n_elements() ||
      cmn_na.n_elements() == 0 ||
      cmn_na.has(bad_data_double)) {
      crpss = bad_data_double;
   }
   else {

      // Get the sum of the weights
      w_sum = wgt_na.sum();

      // Compute the climatological CRPS
      ccbar = oobar = cobar = 0.0;
      for(i=0; i<n_obs; i++) {
         w      = wgt_na[i]/w_sum;
         ccbar += w * cmn_na[i] * cmn_na[i];
         oobar += w * o_na[i]   * o_na[i];
         cobar += w * cmn_na[i] * o_na[i];
      }
      crps_climo = ccbar + oobar - 2.0*cobar;

      // Compute skill score
      crpss = (is_eq(crps_climo, 0.0) ?
               bad_data_double : (crps_climo - crps)/crps_climo);
   }

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
   int i, j, n_vld;
   double var, dev;
   ssvar_bin_map bins;

   // Compute the variance of ensemble member values at each point
   for(i=0; i<o_na.n_elements(); i++) {

      // Add the deviation of each ensemble member
      for(j=0, n_vld=0, var=0; j<n_ens; j++) {

         // Skip bad data
         if(is_bad_data(e_na[j][i]) || is_bad_data(mn_na[i])) continue;
         n_vld++;

         // Add the squared deviation
         dev = (e_na[j][i] - mn_na[i]);
         var += dev*dev;

      } // end for j

      if( !n_vld ) continue;

      //  Calculate the variance
      var = (1.0 / ((double)n_vld - 1.0)) * var;

      // Build a variance point
      ens_ssvar_pt pt;
      pt.var = var;
      pt.f   = mn_na[i];
      pt.o   = o_na[i];
      pt.w   = wgt_na[i];

      // Determine the bin for the current point and add it to the list
      // Bins are defined starting at 0 and are left-closed, right-open
      j=0;
      while(var > (j+1)*ssvar_bin_size || is_eq(var, (j+1)*ssvar_bin_size)) j++;
      string ssvar_min = str_format("%.5e", j*ssvar_bin_size).contents();
      if( !bins.count(ssvar_min) ){
         ssvar_pt_list pts;
         pts.push_back(pt);
         bins[ssvar_min] = pts;
      } else {
         bins[ssvar_min].push_back(pt);
      }

   } // end for i

   // Sort the bins
   set<string,ssvar_bin_comp> sorted_bins;
   for( ssvar_bin_map::iterator map_it = bins.begin();
        map_it != bins.end(); map_it++ ){
      sorted_bins.insert( (*map_it).first );
   }

   // Report the number of bins built
   int n_bin = sorted_bins.size();
   mlog << Debug(4) << "PairDataEnsemble::compute_ssvar() - "
        << "Built " << n_bin << " variance spread/skill bins from "
        << o_na.n_elements() << " observations\n";

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

   if(this == &vx_pd) return(*this);

   assign(vx_pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::init_from_scratch() {

   fcst_info    = (VarInfo *) 0;
   climo_info   = (VarInfo *) 0;
   obs_info     = (VarInfo *) 0;
   pd           = (PairDataEnsemble ***) 0;

   n_msg_typ    = 0;
   n_mask       = 0;
   n_interp     = 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::clear() {
   int i, j, k;

   if(fcst_info)  { delete fcst_info;  fcst_info  = (VarInfo *) 0; }
   if(climo_info) { delete climo_info; climo_info = (VarInfo *) 0; }
   if(obs_info)   { delete obs_info;   obs_info   = (VarInfo *) 0; }

   desc.clear();

   interp_thresh = 0;

   fcst_dpa.clear();
   climo_mn_dpa.clear();
   climo_sd_dpa.clear();

   sid_exc_filt.clear();
   obs_qty_filt.clear();

   fcst_ut = (unixtime) 0;
   beg_ut  = (unixtime) 0;
   end_ut  = (unixtime) 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
         }
      }
   }

   n_msg_typ     = 0;
   n_mask        = 0;
   n_interp      = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::assign(const VxPairDataEnsemble &vx_pd) {
   int i, j, k;

   clear();

   set_fcst_info(vx_pd.fcst_info);
   set_climo_info(vx_pd.climo_info);
   set_obs_info(vx_pd.obs_info);

   desc         = vx_pd.desc;

   fcst_ut      = vx_pd.fcst_ut;
   beg_ut       = vx_pd.beg_ut;
   end_ut       = vx_pd.end_ut;
   sid_exc_filt = vx_pd.sid_exc_filt;
   obs_qty_filt = vx_pd.obs_qty_filt;

   interp_thresh = vx_pd.interp_thresh;

   fcst_dpa     = vx_pd.fcst_dpa;
   climo_mn_dpa = vx_pd.climo_mn_dpa;
   climo_sd_dpa = vx_pd.climo_sd_dpa;

   set_pd_size(vx_pd.n_msg_typ, vx_pd.n_mask, vx_pd.n_interp);

   for(i=0; i<vx_pd.n_msg_typ; i++) {
      for(j=0; j<vx_pd.n_mask; j++) {
         for(k=0; k<vx_pd.n_interp; k++) {
            pd[i][j][k] = vx_pd.pd[i][j][k];
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_fcst_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) 0; }

   // Perform a deep copy
   fcst_info = f.new_var_info(info->file_type());
   *fcst_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_climo_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(climo_info) { delete climo_info; climo_info = (VarInfo *) 0; }

   // Perform a deep copy
   climo_info = f.new_var_info(info->file_type());
   *climo_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_obs_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(obs_info) { delete obs_info; obs_info = (VarInfo *) 0; }

   // Perform a deep copy
   obs_info = f.new_var_info(info->file_type());
   *obs_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_desc(const char *s) {

   desc = s;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_interp_thresh(double t) {

   interp_thresh = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_fcst_dpa(const DataPlaneArray &dpa) {

   fcst_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_climo_mn_dpa(const DataPlaneArray &dpa) {

   climo_mn_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_climo_sd_dpa(const DataPlaneArray &dpa) {

   climo_sd_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_fcst_ut(const unixtime ut) {

   fcst_ut = ut;

   //  set the fcst_ut for all PairBase instances, used for duplicate logic
   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_fcst_ut(ut);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_beg_ut(const unixtime ut) {

   beg_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_end_ut(const unixtime ut) {

   end_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_sid_exc_filt(const StringArray se) {

   sid_exc_filt = se;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_obs_qty_filt(const StringArray q) {

   obs_qty_filt = q;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_pd_size(int types, int masks, int interps) {
   int i, j;

   // Store the dimensions for the PairData array
   n_msg_typ = types;
   n_mask    = masks;
   n_interp  = interps;

   // Allocate space for the PairData array
   pd = new PairDataEnsemble ** [n_msg_typ];

   for(i=0; i<n_msg_typ; i++) {
      pd[i] = new PairDataEnsemble * [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j] = new PairDataEnsemble [n_interp];
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_msg_typ(int i_msg_typ, const char *name) {
   int i, j;

   for(i=0; i<n_mask; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i_msg_typ][i][j].set_msg_typ(name);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_mask_dp(int i_mask, const char *name,
                                     DataPlane *dp_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_dp_ptr(dp_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_mask_sid(int i_mask, const char *name,
                                      StringArray *sid_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_sid_ptr(sid_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_interp(int i_interp,
                                    const char *interp_mthd_str,
                                    int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(interp_mthd_str);
         pd[i][j][i_interp].set_interp_dpth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_interp(int i_interp, InterpMthd mthd,
                                    int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(mthd);
         pd[i][j][i_interp].set_interp_dpth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_ens_size(int n) {
   int i, j, k;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].set_ens_size(n);
         }
      }
   }

   return;
}


////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_ssvar_bin_size(double ssvar_bin_size) {
   int i, j, k;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].ssvar_bin_size = ssvar_bin_size;
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_phist_bin_size(double phist_bin_size) {
   int i, j, k;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].phist_bin_size = phist_bin_size;
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::add_obs(float *hdr_arr, const char *hdr_typ_str,
                                 const char *hdr_sid_str, unixtime hdr_ut,
                                 const char *obs_qty, float *obs_arr,
                                 Grid &gr, const DataPlane * wgt_dp) {
   int i, j, k, x, y;
   double hdr_lat, hdr_lon;
   double obs_x, obs_y, obs_lvl, obs_hgt, to_lvl;
   double cmn_v, csd_v, obs_v, wgt_v;
   int cmn_lvl_blw, cmn_lvl_abv;
   int csd_lvl_blw, csd_lvl_abv;

   // Check the observation VarInfo file type
   if(obs_info->file_type() != FileType_Gb1) {
      mlog << Error << "\nVxPairDataEnsemble::add_obs() -> "
           << "when processing point observations, the observation "
           << "VarInfo type must be GRIB.\n\n";
      exit(1);
   }

   // Create VarInfoGrib pointer
   VarInfoGrib *obs_info_grib = (VarInfoGrib *) obs_info;

   // Check the station ID exclusion list
   if(sid_exc_filt.n_elements() && sid_exc_filt.has(hdr_sid_str)) return;

   // Check whether the GRIB code for the observation matches
   // the specified code
   if(obs_info_grib->code() != nint(obs_arr[1])) return;

   // Check if the observation quality flag is included in the list
   if(obs_qty_filt.n_elements() && strcmp(obs_qty, "")) {
      bool qty_match = false;
      for(i=0; i < obs_qty_filt.n_elements() && !qty_match; i++)
         if( 0 == strcmp(obs_qty, obs_qty_filt[i]) ) qty_match = true;

      if(!qty_match) return;
   }

   // Check whether the observation time falls within the valid time
   // window
   if(hdr_ut < beg_ut || hdr_ut > end_ut) return;

   hdr_lat = hdr_arr[0];
   hdr_lon = hdr_arr[1];

   obs_lvl = obs_arr[2];
   obs_hgt = obs_arr[3];
   obs_v   = obs_arr[4];

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) return;

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   x = nint(obs_x);
   y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(x < 0 || x >= gr.nx() ||
      y < 0 || y >= gr.ny()) return;

   // For pressure levels, check if the observation pressure level
   // falls in the requsted range.
   if(obs_info_grib->level().type() == LevelType_Pres) {

      if(obs_lvl < obs_info_grib->level().lower() ||
         obs_lvl > obs_info_grib->level().upper()) return;
   }
   // For accumulations, check if the observation accumulation interval
   // matches the requested interval.
   else if(obs_info_grib->level().type() == LevelType_Accum) {

      if(obs_lvl < obs_info_grib->level().lower() ||
         obs_lvl > obs_info_grib->level().upper()) return;
   }
   // For vertical levels, check for a surface message type or if the
   // observation height falls within the requested range.
   else if(obs_info_grib->level().type() == LevelType_Vert) {

      if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL &&
         (obs_hgt < obs_info_grib->level().lower() ||
          obs_hgt > obs_info_grib->level().upper())) return;
   }
   // For all other level types (RecNumber, NoLevel), check
   // if the observation height falls within the requested range.
   else {
      if(obs_hgt < obs_info_grib->level().lower() ||
         obs_hgt > obs_info_grib->level().upper()) return;
   }

   // For a single climatology mean field
   if(climo_mn_dpa.n_planes() == 1) {
      cmn_lvl_blw = 0;
      cmn_lvl_abv = 0;
   }
   // For multiple climatology mean fields, find the levels above and
   // below the observation point.
   else {
      // Interpolate using the observation pressure level or height
      to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                obs_lvl : obs_hgt);
      find_vert_lvl(climo_mn_dpa, to_lvl, cmn_lvl_blw, cmn_lvl_abv);
   }

   // For a single climatology standard deviation field
   if(climo_sd_dpa.n_planes() == 1) {
      csd_lvl_blw = 0;
      csd_lvl_abv = 0;
   }
   // For multiple climatology standard deviation fields, find the
   // levels above and below the observation point.
   else {
      // Interpolate using the observation pressure level or height
      to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                obs_lvl : obs_hgt);
      find_vert_lvl(climo_sd_dpa, to_lvl, csd_lvl_blw, csd_lvl_abv);
   }

   // When verifying a vertical level forecast against a surface message type,
   // set the observation level value to bad data so that it's not used in the
   // duplicate logic.
   if(obs_info->level().type() == LevelType_Vert &&
      strstr(onlysf_msg_typ_str, hdr_typ_str) != NULL) obs_lvl = bad_data_double;

   // Look through all of the PairData objects to see if the observation
   // should be added.

   // Check the message types
   for(i=0; i<n_msg_typ; i++) {

      //
      // Check for a matching PrepBufr message type
      //

      // Handle ANYAIR
      if(strcmp(anyair_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anyair_msg_typ_str, hdr_typ_str) == NULL ) continue;
      }

      // Handle ANYSFC
      else if(strcmp(anysfc_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anysfc_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle ONLYSF
      else if(strcmp(onlysf_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle all other message types
      else {
         if(strcmp(hdr_typ_str, pd[i][0][0].msg_typ) != 0) continue;
      }

      // Check the masking areas and points
      for(j=0; j<n_mask; j++) {

         // Check for the obs falling within the masking region
         if(pd[i][j][0].mask_dp_ptr != (DataPlane *) 0) {
            if(!pd[i][j][0].mask_dp_ptr->s_is_on(x, y)) continue;
         }
         // Otherwise, check for the obs Station ID's presence in the
         // masking SID list
         else if(pd[i][j][0].mask_sid_ptr != (StringArray *) 0) {
            if(!pd[i][j][0].mask_sid_ptr->has(hdr_sid_str)) continue;
         }

         // Add the observation for each interpolation method
         for(k=0; k<n_interp; k++) {

            // Compute the interpolated climatology values using the
            // observation pressure level or height
            to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                      obs_lvl : obs_hgt);

            // Compute the interpolated climatology mean
            cmn_v = compute_interp(climo_mn_dpa, obs_x, obs_y, obs_v,
                       k, to_lvl, cmn_lvl_blw, cmn_lvl_abv);

            // Check for valid interpolation options
            if(climo_sd_dpa.n_planes() > 0 &&
               (pd[0][0][k].interp_mthd == InterpMthd_Min    ||
                pd[0][0][k].interp_mthd == InterpMthd_Max    ||
                pd[0][0][k].interp_mthd == InterpMthd_Median ||
                pd[0][0][k].interp_mthd == InterpMthd_Best)) {
               mlog << Warning << "\nVxPairDataEnsemble::add_obs() -> "
                    << "applying the "
                    << interpmthd_to_string(pd[0][0][k].interp_mthd)
                    << " interpolation method to climatological spread "
                    << "may cause unexpected results.\n\n";
            }

            // Compute the interpolated climatology standard deviation
            csd_v = compute_interp(climo_sd_dpa, obs_x, obs_y, obs_v,
                      k, to_lvl, csd_lvl_blw, csd_lvl_abv);

            // Compute weight for current point
            wgt_v = ( wgt_dp == (DataPlane *) 0 ?
                      default_grid_weight : wgt_dp->get(x, y) );

            // Add the observation value
            // Weight is from the nearest grid point
            pd[i][j][k].add_obs(hdr_sid_str, hdr_lat, hdr_lon,
                                obs_x, obs_y, hdr_ut,
                                obs_lvl, obs_hgt, obs_v, obs_qty,
                                cmn_v, csd_v, wgt_v);
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::add_ens(int member, bool mn) {
   int i, j, k, l;
   int f_lvl_blw, f_lvl_abv;
   double to_lvl, fcst_v;

   // Loop through all the PairDataEnsemble objects and interpolate
   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {

            // Process each of the observations
            for(l=0; l<pd[i][j][k].n_obs; l++) {

               // Interpolate using the observation pressure level or height
               to_lvl = (fcst_info->level().type() == LevelType_Pres ?
                         pd[i][j][k].lvl_na[l] : pd[i][j][k].elv_na[l]);

               // For a single forecast field
               if(fcst_dpa.n_planes() == 1) {
                  f_lvl_blw = 0;
                  f_lvl_abv = 0;
               }
               // For multiple forecast fields, find the levels above
               // and below the observation point.
               else {
                  find_vert_lvl(fcst_dpa, to_lvl, f_lvl_blw, f_lvl_abv);
               }

               // Compute the interpolated ensemble value
               fcst_v = compute_interp(fcst_dpa, pd[i][j][k].x_na[l],
                                       pd[i][j][k].y_na[l],
                                       pd[i][j][k].o_na[l], k,
                                       to_lvl, f_lvl_blw, f_lvl_abv);

               // Add the ensemble value, even if it's bad data
               if(!mn) pd[i][j][k].add_ens(member, fcst_v);
               else    pd[i][j][k].mn_na.add(fcst_v);
            }
         } // end for k - n_interp
      } // end for j - n_mask
   } // end for i - n_msg_typ

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::find_vert_lvl(const DataPlaneArray &dpa,
                                       double obs_lvl,
                                       int &i_blw, int &i_abv) {
   int i;
   double dist, dist_blw, dist_abv;

   if(dpa.n_planes() == 0) {
      i_blw = i_abv = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
   dist_blw = dist_abv = 1.0e30;
   for(i=0; i<dpa.n_planes(); i++) {

      dist = obs_lvl - dpa.lower(i);

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && fabs(dist) < dist_blw) {
         dist_blw = fabs(dist);
         i_blw = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && fabs(dist) < dist_abv) {
         dist_abv = fabs(dist);
         i_abv = i;
      }
   }

   // Check if the observation is above the forecast range
   if(is_eq(dist_blw, 1.0e30) && !is_eq(dist_abv, 1.0e30)) {

      // Set the index below to the index above and perform
      // no vertical interpolation
      i_blw = i_abv;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_blw, 1.0e30) && is_eq(dist_abv, 1.0e30)) {

      // Set the index above to the index below and perform
      // no vertical interpolation
      i_abv = i_blw;
   }
   // Check if an error occurred
   else if(is_eq(dist_blw, 1.0e30) && is_eq(dist_abv, 1.0e30)) {
      mlog << Error << "\nVxPairDataEnsemble::find_vert_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation level of " << obs_lvl << ".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int VxPairDataEnsemble::get_n_pair() {
   int n, i, j, k;

   for(i=0, n=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            n += pd[i][j][k].n_obs;
         }
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_duplicate_flag(DuplicateType duplicate_flag) {

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].set_check_unique(duplicate_flag == DuplicateType_Unique);
            pd[i][j][k].set_check_single(duplicate_flag == DuplicateType_Single);
         }
      }
   }

}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::print_duplicate_report() {

   for(int i=0; i < n_msg_typ; i++){
      for(int j=0; j < n_mask; j++){
         for(int k=0; k < n_interp; k++){
            pd[i][j][k].print_duplicate_report();
         }
      }
   }

}

////////////////////////////////////////////////////////////////////////

double VxPairDataEnsemble::compute_interp(const DataPlaneArray &dpa,
                                          double obs_x, double obs_y,
                                          double obs_v, int i_interp,
                                          double to_lvl,
                                          int i_blw, int i_abv) {
   double v, v_blw, v_abv, t;

   // Check for no data
   if(dpa.n_planes() == 0) return(bad_data_double);

   v_blw = compute_horz_interp(dpa[i_blw], obs_x, obs_y, obs_v,
                               pd[0][0][i_interp].interp_mthd,
                               pd[0][0][i_interp].interp_dpth,
                               interp_thresh);

   if(i_blw == i_abv) {
      v = v_blw;
   }
   else {
      v_abv = compute_horz_interp(dpa[i_abv], obs_x, obs_y, obs_v,
                                  pd[0][0][i_interp].interp_mthd,
                                  pd[0][0][i_interp].interp_dpth,
                                  interp_thresh);

      // Check for bad data prior to vertical interpolation
      if(is_bad_data(v_blw) || is_bad_data(v_abv)) {
         return(bad_data_double);
      }

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_info->is_specific_humidity() &&
          obs_info->is_specific_humidity()) {
         t = compute_vert_pinterp(log(v_blw), dpa.lower(i_blw),
                                  log(v_abv), dpa.lower(i_abv),
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(fcst_info->level().type() == LevelType_Pres) {
         v = compute_vert_pinterp(v_blw, dpa.lower(i_blw),
                                  v_abv, dpa.lower(i_abv),
                                  to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(v_blw, dpa.lower(i_blw),
                                  v_abv, dpa.lower(i_abv),
                                  to_lvl);
      }
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Utility function for computing the continuous ranked probability
// score (CRPS), the ignorance score (IGN), and the probability
// itegral transform (PIT)
//
////////////////////////////////////////////////////////////////////////

void compute_crps_ign_pit(double obs, const NumArray &ens_na,
                          double &crps, double &ign, double &pit) {
   double m, s, z;

   // Mean and standard deviation of the ensemble values
   ens_na.compute_mean_stdev(m, s);

   // Check for divide by zero
   if(is_bad_data(m) || is_bad_data(s) || is_eq(s, 0.0)) {
      crps = bad_data_double;
      ign  = bad_data_double;
      pit  = bad_data_double;
   }
   else {

      z = (obs - m)/s;

      // Compute CRPS
      crps = s*(z*(2.0*znorm(z) - 1) + 2.0*dnorm(z) - 1.0/sqrt(pi));

      // Compute IGN
      ign = 0.5*log(2.0*pi*s*s) + (obs - m)*(obs - m)/(2.0*s*s);

      // Compute PIT
      pit = normal_cdf(obs, m, s);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
