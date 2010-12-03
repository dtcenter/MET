// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   vx_wrfdata.cc
//
//   Description:
//      Contains the definition of the field data class.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-03-06  Halley Gotway
//   001    08-19-08  R. Bullock     Fix convex hull routine.
//   002    08-26-09  Halley Gotway  Fix zero_border routine.
//   003    09-08-10  Halley Gotway  Optimize fractional_coverage routine.
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

#ifdef IBM
   #include <fcntl.h>
#else
   #include <sys/fcntl.h>
#endif

#include "vx_wrfdata.h"
#include "shape.h"
#include "vx_util.h"
#include "vx_met_util.h"
#include "vx_math.h"

///////////////////////////////////////////////////////////////////////////////

static bool   is_inside_bb(const BoundingBox &, double, double);
static bool   is_between(double, double, double);
static double dot(double, double, double, double);
static void   boundary_step(const WrfData &, int &, int &, int &);
static int    get_step_case(bool, bool, bool, bool);
static int    compare_int(const void *, const void *);

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class Shape
//
///////////////////////////////////////////////////////////////////////////////

Shape::~Shape() { }

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class FreelyMoveableShape
//
///////////////////////////////////////////////////////////////////////////////

FreelyMoveableShape::~FreelyMoveableShape() { }

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class WrfData
//
///////////////////////////////////////////////////////////////////////////////

WrfData::WrfData() {
   data = (unsigned short *) 0;

   clear();
}

///////////////////////////////////////////////////////////////////////////////

WrfData::~WrfData() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

WrfData::WrfData(const WrfData &f) {

   data = (unsigned short *) 0;

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

WrfData & WrfData::operator=(const WrfData &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::combine(const WrfData *wrfdata_ptr, int num_wrfdata, unixtime valid,
                      int lead, int accum, int combination, double raw_field_max) {
   int i;
   int x, y, n, in_nx, in_ny;
   double v, new_v, cell_max, cell_min;
   double *celldata = (double *) 0;

   //
   // Combination type of maximum (1), minimum (2), and sum (3) are the only
   // ones currently supported
   //
   if(combination != 1 && combination != 2 && combination != 3 ) {
      cerr << "\n\nERROR: void WrfData::combine() -> "
           << "combination type value of " << combination
           << " not currently supported\n\n" << flush;
      exit(1);
   }

   // Check to make sure that nx and ny are the same for all WrfData objects
   in_nx = wrfdata_ptr[0].get_nx();
   in_ny = wrfdata_ptr[0].get_ny();
   for(i=1; i<num_wrfdata; i++) {

      if( wrfdata_ptr[i].get_nx() != in_nx || wrfdata_ptr[i].get_ny() != in_ny ) {
         cerr << "\n\nERROR: void WrfData::combine() -> "
              << "inconsistent values: (nx, ny) = ("
              << in_nx << ", " << in_ny << ") != ("
              << wrfdata_ptr[i].get_nx() << ", " << wrfdata_ptr[i].get_ny()
              << ")\n\n" << flush;
         exit(1);
      }
   }
   // Set the size of the WrfData object
   set_size(in_nx, in_ny);
   celldata = new double [nx*ny];

   // Set the valid, lead, and accum time of the WrfData object
   set_valid_time(valid);
   set_lead_time(lead);
   set_accum_time(accum);

   cell_max = -1.0e30;
   cell_min =  1.0e30;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         new_v = bad_data_double;

         for(i=0; i<num_wrfdata; i++) {

            //
            // If one of the fields contains bad data, the combined field
            // will have bad data at that point as well
            //
            if(wrfdata_ptr[i].is_bad_xy(x, y)) {
               new_v = bad_data_double;
               break;
            }

            v = wrfdata_ptr[i].get_xy_double(x, y);

            if(combination == 1) {
               if(is_bad_data(new_v)) new_v = v;
               else if(v > new_v)     new_v = v;
            }
            else if(combination == 2) {
               if(is_bad_data(new_v)) new_v = v;
               else if(v < new_v)     new_v = v;
            }
            else if(combination == 3) {
               if(is_bad_data(new_v)) new_v = v;
               else                   new_v += v;
            }
         }

         n = two_to_one(x, y);
         celldata[n] = new_v;

         if(!is_bad_data(new_v)) {
            if(new_v > cell_max) cell_max = new_v;
            if(new_v < cell_min) cell_min = new_v;
         }
      }
   }

   if(raw_field_max < 0) {
      set_m((cell_max - cell_min)/wrfdata_int_data_max);
      set_b(cell_min);
   }
   else {
      set_m(raw_field_max/wrfdata_int_data_max);
      set_b(0.0);
   }

   // Calculate image
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         n = two_to_one(x, y);

         v = celldata[n];

         if(is_bad_data(v)) i = bad_data_flag;
         else {
            if(raw_field_max > 0.0) {
               v = (double) wrfdata_int_data_max*(v/raw_field_max);
            }
            else {
               v = (double) wrfdata_int_data_max*(v/cell_max);
            }

            i = nint(v);
            if(i < 0)                    i = 0;
            if(i > wrfdata_int_data_max) i = wrfdata_int_data_max;
         }

         put_xy_int(i, x, y);
      }
   }

   if( celldata ) {
      delete [] celldata;
      celldata = (double *) 0;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::expand(int radius) {
   int x, y, xx, yy, center_value, value;
   WrfData wrfdata = *this;

   // Loop through each grid point
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         // Get the value for the current grid point
         center_value = wrfdata.get_xy_int(x, y);

         // Loop through the neighborhood of the current grid point
         for(xx = x-radius; xx<= x+radius; xx++) {
            for(yy = y-radius; yy<= y+radius; yy++) {

               // Continue if off the grid
               if( xx < 0 || xx >= nx || yy < 0 || yy >= ny) {
                  continue;
               }
               // Continue if too far from the current grid point
               if( (abs(xx - x) >= radius-1 && abs(yy - y) >= radius) ||
                   (abs(yy - y) >= radius-1 && abs(xx - x) >= radius) ) {
                      continue;
               }

               if( get_xy_int(xx, yy) > center_value )
                  value = get_xy_int(xx, yy);
               else
                  value = center_value;

               put_xy_int(value, xx, yy);
            }
         }
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::clear() {

   if ( data ) { delete [] data;  data = (unsigned short *) 0; }

   m = 0.0;
   b = 0.0;

   nx = 0;
   ny = 0;

   valid_time = (unixtime) 0;
   lead_time = 0;

   mom.clear();

   memset(&h, 0, sizeof(h));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::assign(const WrfData &f) {

   clear();

   set_size(f.nx, f.ny);

   memcpy(data, f.data, nx*ny*sizeof(unsigned short));

   mom = f.mom;

   m = f.m;
   b = f.b;

   valid_time = f.valid_time;
   lead_time = f.lead_time;
   accum_time = f.accum_time;

   h = f.h;

   return;
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::get_xy_int(int x, int y) const {
   int j, n;

   n = two_to_one(x, y);

   j = (int) (data[n]);

   return ( j );
}

///////////////////////////////////////////////////////////////////////////////

double WrfData::get_xy_double(int x, int y) const {

   return( int_to_double( get_xy_int(x,y) ) );
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::put_xy_int(int t, int x, int y) {
   int n;

   n = two_to_one(x, y);

   if ( (t < 0) || ( t > wrfdata_int_max) ) {
      cerr << "\n\nERROR: void WrfData::put_xy_int() -> "
           << "bad value: t = " << t  << "\n\n" << flush;
      exit(1);
   }

   data[n] = (unsigned short) t;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::put_xy_double(double(v), int x, int y) {

   put_xy_int(double_to_int(v), x, y);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::set_size(int numx, int numy) {

   data = new unsigned short [numx*numy];

   if ( !data ) {
      cerr << "\n\nERROR: void WrfData::set_size() -> "
           << "memory allocation error\n\n" << flush;
      exit(1);
   }

   memset(data, 0, numx*numy*sizeof(unsigned short));

   nx = numx;
   ny = numy;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::set_header(const WrfDataHeader &header) {

   strcpy(h.magic, header.magic);
   strcpy(h.units, header.units);
   strcpy(h.gridname, header.gridname);

   h.nx = header.nx;
   h.ny = header.ny;

   h.creation_time = header.creation_time;
   h.valid_time = header.valid_time;
   h.lead_time = header.lead_time;
   h.accum_time = header.accum_time;

   h.m = header.m;
   h.b = header.b;

   return;
}

///////////////////////////////////////////////////////////////////////////////

double WrfData::int_to_double(int k) const {
   double v;

   if(k == bad_data_flag)   v = bad_data_double;
   else {
      v = m*k + b;
   }

   return(v);
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::double_to_int(double v) const {
   int k;

   if(is_bad_data(v))              k = bad_data_flag;
   else {
      if(m == 0.0)                 k = 0;
      else                         k = nint( (v - b)/m );

      if(k < 0)                    k = 0;
      if(k > wrfdata_int_data_max) k = wrfdata_int_data_max;
   }

   return(k);
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::two_to_one(int x, int y) const {
   int n;

   if ( (x < 0) || (x >= nx) || (y < 0) || (y >= ny) ) {
      cerr << "\n\nERROR: WrfData::two_to_one() -> "
           << "range check error: (nx, ny) = (" << nx << ", " << ny
           << "), (x, y) = (" << x << ", " << y << ")\n\n" << flush;
      exit(1);
   }

   n = y*nx + x;

   return ( n );
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::one_to_two(int n, int &x, int &y) const {

   if (n < 0 || n >= nx*ny) {
      cerr << "\n\nERROR: WrfData::one_to_two() -> "
           << "range check error: n = " << n << "but nx*ny = " << nx*ny
           << "\n\n" << flush;
      exit(1);
   }

   x = n%nx;
   y = n/nx;

   return;
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::x_left(int y) const {

   if ( (y < 0) || (y >= ny) ) {
      cerr << "\n\nERROR: WrfData::x_left(int) -> "
           << "range check error\n\n" << flush;
      exit(1);
   }

   int x;

   for(x=0; x<nx; x++) {

      if( f_is_on(x, y) ) return(x);
   }

   return(-1);
}

////////////////////////////////////////////////////////////////////////

int WrfData::x_right(int y) const {

   if ( (y < 0) || (y >= ny) ) {
      cerr << "\n\nERROR: WrfData::x_right(int) -> "
           << "range check error\n\n" << flush;
      exit(1);
   }

   int x;

   for(x=(nx - 1); x>=0; x--) {

      if( f_is_on(x, y) ) return(x);
   }

   return(-1);
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::write(const char *filename, const char *units,
                   const char *gridname) const {
   int bytes;
   int fd = -1;
   WrfDataHeader header;

   if ( (fd = open(filename, O_WRONLY | O_CREAT, 0644)) < 0 ) {
      cerr << "\n\nERROR: WrfData::write() -> "
           << "unable to open output file \"" << filename << "\"\n\n" << flush;
      exit(1);
   }

   // Fill in header
   strcpy(header.magic, wrfdata_magic);

   // Set units
   if(strlen(units) > 7) {
      cout << "\n\nWrfData::write() -> "
           << "units string too long \"" << units << "\"\n\n" << flush;

      strcpy(header.units, "");
   }
   else {
      strcpy(header.units, units);
   }

   // Set grid name
   if(strlen(gridname) > 15) {
      cout << "\n\nWrfData::write() -> "
           << "gridname string too long \""
           << gridname << "\"\n\n" << flush;

      strcpy(header.units, "");
   }
   else {
      strcpy(header.gridname, gridname);
   }

   header.nx = nx;
   header.ny = ny;

   header.valid_time = valid_time;
   header.lead_time = lead_time;
   header.accum_time = accum_time;

   header.creation_time = time(0);

   header.m = m;
   header.b = b;

   bytes = sizeof(h);

   if ( ::write(fd, &header, bytes) != bytes ) {
      cerr << "\n\nERROR: WrfData::write() -> "
           << "error writing header for output file \""
           << filename << "\"\n\n" << flush;
      exit(1);
   }

   //  write data
   bytes = nx*ny*sizeof(unsigned short);

   if ( ::write(fd, data, bytes) != bytes ) {
      cerr << "WrfData::write() -> "
           << "error writing data for output file \""
           << filename << "\"\n\n" << flush;
      exit(1);
   }

   //  done

   close(fd);
   fd = -1;

   return(1);
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::read(const char *filename) {
   int bytes;
   int fd = -1;
   WrfDataHeader header;

   clear();

   if ( (fd = open(filename, O_RDONLY)) < 0 ) {
      cerr << "\n\nERROR: WrfData::read() -> "
           << "unable to open input file \""
           << filename << "\"\n\n" << flush;
      exit(1);
   }

   bytes = sizeof(header);

   if ( ::read(fd, &header, bytes) != bytes ) {
      cerr << "\n\nERROR: WrfData::read() -> "
           << "trouble reading header from input file \""
           << filename << "\"\n\n" << flush;
      exit(1);
   }

   if ( strcmp(header.magic, wrfdata_magic_wrf001) != 0 ) {
      cerr << "\n\nERROR: WrfData::read() -> "
           << "bad magic cookie in input file \""
           << filename << "\"\n\n" << flush;
      exit(1);
   }

   set_size(header.nx, header.ny);

   h = header;

   bytes = (h.nx)*(h.ny)*sizeof(unsigned short);

   if ( ::read(fd, data, bytes) != bytes ) {
      cerr << "\n\nERROR: WrfData::read() -> "
           << "trouble reading header from input file \""
           << filename << "\"\n\n" << flush;
      exit(1);
   }

   m = h.m;
   b = h.b;

   valid_time = h.valid_time;
   lead_time = h.lead_time;
   accum_time = h.accum_time;

   calc_moments();

   close(fd);
   fd = -1;

   return(1);
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::rescale_data(double min_v, double max_v) {
   WrfData wrfdata;
   int x, y, v_in, v_out;
   double v_double;

   //
   // Default the new field to the current one
   //
   wrfdata = *this;

   //
   // Reset the new m and b values
   //
   wrfdata.set_m( (max_v - min_v)/(double) wrfdata_int_data_max );
   wrfdata.set_b( min_v );
   wrfdata.h.m = wrfdata.m;
   wrfdata.h.b = wrfdata.b;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         v_in = get_xy_int(x, y);

         //
         // When rescaling data, preserve the bad data flag
         //
         if(v_in == bad_data_flag) v_out = v_in;

         //
         // Calculate the new int value
         //
         else {
            v_double = int_to_double(v_in);
            v_out = wrfdata.double_to_int(v_double);
         }

         wrfdata.put_xy_int(v_out, x, y);
      } // end for y
   } // end for x

   *this = wrfdata;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::centroid(double &xbar, double &ybar) const {

   mom.centroid(xbar, ybar);

   return;
}

///////////////////////////////////////////////////////////////////////////////

double WrfData::area() const {

   double x = (double) (mom.area);

   return ( x );
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::calc_length_width(double &l, double &w) const {
   int x, y, k;
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

   for (x=0; x<nx; ++x) {
      for (y=0; y<ny; ++y) {

         k = f_is_on(x, y);

         if ( !k )  continue;

         xx = (double) x;
         yy = (double) y;

         u = dot(e1x, e1y, xx, yy);
         v = dot(e2x, e2y, xx, yy);

         if ( u > u_max )  u_max = u;
         if ( u < u_min )  u_min = u;

         if ( v > v_max )  v_max = v;
         if ( v < v_min )  v_min = v;
      } // for y
   } // for x

   u_extent = u_max - u_min;
   v_extent = v_max - v_min;

   if ( u_extent > v_extent ) { l = u_extent;  w = v_extent; }
   else                       { l = v_extent;  w = u_extent; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double WrfData::length() const {
   double l, w;

   calc_length_width(l, w);

   return(l);
}

///////////////////////////////////////////////////////////////////////////////

double WrfData::width() const {
   double l, w;

   calc_length_width(l, w);

   return(w);
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::calc_moments() {
   int x, y, k;
   double xx, yy;

   mom.clear();

   for (x=0; x<nx; ++x) {

      xx = ((double) x);

      for (y=0; y<ny; ++y) {

         yy = ((double) y);

         k = f_is_on(x, y);

         if ( k ) {
            mom.area += 1;

            mom.sx    += xx;
            mom.sy    += yy;

            mom.sxx   += xx*xx;
            mom.sxy   += xx*yy;
            mom.syy   += yy*yy;

            mom.sxxx  += xx*xx*xx;
            mom.sxxy  += xx*xx*yy;
            mom.sxyy  += xx*yy*yy;
            mom.syyy  += yy*yy*yy;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::s_is_on(int x, int y) const {

   int j;

   j = get_xy_int(x, y);

   if ( j > 0 )  j = 1;

   return ( j );
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::f_is_on(int x, int y) const {

   if ( s_is_on(x, y) )  return ( 1 );

   if ( (x > 0) && s_is_on(x - 1, y) )  return ( 1 );

   if ( (x > 0) && (y > 0) && s_is_on(x - 1, y - 1) )  return ( 1 );

   if ( (y > 0) && s_is_on(x, y - 1) )  return ( 1 );

   return(0);
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::is_bad_xy(int x, int y) const {

   if(get_xy_int(x, y) == bad_data_flag) {
      return(1);
   }
   else {
      return(0);
   }
}

///////////////////////////////////////////////////////////////////////////////

int WrfData::is_valid_xy(int x, int y) const {

   if(get_xy_int(x, y) <= wrfdata_int_data_max) {
      return(1);
   }
   else {
      return(0);
   }
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::bounding_box(BoundingBox &bb) const {
   int x, y, k;

   bb.x_ll = nx;
   bb.y_ll = ny;
   bb.x_ur = bb.y_ur = 0;

   for(x=0; x<nx; ++x) {
      for(y=0; y<ny; ++y) {

         k = f_is_on(x, y);

         if( !k )  continue;

         if( x > bb.x_ur )  bb.x_ur = x;
         if( x < bb.x_ll )  bb.x_ll = x;

         if( y > bb.y_ur )  bb.y_ur = y;
         if( y < bb.y_ll )  bb.y_ll = y;
      } // for y
   } // for x

   bb.width = bb.x_ur - bb.x_ll;
   bb.height = bb.y_ur - bb.y_ll;

   return;
}

///////////////////////////////////////////////////////////////////////////////

Polyline WrfData::convex_hull() const {
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

      cerr << "\n\nERROR: convex_hull(Polyline &) -> "
           << "attempting to fit convex hull to a shape with area = 0\n\n"
           << flush;
      exit(1);
   }

   hull.extend_points(2*ny);
   outline.extend_points(2*ny);

   Index = new int [2*ny];

   if ( !Index )  {

      cerr << "\n\nERROR: convex_hull(Polyline &) -> "
           << "memory allocation error\n\n" << flush;
      exit(1);
   }

   n = 0;

   for (y=0; y<ny; ++y)  {

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

   }

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

      cerr << "\n\nERROR: convex_hull(Polyline &) -> "
           << "can't find lowest point\n\n" << flush;
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

   angle = deg_per_rad*atan2(beta, alpha);

   angle -= 360.0*floor(angle/360.0);

   //
   //  reset angle very close to 360 to be 0
   //

   if ( angle > 359.9999 )   { angle = 0.0; }

   if ( angle < angle_low )  { angle_low = angle;  j = k; }

   }   //  for k

   if ( j < 0 )  {

      cerr << "\n\nERROR: convex_hull(Polyline &) -> "
           << "can't find next hull point\n\n" << flush;
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

Polyline WrfData::single_boundary() const {

   //
   // Call boundary function with all_points set to false
   // and clockwise set to true
   //

   return( single_boundary(false, 1) );
}

///////////////////////////////////////////////////////////////////////////////
//
//  WrfData::single_boundary() should only be called for split fields
//  containing only one object.
//
///////////////////////////////////////////////////////////////////////////////

Polyline WrfData::single_boundary(bool all_points, int clockwise) const {

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

Polyline WrfData::single_boundary_offset(double d) const {

   //
   // Call boundary offset function with all_points set to false
   // and clockwise set to true
   //

   return( single_boundary_offset(false, 1, d) );
}

///////////////////////////////////////////////////////////////////////////////
//
//  WrfData::single_boundary_offset() should only be called for split fields
//  containing only one object.
//
///////////////////////////////////////////////////////////////////////////////

Polyline WrfData::single_boundary_offset(bool all_points, int clockwise, double d) const {
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
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

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

      cout << "\n\nWrfData::single_boundary_offset() const -> "
           << "no points found in object\n\n" << flush;

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

void WrfData::composite_boundary(Node &boundary) const {

   //
   // Call boundary function with all_points set to false
   // and clockwise set to true
   //

   composite_boundary(false, 1, boundary);

   return;

}

///////////////////////////////////////////////////////////////////////////////
//
//  WrfData::composite_boundary() should only be called for fields
//  containing one or more simple objects.
//
///////////////////////////////////////////////////////////////////////////////

void WrfData::composite_boundary(bool all_points, int clockwise,
                                 Node &boundary) const {
   Polyline single_poly;
   WrfData wfd_split, wfd_single;
   int i, n_shapes;

   //
   // Initialize
   //
   boundary.clear();

   //
   // Split the field to determine the number of objects
   //
   wfd_split = split(*this, n_shapes);

   //
   // Calculate the boundary for each single shape
   //
   for(i=0; i<n_shapes; i++) {

      single_poly.clear();

      wfd_single = select(wfd_split, i+1);

      single_poly = wfd_single.single_boundary(all_points, clockwise);

      boundary.add_child(&single_poly);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  WrfData::single_outline_dist(int) constructs a polyline representation
//  of a single wrfdata object consisting of n equally spaced points along
//  the boundary of the object starting from the axis angle.
//
///////////////////////////////////////////////////////////////////////////////

Polyline WrfData::single_outline_dist(int n) const {
   Polyline boundary, outline;
   int i, j, start;
   double step;
   double xc, yc, x_save, y_save;

   //
   // Calculate the centroid of this object
   //
   centroid(xc, yc);

   //
   // Compute the boundary for this single wrfdata object
   // retaining all points since the outline points must all lie
   // on the boundary
   //
   boundary = single_boundary(true, 1);

   //
   // Compute the boundary point step size based on the number of boundary
   // points and n
   //
   step = (double) boundary.n_points/n;

   //
   // Find the starting boundary point based on the axis angle
   //
   start = single_outline_point(angle_degrees(), xc, yc, boundary, true, x_save, y_save);

   //
   // Select equally spaced boundary points
   //
   for(i=0; i<n; i++) {

      j = nint(start + i*step)%boundary.n_points;
      outline.add_point(boundary.u[j], boundary.v[j]);
   } // end for i

   return(outline);
}

///////////////////////////////////////////////////////////////////////////////
//
//  WrfData::single_outline_step(int) constructs a polyline representation
//  of a single wrfdata object consisting of points equally spaced the
//  specified distance along the boundary of the object starting from the
//  axis angle.
//
///////////////////////////////////////////////////////////////////////////////

Polyline WrfData::single_outline_step(int step) const {
   Polyline boundary, outline;
   int i, j, start;
   double xc, yc, x_save, y_save;

   //
   // Calculate the centroid of this object
   //
   centroid(xc, yc);

   //
   // Compute the boundary for this single wrfdata object
   // retaining all points since the outline points must all lie
   // on the boundary
   //
   boundary = single_boundary(true, 1);

   //
   // Find the starting boundary point based on the axis angle
   //
   start = single_outline_point(angle_degrees(), xc, yc, boundary, true,
                                x_save, y_save);

   //
   // Select equally spaced boundary points
   //
   for(i=0; i<(boundary.n_points/step); i++) {

      j = (start + i*step)%boundary.n_points;
      outline.add_point(boundary.u[j], boundary.v[j]);
   } // end for i

   return(outline);
}

///////////////////////////////////////////////////////////////////////////////
//
//  WrfData::single_outline_angle(int) constructs a polyline representation
//  of a single wrfdata object consisting of n points spaced along the boundary
//  of the object at equal rotation angle increments defined in a clockwise
//  direction starting from the axis angle.  For example, for n=36, the first
//  point will be the northernmost intersection of the axis angle with the
//  boundary and subsequent points will be found using 10 degree rotation
//  increments.
//
///////////////////////////////////////////////////////////////////////////////

Polyline WrfData::single_outline_angle(int n) const {
   Polyline boundary, outline;
   int i, j, start_i;
   double axis_ang, step, rot_ang;
   double xc, yc, x_save, y_save;
   int *index = (int *) 0;

   //
   // Store the indices of the boundary points of which the outline will
   // consist in index
   //
   index = new int [n];

   if(!index) {
      cerr << "\n\nERROR: void WrfData::single_outline_dist() -> "
           << "memory allocation error\n\n" << flush;
      exit(1);
   }

   //
   // Calculate the centroid of this object
   //
   centroid(xc, yc);

   //
   // Compute the boundary for this single wrfdata object
   // retaining all points since the outline points must all lie
   // on the boundary
   //
   boundary = single_boundary(true, 1);

   //
   // Compute the axis angle and step size based on n, the number of
   // requested outline points
   //
   axis_ang = angle_degrees();
   step = (double) 360.0/n;

   //
   // Loop through the angles selecting corresponding boundary points
   //
   for(i=0; i<n; i++) {

      x_save = y_save = 0.0;

      rot_ang = axis_ang + i*step;

      //
      // If no boundary point is found using this rotation angle,
      // increase the rotation by 180 degrees and request the min distance
      // rather than the max
      //
      index[i] = single_outline_point(rot_ang, xc, yc, boundary, true, x_save, y_save);
      if(index[i] < 0) {

         index[i] = single_outline_point(rot_ang+180.0, xc, yc, boundary, false, x_save, y_save);
         if(index[i] < 0) {
            cerr << "\n\nERROR: void WrfData::single_outline_angle() -> "
                 << "unable to find boundary point for rotation angle of "
                 << rot_ang << " or " << rot_ang+180.0 << "\n\n" << flush;
            exit(1);
         }
      }

      if(i==0) start_i = index[i];
   } // end for i

   //
   // Sort the indices so that the outline points appear in the same
   // order as the boundary points
   //
   qsort(index, n, sizeof(int), compare_int);

   //
   // Add the sorted boundary points to the outline
   //
   for(i=0; i<n; i++) {
      if(index[i] == start_i) {
         start_i = i;
         break;
      }
   }
   for(j=0; j<n; j++) {
      outline.add_point(boundary.u[index[(start_i+j)%n]],
                        boundary.v[index[(start_i+j)%n]]);
   }

   return(outline);
}

///////////////////////////////////////////////////////////////////////////////

int compare_int(const void *p1, const void *p2)

{

const int *a = (const int *) p1;
const int *b = (const int *) p2;


if ( (*a) < (*b) )  return ( -1 );

if ( (*a) > (*b) )  return (  1 );


return ( 0 );

}

////////////////////////////////////////////////////////////////////////

int WrfData::single_outline_point(double angle, double xc, double yc,
   Polyline &bnd_poly, bool max, double &x_save, double &y_save) const {
   int i, i_save;
   double e1x, e1y, mm, x, y, xb, yb;
   double cd, cd_extreme;

   i_save = -1;

   e1x = cosd(angle);
   e1y = sind(angle);

   if(is_eq(e1x, 0.0)) mm = 0.0;
   else                mm = e1y/e1x;

   //
   // Find the point on the grid boundary where the line defined by (xc, yc)
   // and slope e1y/e1x intersects
   //
   x = xc;
   y = yc;
   while(x >= 0 && x < nx &&
         y >= 0 && y < ny) {
      x += e1x;
      y += e1y;
   }

   if(x >= nx) { // Right
      xb = nx;
      yb = mm*(xb-xc) + yc;
   }
   else if(y >= ny) { // Top
      yb = ny;
      xb = (yb-yc)/mm + xc;
   }
   else if(x < 0) { // Left
      xb = 0;
      yb = mm*(xb-xc) + yc;
   }
   else { // y < 0: Bottom
      yb = 0;
      xb = (yb-yc)/mm + xc;
   }

   //
   // Find point on the bnd_poly with distance less than 0.5 to
   // the line segment defined by (xc, yc) and (xb, yb) and
   // maximum distance from (xc, yc)
   //
   if(max) cd_extreme = -1.0e30;
   else    cd_extreme = 1.0e30;

   for(i=0; i<bnd_poly.n_points; i++) {

      if(min_dist_linesegment(xc, yc, xb, yb, bnd_poly.u[i], bnd_poly.v[i])
         <= 0.50) {

         cd = point_dist(xc, yc, bnd_poly.u[i], bnd_poly.v[i]);

         if( ( max && cd >= cd_extreme) ||
             (!max && cd <= cd_extreme) ) {
            cd_extreme = cd;
            x_save = bnd_poly.u[i];
            y_save = bnd_poly.v[i];
            i_save = i;
         }
      }
   } // end for i

   return(i_save);
}

////////////////////////////////////////////////////////////////////////

double WrfData::complexity() const {
   int count;
   double shape;
   double hull;
   double u;
   Polyline poly;

   count = s_area();

   if( count == 0 ) {
      cerr << "\n\nERROR: WrfData::complexity() const -> "
           << "empty shape!\n\n" << flush;
      exit(1);
   }

   shape = (double) count;
   poly = convex_hull();
   hull = fabs(poly.uv_signed_area());

   //
   // Complexity is defined as the difference in area between the
   // convex hull and the original shape divided by the area of the
   // convex hull.  0 <= Complexity < 1, and complexity = 0 indicates
   // that the shape is convex.
   //
   u = (hull - shape)/hull;

   return(u);
}

////////////////////////////////////////////////////////////////////////

int WrfData::s_area() const {
   int x, y;
   int count;

   count = 0;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         if( s_is_on(x, y) ) count++;
      }
   }

   return(count);
}

///////////////////////////////////////////////////////////////////////////////
//
// Filter the raw values based on the threshold and threshold indicator
// and zero out the rest
//
///////////////////////////////////////////////////////////////////////////////

void WrfData::filter(SingleThresh t) {
   int x, y, v;

   for (x=0; x<nx; ++x) {
      for (y=0; y<ny; ++y) {

         v = get_xy_int(x, y);

         if(!t.check(v)) {
            put_xy_int(0, x, y);
         }

      } // end for y
   } // end for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::conv_filter(const FilterBox &box) {
   int j;
   int x, y, xx, yy, u, v;
   int count, v_int;
   int max_j, min_j;
   double sum, v_double;
   double min_sum, max_sum;
   WrfData wrfdata = *this;

   max_j = -10000000;
   min_j =  10000000;

   max_sum = -1.0e30;
   min_sum =  1.0e30;

   for (x=0; x<nx; ++x) {
      for (y=0; y<ny; ++y) {

         sum = 0.0;
         count = 0;

         for (u=box.get_xmin(); u<=box.get_xmax(); ++u) {
            xx = x + u;

               for (v=box.get_ymin(); v<=box.get_ymax(); ++v) {
                  yy = y + v;

                  if ( (xx < 0) || (yy < 0) || (xx >= nx) || (yy >= ny) )
                     continue;

                  if ( !(box.is_on(u, v)) )
                     continue;

                  v_int = get_xy_int(xx, yy);

                  if( v_int == bad_data_flag ) v_double = 0.0;
                  else {
                     v_double = int_to_double(v_int);
                  }

                  sum += v_double*(box.get_value(u, v));

                  ++count;
               } // for v
         } // for u

         if ( count == 0 )  v_double = 0.0;

         if ( sum > max_sum )  max_sum = sum;
         if ( sum < min_sum )  min_sum = sum;

         //
         //  check for bad data less than the value of b and
         //  convert to int value
         //
         if(sum < b) j = 0;
         else        j = double_to_int(sum);

         if ( j > max_j )  max_j = j;
         if ( j < min_j )  min_j = j;

         wrfdata.put_xy_int(j, x, y);
      } // for y
   } // for x

   memcpy(data, wrfdata.data, nx*ny*sizeof(unsigned short));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::conv_filter_circ(int diameter, double bd_thresh) {
   int j;
   int x, y, xx, yy, u, v;
   int count, bd_count;
   double sum, mm;
   double min_sum, max_sum, ratio;
   WrfData field = *this;
   const int dm1o2 = (diameter - 1)/2;
   FilterBox box;

   if((diameter%2 == 0) || (diameter < 3)) {
      cerr << "\n\nERROR: WrfData::conv_filter_circ() -> "
           << "diameter must be odd and >= 3 ... diameter = "
           << diameter << "\n\n" << flush;
      exit(1);
   }

   //
   //  We're not using the values, just whether a particular (x, y) is on or off
   //
   box.set_cylinder_volume(dm1o2, 1.0);

   max_sum = -1.0e30;
   min_sum =  1.0e30;

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         //
         // If the bad data threshold is set to zero and the center of the
         // convolution radius contains bad data, set the convolved value to
         // bad data and continue.
         //
         if(bd_thresh == 0 && !is_valid_xy(x, y)) {
            field.put_xy_int(bad_data_int, x, y);
            continue;
         }

         sum = 0.0;
         count = 0;
         bd_count = 0;

         for(u=box.get_xmin(); u<=box.get_xmax(); u++) {

            xx = x + u;

            for(v=box.get_ymin(); v<=box.get_ymax(); v++) {

               yy = y + v;

               if((xx < 0) || (yy < 0) || (xx >= nx) || (yy >= ny)) continue;
               if(!(box.is_on(u, v)))  continue;

               j = get_xy_int(xx, yy);

               if(j > wrfdata_int_data_max) { bd_count++; continue; }

               mm = m*j + b;

               sum += mm;

               count++;
            } // for v

         } // for u

         //
         //  If the center of the convolution contains bad data and the ratio
         //  of bad data in the convolution area is too high, set the convoled
         //  value to the minimum value.
         //
         ratio = (double) bd_count/(bd_count + count);
         if(!is_valid_xy(x, y) && ratio > bd_thresh) {
            sum = bad_data_double;
         }
         else if(count == 0) {
            sum = bad_data_double;
         }
         else {
            sum /= count;
         }

         if(!is_bad_data(sum) && sum > max_sum) max_sum = sum;
         if(!is_bad_data(sum) && sum < min_sum) min_sum = sum;

         field.put_xy_int(double_to_int(sum), x, y);
      } // for y
   } // for x

   memcpy(data, field.data, nx*ny*sizeof(unsigned short));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::coverage_filter(const FilterBox &box, double t) {
   int j;
   int x, y, xx, yy, u, v;
   int count;
   int max_j, min_j;
   double sum, mm;
   double min_sum, max_sum;
   WrfData wrfdata = *this;

   max_j = -10000000;
   min_j =  10000000;

   max_sum = -1.0e30;
   min_sum =  1.0e30;

   for (x=0; x<nx; ++x) {
      for (y=0; y<ny; ++y) {

         sum = 0.0;
         count = 0;

         for (u=box.get_xmin(); u<=box.get_xmax(); ++u) {
            xx = x + u;

               for (v=box.get_ymin(); v<=box.get_ymax(); ++v) {
                  yy = y + v;

                  if ( (xx < 0) || (yy < 0) || (xx >= nx) || (yy >= ny) )
                     continue;

                  if ( !(box.is_on(u, v)))
                     continue;

                  if(!is_valid_xy(xx, yy)) {
                     mm = t;
                  }
                  else {
                     mm = get_xy_double(xx, yy);
                  }

                  if(mm > t)   sum += 1.0;

                  count++;
               } // for v
         } // for u

         if ( count == 0 )  sum = 0.0;
         else               sum = sum/count*wrfdata_int_data_max;

         j = nint(sum);

         if ( sum > max_sum )  max_sum = sum;
         if ( sum < min_sum )  min_sum = sum;

         if ( j > max_j )  max_j = j;
         if ( j < min_j )  min_j = j;

         if ( j > wrfdata_int_data_max )  j = wrfdata_int_data_max;
         if ( j < 0 )                   j = 0;

         wrfdata.put_xy_int(j, x, y);
      } // for y
   } // for x

   set_m( 100.0/wrfdata_int_data_max );
   set_b( 0.0 );

   memcpy(data, wrfdata.data, nx*ny*sizeof(unsigned short));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::zero_border(int k) {

   zero_border(k, 0);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::zero_border(int k, int v_int) {
   int x, y;

   // top

   for (x=0; x<nx; ++x) {
      for (y=(ny - k); y<ny; ++y) {

         put_xy_int(v_int, x, y);
      } // for y
   } // for x

   //  bottom

   for (x=0; x<nx; ++x) {
      for (y=0; y<k; ++y) {

         put_xy_int(v_int, x, y);
      } // for y
   } // for x

   //  left

   for (x=0; x<k; ++x) {
      for (y=0; y<ny; ++y) {

         put_xy_int(v_int, x, y);
      } // for y
   } // for x

   //  right

   for (x=(nx - k); x<nx; ++x) {
      for (y=0; y<ny; ++y) {

         put_xy_int(v_int, x, y);
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::translate(int dx, int dy) {
   int x, y, new_x, new_y;
   WrfData fd_old;

   //
   // Check for zero translation
   //
   if(dx == 0 && dy == 0) return;

   fd_old = *this;
   zero_field(*this);

   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         if(fd_old.s_is_on(x, y)) {

            new_x = x+dx;
            new_y = y+dy;

            if( new_x >= 0 && new_x < nx
             && new_y >= 0 && new_y < ny ) {

               put_xy_int(fd_old.get_xy_int(x, y), new_x, new_y);
            }
         }
      } // end for y
   } // end for x

   calc_moments();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::threshold(int t) {
   SingleThresh st;

   // By default, threshold greater than or equal to
   st.thresh = t;
   st.type   = thresh_ge;

   threshold(st);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::threshold(SingleThresh t) {
   int j, x, y, v;

   //
   // Compare the threshold integer value to the WrfData integer values
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         v = get_xy_int(x, y);

         if(t.check((double) v) && v != bad_data_flag) {
            j = wrfdata_int_data_max;
         }
         else {
            j = 0;
         }
         put_xy_int(j, x, y);
      } // end for y
   } // end for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::threshold_double(double t) {
   SingleThresh st;

   // By default, threshold greater than or equal to
   st.thresh = t;
   st.type   = thresh_ge;

   threshold_double(st);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::threshold_double(SingleThresh t) {
   int j, x, y;
   double v;

   //
   // Compare the threshold double value to the double values for the
   // WrfData field
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         v = get_xy_double(x, y);

         if(t.check(v) && !is_bad_data(v)) {
            j = wrfdata_int_data_max;
         }
         else {
            j = 0;
         }
         put_xy_int(j, x, y);
      } // end for y
   } // end for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::threshold_area(SingleThresh t) {
   int j, n, x, y, v_int;
   WrfData wd_split, wd_object;
   double area_object[1 + max_cells];  // area_object[0] is ignored

   // Split the field to number the shapes
   wd_split = split(*this, n);

   // Zero out area array
   for(j=0; j<max_cells; j++) area_object[j] = 0;

   //
   // Compute the area of each object
   //
   for(j=1; j<=n; j++) {
      wd_object = select(wd_split, j);
      area_object[j] = wd_object.area();
   }

   //
   // Zero out any shapes with an area that doesn't meet the
   // threshold criteria
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         v_int = wd_split.get_xy_int(x, y);

         if(!t.check(area_object[v_int])) {
            put_xy_int(0, x, y);
         }

      } // end for y
   } // end for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void WrfData::threshold_intensity(const WrfData *wd_ptr, int perc, SingleThresh t) {
   int i, n, x, y, v_int, n_obj_inten;
   WrfData s;
   double *obj_inten = (double *) 0, obj_inten_sum;
   double inten_object[1 + max_cells];  // area_object[0] is ignored

   if(perc < 0 || perc > 102) {
      cerr << "\n\nERROR: WrfData:threshold_intensity() -> "
           << "the intensity percentile requested must be between 0 and 102.\n\n"
           << flush;
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

            v_int = s.get_xy_int(x, y);

            if(v_int != i+1) continue;

            if(wd_ptr->is_valid_xy(x, y)) {
               obj_inten[n_obj_inten] = wd_ptr->get_xy_double(x, y);
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

         v_int = s.get_xy_int(x, y);

         if(!t.check(inten_object[v_int])) {
            put_xy_int(0, x, y);
         }

      } // end for y
   } // end for x

   if(obj_inten) { delete [] obj_inten; obj_inten = (double *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double WrfData::angle_degrees() const {

   return(mom.angle_degrees());
}

///////////////////////////////////////////////////////////////////////////////

double WrfData::curvature(double &xcurv, double &ycurv) const {

   return(mom.curvature(xcurv, ycurv));
}

///////////////////////////////////////////////////////////////////////////////
//
//  End Code for class WrfData
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
      cerr << "\n\nERROR: void Cell::add() -> "
           << "too many elements!\n\n" << flush;
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
      cerr << "\n\nERROR: Partition::merge_cells() -> "
           << "range check error\n\n" << flush;
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
      cerr << "\n\nERROR: void Partition::merge_values() -> "
           << "bad values: (v1, v2) = (" << v1 << ", " << v2
           << "), (j1, j2) = (" << j_1 << ", " << j_2 << ")\n\n" << flush;
      return;
   }

   merge_cells(j_1, j_2);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Partition::add(int k) {

   if ( has(k) )  return;

   if ( n >= max_cells ) {
      cerr << "\n\nERROR: void Partition::add() -> "
           << "too many cells!\n\n" << flush;
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

WrfData split(const WrfData &wfd, int &n_shapes) {
   int k, x, y;
   int s;
   int xx, yy, numx, numy;
   int current_shape;
   int shape_assigned;
   WrfData d;
   WrfData q = wfd;
   Partition p;

   numx = wfd.get_nx();
   numy = wfd.get_ny();

   d.set_size(numx, numy);

   n_shapes = 0;

   //  shape numbers start at ONE here!!

   current_shape = 0;

   for (y=(wfd.get_ny() - 2); y>=0; --y) {
      for (x=(wfd.get_nx() - 2); x>=0; --x) {

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
                  p.merge_values(d.get_xy_int(x, y), d.get_xy_int(xx, yy));
               else
                  d.put_xy_int(d.get_xy_int(xx, yy), x, y);

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
                  p.merge_values(d.get_xy_int(x, y), d.get_xy_int(xx, yy));
               else
                  d.put_xy_int(d.get_xy_int(xx, yy), x, y);

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
                  p.merge_values(d.get_xy_int(x, y), d.get_xy_int(xx, yy));
               else
                  d.put_xy_int(d.get_xy_int(xx, yy), x, y);

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
                  p.merge_values(d.get_xy_int(x, y), d.get_xy_int(xx, yy));
               else
                  d.put_xy_int(d.get_xy_int(xx, yy), x, y);

               shape_assigned = 1;
            }
         }

         // is it a new shape?

         if ( !shape_assigned ) {

            d.put_xy_int(++current_shape, x, y);
            p.add(d.get_xy_int(x, y));
         }
      } // for x
   } // for y

   for (x=0; x<numx; ++x) {
      for (y=0; y<numy; ++y) {

         q.put_xy_int(0, x, y);

         for (k=0; k<(p.n); ++k) {
            if ( p.c[k].has(d.get_xy_int(x, y)) )   q.put_xy_int(k + 1, x, y);
         }
      } // for y
   } // for x

   n_shapes = p.n;

   q.calc_moments();

   return(q);
}

///////////////////////////////////////////////////////////////////////////////

int closest_dist(const WrfData &s1, int n1, const WrfData &s2, int n2) {
   int x_1, y_1, x_2, y_2;
   int nx, ny;
   double dist, min_dist;
   double dx, dy;

   nx = s1.get_nx();
   ny = s1.get_ny();

   min_dist = 10000000.0;

   for (x_1=0; x_1<nx; ++x_1) {
      for (y_1=0; y_1<ny; ++y_1) {

         if ( s1.get_xy_int(x_1, y_1) != n1 )  continue;

         for (x_2=0; x_2<nx; ++x_2) {
            for (y_2=0; y_2<ny; ++y_2) {

               if ( s2.get_xy_int(x_2, y_2) != n2 )  continue;

               dx = abs(x_1 - x_2);
               dy = abs(y_1 - y_2);

               dist = dx*dx + dy*dy;

               if ( dist < min_dist )  min_dist = dist;
            }
         }
      }
   }

   return( nint( sqrt(min_dist) ) );
}

///////////////////////////////////////////////////////////////////////////////
//
//  split_field_to_polyline()
//  This routine will convert a split field (i.e. objects numbers 1 to n) into
//  a polyline tree representation of the object's boundaries.
//
///////////////////////////////////////////////////////////////////////////////

void split_field_to_polyline(const WrfData &wfd_split, int n_objects,
                             Node &parent_node, bool all_points,
                             int level) {
   Polyline child_poly;
   WrfData wfd_object, wfd_reverse, wfd_reverse_split;
   int i, n_rev_objects;
   Node *n_ptr;

   // Process each object in the field
   for(i=0; i<n_objects; i++) {

      // Initialize fields
      wfd_object.clear();
      wfd_reverse.clear();
      wfd_reverse_split.clear();

      wfd_object = select(wfd_split, i+1);

      // Calculate the polyline boundary of the current wrfdata object
      child_poly = wfd_object.single_boundary(all_points, level%2);

      // Add the boundary as a child of the current polyline
      parent_node.add_child( &(child_poly) );

      // Point to the most recently added child
      n_ptr = parent_node.child;

      while(n_ptr->sibling) {
         n_ptr = n_ptr->sibling;
      }

      // Calculate reverse video field
      wfd_reverse = reverse_video_polyline(wfd_object, n_ptr);

      // Split reverse video field
      wfd_reverse_split = split(wfd_reverse, n_rev_objects);

      if(n_rev_objects > 0) {
         split_field_to_polyline(wfd_reverse_split, n_rev_objects,
                                 *(n_ptr), all_points, level+1);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  reverse_video_polyline()
//  This routine will generate a reverse video WrfData object inside the
//  polyline indicated.
//
///////////////////////////////////////////////////////////////////////////////

WrfData reverse_video_polyline(const WrfData &wrfdata, const Node *n_ptr) {
   WrfData wrfdata_rv;
   BoundingBox bb;
   int x, y;

   // Initialize the reverse video field
   wrfdata_rv = wrfdata;

   // Initialize the bounding box
   bb.x_ll = wrfdata_rv.get_nx();
   bb.y_ll = wrfdata_rv.get_ny();
   bb.x_ur = bb.y_ur = 0.0;

   // Calculate the bounding box of the polyline tree provided
   n_ptr->bounding_box(bb);

   //
   // Create a reverse video field only searching the bounding box
   // to increase efficiency
   //
   for(x= (int) bb.x_ll; x<=bb.x_ur; x++) {
      for(y= (int) bb.y_ll; y<=bb.y_ur; y++) {

         //
         // Consider each point on or inside the polyline
         // Zero out all boundary points
         //
         if(n_ptr->is_polyline_point(x,y)) {

            wrfdata_rv.put_xy_int(0, x, y);

         }
         //
         // Switch 0/1 interior values
         //
         else if(n_ptr->is_inside(x,y)) {

               if(wrfdata.s_is_on(x,y)) wrfdata_rv.put_xy_int(0, x, y);

               else wrfdata_rv.put_xy_int(1, x, y);

         }
         //
         // Zero out everything else
         //
         else {

            wrfdata_rv.put_xy_int(0, x, y);

         }
      }
   }

   return(wrfdata_rv);
}

///////////////////////////////////////////////////////////////////////////////
//
//  zero_field_polyline()
//  This routine will zero out all of the data in the in the field provided
//  that is contained within the polyline provided.
//
///////////////////////////////////////////////////////////////////////////////

void zero_field_polyline(WrfData &wrfdata, const Node *n_ptr) {
   int x, y;
   BoundingBox bb;

   // Initialize the bounding box
   bb.x_ll = wrfdata.get_nx();
   bb.y_ll = wrfdata.get_ny();
   bb.x_ur = bb.y_ur = 0.0;

   // Calculate the bounding box of the polyline provided
   n_ptr->bounding_box(bb);

   // Only search the bounding box of the polyline to increase efficiency
   for(x= (int) bb.x_ll; x<=bb.x_ur; x++) {
      for(y= (int) bb.y_ll; y<=bb.y_ur; y++) {

         if(n_ptr->is_inside(x, y)) {
            wrfdata.put_xy_int(0, x, y);
         }
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  zero_field()
//  This routine will zero out all of the data in the in the field
//
///////////////////////////////////////////////////////////////////////////////

void zero_field(WrfData &wrfdata) {
   int x, y;

   // Only search the bounding box of the polyline to increase efficiency
   for(x=0; x<wrfdata.get_nx(); x++) {
      for(y=0; y<wrfdata.get_ny(); y++) {

         wrfdata.put_xy_int(0, x, y);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void polyline_to_wrfdata(Polyline &poly, WrfData &wrfdata) {
   int x, y;
   BoundingBox bb;

   //
   // Initiatlize the wrfdata object
   //
   for(x=0; x<wrfdata.get_nx(); x++) {
      for(y=0; y<wrfdata.get_ny(); y++) {

         wrfdata.put_xy_int(0, x, y);
      }
   }

   //
   // Initialize the bounding box
   //
   bb.x_ll = wrfdata.get_nx();
   bb.y_ll = wrfdata.get_ny();
   bb.x_ur = bb.y_ur = 0.0;

   //
   // Calculate the bounding box of the polyline
   //
   poly.bounding_box(bb);

   //
   // Turn on wrfdata grid points inside the polyline
   //
   for(x=nint(bb.x_ll-1); x<=nint(bb.x_ur+1); x++) {
      for(y=nint(bb.y_ll-1); y<=nint(bb.y_ur+1); y++) {

         if(poly.is_inside(x, y)) {
            wrfdata.put_xy_int(wrfdata_int_data_max, x, y);
         }
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

bool is_inside_bb(const BoundingBox &bb, double x, double y) {
   bool inside = false;

   if( is_between(bb.x_ll, bb.x_ur, x)
    && is_between(bb.y_ll, bb.y_ur, y) ) {
      inside = true;
   }

   return(inside);
}

///////////////////////////////////////////////////////////////////////////////

bool is_between(double a, double b, double x) {
   bool between = false;

   if( (x >= a && x <= b)      // a <= b
    || (x <= a && x >= b) ) {  // a > b
      between = true;
   }

   return(between);
}

///////////////////////////////////////////////////////////////////////////////

double dot(double x_1, double y_1, double x_2, double y_2) {
   double d;

   d = x_1*x_2 + y_1*y_2;

   return ( d );
}

///////////////////////////////////////////////////////////////////////////////

WrfData select(const WrfData &id, int n) {
   int k, x, y;
   int nx, ny;
   int count;
   WrfData d = id;

   nx = id.get_nx();
   ny = id.get_ny();

   count = 0;

   for(x=0; x<nx; ++x) {
      for(y=0; y<ny; ++y) {

         k = id.get_xy_int(x, y);

         if(k == n) {
            d.put_xy_int(1, x, y);

            ++count;
         }
         else {
            d.put_xy_int(0, x, y);
         }
      }
   }
   d.calc_moments();

   return(d);
}

///////////////////////////////////////////////////////////////////////////////

WrfData combine(const WrfData *wrfdata, int num_fields) {
   int x, y, n, v_int;
   WrfData u;

   //
   // Initialize u to wrfdata[0]
   //
   u = wrfdata[0];

   for(x=0; x<u.get_nx(); x++) {
      for(y=0; y<u.get_ny(); y++) {

         v_int = 0;

         for(n=0; n<num_fields; n++) {

            if(wrfdata[n].get_xy_int(x, y) > 0) {

               v_int = wrfdata_int_data_max;
            }
         }

         u.put_xy_int(v_int, x, y);
      }
   }
   u.calc_moments();

   return(u);
}

///////////////////////////////////////////////////////////////////////////////

WrfData combine_split(const WrfData *wrfdata, int num_fields) {
   int x, y, n, v_int;
   WrfData u;

   //
   // Initialize u to wrfdata[0]
   //
   u = wrfdata[0];

   for(x=0; x<u.get_nx(); x++) {
      for(y=0; y<u.get_ny(); y++) {

         v_int = 0;

         for(n=0; n<num_fields; n++) {

            if(wrfdata[n].get_xy_int(x, y) > v_int) {

               v_int = wrfdata[n].get_xy_int(x, y);
            }
         }

         u.put_xy_int(v_int, x, y);
      }
   }
   u.calc_moments();

   return(u);
}

///////////////////////////////////////////////////////////////////////////////

void boundary_step(const WrfData &wd, int &xn, int &yn, int &direction) {
   bool lr, ur, ul, ll;

   lr = ur = ul = ll = false;

   //
   // Based on the direction of travel turn on/off lr, ur, ul, ll cells
   //
   switch(direction) {

      case(plus_x):
         if(wd.s_is_on(xn, yn-1)  ) lr = true;
         if(wd.s_is_on(xn+1, yn-1)) ur = true;
         if(wd.s_is_on(xn+1, yn)  ) ul = true;
         if(wd.s_is_on(xn, yn)    ) ll = true;

         xn += 1;
         break;

      case(plus_y):
         if(wd.s_is_on(xn, yn)    ) lr = true;
         if(wd.s_is_on(xn, yn+1)  ) ur = true;
         if(wd.s_is_on(xn-1, yn+1)) ul = true;
         if(wd.s_is_on(xn-1, yn)  ) ll = true;

         yn += 1;
         break;

      case(minus_x):
         if(wd.s_is_on(xn-1, yn)  ) lr = true;
         if(wd.s_is_on(xn-2, yn)  ) ur = true;
         if(wd.s_is_on(xn-2, yn-1)) ul = true;
         if(wd.s_is_on(xn-1, yn-1)) ll = true;

         xn -= 1;
         break;

      case(minus_y):
         if(wd.s_is_on(xn-1, yn-1)) lr = true;
         if(wd.s_is_on(xn-1, yn-2)) ur = true;
         if(wd.s_is_on(xn, yn-2)  ) ul = true;
         if(wd.s_is_on(xn, yn-1)  ) ll = true;

         yn -= 1;
         break;

      default:
         cerr << "\n\nERROR: boundary_step() -> "
              << "bad direction: " << direction << "\n\n" << flush;
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

         cerr << "\n\nERROR: boundary_step() -> "
              << "bad step case: "
              << get_step_case(lr, ur, ul, ll) << "\n\n" << flush;
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
      cerr << "\n\nERROR: get_step_case() -> "
           << "invalid combination: (lr, ur, ul, ll) = (" << lr << ", "
           << ur << ", " << ul << ", " << ll << ")\n\n" << flush;
      exit(1);
   }

   return(-1);
}

///////////////////////////////////////////////////////////////////////////////

int wrfdata_intersection(const WrfData &f1, const WrfData &f2) {
   int x, y, intersection;

   //
   // Check for the same grid dimension
   //
   if(f1.get_nx() != f2.get_nx() ||
      f1.get_ny() != f2.get_ny() ) {

      cerr << "\n\nERROR: wrfdata_intersection() -> "
           << "grid dimensions do not match\n\n" << flush;
      exit(1);
   }

   intersection = 0;
   for(x=0; x<f1.get_nx(); x++) {
      for(y=0; y<f1.get_ny(); y++) {

         if(f1.f_is_on(x, y) && f2.f_is_on(x, y)) intersection++;
      } // end for y
   } // end for x

   return(intersection);
}

///////////////////////////////////////////////////////////////////////////////

void apply_mask(WrfData &f, const char *fname) {
   WrfData mask;

   //
   // Open up the wrfdata file containing the mask information
   //
   mask.read(fname);

   apply_mask(f, mask);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_mask(WrfData &f, WrfData &mask) {
   int x, y;

   if(f.get_nx() != mask.get_nx() ||
      f.get_ny() != mask.get_ny() ) {

      cerr << "\n\nERROR: apply_mask() -> "
           << "grid dimensions do not match\n\n" << flush;
      exit(1);
   }

   for(x=0; x<f.get_nx(); x++) {
      for(y=0; y<f.get_ny(); y++) {

         //
         // Put bad data everywhere the mask is turned off
         //
         if(!mask.s_is_on(x, y)) f.put_xy_int(bad_data_flag, x, y);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void apply_mask(const WrfData &fcst_wd, const WrfData &obs_wd,
                const WrfData &mask_wd,
                NumArray &f_na, NumArray &o_na) {
   int x, y;

   // Initialize the NumArray objects
   f_na.clear();
   o_na.clear();

   if(fcst_wd.get_nx() != obs_wd.get_nx() ||
      fcst_wd.get_ny() != obs_wd.get_ny() ||
      fcst_wd.get_nx() != mask_wd.get_nx() ||
      fcst_wd.get_ny() != mask_wd.get_ny()) {

      cerr << "\n\nERROR: apply_mask() -> "
           << "grid dimensions do not match\n\n" << flush;
      exit(1);
   }

   // Store the pairs in NumArray objects
   for(x=0; x<fcst_wd.get_nx(); x++) {
      for(y=0; y<fcst_wd.get_ny(); y++) {

         // Skip any grid points containing bad data values for
         // either of the raw fields or where the verification
         // masking region is turned off
         if(fcst_wd.is_bad_xy(x, y) ||
            obs_wd.is_bad_xy(x, y)  ||
            !mask_wd.s_is_on(x, y) ) continue;

         f_na.add(fcst_wd.get_xy_double(x, y));
         o_na.add(obs_wd.get_xy_double(x, y));
      } // end for y
   } // end for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void mask_bad_data(WrfData &wd, const WrfData &wd_mask) {
   int x, y;

   if(wd.get_nx() != wd_mask.get_nx() ||
      wd.get_ny() != wd_mask.get_ny() ) {

      cerr << "\n\nERROR: mask_bad_data() -> "
           << "grid dimensions do not match\n\n" << flush;
      exit(1);
   }

   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {

         if(wd_mask.is_bad_xy(x,y))
            wd.put_xy_int(bad_data_flag, x, y);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Compare the values of the two field point by point.  If they both match
// the value indicated, replace the values in both fields with bad data.
// Keep track of the number of replacements.
//
///////////////////////////////////////////////////////////////////////////////

int mask_double_double(WrfData &wd1, WrfData &wd2, double d) {
   int x, y, n;

   if(wd1.get_nx() != wd2.get_nx() ||
      wd1.get_ny() != wd2.get_ny() ) {

      cerr << "\n\nERROR: mask_double_double() -> "
           << "grid dimensions do not match\n\n" << flush;
      exit(1);
   }

   n = 0;
   for(x=0; x<wd1.get_nx(); x++) {
      for(y=0; y<wd1.get_ny(); y++) {

         if(is_eq(wd1.get_xy_double(x,y), d) &&
            is_eq(wd2.get_xy_double(x,y), d)) {

            wd1.put_xy_int(bad_data_flag, x, y);
            wd2.put_xy_int(bad_data_flag, x, y);

            n++;
         }
      }
   }

   return(n);
}

///////////////////////////////////////////////////////////////////////////////
//
// Compare the values of the two arrays point by point.  If they both match
// the value indicated, replace the values in both fields with bad data.
// Keep track of the number of replacements.
//
///////////////////////////////////////////////////////////////////////////////

int mask_double_double(NumArray &na1, NumArray &na2, double d) {
   int i, n;

   if(na1.n_elements() != na2.n_elements()) {

      cerr << "\n\nERROR: mask_double_double() -> "
           << "array lengths do not match\n\n" << flush;

      exit(1);
   }

   n = 0;
   for(i=0; i<na1.n_elements(); i++) {

      if(is_eq(na1[i], d) && is_eq(na2[i], d)) {
         na1.set(i, bad_data_double);
         na2.set(i, bad_data_double);
         n++;
      }
   }

   return(n);
}

///////////////////////////////////////////////////////////////////////////////
//
// Convert the WrfData field to the corresponding fractional coverage meeting
// the threshold critea specified.
//
///////////////////////////////////////////////////////////////////////////////

WrfData fractional_coverage(const WrfData &wd, int wdth, SingleThresh t,
                            double vld_t) {
   int i, j, k, n, x, y, x_ll, y_ll, y_ur, xx, yy, half_width;
   double v;
   int count_vld, count_thr;
   WrfData frac_wd;
   NumArray box_na;

   // Check that width is set to 1 or greater
   if(wdth < 1) {
      cerr << "\n\nERROR: fractional_coverage() -> "
           << "width must be set to a value of 1 or greater.\n\n" << flush;
      exit(1);
   }

   // Set up the WrfData coverage field with a range of values of [0, 1]
   frac_wd.set_size(wd.get_nx(), wd.get_ny());
   frac_wd.set_valid_time(wd.get_valid_time());
   frac_wd.set_lead_time(wd.get_lead_time());
   frac_wd.set_accum_time(wd.get_accum_time());
   frac_wd.set_m(1.0/wrfdata_int_data_max);
   frac_wd.set_b(0.0);

   // Compute the box half-width
   half_width = (wdth - 1)/2;

   // Initialize the box
   for(i=0; i<wdth*wdth; i++) box_na.add(bad_data_int);

   // Compute the fractional coverage meeting the threshold criteria
   for(x=0; x<wd.get_nx(); x++) {

      // Find the lower-left x-coordinate of the neighborhood
      x_ll = x - half_width;

      for(y=0; y<wd.get_ny(); y++) {

         // Find the lower-left y-coordinate of the neighborhood
         y_ll = y - half_width;
         y_ur = y + half_width;

         // Initialize the box for this new column
         if(y == 0) {

            // Initialize counts
            count_vld = count_thr = 0;

            for(i=0; i<wdth; i++) {

               xx = x_ll + i;

               for(j=0; j<wdth; j++) {

                  yy = y_ll + j;

                  n = two_to_one(wdth, wdth, i, j);

                  // Check for being off the grid
                  if(xx < 0 || xx >= wd.get_nx() ||
                     yy < 0 || yy >= wd.get_ny()) {
                     k = bad_data_int;
                  }
                  // Check the value of v to see if it meets the threshold criteria
                  else {
                     v = wd.get_xy_double(xx, yy);
                     if(is_bad_data(v))  k = bad_data_int;
                     else if(t.check(v)) k = 1;
                     else                k = 0;
                  }
                  box_na.set(n, k);

                  // Increment the counts
                  if(!is_bad_data(k)) {
                     count_vld += 1;
                     count_thr += k;
                  }

               } // end for j
            } // end for i
         } // end if

         // Otherwise, update one row of the box
         else {

            // Compute the row of the neighborhood box to be updated
            j = (y - 1) % wdth;

            for(i=0; i<wdth; i++) {

               // Index into the box
               n = two_to_one(wdth, wdth, i, j);

               // Get x and y values to be checked
               xx = x_ll + i;
               yy = y_ur;

               // Decrement counts for data to be replaced
               k = box_na[n];
               if(!is_bad_data(k)) {
                  count_vld -= 1;
                  count_thr -= k;
               }

               // Check for being off the grid
               if(xx < 0 || xx >= wd.get_nx() ||
                  yy < 0 || yy >= wd.get_ny()) {
                  k = bad_data_int;
               }
               // Check the value of v to see if it meets the threshold criteria
               else {
                  v = wd.get_xy_double(xx, yy);
                  if(is_bad_data(v))  k = bad_data_int;
                  else if(t.check(v)) k = 1;
                  else                k = 0;
               }
               box_na.set(n, k);

               // Increment the counts
               if(!is_bad_data(k)) {
                  count_vld += 1;
                  count_thr += k;
               }

            } // end for i
         } // end else

         // Check whether enough valid grid points were found
         if((double) count_vld/(wdth*wdth) < vld_t ||
            count_vld == 0) {
            v = bad_data_double;
         }
         // Compute the fractional coverage
         else {
            v = (double) count_thr/count_vld;
         }

         // Store the fractional coverage value
         frac_wd.put_xy_double(v, x, y);

      } // end for y
   } // end for x

   return(frac_wd);
}

////////////////////////////////////////////////////////////////////////

WrfData smooth_field(const WrfData &wd, InterpMthd mthd, int wdth, double t) {
   WrfData smooth_wd;
   double v;
   int x, y, x_ll, y_ll;

   // Initialize the smoothed field to the raw field
   smooth_wd = wd;

   // Check for a width value of 1 for which no smoothing should
   // be performed
   if(wdth <= 1) return(smooth_wd);

   // Otherwise, apply smoothing to each grid point
   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {

         // The neighborhood width must be odd, find the lower-left
         // corner of the neighborhood
         x_ll = x - (wdth - 1)/2;
         y_ll = y - (wdth - 1)/2;

         // Compute the smoothed value based on the interpolation method
         switch(mthd) {

            case(im_min):     // Minimum
               v = interp_min(wd, x_ll, y_ll, wdth, t);
               break;

            case(im_max):     // Maximum
               v = interp_max(wd, x_ll, y_ll, wdth, t);
               break;

            case(im_median):  // Median
               v = interp_median(wd, x_ll, y_ll, wdth, t);
               break;

            case(im_uw_mean): // Unweighted Mean
               v = interp_uw_mean(wd, x_ll, y_ll, wdth, t);
               break;

            // Distance-weighted mean is omitted here since it is not
            // an option for gridded data

            // Least-squares fit is omitted here since it is not
            // an option for gridded data

            default:
               cerr << "\n\nERROR: smooth_field() -> "
                    << "unexpected interpolation method encountered: "
                    << mthd << "\n\n" << flush;
               exit(1);
               break;
         }

         // Store the smoothed value
         smooth_wd.put_xy_double(v, x, y);
      } // end for y
   } // end for x

   return(smooth_wd);
}

////////////////////////////////////////////////////////////////////////

double interp_min(const WrfData &wd, int x_ll, int y_ll, int wdth, double t) {
   int x, y, count;
   double v, min_v;

   // Search the neighborhood for the minimum value
   count = 0;
   min_v = 1.0e30;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= wd.get_nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= wd.get_ny()) continue;
         if(wd.is_bad_xy(x, y))        continue;

         v = wd.get_xy_double(x, y);
         if(v < min_v) min_v = v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to trust
   // the minimum value computed
   if( (double) count/(wdth*wdth) < t
    || count == 0) {
      min_v = bad_data_double;
   }

   return(min_v);
}

////////////////////////////////////////////////////////////////////////

double interp_max(const WrfData &wd, int x_ll, int y_ll, int wdth, double t) {
   int x, y, count;
   double v, max_v;

   // Search the neighborhood for the maximum value
   count = 0;
   max_v = -1.0e30;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= wd.get_nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= wd.get_ny()) continue;
         if(wd.is_bad_xy(x, y))        continue;

         v = wd.get_xy_double(x, y);
         if(v > max_v) max_v = v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to trust
   // the maximum value computed
   if( (double) count/(wdth*wdth) < t
    || count == 0) {
      max_v = bad_data_double;
   }

   return(max_v);
}

////////////////////////////////////////////////////////////////////////

double interp_median(const WrfData &wd, int x_ll, int y_ll, int wdth, double t) {
   double *data;
   int x, y, count;
   double v, median_v;

   // Allocate space to store the data points
   data = new double [wdth*wdth];

   // Search the neighborhood for valid data points
   count = 0;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= wd.get_nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= wd.get_ny()) continue;
         if(wd.is_bad_xy(x, y))        continue;

         v = wd.get_xy_double(x, y);
         data[count] = v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to compute
   // a median value
   if( (double) count/(wdth*wdth) < t
    || count == 0) {
      median_v = bad_data_double;
   }
   else {
      sort(data, count);
      median_v = percentile(data, count, 0.50);
   }

   if(data) { delete [] data; data = (double *) 0; }

   return(median_v);
}

////////////////////////////////////////////////////////////////////////

double interp_uw_mean(const WrfData &wd, int x_ll, int y_ll, int wdth, double t) {
   int x, y, count;
   double v, sum, uw_mean_v;

   // Sum up the valid data in the neighborhood
   count = 0;
   sum = 0.0;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= wd.get_nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= wd.get_ny()) continue;
         if(wd.is_bad_xy(x, y))        continue;

         v = wd.get_xy_double(x, y);
         sum += v;
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to compute
   // a mean value
   if( (double) count/(wdth*wdth) < t
    || count == 0) {
      uw_mean_v = bad_data_double;
   }
   else {
      uw_mean_v = sum/count;
   }

   return(uw_mean_v);
}

////////////////////////////////////////////////////////////////////////
//
// Compute the distance-weighted mean using Shepards Method
//
////////////////////////////////////////////////////////////////////////

double interp_dw_mean(const WrfData &wd, int x_ll, int y_ll, int wdth,
                      double obs_x, double obs_y, int i_pow, double t) {
   double *data, *dist;
   int i, x, y, count;
   double v, dw_mean_v, w, wght_sum;

   // Allocate space to store the data points
   data = new double [wdth*wdth];
   dist = new double [wdth*wdth];

   // Search the neighborhood for valid data points
   count = 0;
   for(x=x_ll; x<x_ll+wdth; x++) {
      if(x < 0 || x >= wd.get_nx()) continue;

      for(y=y_ll; y<y_ll+wdth; y++) {
         if(y < 0 || y >= wd.get_ny()) continue;
         if(wd.is_bad_xy(x, y))        continue;

         v = wd.get_xy_double(x, y);
         data[count] = v;
         dist[count] = sqrt(pow((obs_x-x), 2) + pow((obs_y-y), 2));
         count++;
      } // end for y
   } // end for x

   // Check whether enough valid grid points were found to compute
   // a distance weighted mean value
   if( (double) count/(wdth*wdth) < t
    || count == 0) {
      dw_mean_v = bad_data_double;
   }
   else {

      // Compute the sum of the weights
      for(i=0, wght_sum=0; i<count; i++) {

         // If the distance is very close to zero,
         // break out of the loop
         if(dist[i] <= 0.001) break;

         // Otherwise, compute the sum of the weights
         wght_sum += pow(dist[i], -1*i_pow);
      }

      for(i=0, dw_mean_v=0; i<count; i++) {

         // If the distance is very close to zero, set the dw_mean_v
         // value to the current data value and break out of the loop
         if(dist[i] <= 0.001) {
            dw_mean_v = data[i];
            break;
         }

         w = pow(dist[i], -1*i_pow)/wght_sum;
         dw_mean_v += w*data[i];
      }
   }

   if(data) { delete [] data; data = (double *) 0; }
   if(dist) { delete [] dist; dist = (double *) 0; }

   return(dw_mean_v);
}

////////////////////////////////////////////////////////////////////////
//
// Compute a least-squares fit interpolation
//
////////////////////////////////////////////////////////////////////////

double interp_ls_fit(const WrfData &wd, int x_ll, int y_ll, int wdth,
                     double obs_x, double obs_y, double t) {
   int i, j, x, y, count;
   const int N  = wdth;
   const int N2 = N*N;
   const double alpha     = (N2*(N2 - 1.0))/12.0;
   const double beta      = 0.5*(N - 1.0);
   const double x_center  = x_ll + beta;
   const double y_center  = y_ll + beta;
   const double u_test    = obs_x - x_center;
   const double v_test    = obs_y - y_center;
   double A, B, C;
   double suz, svz, sz;
   double u, v, z;

   if(N < 2) {
      cerr << "\n\nERROR: interp_ls_fit() -> "
           << "the interpolation width (" << N
           << ") must be set >= 2\n\n" << flush;

      exit (1);
   }

   suz = svz = sz = 0.0;

   // Search the neighborhood
   count = 0;
   for(i=0; i<N; i++) {

      u = i - beta;
      x = x_ll + i;

      if(x < 0 || x >= wd.get_nx()) continue;

      for(j=0; j<N; j++) {

         v = j - beta;
         y = y_ll + j;

         if(y < 0 || y >= wd.get_ny()) continue;
         if(wd.is_bad_xy(x, y))        continue;

         z = wd.get_xy_double(x, y);
         count++;

         suz += u*z;
         svz += v*z;
         sz  += z;
      }
   }

   A = suz/alpha;
   B = svz/alpha;
   C = sz/N2;

   z = A*u_test + B*v_test + C;

   // Check for not enough valid data
   if( (double) count/N2 < t || count == 0) {
      z = bad_data_double;
   }

   return(z);
}

///////////////////////////////////////////////////////////////////////////////
//
//  End Code for Miscellaneous Functions
//
///////////////////////////////////////////////////////////////////////////////
