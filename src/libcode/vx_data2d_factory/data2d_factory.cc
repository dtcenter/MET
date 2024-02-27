// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
#include <exception>

#include "data2d_factory.h"
#include "data2d_factory_utils.h"
#include "grdfiletype_to_string.h"
#include "data2d_grib.h"
#include "data2d_nc_met.h"
#include "data2d_nc_wrf.h"
#include "data2d_nc_cf.h"
#ifdef WITH_UGRID
#include "data2d_ugrid.h"
#endif


#ifdef WITH_PYTHON
#include "data2d_python.h"
#endif

#ifdef WITH_GRIB2
   #include "data2d_grib2.h"
#endif

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class Met2dDataFileFactory
//
////////////////////////////////////////////////////////////////////////

Met2dDataFile * Met2dDataFileFactory::new_met_2d_data_file(GrdFileType type)

{

   Met2dDataFile *mtddf = (Met2dDataFile *) nullptr;
   const char *method_name ="Met2dDataFileFactory::new_met_2d_data_file() -> ";

#ifdef WITH_PYTHON
MetPythonDataFile * p = nullptr;
#endif

   //
   // Switch on file type and instantiate the appropriate class.
   // The Met2dDataFile object is allocated and needs to be deleted by caller.
   //

   switch(type) {

      case FileType_Gb1:
         mtddf = new MetGrib1DataFile;
         break;

      case FileType_Gb2:
#ifdef WITH_GRIB2
         mtddf = new MetGrib2DataFile;
#else
         mlog << Error << "\n" << method_name
              << "Support for GRIB2 has not been compiled!\n"
              << "To read GRIB2 files, recompile with the --enable-grib2 option.\n\n";
         exit(1);
#endif
         break;

      case FileType_NcMet:
         mtddf = new MetNcMetDataFile;
         break;

      case FileType_NcWrf:
      case FileType_NcPinterp:
         mtddf = new MetNcWrfDataFile;
         break;

      case FileType_NcCF:
         mtddf = new MetNcCFDataFile;
         break;

#ifdef WITH_PYTHON

      case FileType_Python_Numpy:
         p = new MetPythonDataFile;
         p->set_type(type);
         mtddf = p;
         break;

      case FileType_Python_Xarray:
         p = new MetPythonDataFile;
         p->set_type(type);
         mtddf = p;
         break;

#else

      case FileType_Python_Numpy:
      case FileType_Python_Xarray:

         python_compile_error(method_name);

#endif

      case FileType_HdfEos:

         mlog << Error << "\n" << method_name
              << "Support for GrdFileType = \"" << grdfiletype_to_string(type)
              << "\" not yet implemented!\n\n";
         exit(1);

      case FileType_Bufr:

         mlog << Error << "\n" << method_name
              << "cannot use this factory to read files of type \""
              << grdfiletype_to_string(type) << "\"\n\n";
         exit(1);

      case FileType_UGrid:
#ifdef WITH_UGRID
         // For FileType_None, silently return a nullptr pointer
         mtddf = new MetUGridDataFile;
#else
         ugrid_compile_error(method_name);
#endif
         break;

      case FileType_None:
         // For FileType_None, silently return a nullptr pointer
         mtddf = (Met2dDataFile *) nullptr;
         break;

      default:
         mlog << Error << "\n" << method_name
              << "unsupported gridded data file type \"" << grdfiletype_to_string(type)
              << "\"\n\n";
         exit(1);

   } // end switch

   mlog << Debug(4)
        << "Met2dDataFileFactory::new_met_2d_data_file() -> "
        << "created new Met2dDataFile object of type \""
        << grdfiletype_to_string(type) << "\".\n";

   return(mtddf);
}

////////////////////////////////////////////////////////////////////////

Met2dDataFile * Met2dDataFileFactory::new_met_2d_data_file(const char *filename) {
   GrdFileType type;
   Met2dDataFile *mtddf = (Met2dDataFile *) nullptr;

   //
   // Determine the file type
   //
   type = grd_file_type(filename);

   //
   // Create a new data file object
   //
   mtddf = new_met_2d_data_file(type);

   //
   // Call open for non-python types
   //
   if(mtddf &&
      type != FileType_Python_Numpy &&
      type != FileType_Python_Xarray) {
      if(!(mtddf->open(filename))) {
         mlog << Error << "\nMet2dDataFileFactory::new_met_2d_data_file() -> "
              << "error opening file \"" << filename << "\"\n\n";
         exit(1);
      }
   }

   return(mtddf);
}

////////////////////////////////////////////////////////////////////////

Met2dDataFile * Met2dDataFileFactory::new_met_2d_data_file(const char *filename, GrdFileType type)

{

   Met2dDataFile *mtddf = (Met2dDataFile *) nullptr;

   //
   // Use the file type, if valid
   //
   if(type != FileType_None) {

      //
      // Create a new data file object
      //
      mtddf = new_met_2d_data_file(type);

#ifdef WITH_PYTHON
      //
      // Set MET_PYTHON_INPUT_ARG environment variable for python types
      //
      if(type == FileType_Python_Numpy ||
         type == FileType_Python_Xarray) {
         setenv(met_python_input_arg, filename, 1);
      }
#endif

      //
      // Call open for non-python types
      //
      if(mtddf &&
         type != FileType_Python_Numpy &&
         type != FileType_Python_Xarray) {
         if(!(mtddf->open(filename))) {
            mlog << Error << "\nMet2dDataFileFactory::new_met_2d_data_file() -> "
                 << "error opening file \"" << filename << "\"\n\n";
            exit(1);
         }
      }
   }

   //
   // Otherwise determine the type from the file name
   //
   else {
      mtddf = new_met_2d_data_file(filename);
   }

   return(mtddf);
}

///////////////////////////////////////////////////////////////////////////////

bool is_2d_data_file(const ConcatString &filename,
                     const ConcatString &config_str) {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) nullptr;
   GrdFileType type = FileType_None;

   // Check for a requested file type
   if(config_str.nonempty()) {
      MetConfig config;
      config.read(replace_path(config_const_filename).c_str());
      config.read_string(config_str.c_str());
      type = parse_conf_file_type(&config);
   }

   mtddf = mtddf_factory.new_met_2d_data_file(filename.c_str(), type);
   bool status = (mtddf != 0);

   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) nullptr; }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////
