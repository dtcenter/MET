

////////////////////////////////////////////////////////////////////////


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
#include "vx_vector.h"
#include "vx_util.h"

#include "earth_rotation.h"
#include "latlon_xyz.h"


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




   //
   //  
   //

return;

}


////////////////////////////////////////////////////////////////////////


void EarthRotation::set_tcrmw(double lat_center, double lon_center)

{

// const double clat = cosd(lat_center);
// const double slat = sind(lat_center);
// 
// const double clon = cosd(lon_center);
// const double slon = sind(lon_center);

clear();

   //
   //  first column is image of I
   //
/*
M11 = -clon;
M21 =  slon;
M31 =   0.0;

   //
   //  second column is image of J
   //

M12 = -slat*slon;
M22 = -slat*clon;
M32 =  clat;

   //
   //  third column is image of K
   //

M13 =  clat*slon;
M23 =  clat*clon;
M33 =  slat;
*/

set_np(lat_center, lon_center, lon_center - 180.0);

   //
   //
   //

return;

}


////////////////////////////////////////////////////////////////////////

/*
Vector EarthRotation::forward(const Vector & V) const

{

Vector VV;
double xx, yy, zz;

SO3::forward(V.x(), V.y(), V.z(), xx, yy, zz);

VV.set_xyz(xx, yy, zz);


return ( VV );

}
*/

////////////////////////////////////////////////////////////////////////

/*
Vector EarthRotation::reverse(const Vector & V) const

{

Vector VV;
double xx, yy, zz;

SO3::reverse(V.x(), V.y(), V.z(), xx, yy, zz);

VV.set_xyz(xx, yy, zz);


return ( VV );

}
*/

////////////////////////////////////////////////////////////////////////

/*
void EarthRotation::true_to_rot(double  lat_true, double  lon_true, 
                                double   ve_true, double   vn_true,
                                double & ve_rot,  double & vn_rot) const

{

const Vector E_true = latlon_to_east  (lat_true, lon_true);
const Vector N_true = latlon_to_north (lat_true, lon_true);

const Vector V = ve_true*E_true + vn_true*N_true;

const Vector E_rot = forward(E_true);
const Vector N_rot = forward(N_true);

ve_rot = dot(V, E_rot);
vn_rot = dot(V, N_rot);


   //
   //  done
   //

return;

}
*/

////////////////////////////////////////////////////////////////////////

/*
void EarthRotation::rot_to_true(double  lat_rot,   double  lon_rot,
                                double   ve_rot,   double   vn_rot,
                                double & ve_true,  double & vn_true) const

{

const Vector E_rot = latlon_to_east  (lat_rot, lon_rot);
const Vector N_rot = latlon_to_north (lat_rot, lon_rot);

const Vector V = ve_rot*E_rot + vn_rot*N_rot;

const Vector E_true = reverse(E_rot);
const Vector N_true = reverse(N_rot);

ve_true = dot(V, E_true);
vn_true = dot(V, N_true);





   //
   //  done
   //

return;

}
*/

////////////////////////////////////////////////////////////////////////



