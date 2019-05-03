

////////////////////////////////////////////////////////////////////////


#ifndef  __WWMCA_GRID_DEFINITIONS_H__
#define  __WWMCA_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


#include "vx_data_grids/grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Stereographic grid definitions
   //


                                                    //       name        Hemisphere   scale_lat    lat_pin   lon_pin    x_pin  y_pin  lon_orient      d_km       r_km    nx    ny
                                                    //   ========        ==========   =========    =======   ========   =====  =====  ==========   ========   =======  ====  ====
static const StereographicData wwmca_north_data  = {     "wwmca_north",         'N',       60.0,      90.0,    0.0,     511.0, 511.0,       80.0,  23.79848,  6367.47, 1024, 1024 };
static const StereographicData wwmca_south_data  = {     "wwmca_south",         'S',      -60.0,     -90.0,    0.0,     511.0, 511.0,     -100.0,  23.79848,  6367.47, 1024, 1024 };


////////////////////////////////////////////////////////////////////////


#endif   /*  __WWMCA_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



