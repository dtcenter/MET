// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   shapedata.cc
//
//   Description:
//      Contains the definition of the field data class.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-05-31  Halley Gotway  Adapated from wrfdata.cc.
//   001    14-05-29  Halley Gotway  Add ShapeData::n_objects()
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <ctime>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "shapedata.h"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"

///////////////////////////////////////////////////////////////////////////////


#define  STANDARD_XY_YO_N(Nx, x, y) ((y)*(Nx) + (x))


///////////////////////////////////////////////////////////////////////////////

static double dot(double, double, double, double);
static void   boundary_step(const ShapeData &, int &, int &, int &);
static int    get_step_case(bool, bool, bool, bool);

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class ShapeData
//
///////////////////////////////////////////////////////////////////////////////

ShapeData::ShapeData() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

ShapeData::~ShapeData() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

ShapeData::ShapeData(const ShapeData &f) {

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

ShapeData & ShapeData::operator=(const ShapeData &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return(*this);
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::clear() {

   data.clear();
   mom.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::assign(const ShapeData &d) {

   clear();

   data = d.data;
   mom  = d.mom;

   return;
}

///////////////////////////////////////////////////////////////////////////////

int ShapeData::x_left(int y) const {

   if((y < 0) || (y >= data.ny())) {
      mlog << Error << "\nShapeData::x_left(int) -> "
           << "range check error\n\n";
      exit(1);
   }

   int x;

   for(x=0; x<data.nx(); x++) {
      if(f_is_on(x, y)) return(x);
   }

   return(-1);
}

////////////////////////////////////////////////////////////////////////

int ShapeData::x_right(int y) const {

   if((y < 0) || (y >= data.ny())) {
      mlog << Error << "\nShapeData::x_right(int) -> "
           << "range check error\n\n";
      exit(1);
   }

   int x;

   for(x=(data.nx() - 1); x>=0; x--) {
      if(f_is_on(x, y)) return(x);
   }

   return(-1);
}

///////////////////////////////////////////////////////////////////////////////

bool ShapeData::s_is_on(int x, int y) const

{

   // Check if the current point is non-zero

   return ( data(x, y) > 0.0 );

}


///////////////////////////////////////////////////////////////////////////////

bool ShapeData::f_is_on(int x, int y) const

{

   // Check if the current point or any of of it's neighbors are non-zero

   if(s_is_on(x, y))                            return ( true );
   if((x > 0) && s_is_on(x-1, y))               return ( true );
   if((x > 0) && (y > 0) && s_is_on(x-1, y-1))  return ( true );
   if((y > 0) && s_is_on(x, y-1))               return ( true );

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::calc_moments()

{

   int x, y;
   double xx, yy;

   mom.clear();

   for(x=0; x<data.nx(); ++x) {

      xx = ((double) x);

      for(y=0; y<data.ny(); ++y) {

         yy =((double) y);

         // Object area based on s_is_on() logic
         if(s_is_on(x, y)) mom.s_area += 1;

         if(f_is_on(x, y)) {

            mom.f_area += 1;

            mom.sx     += xx;
            mom.sy     += yy;

            mom.sxx    += xx*xx;
            mom.sxy    += xx*yy;
            mom.syy    += yy*yy;

            mom.sxxx   += xx*xx*xx;
            mom.sxxy   += xx*xx*yy;
            mom.sxyy   += xx*yy*yy;
            mom.syyy   += yy*yy*yy;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::centroid(double &xbar, double &ybar) const {

   mom.centroid(xbar, ybar);

   return;
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::angle_degrees() const {

   return(mom.angle_degrees());
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::curvature(double &xcurv, double &ycurv) const {

   return(mom.curvature(xcurv, ycurv));
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::area() const {

   double x = (double) (mom.s_area);

   return(x);
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::calc_length_width(double &l, double &w) const {
   int x, y;
   double xx, yy;
   double u, v, u_max, u_min, v_max, v_min;
   double u_extent, v_extent;
   double angle_rad, angle_deg;
   double e1x, e1y, e2x, e2y;

   angle_deg = angle_degrees();
   angle_rad = angle_deg/deg_per_rad;

   e1x = cos(angle_rad);
   e1y = sin(angle_rad);

   e2x = cos(angle_rad + piover2);
   e2y = sin(angle_rad + piover2);

   u_max = v_max = -1.0e30;
   u_min = v_min =  1.0e30;

   for (x=0; x<data.nx(); ++x) {
      for (y=0; y<data.ny(); ++y) {

         if(!f_is_on(x, y)) continue;

         xx = (double) x;
         yy = (double) y;

         u = dot(e1x, e1y, xx, yy);
         v = dot(e2x, e2y, xx, yy);

         if(u > u_max) u_max = u;
         if(u < u_min) u_min = u;

         if(v > v_max) v_max = v;
         if(v < v_min) v_min = v;
      } // for y
   } // for x

   u_extent = u_max - u_min;
   v_extent = v_max - v_min;

   if(u_extent > v_extent) { l = u_extent;  w = v_extent; }
   else                    { l = v_extent;  w = u_extent; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::length() const {
   double l, w;

   calc_length_width(l, w);

   return(l);
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::width() const {
   double l, w;

   calc_length_width(l, w);

   return(w);
}

////////////////////////////////////////////////////////////////////////

double ShapeData::complexity() const {
   int count;
   double shape;
   double hull;
   double u;
   Polyline poly;

   count = nint(mom.s_area);

   if(count == 0) {
      mlog << Error << "\nShapeData::complexity() const -> "
           << "empty shape!\n\n";
      exit(1);
   }

   shape = (double) count;
   poly  = convex_hull();
   hull  = fabs(poly.uv_signed_area());

   //
   // Complexity is defined as the difference in area between the
   // convex hull and the original shape divided by the area of the
   // convex hull.  0 <= Complexity < 1, and complexity = 0 indicates
   // that the shape is convex.
   //
   u = (hull - shape)/hull;

   return(u);
}

///////////////////////////////////////////////////////////////////////////////


void ShapeData::conv_filter_circ(int diameter, double vld_thresh)

{

int x, y, xx, yy, u, v;
int dn, fn, nn;
int vpr, upr;
int count, bd_count;
double center, cur, sum;
double dx, dy, dist;
double vld_ratio;
const int nx = data.nx();
const int ny = data.ny();
bool * f = (bool *) 0;
bool center_bad = false;
DataPlane in_data = data;
const bool vld_thresh_one = is_eq(vld_thresh, 1.0);


if ( (diameter%2 == 0) || (diameter < 3) )  {

   mlog << Error << "\nShapeData::conv_filter_circ() -> "
        << "diameter must be odd and >= 3 ... diameter = "
        << diameter << "\n\n";

   exit(1);

}

const int radius = (diameter - 1)/2;

const double * in  = in_data.Data;
      double * out = data.Data;

f = new bool [diameter*diameter];

   //
   //  set up the filter
   //

for (y=0; y<diameter; ++y)  {

   dy = y - radius;

   for (x=0; x<diameter; ++x)  {

      dx = x - radius;

      dist = sqrt( dx*dx + dy*dy );

      fn = STANDARD_XY_YO_N(diameter, x, y) ;

      f[fn] = (dist <= radius);

   }

}

   //
   //  do the convolution
   //

dn = -1;

for(y=0; y<ny; y++) {

   for(x=0; x<nx; x++) {

      ++dn;

         //
         // If the bad data threshold is set to zero and the center of the
         // convolution radius contains bad data, set the convolved value to
         // bad data and continue.
         //

      center = in[dn];

      center_bad = ::is_bad_data(center);

      if ( center_bad && vld_thresh_one ) { out[dn] = bad_data_double;  continue; }

      sum      = 0.0;
      count    = 0;
      bd_count = 0;

      for (v=-radius; v<=radius; ++v) {

         yy = y + v;

         if ( (yy < 0) || (yy >= ny) )  continue;

         vpr = v + radius;

         for(u=-radius; u<=radius; ++u) {

            xx = x + u;

            if ( (xx < 0) || (xx >= nx) )  continue;

            upr = u + radius;

            fn = STANDARD_XY_YO_N(diameter, upr, vpr);

            if ( !(f[fn]) )  continue;

            nn = STANDARD_XY_YO_N(nx, xx, yy) ;

            cur = in[nn];

            if( ::is_bad_data(cur) ) { bd_count++;  continue; }

            sum += cur;

            count++;

         } // for v

      } // for u

         //
         //  If the center of the convolution contains bad data and the ratio
         //  of bad data in the convolution area is too high, set the convoled
         //  value to bad data.
         //

      if ( count == 0 )  sum = bad_data_double;
      else {

         vld_ratio = ((double) count)/(bd_count + count);

         if ( center_bad && (vld_ratio < vld_thresh) )  sum = bad_data_double;
         else                                           sum /= count;

      }

      out[dn] = sum;

   } // for y

} // for x

   //
   //  done
   //

if ( f )  { delete [] f;   f = (bool *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


Polyline ShapeData::convex_hull() const

{

   int j, k, n, y;
   int done;
   int *Index = (int *) 0;
   Polyline outline;
   Polyline hull;
   double e1u, e1v, e2u, e2v;
   double angle_low, v_low, alpha, beta;
   double t, angle, p1u, p1v, p2u, p2v;

   hull.clear();
   outline.clear();

   if(area() <= 0) {

      mlog << Error << "\nconvex_hull(Polyline &) -> "
           << "attempting to fit convex hull to a shape with area = 0\n\n";
      exit(1);
   }

   hull.extend_points(2*data.ny());
   outline.extend_points(2*data.ny());

   Index = new int [2*data.ny()];

   if ( !Index )  {

      mlog << Error << "\nconvex_hull(Polyline &) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   n = 0;

   for (y=0; y<data.ny(); ++y)  {

      j = x_left(y);

      if ( j < 0 )  continue;

      outline.u[n] = (double) j;
      outline.v[n] = (double) y;

      ++n;

      k = x_right(y);

      if ( k < 0 )  continue;

      if ( j == k )  continue;

      outline.u[n] = (double) k;
      outline.v[n] = (double) y;

      ++n;

   }   //  for y

   outline.n_points = n;

      //
      //  find "lowest" point in outline
      //

   v_low = 1.0e10;

   j = -1;

   for (k=0; k<(outline.n_points); ++k)  {

      if ( outline.v[k] < v_low )  { v_low = outline.v[k];  j = k; }

   }

   if ( j < 0 )  {

      mlog << Error << "\nconvex_hull(Polyline &) -> "
           << "can't find lowest point\n\n";
      exit(1);

   }

   n = 1;

   Index[0] = j;

      //
      //  find hull
      //

   e1u = 1.0;
   e1v = 0.0;

   done = 0;

   while ( !done  )  {

      e2u = -e1v;
      e2v =  e1u;

      angle_low = 1.0e10;

      p1u = outline.u[Index[n - 1]];
      p1v = outline.v[Index[n - 1]];

      j = -1;

      for (k=0; k<(outline.n_points); ++k)  {

         if ( k == Index[n - 1] )  continue;

         p2u = outline.u[k];
         p2v = outline.v[k];

         alpha = (p2u - p1u)*e1u + (p2v - p1v)*e1v;
         beta  = (p2u - p1u)*e2u + (p2v - p1v)*e2v;

         if ( alpha == 0 && beta == 0 )  continue;

         angle = deg_per_rad*atan2(beta, alpha);

         angle -= 360.0*floor(angle/360.0);

            //
            //  reset angle very close to 360 to be 0
            //

         if ( angle > 359.9999 )   { angle = 0.0; }

         if ( angle < angle_low )  { angle_low = angle;  j = k; }

      }   //  for k

      if ( j < 0 )  {

         mlog << Error << "\nconvex_hull(Polyline &) -> "
              << "can't find next hull point\n\n";
         exit(1);
      }

      p2u = outline.u[j];
      p2v = outline.v[j];

      e1u = p2u - p1u;
      e1v = p2v - p1v;

      t = sqrt( e1u*e1u + e1v*e1v );

      e1u /= t;
      e1v /= t;

      Index[n++] = j;

      if ( (n >= 3) && (Index[n - 1] == Index[0]) ) done = 1;

   }   //  while

      //
      //  load up hull
      //

   --n;

   hull.n_points = n;

   for (j=0; j<n; ++j)  {

      hull.u[j] = outline.u[Index[j]];
      hull.v[j] = outline.v[Index[j]];

   }

      //
      //  done
      //

   delete [] Index;   Index = (int *) 0;

   return(hull);

}


///////////////////////////////////////////////////////////////////////////////

Polyline ShapeData::single_boundary() const {

   //
   // Call boundary function with all_points set to false
   // and clockwise set to true
   //

   return( single_boundary(false, 1) );
}

///////////////////////////////////////////////////////////////////////////////
//
//  ShapeData::single_boundary() should only be called for split fields
//  containing only one object.
//
///////////////////////////////////////////////////////////////////////////////

Polyline ShapeData::single_boundary(bool all_points, int clockwise) const {

   //
   // Call boundary offset function with all_points set to false
   // and clockwise set to true
   //

   return( single_boundary_offset(false, 1, 0.0) );
}

/////////////////////////////////////////////////////////////////////////////////
//
// By default, each grid point defines the grid box to its upper-right.
// By providing a offset value, that definition may be modified.
// For example, an offset of -0.5 defines the grid box whose center is the grid
// point turned on.  An offset of -1.0 defines the grid box whose upper-right
// corner is the grid box turned on.  An offset of 0.0 defines the grid box
// whose lower-left corner is the grid box turned on.
//
/////////////////////////////////////////////////////////////////////////////////

Polyline ShapeData::single_boundary_offset(double d) const {

   //
   // Call boundary offset function with all_points set to false
   // and clockwise set to true
   //

   return( single_boundary_offset(false, 1, d) );
}

///////////////////////////////////////////////////////////////////////////////
//
//  ShapeData::single_boundary_offset() should only be called for split fields
//  containing only one object.
//
///////////////////////////////////////////////////////////////////////////////

Polyline ShapeData::single_boundary_offset(bool all_points, int clockwise,
                                           double d) const {
   Polyline boundary, temp;
   int i, x, y, x0, y0, xn, yn;
   int direction, new_direction;
   bool found;

   // Initialize
   boundary.clear();

   //
   // Find the first point in the object
   //
   found = false;
   for(x=0; x<data.nx(); x++) {
      for(y=0; y<data.ny(); y++) {

         if(f_is_on(x, y)) {
            x0 = x;
            y0 = y;
            found = true;
            break;
         }
      }
      if(found) break;
   }

   if(!found) {

      mlog << Debug(1) << "\n\nShapeData::single_boundary_offset() const -> "
           << "no points found in object\n\n";

      return(boundary);
   }

   //
   // Due to the search order, the initial direction will be plus_x
   //
   direction = plus_x;
   new_direction = direction;
   boundary.add_point(x0+d, y0+d);

   //
   // Initialize xn and yn to starting point
   //
   xn = x0;
   yn = y0;

   //
   // Find next point along boundary
   //
   boundary_step(*this, xn, yn, new_direction);

   //
   // Store only points where a change of direction occurs
   // or all points if so indicated
   //
   if( all_points ||
      (!all_points && direction != new_direction) ) {

      boundary.add_point(xn+d, yn+d);
   }
   direction = new_direction;

   //
   // Step along the boundary and store each point of the boundary polyline
   // where a change in direction occurs
   //
   while(xn != x0 || yn != y0) {

      boundary_step(*this, xn, yn, new_direction);

      //
      // Store only points where a change of direction occurs
      // or all points if so indicated
      //
      if( all_points ||
         (!all_points && direction != new_direction) ) {
         boundary.add_point(xn+d, yn+d);
      }
      direction = new_direction;
   }

   //
   // If indicated, reverse the direction of the points from clockwise to
   // counter-clockwise
   //
   if(!clockwise) {
      temp = boundary;
      boundary.clear();

      for(i=temp.n_points-1; i>=0; i--) {

         boundary.add_point(temp.u[i], temp.v[i]);
      }
   }

   return(boundary);
}

///////////////////////////////////////////////////////////////////////////////
//
//  zero_field()
//  This routine will zero out all of the data in the in the field
//
///////////////////////////////////////////////////////////////////////////////

void ShapeData::zero_field()

{
   data.set_constant(0.0);

   return;
}


///////////////////////////////////////////////////////////////////////////////
//
//  End Code for class ShapeData
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class Cell
//
///////////////////////////////////////////////////////////////////////////////

Cell::Cell() {

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

Cell::~Cell() { }

///////////////////////////////////////////////////////////////////////////////

Cell::Cell(const Cell &c) {

   assign(c);
}

///////////////////////////////////////////////////////////////////////////////

Cell & Cell::operator=(const Cell &c) {

   if ( this == &c )  return ( *this );

   assign(c);

   return(*this);
}

///////////////////////////////////////////////////////////////////////////////

void Cell::clear() {
   int j;

   for (j=0; j<max_cell_elements; ++j)  e[j] = -1;

   n = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Cell::assign(const Cell &c) {
   int j;

   clear();

   for (j=0; j<(c.n); ++j)  e[j] = c.e[j];

   n = c.n;

   return;
}

///////////////////////////////////////////////////////////////////////////////

int Cell::has(int k) const {
   int j;

   for (j=0; j<n; ++j) {
      if ( e[j] == k )  return ( 1 );
   }

   return(0);
}

///////////////////////////////////////////////////////////////////////////////

void Cell::add(int k) {

   if ( has(k) )  return;

   if ( n >= max_cell_elements ) {
      mlog << Error << "\nvoid Cell::add() -> "
           << "too many elements!\n\n";
      exit(1);
   }

   e[n++] = k;

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  End Code for class Cell
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class Partition
//
///////////////////////////////////////////////////////////////////////////////

Partition::Partition() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

Partition::~Partition() { }

///////////////////////////////////////////////////////////////////////////////

Partition::Partition(const Partition &p) {

   assign(p);
}

///////////////////////////////////////////////////////////////////////////////

Partition & Partition::operator=(const Partition &p) {

   if ( this == &p )  return ( *this );

   assign(p);

   return(*this);
}

///////////////////////////////////////////////////////////////////////////////

void Partition::clear() {
   int j;

   for (j=0; j<max_cells; ++j)  c[j].clear();

   n = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::assign(const Partition &p) {
   int j;

   clear();

   for (j=0; j<(p.n); ++j)  c[j] = p.c[j];

   n = p.n;

   return;
}

///////////////////////////////////////////////////////////////////////////////

int Partition::has(int k) const {
   int j;

   for (j=0; j<n; ++j) {
      if ( c[j].has(k) )  return ( 1 );
   }

   return(0);
}

///////////////////////////////////////////////////////////////////////////////

int Partition::which_cell(int k) const {
   int j;

   for (j=0; j<n; ++j) {
      if ( c[j].has(k) )  return ( j );
   }

   return(-1);
}

///////////////////////////////////////////////////////////////////////////////

void Partition::merge_cells(int j_1, int j_2) {
   int k;
   int j_min, j_max;

   if ( (j_1 < 0) || (j_1 >= n) || (j_2 < 0) || (j_2 >= n) ) {
      mlog << Error << "\nPartition::merge_cells() -> "
           << "range check error\n\n";
      exit(1);
   }

   if ( j_1 == j_2 )  return;

   if ( j_1 < j_2 ) {
      j_min = j_1;
      j_max = j_2;
   }
   else {
      j_min = j_2;
      j_max = j_1;
   }

   for (k=0; k<(c[j_max].n); ++k) {
      c[j_min].add(c[j_max].e[k]);
   }

   c[j_max] = c[n - 1];

   c[n - 1].clear();

   --n;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::merge_values(int v1, int v2) {
   int j_1, j_2;

   if ( v1 == v2 )  return;

   j_1 = which_cell(v1);
   j_2 = which_cell(v2);

   if ( (j_1 < 0) || (j_2 < 0) ) {
      mlog << Error << "\nvoid Partition::merge_values() -> "
           << "bad values: (v1, v2) = (" << v1 << ", " << v2
           << "), (j1, j2) = (" << j_1 << ", " << j_2 << ")\n\n";
      return;
   }

   merge_cells(j_1, j_2);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::add(int k) {

   if ( has(k) )  return;

   if ( n >= max_cells ) {
      mlog << Error << "\nvoid Partition::add() -> "
           << "too many cells!\n\n";
      exit(1);
   }

   c[n++].add(k);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  End Code for class Partition
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for Miscellaneous Functions
//
///////////////////////////////////////////////////////////////////////////////

double dot(double x_1, double y_1, double x_2, double y_2) {
   double d;

   d = x_1*x_2 + y_1*y_2;

   return(d);
}

///////////////////////////////////////////////////////////////////////////////

void boundary_step(const ShapeData &sd, int &xn, int &yn, int &direction) {
   bool lr, ur, ul, ll;

   lr = ur = ul = ll = false;

   //
   // Based on the direction of travel turn on/off lr, ur, ul, ll cells
   //
   switch(direction) {

      case(plus_x):
         if(sd.s_is_on(xn, yn-1)  ) lr = true;
         if(sd.s_is_on(xn+1, yn-1)) ur = true;
         if(sd.s_is_on(xn+1, yn)  ) ul = true;
         if(sd.s_is_on(xn, yn)    ) ll = true;

         xn += 1;
         break;

      case(plus_y):
         if(sd.s_is_on(xn, yn)    ) lr = true;
         if(sd.s_is_on(xn, yn+1)  ) ur = true;
         if(sd.s_is_on(xn-1, yn+1)) ul = true;
         if(sd.s_is_on(xn-1, yn)  ) ll = true;

         yn += 1;
         break;

      case(minus_x):
         if(sd.s_is_on(xn-1, yn)  ) lr = true;
         if(sd.s_is_on(xn-2, yn)  ) ur = true;
         if(sd.s_is_on(xn-2, yn-1)) ul = true;
         if(sd.s_is_on(xn-1, yn-1)) ll = true;

         xn -= 1;
         break;

      case(minus_y):
         if(sd.s_is_on(xn-1, yn-1)) lr = true;
         if(sd.s_is_on(xn-1, yn-2)) ur = true;
         if(sd.s_is_on(xn, yn-2)  ) ul = true;
         if(sd.s_is_on(xn, yn-1)  ) ll = true;

         yn -= 1;
         break;

      default:
         mlog << Error << "\nboundary_step() -> "
              << "bad direction: " << direction << "\n\n";
         exit(1);
         break;
   }

   //
   // Determine the direction to head by the combination of lr, ur, ul, and ll
   //
   switch(get_step_case(lr, ur, ul, ll)) {

      case ll_case:
      case lr_ul_case:
      case lr_ur_ul_case:
         // Turn left
         direction = (direction + 1)%4;
         if(direction < 0) direction += 4;
         break;

      case lr_case:
      case ur_ll_case:
      case ur_ul_ll_case:
         // Turn right
         direction = (direction - 1)%4;
         if(direction < 0) direction += 4;
         break;

      case ul_ll_case:
      case lr_ur_case:
         // Continue straight: direction remains unchanged
         break;

      default:

         mlog << Error << "\nboundary_step() -> "
              << "bad step case: "
              << get_step_case(lr, ur, ul, ll) << "\n\n";
         exit(1);
         break;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int get_step_case(bool lr, bool ur, bool ul, bool ll) {

   //
   // Valid cases with exactly one cell on
   //

   // Lower Left
   if(!lr && !ur && !ul && ll) return(ll_case);
   // Lower Right
   else if(lr && !ur && !ul && !ll) return(lr_case);

   //
   // Valid cases with exactly two cells on
   //

   // Upper Left, Lower Left
   else if(!lr && !ur && ul && ll) return(ul_ll_case);
   // Lower Right, Upper Right
   else if(lr && ur && !ul && !ll) return(lr_ur_case);
   // Lower Right, Upper Left
   else if(lr && !ur && ul && !ll) return(lr_ul_case);
   // Upper Right, Lower Left
   else if(!lr && ur && !ul && ll) return(ur_ll_case);

   //
   // Valid cases with exactly three cells on
   //

   // Upper Right, Upper Left, Lower Left
   else if(!lr && ur && ul && ll) return(ur_ul_ll_case);
   // Lower Right, Upper Right, Upper Left
   else if(lr && ur && ul && !ll) return(lr_ur_ul_case);

   //
   // Otherwise, combination is invalid
   //
   else {
      mlog << Error << "\nget_step_case() -> "
           << "invalid combination: (lr, ur, ul, ll) = (" << lr << ", "
           << ur << ", " << ul << ", " << ll << ")\n\n";
      exit(1);
   }

   return(-1);
}

///////////////////////////////////////////////////////////////////////////////

void apply_mask(ShapeData &f, ShapeData &mask)

{
   int x, y;

   if(f.data.nx() != mask.data.nx() ||
      f.data.ny() != mask.data.ny() ) {

      mlog << Error << "\napply_mask() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   for(x=0; x<f.data.nx(); x++) {
      for(y=0; y<f.data.ny(); y++) {

         //
         // Put bad data everywhere the mask is turned off
         //
         if(!mask.s_is_on(x, y)) f.data.set(bad_data_float, x, y);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int ShapeData::n_objects() const

{
   int n;
   ShapeData sd;

   // Split the field to number the shapes
   sd = split(*this, n);
   
   return(n);
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::threshold(SingleThresh t) {
   int j, x, y;
   double v;
   const int nx = data.nx();
   const int ny = data.ny();

   //
   // Compare the threshold double value to the double values for the
   // ShapeData field
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         v = data(x, y);

         if(t.check(v) && ! ::is_bad_data(v)) {
            j = 1;
         }
         else {
            j = 0;
         }
         data.set((double) j, x, y);
      } // end for y
   } // end for x

   return;
}

///////////////////////////////////////////////////////////////////////////////


void ShapeData::threshold_area(SingleThresh t) {
   int j, n, x, y, v_int;
   ShapeData sd_split, sd_object;
   double area_object[1 + max_cells];  // area_object[0] is ignored
   const int nx = data.nx();
   const int ny = data.ny();

   // Split the field to number the shapes
   sd_split = split(*this, n);

   // Zero out area array
   for(j=0; j<max_cells; j++) area_object[j] = 0;

   //
   // Compute the area of each object
   //
   for(j=1; j<=n; j++) {
      sd_object = select(sd_split, j);
      area_object[j] = sd_object.area();
   }

   //
   // Zero out any shapes with an area that doesn't meet the
   // threshold criteria
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         v_int = nint(sd_split.data(x, y));

         if(!t.check(area_object[v_int])) {
            data.set(0.0, x, y);
         }

      } // end for y
   } // end for x

   return;
}


///////////////////////////////////////////////////////////////////////////////


void ShapeData::threshold_intensity(const ShapeData *sd_ptr, int perc, SingleThresh t)

{
   int i, n, x, y, v_int, n_obj_inten;
   ShapeData s;
   double *obj_inten = (double *) 0, obj_inten_sum;
   double inten_object[1 + max_cells];  // area_object[0] is ignored
   const int nx = data.nx();
   const int ny = data.ny();

   if(perc < 0 || perc > 102) {
      mlog << Error << "\nShapeData:threshold_intensity() -> "
           << "the intensity percentile requested must be between 0 and 102.\n\n";
      exit(1);
   }

   obj_inten = new double [nx*ny];

   //
   // Split the field to number the shapes
   //
   s = split(*this, n);

   //
   // For each object, compute the requested percentile of intensity
   //
   for(i=0; i<n; i++) {

      n_obj_inten = 0;
      obj_inten_sum = 0.0;
      for(x=0; x<nx; x++) {
         for(y=0; y<ny; y++) {

            v_int = nint(s.data(x, y));

            if(v_int != i+1) continue;

            if(sd_ptr->is_valid_xy(x, y)) {
               obj_inten[n_obj_inten] = sd_ptr->data(x, y);
               obj_inten_sum += obj_inten[n_obj_inten];
               n_obj_inten++;
            }
         } // end for y
      } // end for x

      sort(obj_inten, n_obj_inten);

      //
      // Compute the mean of the intensities
      //
      if(perc == 101) {
         inten_object[i+1] = obj_inten_sum/n_obj_inten;
      }
      //
      // Compute the sum of the intensities
      //
      else if(perc == 102) {
         inten_object[i+1] = obj_inten_sum;
      }
      //
      // Compute a percentile of intensity
      //
      else {
         inten_object[i+1] = percentile(obj_inten, n_obj_inten, (double) perc/100.0);
      }
   }

   //
   // Zero out any shapes with an intensity that doesn't meet the
   // threshold criteria
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         v_int = nint(s.data(x, y));

         if(!t.check(inten_object[v_int])) {
            data.set(0.0, x, y);
         }

      } // end for y
   } // end for x

   if(obj_inten) { delete [] obj_inten; obj_inten = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

ShapeData split(const ShapeData &wfd, int &n_shapes)

{
   int k, x, y;
   int s;
   int xx, yy, numx, numy;
   int current_shape;
   int shape_assigned;
   ShapeData d;
   ShapeData q = wfd;
   Partition p;

   numx = wfd.data.nx();
   numy = wfd.data.ny();

   d.data.set_size(numx, numy);

   n_shapes = 0;

   //  shape numbers start at ONE here!!

   current_shape = 0;

   for (y=(wfd.data.ny() - 2); y>=0; --y) {
      for (x=(wfd.data.nx() - 2); x>=0; --x) {

         s = wfd.s_is_on(x, y);

         if ( !s ) continue;

         shape_assigned = 0;

         // check above left

         xx = x - 1;
         yy = y + 1;

         if ( (xx >= 0) && (yy < numy) ) {

            s = wfd.s_is_on(xx, yy);

            if ( s ) {
               if ( shape_assigned )
                  p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
               else
                  d.data.set(d.data(xx, yy), x, y);

               shape_assigned = 1;
            }
         }

         // check above

         xx = x;
         yy = y + 1;

         if ( yy < numy ) {

            s = wfd.s_is_on(xx, yy);

            if ( s ) {
               if ( shape_assigned )
                  p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
               else
                  d.data.set(d.data(xx, yy), x, y);

               shape_assigned = 1;
            }
         }

         // check upper right

         xx = x + 1;
         yy = y + 1;

         if ( (xx < numx) && (yy < numy) ) {

            s = wfd.s_is_on(xx, yy);

            if ( s ) {

               if ( shape_assigned )
                  p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
               else
                  d.data.set(d.data(xx, yy), x, y);

               shape_assigned = 1;
            }
         }

         // check to the right

         xx = x + 1;
         yy = y;

         if ( xx < numx ) {

            s = wfd.s_is_on(xx, yy);

            if ( s ) {

               if ( shape_assigned )
                  p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
               else
                  d.data.set(d.data(xx, yy), x, y);

               shape_assigned = 1;
            }
         }

         // is it a new shape?

         if ( !shape_assigned ) {

            d.data.set(++current_shape, x, y);
            p.add(nint(d.data(x, y)));
         }
      } // for x
   } // for y

   for (x=0; x<numx; ++x) {
      for (y=0; y<numy; ++y) {

         q.data.set(0, x, y);

         for (k=0; k<(p.n); ++k) {
            if ( p.c[k].has(nint(d.data(x, y))) )   q.data.set(k + 1, x, y);
         }
      } // for y
   } // for x

   n_shapes = p.n;

   q.calc_moments();

   return(q);

}


///////////////////////////////////////////////////////////////////////////////

ShapeData select(const ShapeData &id, int n)

{
   int k, x, y;
   int nx, ny;
   int count;
   ShapeData d = id;

   nx = id.data.nx();
   ny = id.data.ny();

   count = 0;

   for(x=0; x<nx; ++x) {
      for(y=0; y<ny; ++y) {

         k = nint(id.data(x, y));

         if(k == n) {
            d.data.set(1, x, y);

            ++count;
         }
         else {
            d.data.set(0, x, y);
         }
      }
   }
   d.calc_moments();

   return(d);

}


///////////////////////////////////////////////////////////////////////////////

void ShapeData::zero_border(int size) {

   zero_border(size, 0.0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::zero_border(int size, double value) {
   int x, y;

   // top

   for (x=0; x<data.nx(); ++x) {
      for (y=(data.ny() - size); y<data.ny(); ++y) {

         data.set(value, x, y);
      } // for y
   } // for x

   //  bottom

   for (x=0; x<data.nx(); ++x) {
      for (y=0; y<size; ++y) {

         data.set(value, x, y);
      } // for y
   } // for x

   //  left

   for (x=0; x<size; ++x) {
      for (y=0; y<data.ny(); ++y) {

         data.set(value, x, y);
      } // for y
   } // for x

   //  right

   for (x=(data.nx() - size); x<data.nx(); ++x) {
      for (y=0; y<data.ny(); ++y) {

         data.set(value, x, y);
      } // for y
   } // for x

   return;
}


///////////////////////////////////////////////////////////////////////////////


void ShapeData::filter(SingleThresh t) {
   int x, y;
   double v;
   const int nx = data.nx();
   const int ny = data.ny();

   for (x=0; x<nx; ++x) {
      for (y=0; y<ny; ++y) {

         v = data(x, y);

         if(!t.check(v) && ! ::is_bad_data(v)) {
            data.set(0.0, x, y);
         }

      } // end for y
   } // end for x

   return;
}


///////////////////////////////////////////////////////////////////////////////


int ShapeData_intersection(const ShapeData &f1, const ShapeData &f2) {
   int x, y, intersection;

   //
   // Check for the same grid dimension
   //
   if(f1.data.nx() != f2.data.nx() ||
      f1.data.ny() != f2.data.ny() ) {

      mlog << Error << "\nShapeData_intersection() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   intersection = 0;
   for(x=0; x<f1.data.nx(); x++) {
      for(y=0; y<f1.data.ny(); y++) {

         if(f1.s_is_on(x, y) && f2.s_is_on(x, y)) intersection++;
      } // end for y
   } // end for x

   return(intersection);
}


///////////////////////////////////////////////////////////////////////////////
//
//  End Code for Miscellaneous Functions
//
///////////////////////////////////////////////////////////////////////////////
