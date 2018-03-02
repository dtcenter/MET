// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
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

static const int n_mode_single_columns = 20; // CENTROID_X thru INTENSITY_SUM
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
