// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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


static const bool use_new = true;

static const int split_enlarge = 4;   //  used for ShapeData  shrink and expand

static const bool do_split_fatten = true;


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
#include "mode_columns.h"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"

#include "ihull.h"


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

bool ShapeData::s_is_on(int x, int y, bool error_out) const

{

   // Unless error out is true, return bad status for being off the grid

   if(!error_out) {
      if(x < 0 || x >= data.nx() || y < 0 || y >= data.ny()) return ( false );
   }

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

double ShapeData::area_thresh(const ShapeData *raw_ptr,
                              const SingleThresh &obj_thresh) const {
   int i, cur_area;
   const int Nxy = data.nx()*data.ny();

   // Number of points inside the object that meet the threshold criteria
   for(i=0, cur_area=0; i<Nxy; i++) {
      if(data.data()[i] > 0 && obj_thresh.check(raw_ptr->data.data()[i])) cur_area++;
   }

   return(cur_area);
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

double ShapeData::intensity_percentile(const ShapeData *raw_ptr, int perc,
                                       bool precip_flag) const {
   int i, n;
   double * val = (double *) 0;
   double val_sum, v;
   const int Nxy = data.nx()*data.ny();

   if(perc < 0 || perc > 102) {
      mlog << Error << "\nShapeData::intensity_percentile() -> "
           << "the intensity percentile requested must be between 0 and 102.\n\n";
      exit(1);
   }

   val = new double [Nxy];

   // Compute the requested percentile of intensity
   for(i=0, n=0, val_sum=0.0; i<Nxy; i++) {

      // Process points for the current object
      if(data.data()[i] > 0) {

         v = raw_ptr->data.data()[i];

         // Skip bad data and zero precip
         if(::is_bad_data(v) || (precip_flag && is_eq(v, 0.0))) continue;

         // Store current value
         val[n] = v;
         val_sum += v;
         n++;
      }
   }

   // Compute the mean of the intensities
   if(perc == 101) {
      v = val_sum/n;
   }
   // Compute the sum of the intensities
   else if(perc == 102) {
      v = val_sum;
   }
   // Compute a percentile of intensity
   else {
      sort(val, n);
      v = percentile(val, n, (double) perc/100.0);
   }

   // Clean up
   if(val) { delete [] val; val = (double *) 0; };

   return(v);
}

////////////////////////////////////////////////////////////////////////

double ShapeData::get_attr(const ConcatString &attr_name,
                           const ShapeData *raw_ptr,
                           const SingleThresh &obj_thresh,
                           const Grid *grid,
                           bool precip_flag) const {
   double v1, v2, v3, attr_val;

   if(strcasecmp(attr_name.c_str(), "CENTROID_X") == 0) {
      centroid(attr_val, v2);
   }
   else if(strcasecmp(attr_name.c_str(), "CENTROID_Y") == 0) {
      centroid(v1, attr_val);
   }
   else if(strcasecmp(attr_name.c_str(), "CENTROID_LAT") == 0) {
      centroid(v1, v2);
      grid->xy_to_latlon(v1, v2, attr_val, v3);
   }
   else if(strcasecmp(attr_name.c_str(), "CENTROID_LON") == 0) {
      centroid(v1, v2);
      grid->xy_to_latlon(v1, v2, v3, attr_val);
   }
   else if(strcasecmp(attr_name.c_str(), "AXIS_ANG") == 0) {
      attr_val = angle_degrees();
   }
   else if(strcasecmp(attr_name.c_str(), "LENGTH") == 0) {
      attr_val = length();
   }
   else if(strcasecmp(attr_name.c_str(), "WIDTH") == 0) {
      attr_val = width();
   }
   else if(strcasecmp(attr_name.c_str(), "ASPECT_RATIO") == 0) {
      calc_length_width(v1, v2);
      attr_val = v2/v1;
   }
   else if(strcasecmp(attr_name.c_str(), "AREA") == 0) {
      attr_val = area();
   }
   else if(strcasecmp(attr_name.c_str(), "AREA_THRESH") == 0) {
      attr_val = area_thresh(raw_ptr, obj_thresh);
   }
   else if(strcasecmp(attr_name.c_str(), "CURVATURE") == 0) {
      attr_val = curvature(v1, v2);
   }
   else if(strcasecmp(attr_name.c_str(), "CURVATURE_X") == 0) {
      v1 = curvature(attr_val, v2);
   }
   else if(strcasecmp(attr_name.c_str(), "CURVATURE_Y") == 0) {
      v1 = curvature(v1, attr_val);
   }
   else if(strcasecmp(attr_name.c_str(), "COMPLEXITY") == 0) {
      attr_val = complexity();
   }
   else if(strncasecmp(attr_name.c_str(), "INTENSITY_", strlen("INTENSITY_")) == 0) {
      StringArray sa = attr_name.split("_");
      attr_val = intensity_percentile(raw_ptr, atoi(sa[1].c_str()), precip_flag);
   }
   else {
      mlog << Warning << "\nShapeData::get_attr() -> "
           << "Filtering requested for unsupported object attribute \""
           << attr_name << "\".\n\n";
      attr_val = bad_data_double;
   }

   return(attr_val);
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

const vector<double> * in  = &(in_data.Data);
      vector<double> * out = &(data.Data);

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

      center = (*in)[dn];

      center_bad = ::is_bad_data(center);

      if ( center_bad && vld_thresh_one ) { (*out)[dn] = bad_data_double;  continue; }

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

            cur = (*in)[nn];

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

      (*out)[dn] = sum;

   } // for y

} // for x

   //
   //  done
   //

if ( f )  { delete [] f;   f = (bool *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


Polyline ShapeData::convex_hull_new() const

{

int j, k, y;
int n_in, n_out;
Polyline hull_poly;
IntPoint * in = new IntPoint [2*(data.ny() + 1)];


n_in = 0;

for (y=0; y<(data.ny()); ++y)  {

   j = x_left(y);

   if ( j < 0 )  continue;

   in[n_in].x = j;
   in[n_in].y = y;

   ++n_in;

   k = x_right(y);

   if ( k < 0 )  continue;

   if ( j == k )  continue;

   // ++k;

   // if ( k >= data.ny() )  k = data.ny() - 1;

   in[n_in].x = k;
   in[n_in].y = y;

   ++n_in;

}   //  for y

IntPoint * out = new IntPoint [n_in + 2];

ihull(in, n_in, out, n_out);

hull_poly.extend_points(n_out);

for (j=0; j<n_out; ++j)  {

   hull_poly.add_point(out[j].x, out[j].y);

}


   //
   //  done
   //

if ( out )  { delete [] out;  out = 0; }
if (  in )  { delete []  in;   in = 0; }

return ( hull_poly );

}


////////////////////////////////////////////////////////////////////////


Polyline ShapeData::convex_hull() const

{

Polyline p;

if ( use_new )  p = convex_hull_new ();
else            p = convex_hull_old ();

return ( p );

}


////////////////////////////////////////////////////////////////////////


Polyline ShapeData::convex_hull_old() const

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

      mlog << Error << "\nShapedata::convex_hull() -> "
           << "attempting to fit convex hull to a shape with area = 0\n\n";
      exit(1);
   }

   hull.extend_points(2*data.ny());
   outline.extend_points(2*data.ny());

   Index = new int [2*data.ny()];

   if ( !Index )  {

      mlog << Error << "\nShapedata::convex_hull() -> "
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

      mlog << Error << "\nShapedata::convex_hull() -> "
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

         mlog << Error << "\nShapedata::convex_hull() -> "
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

   return( single_boundary_offset(all_points, clockwise, 0.0) );
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
   int x, y, x0, y0, xn, yn;
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

      for(int i=temp.n_points-1; i>=0; i--) {

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


void ShapeData::expand(const int W)

{

if ( W <= 0 )  {

   mlog << Error
        << "\n\n  ShapeData::expand(const int) -> bad value ... " << W << "\n\n";

   exit ( 1 );

}


int x_old, y_old, x_new, y_new;
const int nx_old = data.nx();
const int ny_old = data.ny();
const int nx_new = nx_old + 2*W;
const int ny_new = ny_old + 2*W;
DataPlane old = data;

data.set_size(nx_new, ny_new);

data.set_constant(0.0);

for (x_old=0; x_old<nx_old; ++x_old)  {

   x_new = x_old + W;

   for (y_old=0; y_old<ny_old; ++y_old)  {

      y_new = y_old + W;

      data.put(old.get(x_old, y_old), x_new, y_new);

   }   //  for y_old

}   //  for x_old



return;

}


///////////////////////////////////////////////////////////////////////////////


void ShapeData::shrink(const int W)

{

if ( W <= 0 )  {

   mlog << Error
        << "\n\n  ShapeData::shrink(const int) -> bad value ... " << W << "\n\n";

   exit ( 1 );

}

const int nx_old = data.nx();
const int ny_old = data.ny();
const int nx_new = nx_old - 2*W;
const int ny_new = ny_old - 2*W;

if ( (nx_new <= 0) || (ny_new <= 0) )  {

   mlog << Error
        << "\n\n  ShapeData::shrink(const int) -> value too large ... new grid is empty\n\n";

   exit ( 1 );

}


int x_old, y_old, x_new, y_new;
DataPlane old = data;


for (x_new=0; x_new<nx_new; ++x_new)  {

   x_old = x_new + W;

   for (y_new=0; y_new<ny_new; ++y_new)  {

      y_old = y_new + W;

      data.put(old.get(x_old, y_old), x_new, y_new);

   }

}








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

Cell::Cell()

{

init_from_scratch();

return;

}

///////////////////////////////////////////////////////////////////////////////

Cell::~Cell()

{

clear();

}

///////////////////////////////////////////////////////////////////////////////

Cell::Cell(const Cell & c)

{

init_from_scratch();

assign(c);

}

///////////////////////////////////////////////////////////////////////////////

Cell & Cell::operator=(const Cell & c)

{

if ( this == &c )  return ( * this );

assign(c);

return ( * this );

}

///////////////////////////////////////////////////////////////////////////////


void Cell::init_from_scratch()

{

e = 0;

clear();

return;

}


///////////////////////////////////////////////////////////////////////////////

void Cell::clear()

{

if ( e )  { delete [] e;  e = (int *) 0; }

n = 0;

n_alloc = 0;

return;

}

///////////////////////////////////////////////////////////////////////////////

void Cell::assign(const Cell & c)

{

int j;

clear();

if ( c.n == 0 )  return;

extend(c.n);

for (j=0; j<(c.n); ++j)  e[j] = c.e[j];

n = c.n;

return;

}

///////////////////////////////////////////////////////////////////////////////


void Cell::extend(int N)

{

if ( n_alloc >= N )  return;

N = (N + cell_alloc_inc - 1)/cell_alloc_inc;

N *= cell_alloc_inc;

int j;
int * u = 0;

u = new int [N];

for (j=0; j<n; ++j)  u[j] = e[j];

for (j=n; j<N; ++j)  u[j] = -1;

if ( e )  { delete [] e;  e = 0; }

e = u;

n_alloc = N;

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////

bool Cell::has(int k) const

{

int j;

for (j=0; j<n; ++j) {

   if ( e[j] == k )  return ( true );

}

return ( false );

}

///////////////////////////////////////////////////////////////////////////////

void Cell::add(int k)

{

if ( has(k) )  return;

extend(n + 1);

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

Partition::Partition()

{

init_from_scratch();

}

///////////////////////////////////////////////////////////////////////////////

Partition::~Partition()

{

clear();

}

///////////////////////////////////////////////////////////////////////////////

Partition::Partition(const Partition & p)

{

init_from_scratch();

assign(p);

}

///////////////////////////////////////////////////////////////////////////////

Partition & Partition::operator=(const Partition & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}

///////////////////////////////////////////////////////////////////////////////


void Partition::init_from_scratch()

{

c = (Cell **) 0;

clear();

return;

}


///////////////////////////////////////////////////////////////////////////////

void Partition::clear()

{

int j;

if ( c )  {

   for (j=0; j<n_alloc; ++j)  {

     if ( c[j] )  { delete c[j];  c[j] = (Cell *) 0; }

   }

   delete [] c;  c = (Cell **) 0;

}

n = 0;

n_alloc = 0;

return;

}

///////////////////////////////////////////////////////////////////////////////

void Partition::assign(const Partition & p)

{

clear();

if ( !(p.c) )  return;

int j;

extend(p.n);

for (j=0; j<(p.n); ++j)  {

   c[j] = new Cell;

   *(c[j]) = *(p.c[j]);

}

n = p.n;

return;

}

///////////////////////////////////////////////////////////////////////////////


void Partition::extend(int N)

{

if ( N <= n_alloc )  return;

Cell ** u = (Cell **) 0;

N = partition_alloc_inc*((N + partition_alloc_inc - 1)/partition_alloc_inc);

u = new Cell * [N];

memset(u, 0, N*(sizeof(Cell *)));

if ( c )  {

   int j;

   for(j=0; j<n; ++j)  u[j] = c[j];

   delete [] c;  c = (Cell **) 0;

}


c = u;  u = (Cell **) 0;

n_alloc = N;



return;

}


///////////////////////////////////////////////////////////////////////////////

bool Partition::has(int k) const

{

int j;

for (j=0; j<n; ++j) {

   if ( c[j]->has(k) )  return ( true );

}

return ( false );

}

///////////////////////////////////////////////////////////////////////////////

int Partition::which_cell(int k) const

{

int j;

for (j=0; j<n; ++j) {

   if ( c[j]->has(k) )  return ( j );

}

return ( -1 );

}

///////////////////////////////////////////////////////////////////////////////

void Partition::merge_cells(int j_1, int j_2)

{

int k, nn;
int j_min, j_max;


if ( (j_1 < 0) || (j_1 >= n) || (j_2 < 0) || (j_2 >= n) ) {

   mlog << Error
        << "\nPartition::merge_cells() -> "
        << "range check error\n\n";

   exit(1);

}

if ( j_1 == j_2 )  return;



if ( j_1 < j_2 ) {

   j_min = j_1;
   j_max = j_2;

} else {

   j_min = j_2;
   j_max = j_1;

}

nn = c[j_max]->n;

for (k=0; k<nn; ++k) {

   c[j_min]->add(c[j_max]->e[k]);

}

*(c[j_max]) = *(c[n - 1]);

c[n - 1]->clear();

--n;

   //
   //  done
   //

return;

}

///////////////////////////////////////////////////////////////////////////////

void Partition::merge_values(int v1, int v2)

{

int j_1, j_2;

if ( v1 == v2 )  return;

j_1 = which_cell(v1);
j_2 = which_cell(v2);

if ( (j_1 < 0) || (j_2 < 0) ) {

   mlog << Error
        << "\nvoid Partition::merge_values() -> "
        << "bad values: (v1, v2) = (" << v1 << ", " << v2
        << "), (j1, j2) = (" << j_1 << ", " << j_2 << ")\n\n";

   return;

}

merge_cells(j_1, j_2);

   //
   //  done
   //

return;

}

///////////////////////////////////////////////////////////////////////////////

void Partition::add(int k)

{

if ( has(k) )  return;

extend(n + 1);

c[n] = new Cell;

c[n]->add(k);

++n;

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

double dot(double x_1, double y_1, double x_2, double y_2)

{

double d;

d = x_1*x_2 + y_1*y_2;

return ( d );

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
         if(sd.s_is_on(xn,   yn-1, false)) lr = true;
         if(sd.s_is_on(xn+1, yn-1, false)) ur = true;
         if(sd.s_is_on(xn+1, yn  , false)) ul = true;
         if(sd.s_is_on(xn,   yn  , false)) ll = true;

         xn += 1;
         break;

      case(plus_y):
         if(sd.s_is_on(xn,   yn  , false)) lr = true;
         if(sd.s_is_on(xn,   yn+1, false)) ur = true;
         if(sd.s_is_on(xn-1, yn+1, false)) ul = true;
         if(sd.s_is_on(xn-1, yn  , false)) ll = true;

         yn += 1;
         break;

      case(minus_x):
         if(sd.s_is_on(xn-1, yn  , false)) lr = true;
         if(sd.s_is_on(xn-2, yn  , false)) ur = true;
         if(sd.s_is_on(xn-2, yn-1, false)) ul = true;
         if(sd.s_is_on(xn-1, yn-1, false)) ll = true;

         xn -= 1;
         break;

      case(minus_y):
         if(sd.s_is_on(xn-1, yn-1, false)) lr = true;
         if(sd.s_is_on(xn-1, yn-2, false)) ur = true;
         if(sd.s_is_on(xn,   yn-2, false)) ul = true;
         if(sd.s_is_on(xn,   yn-1, false)) ll = true;

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

void ShapeData::threshold_attr(const map<ConcatString,ThreshArray> &attr_map,
                               const ShapeData *raw_ptr,
                               const SingleThresh &obj_thresh,
                               const Grid *grid,
                               bool precip_flag) {
   int i, j, n;
   ShapeData sd_split, sd_object;
   map<ConcatString,ThreshArray>::const_iterator it;
   double attr_val;

   // Split the field to number the shapes
   sd_split = split(*this, n);

   bool * keep_object = new bool [1 + n];  // keep_object[0] is ignored

   // Apply attribute filtering logic to each object
   for(i=1; i<=n; i++) {

      // Select the current object
      sd_object = select(sd_split, i);

      // Loop over attribute filter map
      for(it=attr_map.begin(); it!= attr_map.end(); it++) {

         attr_val = sd_object.get_attr(it->first, raw_ptr, obj_thresh, grid,
                                       precip_flag);

         // Discard objects whose attributes do not meet the threshold criteria
         for(j=0; j<it->second.n_elements(); j++) {

            keep_object[i] = it->second[j].check(attr_val);

            // Break out of the ThreshArray loop
            if(!keep_object[i]) {
               mlog << Debug(4)
                    << "Discarding object since " << it->first << " of "
                    << attr_val << " is not " << it->second[j].get_str()
                    << ".\n";
               break;
            }
         } // end for j

         // Break out of the attribute map loop
         if(!keep_object[i]) break;

      } // end for it
   } // end for i

   // Zero out discarded shapes
   int Nxy = data.nx()*data.ny();
   for(i=0; i<Nxy; i++) {
      if(!keep_object[nint(sd_split.data.buf()[i])]) data.buf()[i] = 0.0;
   }

   // Clean up
   if(keep_object) { delete [] keep_object; keep_object = (bool *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////


void ShapeData::threshold_area(SingleThresh t)

{

   int j, n, x, y, v_int;
   ShapeData sd_split, sd_object;
   const int nx = data.nx();
   const int ny = data.ny();

   // Split the field to number the shapes
   sd_split = split(*this, n);

   double * area_object = new double [1 + n];  // area_object[0] is ignored

   // Zero out area array
   for(j=0; j<=n; j++) area_object[j] = 0;   // want <= here, not <

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

   if ( area_object )  { delete [] area_object;  area_object = (double *) 0; }

   return;
}


///////////////////////////////////////////////////////////////////////////////


void ShapeData::threshold_intensity(const ShapeData *sd_ptr, int perc, SingleThresh t)

{
   int i, n, x, y, v_int, n_obj_inten;
   ShapeData s;
   double * obj_inten = (double *) 0, obj_inten_sum;
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

   double * inten_object = new double [1 + n];  // area_object[0] is ignored

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

if ( inten_object )  { delete [] inten_object;   inten_object = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////


ShapeData split(const ShapeData & wfd, int & n_shapes)

{

int k, x, y;
int s;
int xx, yy, nx, ny;
int current_shape;
bool shape_assigned = false;
ShapeData d;
ShapeData out = wfd;
ShapeData fat = wfd;
Partition p;


if ( do_split_fatten )  fat.expand(split_enlarge);


nx = fat.data.nx();
ny = fat.data.ny();

d.data.set_size(nx, ny);

n_shapes = 0;

   //
   //  shape numbers start at ONE here!!
   //

current_shape = 0;

for (y=(fat.data.ny() - 2); y>=0; --y) {

   for (x=(fat.data.nx() - 2); x>=0; --x) {

      s = fat.s_is_on(x, y);

      if ( !s ) continue;

      shape_assigned = false;

         //
         //  check above left
         //

      xx = x - 1;
      yy = y + 1;

      if ( (xx >= 0) && (yy < ny) ) {

         s = fat.s_is_on(xx, yy);

         if ( s ) {

            if ( shape_assigned )
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            else
               d.data.set(d.data(xx, yy), x, y);

            shape_assigned = true;

         }

      }

         //
         //  check above
         //

      xx = x;
      yy = y + 1;

      if ( yy < ny ) {

         s = fat.s_is_on(xx, yy);

         if ( s ) {

            if ( shape_assigned )
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            else
               d.data.set(d.data(xx, yy), x, y);

            shape_assigned = true;

         }

      }

         //
         //  check upper right
         //

      xx = x + 1;
      yy = y + 1;

      if ( (xx < nx) && (yy < ny) ) {

         s = fat.s_is_on(xx, yy);

         if ( s ) {

            if ( shape_assigned )
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            else
               d.data.set(d.data(xx, yy), x, y);

            shape_assigned = true;
         }
      }

         //
         //  check to the right
         //

      xx = x + 1;
      yy = y;

      if ( xx < nx ) {

         s = fat.s_is_on(xx, yy);

         if ( s ) {

            if ( shape_assigned )
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            else
               d.data.set(d.data(xx, yy), x, y);

            shape_assigned = true;
         }
      }

         //
         //  is it a new shape?
         //

      if ( !shape_assigned ) {

         d.data.set(++current_shape, x, y);

         p.add(nint(d.data(x, y)));

      }

   } // for x

} // for y


if ( do_split_fatten )  d.shrink(split_enlarge);


     ///////////////////////////////////

nx = wfd.data.nx();
ny = wfd.data.ny();

for (x=0; x<nx; ++x) {

   for (y=0; y<ny; ++y) {

      out.data.set(0, x, y);

      for (k=0; k<(p.n); ++k) {

         if ( p.c[k]->has(nint(d.data(x, y))) )   out.data.set(k + 1, x, y);

      }

   } // for y

} // for x


     ///////////////////////////////////


n_shapes = p.n;

out.calc_moments();

   //
   //  done
   //

return ( out );

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
