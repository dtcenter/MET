// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef __GRIB_STRINGS_H__
#define __GRIB_STRINGS_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

static const string missing_str        = "MISSING";
static const char ugrd_abbr_str[]      = "UGRD";
static const char vgrd_abbr_str[]     = "VGRD";
static const string ugrd_vgrd_abbr_str = "UGRD_VGRD";
static const string wind_abbr_str      = "WIND";
static const string wdir_abbr_str      = "WDIR";

////////////////////////////////////////////////////////////////////////

//
// List of GRIB1 precipitation variable names
//
static const char *grib_precipitation_abbr[] = {
   "PWAT",    // Code  54, Precipitable water, kg/m^2
   "PRATE",   // Code  59, Precipitation rate, kg/m^2/s
   "APCP",    // Code  61, Total precipitation, kg/m^2
   "NCPCP",   // Code  62, Large scale precipitation (non-conv.), kg/m^2
   "ACPCP",   // Code  63, Convective precipitation, kg/m^2
   "CPRAT",   // Code 214, Convective Precipitation rate, kg/m^2/s
   "RPRATE",  // Code 223, Rain Precipitation Rate, kg/m^2/s
   "SPRATE",  // Code 224, Snow Precipitation Rate, kg/m^2/s
   "FPRATE",  // Code 225, Freezing Rain Precipitation Rate, kg/m^2/s
   "IPRATE",  // Code 226, Ice Pellets Precipitation Rate, kg/m^2/s
   "PSPCP",   // Code 252, Pseudo-Precipitation, kg/m^2
   "LSPA",    // Code 154, Land Surface Precipitation Accumulation (LSPA), kg/m^2
   "ASNOW",   // Code 161, Frozen precipitation (e.g. snowfall), kg/m^2
   "ARAIN",   // Code 162, Liquid precipitation (rainfall), kg/m^2
   "ASNOW",   // Code 161, Frozen precipitation (e.g. snowfall), kg/m^2
   "ARAIN",   // Code 162, Liquid precipitation (rainfall), kg/m^2
   "APCPN",   // Code 202, Total precipitation (nearest grid point), kg/m^2
   "ACPCPN",  // Code 206, Convective precipitation (nearest grid point), kg/m^2
   "CPRAT"    // Code 214, Convective precip. rate, kg/m^2/s
};

//
// Number of GRIB1 precipitation variable names
//
static const int n_grib_precipitation_abbr =
                     sizeof(grib_precipitation_abbr)/
                    sizeof(*grib_precipitation_abbr);

////////////////////////////////////////////////////////////////////////

//
// List of GRIB1 specific humidity abbreviations
//
static const char *grib_specific_humidity_abbr[] = {
   "SPFH",  // Code  51, Specific humidity, kg/kgSpecific humidity, kg kg-1
   "QZ0",   // Code 195, Specific humidity at top of viscous sublayer, kg/kg
   "QMAX",  // Code 204, Maximum specific humidity at 2m, kg/kg
   "QMIN"   // Code 205, Minimum specific humidity at 2m, kg/kg
};

//
// Number of GRIB1 specific humidity abbreviations
//
static const int n_grib_specific_humidity_abbr =
                     sizeof(grib_specific_humidity_abbr)/
                    sizeof(*grib_specific_humidity_abbr);

////////////////////////////////////////////////////////////////////////
//
// NCEP Office Note 388 - Table 2
// Grib Code to Units and Abbreviations
//
// (1) GRIB Code
// (2) GRIB Code Name
// (3) GRIB Code Units
// (4) GRIB Code Abbreviation
//
////////////////////////////////////////////////////////////////////////

struct GribCodeData {
   int         code;
   const char *name;
   const char *unit;
   const char *abbr;
};

////////////////////////////////////////////////////////////////////////
//
// NCEP Office Note 388 - Table 3
// Level Types and Values
//
// (1) Level Value
// (2) Level Type:
//     0 -> no specific level
//     2 -> vertical level type
//     3 -> pressure level type
// (3) Level Flag:
//     Octets 11 and 12 in the PDS:
//     0 -> ignore them
//     1 -> combine them to form one number
//     2 -> octet 11 contains the beginning of a range and
//          octet 12 contains the end of a range
// (4) Level Name
// (5) Level Abbreviation
//
////////////////////////////////////////////////////////////////////////

struct GribLevelData {
   int         level;
   int         type;
   int         flag;
   const char *name;
   const char *abbr;
};

////////////////////////////////////////////////////////////////////////
//
// GRIB tables: http://www.nco.ncep.noaa.gov/pmb/docs/on388/table3.html
// Last Updated 05/11/2009
//
////////////////////////////////////////////////////////////////////////

static const GribLevelData grib_level_list[] = {
   {   1, 2, 0, "Ground or water surface", "SFC" },
   {   2, 0, 0, "Cloud base level", "CBL" },
   {   3, 0, 0, "Cloud top level", "CTL" },
   {   4, 0, 0, "Level of 0 deg (C) isotherm", "0DEG" },
   {   5, 0, 0, "Level of adiabatic condensation lifted from the surface", "ADCL" },
   {   6, 0, 0, "Maximum wind level", "MWSL" },
   {   7, 0, 0, "Tropopause", "TRO" },
   {   8, 0, 0, "Nominal top of atmosphere", "NTAT" },
   {   9, 0, 0, "Sea bottom", "SEAB" },
   {  20, 0, 2, "Isothermal level (temperature in 1/100 K in octets 11 and 12)", "TMPL" },
   { 100, 3, 1, "isobaric level", "ISBL" },
   { 101, 3, 2, "layer between two isobaric levels", "ISBY" },
   { 102, 2, 0, "mean sea level", "MSL" },
   { 103, 2, 1, "Specified altitude above MSL", "GPML" },
   { 104, 2, 2, "layer between two specified altitudes above MSL", "GPMY" },
   { 105, 2, 1, "specified height level above ground", "HTGL" },
   { 106, 2, 2, "layer between two specified height levels above ground", "HTGY" },
   { 107, 0, 1, "sigma level", "SIGL" },
   { 108, 0, 2, "layer between two sigma levels", "SIGY" },
   { 109, 0, 1, "Hybrid level", "HYBL" },
   { 110, 0, 2, "layer between two hybrid levels", "HYBY" },
   { 111, 2, 1, "depth below land surface", "DBLL" },
   { 112, 2, 2, "layer between two depths below land surface", "DBLY" },
   { 113, 0, 1, "isentropic (theta) level", "THEL" },
   { 114, 0, 2, "layer between two isentropic levels", "THEY" },
   { 115, 0, 1, "level at specified pressure difference from ground to level", "SPDL" },
   { 116, 0, 2, "layer between two levels at specified pressure difference from ground to level", "SPDY" },
   { 117, 0, 1, "potential vorticity(pv) surface", "PVL" },
   { 119, 0, 1, "NAM level", "NAML" },
   { 120, 0, 2, "layer between two NAM levels", "NAMY" },
   { 121, 0, 2, "layer between two isobaric surfaces (high precision)", "IBYH" },
   { 125, 2, 1, "specified height level above ground (high precision)", "HGLH" },
   { 126, 3, 1, "isobaric level", "ISBP" },
   { 128, 0, 2, "layer between two sigma levels (high precision)", "SGYH" },
   { 141, 0, 2, "layer between two isobaric surfaces (mixed precision)", "IBYM" },
   { 160, 2, 1, "depth below sea level", "DBSL" },
   { 200, 0, 0, "entire atmosphere (considered as a single layer)", "EATM" },
   { 201, 0, 0, "entire ocean (considered as a single layer)", "EOCN" },
   { 204, 0, 0, "Highest tropospheric freezing level", "HTFL" },
   { 206, 0, 0, "Grid scale cloud bottom level", "GCBL" },
   { 207, 0, 0, "Grid scale cloud top level", "GCTL" },
   { 209, 0, 0, "Boundary layer cloud bottom level", "BCBL" },
   { 210, 0, 0, "Boundary layer cloud top level", "BCTL" },
   { 211, 0, 0, "Boundary layer cloud layer", "BCY" },
   { 212, 0, 0, "Low cloud bottom level", "LCBL" },
   { 213, 0, 0, "Low cloud top level", "LCTL" },
   { 214, 0, 0, "Low cloud layer", "LCY" },
   { 215, 0, 0, "Cloud ceiling", "CEIL" },
   { 216, 0, 0, "Cumulonimbus Base (m)", "CBB" },
   { 217, 0, 0, "Cumulonimbus Top (m)", "CBT" },
   { 220, 0, 0, "Planetary Boundary Layer (derived from Richardson number)", "PBLRI" },
   { 222, 0, 0, "Middle cloud bottom level", "MCBL" },
   { 223, 0, 0, "Middle cloud top level", "MCTL" },
   { 224, 0, 0, "Middle cloud layer", "MCY" },
   { 232, 0, 0, "High cloud bottom level", "HCBL" },
   { 233, 0, 0, "High cloud top level", "HCTL" },
   { 234, 0, 0, "High cloud layer", "HCY" },
   { 235, 0, 0, "Ocean Isotherm Level (1/10 deg C)", "OITL" },
   { 236, 0, 2, "Layer between two depths below ocean surface", "OLYR" },
   { 237, 0, 0, "Bottom of Ocean Mixed Layer (m)", "OBML" },
   { 238, 0, 0, "Bottom of Ocean Isothermal Layer (m)", "OBIL" },
   { 239, 0, 0, "Layer Ocean Surface and 26C Ocean Isothermal Level", "S26CY" },
   { 240, 0, 0, "Ocean Mixed Layer", "OMXL" },
   { 241, 0, 0, "Ordered Sequence of Data", "OSEQD" },
   { 242, 0, 0, "Convective cloud bottom level", "CCBL" },
   { 243, 0, 0, "Convective cloud top level", "CCTL" },
   { 244, 0, 0, "Convective cloud layer", "CCY" },
   { 245, 0, 0, "Lowest level of the wet bulb zero", "LLTW" },
   { 246, 0, 0, "Maximum equivalent potential temperature level", "MTHE" },
   { 247, 0, 0, "Equilibrium level", "EHLT" },
   { 248, 0, 0, "Shallow convective cloud bottom level", "SCBL" },
   { 249, 0, 0, "Shallow convective cloud top level", "SCTL" },
   { 251, 0, 0, "Deep convective cloud bottom level", "DCBL" },
   { 252, 0, 0, "Deep convective cloud top level", "DCTL" },
   { 253, 0, 0, "Lowest bottom level of supercooled liquid water layer", "LBLSW" },
   { 254, 0, 0, "Highest top level of supercooled liquid water layer", "HTLSW" },
   { 255, 0, 0, "Missing", "NA" }
};
static const int n_grib_level_list = sizeof(grib_level_list)/sizeof(GribLevelData);

////////////////////////////////////////////////////////////////////////

ConcatString get_grib_code_name(int, int);
ConcatString get_grib_code_unit(int, int);
ConcatString get_grib_code_abbr(int, int);

ConcatString get_grib_level_name(int);
ConcatString get_grib_level_abbr(int);
ConcatString get_grib_level_str(int, unsigned char *);

int str_to_grib_code(const char *);
int str_to_grib_code(const char *, int);

int str_to_grib_code(const char *, int &, double &, double &);
int str_to_grib_code(const char *, int &, double &, double &, int);

int str_to_prob_info(const char *, double &, double &, int);

void get_grib_code_list(int, int, int &, const GribCodeData *&);
ConcatString get_grib_code_list_str(int, int, int);
ConcatString get_grib_level_list_str(int, int);

///////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////

#endif   //  __GRIB_STRINGS_H__

////////////////////////////////////////////////////////////////////////

