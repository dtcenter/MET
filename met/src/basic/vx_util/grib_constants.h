// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef __GRIB_CONSTANTS_H__
#define __GRIB_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////

//
// Grib code for precipiation types
//
static const int prate_grib_code = 59;
static const int tstm_grib_code  = 60;
static const int apcp_grib_code  = 61;
static const int ncpcp_grib_code = 62;
static const int acpcp_grib_code = 63;

//
// Grib codes corresponding to P, Q, T, Z, U, V variables
//
static const int pres_grib_code = 1;  // Pressure
static const int spfh_grib_code = 51; // Specific Humidity
static const int tmp_grib_code  = 11; // Temperature
static const int hgt_grib_code  = 7;  // Geopotential Height
static const int ugrd_grib_code = 33; // U-component of wind
static const int vgrd_grib_code = 34; // V-component of wind

//
// Grib codes for quantities that can be derived from
// the P, Q, T, Z, U, V variables
//
static const int dpt_grib_code   = 17; // Dewpoint temperature
static const int wdir_grib_code  = 31; // Wind direction
static const int wind_grib_code  = 32; // Wind speed
static const int rh_grib_code    = 52; // Relative humidity
static const int mixr_grib_code  = 53; // Humidity mixing ratio
static const int prmsl_grib_code = 2;  // Pressure Reduced to MSL

////////////////////////////////////////////////////////////////////////

bool is_precip_grib_code(int);

////////////////////////////////////////////////////////////////////////

#endif   //  __GRIB_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////

