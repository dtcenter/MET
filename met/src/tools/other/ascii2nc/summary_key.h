// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARYKEY_H__
#define  __SUMMARYKEY_H__


////////////////////////////////////////////////////////////////////////

#include <string>


////////////////////////////////////////////////////////////////////////


class SummaryKey
{

public:

  SummaryKey(const string &header_type,
	     const string &station_id,
	     const double lat, const double lon, const double elev,
	     const int grib_code,
	     const double height_m, const double pressure_level);
  
  virtual ~SummaryKey();


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
  
  int getGribCode() const
  {
    return _gribCode;
  }
  
  double getHeight() const
  {
    return _height;
  }

  double getPressureLevel() const
  {
    return _pressureLevel;
  }

  ///////////////
  // Operators //
  ///////////////

  bool operator< (const SummaryKey &other) const
  {
    // We need to use all of the fields for sorting so handle each one
    // in turn.

    // Header type

    if (_headerType != other._headerType)
      return _headerType < other._headerType;
    
    // Station id

    if (_stationId != other._stationId)
      return _stationId < other._stationId;
    
    // Location.
    // This should be taken care of by the station id, but is in here just
    // in case we somehow have 2 stations with the same ID but different
    // locations.

    if (_latitude != other._latitude)
      return _latitude < other._latitude;
    
    if (_longitude != other._longitude)
      return _longitude < other._longitude;
    
    if (_elevation != other._elevation)
      return _elevation < other._elevation;
    
    // Height

    if (_height != other._height)
      return _height < other._height;

    // Pressure Level

    if (_pressureLevel != other._pressureLevel)
      return _pressureLevel < other._pressureLevel;
    
    // Grib code

    return _gribCode < other._gribCode;
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _headerType;
  string _stationId;
  double _latitude;
  double _longitude;
  double _elevation;
  int _gribCode;
  double _height;
  double _pressureLevel;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARYKEY_H__  */


////////////////////////////////////////////////////////////////////////


