

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __GRID_FIND_GRID_BY_NAME_H__
#define  __GRID_FIND_GRID_BY_NAME_H__


////////////////////////////////////////////////////////////////////////


#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


//
// Radius of the earth in km as defined in the NCEP w3 library, which
// differs from the radius defined in the GRIB specification as
// 6367.47km.
//
static const double ncep_earth_radius_km = 6371.20;

static const bool west_longitude_positive = false;

////////////////////////////////////////////////////////////////////////


extern bool find_grid_by_name(const char *, Grid &);

extern bool find_grid_by_name(const char *, GridInfo &);

extern bool parse_grid_def(const StringArray &, Grid &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __GRID_FIND_GRID_BY_NAME_H__  */


////////////////////////////////////////////////////////////////////////




