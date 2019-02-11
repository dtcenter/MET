// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   polyline.cc
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-03-06  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "polyline.h"
#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"
#include "grid_closed_poly.h"


///////////////////////////////////////////////////////////////////////////////
//
// Code for class GridClosedPoly
//
///////////////////////////////////////////////////////////////////////////////


GridClosedPoly::GridClosedPoly()

{

gcp_init_from_scratch();

}


///////////////////////////////////////////////////////////////////////////////


GridClosedPoly::~GridClosedPoly()

{

}


///////////////////////////////////////////////////////////////////////////////


void GridClosedPoly::clear()

{

Polyline::clear();

u_min = u_max = v_min = v_max = 0.0;

return;

}


///////////////////////////////////////////////////////////////////////////////


GridClosedPoly::GridClosedPoly(const GridClosedPoly & g)

{

gcp_init_from_scratch();

gcp_assign(g);

}


///////////////////////////////////////////////////////////////////////////////


GridClosedPoly & GridClosedPoly::operator=(const GridClosedPoly & g)

{

if ( this == &g )  return ( * this );

gcp_assign(g);


return ( * this );

}


///////////////////////////////////////////////////////////////////////////////


void GridClosedPoly::gcp_init_from_scratch()

{

u_min = u_max = v_min = v_max = 0.0;

return;

}


///////////////////////////////////////////////////////////////////////////////


void GridClosedPoly::gcp_assign(const GridClosedPoly & g)

{

Polyline::assign(g);

u_min = g.u_min;
u_max = g.u_max;

v_min = g.v_min;
v_max = g.v_max;

return;

}


///////////////////////////////////////////////////////////////////////////////


void GridClosedPoly::add_point(double uu, double vv)

{

if ( n_points == 0 )  {

   u_min = u_max = uu;  v_min = v_max = vv;

} else {

   if ( uu < u_min )  u_min = uu;
   if ( uu > u_max )  u_max = uu;

   if ( vv < v_min )  v_min = vv;
   if ( vv > v_max )  v_max = vv;

}

Polyline::add_point(uu, vv);


return;

}


///////////////////////////////////////////////////////////////////////////////


int GridClosedPoly::is_inside(double u_test, double v_test) const

{

   //
   //  test bounding box
   //

if ( u_test < u_min )  return ( 0 );
if ( u_test > u_max )  return ( 0 );

if ( v_test < v_min )  return ( 0 );
if ( v_test > v_max )  return ( 0 );

   //
   //  test polyline
   //

const int status = Polyline::is_inside(u_test, v_test);

return ( status );

}


///////////////////////////////////////////////////////////////////////////////
//
// Code for class GridClosedPolyArray
//
///////////////////////////////////////////////////////////////////////////////


bool GridClosedPolyArray::is_inside(double u_test, double v_test) const

{

if ( Nelements == 0 )  return ( false );

int j, status;

   //
   //  if the test point is inside **ANY** of the element polylines, return true
   //

for (j=0; j<Nelements; ++j)  {

   status = e[j]->is_inside(u_test, v_test);

   if ( status != 0 )  return ( true );

}


return ( false );

}


///////////////////////////////////////////////////////////////////////////////


void GridClosedPolyArray::set(const ShpPolyRecord & r, const Grid & grid)

{

clear();

int i, j, k, m;
int start, stop;
double lat, lon;
double x, y;
GridClosedPoly g;


if ( r.n_parts == 0 )  return;


for (j=0; j<(r.n_parts); ++j)  {

   g.clear();

   start = r.start_index(j);

   stop  = r.stop_index(j);

   i = stop - start + 1;

   // g.extend(stop - start + 1);

   for (k=0; k<i; ++k)  {

      m = start + k;

      lat = r.lat(m);
      lon = r.lon(m);

      grid.latlon_to_xy(lat, lon, x, y);

      g.add_point(x, y);

   }   //  for k

}   //  for j


return;

}


///////////////////////////////////////////////////////////////////////////////



