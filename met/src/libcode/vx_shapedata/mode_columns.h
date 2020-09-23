// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MODE_COLUMNS_H__
#define  __MODE_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

static const string mode_hdr_columns [] = {

   "VERSION",                    //   1
   "MODEL",                      //   2

   "N_VALID",                    //   3
   "GRID_RES",                   //   4

   "DESC",                       //   5
   "FCST_LEAD",                  //   6
   "FCST_VALID",                 //   7
   "FCST_ACCUM",                 //   8
   "OBS_LEAD",                   //   9
   "OBS_VALID",                  //  10
   "OBS_ACCUM",                  //  11
   "FCST_RAD",                   //  12
   "FCST_THR",                   //  13
   "OBS_RAD",                    //  14
   "OBS_THR",                    //  15
   "FCST_VAR",                   //  16
   "FCST_UNITS",                 //  17
   "FCST_LEV",                   //  18
   "OBS_VAR",                    //  19
   "OBS_UNITS",                  //  20
   "OBS_LEV",                    //  21

   "OBTYPE",                     //  22

};

static const string mode_obj_columns [] = {

   "OBJECT_ID",                  //  23
   "OBJECT_CAT",                 //  24
   "CENTROID_X",                 //  25
   "CENTROID_Y",                 //  26
   "CENTROID_LAT",               //  27
   "CENTROID_LON",               //  28
   "AXIS_ANG",                   //  29
   "LENGTH",                     //  30
   "WIDTH",                      //  31
   "AREA",                       //  32
   "AREA_THRESH",                //  33
   "CURVATURE",                  //  34
   "CURVATURE_X",                //  35
   "CURVATURE_Y",                //  36
   "COMPLEXITY",                 //  37
   "INTENSITY_10",               //  38
   "INTENSITY_25",               //  39
   "INTENSITY_50",               //  40
   "INTENSITY_75",               //  41
   "INTENSITY_90",               //  42
   "INTENSITY_USER",             //  43
   "INTENSITY_SUM",              //  44
   "CENTROID_DIST",              //  45
   "BOUNDARY_DIST",              //  46
   "CONVEX_HULL_DIST",           //  47
   "ANGLE_DIFF",                 //  48
   "ASPECT_DIFF",                //  49
   "AREA_RATIO",                 //  50
   "INTERSECTION_AREA",          //  51
   "UNION_AREA",                 //  52
   "SYMMETRIC_DIFF",             //  53
   "INTERSECTION_OVER_AREA",     //  54
   "CURVATURE_RATIO",            //  55
   "COMPLEXITY_RATIO",           //  56
   "PERCENTILE_INTENSITY_RATIO", //  57
   "INTEREST"                    //  58
};

static const int n_mode_single_columns = 20; // CENTROID_X thru INTENSITY_SUM
static const int n_mode_pair_columns   = 14; // CENTROID_DIST thru INTEREST

static const char * mode_cts_columns [] = {

   "FIELD",                      //  23
   "TOTAL",                      //  24
   "FY_OY",                      //  25
   "FY_ON",                      //  26
   "FN_OY",                      //  27
   "FN_ON",                      //  28
   "BASER",                      //  29
   "FMEAN",                      //  30
   "ACC",                        //  31
   "FBIAS",                      //  32
   "PODY",                       //  33
   "PODN",                       //  34
   "POFD",                       //  35
   "FAR",                        //  36
   "CSI",                        //  37
   "GSS",                        //  38
   "HK",                         //  39
   "HSS",                        //  40
   "ODDS"                        //  41
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
