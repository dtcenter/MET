// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
//   000    05/31/11  Halley Gotway  Adapated from wrfdata.cc
//   001    05/29/14  Halley Gotway  Add ShapeData::n_objects()
//   002    11/02/23  Halley Gotway  MET #2724 add OpenMP to convolution
//
///////////////////////////////////////////////////////////////////////////////


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
#include <numeric>
#include <algorithm>

#include "shapedata.h"
#include "mode_columns.h"
#include "enum_as_int.hpp"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"

#include "ihull.h"

using namespace std;


///////////////////////////////////////////////////////////////////////////////


static const int split_enlarge = 4;   //  used for ShapeData  shrink and expand

static const bool do_split_fatten = true;


///////////////////////////////////////////////////////////////////////////////


#define  STANDARD_XY_YO_N(Nx, x, y) ((y)*(Nx) + (x))


///////////////////////////////////////////////////////////////////////////////

static double dot(double, double, double, double);
static void   boundary_step(const ShapeData &, int &, int &, int &);
static StepCase get_step_case(bool, bool, bool, bool);

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

   if ( this == &f )  return *this;

   assign(f);

   return *this;
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

   for(int x=0; x<data.nx(); x++) {
      if(f_is_on(x, y)) return x;
   }

   return -1;
}

////////////////////////////////////////////////////////////////////////

int ShapeData::x_right(int y) const {

   if((y < 0) || (y >= data.ny())) {
      mlog << Error << "\nShapeData::x_right(int) -> "
           << "range check error\n\n";
      exit(1);
   }

   for(int x=(data.nx() - 1); x>=0; x--) {
      if(f_is_on(x, y)) return x;
   }

   return -1;
}

///////////////////////////////////////////////////////////////////////////////

bool ShapeData::s_is_on(int x, int y, bool error_out) const

{

   // Unless error out is true, return bad status for being off the grid

   if(!error_out) {
      if(x < 0 || x >= data.nx() || y < 0 || y >= data.ny()) return false;
   }

   // Check if the current point is non-zero

   return data(x, y) > 0.0;

}


///////////////////////////////////////////////////////////////////////////////

bool ShapeData::f_is_on(int x, int y) const

{

   // Check if the current point or any of of it's neighbors are non-zero

   if(s_is_on(x, y))                            return true;
   if((x > 0) && s_is_on(x-1, y))               return true;
   if((x > 0) && (y > 0) && s_is_on(x-1, y-1))  return true;
   if((y > 0) && s_is_on(x, y-1))               return true;

   return false;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::calc_moments() {

   int s_area = 0;
   int f_area = 0;
   vector<double> psum(9, 0.0);

#pragma omp declare reduction(vec_dbl_plus : vector<double> :             \
                              transform(omp_out.begin(), omp_out.end(),   \
                                         omp_in.begin(), omp_out.begin(), \
                                        plus<double>()))                  \
                    initializer(omp_priv = decltype(omp_orig)(omp_orig.size()))

#pragma omp parallel default(none)    \
   shared(data, s_area, f_area, psum)
   {

#pragma omp for reduction(+: s_area, f_area)   \
                reduction(vec_dbl_plus : psum)
      for(int x=0; x<data.nx(); ++x) {

         auto xx = ((double) x);

         for(int y=0; y<data.ny(); ++y) {

            auto yy = ((double) y);

            // Object area based on s_is_on() logic
            if(s_is_on(x, y)) s_area += 1;

            if(f_is_on(x, y)) {

               f_area += 1;

               psum[0] += xx;
               psum[1] += yy;

               psum[2] += xx*xx;
               psum[3] += xx*yy;
               psum[4] += yy*yy;

               psum[5] += xx*xx*xx;
               psum[6] += xx*xx*yy;
               psum[7] += xx*yy*yy;
               psum[8] += yy*yy*yy;
            }
         } // for y
      } // for x
   } // End omp parallel

   // Store result
   mom.s_area = s_area;
   mom.f_area = f_area;
   mom.sx     = psum[0];
   mom.sy     = psum[1];
   mom.sxx    = psum[2];
   mom.sxy    = psum[3];
   mom.syy    = psum[4];
   mom.sxxx   = psum[5];
   mom.sxxy   = psum[6];
   mom.sxyy   = psum[7];
   mom.syyy   = psum[8];

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::centroid(double &xbar, double &ybar) const {

   mom.centroid(xbar, ybar);

   return;
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::angle_degrees() const {

   return mom.angle_degrees();
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::curvature(double &xcurv, double &ycurv) const {

   return mom.curvature(xcurv, ycurv);
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::area() const {

   double x = (double) (mom.s_area);

   return x;
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::area_thresh(const ShapeData *raw_ptr,
                              const SingleThresh &obj_thresh) const {
   int cur_area = 0;
   const int Nxy = data.nx()*data.ny();

   // Number of points inside the object that meet the threshold criteria
#pragma omp parallel default(none)                  \
   shared(raw_ptr, obj_thresh, Nxy, data, cur_area)
   {

#pragma omp for reduction(+: cur_area)
      for(int i=0; i<Nxy; i++) {
         if(data.data()[i] > 0 &&
            obj_thresh.check(raw_ptr->data.data()[i])) cur_area++;
      }
   } // End omp parallel

   return cur_area;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::calc_length_width(double &l, double &w) const {
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

   for (int x=0; x<data.nx(); ++x) {
      for (int y=0; y<data.ny(); ++y) {

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

   return l;
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::width() const {
   double l, w;

   calc_length_width(l, w);

   return w;
}

////////////////////////////////////////////////////////////////////////

double ShapeData::complexity() const {

   int count = nint(mom.s_area);

   if(count == 0) {
      mlog << Error << "\nShapeData::complexity() const -> "
           << "empty shape!\n\n";
      exit(1);
   }

   Polyline poly(convex_hull());
   double hull  = fabs(poly.uv_signed_area());
   auto shape = (double) count;

   //
   // Complexity is defined as the difference in area between the
   // convex hull and the original shape divided by the area of the
   // convex hull.  0 <= Complexity < 1, and complexity = 0 indicates
   // that the shape is convex.
   //
   double u = 0.0;
   if(!is_eq(hull, 0.0)) u = (hull - shape)/hull;

   return u;
}

///////////////////////////////////////////////////////////////////////////////

double ShapeData::intensity_percentile(const ShapeData *raw_ptr, int perc,
                                       bool precip_flag) const {

   if(perc < 0 || perc > 102) {
      mlog << Error << "\nShapeData::intensity_percentile() -> "
           << "the intensity percentile requested must be between 0 and 102.\n\n";
      exit(1);
   }

   const int Nxy = data.nx()*data.ny();
   vector<double> val;
   val.reserve(Nxy);

   // Compute the requested percentile of intensity
   for(int i=0; i<Nxy; i++) {

      // Process points for the current object
      if(data.data()[i] > 0) {

         double v = raw_ptr->data.data()[i];

         // Skip bad data and zero precip
         if(::is_bad_data(v) || (precip_flag && is_eq(v, 0.0))) continue;

         // Store current value
         val.emplace_back(v);
      }
   }

   // Compute the mean of the intensities
   double v = 0.0;
   if(perc == 101) {
      v = accumulate(val.begin(), val.end(), 0.0)/(double) val.size();
   }
   // Compute the sum of the intensities
   else if(perc == 102) {
      v = accumulate(val.begin(), val.end(), 0.0);
   }
   // Compute a percentile of intensity
   else {
      sort(val.begin(), val.end());
      v = percentile(val.data(), (int) val.size(), (double) perc/100.0);
   }

   return v;
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
   else if(strncasecmp(attr_name.c_str(), "INTENSITY_", m_strlen("INTENSITY_")) == 0) {
      StringArray sa = attr_name.split("_");
      attr_val = intensity_percentile(raw_ptr, atoi(sa[1].c_str()), precip_flag);
   }
   else {
      mlog << Warning << "\nShapeData::get_attr() -> "
           << "Filtering requested for unsupported object attribute \""
           << attr_name << "\".\n\n";
      attr_val = bad_data_double;
   }

   return attr_val;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::conv_filter_circ(int diameter, double vld_thresh) {
   const char *method_name = "ShapeData::conv_filter_circ() -> ";
   GridPoint *gp = nullptr;
   int count;
   int n_vld;
   double v;
   double v_sum;
   DataPlane conv_dp;

   // Check the diameter
   if(diameter%2 == 0 || diameter < 3) {
      mlog << Error << "\n" << method_name
           << "diameter must be odd and >= 3 ... diameter = "
           << diameter << "\n\n";
      exit(1);
   }

#pragma omp parallel default(none)                   \
   shared(mlog, data, conv_dp, diameter, vld_thresh) \
   private(count, n_vld, v, v_sum, gp)
   {

      // Build the grid template with shape circle and wrap_lon false
      GridTemplateFactory gtf;
      GridTemplate* gt = gtf.buildGT(GridTemplateFactory::GridTemplates::Circle,
                                     diameter, false);

#pragma omp single
      {
         // Initialize the convolved field to bad data
         conv_dp = data;
         conv_dp.set_constant(bad_data_double);
      }

      // Compute the convolved values
#pragma omp for schedule (static)
      for(int x=0; x<data.nx(); x++) {
         for(int y=0; y<data.ny(); y++) {

            // For a new column, reset the grid template and counts
            if(y == 0) {

               // Initialize counts and sum
               count = 0;
               n_vld = 0;
               v_sum = 0.0;

               // Sum all the points
               for(gp  = gt->getFirstInGrid(x, y, data.nx(), data.ny());
                   gp != nullptr;
                   gp  = gt->getNextInGrid()) {
                  v = data.get(gp->x, gp->y);
                  count += 1;
                  if(::is_bad_data(v)) continue;
                  n_vld += 1;
                  v_sum += v;
               }
            }
            // Subtract off the bottom edge, shift up, and add the top
            else {

               // Subtract points from the the bottom edge
               for(gp  = gt->getFirstInBotEdge();
                   gp != nullptr;
                   gp  = gt->getNextInBotEdge()) {
                  v = data.get(gp->x, gp->y);
                  count -= 1;
                  if(::is_bad_data(v)) continue;
                  n_vld -= 1;
                  v_sum -= v;
               }

               // Increment Y
               gt->incBaseY(1);

               // Add points from the the top edge
               for(gp  = gt->getFirstInTopEdge();
                   gp != nullptr;
                   gp  = gt->getNextInTopEdge()) {
                  v = data.get(gp->x, gp->y);
                  count += 1;
                  if(::is_bad_data(v)) continue;
                  n_vld += 1;
                  v_sum += v;
               }
            }

            //  If the center of the convolution contains bad data and the ratio
            //  of bad data in the convolution area is too high, set the convoled
            //  value to bad data.
            if(count == 0 || n_vld == 0)                v = bad_data_double;
            else if(::is_bad_data(data.get(x, y)) &&
                    (double)(n_vld)/count < vld_thresh) v = bad_data_double;
            else                                        v = (double) v_sum/n_vld;
            conv_dp.set(v, x, y);

         } // end for y

         // Increment X
         if(x < (data.nx() - 1)) gt->incBaseX(1);

      } // end for x

      delete gt;

   } // End of omp parallel

   // Save the result
   data = conv_dp;

   return;
}

////////////////////////////////////////////////////////////////////////

Polyline ShapeData::convex_hull() const {

   vector<IntPoint> in(2*(data.ny() + 1));
   int n_in = 0;

   for(int y=0; y<(data.ny()); y++) {

      int j = x_left(y);

      if(j < 0) continue;

      in[n_in].x = j;
      in[n_in].y = y;

      n_in++;

      int k = x_right(y);

      if(k < 0) continue;

      if(j == k) continue;

      in[n_in].x = k;
      in[n_in].y = y;

      n_in++;

   }   //  for y

   vector<IntPoint> out(n_in + 2);

   int n_out;
   ihull(in.data(), n_in, out.data(), n_out);

   Polyline hull_poly;
   hull_poly.extend_points(n_out);

   for(int j=0; j<n_out; j++) {
      hull_poly.add_point(out[j].x, out[j].y);
   }

   //
   //  done
   //

   return hull_poly;
}

///////////////////////////////////////////////////////////////////////////////

Polyline ShapeData::single_boundary() const {

   //
   // Call boundary function with all_points set to false
   // and clockwise set to true
   //

   return single_boundary(false, 1);
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

   return single_boundary_offset(all_points, clockwise, 0.0);
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

   return single_boundary_offset(false, 1, d);
}

///////////////////////////////////////////////////////////////////////////////
//
//  ShapeData::single_boundary_offset() should only be called for split fields
//  containing only one object.
//
///////////////////////////////////////////////////////////////////////////////

Polyline ShapeData::single_boundary_offset(bool all_points, int clockwise,
                                           double d) const {
   Polyline boundary;

   //
   // Find the first point in the object
   //
   int x0 = 0;
   int y0 = 0;
   bool found = false;
   for(int x=0; x<data.nx(); x++) {
      for(int y=0; y<data.ny(); y++) {

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

      return boundary;
   }

   //
   // Due to the search order, the initial direction will be plus_x
   //
   int direction = plus_x;
   int new_direction = direction;
   boundary.add_point(x0+d, y0+d);

   //
   // Initialize xn and yn to starting point
   //
   int xn = x0;
   int yn = y0;

   //
   // Find next point along boundary
   //
   boundary_step(*this, xn, yn, new_direction);

   //
   // Store only points where a change of direction occurs
   // or all points if so indicated
   //
   if(all_points ||
      (!all_points && direction != new_direction)) {
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
      if(all_points ||
         (!all_points && direction != new_direction)) {
         boundary.add_point(xn+d, yn+d);
      }
      direction = new_direction;
   }

   //
   // If indicated, reverse the direction of the points from clockwise to
   // counter-clockwise
   //
   if(!clockwise) {
      Polyline temp(boundary);
      boundary.clear();

      for(int i=temp.n_points-1; i>=0; i--) {
         boundary.add_point(temp.u[i], temp.v[i]);
      }
   }

   return boundary;
}

///////////////////////////////////////////////////////////////////////////////
//
//  zero_field()
//  This routine will zero out all of the data in the in the field
//
///////////////////////////////////////////////////////////////////////////////

void ShapeData::zero_field() {

   data.set_constant(0.0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::expand(const int W) {

   if(W <= 0) {
      mlog << Error << "\nShapeData::expand(const int) -> "
           << "bad value ... " << W << "\n\n";
      exit(1);
   }

   const int nx_old = data.nx();
   const int ny_old = data.ny();
   DataPlane old(data);

   const int nx_new = nx_old + 2*W;
   const int ny_new = ny_old + 2*W;
   data.set_size(nx_new, ny_new);
   data.set_constant(0.0);

   for(int x_old=0; x_old<nx_old; x_old++) {
      int x_new = x_old + W;
      for(int y_old=0; y_old<ny_old; y_old++) {
         int y_new = y_old + W;
         data.put(old.get(x_old, y_old), x_new, y_new);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::shrink(const int W) {

   if(W <= 0) {
      mlog << Error << "\nShapeData::shrink(const int) -> "
           << "bad value ... " << W << "\n\n";
      exit(1);
   }

   const int nx_old = data.nx();
   const int ny_old = data.ny();
   const int nx_new = nx_old - 2*W;
   const int ny_new = ny_old - 2*W;

   if(nx_new <= 0 || ny_new <= 0) {
      mlog << Error << "\nShapeData::shrink(const int) -> "
           << "value too large ... new grid is empty\n\n";
      exit(1);
   }

   DataPlane old(data);

   for(int x_new=0; x_new<nx_new; x_new++) {
      int x_old = x_new + W;
      for(int y_new=0; y_new<ny_new; y_new++) {
         int y_old = y_new + W;
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

Cell::Cell() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

Cell::~Cell() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

Cell::Cell(const Cell & c) {
   assign(c);
}

///////////////////////////////////////////////////////////////////////////////

Cell & Cell::operator=(const Cell & c) {

   if(this == &c) return *this;

   assign(c);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

void Cell::clear() {

   e.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Cell::assign(const Cell & c) {

   e = c.e;

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool Cell::has(int k) const {
 
   return any_of(e.begin(), e.end(), [k](int j){ return j == k; });
}

///////////////////////////////////////////////////////////////////////////////

void Cell::add(int k) {

   if(!has(k)) e.emplace_back(k);

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

Partition::~Partition() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

Partition::Partition(const Partition & p) {
   assign(p);
}

///////////////////////////////////////////////////////////////////////////////

Partition & Partition::operator=(const Partition & p) {

   if(this == &p) return *this;

   assign(p);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::clear() {

   c.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::assign(const Partition & p) {

   c = p.c;

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool Partition::has(int k) const {

   return any_of(c.begin(), c.end(), [k](Cell e){ return e.has(k); });
}

///////////////////////////////////////////////////////////////////////////////

int Partition::which_cell(int k) const {

   for(int j=0; j<c.size(); j++) {
      if(c[j].has(k)) return j;
   }

   return -1;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::merge_cells(int j_1, int j_2) {

   if(j_1 < 0 || j_1 >= c.size() || j_2 < 0 || j_2 >= c.size()) {
      mlog << Error << "\nPartition::merge_cells() -> "
           << "range check error\n\n";
      exit(1);
   }

   if( j_1 == j_2) return;

   int j_min;
   int j_max;

   if(j_1 < j_2) {
      j_min = j_1;
      j_max = j_2;
   }
   else {
      j_min = j_2;
      j_max = j_1;
   }

   auto nn = (int) c[j_max].e.size();

   for(int k=0; k<nn; k++) {
      c[j_min].add(c[j_max].e[k]);
   }

   c[j_max] = c.back();
   c.pop_back();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::merge_values(int v1, int v2) {

   if(v1 == v2) return;

   int j_1 = which_cell(v1);
   int j_2 = which_cell(v2);

   if(j_1 < 0 || j_2 < 0) {
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

   if(has(k)) return;

   Cell new_c;
   new_c.add(k);

   c.emplace_back(new_c);

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

static double dot(double x_1, double y_1, double x_2, double y_2) {
   return x_1*x_2 + y_1*y_2;
}

///////////////////////////////////////////////////////////////////////////////

static void boundary_step(const ShapeData &sd, int &xn, int &yn, int &direction) {
   bool lr, ur, ul, ll;

   lr = ur = ul = ll = false;

   //
   // Based on the direction of travel turn on/off lr, ur, ul, ll cells
   //
   switch(direction) {

      case plus_x:
         if(sd.s_is_on(xn,   yn-1, false)) lr = true;
         if(sd.s_is_on(xn+1, yn-1, false)) ur = true;
         if(sd.s_is_on(xn+1, yn  , false)) ul = true;
         if(sd.s_is_on(xn,   yn  , false)) ll = true;

         xn += 1;
         break;

      case plus_y:
         if(sd.s_is_on(xn,   yn  , false)) lr = true;
         if(sd.s_is_on(xn,   yn+1, false)) ur = true;
         if(sd.s_is_on(xn-1, yn+1, false)) ul = true;
         if(sd.s_is_on(xn-1, yn  , false)) ll = true;

         yn += 1;
         break;

      case minus_x:
         if(sd.s_is_on(xn-1, yn  , false)) lr = true;
         if(sd.s_is_on(xn-2, yn  , false)) ur = true;
         if(sd.s_is_on(xn-2, yn-1, false)) ul = true;
         if(sd.s_is_on(xn-1, yn-1, false)) ll = true;

         xn -= 1;
         break;

      case minus_y:
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
   }

   //
   // Determine the direction to head by the combination of lr, ur, ul, and ll
   //
   switch(get_step_case(lr, ur, ul, ll)) {

      case StepCase::ll_case:
      case StepCase::lr_ul_case:
      case StepCase::lr_ur_ul_case:
         // Turn left
         direction = (direction + 1)%4;
         if(direction < 0) direction += 4;
         break;

      case StepCase::lr_case:
      case StepCase::ur_ll_case:
      case StepCase::ur_ul_ll_case:
         // Turn right
         direction = (direction - 1)%4;
         if(direction < 0) direction += 4;
         break;

      case StepCase::ul_ll_case:
      case StepCase::lr_ur_case:
         // Continue straight: direction remains unchanged
         break;

      default:

         mlog << Error << "\nboundary_step() -> "
              << "bad step case: "
              << enum_class_as_int(get_step_case(lr, ur, ul, ll)) << "\n\n";
         exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

StepCase get_step_case(bool lr, bool ur, bool ul, bool ll) {

   //
   // Valid cases with exactly one cell on
   //

   // Lower Left
   if(!lr && !ur && !ul && ll) return StepCase::ll_case;
   // Lower Right
   else if(lr && !ur && !ul && !ll) return StepCase::lr_case;

   //
   // Valid cases with exactly two cells on
   //

   // Upper Left, Lower Left
   else if(!lr && !ur && ul && ll) return StepCase::ul_ll_case;
   // Lower Right, Upper Right
   else if(lr && ur && !ul && !ll) return StepCase::lr_ur_case;
   // Lower Right, Upper Left
   else if(lr && !ur && ul && !ll) return StepCase::lr_ul_case;
   // Upper Right, Lower Left
   else if(!lr && ur && !ul && ll) return StepCase::ur_ll_case;

   //
   // Valid cases with exactly three cells on
   //

   // Upper Right, Upper Left, Lower Left
   else if(!lr && ur && ul && ll) return StepCase::ur_ul_ll_case;
   // Lower Right, Upper Right, Upper Left
   else if(lr && ur && ul && !ll) return StepCase::lr_ur_ul_case;

   //
   // Otherwise, combination is invalid
   //
   else {
      mlog << Error << "\nget_step_case() -> "
           << "invalid combination: (lr, ur, ul, ll) = (" << lr << ", "
           << ur << ", " << ul << ", " << ll << ")\n\n";
      exit(1);
   }

}

///////////////////////////////////////////////////////////////////////////////

void apply_mask(ShapeData &f, const ShapeData &mask) {

   if(f.data.nx() != mask.data.nx() ||
      f.data.ny() != mask.data.ny() ) {
      mlog << Error << "\napply_mask() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

#pragma omp parallel default(none) \
   shared(f, mask, bad_data_float)
   {

      //
      // Put bad data everywhere the mask is turned off
      //
#pragma omp for schedule(static)
      for(int x=0; x<f.data.nx(); x++) {
         for(int y=0; y<f.data.ny(); y++) {
            if(!mask.s_is_on(x, y)) f.data.set(bad_data_float, x, y);
         }
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

int ShapeData::n_objects() const {

   // Split the field to number the shapes
   int n;
   ShapeData sd_split(split(*this, n));

   return n;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::threshold(SingleThresh t) {

   //
   // Compare the threshold double value to the double values for the
   // ShapeData field
   //
   int nxy = data.nxy();

#pragma omp parallel default(none) \
   shared(t, data, nxy)
   {

#pragma omp for schedule(static)
      for(int j=0; j<nxy; j++) {

         double v = data.buf()[j];

         data.buf()[j] = (t.check(v) && ! ::is_bad_data(v) ? 1.0 : 0.0);
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::set_to_1_or_0() {

   int nxy = data.nxy();

#pragma omp parallel default(none) \
   shared(data, nxy)
   {

#pragma omp for schedule(static)
      for(int j=0; j<nxy; j++) {

         double v = data.buf()[j];

         data.buf()[j] = (::is_bad_data(v) ? 0.0 : 1.0);
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::threshold_attr(const map<ConcatString,ThreshArray> &attr_map,
                               const ShapeData *raw_ptr,
                               const SingleThresh &obj_thresh,
                               const Grid *grid,
                               bool precip_flag) {

   // Split the field to number the shapes
   int n;
   ShapeData sd_split(split(*this, n));

   vector<bool> keep_object(n+1);  // keep_object[0] is ignored

   // Apply attribute filtering logic to each object
   for(int i=1; i<=n; i++) {

      // Select the current object
      ShapeData sd_object(select(sd_split, i));
      keep_object[i] = true;

      // Loop over attribute filter map
      for(auto it : attr_map) {

         double attr_val = sd_object.get_attr(it.first, raw_ptr, obj_thresh,
                                              grid, precip_flag);

         // Discard objects whose attributes do not meet the threshold criteria
         for(int j=0; j<it.second.n_elements(); j++) {

            keep_object[i] = it.second[j].check(attr_val);

            // Break out of the ThreshArray loop
            if(!keep_object[i]) {
               mlog << Debug(4)
                    << "Discarding object since " << it.first << " of "
                    << attr_val << " is not " << it.second[j].get_str()
                    << ".\n";
               break;
            }
         } // end for j

         // Break out of the attribute map loop
         if(!keep_object[i]) break;

      } // end for it
   } // end for i

   // Zero out discarded shapes
   int nxy = data.nx()*data.ny();

#pragma omp parallel default(none)          \
   shared(keep_object, sd_split, data, nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<nxy; i++) {
         int obj_id = nint(sd_split.data.buf()[i]);
         if(!keep_object[obj_id]) data.buf()[i] = 0.0;
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::threshold_area(SingleThresh t) {

   // Split the field to number the shapes
   int n;
   ShapeData sd_split(split(*this, n));

   vector<double> area_object(n+1);  // area_object[0] is ignored

   //
   // Compute the area of each object
   //
   for(int j=1; j<=n; j++) {
      ShapeData sd_object(select(sd_split, j));
      area_object[j] = sd_object.area();
   }

   //
   // Zero out any shapes with an area that doesn't meet the
   // threshold criteria
   //
   int nxy = data.nx()*data.ny();

#pragma omp parallel default(none)             \
   shared(t, area_object, sd_split, data, nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<nxy; i++) {
         int obj_id = nint(sd_split.data.buf()[i]);
         if(!t.check(area_object[obj_id])) data.buf()[i] = 0.0;
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::threshold_intensity(const ShapeData *sd_ptr, int perc,
                                    SingleThresh t) {

   if(perc < 0 || perc > 102) {
      mlog << Error << "\nShapeData:threshold_intensity() -> "
           << "the intensity percentile requested must be between 0 and 102.\n\n";
      exit(1);
   }

   //
   // Split the field to number the shapes
   //
   int n;
   ShapeData sd_split(split(*this, n));

   vector<double> inten_object(n+1);  // inten_object[0] is ignored

   int nx = data.nx();
   int ny = data.ny();

   //
   // For each object, compute the requested percentile of intensity
   //
   for(int i=0; i<n; i++) {
   
      vector<double> raw_v;
      raw_v.reserve(nx*ny);

      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {

            int obj_id = nint(sd_split.data(x, y));

            if(obj_id != i+1) continue;

            if(sd_ptr->is_valid_xy(x, y)) {
               raw_v.emplace_back(sd_ptr->data(x, y));
            }
         } // end for y
      } // end for x

      //
      // Compute the mean of the intensities
      //
      if(perc == 101) {
         inten_object[i+1] = accumulate(raw_v.begin(), raw_v.end(), 0.0)/(double) raw_v.size();
      }
      //
      // Compute the sum of the intensities
      //
      else if(perc == 102) {
         inten_object[i+1] = accumulate(raw_v.begin(), raw_v.end(), 0.0);
      }
      //
      // Compute a percentile of intensity
      //
      else {
         sort(raw_v.begin(), raw_v.end());
         inten_object[i+1] = percentile(raw_v.data(), (int) raw_v.size(), (double) perc/100.0);
      }
   }

   //
   // Zero out any shapes with an intensity that doesn't meet the
   // threshold criteria
   //
   int nxy = data.nx()*data.ny();

#pragma omp parallel default(none)             \
   shared(t, inten_object, sd_split, data, nxy)
   {

#pragma omp for schedule(static)
      for(int i=0; i<nxy; i++) {
         int obj_id = nint(sd_split.data.buf()[i]);
         if(!t.check(inten_object[obj_id])) data.buf()[i] = 0.0;
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

ShapeData split(const ShapeData & wfd, int & n_shapes) {
   int current_shape;
   bool shape_assigned = false;
   ShapeData out(wfd);
   ShapeData fat(wfd);
   Partition p;

   if(do_split_fatten) fat.expand(split_enlarge);

   int nx = fat.data.nx();
   int ny = fat.data.ny();

   ShapeData d;
   d.data.set_size(nx, ny);

   n_shapes = 0;

   //
   // Shape numbers start at ONE here!!
   //
   current_shape = 0;

   for(int y=(fat.data.ny() - 2); y>=0; --y) {
      for(int x=(fat.data.nx() - 2); x>=0; --x) {

         if(!fat.s_is_on(x,y)) continue;

         shape_assigned = false;

         //
         // Check above left
         //
         int xx = x - 1;
         int yy = y + 1;

         if(xx >= 0 &&
            yy < ny &&
            fat.s_is_on(xx, yy)) {

            if(shape_assigned) {
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            }
            else {
               d.data.set(d.data(xx, yy), x, y);
            }
            shape_assigned = true;
         }

         //
         // Check above
         //
         xx = x;
         yy = y + 1;

         if(yy < ny &&
            fat.s_is_on(xx, yy)) {

            if(shape_assigned) {
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            }
            else {
               d.data.set(d.data(xx, yy), x, y);
            }
            shape_assigned = true;
         }

         //
         // Check upper right
         //
         xx = x + 1;
         yy = y + 1;

         if(xx < nx &&
            yy < ny &&
            fat.s_is_on(xx, yy)) {

            if(shape_assigned) {
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            }
            else {
               d.data.set(d.data(xx, yy), x, y);
            }
            shape_assigned = true;
         }

         //
         // Check to the right
         //
         xx = x + 1;
         yy = y;

         if(xx < nx &&
            fat.s_is_on(xx, yy)) {

            if(shape_assigned) {
               p.merge_values(nint(d.data(x, y)), nint(d.data(xx, yy)));
            }
            else {
               d.data.set(d.data(xx, yy), x, y);
            }
            shape_assigned = true;
         }

         //
         // Is it a new shape?
         //

         if(!shape_assigned) {
            current_shape++;
            d.data.set(current_shape, x, y);
            p.add(nint(d.data(x, y)));
         }
      } // for x
   } // for y

   if(do_split_fatten) d.shrink(split_enlarge);

     ///////////////////////////////////

   nx = wfd.data.nx();
   ny = wfd.data.ny();

   for(int x=0; x<nx; x++) {
      for(int y=0; y<ny; y++) {

         out.data.set(0, x, y);

         for(int k=0; k<(p.c.size()); k++) {
            if(p.c[k].has(nint(d.data(x, y)))) {
               out.data.set(k + 1, x, y);
            }
         }
      } // for y
   } // for x

     ///////////////////////////////////

   n_shapes = (int) p.c.size();

   out.calc_moments();

   //
   //  done
   //

   return out;
}

///////////////////////////////////////////////////////////////////////////////

ShapeData select(const ShapeData &id, int n) {

   ShapeData d(id);
   int nxy = id.data.nxy();
 
#pragma omp parallel default(none) \
   shared(id, d, n, nxy)
   {

#pragma for schedule(static)
      for(int j=0; j<nxy; j++) {
         int obj_id = nint(id.data.data()[j]);
         if(obj_id == n) d.data.buf()[j] = 1;
         else            d.data.buf()[j] = 0;
      }
   } // End omp parallel

   d.calc_moments();

   return d;
}

///////////////////////////////////////////////////////////////////////////////

void ShapeData::filter(SingleThresh t) {

   int nxy = data.nxy();

#pragma omp parallel default(none) \
   shared(t, data, nxy)
   {

#pragma omp for schedule(static)
      for(int j=0; j<nxy; j++) {

         double v = data.buf()[j];

         if(!t.check(v) && ! ::is_bad_data(v)) {
            data.buf()[j] = 0.0;
         }
      }
   } // End omp parallel

   return;
}

///////////////////////////////////////////////////////////////////////////////

int ShapeData_intersection(const ShapeData &f1, const ShapeData &f2) {

   //
   // Check for the same grid dimension
   //
   if(f1.data.nx() != f2.data.nx() ||
      f1.data.ny() != f2.data.ny() ) {
      mlog << Error << "\nShapeData_intersection() -> "
           << "grid dimensions do not match\n\n";
      exit(1);
   }

   int intersection = 0;

#pragma omp parallel default(none) \
   shared(f1, f2, intersection)
   {

#pragma omp for reduction(+: intersection)   
      for(int x=0; x<f1.data.nx(); x++) {
         for(int y=0; y<f1.data.ny(); y++) {
            if(f1.s_is_on(x, y) && f2.s_is_on(x, y)) intersection++;
         }
      }
   } // End omp parallel

   return intersection;
}

///////////////////////////////////////////////////////////////////////////////
//
//  End Code for Miscellaneous Functions
//
///////////////////////////////////////////////////////////////////////////////
