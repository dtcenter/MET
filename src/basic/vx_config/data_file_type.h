// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef  __DATA_FILE_TYPE_H__
#define  __DATA_FILE_TYPE_H__

///////////////////////////////////////////////////////////////////////////////

//
// Enumeration of supported gridded file types
//

enum GrdFileType {

   GrdFileType_None,           // Default

   GrdFileType_Gb1,            // GRIB version 1
   GrdFileType_Gb2,            // GRIB version 2
   GrdFileType_NcMet,          // NetCDF MET format
   GrdFileType_General_Netcdf, // NetCDF
   GrdFileType_NcWrf,          // NetCDF output directly from WRF-ARW
   GrdFileType_NcPinterp,      // NetCDF output of WRF-ARW pinterp tool
   GrdFileType_NcCF,           // NetCDF Climate-Forecast Convention
   GrdFileType_HdfEos,         // Hierarchical Data Format - Earth Observing System
   GrdFileType_Bufr,           // Bufr or PrepBufr format
   GrdFileType_Python_Numpy,   // Python script using numpy array and attributes dictionary
   GrdFileType_Python_Xarray,  // Python script using xarray dataplane
   GrdFileType_UGrid,          // Unstructured grid

};

///////////////////////////////////////////////////////////////////////////////

inline bool is_netcdf_grdfiletype(const GrdFileType _t) {
   return(_t == GrdFileType_NcMet          ||
          _t == GrdFileType_General_Netcdf ||
          _t == GrdFileType_NcWrf          ||
          _t == GrdFileType_NcPinterp      ||
          _t == GrdFileType_NcCF);
}


///////////////////////////////////////////////////////////////////////////////

inline bool is_python_grdfiletype(const GrdFileType _t) {
   return(_t == GrdFileType_Python_Xarray ||
          _t == GrdFileType_Python_Numpy);
}

///////////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA_FILE_TYPE_H__  */

///////////////////////////////////////////////////////////////////////////////
