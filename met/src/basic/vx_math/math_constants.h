// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef __MATH_CONSTANTS_H__
#define __MATH_CONSTANTS_H__


////////////////////////////////////////////////////////////////////////


#include "trig.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Constants
   //

static const double earth_radius_km = 6378.140;

static const double pi              = 4.0*atan(1.0);
static const double piover4         = atan(1.0);
static const double piover2         = 2.0*atan(1.0);
static const double twopi           = 8.0*atan(1.0);

static const double flattening      = 1.0/298.257;

static const int standard_time_zone = 7;   //  mountain standard time

static const double vx_math_e       = exp(1.0);

   //
   //  Conversion factors
   //

static const double meters_per_foot       = 0.3048;
static const double feet_per_meter        = 1.0/meters_per_foot;

static const double mps_per_knot          = 0.514444;  // mps = meters per second
static const double knots_per_mps         = 1.0/mps_per_knot;

static const double km_per_nautical_mile  = (twopi*earth_radius_km)/(360.0*60.0);
static const double nautical_miles_per_km = 1.0/km_per_nautical_mile;

static const double tc_km_per_nautical_miles = 1.8520;
static const double tc_nautical_miles_per_km = 1.0/tc_km_per_nautical_miles;

static const double nautical_miles_per_deg = 60.0;
static const double deg_per_nautical_mile  = 1.0/nautical_miles_per_deg;

static const double km_per_statute_mile   = 1.609344;
static const double statute_miles_per_km  = 1.0/km_per_statute_mile;

static const double statute_miles_per_nautical_mile = statute_miles_per_km*km_per_nautical_mile;
static const double nautical_miles_per_statute_mile = 1.0/statute_miles_per_nautical_mile;

static const int    sec_per_minute        = 60;
static const int    sec_per_hour          = 3600;
static const int    sec_per_day           = 86400;

static const double k_to_c                = -273.15;
static const double c_to_k                =  273.15;

static const double mg_per_kg             = 1.0e6;
static const double kg_per_mg             = 1.0/mg_per_kg;

static const double pa_per_mb             = 100.0;
static const double mb_per_pa             = 1.0/pa_per_mb;

static const double m_per_km              = 1000.0;
static const double km_per_m              = 1.0/m_per_km;

   //
   //  Default tolerance to determine if two number of close enough
   //  to be considered equal
   //

static const double tight_tol             = 10E-10;
static const double loose_tol             = 10E-5;
static const double default_tol           = tight_tol;

////////////////////////////////////////////////////////////////////////


#endif   //  __MATH_CONSTANTS_H__


////////////////////////////////////////////////////////////////////////


