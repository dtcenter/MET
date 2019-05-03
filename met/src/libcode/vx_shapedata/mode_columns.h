// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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

static const int n_mode_single_columns = 21; // CENTROID_X thru INTENSITY_SUM
static const int n_mode_pair_columns   = 12; // CENTROID_DIST thru INTEREST

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

#endif   /*  __MODE_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
