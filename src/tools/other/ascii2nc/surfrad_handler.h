

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __SURFRADHANDLER_H__
#define  __SURFRADHANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "file_handler.h"


////////////////////////////////////////////////////////////////////////


class SurfradHandler : public FileHandler
{

public:

  SurfradHandler(const string &program_name);
  virtual ~SurfradHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const;
  
  static string getFormatString()
  {
    return "surfrad";
  }

protected:  

  /////////////////////////
  // Protected constants //
  /////////////////////////

  // The number of columns in the second header line in the file.  This line
  // is used to determine if this is a SURFRAD file since the first line has
  // an indeterminate number of tokens.

  static const int NUM_HDR_COLS;

  // The number of columns in the observation lines in the file.

  static const int NUM_OBS_COLS;

  // Grib codes for the different fields

  static const int DW_PSP_GRIB_CODE;
  static const int UW_PSP_GRIB_CODE;
  static const int DIRECT_GRIB_CODE;
  static const int DIFFUSE_GRIB_CODE;
  static const int DW_PIR_GRIB_CODE;
  static const int DW_CASETEMP_GRIB_CODE;
  static const int DW_DOMETEMP_GRIB_CODE;
  static const int UW_PIR_GRIB_CODE;
  static const int UW_CASETEMP_GRIB_CODE;
  static const int UW_DOMETEMP_GRIB_CODE;
  static const int UVB_GRIB_CODE;
  static const int PAR_GRIB_CODE;
  static const int NETSOLAR_GRIB_CODE;
  static const int NETIR_GRIB_CODE;
  static const int TOTALNET_GRIB_CODE;
  static const int TEMP_GRIB_CODE;
  static const int RH_GRIB_CODE;
  static const int WINDSPD_GRIB_CODE;
  static const int WINDDIR_GRIB_CODE;
  static const int PRESSURE_GRIB_CODE;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Convert millibars to Pascals

  static double _pressMbToPa(const double mb_value)
  {
    return mb_value * 100.0;
  }
  
  // Convert K to C

  static double _tempKtoC(const double k_value)
  {
    return k_value - 273.15;
  }
  
  // Get the station id from the given header line.  This should be the first
  // line in the file which contains the station name.

  int _getStationId(const DataLine &data_line);

  // Get the station location (lat, lon, alt) from the given header line.
  // This should be the second line ion the file.

  int _getLocation(const DataLine &data_line);
  
  // Get the observation valid time from the given observation line

  string _getValidTime(const DataLine &data_line) const;
  
  // Get the number of header records that will be needed for the netCDF
  // file.  This must be done before calling _processObs().

  bool _prepareHeaders(LineDataFile &ascii_file);

  // Process the observations in the file.  Assumes that _nhdrs contains the
  // number of header records for this file.

  bool _processObs(LineDataFile &ascii_file,
		   const string &nc_filename);
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  // Unchanging header information

  string _stationId;
  double _stationLat;
  double _stationLon;
  double _stationAlt;
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SURFRADHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


