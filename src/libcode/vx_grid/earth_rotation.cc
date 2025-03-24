// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_math.h"
#include "vx_vector.h"
#include "vx_util.h"

#include "earth_rotation.h"
#include "latlon_xyz.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class EarthRotation
   //


////////////////////////////////////////////////////////////////////////


EarthRotation::EarthRotation()

{


}


////////////////////////////////////////////////////////////////////////


EarthRotation::~EarthRotation()

{


}


////////////////////////////////////////////////////////////////////////


void EarthRotation::latlon_rot_to_true(double lat_rot, double lon_rot, double & lat_true, double & lon_true) const

{

double x_rot, y_rot, z_rot;
double x_true, y_true, z_true;


grid_latlon_to_xyz(lat_rot, lon_rot, x_rot, y_rot, z_rot);

SO3::reverse(x_rot, y_rot, z_rot, x_true, y_true, z_true);

grid_xyz_to_latlon(x_true, y_true, z_true, lat_true, lon_true);


return;

}


////////////////////////////////////////////////////////////////////////


void EarthRotation::latlon_true_to_rot(double lat_true, double lon_true, double & lat_rot, double & lon_rot) const

{

double x_rot, y_rot, z_rot;
double x_true, y_true, z_true;


grid_latlon_to_xyz(lat_true, lon_true, x_true, y_true, z_true);

SO3::forward(x_true, y_true, z_true, x_rot, y_rot, z_rot);

grid_xyz_to_latlon(x_rot, y_rot, z_rot, lat_rot, lon_rot);

return;

}


////////////////////////////////////////////////////////////////////////


void EarthRotation::set_np(double true_lat_north_pole, double true_lon_north_pole, double aux_rotation)

{

if ( true_lat_north_pole >= 89.9999 )  { set_identity();  return; }

double px, py, pz;
double axis_x, axis_y, axis_z;



grid_latlon_to_xyz(true_lat_north_pole, true_lon_north_pole, px, py, pz);

cross_product(px, py, pz, 0.0, 0.0, 1.0, axis_x, axis_y, axis_z);

normalize(axis_x, axis_y, axis_z);

set_axis_angle(axis_x, axis_y, axis_z, 90.0 - true_lat_north_pole);

if ( aux_rotation != 0.0 )  post_axis_angle(px, py, pz, aux_rotation);

return;

}


////////////////////////////////////////////////////////////////////////


void EarthRotation::set_rng_azi_center(double lat_center, double lon_center)

{

clear();

   //
   // MET #2841 define the rotation by subtracting 90 degrees
   // instead of 180 to define Range/Azimuth grids as pointing east
   // instead of north.
   //

set_np(lat_center, lon_center, lon_center - 90.0);

   //
   //
   //

return;

}


////////////////////////////////////////////////////////////////////////

