// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __UTIL_CONSTANTS_H__
#define  __UTIL_CONSTANTS_H__

#if HAVE_CONFIG_H
# include "config.h"
#endif

////////////////////////////////////////////////////////////////////////

// Released versions of MET
static const char met_version_9_1[]   = "V9.1";
static const char met_version_9_0[]   = "V9.0";
static const char met_version_8_1[]   = "V8.1";
static const char met_version_8_0[]   = "V8.0";
static const char met_version_7_0[]   = "V7.0";
static const char met_version_6_1[]   = "V6.1";
static const char met_version_6_0[]   = "V6.0";
static const char met_version_5_2[]   = "V5.2";
static const char met_version_5_1[]   = "V5.1";
static const char met_version_5_0[]   = "V5.0";
static const char met_version_4_1[]   = "V4.1";
static const char met_version_4_0[]   = "V4.0";
static const char met_version_3_1[]   = "V3.1";
static const char met_version_3_0_1[] = "V3.0.1";
static const char met_version_3_0[]   = "V3.0";
static const char met_version_2_0[]   = "V2.0";
static const char met_version_1_1[]   = "V1.1";

////////////////////////////////////////////////////////////////////////

static const char * const met_version    = met_version_9_1;
static const char default_met_data_dir[] = "MET_BASE";
static const char txt_file_ext[]         = ".txt";
static const char stat_file_ext[]        = ".stat";
static const char tc_stat_file_ext[]     = ".tcst";
static const char full_domain_str[]      = "FULL";

////////////////////////////////////////////////////////////////////////

static const char latlon_proj_type         [] = "LatLon";
static const char rotated_latlon_proj_type [] = "Rotated LatLon";
static const char mercator_proj_type       [] = "Mercator";
static const char lambert_proj_type        [] = "Lambert Conformal";
static const char stereographic_proj_type  [] = "Polar Stereographic";
static const char gaussian_proj_type       [] = "Gaussian";

// String containing all valid PrepBufr message types
static const char vld_msg_typ_str[] =
   "ADPUPA AIRCAR AIRCFT ADPSFC ERS1DA GOESND GPSIPW MSONET \
    PROFLR QKSWND RASSDA SATEMP SATWND SFCBOG SFCSHP SPSSMI \
    SYNDAT VADWND ANYAIR ANYSFC ONLYSF ASCATW WDSATR";

// Array of valid PrepBufr message types
static const char *vld_msg_typ_list[] = {
   "ADPUPA", "AIRCAR", "AIRCFT", "ADPSFC", "ERS1DA", "GOESND",
   "GPSIPW", "MSONET", "PROFLR", "QKSWND", "RASSDA", "SATEMP",
   "SATWND", "SFCBOG", "SFCSHP", "SPSSMI", "SYNDAT", "VADWND",
   "ASCATW", "WDSATR"  };

// Number of valid PrepBufr message types
static const int n_vld_msg_typ =
   sizeof(vld_msg_typ_list)/sizeof(*vld_msg_typ_list);

// Message type group names
static const char surface_msg_typ_group_str [] = "SURFACE"; // Surface message type group
static const char landsf_msg_typ_group_str  [] = "LANDSF";  // Surface land message type group
static const char watersf_msg_typ_group_str [] = "WATERSF"; // Surface water message type group

// Commonly used regular expressions
static const char yyyymmdd_hhmmss_reg_exp[] =
   "[0-9]\\{8,8\\}_[0-9]\\{6,6\\}";
static const char ws_reg_exp[] = "[ \t\r\n]";
static const char ws_line_reg_exp[] = "^[ \t\r\n]*$";
static const char sep_str[] = "--------------------------------------------------------------------------------";

// Bootstrap methods
static const int boot_bca_flag  = 0;
static const int boot_perc_flag = 1;

////////////////////////////////////////////////////////////////////////

static const int max_line_len = 2048;
static const double grib_earth_radius_km = 6371.20;
static const int default_nc_compression = 0;
static const int default_precision = 5;
static const double default_grid_weight = 1.0;
static const char default_tmp_dir[] = "/tmp";

////////////////////////////////////////////////////////////////////////

// Constants used in deriving observation values
static const double const_a     = 1.0/273.15;
static const double const_b     = 2500000.0/461.5;
static const double const_c     = 6.11;

// Constants used in deriving RH
static const double const_pq0   = 379.90516;
static const double const_a2    = 17.2693882;
static const double const_a3    = 273.16;
static const double const_a4    = 35.86;

// Constants used in deriving PRMSL
static const double const_tmb   = 288.15;  // kelvin
static const double const_gop   = 9.80616; // from The Ceaseless Wind
static const double const_rd    = 287.0;   // kg/k dry gas constant

static const int vx_data2d_star = -12345;

////////////////////////////////////////////////////////////////////////

// Define isatty() function, if needed.  It used used by lex but is
// not defined on some systems
#ifndef HAVE_ISATTY
   inline int isatty(int fd) { return 0; }
#endif

////////////////////////////////////////////////////////////////////////

#endif   //  __UTIL_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////
