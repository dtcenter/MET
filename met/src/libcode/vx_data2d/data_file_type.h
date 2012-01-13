

///////////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_FILE_TYPE_H__
#define  __DATA_FILE_TYPE_H__


///////////////////////////////////////////////////////////////////////////////


   //
   // Enumeration of supported gridded file types
   //

enum GrdFileType
{
   FileType_None           = 0, // Default
   FileType_Gb1            = 1, // GRIB version 1
   FileType_Gb2            = 2, // GRIB version 2
   FileType_NcMet          = 3, // NetCDF MET format
   FileType_General_Netcdf = 4, // NetCDF
   FileType_NcPinterp      = 5, // NetCDF output of WRF-ARW pinterp tool
   FileType_NcCF           = 6, // NetCDF Climate-Forecast Convention
   FileType_HdfEos         = 7, // Hierarchical Data Format - Earth Observing System
   FileType_Bufr           = 8, // Bufr or PrepBufr format
};


///////////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA_FILE_TYPE_H__  */

///////////////////////////////////////////////////////////////////////////////
