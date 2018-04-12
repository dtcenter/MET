// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SHAPEFILES_SHP_FILE_H__
#define  __VX_SHAPEFILES_SHP_FILE_H__


////////////////////////////////////////////////////////////////////////


static const int max_shp_parts  =  1000;
static const int max_shp_points = (1 << 17);


////////////////////////////////////////////////////////////////////////


static const int shp_header_bytes = 100;


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

   shape_type_multipatch   = 31

};


////////////////////////////////////////////////////////////////////////


struct ShpFileHeader {

   int file_code;

   int file_length_16;      //  length in 16-bit words

   int file_length_bytes;   //  length in bytes

   int version;

   int shape_type;


   double x_min, x_max; 
   double y_min, y_max;
   double z_min, z_max;
   double m_min, m_max;

      //
      //
      //

   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


struct ShpRecordHeader {


   int record_number_1;   //  NOTE:  1-based
   int record_number_0;   //  NOTE:  0-based


   int content_length_16;

   int content_length_bytes;

      //

   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


struct ShpPoint {

   double x;
   double y;

};


////////////////////////////////////////////////////////////////////////


struct ShpPolylineRecord {   //  this should really be a class, not a struct

   int shape_type;

   double bbox[4];   //  order: x_min, y_min, x_max, y_max

   int n_parts;

   int n_points;

   int parts[max_shp_parts];

   ShpPoint points[max_shp_points];

      //

   double x_min() const;
   double x_max() const;

   double y_min() const;
   double y_max() const;

   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline double ShpPolylineRecord::x_min() const { return ( bbox[0] ); }
inline double ShpPolylineRecord::x_max() const { return ( bbox[2] ); }

inline double ShpPolylineRecord::y_min() const { return ( bbox[1] ); }
inline double ShpPolylineRecord::y_max() const { return ( bbox[3] ); }


////////////////////////////////////////////////////////////////////////


struct ShpPolygonRecord {   //  this should really be a class, not a struct

   int shape_type;

   double bbox[4];   //  order: x_min, y_min, x_max, y_max

   int n_parts;

   int n_points;

   int parts[max_shp_parts];

   ShpPoint points[max_shp_points];

      //

   double x_min() const;
   double x_max() const;

   double y_min() const;
   double y_max() const;

   double lat(int) const;
   double lon(int) const;

   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

   int start_index(int partno) const;
   int  stop_index(int partno) const;

};


////////////////////////////////////////////////////////////////////////


inline double ShpPolygonRecord::x_min() const { return ( bbox[0] ); }
inline double ShpPolygonRecord::x_max() const { return ( bbox[2] ); }

inline double ShpPolygonRecord::y_min() const { return ( bbox[1] ); }
inline double ShpPolygonRecord::y_max() const { return ( bbox[3] ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SHAPEFILES_SHP_FILE_H__  */


////////////////////////////////////////////////////////////////////////


