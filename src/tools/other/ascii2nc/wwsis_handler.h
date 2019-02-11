// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __WWSISHANDLER_H__
#define  __WWSISHANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <time.h>

#include "file_handler.h"


////////////////////////////////////////////////////////////////////////


class WwsisHandler : public FileHandler
{

public:

  WwsisHandler(const string &program_name);
  virtual ~WwsisHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const;
  
  static string getFormatString()
  {
    return "wwsis";
  }

protected:  

  /////////////////////////
  // Protected constants //
  /////////////////////////

  // The header type for these observations

  static const string HEADER_TYPE;

  // Grib code for the field

  static const int GRIB_CODE;


  ///////////////////////
  // Protected members //
  ///////////////////////

  // Unchanging header information

  double _stationLat;
  double _stationLon;
  int _timeZoneOffsetSecs;
  int _stepSecs;
  double _watts;
  double _derate;
  string _tracking;
  double _tilt;
  double _azimuth;
  
  
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
  
  // Get the initial valid time from the file name

  int _getYearFromFilename(const string &filename) const;
  time_t _initValidTime(const string &filename) const;
  
  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file);
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __WWSISHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


