

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "goes_grid.h"


////////////////////////////////////////////////////////////////////////


//static double     lc_func(double lat, double Cone, const bool is_north);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GoesImagerGrid
   //


////////////////////////////////////////////////////////////////////////


GoesImagerGrid::GoesImagerGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


GoesImagerGrid::~GoesImagerGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void GoesImagerGrid::clear()

{

Nx = 0;
Ny = 0;

Name.clear();

memset(&Data, 0, sizeof(Data));

return;

}


////////////////////////////////////////////////////////////////////////


GoesImagerGrid::GoesImagerGrid(const GoesImagerData & data)

{

clear();


Nx = data.nx;
Ny = data.ny;

Name = data.name;

Data = data;

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


void GoesImagerGrid::latlon_to_xy(double lat, double lon, double & x_idx, double & y_idx) const

{

  static const string methodName = "GoesRnetCDF2Mdv::_latLon2XY()";
  // 0.0174533
  float c_lat = atan(Data.inv_radius_ratio2*tan(lat*deg_per_rad));
  float cos_clat = cos(c_lat);

  float rc = Data.semi_minor_axis/sqrt(1.0 - pow(Data.ecc*cos_clat, 2.0));
      
  float del_lon_angle = (lon - Data.lon_of_projection_origin)*deg_per_rad;

  float sx = Data.H - (rc*cos_clat*cos(del_lon_angle));
  float sy = -rc*cos_clat*sin(del_lon_angle);
  float sz = rc*sin(c_lat);
      
  // // check that point is on disk of the earth
  if((Data.H*(Data.H - sx)) < (sy*sy + Data.radius_ratio2*sz*sz)) {
    x_idx = -1;
    y_idx = -1;
    return;
  }

  float rl = sqrt((sx*sx + sy*sy + sz*sz));
  float xx = asin((-sy/rl));
  float yy = atan((sz/sx));

  
  x_idx = round((xx - Data.x_image_bounds[0])/Data.dx_rad);
  y_idx = round((Data.y_image_bounds[0] - yy)/Data.dy_rad);

  //  cerr << "lat: " << lat << "  lon: " << lon << "  ximage: " << xx << "  yimage: " << yy << endl;
  return;

}


////////////////////////////////////////////////////////////////////////


void GoesImagerGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

static const string method_name = "GoesImagerGrid::xy_to_latlon()";

mlog << Error << "\n" << method_name << " -> "
     << "is not implemented.\n\n";
     
//if ( Has_SO2 )  {
//
//   x -= Data.x_pin;
//   y -= Data.y_pin;
//
//   so2_reverse(x, y);
//
//   x += Data.x_pin;
//   y += Data.y_pin;
//
//}
//
//double r, theta;
//const double H = ( IsNorthHemisphere ? 1.0 : -1.0 );
//
//x = (x - Bx)/(H*Alpha);
//y = (y - By)/(H*Alpha);
//
//r = sqrt( x*x + y*y );
//
//lat = lc_inv_func(r, Cone, IsNorthHemisphere);
//
//if ( fabs(r) < 1.0e-5 )  theta = 0.0;
//else                     theta = atan2d(x, -y);   //  NOT atan2d(y, x);
//
//lon = Lon_orient - theta/(H*Cone);
//
//reduce(lon);

return;

}


////////////////////////////////////////////////////////////////////////

double GoesImagerGrid::calc_area(int x, int y) const

{

  static const string method_name = "GoesImagerGrid::calc_area()";

//double u[4], v[4];
double sum = 0;

mlog << Error << "\n" << method_name << " -> "
     << "is not implemented.\n\n";

// xy_to_uv(x - 0.5, y - 0.5, u[0], v[0]);  //  lower left
// xy_to_uv(x + 0.5, y - 0.5, u[1], v[1]);  //  lower right
// xy_to_uv(x + 0.5, y + 0.5, u[2], v[2]);  //  upper right
// xy_to_uv(x - 0.5, y + 0.5, u[3], v[3]);  //  upper left

//xy_to_uv(x      , y      , u[0], v[0]);  //  lower left
//xy_to_uv(x + 1.0, y      , u[1], v[1]);  //  lower right
//xy_to_uv(x + 1.0, y + 1.0, u[2], v[2]);  //  upper right
//xy_to_uv(x      , y + 1.0, u[3], v[3]);  //  upper left
//
//
//sum = uv_closedpolyline_area(u, v, 4);
//
//sum *= earth_radius_km*earth_radius_km;

return ( sum );
}


////////////////////////////////////////////////////////////////////////


int GoesImagerGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int GoesImagerGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString GoesImagerGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


void GoesImagerGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);



out << prefix << "Name       = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';

out << prefix << "Projection = Goes Imager\n";

out << prefix << "\n";

out << prefix << "Nx         = " << Nx << "\n";
out << prefix << "Ny         = " << Ny << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString GoesImagerGrid::serialize() const

{

ConcatString a;
char junk[256];

a << "Projection: GoesImager";

a << " Nx: " << Nx;
a << " Ny: " << Ny;

//snprintf(junk, sizeof(junk), " Lat_LL: %.3f", Lat_LL);   a << junk;
//snprintf(junk, sizeof(junk), " Lon_LL: %.3f", Lon_LL);   a << junk;

//snprintf(junk, sizeof(junk), " Lon_orient: %.3f", Lon_orient);   a << junk;

//snprintf(junk, sizeof(junk), " Alpha: %.3f", Alpha);   a << junk;

//snprintf(junk, sizeof(junk), " Cone: %.3f", Cone);   a << junk;

//snprintf(junk, sizeof(junk), " Bx: %.4f", Bx);   a << junk;
//snprintf(junk, sizeof(junk), " By: %.4f", By);   a << junk;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


GridInfo GoesImagerGrid::info() const

{

GridInfo i;

i.set(Data);

return ( i );

}


////////////////////////////////////////////////////////////////////////


double GoesImagerGrid::rot_grid_to_earth(int x, int y) const

{

//
// The rotation angle from grid relative to earth relative is zero
// for the Mercator projection in it's standard aspect
//

return ( 0.0 );

}


////////////////////////////////////////////////////////////////////////


bool GoesImagerGrid::is_global() const

{

return ( false );

}


////////////////////////////////////////////////////////////////////////


void GoesImagerGrid::shift_right(int N)

{

if ( N == 0 )  return;

mlog << Error << "\nGoesImagerGrid::shift_right(int) -> "
     << "shifting is not implemented\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


GridRep * GoesImagerGrid::copy() const

{

GoesImagerGrid * p = new GoesImagerGrid (Data);

p->Name = Name;

return ( p );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct GoesImagerData
   //


////////////////////////////////////////////////////////////////////////
// Note: this is called 2500 * 1500 times

void convert_angles_to_geodetic(
      double cos_x, double sin_x, double cos_y, double sin_y,
      double axis_ratio, double data_H, double param_c,
      double semi_major_axis_sqr,
      double &lat_rad, double &lon_rad, bool print_args = false) {
   double a, b, disdance_sat_to_p, h_minus_sx;
   double sx, sy, sz;
   static const string method_name = "goes_grid convert_angles_to_geodetic()";
   
   a = sin_x*sin_x + cos_x*cos_x*(cos_y*cos_y + axis_ratio*sin_y*sin_y);
   b = -2 * data_H * cos_x * cos_y;
   disdance_sat_to_p = (-b - sqrt(b*b - 4*a*param_c)) / (2 * a);
   sx = disdance_sat_to_p * cos_x * cos_y;
   sy = -disdance_sat_to_p * sin_x;
   sz = disdance_sat_to_p * cos_x * sin_y;
   h_minus_sx = data_H - sx;
   lat_rad = atan(axis_ratio * sz / sqrt(h_minus_sx*h_minus_sx + sy*sy));
   lon_rad = atan(sy / h_minus_sx);
   if (print_args) {
      mlog << Debug(5) << method_name << " Intermediate values: "
           << "\n\ta=" << a << "\n\tb: " << b << "\n\tc: " << param_c
           << "\n\tdisdance_sat_to_p: " << disdance_sat_to_p
           << "\n\tsx: " << sx << "\n\tsy: " << sy << "\n\tsz: " << sz << "\n";
   }
}

////////////////////////////////////////////////////////////////////////

void GoesImagerData::compute_lat_lon()
{

   int index, buf_len;
   float lat, lon;
   double lat_rad, lon_rad;
   float lat_min, lat_max, lon_min, lon_max;
   int idx_lat_min, idx_lat_max, idx_lon_min, idx_lon_max;
   double x_rad, cos_x, sin_x;
   double y_rad, cos_y, sin_y;
   double semi_major_axis_sqr = semi_major_axis * semi_major_axis;
   double axis_ratio = semi_major_axis_sqr / (semi_minor_axis*semi_minor_axis);
   double param_c = H * H - semi_major_axis_sqr;
   static const string method_name = "GoesImagerData::compute_lat_lon() ";
   
   lat_min = 90.0;
   lat_max = -90.0;
   lon_min = 360.0;
   lon_max = -360.0;
   idx_lat_min = idx_lat_max = idx_lon_min = idx_lon_max = 0;
   buf_len = nx * ny;

   if ((0 != x_values) && (0 != y_values)) {
      lat_values = new float[buf_len];
      lon_values = new float[buf_len];
      
      // Order matters, checked with binary coordinate file
      for (int yIdx=0; yIdx<ny; yIdx++) {
         y_rad = y_values[yIdx];
         cos_y = cos(y_rad);
         sin_y = sin(y_rad);
         for (int xIdx=0; xIdx<nx; xIdx++) {
            x_rad = x_values[xIdx];
            cos_x = cos(x_rad);
            sin_x = sin(x_rad);
            convert_angles_to_geodetic(cos_x, sin_x, cos_y, sin_y,
                axis_ratio, H, param_c, semi_major_axis_sqr, lat_rad, lon_rad);
            index = yIdx * nx + xIdx;
            if (index >= buf_len)
                mlog << Error << method_name << " index=" << index
                     << "  too big than " << buf_len << "\n";
            else {
               if (isnan(lat_rad)) lat = bad_data_float;
               else {
                  lat = lat_rad * deg_per_rad;
                  if (lat > lat_max) {lat_max = lat; idx_lat_max = index; }
                  if (lat < lat_min) {lat_min = lat; idx_lat_min = index; }
               }
               if (isnan(lon_rad)) lon = bad_data_float;
               else {
                  lon = lon_of_projection_origin - (lon_rad * deg_per_rad);
                  if (lon > lon_max) {lon_max = lon; idx_lon_max = index; }
                  if (lon < lon_min) {lon_min = lon; idx_lon_min = index; }
               }
               lat_values[index] = lat;
               lon_values[index] = lon;
               
               mlog << Debug(10) << method_name << "index=" << index
                    << "  lat: " << lat << "  lon: " << lon
                    << "  lat_rad: " << lat_rad << "  lon_rad: " << lon_rad << "\n";
            }
         }
      }
   }
   
   mlog << Debug(4) << method_name << " lon: " << lon_min << " to " << lon_max
        << ", lat: " << lat_min << " to " << lat_max << " at index "
        << idx_lon_min << " & " << idx_lon_max << ", "
        << idx_lat_min << " & " << idx_lat_max << " from "
        << "  x: " << x_values[0] << " to " << x_values[nx-1]
        << "  y: " << y_values[0] << " to " << y_values[ny-1] << "\n";
}

////////////////////////////////////////////////////////////////////////

void GoesImagerData::copy(const GoesImagerData *from)
{
   name = from->name;
   nx = from->nx;
   ny = from->ny;
   dx_rad = from->dx_rad;
   dy_rad = from->dy_rad;
   H = from->H;
   ecc = from->ecc;
   radius_ratio2 = from->radius_ratio2;
   inv_radius_ratio2 = from->inv_radius_ratio2;
   perspective_point_height = from->perspective_point_height;
   semi_major_axis = from->semi_major_axis;
   semi_minor_axis = from->semi_minor_axis;
   inverse_flattening = from->inverse_flattening;
   lat_of_projection_origin = from->lat_of_projection_origin;
   lon_of_projection_origin = from->lon_of_projection_origin;
   
   int var_x_size = sizeof(from->x_image_bounds[0]);
   int var_y_size = sizeof(from->y_image_bounds[0]);
   int var_x_bound = sizeof(from->x_image_bounds) / var_x_size;
   int var_y_bound = sizeof(from->y_image_bounds) / var_y_size;
   mlog << Debug(5) << "GoesImager copy(): bound count: x=" 
        << var_x_bound << ", y=" << var_y_bound
        << " data size (bytes): x=" << var_x_size << ", y=" << var_y_size << "\n";
   if (0 != var_x_bound) {
      x_image_bounds = new double[var_x_bound];
      memcpy(x_image_bounds, from->x_image_bounds,
            var_x_bound * var_x_size);
   }
   if (0 != var_y_bound) {
      y_image_bounds = new double[var_y_bound];
      memcpy(y_image_bounds, from->y_image_bounds,
            var_y_bound * var_y_size);
   }
   
   if (0 != from->lat_values) {
      if (lat_values) delete[] lat_values;
      lat_values = new float[nx*ny];
      memcpy(lat_values, from->lat_values, nx*ny*sizeof(lat_values[0]));
   }
   //else lat_values = 0;
   
   if (0 != from->lon_values) {
      if (lon_values) delete[] lon_values;
      lon_values = new float[nx*ny];
      memcpy(lon_values, from->lon_values, nx*ny*sizeof(lon_values[0]));
   }
   //else lon_values = 0;

   if (0 != from->x_values) {
      if (x_values) delete[] x_values;
      x_values = new double[nx];
      memcpy(x_values, from->x_values, nx*sizeof(x_values[0]));
   }
   //else x_values = 0;
   
   if (0 != from->y_values) {
      if (y_values) delete[] y_values;
      y_values = new double[ny];
      memcpy(y_values, from->y_values, ny*sizeof(y_values[0]));
   }
   //else from->y_values = 0;

}

////////////////////////////////////////////////////////////////////////

void GoesImagerData::dump()
{

mlog << Debug(4)
     << "\nGoesImager Grid Data:\n"
     << "      name: " << name << "\n"
     << "  scene_id: " << scene_id << "\n"
     << "        nx: " << nx << "\n"
     << "        ny: " << ny << "\n"
     << "    dx_rad: " << dx_rad << "\n"
     << "    dy_rad: " << dy_rad << "\n"
     << "         H: " << H << "\n"
     << "       ecc: " << ecc  << "\n"
     << "      x_image_bounds: " << x_image_bounds[0] << " " << x_image_bounds[1] << "\n"
     << "      y_image_bounds: " << y_image_bounds[0] << " " << y_image_bounds[1] << "\n"
     << "       radius_ratio2: " << radius_ratio2 << "\n"
     << "   inv_radius_ratio2: " << inv_radius_ratio2 << "\n"
     << "  perspective_point_height: " << perspective_point_height << "\n"
     << "           semi_major_axis: " << semi_major_axis << "\n"
     << "           semi_minor_axis: " << semi_minor_axis << "\n"
     << "        inverse_flattening: " << inverse_flattening << "\n"
     << "  lat_of_projection_origin: " << lat_of_projection_origin << "\n"
     << "  lon_of_projection_origin: " << lon_of_projection_origin << "\n"
     << "\n";

}

////////////////////////////////////////////////////////////////////////

void GoesImagerData::reset() {
   lat_values = 0;
   lon_values = 0;
   x_values = 0; //radian
   y_values = 0; //radian
   x_image_bounds = 0;
   y_image_bounds = 0;
   scene_id = 0;
}

////////////////////////////////////////////////////////////////////////

void GoesImagerData::release() {
   if (lat_values) { delete[] lat_values; lat_values=0; }
   if (lon_values) { delete[] lon_values; lon_values=0; }
   if (x_values) { delete[] x_values; x_values=0; }
   if (y_values) { delete[] y_values; y_values=0; }
   if (x_image_bounds) { delete[] x_image_bounds; x_image_bounds=0; }
   if (y_image_bounds) { delete[] y_image_bounds; y_image_bounds=0; }
   if (scene_id) { delete[] scene_id; scene_id=0; }
}

////////////////////////////////////////////////////////////////////////

void GoesImagerData::test() {
   double lat, lon;
   double lat_rad, lon_rad;
   double semi_major_axis_sqr = semi_major_axis * semi_major_axis;
   double axis_ratio = semi_major_axis_sqr / (semi_minor_axis*semi_minor_axis);
   double param_c = H * H - semi_major_axis_sqr;
   static double target_lat = 33.846162;
   static double target_lon = -84.690932;
   static double x_rad = -0.024052;
   static double y_rad = 0.095340;

   double cos_x = cos(x_rad);
   double sin_x = sin(x_rad);
   double cos_y = cos(y_rad);
   double sin_y = sin(y_rad);
   static const string method_name = "GoesImagerData::test() ";

   convert_angles_to_geodetic(cos_x, sin_x, cos_y, sin_y, axis_ratio,
       H, param_c, semi_major_axis_sqr, lat_rad, lon_rad, true);
   lat = lat_rad * deg_per_rad;
   lon = lon_of_projection_origin - (lon_rad * deg_per_rad);

   mlog << Debug(5) << method_name
        << " TEST lat: " << lat << ",  lon: " << lon 
        << "\n\twhich is " << lat_rad << " and " << lon_rad
        << " (rad) from " << x_rad << " and " << y_rad << " (rad)."
        << "\n\tshould be " << target_lat << " (" << (lat - target_lat)
        << ") and " << target_lon << " (" << (lon - target_lon) << ")\n";
   
   if (! is_eq(lat, target_lat, loose_tol)) {
     mlog << Error << method_name << " computed latitude does not match: " << lat
          << " should be " << target_lat << " diff: " << (lat - target_lat) <<  "\n";
   }
   if (! is_eq(lon, target_lon, loose_tol)) {
     mlog << Error << method_name << " computed longitude does not match: " << lon
          << " should be " << target_lon << "  diff: " << (lon - target_lon) << "\n";
   }
}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


Grid::Grid(const GoesImagerData & data)

{

init_from_scratch();

set(data);


}


////////////////////////////////////////////////////////////////////////


void Grid::set(const GoesImagerData & data)

{

clear();

rep = new GoesImagerGrid (data);

if ( !rep )  {

   mlog << Error << "\nGrid::set(const GoesImagerData &) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

}


////////////////////////////////////////////////////////////////////////

