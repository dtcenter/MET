// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
   "FCST_LEV",                   //  17
   "OBS_VAR",                    //  18
   "OBS_LEV",                    //  19

   "OBTYPE",                     //  20

};

static const string mode_obj_columns [] = {

   "OBJECT_ID",                  //  21
   "OBJECT_CAT",                 //  22
   "CENTROID_X",                 //  23
   "CENTROID_Y",                 //  24
   "CENTROID_LAT",               //  25
   "CENTROID_LON",               //  26
   "AXIS_ANG",                   //  27
   "LENGTH",                     //  28
   "WIDTH",                      //  29
   "AREA",                       //  30
   "AREA_THRESH",                //  31
   "CURVATURE",                  //  32
   "CURVATURE_X",                //  33
   "CURVATURE_Y",                //  34
   "COMPLEXITY",                 //  35
   "INTENSITY_10",               //  36
   "INTENSITY_25",               //  37
   "INTENSITY_50",               //  38
   "INTENSITY_75",               //  39
   "INTENSITY_90",               //  40
   "INTENSITY_USER",             //  41
   "INTENSITY_SUM",              //  42
   "CENTROID_DIST",              //  43
   "BOUNDARY_DIST",              //  44
   "CONVEX_HULL_DIST",           //  45
   "ANGLE_DIFF",                 //  46
   "ASPECT_DIFF",                //  47
   "AREA_RATIO",                 //  48
   "INTERSECTION_AREA",          //  49
   "UNION_AREA",                 //  50
   "SYMMETRIC_DIFF",             //  51
   "INTERSECTION_OVER_AREA",     //  52
   "CURVATURE_RATIO",            //  53
   "COMPLEXITY_RATIO",           //  54
   "PERCENTILE_INTENSITY_RATIO", //  55
   "INTEREST"                    //  56
};

static const int n_mode_single_columns = 20; // CENTROID_X thru INTENSITY_SUM
static const int n_mode_pair_columns   = 14; // CENTROID_DIST thru INTEREST

static const char * mode_cts_columns [] = {

   "FIELD",                      //  21
   "TOTAL",                      //  22
   "FY_OY",                      //  23
   "FY_ON",                      //  24
   "FN_OY",                      //  25
   "FN_ON",                      //  26
   "BASER",                      //  27
   "FMEAN",                      //  28
   "ACC",                        //  29
   "FBIAS",                      //  30
   "PODY",                       //  31
   "PODN",                       //  32
   "POFD",                       //  33
   "FAR",                        //  34
   "CSI",                        //  35
   "GSS",                        //  36
   "HK",                         //  37
   "HSS",                        //  38
   "ODDS"                        //  39
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
