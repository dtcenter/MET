// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class Polyline
//
///////////////////////////////////////////////////////////////////////////////

Polyline::Polyline() {
   u = v = (double *) 0;

   clear();
}

///////////////////////////////////////////////////////////////////////////////

Polyline::~Polyline() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

Polyline::Polyline(const Polyline &c) {
   u = v = (double *) 0;

   assign(c);
}

///////////////////////////////////////////////////////////////////////////////

Polyline & Polyline::operator=(const Polyline &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::clear() {

   if(u)    { delete [] u;    u = (double *) 0; }
   if(v)    { delete [] v;    v = (double *) 0; }

   n_points = n_alloc = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::assign(const Polyline &c) {
   int i;

   clear();

   set_name(c.name);

   if(c.n_points == 0) return;

   extend_points(c.n_points);

   n_alloc = n_points = c.n_points;

   for(i=0; i<n_points; i++) {

      u[i] = c.u[i];
      v[i] = c.v[i];
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////


void Polyline::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);
Indent p2(depth + 1);

out << prefix << "n_points = " << n_points << "\n";
out << prefix << "n_alloc  = " << n_alloc  << "\n";

for (j=0; j<n_points; ++j)  {

   out << p2 << "point # " << j << " ... ("
       << u[j] << ", " << v[j] << ")\n";

   if ( (j%5) == 4 )  out << p2 << "\n";

}



   //
   //  done
   //

out.flush();

return;

}


///////////////////////////////////////////////////////////////////////////////

void Polyline::set_name(std::string n) {

   if(!n.empty()) {

      name = n;

      if(name.empty()) {
         mlog << Error << "\nPolyline::set_name(const char *) -> "
              << "memory allocation error 1" << "\n\n";

         exit(1);
      }

   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::add_point(double uu, double vv) {

   extend_points(n_points + 1);

   u[n_points] = uu;
   v[n_points] = vv;

   n_points++;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::extend_points(int n) {

   if(n_alloc >= n) return;

   if(n_alloc == 0) {

      u = new double [n];
      v = new double [n];

      if(!u || !v) {
         mlog << Error << "\nPolyline::extend_points(int) -> "
	      << "memory allocation error 1" << "\n\n";

         exit(1);
      }

      n_alloc = n;

      n_points = 0;

      return;
   }

   int i;
   double *uu = (double *) 0;
   double *vv = (double *) 0;

   uu = new double [n];
   vv = new double [n];

   if(!uu || !vv) {
      mlog << Error << "\nPolyline::extend_points(int) -> "
           << "memory allocation error 2" << "\n\n";

      exit(1);
   }

   for(i=0; i<n_points; i++) {

      uu[i] = u[i];
      vv[i] = v[i];
   }

   delete [] u; u = (double *) 0;
   delete [] v; v = (double *) 0;

   u = uu;
   v = vv;

   uu = vv = (double *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

int Polyline::is_closed() const {
   int closed;

   // Consider an empty Polyline closed
   if(n_points == 0) {

      closed = 1;
   }

   // Non-empty Polyline must have more than 2 points to be closed
   else if( n_points > 0 && n_points < 3) {

      closed = 0;
   }

   else  {

      double ds = fabs(u[n_points - 1] - u[0]) + fabs(v[n_points - 1] - v[0]);

      if(ds < 1.0e-4) closed = 1;
      else            closed = 0;
   }

   return(closed);
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::centroid(double &ubar, double &vbar) const {
   double sa, sum_x, sum_y;

   sum_x = sum_y = 0.0;

   sum_first_moments(sum_x, sum_y);

   sum_x /= 6.0;
   sum_y /= 6.0;

   sa = uv_signed_area();

   ubar = sum_x/sa;
   vbar = sum_y/sa;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::translate(double du, double dv) {
   int i;

   // Translate each point of the current polyline
   for(i=0; i<n_points; i++) {

      u[i] += du;
      v[i] += dv;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double Polyline::angle() const {
   double a, sa, Ixx, Ixy, Iyy, x_bar, y_bar;

   if(n_points < 3) {
      mlog << Error << "\nPolyline::angle() -> "
	   << "not enough points!\n\n";

      exit(1);
   }

   centroid(x_bar, y_bar);

   Ixx = Ixy = Iyy = 0.0;

   sum_second_moments(x_bar, y_bar, Ixx, Ixy, Iyy);

   sa = uv_signed_area();

   Ixx /= 12.0*sa;
   Iyy /= 12.0*sa;
   Ixy /= 24.0*sa;

   a = 0.5*deg_per_rad*atan2(2.0*Ixy, Ixx - Iyy);

   return(a);
}

///////////////////////////////////////////////////////////////////////////////
//
//  By default, rotate about the centroid of the polyline
//
///////////////////////////////////////////////////////////////////////////////

void Polyline::rotate(double deg) {
   double ubar, vbar;

   centroid(ubar, vbar);

   rotate(deg, ubar, vbar);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Rotate the current polyline about the point specified
//
///////////////////////////////////////////////////////////////////////////////

void Polyline::rotate(double deg, double ubar, double vbar) {
   int i;
   double c, s;
   double x_old, y_old, x_new, y_new;

   s = sin(deg*rad_per_deg);
   c = cos(deg*rad_per_deg);

   // Rotate points of the current Polyline
   for(i=0; i<n_points; i++) {

      x_old = u[i] - ubar;
      y_old = v[i] - vbar;

      x_new = x_old*c - y_old*s;
      y_new = x_old*s + y_old*c;

      u[i] = ubar + x_new;
      v[i] = vbar + y_new;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double Polyline::uv_signed_area() const {
   int i, j;
   double sum, x_0, y_0, x_1, y_1;

   sum = 0.0;

   if(n_points > 0) {

      x_0 = u[0];
      y_0 = v[0];

      for(i=0; i<n_points; i++) {

         j = (i+1)%n_points;

         x_1 = u[j];
         y_1 = v[j];

         sum += x_0*y_1 - y_0*x_1;

         x_0 = x_1;
         y_0 = y_1;
      }

      sum *= 0.5;
   }

   return(sum);
}

///////////////////////////////////////////////////////////////////////////////
//
//  Return whether or not the test point is contained within the current
//  polyline and do not check the descendent polylines.
//
///////////////////////////////////////////////////////////////////////////////

int Polyline::is_inside(double u_test, double v_test) const

{

   int i, j;
   double Angle, Angle0, a, b, c, d;

   if(n_points == 0) {

      return(0);
   }

   a = u[0] - u_test;
   b = v[0] - v_test;

   Angle = Angle0 = atan2(b, a)/pi;

   for(i=0; i<n_points; i++) {

      j = (i+1)%n_points;

      c = u[j] - u_test;
      d = v[j] - v_test;

      Angle += atan2(a*d - b*c, a*c + b*d)/pi;

      a = c;
      b = d;
   }

   return(nint( (Angle - Angle0)/2 ));
}

///////////////////////////////////////////////////////////////////////////////

int Polyline::is_polyline_point(double u_test, double v_test) const {
   int i, poly_point;

   poly_point = 0;

   // Check if it's one of the current Polyline's points
   for(i=0; i<n_points; i++) {

      if(is_eq(u_test, u[i]) && is_eq(v_test, v[i])) poly_point = 1;
   }

   return(poly_point);
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::bounding_box(Box &bb) const {
   int i;
   double L, R, B, T;

   L = R = u[0];
   B = T = v[0];

   // Find min and max of current Polyline's points
   for(i=1; i<n_points; i++) {   //  i starts at one, here

      if(u[i] > R) R = u[i];
      if(u[i] < L) L = u[i];

      if(v[i] > T) T = v[i];
      if(v[i] < B) B = v[i];

   }

   bb.set_lrbt(L, R, B, T);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Polyline::sum_first_moments(double &sum_x, double &sum_y) const {
   int i, j;
   double x_0, y_0, x_1, y_1;

   if(n_points > 0) {

      x_0 = u[0];
      y_0 = v[0];

      for(i=0; i<n_points; i++) {

         j = (i+1)%n_points;

         x_1 = u[j];
         y_1 = v[j];

         sum_x +=  (x_0*x_0 + x_0*x_1 + x_1*x_1)*(y_1 - y_0);

         sum_y += -(y_0*y_0 + y_0*y_1 + y_1*y_1)*(x_1 - x_0);

         x_0 = x_1;
         y_0 = y_1;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////////////

void Polyline::sum_second_moments(double x_bar, double y_bar,
                                  double &Ixx, double &Ixy, double &Iyy) const {
   int i, j;
   double x_0, y_0, x_1, y_1;

   if(n_points > 0) {

      x_0 = u[0] - x_bar;
      y_0 = v[0] - y_bar;

      for(i=0; i<n_points; i++) {

         j = (i+1)%n_points;

         x_1 = u[j] - x_bar;
         y_1 = v[j] - y_bar;

         Ixx +=  (x_0*x_0*x_0 + x_0*x_0*x_1 + x_0*x_1*x_1 + x_1*x_1*x_1)
                *(y_1 - y_0);

         Iyy += -(y_0*y_0*y_0 + y_0*y_0*y_1 + y_0*y_1*y_1 + y_1*y_1*y_1)
                *(x_1 - x_0);

         Ixy +=  (x_0*(2.0*y_0 + y_1)
                + x_1*(2.0*y_1 + y_0))*(x_0*y_1 - x_1*y_0);

         x_0 = x_1;
         y_0 = y_1;
      }
   }

   return;
}


///////////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
///////////////////////////////////////////////////////////////////////////////

double point_dist(double x1, double y1, double x2, double y2) {
   double dx, dy;

   dx = x1 - x2;
   dy = y1 - y2;

   return( sqrt( dx*dx + dy*dy ) );
}

///////////////////////////////////////////////////////////////////////////////

double polyline_dist(const Polyline & a, const Polyline & b) {
   int j, k, j2, k2;
   double dist, min_dist;

   //
   //  Check whether one polyline is completely contained inside the other
   //  i.e. Check each vertex of one polyline to see if it's contained
   //  in the other
   //

   for (j=0; j<(a.n_points); ++j)  {

      if ( b.is_inside(a.u[j], a.v[j]) )  return ( 0.0 );
   }

   for (j=0; j<(b.n_points); ++j)  {

      if ( a.is_inside(b.u[j], b.v[j]) )  return ( 0.0 );
   }

   //
   //  Check to see if the polylines intersect
   //

   for (j=0; j<(a.n_points); ++j)  {

      j2 = (j + 1)%(a.n_points);

      for (k=0; k<(b.n_points); ++k)  {

         k2 = (k + 1)%(b.n_points);

         if ( intersect_linesegment( a.u[j], a.v[j], a.u[j2], a.v[j2],
                                     b.u[k], b.v[k], b.u[k2], b.v[k2] ) ) {

            return ( 0.0 );

         }
      }
   }

   //
   //  Find the minimum distance between the polylines
   //

   min_dist = 1.0e10;

   //
   //  Points of A from sides of B
   //

   for (j=0; j<(a.n_points); ++j)  {

      for (k=0; k<(b.n_points); ++k)  {

         k2 = (k + 1)%(b.n_points);

         dist = min_dist_linesegment(b.u[k], b.v[k], b.u[k2], b.v[k2], a.u[j], a.v[j]);

         if ( dist < min_dist )  min_dist = dist;

      }

   }

   //
   //  Points of B from sides of A
   //

   for (j=0; j<(b.n_points); ++j)  {

      for (k=0; k<(a.n_points); ++k)  {

         k2 = (k + 1)%(a.n_points);

         dist = min_dist_linesegment(a.u[k], a.v[k], a.u[k2], a.v[k2], b.u[j], b.v[j]);

         if ( dist < min_dist )  min_dist = dist;

      }

   }

   //
   //  done
   //

   return ( min_dist );
}

///////////////////////////////////////////////////////////////////////////////
//
//  polyline_pw_ls_mean_dist() returns the mean pointwise distance between two
//  polylines.  For each point of 'a', a minimum distance to the line segments
//  of 'b' is computed.  Those distances are averaged and returned.
//
///////////////////////////////////////////////////////////////////////////////


double polyline_pw_ls_mean_dist(const Polyline &a, const Polyline &b) {
   double d, min_d, d_sum, d_avg;
   int i, j;

   d_sum = 0;

   for(i=0; i<a.n_points; i++) {

      min_d = 1.0e10;

      for(j=0; j<b.n_points; j++) {

	 d = min_dist_linesegment(b.u[j], b.v[j],
	                          b.u[(j+1)%b.n_points], b.v[(j+1)%b.n_points],
				  a.u[i], a.v[i]);

         if(d < min_d) min_d = d;
      }

      d_sum += min_d;
   }

   if(a.n_points == 0) d_avg = 0;
   else                d_avg = d_sum/a.n_points;

   return(d_avg);
}


///////////////////////////////////////////////////////////////////////////////
//
//  polyline_pw_mean_sq_dist() returns the mean squared pointwise distance
//  between two polylines.  For each point of 'a', a minimum squared distance
//  to the points of 'b' is computed.  The average (mean) of those squared
//  distances is returned.
//
///////////////////////////////////////////////////////////////////////////////


double polyline_pw_mean_sq_dist(const Polyline &a, const Polyline &b) {
   double sqd, min_sqd, sqd_sum, sqd_avg;
   int i, j;

   sqd_sum = 0;

   for(i=0; i<a.n_points; i++) {

      min_sqd = 1.0e10;

      for(j=0; j<b.n_points; j++) {

         sqd = (a.u[i]-b.u[j])*(a.u[i]-b.u[j]) +
               (a.v[i]-b.v[j])*(a.v[i]-b.v[j]);

         if(sqd < min_sqd) min_sqd = sqd;
      }

      sqd_sum += min_sqd;
   }

   if(a.n_points == 0) sqd_avg = 0;
   else                sqd_avg = sqd_sum/a.n_points;

   return(sqd_avg);
}

///////////////////////////////////////////////////////////////////////////////

int intersect_linesegment(double x1, double y1, double x2, double y2,
                          double x3, double y3, double x4, double y4) {
   double ua_num, ub_num, denom, ua, ub, dx, dy;

   //
   //  Compute the intersection point of the lines defined by the two segments
   //

   ua_num = (x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3);
   ub_num = (x2 - x1)*(y1 - y3) - (y2 - y1)*(x1 - x3);
   denom  = (y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1);

   //
   //  Check for parallel lines which are coincident
   //
   if ( is_eq(denom, 0.0) && is_eq(ua_num, 0.0) && is_eq(ub_num, 0.0) ) {

      //
      //  Check for overlap of the line segments
      //  If they overlap one of the points (x1, y1) or (x2, y2) must
      //  lie between (x3, y3) and (x4, y4)
      //
      dx = fabs ( x3 - x4 );
      dy = fabs ( y3 - y4 );

      if ( ( fabs ( x1 - x3 ) <= dx && fabs ( x1 - x4 ) <= dx &&
             fabs ( y1 - y3 ) <= dy && fabs ( y1 - y4 ) <= dy )
            ||
           ( fabs ( x2 - x3 ) <= dx && fabs ( x2 - x4 ) <= dx &&
             fabs ( y2 - y3 ) <= dy && fabs ( y2 - y4 ) <= dy )
         )  return ( 1 );
   }

   //
   //  Check for parallel lines which are not coincident
   //
   else if ( is_eq(denom, 0.0) )  return ( 0 );

   else {

      ua = ua_num/denom;
      ub = ub_num/denom;

      //
      //  Check for the intersection point occuring in both lines segments:
      //  It is only necessary to test if ua and ub lie between 0 and 1.
      //  Whichever one lies within that range then the corresponding line
      //  segment contains the intersection point. If both lie within the
      //  range of 0 to 1 then the intersection point is within both
      //  line segments.
      //
      if ( 0 <= ua && ua <= 1 && 0 <= ub && ub <= 1 )  return ( 1 );
   }

   return ( 0 );
}

///////////////////////////////////////////////////////////////////////////////

double min_dist_linesegment(double px, double py, double qx, double qy,
                            double x_test, double y_test) {
   double t0;
   double qmpx, qmpy;
   double rmpx, rmpy;
   double qmp, rmp;
   double cx, cy;
   double vx, vy;
   double dx, dy;

   qmpx = qx - px;
   qmpy = qy - py;

   rmpx = x_test - px;
   rmpy = y_test - py;

   qmp = sqrt( qmpx*qmpx + qmpy*qmpy );

   rmp = sqrt( rmpx*rmpx + rmpy*rmpy );

   //
   // If qmp == 0, then px == qx and py == qy
   // Compute distance between (px, py) and (x_test, y_test)
   //

   if ( is_eq(qmp, 0.0) )  {

      dx = x_test - qx;
      dy = y_test - qy;

      return ( sqrt( dx*dx + dy*dy ) );
   }
   else {
      cx = qmpx/qmp;
      cy = qmpy/qmp;
   }

   t0 = rmpx*cx + rmpy*cy;

   if ( t0 < 0.0 )  return ( rmp );

   if ( t0 > qmp )  {

      dx = x_test - qx;
      dy = y_test - qy;

      return ( sqrt( dx*dx + dy*dy ) );
   }

   vx = rmpx - t0*cx;
   vy = rmpy - t0*cy;

   return ( sqrt( vx*vx + vy*vy ) );

}

///////////////////////////////////////////////////////////////////////////////
//
// Parse lat/lon values out of a space separated list of them.  The first
// token in the list is assumed to be the name of the polyline.
//
///////////////////////////////////////////////////////////////////////////////

void parse_latlon_poly_str(const char *poly_str, Polyline &poly) {
   ConcatString cs;
   StringArray sa;
   double lat, lon;

   cs << poly_str;
   sa = cs.split(" ");

   // Must have at least 7 tokens and be odd:
   //   name followed by at least 3 lat/lon pairs
   if(sa.n() < 7 || sa.n()%2 != 1) {
      mlog << Error << "\nparse_latlon_poly_str() -> "
           << "The polyline string (" << poly_str
           << ") must begin with a name and contain at least 3 pairs of "
           << "points to define a masking region.\n\n";
      exit(1);
   }

   // Store the name
   poly.set_name(sa[0]);

   // Store the lat/lon pairs
   for(int i=1; i<sa.n(); i+=2) {

      // Convert longitude from degrees east to west and rescale to (-180, 180).
      lat = atof(sa[i].c_str());
      lon = rescale_lon(-1.0*atof(sa[i+1].c_str()));

      // Add the subsequent lat/lon polyline points
      poly.add_point(lon, lat);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Parse lat/lon values out of a file containing a space separated list of
// them.  The first token in the list is assumed to be the name of the polyline.
//
///////////////////////////////////////////////////////////////////////////////

void parse_latlon_poly_file(const char *poly_file, Polyline &poly) {
   ifstream in;
   std::string tmp_str;
   double lat, lon;

   // Open the polyline file specified
   in.open(poly_file);

   if(!in) {
      mlog << Error << "\nparse_latlon_poly_file() -> "
           << "Can't open the polyline file specified ("
           << poly_file << ").\n\n";
      exit(1);
   }

   // Read in the first value, the name of the polyline
   in >> tmp_str;
   poly.set_name(tmp_str);

   // Read in the first lat/lon pair
   in >> lat >> lon;

   while(in) {

      // Convert from degrees east to degrees west
      lon *= -1.0;

      // Rescale the longitude value to -180 to 180
      lon = rescale_lon(lon);

      // Add the lat/lon point to the polyline
      poly.add_point(lon, lat);

      // Get the next lat/lon point from the file
      in >> lat >> lon;
   }

   in.close();

   if(poly.n_points < 3) {
      mlog << Error << "\nparse_latlon_poly_file() -> "
           << "The polyline file supplied ("
           << poly_file << ") must contain at least 3 pairs of points "
           << "to define a masking region.\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Parse grid x/y values out of a space separated list of them.  The first
// token in the list is assumed to be the name of the polyline.
//
///////////////////////////////////////////////////////////////////////////////

void parse_xy_poly_str(const char *poly_str, Polyline &poly) {
   ConcatString cs;
   StringArray sa;

   cs << poly_str;
   sa = cs.split(" ");

   // Must have at least 7 tokens and be odd:
   //   name followed by at least 3 lat/lon pairs
   if(sa.n() < 7 || sa.n()%2 != 1) {
      mlog << Error << "\nparse_xy_poly_str() -> "
           << "The polyline string (" << poly_str
           << ") must begin with a name and contain at least 3 pairs of "
           << "points to define a masking region.\n\n";
      exit(1);
   }

   // Store the name
   poly.set_name(sa[0]);

   // Store the x/y points
   for(int i=1; i<sa.n(); i+=2) {
      poly.add_point(atof(sa[i].c_str()), atof(sa[i+1].c_str()));
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Parse grid x/y values out of a file containing a space separated list of
// them.  The first token in the list is assumed to be the name of the polyline.
//
///////////////////////////////////////////////////////////////////////////////

void parse_xy_poly_file(const char *poly_file, Polyline &poly) {
   ifstream in;
   std::string tmp_str;
   double x, y;

   // Open the polyline file specified
   in.open(poly_file);

   if(!in) {
      mlog << Error << "\nparse_xy_poly_file() -> "
           << "Can't open the polyline file specified ("
           << poly_file << ").\n\n";
      exit(1);
   }

   // Read in the first value, the name of the polyline
   in >> tmp_str;
   poly.set_name(tmp_str);

   in >> x >> y;

   while(in) {

      // Add the x/y point to the polyline
      poly.add_point(x, y);

      // Get the next x/y point from the file
      in >> x >> y;
   }

   in.close();

   return;
}

///////////////////////////////////////////////////////////////////////////////



