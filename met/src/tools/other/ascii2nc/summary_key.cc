// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include "summary_key.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SummaryKey
   //


////////////////////////////////////////////////////////////////////////

SummaryKey::SummaryKey(const string &header_type,
		       const string &station_id,
		       const double lat, const double lon, const double elev,
		       const int grib_code,
		       const double height, const double pressure_level) :
  _headerType(header_type),
  _stationId(station_id),
  _latitude(lat),
  _longitude(lon),
  _elevation(elev),
  _gribCode(grib_code),
  _height(height),
  _pressureLevel(pressure_level)
{
}

////////////////////////////////////////////////////////////////////////

SummaryKey::~SummaryKey()
{
  // Do nothing
}
