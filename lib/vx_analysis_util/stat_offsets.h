// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MET_ANALYSIS_STAT_OFFSETS_H__
#define  __MET_ANALYSIS_STAT_OFFSETS_H__


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

//
// SAL1L2 Line Type offsets
//
static const int sal1l2_total_offset     = nhc + 0;
static const int sal1l2_fabar_offset     = nhc + 1;
static const int sal1l2_oabar_offset     = nhc + 2;
static const int sal1l2_foabar_offset    = nhc + 3;
static const int sal1l2_ffabar_offset    = nhc + 4;
static const int sal1l2_ooabar_offset    = nhc + 5;

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
static const int nbrcts_total_offset     = nhc + 0;
static const int nbrcts_baser_offset     = nhc + 1;
static const int nbrcts_baser_ncl_offset = nhc + 2;
static const int nbrcts_baser_ncu_offset = nhc + 3;
static const int nbrcts_baser_bcl_offset = nhc + 4;
static const int nbrcts_baser_bcu_offset = nhc + 5;
static const int nbrcts_fmean_offset     = nhc + 6;
static const int nbrcts_fmean_ncl_offset = nhc + 7;
static const int nbrcts_fmean_ncu_offset = nhc + 8;
static const int nbrcts_fmean_bcl_offset = nhc + 9;
static const int nbrcts_fmean_bcu_offset = nhc + 10;
static const int nbrcts_acc_offset       = nhc + 11;
static const int nbrcts_acc_ncl_offset   = nhc + 12;
static const int nbrcts_acc_ncu_offset   = nhc + 13;
static const int nbrcts_acc_bcl_offset   = nhc + 14;
static const int nbrcts_acc_bcu_offset   = nhc + 15;
static const int nbrcts_fbias_offset     = nhc + 16;
static const int nbrcts_fbias_bcl_offset = nhc + 17;
static const int nbrcts_fbias_bcu_offset = nhc + 18;
static const int nbrcts_pody_offset      = nhc + 19;
static const int nbrcts_pody_ncl_offset  = nhc + 20;
static const int nbrcts_pody_ncu_offset  = nhc + 21;
static const int nbrcts_pody_bcl_offset  = nhc + 22;
static const int nbrcts_pody_bcu_offset  = nhc + 23;
static const int nbrcts_podn_offset      = nhc + 24;
static const int nbrcts_podn_ncl_offset  = nhc + 25;
static const int nbrcts_podn_ncu_offset  = nhc + 26;
static const int nbrcts_podn_bcl_offset  = nhc + 27;
static const int nbrcts_podn_bcu_offset  = nhc + 28;
static const int nbrcts_pofd_offset      = nhc + 29;
static const int nbrcts_pofd_ncl_offset  = nhc + 30;
static const int nbrcts_pofd_ncu_offset  = nhc + 31;
static const int nbrcts_pofd_bcl_offset  = nhc + 32;
static const int nbrcts_pofd_bcu_offset  = nhc + 33;
static const int nbrcts_far_offset       = nhc + 34;
static const int nbrcts_far_ncl_offset   = nhc + 35;
static const int nbrcts_far_ncu_offset   = nhc + 36;
static const int nbrcts_far_bcl_offset   = nhc + 37;
static const int nbrcts_far_bcu_offset   = nhc + 38;
static const int nbrcts_csi_offset       = nhc + 39;
static const int nbrcts_csi_ncl_offset   = nhc + 40;
static const int nbrcts_csi_ncu_offset   = nhc + 41;
static const int nbrcts_csi_bcl_offset   = nhc + 42;
static const int nbrcts_csi_bcu_offset   = nhc + 43;
static const int nbrcts_gss_offset       = nhc + 44;
static const int nbrcts_gss_bcl_offset   = nhc + 45;
static const int nbrcts_gss_bcu_offset   = nhc + 46;
static const int nbrcts_hk_offset        = nhc + 47;
static const int nbrcts_hk_ncl_offset    = nhc + 48;
static const int nbrcts_hk_ncu_offset    = nhc + 49;
static const int nbrcts_hk_bcl_offset    = nhc + 50;
static const int nbrcts_hk_bcu_offset    = nhc + 51;
static const int nbrcts_hss_offset       = nhc + 52;
static const int nbrcts_hss_bcl_offset   = nhc + 53;
static const int nbrcts_hss_bcu_offset   = nhc + 54;
static const int nbrcts_odds_offset      = nhc + 55;
static const int nbrcts_odds_ncl_offset  = nhc + 56;
static const int nbrcts_odds_ncu_offset  = nhc + 57;
static const int nbrcts_odds_bcl_offset  = nhc + 58;
static const int nbrcts_odds_bcu_offset  = nhc + 59;

//
// NBRCNT Line Type offsets
//
static const int nbrcnt_total_offset     = nhc + 0;
static const int nbrcnt_fbs_offset       = nhc + 1;
static const int nbrcnt_fbs_bcl_offset   = nhc + 2;
static const int nbrcnt_fbs_bcu_offset   = nhc + 3;
static const int nbrcnt_fss_offset       = nhc + 4;
static const int nbrcnt_fss_bcl_offset   = nhc + 5;
static const int nbrcnt_fss_bcu_offset   = nhc + 6;

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
static const int rhist_total_offset      = nhc + 0;
static const int rhist_n_rank_offset     = nhc + 1;
inline int rhist_rank_offset(int i)      { return(nhc + 2 + i); }

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
static const int orank_rank_offset       = nhc + 8;
static const int orank_n_ens_vld_offset  = nhc + 9;
static const int orank_n_ens_offset      = nhc + 10;
inline int orank_ens_offset(int i)       { return(nhc + 11 + i); }

////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_ANALYSIS_STAT_OFFSETS_H__  */


////////////////////////////////////////////////////////////////////////
