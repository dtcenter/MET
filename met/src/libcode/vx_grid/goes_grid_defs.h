

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __GOES_IMAGER_GRID_DEFINITIONS_H__
#define  __GOES_IMAGER_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"
////////////////////////////////////////////////////////////////////////

struct GoesImagerData {

   const char *name;
   const char *scene_id;

   double perspective_point_height;
   double semi_major_axis;
   double semi_minor_axis;
   double inverse_flattening;
   double lat_of_projection_origin;
   double lon_of_projection_origin;
   const char *sweep_angle_axisconst;
   
   int nx;
   int ny;

   double dx_rad;
   double dy_rad;

   //std::vector< float > x_image_bounds;
   //std::vector< float > y_image_bounds;
   double *x_image_bounds;
   double *y_image_bounds;
   
   double ecc;
   double radius_ratio2;
   double inv_radius_ratio2;
   double H;
   //int _xSubSatIdx;
   //int _ySubSatIdx;
   
   float *lat_values;
   float *lon_values;
   double *x_values; //radian
   double *y_values; //radian
   
   void dump();
   void compute_lat_lon();
   void copy(const GoesImagerData *from);
   void reset();
   void release();
   void test();

};

////////////////////////////////////////////////////////////////////////


#endif   //  __GOES_IMAGER_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////



