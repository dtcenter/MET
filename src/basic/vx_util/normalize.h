// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef  __NORMALIZE_H__
#define  __NORMALIZE_H__

///////////////////////////////////////////////////////////////////////////////

#include "concat_string.h"
#include "data_plane.h"

///////////////////////////////////////////////////////////////////////////////

//
// Enumeration for normalization types
//

enum class NormalizeType {
   None,         // No normalization
   ClimoAnom,    // Subtract climo mean
   ClimoStdAnom, // Subtract climo mean and divide stdev
   FcstAnom,     // Subtract fcst mean
   FcstStdAnom   // Subtract fcst mean and divide stdev
};

///////////////////////////////////////////////////////////////////////////////

//
// String corresponding to the enumerated values above
//
static const char normalizetype_none_str[]           = "NONE";
static const char normalizetype_climo_anom_str[]     = "CLIMO_ANOM";
static const char normalizetype_climo_std_anom_str[] = "CLIMO_STD_ANOM";
static const char normalizetype_fcst_anom_str[]      = "FCST_ANOM";
static const char normalizetype_fcst_std_anom_str[]  = "FCST_STD_ANOM";

///////////////////////////////////////////////////////////////////////////////

extern ConcatString normalizetype_to_string(const NormalizeType);

extern void normalize_data(DataPlane &, const NormalizeType,
                           const DataPlane *, const DataPlane *,
                           const DataPlane *, const DataPlane *);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __NORMALIZE_H__

///////////////////////////////////////////////////////////////////////////////
