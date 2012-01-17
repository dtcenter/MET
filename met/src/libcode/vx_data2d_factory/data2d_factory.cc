// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_factory.h"
#include "data2d_factory_utils.h"
#include "grdfiletype_to_string.h"
#include "data2d_grib.h"
#include "data2d_nc_met.h"
#include "data2d_nc_pinterp.h"

#ifdef WITH_GRIB2
   #include "data2d_grib2.h"
#endif

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class Met2dDataFileFactory
//
////////////////////////////////////////////////////////////////////////

Met2dDataFile * Met2dDataFileFactory::new_met_2d_data_file(GrdFileType t) {
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   //
   // Switch on file type and instantiate the appropriate class.
   // The Met2dDataFile object is allocated and needs to be deleted by caller.
   //
   
   switch(t) {

      case FileType_Gb1:
         mtddf = new MetGrib1DataFile;
         break;

      case FileType_Gb2:
#ifdef WITH_GRIB2
         mtddf = new MetGrib2DataFile;
#else
         mlog << Error << "\n\n  Met2dDataFileFactory::new_met_2d_data_file() -> "
              << "Support for GRIB2 has not been compiled!\n\n";
         exit(1);
#endif
         break;

      case FileType_NcMet:
         mtddf = new MetNcMetDataFile;
         break;

      case FileType_NcPinterp:
         mtddf = new MetNcPinterpDataFile;
         break;

      case FileType_NcCF:
      case FileType_HdfEos:

         mlog << Error << "\n\n  Met2dDataFileFactory::new_met_2d_data_file() -> "
              << "Support for GrdFileType = \"" << grdfiletype_to_string(t)
              << "\" not yet implemented!\n\n";
         exit(1);
         break;

      case FileType_Bufr:

         mlog << Error << "\n\n  Met2dDataFileFactory::new_met_2d_data_file() -> "
              << "cannot use this factory to read files of type \""
              << grdfiletype_to_string(t) << "\"\n\n";
         exit(1);
         break;

      case FileType_None:
         // For FileType_None, silently return a NULL pointer
         mtddf = (Met2dDataFile *) 0;
         break;

      default:
         mlog << Error << "\n\n  Met2dDataFileFactory::new_met_2d_data_file() -> "
              << "unsupported gridded data file type \"" << t
              << "\"\n\n";
         exit(1);
         break;

   } // end switch

   return(mtddf);
}

////////////////////////////////////////////////////////////////////////

Met2dDataFile * Met2dDataFileFactory::new_met_2d_data_file(ConcatString filename) {
   GrdFileType type;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   //
   // Determine the file type
   //
   type = grd_file_type(filename);

   //
   // Create a new data file object and call open if the point is non-zero
   //
   mtddf = new_met_2d_data_file(type);
   if(mtddf) mtddf->open(filename);

   return(mtddf);
}

///////////////////////////////////////////////////////////////////////////////
