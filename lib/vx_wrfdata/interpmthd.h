// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   interpmthd.h
//
//   Description:
//      Contains the declaration of the FilterBox class.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11-17-08  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __WRFDATA_INTERPMTHD_H__
#define  __WRFDATA_INTERPMTHD_H__

///////////////////////////////////////////////////////////////////////////////

using namespace std;

///////////////////////////////////////////////////////////////////////////////

//
// Enumeration for interpolation methods
//
enum InterpMthd {
   im_na      = 0,
   im_min     = 1,
   im_max     = 2,
   im_median  = 3,
   im_uw_mean = 4,
   im_dw_mean = 5,
   im_ls_fit  = 6,
   im_nbrhd   = 7,
   im_bilin   = 8
};

//
// String corresponding to the enumerated values above
//
static const char im_na_str[]      = "NA";
static const char im_min_str[]     = "MIN";
static const char im_max_str[]     = "MAX";
static const char im_median_str[]  = "MEDIAN";
static const char im_uw_mean_str[] = "UW_MEAN";
static const char im_dw_mean_str[] = "DW_MEAN";
static const char im_ls_fit_str[]  = "LS_FIT";
static const char im_nbrhd_str[]   = "NBRHD";
static const char im_bilin_str[]   = "BILIN";

///////////////////////////////////////////////////////////////////////////////

extern void       interpmthd_to_string(const InterpMthd, char *);
extern InterpMthd string_to_interpmthd(const char *);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __WRFDATA_INTERPMTHD_H__

///////////////////////////////////////////////////////////////////////////////
