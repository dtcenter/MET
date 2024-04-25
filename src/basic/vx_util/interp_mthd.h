// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef  __INTERP_MTHD_H__
#define  __INTERP_MTHD_H__

///////////////////////////////////////////////////////////////////////////////

#include "concat_string.h"

///////////////////////////////////////////////////////////////////////////////

//
// Enumeration for interpolation methods
//
enum class InterpMthd {
   None,
   Min,
   Max,
   Median,
   UW_Mean,
   DW_Mean,
   AW_Mean,
   LS_Fit,
   Nbrhd,
   Bilin,
   Nearest,
   Budget,
   Force,
   Best,
   Upper_Left,
   Upper_Right,
   Lower_Right,
   Lower_Left,
   Gaussian,
   MaxGauss,
   Geog_Match,
   HiRA
};

//
// String corresponding to the enumerated values above
//
static const char interpmthd_none_str[]        = "NONE";
static const char interpmthd_min_str[]         = "MIN";
static const char interpmthd_max_str[]         = "MAX";
static const char interpmthd_median_str[]      = "MEDIAN";
static const char interpmthd_uw_mean_str[]     = "UW_MEAN";
static const char interpmthd_dw_mean_str[]     = "DW_MEAN";
static const char interpmthd_aw_mean_str[]     = "AW_MEAN";
static const char interpmthd_ls_fit_str[]      = "LS_FIT";
static const char interpmthd_nbrhd_str[]       = "NBRHD";
static const char interpmthd_bilin_str[]       = "BILIN";
static const char interpmthd_nearest_str[]     = "NEAREST";
static const char interpmthd_budget_str[]      = "BUDGET";
static const char interpmthd_force_str[]       = "FORCE";
static const char interpmthd_best_str[]        = "BEST";
static const char interpmthd_upper_left_str[]  = "UPPER_LEFT";
static const char interpmthd_upper_right_str[] = "UPPER_RIGHT";
static const char interpmthd_lower_right_str[] = "LOWER_RIGHT";
static const char interpmthd_lower_left_str[]  = "LOWER_LEFT";
static const char interpmthd_gaussian_str[]    = "GAUSSIAN";
static const char interpmthd_maxgauss_str[]    = "MAXGAUSS";
static const char interpmthd_geog_match_str[]  = "GEOG_MATCH";
static const char interpmthd_hira_str[]        = "HIRA";

///////////////////////////////////////////////////////////////////////////////

extern ConcatString interpmthd_to_string(const InterpMthd);
extern InterpMthd   string_to_interpmthd(const char *);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __INTERP_MTHD_H__

///////////////////////////////////////////////////////////////////////////////
