

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SHP_TYPE_FILE_H__
#define  __VX_SHP_TYPE_FILE_H__


////////////////////////////////////////////////////////////////////////


enum ShapeType {

   shape_type_null_shape   =  0,

   shape_type_point        =  1,
   shape_type_polyline     =  3,
   shape_type_polygon      =  5,
   shape_type_multipoint   =  8,

   shape_type_point_z      = 11,
   shape_type_polyline_z   = 13,
   shape_type_polygon_z    = 15,
   shape_type_multipoint_z = 18,

   shape_type_point_m      = 21,
   shape_type_polyline_m   = 23,
   shape_type_polygon_m    = 25,
   shape_type_multipoint_m = 28,

   shape_type_multipatch   = 31,

};


////////////////////////////////////////////////////////////////////////


struct ShpPoint {

   double x;
   double y;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SHP_TYPE_FILE_H__  */


////////////////////////////////////////////////////////////////////////


