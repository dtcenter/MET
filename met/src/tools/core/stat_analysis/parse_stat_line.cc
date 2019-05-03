// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   parse_stat_line.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add parse_rhist_line and
//                    parse_orank_line.
//   002    06/09/10  Halley Gotway   Add parse_mctc_ctable.
//   003    03/07/13  Halley Gotway   Add parse_ssvar_line.
//   004    05/19/14  Halley Gotway   Add OBS_QC to MPR and ORANK lines.
//   005    05/20/14  Halley Gotway   Add AFSS, UFSS, F_RATE, and O_RATE
//                                      to the NBRCNT line.
//   006    06/03/14  Halley Gotway   Add PHIST line type.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>

#include "vx_log.h"

#include "parse_stat_line.h"

////////////////////////////////////////////////////////////////////////

extern void parse_rhist_line_pre5_1 (STATLine &l, RHISTData &r_data);
extern void parse_rhist_line_current(STATLine &l, RHISTData &r_data);

////////////////////////////////////////////////////////////////////////

void parse_fho_ctable(STATLine &l, TTContingencyTable &ct) {
   int n, fy, fy_oy, oy;
   double f_rate, h_rate, o_rate;

   n      = atoi(l.get_item(fho_total_offset));
   f_rate = atof(l.get_item(fho_f_rate_offset));
   h_rate = atof(l.get_item(fho_h_rate_offset));
   o_rate = atof(l.get_item(fho_o_rate_offset));

   fy    = nint(n * f_rate);
   fy_oy = nint(n * h_rate);
   oy    = nint(n * o_rate);

   // FY_OY
   ct.set_fy_oy(fy_oy);

   // FY_ON
   ct.set_fy_on(fy - fy_oy);

   // FN_OY
   ct.set_fn_oy(oy - fy_oy);

   // FN_ON
   ct.set_fn_on(n - fy - oy + fy_oy);

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_ctc_ctable(STATLine &l, TTContingencyTable &ct) {

   // FY_OY
   ct.set_fy_oy(atoi(l.get_item(ctc_fy_oy_offset)));

   // FY_ON
   ct.set_fy_on(atoi(l.get_item(ctc_fy_on_offset)));

   // FN_OY
   ct.set_fn_oy(atoi(l.get_item(ctc_fn_oy_offset)));

   // FN_ON
   ct.set_fn_on(atoi(l.get_item(ctc_fn_on_offset)));

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_mctc_ctable(STATLine &l, ContingencyTable &ct) {
   int n_cat, i, j, offset;

   // N_CAT
   n_cat = atoi(l.get_item(mctc_n_cat_offset));
   ct.set_size(n_cat);

   // Fi_Oj
   for(i=0; i<n_cat; i++) {
      for(j=0; j<n_cat; j++) {
         offset = mctc_fi_oj_offset(i, j, n_cat);
         ct.set_entry(i, j, atoi(l.get_item(offset)));
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_nbrctc_ctable(STATLine &l, TTContingencyTable &ct) {

   // FY_OY
   ct.set_fy_oy(atoi(l.get_item(nbrctc_fy_oy_offset)));

   // FY_ON
   ct.set_fy_on(atoi(l.get_item(nbrctc_fy_on_offset)));

   // FN_OY
   ct.set_fn_oy(atoi(l.get_item(nbrctc_fn_oy_offset)));

   // FN_ON
   ct.set_fn_on(atoi(l.get_item(nbrctc_fn_on_offset)));

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_nx2_ctable(STATLine &l, Nx2ContingencyTable &pct) {
   int i, n, oy, on;
   double *thresh = (double *) 0;

   // N_THRESH
   n = atoi(l.get_item(pct_n_thresh_offset));
   pct.set_size(n-1);

   // Allocate space for list of thresholds
   thresh = new double [n];

   for(i=0; i<n-1; i++) {

      // THRESH_i
      thresh[i] = atof(l.get_item(pct_thresh_offset(i)));

      // OY_i
      oy = atoi(l.get_item(pct_oy_offset(i)));
      pct.set_entry(i, nx2_event_column, oy);

      // ON_i
      on = atoi(l.get_item(pct_on_offset(i)));
      pct.set_entry(i, nx2_nonevent_column, on);
   }

   // THRESH_n
   thresh[n-1] = atof(l.get_item(pct_thresh_offset(n-1)));
   pct.set_thresholds(thresh);

   if(thresh) { delete thresh; thresh = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_sl1l2_line(STATLine &l, SL1L2Info &s_info) {

   s_info.clear();

   s_info.scount = atoi(l.get_item(sl1l2_total_offset));
   s_info.fbar   = atof(l.get_item(sl1l2_fbar_offset));
   s_info.obar   = atof(l.get_item(sl1l2_obar_offset));
   s_info.fobar  = atof(l.get_item(sl1l2_fobar_offset));
   s_info.ffbar  = atof(l.get_item(sl1l2_ffbar_offset));
   s_info.oobar  = atof(l.get_item(sl1l2_oobar_offset));

   // Parse MAE, if present
   s_info.mae = (l.n_items() > sl1l2_mae_offset ?
                 atof(l.get_item(sl1l2_mae_offset)) :
                 bad_data_double);

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_sal1l2_line(STATLine &l, SL1L2Info &s_info) {

   s_info.clear();

   s_info.sacount = atoi(l.get_item(sal1l2_total_offset));
   s_info.fabar   = atof(l.get_item(sal1l2_fabar_offset));
   s_info.oabar   = atof(l.get_item(sal1l2_oabar_offset));
   s_info.foabar  = atof(l.get_item(sal1l2_foabar_offset));
   s_info.ffabar  = atof(l.get_item(sal1l2_ffabar_offset));
   s_info.ooabar  = atof(l.get_item(sal1l2_ooabar_offset));

   // Parse MAE, if present
   s_info.mae = (l.n_items() > sal1l2_mae_offset ?
                 atof(l.get_item(sal1l2_mae_offset)) :
                 bad_data_double);

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_vl1l2_line(STATLine &l, VL1L2Info &v_info) {

   v_info.clear();

   v_info.vcount  = atoi(l.get_item(vl1l2_total_offset));
   v_info.ufbar   = atof(l.get_item(vl1l2_ufbar_offset));
   v_info.vfbar   = atof(l.get_item(vl1l2_vfbar_offset));
   v_info.uobar   = atof(l.get_item(vl1l2_uobar_offset));
   v_info.vobar   = atof(l.get_item(vl1l2_vobar_offset));
   v_info.uvfobar = atof(l.get_item(vl1l2_uvfobar_offset));
   v_info.uvffbar = atof(l.get_item(vl1l2_uvffbar_offset));
   v_info.uvoobar = atof(l.get_item(vl1l2_uvoobar_offset));

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_val1l2_line(STATLine &l, VL1L2Info &v_info) {

   v_info.clear();

   v_info.vacount  = atoi(l.get_item(val1l2_total_offset));
   v_info.ufabar   = atof(l.get_item(val1l2_ufabar_offset));
   v_info.vfabar   = atof(l.get_item(val1l2_vfabar_offset));
   v_info.uoabar   = atof(l.get_item(val1l2_uoabar_offset));
   v_info.voabar   = atof(l.get_item(val1l2_voabar_offset));
   v_info.uvfoabar = atof(l.get_item(val1l2_uvfoabar_offset));
   v_info.uvffabar = atof(l.get_item(val1l2_uvffabar_offset));
   v_info.uvooabar = atof(l.get_item(val1l2_uvooabar_offset));

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_nbrcnt_line(STATLine &l, NBRCNTInfo &n_info) {

   n_info.clear();

   n_info.nbr_wdth   = atoi(l.get_item(interp_pnts_offset));
   n_info.fthresh.set(l.get_item(fcst_thresh_offset));
   n_info.othresh.set(l.get_item(obs_thresh_offset));
   n_info.sl1l2_info.scount = atoi(l.get_item(nbrcnt_total_offset));
   n_info.fbs.v      = atof(l.get_item(nbrcnt_fbs_offset));
   n_info.fss.v      = atof(l.get_item(nbrcnt_fss_offset));

   // Parse AFSS, UFSS, F_RATE, and O_RATE, if present
   n_info.afss.v     = (l.n_items() > nbrcnt_afss_offset ?
                        atof(l.get_item(nbrcnt_afss_offset)) :
                        bad_data_double);
   n_info.ufss.v     = (l.n_items() > nbrcnt_ufss_offset ?
                        atof(l.get_item(nbrcnt_ufss_offset)) :
                        bad_data_double);
   n_info.f_rate.v   = (l.n_items() > nbrcnt_f_rate_offset ?
                        atof(l.get_item(nbrcnt_f_rate_offset)) :
                        bad_data_double);
   n_info.o_rate.v   = (l.n_items() > nbrcnt_o_rate_offset ?
                        atof(l.get_item(nbrcnt_o_rate_offset)) :
                        bad_data_double);

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_mpr_line(STATLine &l, MPRData &m_data) {

   m_data.fcst_var = l.get_item(fcst_var_offset);
   m_data.obs_var  = l.get_item(obs_var_offset);
   m_data.total    = atoi(l.get_item(mpr_total_offset));
   m_data.index    = atoi(l.get_item(mpr_index_offset));
   m_data.obs_sid  = l.get_item(mpr_obs_sid_offset);
   m_data.obs_lat  = atof(l.get_item(mpr_obs_lat_offset));
   m_data.obs_lon  = atof(l.get_item(mpr_obs_lon_offset));
   m_data.obs_lvl  = atof(l.get_item(mpr_obs_lvl_offset));
   m_data.obs_elv  = atof(l.get_item(mpr_obs_elv_offset));
   m_data.fcst     = atof(l.get_item(mpr_fcst_offset));
   m_data.obs      = atof(l.get_item(mpr_obs_offset));
   m_data.climo    = atof(l.get_item(mpr_climo_offset));

   // Parse OBS_QC, if present
   if(l.n_items() > mpr_obs_qc_offset) {
      m_data.obs_qc = l.get_item(mpr_obs_qc_offset);
   }
   else {
      m_data.obs_qc.clear();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_isc_line(STATLine &l, ISCInfo &i_info, int &iscale) {

   i_info.total    = atoi(l.get_item(isc_total_offset));
   i_info.tile_dim = atoi(l.get_item(isc_tile_dim_offset));
   i_info.tile_xll = atoi(l.get_item(isc_tile_xll_offset));
   i_info.tile_yll = atoi(l.get_item(isc_tile_yll_offset));
   i_info.n_scale  = atoi(l.get_item(isc_nscale_offset));
   iscale          = atoi(l.get_item(isc_iscale_offset));
   i_info.mse      = atof(l.get_item(isc_mse_offset));
   i_info.isc      = atof(l.get_item(isc_isc_offset));
   i_info.fen      = atof(l.get_item(isc_fenergy2_offset));
   i_info.oen      = atof(l.get_item(isc_oenergy2_offset));
   i_info.baser    = atof(l.get_item(isc_baser_offset));
   i_info.fbias    = atof(l.get_item(isc_fbias_offset));

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_rhist_line(STATLine &l, RHISTData &r_data) {

   if(less_than_met_version(l.version(), "V5.1")) {
      parse_rhist_line_pre5_1 (l, r_data);
   }
   else {
      parse_rhist_line_current(l, r_data);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_rhist_line_pre5_1(STATLine &l, RHISTData &r_data) {
   int i;

   r_data.total  = atoi(l.get_item(rhist_total_offset));
   r_data.crps   = atof(l.get_item(rhist_crps_offset));
   r_data.ign    = atof(l.get_item(rhist_ign_offset));
   r_data.n_rank = atoi(l.get_item(rhist_n_rank_offset));
   r_data.crpss  = bad_data_double;

   r_data.rhist_na.clear();

   // Parse out RANK_i
   for(i=0; i<r_data.n_rank; i++) {
      r_data.rhist_na.add(atoi(l.get_item(rhist_rank_offset_pre5_1(i))));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_rhist_line_current(STATLine &l, RHISTData &r_data) {
   int i;

   r_data.total  = atoi(l.get_item(rhist_total_offset));
   r_data.crps   = atof(l.get_item(rhist_crps_offset));
   r_data.ign    = atof(l.get_item(rhist_ign_offset));
   r_data.n_rank = atoi(l.get_item(rhist_n_rank_offset));
   r_data.crpss  = atof(l.get_item(rhist_crpss_offset));

   r_data.rhist_na.clear();

   // Parse out RANK_i
   for(i=0; i<r_data.n_rank; i++) {
      r_data.rhist_na.add(atoi(l.get_item(rhist_rank_offset(i))));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_phist_line(STATLine &l, PHISTData &p_data) {
   int i;

   p_data.total    = atoi(l.get_item(phist_total_offset));
   p_data.bin_size = atof(l.get_item(phist_bin_size_offset));
   p_data.n_bin    = atoi(l.get_item(phist_n_bin_offset));

   p_data.phist_na.clear();

   // Parse out BIN_i
   for(i=0; i<p_data.n_bin; i++) {
      p_data.phist_na.add(atoi(l.get_item(phist_bin_offset(i))));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_orank_line(STATLine &l, ORANKData &o_data) {
   int i;

   o_data.total     = atoi(l.get_item(orank_total_offset));
   o_data.index     = atoi(l.get_item(orank_index_offset));
   o_data.obs_sid   = l.get_item(orank_obs_sid_offset);
   o_data.obs_lat   = atof(l.get_item(orank_obs_lat_offset));
   o_data.obs_lon   = atof(l.get_item(orank_obs_lon_offset));
   o_data.obs_lvl   = atof(l.get_item(orank_obs_lvl_offset));
   o_data.obs_elv   = atof(l.get_item(orank_obs_elv_offset));
   o_data.obs       = atof(l.get_item(orank_obs_offset));

   o_data.pit       = atof(l.get_item(orank_pit_offset));

   o_data.rank      = atoi(l.get_item(orank_rank_offset));
   o_data.n_ens_vld = atoi(l.get_item(orank_n_ens_vld_offset));
   o_data.n_ens     = atoi(l.get_item(orank_n_ens_offset));

   // Parse out ENS_i
   o_data.ens_na.clear();
   for(i=0; i<o_data.n_ens; i++) {
      o_data.ens_na.add(atof(l.get_item(orank_ens_offset(i))));
   }

   // Parse OBS_QC, if present
   if(l.n_items() > orank_obs_qc_offset(o_data.n_ens)) {
      o_data.obs_qc = l.get_item(orank_obs_qc_offset(o_data.n_ens));
   }
   else {
      o_data.obs_qc.clear();
   }

   // Parse ENS_MEAN, if present
   if(l.n_items() > orank_ens_mean_offset(o_data.n_ens)) {
      o_data.ens_mean = atof(l.get_item(orank_ens_mean_offset(o_data.n_ens)));
   }
   else {
      o_data.ens_mean = bad_data_double;
   }

   // Parse CLIMO, if present
   if(l.n_items() > orank_climo_offset(o_data.n_ens)) {
      o_data.climo= atof(l.get_item(orank_climo_offset(o_data.n_ens)));
   }
   else {
      o_data.climo = bad_data_double;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_ssvar_line(STATLine &l, SSVARInfo &ssvar_info) {

   ssvar_info.n_bin    = atoi(l.get_item(ssvar_n_bin_offset));
   ssvar_info.bin_i    = atoi(l.get_item(ssvar_bin_i_offset));
   ssvar_info.bin_n    = atoi(l.get_item(ssvar_bin_n_offset));

   ssvar_info.var_min  = atof(l.get_item(ssvar_var_min_offset));
   ssvar_info.var_max  = atof(l.get_item(ssvar_var_max_offset));
   ssvar_info.var_mean = atof(l.get_item(ssvar_var_mean_offset));

   ssvar_info.sl1l2_info.scount = ssvar_info.bin_n;
   ssvar_info.sl1l2_info.fbar   = atof(l.get_item(ssvar_fbar_offset));
   ssvar_info.sl1l2_info.obar   = atof(l.get_item(ssvar_obar_offset));
   ssvar_info.sl1l2_info.fobar  = atof(l.get_item(ssvar_fobar_offset));
   ssvar_info.sl1l2_info.ffbar  = atof(l.get_item(ssvar_ffbar_offset));
   ssvar_info.sl1l2_info.oobar  = atof(l.get_item(ssvar_oobar_offset));

   return;
}

////////////////////////////////////////////////////////////////////////
