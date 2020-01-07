// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SHAPEFILES_SHX_FILE_H__
#define  __VX_SHAPEFILES_SHX_FILE_H__


////////////////////////////////////////////////////////////////////////


#include "shp_file.h"


////////////////////////////////////////////////////////////////////////


typedef ShpFileHeader ShxFileHeader;   //  they're the same


////////////////////////////////////////////////////////////////////////


struct ShxRecord {

   int offset_16;

   int offset_bytes;

   int content_length_16;

   int content_length_bytes;

      //

   void set(unsigned char * buf);

   void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SHAPEFILES_SHX_FILE_H__  */


////////////////////////////////////////////////////////////////////////


