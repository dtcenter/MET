// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include "observation.h"


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
			 const int grib_code, const double pressure_level_hpa,
			 const double height_m, const double value) :
  _headerType(header_type),
  _stationId(station_id),
  _validTime(valid_time),
  _latitude(latitude),
  _longitude(longitude),
  _elevation(elevation),
  _qualityFlag(quality_flag),
  _gribCode(grib_code),
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
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////

