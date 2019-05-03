

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SHAPEFILES_POLY_RECORD_H__
#define  __VX_SHAPEFILES_POLY_RECORD_H__


////////////////////////////////////////////////////////////////////////


#include <sys/types.h>
#include <unistd.h>

#include "shp_types.h"
#include "shp_file.h"
#include "shp_array.h"


////////////////////////////////////////////////////////////////////////


// static const int max_shp_poly_parts  =   600;
// static const int max_shp_poly_points = (1 << 17);


////////////////////////////////////////////////////////////////////////


   //
   //  for both polyline and polygon records
   //


struct ShpPolyRecord {   //  this should really be a class, not a struct

   ShpRecordHeader rh;

   int shape_type;

   double bbox[4];   //  order: x_min, y_min, x_max, y_max

   int n_parts;

   int n_points;

   Shp_Array<int> parts;

   Shp_Array<ShpPoint> points;

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

   bool is_closed() const;

   void toggle_longitudes(); 

};


////////////////////////////////////////////////////////////////////////


inline double ShpPolyRecord::x_min() const { return ( bbox[0] ); }
inline double ShpPolyRecord::x_max() const { return ( bbox[2] ); }

inline double ShpPolyRecord::y_min() const { return ( bbox[1] ); }
inline double ShpPolyRecord::y_max() const { return ( bbox[3] ); }

inline bool   ShpPolyRecord::is_closed() const { return ( shape_type == shape_type_polygon ); }


////////////////////////////////////////////////////////////////////////


extern bool operator>>(ShpFile &, ShpPolyRecord &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SHAPEFILES_POLY_RECORD_H__  */


////////////////////////////////////////////////////////////////////////


