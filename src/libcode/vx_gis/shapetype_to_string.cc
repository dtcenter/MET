// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //
   //     Created by enum_to_string from file "shp_types.h"
   //
   //     on April 27, 2022   2:35 pm MDT
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "shapetype_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString shapetype_to_string(const ShapeType t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case shape_type_null_shape:     s = "shape_type_null_shape";     break;
   case shape_type_point:          s = "shape_type_point";          break;
   case shape_type_polyline:       s = "shape_type_polyline";       break;
   case shape_type_polygon:        s = "shape_type_polygon";        break;
   case shape_type_multipoint:     s = "shape_type_multipoint";     break;

   case shape_type_point_z:        s = "shape_type_point_z";        break;
   case shape_type_polyline_z:     s = "shape_type_polyline_z";     break;
   case shape_type_polygon_z:      s = "shape_type_polygon_z";      break;
   case shape_type_multipoint_z:   s = "shape_type_multipoint_z";   break;
   case shape_type_point_m:        s = "shape_type_point_m";        break;

   case shape_type_polyline_m:     s = "shape_type_polyline_m";     break;
   case shape_type_polygon_m:      s = "shape_type_polygon_m";      break;
   case shape_type_multipoint_m:   s = "shape_type_multipoint_m";   break;
   case shape_type_multipatch:     s = "shape_type_multipatch";     break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


