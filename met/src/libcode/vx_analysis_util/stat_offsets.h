// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __STAT_OFFSETS_H__
#define  __STAT_OFFSETS_H__


////////////////////////////////////////////////////////////////////////

//
// Header Column offsets
//
static const int version_offset          = 0;
static const int model_offset            = 1;

static const int fcst_lead_offset        = 2;
static const int fcst_valid_beg_offset   = 3;
static const int fcst_valid_end_offset   = 4;

static const int obs_lead_offset         = 5;
static const int obs_valid_beg_offset    = 6;
static const int obs_valid_end_offset    = 7;

static const int fcst_var_offset         = 8;
static const int fcst_lev_offset         = 9;

static const int obs_var_offset          = 10;
static const int obs_lev_offset          = 11;

static const int obtype_offset           = 12;
static const int vx_mask_offset          = 13;

static const int interp_mthd_offset      = 14;
static const int interp_pnts_offset      = 15;

static const int fcst_thresh_offset      = 16;
static const int obs_thresh_offset       = 17;
static const int cov_thresh_offset       = 18;

static const int alpha_offset            = 19;
static const int line_type_offset        = 20;

static const int nhc                     = 21;

//
// SL1L2 Line Type offsets
//
static const int sl1l2_total_offset      = nhc + 0;
static const int sl1l2_fbar_offset       = nhc + 1;
static const int sl1l2_obar_offset       = nhc + 2;
static const int sl1l2_fobar_offset      = nhc + 3;
static const int sl1l2_ffbar_offset      = nhc + 4;
static const int sl1l2_oobar_offset      = nhc + 5;
static const int sl1l2_mae_offset        = nhc + 6;

//
// SAL1L2 Line Type offsets
//
static const int sal1l2_total_offset     = nhc + 0;
static const int sal1l2_fabar_offset     = nhc + 1;
static const int sal1l2_oabar_offset     = nhc + 2;
static const int sal1l2_foabar_offset    = nhc + 3;
static const int sal1l2_ffabar_offset    = nhc + 4;
static const int sal1l2_ooabar_offset    = nhc + 5;
static const int sal1l2_mae_offset       = nhc + 6;

//
// VL1L2 Line Type offsets
//
static const int vl1l2_total_offset      = nhc + 0;
static const int vl1l2_ufbar_offset      = nhc + 1;
static const int vl1l2_vfbar_offset      = nhc + 2;
static const int vl1l2_uobar_offset      = nhc + 3;
static const int vl1l2_vobar_offset      = nhc + 4;
static const int vl1l2_uvfobar_offset    = nhc + 5;
static const int vl1l2_uvffbar_offset    = nhc + 6;
static const int vl1l2_uvoobar_offset    = nhc + 7;

//
// VAL1L2 Line Type offsets
//
static const int val1l2_total_offset     = nhc + 0;
static const int val1l2_ufabar_offset    = nhc + 1;
static const int val1l2_vfabar_offset    = nhc + 2;
static const int val1l2_uoabar_offset    = nhc + 3;
static const int val1l2_voabar_offset    = nhc + 4;
static const int val1l2_uvfoabar_offset  = nhc + 5;
static const int val1l2_uvffabar_offset  = nhc + 6;
static const int val1l2_uvooabar_offset  = nhc + 7;

//
// FHO Line Type offsets
//
static const int fho_total_offset        = nhc + 0;
static const int fho_f_rate_offset       = nhc + 1;
static const int fho_h_rate_offset       = nhc + 2;
static const int fho_o_rate_offset       = nhc + 3;

//
// CTC Line Type offsets
//
static const int ctc_total_offset        = nhc + 0;
static const int ctc_fy_oy_offset        = nhc + 1;
static const int ctc_fy_on_offset        = nhc + 2;
static const int ctc_fn_oy_offset        = nhc + 3;
static const int ctc_fn_on_offset        = nhc + 4;

//
// CTS Line Type offsets
//
static const int cts_total_offset        = nhc + 0;
static const int cts_baser_offset        = nhc + 1;
static const int cts_baser_ncl_offset    = nhc + 2;
static const int cts_baser_ncu_offset    = nhc + 3;
static const int cts_baser_bcl_offset    = nhc + 4;
static const int cts_baser_bcu_offset    = nhc + 5;
static const int cts_fmean_offset        = nhc + 6;
static const int cts_fmean_ncl_offset    = nhc + 7;
static const int cts_fmean_ncu_offset    = nhc + 8;
static const int cts_fmean_bcl_offset    = nhc + 9;
static const int cts_fmean_bcu_offset    = nhc + 10;
static const int cts_acc_offset          = nhc + 11;
static const int cts_acc_ncl_offset      = nhc + 12;
static const int cts_acc_ncu_offset      = nhc + 13;
static const int cts_acc_bcl_offset      = nhc + 14;
static const int cts_acc_bcu_offset      = nhc + 15;
static const int cts_fbias_offset        = nhc + 16;
static const int cts_fbias_bcl_offset    = nhc + 17;
static const int cts_fbias_bcu_offset    = nhc + 18;
static const int cts_pody_offset         = nhc + 19;
static const int cts_pody_ncl_offset     = nhc + 20;
static const int cts_pody_ncu_offset     = nhc + 21;
static const int cts_pody_bcl_offset     = nhc + 22;
static const int cts_pody_bcu_offset     = nhc + 23;
static const int cts_podn_offset         = nhc + 24;
static const int cts_podn_ncl_offset     = nhc + 25;
static const int cts_podn_ncu_offset     = nhc + 26;
static const int cts_podn_bcl_offset     = nhc + 27;
static const int cts_podn_bcu_offset     = nhc + 28;
static const int cts_pofd_offset         = nhc + 29;
static const int cts_pofd_ncl_offset     = nhc + 30;
static const int cts_pofd_ncu_offset     = nhc + 31;
static const int cts_pofd_bcl_offset     = nhc + 32;
static const int cts_pofd_bcu_offset     = nhc + 33;
static const int cts_far_offset          = nhc + 34;
static const int cts_far_ncl_offset      = nhc + 35;
static const int cts_far_ncu_offset      = nhc + 36;
static const int cts_far_bcl_offset      = nhc + 37;
static const int cts_far_bcu_offset      = nhc + 38;
static const int cts_csi_offset          = nhc + 39;
static const int cts_csi_ncl_offset      = nhc + 40;
static const int cts_csi_ncu_offset      = nhc + 41;
static const int cts_csi_bcl_offset      = nhc + 42;
static const int cts_csi_bcu_offset      = nhc + 43;
static const int cts_gss_offset          = nhc + 44;
static const int cts_gss_bcl_offset      = nhc + 45;
static const int cts_gss_bcu_offset      = nhc + 46;
static const int cts_hk_offset           = nhc + 47;
static const int cts_hk_ncl_offset       = nhc + 48;
static const int cts_hk_ncu_offset       = nhc + 49;
static const int cts_hk_bcl_offset       = nhc + 50;
static const int cts_hk_bcu_offset       = nhc + 51;
static const int cts_hss_offset          = nhc + 52;
static const int cts_hss_bcl_offset      = nhc + 53;
static const int cts_hss_bcu_offset      = nhc + 54;
static const int cts_odds_offset         = nhc + 55;
static const int cts_odds_ncl_offset     = nhc + 56;
static const int cts_odds_ncu_offset     = nhc + 57;
static const int cts_odds_bcl_offset     = nhc + 58;
static const int cts_odds_bcu_offset     = nhc + 59;
static const int cts_lodds_offset        = nhc + 60;
static const int cts_lodds_ncl_offset    = nhc + 61;
static const int cts_lodds_ncu_offset    = nhc + 62;
static const int cts_lodds_bcl_offset    = nhc + 63;
static const int cts_lodds_bcu_offset    = nhc + 64;
static const int cts_orss_offset         = nhc + 65;
static const int cts_orss_ncl_offset     = nhc + 66;
static const int cts_orss_ncu_offset     = nhc + 67;
static const int cts_orss_bcl_offset     = nhc + 68;
static const int cts_orss_bcu_offset     = nhc + 69;
static const int cts_eds_offset          = nhc + 70;
static const int cts_eds_ncl_offset      = nhc + 71;
static const int cts_eds_ncu_offset      = nhc + 72;
static const int cts_eds_bcl_offset      = nhc + 73;
static const int cts_eds_bcu_offset      = nhc + 74;
static const int cts_seds_offset         = nhc + 75;
static const int cts_seds_ncl_offset     = nhc + 76;
static const int cts_seds_ncu_offset     = nhc + 77;
static const int cts_seds_bcl_offset     = nhc + 78;
static const int cts_seds_bcu_offset     = nhc + 79;
static const int cts_edi_offset          = nhc + 80;
static const int cts_edi_ncl_offset      = nhc + 81;
static const int cts_edi_ncu_offset      = nhc + 82;
static const int cts_edi_bcl_offset      = nhc + 83;
static const int cts_edi_bcu_offset      = nhc + 84;
static const int cts_sedi_offset         = nhc + 85;
static const int cts_sedi_ncl_offset     = nhc + 86;
static const int cts_sedi_ncu_offset     = nhc + 87;
static const int cts_sedi_bcl_offset     = nhc + 88;
static const int cts_sedi_bcu_offset     = nhc + 89;
static const int cts_bagss_offset        = nhc + 90;
static const int cts_bagss_bcl_offset    = nhc + 91;
static const int cts_bagss_bcu_offset    = nhc + 92;

//
// MCTC Line Type offsets
//
static const int mctc_total_offset       = nhc + 0;
static const int mctc_n_cat_offset       = nhc + 1;
inline int mctc_fi_oj_offset(int i, int j, int n) { return(nhc + 2 + n*i + j); }

//
// CTS Line Type offsets
//
static const int mcts_total_offset       = nhc + 0;
static const int mcts_n_cat_offset       = nhc + 1;
static const int mcts_acc_offset         = nhc + 2;
static const int mcts_acc_ncl_offset     = nhc + 3;
static const int mcts_acc_ncu_offset     = nhc + 4;
static const int mcts_acc_bcl_offset     = nhc + 5;
static const int mcts_acc_bcu_offset     = nhc + 6;
static const int mcts_hk_offset          = nhc + 7;
static const int mcts_hk_bcl_offset      = nhc + 8;
static const int mcts_hk_bcu_offset      = nhc + 9;
static const int mcts_hss_offset         = nhc + 10;
static const int mcts_hss_bcl_offset     = nhc + 11;
static const int mcts_hss_bcu_offset     = nhc + 12;
static const int mcts_ger_offset         = nhc + 13;
static const int mcts_ger_bcl_offset     = nhc + 14;
static const int mcts_ger_bcu_offset     = nhc + 15;

//
// CNT Line Type offsets
//
static const int cnt_total_offset        = nhc + 0;
static const int cnt_fbar_offset         = nhc + 1;
static const int cnt_fbar_ncl_offset     = nhc + 2;
static const int cnt_fbar_ncu_offset     = nhc + 3;
static const int cnt_fbar_bcl_offset     = nhc + 4;
static const int cnt_fbar_bcu_offset     = nhc + 5;
static const int cnt_fstdev_offset       = nhc + 6;
static const int cnt_fstdev_ncl_offset   = nhc + 7;
static const int cnt_fstdev_ncu_offset   = nhc + 8;
static const int cnt_fstdev_bcl_offset   = nhc + 9;
static const int cnt_fstdev_bcu_offset   = nhc + 10;
static const int cnt_obar_offset         = nhc + 11;
static const int cnt_obar_ncl_offset     = nhc + 12;
static const int cnt_obar_ncu_offset     = nhc + 13;
static const int cnt_obar_bcl_offset     = nhc + 14;
static const int cnt_obar_bcu_offset     = nhc + 15;
static const int cnt_ostdev_offset       = nhc + 16;
static const int cnt_ostdev_ncl_offset   = nhc + 17;
static const int cnt_ostdev_ncu_offset   = nhc + 18;
static const int cnt_ostdev_bcl_offset   = nhc + 19;
static const int cnt_ostdev_bcu_offset   = nhc + 20;
static const int cnt_pr_corr_offset      = nhc + 21;
static const int cnt_pr_corr_ncl_offset  = nhc + 22;
static const int cnt_pr_corr_ncu_offset  = nhc + 23;
static const int cnt_pr_corr_bcl_offset  = nhc + 24;
static const int cnt_pr_corr_bcu_offset  = nhc + 25;
static const int cnt_sp_corr_offset      = nhc + 26;
static const int cnt_kt_corr_offset      = nhc + 27;
static const int cnt_ranks_offset        = nhc + 28;
static const int cnt_frank_ties_offset   = nhc + 29;
static const int cnt_orank_ties_offset   = nhc + 30;
static const int cnt_me_offset           = nhc + 31;
static const int cnt_me_ncl_offset       = nhc + 32;
static const int cnt_me_ncu_offset       = nhc + 33;
static const int cnt_me_bcl_offset       = nhc + 34;
static const int cnt_me_bcu_offset       = nhc + 35;
static const int cnt_estdev_offset       = nhc + 36;
static const int cnt_estdev_ncl_offset   = nhc + 37;
static const int cnt_estdev_ncu_offset   = nhc + 38;
static const int cnt_estdev_bcl_offset   = nhc + 39;
static const int cnt_estdev_bcu_offset   = nhc + 40;
static const int cnt_mbias_offset        = nhc + 41;
static const int cnt_mbias_bcl_offset    = nhc + 42;
static const int cnt_mbias_bcu_offset    = nhc + 43;
static const int cnt_mae_offset          = nhc + 44;
static const int cnt_mae_bcl_offset      = nhc + 45;
static const int cnt_mae_bcu_offset      = nhc + 46;
static const int cnt_mse_offset          = nhc + 47;
static const int cnt_mse_bcl_offset      = nhc + 48;
static const int cnt_mse_bcu_offset      = nhc + 49;
static const int cnt_bcmse_offset        = nhc + 50;
static const int cnt_bcmse_bcl_offset    = nhc + 51;
static const int cnt_bcmse_bcu_offset    = nhc + 52;
static const int cnt_rmse_offset         = nhc + 53;
static const int cnt_rmse_bcl_offset     = nhc + 54;
static const int cnt_rmse_bcu_offset     = nhc + 55;
static const int cnt_e10_offset          = nhc + 56;
static const int cnt_e10_bcl_offset      = nhc + 57;
static const int cnt_e10_bcu_offset      = nhc + 58;
static const int cnt_e25_offset          = nhc + 59;
static const int cnt_e25_bcl_offset      = nhc + 60;
static const int cnt_e25_bcu_offset      = nhc + 61;
static const int cnt_e50_offset          = nhc + 62;
static const int cnt_e50_bcl_offset      = nhc + 63;
static const int cnt_e50_bcu_offset      = nhc + 64;
static const int cnt_e75_offset          = nhc + 65;
static const int cnt_e75_bcl_offset      = nhc + 66;
static const int cnt_e75_bcu_offset      = nhc + 67;
static const int cnt_e90_offset          = nhc + 68;
static const int cnt_e90_bcl_offset      = nhc + 69;
static const int cnt_e90_bcu_offset      = nhc + 70;
static const int cnt_eiqr_offset         = nhc + 71;
static const int cnt_eiqr_bcl_offset     = nhc + 72;
static const int cnt_eiqr_bcu_offset     = nhc + 73;
static const int cnt_mad_offset          = nhc + 74;
static const int cnt_mad_bcl_offset      = nhc + 75;
static const int cnt_mad_bcu_offset      = nhc + 76;
static const int cnt_anom_corr_offset    = nhc + 77;
static const int cnt_anom_corr_ncl_offset= nhc + 78;
static const int cnt_anom_corr_ncu_offset= nhc + 79;
static const int cnt_anom_corr_bcl_offset= nhc + 80;
static const int cnt_anom_corr_bcu_offset= nhc + 81;
static const int cnt_me2_offset          = nhc + 82;
static const int cnt_me2_bcl_offset      = nhc + 83;
static const int cnt_me2_bcu_offset      = nhc + 84;
static const int cnt_msess_offset        = nhc + 85;
static const int cnt_msess_bcl_offset    = nhc + 86;
static const int cnt_msess_bcu_offset    = nhc + 87;

//
// MPR Line Type offsets
//
static const int mpr_total_offset        = nhc + 0;
static const int mpr_index_offset        = nhc + 1;
static const int mpr_obs_sid_offset      = nhc + 2;
static const int mpr_obs_lat_offset      = nhc + 3;
static const int mpr_obs_lon_offset      = nhc + 4;
static const int mpr_obs_lvl_offset      = nhc + 5;
static const int mpr_obs_elv_offset      = nhc + 6;
static const int mpr_fcst_offset         = nhc + 7;
static const int mpr_obs_offset          = nhc + 8;
static const int mpr_climo_offset        = nhc + 9;
static const int mpr_obs_qc_offset       = nhc + 10;

//
// NBRCTC Line Type offsets
//
static const int nbrctc_total_offset     = nhc + 0;
static const int nbrctc_fy_oy_offset     = nhc + 1;
static const int nbrctc_fy_on_offset     = nhc + 2;
static const int nbrctc_fn_oy_offset     = nhc + 3;
static const int nbrctc_fn_on_offset     = nhc + 4;

//
// NBRCTS Line Type offsets
//
static const int nbrcts_total_offset        = nhc + 0;
static const int nbrcts_baser_offset        = nhc + 1;
static const int nbrcts_baser_ncl_offset    = nhc + 2;
static const int nbrcts_baser_ncu_offset    = nhc + 3;
static const int nbrcts_baser_bcl_offset    = nhc + 4;
static const int nbrcts_baser_bcu_offset    = nhc + 5;
static const int nbrcts_fmean_offset        = nhc + 6;
static const int nbrcts_fmean_ncl_offset    = nhc + 7;
static const int nbrcts_fmean_ncu_offset    = nhc + 8;
static const int nbrcts_fmean_bcl_offset    = nhc + 9;
static const int nbrcts_fmean_bcu_offset    = nhc + 10;
static const int nbrcts_acc_offset          = nhc + 11;
static const int nbrcts_acc_ncl_offset      = nhc + 12;
static const int nbrcts_acc_ncu_offset      = nhc + 13;
static const int nbrcts_acc_bcl_offset      = nhc + 14;
static const int nbrcts_acc_bcu_offset      = nhc + 15;
static const int nbrcts_fbias_offset        = nhc + 16;
static const int nbrcts_fbias_bcl_offset    = nhc + 17;
static const int nbrcts_fbias_bcu_offset    = nhc + 18;
static const int nbrcts_pody_offset         = nhc + 19;
static const int nbrcts_pody_ncl_offset     = nhc + 20;
static const int nbrcts_pody_ncu_offset     = nhc + 21;
static const int nbrcts_pody_bcl_offset     = nhc + 22;
static const int nbrcts_pody_bcu_offset     = nhc + 23;
static const int nbrcts_podn_offset         = nhc + 24;
static const int nbrcts_podn_ncl_offset     = nhc + 25;
static const int nbrcts_podn_ncu_offset     = nhc + 26;
static const int nbrcts_podn_bcl_offset     = nhc + 27;
static const int nbrcts_podn_bcu_offset     = nhc + 28;
static const int nbrcts_pofd_offset         = nhc + 29;
static const int nbrcts_pofd_ncl_offset     = nhc + 30;
static const int nbrcts_pofd_ncu_offset     = nhc + 31;
static const int nbrcts_pofd_bcl_offset     = nhc + 32;
static const int nbrcts_pofd_bcu_offset     = nhc + 33;
static const int nbrcts_far_offset          = nhc + 34;
static const int nbrcts_far_ncl_offset      = nhc + 35;
static const int nbrcts_far_ncu_offset      = nhc + 36;
static const int nbrcts_far_bcl_offset      = nhc + 37;
static const int nbrcts_far_bcu_offset      = nhc + 38;
static const int nbrcts_csi_offset          = nhc + 39;
static const int nbrcts_csi_ncl_offset      = nhc + 40;
static const int nbrcts_csi_ncu_offset      = nhc + 41;
static const int nbrcts_csi_bcl_offset      = nhc + 42;
static const int nbrcts_csi_bcu_offset      = nhc + 43;
static const int nbrcts_gss_offset          = nhc + 44;
static const int nbrcts_gss_bcl_offset      = nhc + 45;
static const int nbrcts_gss_bcu_offset      = nhc + 46;
static const int nbrcts_hk_offset           = nhc + 47;
static const int nbrcts_hk_ncl_offset       = nhc + 48;
static const int nbrcts_hk_ncu_offset       = nhc + 49;
static const int nbrcts_hk_bcl_offset       = nhc + 50;
static const int nbrcts_hk_bcu_offset       = nhc + 51;
static const int nbrcts_hss_offset          = nhc + 52;
static const int nbrcts_hss_bcl_offset      = nhc + 53;
static const int nbrcts_hss_bcu_offset      = nhc + 54;
static const int nbrcts_odds_offset         = nhc + 55;
static const int nbrcts_odds_ncl_offset     = nhc + 56;
static const int nbrcts_odds_ncu_offset     = nhc + 57;
static const int nbrcts_odds_bcl_offset     = nhc + 58;
static const int nbrcts_odds_bcu_offset     = nhc + 59;
static const int nbrcts_lodds_offset        = nhc + 60;
static const int nbrcts_lodds_ncl_offset    = nhc + 61;
static const int nbrcts_lodds_ncu_offset    = nhc + 62;
static const int nbrcts_lodds_bcl_offset    = nhc + 63;
static const int nbrcts_lodds_bcu_offset    = nhc + 64;
static const int nbrcts_orss_offset         = nhc + 65;
static const int nbrcts_orss_ncl_offset     = nhc + 66;
static const int nbrcts_orss_ncu_offset     = nhc + 67;
static const int nbrcts_orss_bcl_offset     = nhc + 68;
static const int nbrcts_orss_bcu_offset     = nhc + 69;
static const int nbrcts_eds_offset          = nhc + 70;
static const int nbrcts_eds_ncl_offset      = nhc + 71;
static const int nbrcts_eds_ncu_offset      = nhc + 72;
static const int nbrcts_eds_bcl_offset      = nhc + 73;
static const int nbrcts_eds_bcu_offset      = nhc + 74;
static const int nbrcts_seds_offset         = nhc + 75;
static const int nbrcts_seds_ncl_offset     = nhc + 76;
static const int nbrcts_seds_ncu_offset     = nhc + 77;
static const int nbrcts_seds_bcl_offset     = nhc + 78;
static const int nbrcts_seds_bcu_offset     = nhc + 79;
static const int nbrcts_edi_offset          = nhc + 80;
static const int nbrcts_edi_ncl_offset      = nhc + 81;
static const int nbrcts_edi_ncu_offset      = nhc + 82;
static const int nbrcts_edi_bcl_offset      = nhc + 83;
static const int nbrcts_edi_bcu_offset      = nhc + 84;
static const int nbrcts_sedi_offset         = nhc + 85;
static const int nbrcts_sedi_ncl_offset     = nhc + 86;
static const int nbrcts_sedi_ncu_offset     = nhc + 87;
static const int nbrcts_sedi_bcl_offset     = nhc + 88;
static const int nbrcts_sedi_bcu_offset     = nhc + 89;
static const int nbrcts_bagss_offset        = nhc + 90;
static const int nbrcts_bagss_bcl_offset    = nhc + 91;
static const int nbrcts_bagss_bcu_offset    = nhc + 92;

//
// NBRCNT Line Type offsets
//
static const int nbrcnt_total_offset      = nhc + 0;
static const int nbrcnt_fbs_offset        = nhc + 1;
static const int nbrcnt_fbs_bcl_offset    = nhc + 2;
static const int nbrcnt_fbs_bcu_offset    = nhc + 3;
static const int nbrcnt_fss_offset        = nhc + 4;
static const int nbrcnt_fss_bcl_offset    = nhc + 5;
static const int nbrcnt_fss_bcu_offset    = nhc + 6;
static const int nbrcnt_afss_offset       = nhc + 7;
static const int nbrcnt_afss_bcl_offset   = nhc + 8;
static const int nbrcnt_afss_bcu_offset   = nhc + 9;
static const int nbrcnt_ufss_offset       = nhc + 10;
static const int nbrcnt_ufss_bcl_offset   = nhc + 11;
static const int nbrcnt_ufss_bcu_offset   = nhc + 12;
static const int nbrcnt_f_rate_offset     = nhc + 13;
static const int nbrcnt_f_rate_bcl_offset = nhc + 14;
static const int nbrcnt_f_rate_bcu_offset = nhc + 15;
static const int nbrcnt_o_rate_offset     = nhc + 16;
static const int nbrcnt_o_rate_bcl_offset = nhc + 17;
static const int nbrcnt_o_rate_bcu_offset = nhc + 18;

//
// ISC Line Type offsets
//
static const int isc_total_offset        = nhc + 0;
static const int isc_tile_dim_offset     = nhc + 1;
static const int isc_tile_xll_offset     = nhc + 2;
static const int isc_tile_yll_offset     = nhc + 3;
static const int isc_nscale_offset       = nhc + 4;
static const int isc_iscale_offset       = nhc + 5;
static const int isc_mse_offset          = nhc + 6;
static const int isc_isc_offset          = nhc + 7;
static const int isc_fenergy2_offset     = nhc + 8;
static const int isc_oenergy2_offset     = nhc + 9;
static const int isc_baser_offset        = nhc + 10;
static const int isc_fbias_offset        = nhc + 11;

//
// PCT Line Type offsets
//
static const int pct_total_offset        = nhc + 0;
static const int pct_n_thresh_offset     = nhc + 1;
inline int pct_thresh_offset(int i)      { return(nhc + 2 + 3*i    ); }
inline int pct_oy_offset(int i)          { return(nhc + 2 + 3*i + 1); }
inline int pct_on_offset(int i)          { return(nhc + 2 + 3*i + 2); }

//
// PSTD Line Type offsets
//
static const int pstd_total_offset       = nhc + 0;
static const int pstd_n_thresh_offset    = nhc + 1;
static const int pstd_baser_offset       = nhc + 2;
static const int pstd_baser_ncl_offset   = nhc + 3;
static const int pstd_baser_ncu_offset   = nhc + 4;
static const int pstd_reliability_offset = nhc + 5;
static const int pstd_resolution_offset  = nhc + 6;
static const int pstd_uncertainty_offset = nhc + 7;
static const int pstd_roc_auc_offset     = nhc + 8;
static const int pstd_brier_offset       = nhc + 9;
static const int pstd_brier_ncl_offset   = nhc + 10;
static const int pstd_brier_ncu_offset   = nhc + 11;
static const int pstd_briercl_offset     = nhc + 12;
static const int pstd_briercl_ncl_offset = nhc + 13;
static const int pstd_briercl_ncu_offset = nhc + 14;
static const int pstd_bss_offset         = nhc + 15;
inline int pstd_thresh_offset(int i)     { return(nhc + 12 + i); }

//
// PJC Line Type offsets
//
static const int pjc_total_offset        = nhc + 0;
static const int pjc_n_thresh_offset     = nhc + 1;
inline int pjc_thresh_offset(int i)      { return(nhc + 2 + 7*i    ); }
inline int pjc_oy_tp_offset(int i)       { return(nhc + 2 + 7*i + 1); }
inline int pjc_on_tp_offset(int i)       { return(nhc + 2 + 7*i + 2); }
inline int pjc_calibration_offset(int i) { return(nhc + 2 + 7*i + 3); }
inline int pjc_refinement_offset(int i)  { return(nhc + 2 + 7*i + 4); }
inline int pjc_likelihood_offset(int i)  { return(nhc + 2 + 7*i + 5); }
inline int pjc_baser_offset(int i)       { return(nhc + 2 + 7*i + 6); }

//
// PRC Line Type offsets
//
static const int prc_total_offset        = nhc + 0;
static const int prc_n_thresh_offset     = nhc + 1;
inline int prc_thresh_offset(int i)      { return(nhc + 2 + 3*i    ); }
inline int prc_pody_offset(int i)        { return(nhc + 2 + 3*i + 1); }
inline int prc_pofd_offset(int i)        { return(nhc + 2 + 3*i + 2); }

//
// RHIST Line Type offsets
//
static const int rhist_total_offset        = nhc + 0;
static const int rhist_crps_offset         = nhc + 1;
static const int rhist_ign_offset          = nhc + 2;
static const int rhist_n_rank_offset       = nhc + 3;
static const int rhist_crpss_offset        = nhc + 4;
inline int rhist_rank_offset(int i)        { return(nhc + 5 + i); }
inline int rhist_rank_offset_pre5_1(int i) { return(nhc + 4 + i); }

//
// PHIST Line Type offsets
//
static const int phist_total_offset      = nhc + 0;
static const int phist_bin_size_offset   = nhc + 1;
static const int phist_n_bin_offset      = nhc + 2;
inline int phist_bin_offset(int i)       { return(nhc + 3 + i); }

//
// ORANK Line Type offsets
//
static const int orank_total_offset      = nhc + 0;
static const int orank_index_offset      = nhc + 1;
static const int orank_obs_sid_offset    = nhc + 2;
static const int orank_obs_lat_offset    = nhc + 3;
static const int orank_obs_lon_offset    = nhc + 4;
static const int orank_obs_lvl_offset    = nhc + 5;
static const int orank_obs_elv_offset    = nhc + 6;
static const int orank_obs_offset        = nhc + 7;
static const int orank_pit_offset        = nhc + 8;
static const int orank_rank_offset       = nhc + 9;
static const int orank_n_ens_vld_offset  = nhc + 10;
static const int orank_n_ens_offset      = nhc + 11;
inline int orank_ens_offset(int i)      { return(nhc + 12 + i); } // i is the 0-based ensemble member index
inline int orank_obs_qc_offset(int n)   { return(nhc + 12 + n); } // n is the number of ensemble members
inline int orank_ens_mean_offset(int n) { return(nhc + 13 + n); } // n is the number of ensemble members
inline int orank_climo_offset(int n)    { return(nhc + 14 + n); } // n is the number of ensemble members

//
// SSVAR Line Type offsets
//
static const int ssvar_total_offset       = nhc + 0;
static const int ssvar_n_bin_offset       = nhc + 1;
static const int ssvar_bin_i_offset       = nhc + 2;
static const int ssvar_bin_n_offset       = nhc + 3;
static const int ssvar_var_min_offset     = nhc + 4;
static const int ssvar_var_max_offset     = nhc + 5;
static const int ssvar_var_mean_offset    = nhc + 6;
static const int ssvar_fbar_offset        = nhc + 7;
static const int ssvar_obar_offset        = nhc + 8;
static const int ssvar_fobar_offset       = nhc + 9;
static const int ssvar_ffbar_offset       = nhc + 10;
static const int ssvar_oobar_offset       = nhc + 11;
static const int ssvar_fbar_ncl_offset    = nhc + 12;
static const int ssvar_fbar_ncu_offset    = nhc + 13;
static const int ssvar_fstdev_offset      = nhc + 14;
static const int ssvar_fstdev_ncl_offset  = nhc + 15;
static const int ssvar_fstdev_ncu_offset  = nhc + 16;
static const int ssvar_obar_ncl_offset    = nhc + 17;
static const int ssvar_obar_ncu_offset    = nhc + 18;
static const int ssvar_ostdev_offset      = nhc + 19;
static const int ssvar_ostdev_ncl_offset  = nhc + 20;
static const int ssvar_ostdev_ncu_offset  = nhc + 21;
static const int ssvar_pr_corr_offset     = nhc + 22;
static const int ssvar_pr_corr_ncl_offset = nhc + 23;
static const int ssvar_pr_corr_ncu_offset = nhc + 24;
static const int ssvar_me_offset          = nhc + 25;
static const int ssvar_me_ncl_offset      = nhc + 26;
static const int ssvar_me_ncu_offset      = nhc + 27;
static const int ssvar_estdev_offset      = nhc + 28;
static const int ssvar_estdev_ncl_offset  = nhc + 29;
static const int ssvar_estdev_ncu_offset  = nhc + 30;
static const int ssvar_mbias_offset       = nhc + 31;
static const int ssvar_mse_offset         = nhc + 32;
static const int ssvar_bcmse_offset       = nhc + 33;
static const int ssvar_rmse_offset        = nhc + 34;

////////////////////////////////////////////////////////////////////////


#endif   /*  __STAT_OFFSETS_H__  */


////////////////////////////////////////////////////////////////////////
