// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef __NC_CONSTANTS_H__
#define __NC_CONSTANTS_H__


////////////////////////////////////////////////////////////////////////


#include "math_constants.h"


////////////////////////////////////////////////////////////////////////
//
//  NetCDF keywords expected to be used in the configuration file.
//
////////////////////////////////////////////////////////////////////////


// NetCDF keywords
static const char * const CONFIG_NetCDF_Dimension = "NetCDF_Dimension";

// Flag value used to indicate a range of values within a dimension
static const int range_flag = bad_data_int;


////////////////////////////////////////////////////////////////////////


#endif   //  __NC_CONSTANTS_H__


////////////////////////////////////////////////////////////////////////


