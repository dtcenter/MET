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
   "DESC",                       //   3
   "FCST_LEAD",                  //   4
   "FCST_VALID",                 //   5
   "FCST_ACCUM",                 //   6
   "OBS_LEAD",                   //   7
   "OBS_VALID",                  //   8
   "OBS_ACCUM",                  //   9
   "FCST_RAD",                   //  10
   "FCST_THR",                   //  11
   "OBS_RAD",                    //  12
   "OBS_THR",                    //  13
   "FCST_VAR",                   //  14
   "FCST_LEV",                   //  15
   "OBS_VAR",                    //  16
   "OBS_LEV"                     //  17
};

static const char * mode_obj_columns [] = {

   "OBJECT_ID",                  //  18
   "OBJECT_CAT",                 //  19
   "CENTROID_X",                 //  20
   "CENTROID_Y",                 //  21
   "CENTROID_LAT",               //  22
   "CENTROID_LON",               //  23
   "AXIS_ANG",                   //  24
   "LENGTH",                     //  25
   "WIDTH",                      //  26
   "AREA",                       //  27
   "AREA_FILTER",                //  28
   "AREA_THRESH",                //  29
   "CURVATURE",                  //  30
   "CURVATURE_X",                //  31
   "CURVATURE_Y",                //  32
   "COMPLEXITY",                 //  33
   "INTENSITY_10",               //  34
   "INTENSITY_25",               //  35
   "INTENSITY_50",               //  36
   "INTENSITY_75",               //  37
   "INTENSITY_90",               //  38
   "INTENSITY_USER",             //  39
   "INTENSITY_SUM",              //  40
   "CENTROID_DIST",              //  41
   "BOUNDARY_DIST",              //  42
   "CONVEX_HULL_DIST",           //  43
   "ANGLE_DIFF",                 //  44
   "AREA_RATIO",                 //  45
   "INTERSECTION_AREA",          //  46
   "UNION_AREA",                 //  47
   "SYMMETRIC_DIFF",             //  48
   "INTERSECTION_OVER_AREA",     //  49
   "COMPLEXITY_RATIO",           //  50
   "PERCENTILE_INTENSITY_RATIO", //  51
   "INTEREST"                    //  52
};

static const char * mode_cts_columns [] = {

   "FIELD",                      //  18
   "TOTAL",                      //  19
   "FY_OY",                      //  20
   "FY_ON",                      //  21
   "FN_OY",                      //  22
   "FN_ON",                      //  23
   "BASER",                      //  24
   "FMEAN",                      //  25
   "ACC",                        //  26
   "FBIAS",                      //  27
   "PODY",                       //  28
   "PODN",                       //  29
   "POFD",                       //  30
   "FAR",                        //  31
   "CSI",                        //  32
   "GSS",                        //  33
   "HK",                         //  34
   "HSS",                        //  35
   "ODDS"                        //  36
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
static const int mode_desc_offset                        =  2;
static const int mode_fcst_lead_offset                   =  3;
static const int mode_fcst_valid_offset                  =  4;
static const int mode_fcst_accum_offset                  =  5;
static const int mode_obs_lead_offset                    =  6;
static const int mode_obs_valid_offset                   =  7;
static const int mode_obs_accum_offset                   =  8;
static const int mode_fcst_rad_offset                    =  9;
static const int mode_fcst_thr_offset                    = 10;
static const int mode_obs_rad_offset                     = 11;
static const int mode_obs_thr_offset                     = 12;
static const int mode_fcst_var_offset                    = 13;
static const int mode_fcst_lev_offset                    = 14;
static const int mode_obs_var_offset                     = 15;
static const int mode_obs_lev_offset                     = 16;

//
// MODE Object column offsets
//
static const int mode_object_id_offset                   = 17;
static const int mode_object_cat_offset                  = 18;
static const int mode_centroid_x_offset                  = 19;
static const int mode_centroid_y_offset                  = 20;
static const int mode_centroid_lat_offset                = 21;
static const int mode_centroid_lon_offset                = 22;
static const int mode_axis_ang_offset                    = 23;
static const int mode_length_offset                      = 24;
static const int mode_width_offset                       = 25;
static const int mode_area_offset                        = 26;
static const int mode_area_filter_offset                 = 27;
static const int mode_area_thresh_offset                 = 28;
static const int mode_curvature_offset                   = 29;
static const int mode_curvature_x_offset                 = 30;
static const int mode_curvature_y_offset                 = 31;
static const int mode_complexity_offset                  = 32;
static const int mode_intensity_10_offset                = 33;
static const int mode_intensity_25_offset                = 34;
static const int mode_intensity_50_offset                = 35;
static const int mode_intensity_75_offset                = 36;
static const int mode_intensity_90_offset                = 37;
static const int mode_intensity_user_offset              = 38;
static const int mode_intensity_sum_offset               = 39;
static const int mode_centroid_dist_offset               = 40;
static const int mode_boundary_dist_offset               = 41;
static const int mode_convex_hull_dist_offset            = 42;
static const int mode_angle_diff_offset                  = 43;
static const int mode_area_ratio_offset                  = 44;
static const int mode_intersection_area_offset           = 45;
static const int mode_union_area_offset                  = 46;
static const int mode_symmetric_diff_offset              = 47;
static const int mode_intersection_over_area_offset      = 48;
static const int mode_complexity_ratio_offset            = 49;
static const int mode_percentile_intensity_ratio_offset  = 50;
static const int mode_interest_offset                    = 51;

//
// MODE Contingecy table column offsets
//
static const int mode_field_offset                       = 17;
static const int mode_total_offset                       = 18;
static const int mode_fy_oy_offset                       = 19;
static const int mode_fy_on_offset                       = 20;
static const int mode_fn_oy_offset                       = 21;
static const int mode_fn_on_offset                       = 22;
static const int mode_baser_offset                       = 23;
static const int mode_fmean_offset                       = 24;
static const int mode_acc_offset                         = 25;
static const int mode_fbias_offset                       = 26;
static const int mode_pody_offset                        = 27;
static const int mode_podn_offset                        = 28;
static const int mode_pofd_offset                        = 29;
static const int mode_far_offset                         = 30;
static const int mode_csi_offset                         = 31;
static const int mode_gss_offset                         = 32;
static const int mode_hk_offset                          = 33;
static const int mode_hss_offset                         = 34;
static const int mode_odds_offset                        = 35;

////////////////////////////////////////////////////////////////////////

#endif   /*  __MODE_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
