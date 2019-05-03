// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MODE_COLUMNS_H__
#define  __MODE_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

static const char * mode_hdr_columns [] = {

   "VERSION",                    //   1
   "MODEL",                      //   2
   "FCST_LEAD",                  //   3
   "FCST_VALID",                 //   4
   "FCST_ACCUM",                 //   5
   "OBS_LEAD",                   //   6
   "OBS_VALID",                  //   7
   "OBS_ACCUM",                  //   8
   "FCST_RAD",                   //   9
   "FCST_THR",                   //  10
   "OBS_RAD",                    //  11
   "OBS_THR",                    //  12
   "FCST_VAR",                   //  13
   "FCST_LEV",                   //  14
   "OBS_VAR",                    //  15
   "OBS_LEV"                     //  16
};

static const char * mode_obj_columns [] = {

   "OBJECT_ID",                  //  17
   "OBJECT_CAT",                 //  18
   "CENTROID_X",                 //  19
   "CENTROID_Y",                 //  20
   "CENTROID_LAT",               //  21
   "CENTROID_LON",               //  22
   "AXIS_ANG",                   //  23
   "LENGTH",                     //  24
   "WIDTH",                      //  25
   "AREA",                       //  26
   "AREA_FILTER",                //  27
   "AREA_THRESH",                //  28
   "CURVATURE",                  //  29
   "CURVATURE_X",                //  30
   "CURVATURE_Y",                //  31
   "COMPLEXITY",                 //  32
   "INTENSITY_10",               //  33
   "INTENSITY_25",               //  34
   "INTENSITY_50",               //  35
   "INTENSITY_75",               //  36
   "INTENSITY_90",               //  37
   "INTENSITY_USER",             //  38
   "INTENSITY_SUM",              //  39
   "CENTROID_DIST",              //  40
   "BOUNDARY_DIST",              //  41
   "CONVEX_HULL_DIST",           //  42
   "ANGLE_DIFF",                 //  43
   "AREA_RATIO",                 //  44
   "INTERSECTION_AREA",          //  45
   "UNION_AREA",                 //  46
   "SYMMETRIC_DIFF",             //  47
   "INTERSECTION_OVER_AREA",     //  48
   "COMPLEXITY_RATIO",           //  49
   "PERCENTILE_INTENSITY_RATIO", //  50
   "INTEREST"                    //  51
};

static const char * mode_cts_columns [] = {

   "FIELD",                      //  17
   "TOTAL",                      //  18
   "FY_OY",                      //  19
   "FY_ON",                      //  20
   "FN_OY",                      //  21
   "FN_ON",                      //  22
   "BASER",                      //  23
   "FMEAN",                      //  24
   "ACC",                        //  25
   "FBIAS",                      //  26
   "PODY",                       //  27
   "PODN",                       //  28
   "POFD",                       //  29
   "FAR",                        //  30
   "CSI",                        //  31
   "GSS",                        //  32
   "HK",                         //  33
   "HSS",                        //  34
   "ODDS"                        //  35
};

////////////////////////////////////////////////////////////////////////

static const int n_mode_hdr_columns =
                    sizeof(mode_hdr_columns)/sizeof(*mode_hdr_columns);

static const int n_mode_obj_columns =
                    sizeof(mode_obj_columns)/sizeof(*mode_obj_columns);

static const int n_mode_cts_columns =
                    sizeof(mode_cts_columns)/sizeof(*mode_cts_columns);

////////////////////////////////////////////////////////////////////////

//
// MODE Header column offsets
//
static const int mode_version_offset                     =  0;
static const int mode_model_offset                       =  1;
static const int mode_fcst_lead_offset                   =  2;
static const int mode_fcst_valid_offset                  =  3;
static const int mode_fcst_accum_offset                  =  4;
static const int mode_obs_lead_offset                    =  5;
static const int mode_obs_valid_offset                   =  6;
static const int mode_obs_accum_offset                   =  7;
static const int mode_fcst_rad_offset                    =  8;
static const int mode_fcst_thr_offset                    =  9;
static const int mode_obs_rad_offset                     = 10;
static const int mode_obs_thr_offset                     = 11;
static const int mode_fcst_var_offset                    = 12;
static const int mode_fcst_lev_offset                    = 13;
static const int mode_obs_var_offset                     = 14;
static const int mode_obs_lev_offset                     = 15;

//
// MODE Object column offsets
//
static const int mode_object_id_offset                   = 16;
static const int mode_object_cat_offset                  = 17;
static const int mode_centroid_x_offset                  = 18;
static const int mode_centroid_y_offset                  = 19;
static const int mode_centroid_lat_offset                = 20;
static const int mode_centroid_lon_offset                = 21;
static const int mode_axis_ang_offset                    = 22;
static const int mode_length_offset                      = 23;
static const int mode_width_offset                       = 24;
static const int mode_area_offset                        = 25;
static const int mode_area_filter_offset                 = 26;
static const int mode_area_thresh_offset                 = 27;
static const int mode_curvature_offset                   = 28;
static const int mode_curvature_x_offset                 = 29;
static const int mode_curvature_y_offset                 = 30;
static const int mode_complexity_offset                  = 31;
static const int mode_intensity_10_offset                = 32;
static const int mode_intensity_25_offset                = 33;
static const int mode_intensity_50_offset                = 34;
static const int mode_intensity_75_offset                = 35;
static const int mode_intensity_90_offset                = 36;
static const int mode_intensity_user_offset              = 37;
static const int mode_intensity_sum_offset               = 38;
static const int mode_centroid_dist_offset               = 39;
static const int mode_boundary_dist_offset               = 40;
static const int mode_convex_hull_dist_offset            = 41;
static const int mode_angle_diff_offset                  = 42;
static const int mode_area_ratio_offset                  = 43;
static const int mode_intersection_area_offset           = 44;
static const int mode_union_area_offset                  = 45;
static const int mode_symmetric_diff_offset              = 46;
static const int mode_intersection_over_area_offset      = 47;
static const int mode_complexity_ratio_offset            = 48;
static const int mode_percentile_intensity_ratio_offset  = 49;
static const int mode_interest_offset                    = 50;

//
// MODE Contingecy table column offsets
//
static const int mode_field_offset                       = 16;
static const int mode_total_offset                       = 17;
static const int mode_fy_oy_offset                       = 18;
static const int mode_fy_on_offset                       = 19;
static const int mode_fn_oy_offset                       = 20;
static const int mode_fn_on_offset                       = 21;
static const int mode_baser_offset                       = 22;
static const int mode_fmean_offset                       = 23;
static const int mode_acc_offset                         = 24;
static const int mode_fbias_offset                       = 25;
static const int mode_pody_offset                        = 26;
static const int mode_podn_offset                        = 27;
static const int mode_pofd_offset                        = 28;
static const int mode_far_offset                         = 29;
static const int mode_csi_offset                         = 30;
static const int mode_gss_offset                         = 31;
static const int mode_hk_offset                          = 32;
static const int mode_hss_offset                         = 33;
static const int mode_odds_offset                        = 34;

////////////////////////////////////////////////////////////////////////

#endif   /*  __MODE_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
