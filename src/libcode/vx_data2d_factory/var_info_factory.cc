// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

#include <map>
#include <stdlib.h>
#include <strings.h>

#include "var_info_factory.h"
#include "grdfiletype_to_string.h"
#include "var_info_grib.h"
#include "var_info_nc_cf.h"
#include "var_info_nc_met.h"
#include "var_info_nc_wrf.h"
#include "var_info_pairs.h"
#include "var_info_ugrid.h"

#ifdef WITH_PYTHON
   #include "var_info_python.h"
#endif

#ifdef WITH_GRIB2
   #include "var_info_grib2.h"
#endif

#include "vx_cal.h"
#include "vx_log.h"
#include <memory>

using namespace std;


///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoFactory
//
///////////////////////////////////////////////////////////////////////////////

unique_ptr<VarInfo> VarInfoFactory::new_var_info(GrdFileType type)

{

   unique_ptr<VarInfo> vi;
   const char *method_name = "VarInfoFactory::new_var_info() -> ";

#ifdef WITH_PYTHON
   unique_ptr<VarInfoPython> py_vi;
#endif

   //
   // Switch on file type and instantiate the appropriate class.
   //

   switch(type) {

      case FileType_Gb1:
         vi = make_unique<VarInfoGrib>();
         break;

      case FileType_Gb2:
#ifdef WITH_GRIB2
         vi = make_unique<VarInfoGrib2>();
         break;
#else
         mlog << Error << "\n" << method_name
              << "Support for GRIB2 has not been compiled!\n"
              << "To read GRIB2 files, recompile with the --enable-grib2 option.\n\n";
         exit(1);
#endif

      case FileType_NcMet:
         vi = make_unique<VarInfoNcMet>();
         break;

      case FileType_NcWrf:
      case FileType_NcPinterp:
         vi = make_unique<VarInfoNcWrf>();
         break;

      case FileType_Python_Numpy:
      case FileType_Python_Xarray:
#ifdef WITH_PYTHON
         py_vi = make_unique<VarInfoPython>();
         py_vi->set_file_type(type);
         vi = std::move(py_vi);
         break;
#else
         python_compile_error(method_name);
#endif

      case FileType_NcCF:
         vi = make_unique<VarInfoNcCF>();
         break;

      case FileType_UGrid:
#ifdef WITH_UGRID
         vi = make_unique<VarInfoUGrid>();
         break;
#else
         ugrid_compile_error(method_name);
#endif

      case FileType_Pairs:
         vi = make_unique<VarInfoPairs>();
         break;

      case FileType_HdfEos:
         mlog << Error << "\n" << method_name
              << "Support for GrdFileType = " << grdfiletype_to_string(type)
              << " not yet implemented!\n\n";
         exit(1);

      default:
         mlog << Error << "\n" << method_name
              << "unsupported gridded data file type \"" << grdfiletype_to_string(type)
              << "\"\n\n";
         exit(1);
   } // end switch

   mlog << Debug(4) << method_name
        << "created new VarInfo object of type \""
        << grdfiletype_to_string(type) << "\".\n";

   return vi;
}

///////////////////////////////////////////////////////////////////////////////

unique_ptr<VarInfo> VarInfoFactory::new_var_info(ConcatString s) {
   GrdFileType type;

   // Convert the string to a gridded data file type
   string_to_grdfiletype(s.c_str(), type);

   return new_var_info(type);
}

///////////////////////////////////////////////////////////////////////////////

unique_ptr<VarInfo> VarInfoFactory::new_copy(const VarInfo *vi_in) {

   if(!vi_in) return nullptr;

   auto vi_copy = new_var_info(vi_in->file_type());

   *vi_copy = *vi_in;

   return vi_copy;

}

///////////////////////////////////////////////////////////////////////////////
