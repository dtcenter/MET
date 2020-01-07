

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SHAPEFILES_POINT_RECORD_H__
#define  __VX_SHAPEFILES_POINT_RECORD_H__


////////////////////////////////////////////////////////////////////////


#include <sys/types.h>
#include <unistd.h>

#include "shp_types.h"
#include "shp_file.h"


////////////////////////////////////////////////////////////////////////


static const int shp_point_record_bytes = 20;


////////////////////////////////////////////////////////////////////////


   //
   //  for point records
   //


struct ShpPointRecord {   //  this should really be a class, not a struct

   ShpRecordHeader rh;


   int shape_type;


   double x;

   double y;


      ////////////////


   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


extern bool operator>>(ShpFile &, ShpPointRecord &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SHAPEFILES_POINT_RECORD_H__  */


////////////////////////////////////////////////////////////////////////


