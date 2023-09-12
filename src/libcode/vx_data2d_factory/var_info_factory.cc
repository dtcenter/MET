// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <map>
#include <stdlib.h>
#include <strings.h>

#include "var_info_factory.h"
#include "grdfiletype_to_string.h"
#include "var_info_grib.h"
#include "var_info_nccf.h"
#include "var_info_nc_met.h"
#include "var_info_nc_pinterp.h"
#include "var_info_ugrid.h"

#ifdef WITH_PYTHON
   #include "var_info_python.h"
#endif

#ifdef WITH_GRIB2
   #include "var_info_grib2.h"
#endif

#include "vx_cal.h"
#include "vx_log.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoFactory
//
///////////////////////////////////////////////////////////////////////////////

VarInfo * VarInfoFactory::new_var_info(GrdFileType type)

{

   VarInfo *vi = (VarInfo *) 0;

#ifdef WITH_PYTHON
   VarInfoPython * p = 0;
#endif

   //
   // Switch on file type and instantiate the appropriate class.
   // The VarInfo object is allocated and needs to be deleted by caller.
   //

   switch(type) {

      case FileType_Gb1:
         vi = new VarInfoGrib;
         break;

      case FileType_Gb2:
#ifdef WITH_GRIB2
         vi = new VarInfoGrib2;
         break;
#else
         mlog << Error << "\nVarInfoFactory::new_var_info() -> "
              << "Support for GRIB2 has not been compiled!\n"
              << "To read GRIB2 files, recompile with the --enable-grib2 option.\n\n";
         exit(1);
#endif

      case FileType_NcMet:
         vi = new VarInfoNcMet;
         break;

      case FileType_NcPinterp:
         vi = new VarInfoNcPinterp;
         break;

      case FileType_Python_Numpy:
      case FileType_Python_Xarray:
#ifdef WITH_PYTHON
         p = new VarInfoPython;
         p->set_file_type(type);
         vi = p;
         p = 0;
         break;
#else
         python_compile_error("VarInfoFactory::new_var_info() -> ");
#endif

      case FileType_NcCF:
         vi = new VarInfoNcCF;
         break;

      case FileType_UGrid:
         vi = new VarInfoUGrid;
         break;

      case FileType_HdfEos:
         mlog << Error << "\nVarInfoFactory::new_var_info() -> "
              << "Support for GrdFileType = " << grdfiletype_to_string(type)
              << " not yet implemented!\n\n";
         exit(1);

      default:
         mlog << Error << "\nVarInfoFactory::new_var_info() -> "
              << "unsupported gridded data file type \"" << grdfiletype_to_string(type)
              << "\"\n\n";
         exit(1);
   } // end switch

   mlog << Debug(4)
        << "VarInfoFactory::new_var_info() -> "
        << "created new VarInfo object of type \""
        << grdfiletype_to_string(type) << "\".\n";

   return(vi);
}

///////////////////////////////////////////////////////////////////////////////

VarInfo * VarInfoFactory::new_var_info(ConcatString s) {
   GrdFileType type;

   // Convert the string to a gridded data file type
   string_to_grdfiletype(s.c_str(), type);

   return(new_var_info(type));
}

///////////////////////////////////////////////////////////////////////////////

VarInfo * VarInfoFactory::new_copy(const VarInfo *vi_in) {

   if(!vi_in) return ( nullptr );

   VarInfo *vi_copy = new_var_info(vi_in->file_type());

   *vi_copy = *vi_in;

   return(vi_copy);

}

///////////////////////////////////////////////////////////////////////////////
