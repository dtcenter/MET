// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __WWMCA_GRID_DEFINITIONS_H__
#define  __WWMCA_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Stereographic grid definitions
   //


                                                    //       name        Hemisphere   scale_lat    lat_pin   lon_pin    x_pin  y_pin  lon_orient      d_km       r_km    nx    ny
                                                    //   ========        ==========   =========    =======   ========   =====  =====  ==========   ========   =======  ====  ====
static const StereographicData wwmca_north_data  = {     "wwmca_north",         'N',       60.0,      90.0,    0.0,     511.0, 511.0,       80.0,  23.79848,  6371.20, 1024, 1024 };
static const StereographicData wwmca_south_data  = {     "wwmca_south",         'S',      -60.0,     -90.0,    0.0,     511.0, 511.0,     -100.0,  23.79848,  6371.20, 1024, 1024 };


////////////////////////////////////////////////////////////////////////


#endif   /*  __WWMCA_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



