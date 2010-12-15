// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "boundingbox.h"

////////////////////////////////////////////////////////////////////////

static bool is_inside_bb(const BoundingBox &, double, double);
static bool is_between(double, double, double);

////////////////////////////////////////////////////////////////////////

bool bb_intersect(const BoundingBox &b1, const BoundingBox &b2) {
   bool intersect = false;

   //
   // Check if the four corners of box 1 are inside box 2
   // lower left, lower right, upper right, upper left
   //
   if( is_inside_bb(b2, b1.x_ll, b1.y_ll)
    || is_inside_bb(b2, b1.x_ur, b1.y_ll)
    || is_inside_bb(b2, b1.x_ur, b1.y_ur)
    || is_inside_bb(b2, b1.x_ll, b1.y_ur) ) {
      intersect = true;
   }
   //
   // Check if the four corners of box 2 are inside box 1
   // lower left, lower right, upper right, upper left
   //
   if( is_inside_bb(b1, b2.x_ll, b2.y_ll)
    || is_inside_bb(b1, b2.x_ur, b2.y_ll)
    || is_inside_bb(b1, b2.x_ur, b2.y_ur)
    || is_inside_bb(b1, b2.x_ll, b2.y_ur) ) {
      intersect = true;
   }

   return(intersect);
}

////////////////////////////////////////////////////////////////////////

bool is_inside_bb(const BoundingBox &bb, double x, double y) {
   bool inside = false;

   if( is_between(bb.x_ll, bb.x_ur, x)
    && is_between(bb.y_ll, bb.y_ur, y) ) {
      inside = true;
   }

   return(inside);
}

////////////////////////////////////////////////////////////////////////

bool is_between(double a, double b, double x) {
   bool between = false;

   if( (x >= a && x <= b)      // a <= b
    || (x <= a && x >= b) ) {  // a > b
      between = true;
   }

   return(between);
}

////////////////////////////////////////////////////////////////////////
