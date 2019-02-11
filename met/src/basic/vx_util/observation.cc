// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include "observation.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Observation
   //


////////////////////////////////////////////////////////////////////////


Observation::Observation(const string &header_type, const string &station_id,
                         const time_t valid_time,
                         const double latitude, const double longitude,
                         const double elevation,
                         const string &quality_flag,
                         const int var_code, const double pressure_level_hpa,
                         const double height_m, const double value,
                         const string &var_name) :
  _headerType(header_type),
  _stationId(station_id),
  _validTime(valid_time),
  _latitude(latitude),
  _longitude(longitude),
  _elevation(elevation),
  _qualityFlag(quality_flag),
  _varName(var_name),
  varCode(var_code),
  hdrIndex(0),
  _pressureLevel(pressure_level_hpa),
  _height(height_m),
  _value(value)
{
}


////////////////////////////////////////////////////////////////////////


Observation::~Observation()
{
  // Do nothing
}

////////////////////////////////////////////////////////////////////////

bool Observation::hasSameHeader(Observation &obs) const {
  bool same_header = true;
  if ( this != &obs ) {
    if (obs.getHeaderType() != getHeaderType()    ||
        obs.getStationId()  != getStationId()     ||
        obs.getValidTime()  != getValidTime()     ||
        !is_eq(obs.getLatitude(),  getLatitude())  ||
        !is_eq(obs.getLongitude(), getLongitude()) ||
        !is_eq(obs.getElevation(), getElevation()) ) {
      same_header = false;
    }
  }
  return same_header;
}

////////////////////////////////////////////////////////////////////////

bool Observation::hasSameHeader(Observation *obs) const {
  bool same_header = true;
  if ( this != obs ) {
    if (obs == 0) {
      same_header = false;
    }
    else if (obs->getHeaderType() != getHeaderType()    ||
        obs->getStationId()  != getStationId()     ||
        obs->getValidTime()  != getValidTime()     ||
        !is_eq(obs->getLatitude(),  getLatitude())  ||
        !is_eq(obs->getLongitude(), getLongitude()) ||
        !is_eq(obs->getElevation(), getElevation()) ) {
      same_header = false;
    }
  }
  return same_header;
}

////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////

