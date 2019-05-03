// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
   FileType_NcPinterp,      // NetCDF output of WRF-ARW pinterp tool
   FileType_NcCF,           // NetCDF Climate-Forecast Convention
   FileType_HdfEos,         // Hierarchical Data Format - Earth Observing System
   FileType_Bufr            // Bufr or PrepBufr format
};


///////////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA_FILE_TYPE_H__  */

///////////////////////////////////////////////////////////////////////////////
