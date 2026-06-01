// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_factory_utils.h"
#include "is_grib_file.h"
#include "is_netcdf_file.h"
#include "is_bufr_file.h"
#include "is_zarr_store.h"

#include "vx_data2d.h"
#include "vx_util.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

//
// List of file name extensions corresponding to these file types
//
static const std::vector<std::string> gb_file_ext   = { ".grib", ".grb", ".gb"};
static const std::vector<std::string> gb2_file_ext  = { ".grib2", ".grb2", ".gb2" };
static const std::vector<std::string> bf_file_ext   = { ".bufr", ".bfr", ".prepbufr", ".pb" };
static const std::vector<std::string> zarr_file_ext = { ".zarr" };

////////////////////////////////////////////////////////////////////////

static GrdFileType file_type_by_suffix(const char * filename);

////////////////////////////////////////////////////////////////////////

GrdFileType grd_file_type(const char * filename) {
   GrdFileType suffix_type = FileType_None;
   GrdFileType data_type   = FileType_None;

   // first check for python strings
   if(strcasecmp(filename, conf_val_python_numpy) == 0) {
      return FileType_Python_Numpy;
   }
   else if(strcasecmp(filename, conf_val_python_xarray) == 0) {
      return FileType_Python_Xarray;
   }

   // next, check to see if this file exists
   if(!file_exists(filename)) {
      mlog << Error << "\ngrd_file_type() -> "
           << "file does not exist \"" << filename << "\"\n\n";
      exit(1);
   }

   // try to get the file type from the filename suffix
   suffix_type = file_type_by_suffix(filename);

   // look inside the directory or file
        if(is_zarr_store    (filename)) data_type = FileType_Zarr;
   else if(is_grib1_file    (filename)) data_type = FileType_Gb1;
   else if(is_grib2_file    (filename)) data_type = FileType_Gb2;
   else if(is_ncpinterp_file(filename)) data_type = FileType_NcPinterp;
   else if(is_ncwrf_file    (filename)) data_type = FileType_NcWrf;
   else if(is_nccf_file     (filename)) data_type = FileType_NcCF;
   else if(is_ncmet_file    (filename)) data_type = FileType_NcMet;
   else if(is_bufr_file     (filename)) data_type = FileType_Bufr;
   else if(is_ugrid_file    (filename)) data_type = FileType_UGrid;
   else                                 data_type = FileType_None;

   // print warning for inconsistent types
   if(suffix_type != FileType_None && suffix_type != data_type) {

      mlog << Warning << "\ngrd_file_type() -> "
           << "the file type indicated by the suffix \""
           << grdfiletype_to_string(suffix_type)
           << "\" does not match the file type indicated by the data \""
           << grdfiletype_to_string(data_type) << "\".\n\n";
   }

   // return the suffix type, if defined
   if(suffix_type != FileType_None) return suffix_type;
   else                             return data_type;
}

////////////////////////////////////////////////////////////////////////

GrdFileType file_type_by_suffix(const char * filename) {
   const ConcatString suffix = filename_suffix(filename);

   if(suffix.empty()) return FileType_None;

   // grib ?
   for(const auto &x : gb_file_ext) {
      if(suffix == x) return FileType_Gb1;
   }

   // grib2 ?
   for(const auto &x : gb2_file_ext) {
      if(suffix == x) return FileType_Gb2;
   }

   // prepbufr or bufr ?
   for(const auto &x : bf_file_ext) {
      if(suffix == x) return FileType_Bufr;
   }

   // zarr ?
   for(const auto &x : zarr_file_ext) {
      if(suffix == x) return FileType_Zarr;
   }

   return FileType_None;
}

////////////////////////////////////////////////////////////////////////

void update_mtddf_grid(Met2dDataFile *mtddf, VarInfo *vinfo) {

   // Read the requested data to define the grid for Python and
   // range/azimuth inputs and when set_attr_grid is specified
   if(is_python_grdfiletype(mtddf->file_type()) ||
      mtddf->grid().info().ra                   ||
      vinfo->grid_attr().is_set() )  {

      DataPlane dp;

      if(!mtddf->data_plane(*vinfo, dp)) {
         mlog << Error << "\nupdate_mtddf_grid() -> "
              << "Trouble reading \"" << vinfo->magic_str()
              << " \"data from input file \""
              << mtddf->filename() << "\"\n\n";
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////
