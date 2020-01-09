// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <time.h>

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

  // The header type for these observations

  static const string HEADER_TYPE;

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
  // Protected members //
  ///////////////////////

  // Unchanging header information

  string _stationId;
  double _stationLat;
  double _stationLon;
  double _stationAlt;
  
  
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
  
  // Read and save the header information from the given file,  The file pointer
  // is assumed to be at the beginning of the file.  Sets the _stationId,
  // _stationLat, _stationLon and _stationAlt values.

  bool _readHeaderInfo(LineDataFile &ascii_file);
  
  // Get the observation valid time from the given observation line

  time_t _getValidTime(const DataLine &data_line) const;
  
  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file);
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SURFRADHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


