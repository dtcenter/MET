// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   filterbox.cc
//
//   Description:
//      Contains the definition of the FilterBox class.
//
//   Mod#   Date      Name      Description
//   ----   ----      ----      -----------
//   000   07-15-04   Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "filterbox.h"
#include "vx_log.h"
#include "vx_math.h"

///////////////////////////////////////////////////////////////////////////////

extern int nint(double);

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class FilterBox
//
///////////////////////////////////////////////////////////////////////////////

FilterBox::FilterBox() {

   z = (double *) 0;

   on = (int *) 0;

   xmin = xmax = ymin = ymax = 0;

   nx = ny = 0;
}

///////////////////////////////////////////////////////////////////////////////

FilterBox::~FilterBox() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

FilterBox::FilterBox(const FilterBox &f) {

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

FilterBox & FilterBox::operator=(const FilterBox &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::clear() {

   if ( z )  { delete [] z;  z = (double *) 0; }

   if ( on )  { delete [] on;  on = (int *) 0; }

   xmin = xmax = ymin = ymax = 0;

   nx = ny = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::alloc(int n) {

   if ( z )  { delete [] z;  z = (double *) 0; }

   if ( on )  { delete [] on;  on = (int *) 0; }

   z = new double [n];

   on = new int [n];

   if ( !z || !on )  {
      mlog << Error << "\n\nERROR: FilterBox::alloc() -> "
	   << "memory allocation error\n\n";

      exit ( 1 );
   }

   memset( z, n, 0);
   memset(on, n, 0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::assign(const FilterBox &f) {

   clear();

   xmax = f.xmax;
   xmin = f.xmin;

   ymax = f.ymax;
   ymin = f.ymin;

   nx = f.nx;
   ny = f.ny;

   alloc(nx*ny);

   memcpy(z, f.z, nx*ny*sizeof(double));

   memcpy(on, f.on, nx*ny*sizeof(int));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::set_cylinder_volume(int radius, double volume) {
   double height;

   height = volume/(pi*radius*radius);

   set_cylinder_height(radius, height);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::set_cylinder_height(int radius, double height) {
   int x, y, n;
   double dx, dy, dist;

   clear();

   xmin = ymin = -radius;
   xmax = ymax =  radius;

   nx = xmax - xmin + 1;
   ny = ymax - ymin + 1;

   alloc(nx*ny);

   for (x=xmin; x<=xmax; ++x) {
      for (y=ymin; y<=ymax; ++y)  {

         n = two_to_one(x, y);

         dx = x - 0.5*(xmax + xmin);
         dy = y - 0.5*(ymax + ymin);

         dist = sqrt( dx*dx + dy*dy );

         on[n] = 0;
         z[n] = 0;

         if ( dist <= radius ) {
            on[n] = 1;
            z[n] = height;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::set_cone(int radius, int height) {
   int x, y, n;
   double dx, dy, dist;

   clear();

   xmin = ymin = -radius;
   xmax = ymax =  radius;

   nx = xmax - xmin + 1;
   ny = ymax - ymin + 1;

   alloc(nx*ny);

   for (x=xmin; x<=xmax; ++x)  {
      for (y=ymin; y<=ymax; ++y)  {

         n = two_to_one(x, y);

         dx = (double) (x - xmin);
         dy = (double) (y - ymin);

         dist = sqrt( dx*dx + dy*dy );

         on[n] = 0;
         z[n] = 0;

         if ( dist <= radius ) {
            on[n] = 1;
            z[n] = nint( height*(1.0 - dist/radius) );
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::set_box_volume(int x_half_side, int y_half_side,
			       double volume) {
   double height;

   height = volume/((2.0*x_half_side) * (2.0*y_half_side));

   set_box_height(x_half_side, y_half_side, height);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FilterBox::set_box_height(int x_half_side, int y_half_side,
			       double height) {
   int x, y, n;

   clear();

   xmin = -x_half_side;
   ymin = -y_half_side;
   xmax = x_half_side;
   ymax = y_half_side;

   nx = xmax - xmin + 1;
   ny = ymax - ymin + 1;

   alloc(nx*ny);

   for (x=xmin; x<=xmax; ++x) {
      for (y=ymin; y<=ymax; ++y)  {

         n = two_to_one(x, y);
         on[n] = 1;
         z[n] = height;
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  End Code for class FilterBox
//
///////////////////////////////////////////////////////////////////////////////
