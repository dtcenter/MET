// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __OBSERVATION_H__
#define  __OBSERVATION_H__


////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <string>
#include <time.h>


////////////////////////////////////////////////////////////////////////


class Observation
{

public:

  Observation(const string &header_type, const string &station_id,
	      const time_t valid_time,
	      const double latitude, const double longitude,
	      const double elevation,
	      const string &quality_flag,
	      const int grib_code, const double pressure_level_hpa,
	      const double height_m, const double value);
  
  Observation(const string &header_type, const string &station_id,
	      const time_t start_time, const time_t end_time,
	      const double latitude, const double longitude,
	      const double elevation,
	      const string &quality_flag,
	      const int grib_code, const double pressure_level_hpa,
	      const double height_m, const double value);
  
  virtual ~Observation();


  ////////////////////
  // Access methods //
  ////////////////////

  string getHeaderType() const
  {
    return _headerType;
  }
  
  string getStationId() const
  {
    return _stationId;
  }
  
  time_t getValidTime() const
  {
    return _validTime;
  }
  
  string getValidTimeString() const
  {
    return _getTimeString(_validTime);
  }
  
  double getLatitude() const
  {
    return _latitude;
  }
  
  double getLongitude() const
  {
    return _longitude;
  }
  
  double getElevation() const
  {
    return _elevation;
  }
  
  string getQualityFlag() const
  {
    return _qualityFlag;
  }
  
  int getGribCode() const
  {
    return _gribCode;
  }
  
  double getPressureLevel() const
  {
    return _pressureLevel;
  }
  
  double getHeight() const
  {
    return _height;
  }
  
  double getValue() const
  {
    return _value;
  }
  
  ///////////////
  // Operators //
  ///////////////

  bool operator< (const Observation &other) const
  {
    // First sort on valid time

    if (_validTime != other._validTime)
      return _validTime < other._validTime;
    
    // Second sort on header type

    if (_headerType != other._headerType)
      return _headerType < other._headerType;
    
    // Third sort on station id

    if (_stationId != other._stationId)
      return _stationId < other._stationId;
    
    // Final sort on grib code

    return _gribCode < other._gribCode;
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _headerType;
  string _stationId;
  time_t _validTime;
  double _latitude;
  double _longitude;
  double _elevation;
  
  string _qualityFlag;
  int _gribCode;
  double _pressureLevel;
  double _height;
  double _value;
  
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  static string _getTimeString(const time_t &unix_time)
  {
    struct tm *time_struct = gmtime(&unix_time);
  
    char time_string[80];
  
    sprintf(time_string, "%04d%02d%02d_%02d%02d%02d",
	    time_struct->tm_year + 1900, time_struct->tm_mon + 1,
	    time_struct->tm_mday,
	    time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
  
    return string(time_string);
  }

  static string _getTimeString(const time_t &start_time,
			       const time_t &end_time)
  {
    string start_time_string = _getTimeString(start_time);
    string end_time_string = _getTimeString(end_time);
  
    char time_string[80];
  
    sprintf(time_string, "%s-%s",
	    start_time_string.c_str(), end_time_string.c_str());
  
    return string(time_string);
  }

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __OBSERVATION_H__  */


////////////////////////////////////////////////////////////////////////


