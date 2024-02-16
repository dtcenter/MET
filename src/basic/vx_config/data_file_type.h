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

enum GrdFileType
{

   FileType_None,           // Default

   FileType_Gb1,            // GRIB version 1
   FileType_Gb2,            // GRIB version 2
   FileType_NcMet,          // NetCDF MET format
   FileType_General_Netcdf, // NetCDF
   FileType_NcWrf,          // NetCDF output directly from WRF-ARW
   FileType_NcPinterp,      // NetCDF output of WRF-ARW pinterp tool
   FileType_NcCF,           // NetCDF Climate-Forecast Convention
   FileType_HdfEos,         // Hierarchical Data Format - Earth Observing System
   FileType_Bufr,           // Bufr or PrepBufr format
   FileType_Python_Numpy,   // Python script using numpy array and attributes dictionary
   FileType_Python_Xarray,  // Python script using xarray dataplane
   FileType_UGrid,          // Unstructured grid

};


///////////////////////////////////////////////////////////////////////////////


inline bool is_netcdf_grdfiletype(const GrdFileType _t)

{

return ( (_t == FileType_NcMet) || (_t == FileType_General_Netcdf) ||
         (_t == FileType_NcWrf) || (_t == FileType_NcPinterp) ||
         (_t == FileType_NcCF) );

}


///////////////////////////////////////////////////////////////////////////////


inline bool is_python_grdfiletype(const GrdFileType _t)

{

return ( (_t == FileType_Python_Xarray) || (_t == FileType_Python_Numpy) );

}


///////////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA_FILE_TYPE_H__  */

///////////////////////////////////////////////////////////////////////////////
