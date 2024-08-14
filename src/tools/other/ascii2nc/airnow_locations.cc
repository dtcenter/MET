// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <sstream>
#include <assert.h> 
#include <algorithm>

#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"

#include "airnow_locations.h"

using namespace std;


////////////////////////////////////////////////////////////////////////

//
//  Code for class AirnowLocations
//

////////////////////////////////////////////////////////////////////////

AirnowLocations::AirnowLocations() 
{
  //
  // this is a default
  //
  monitoringSiteFileName = "Monitoring_Site_Locations_V2.dat";
}

////////////////////////////////////////////////////////////////////////

AirnowLocations::~AirnowLocations()
{
}

////////////////////////////////////////////////////////////////////////

bool AirnowLocations::initialize(const string &fileName)
{
  string method_name = "AirnowLocations::initialize()";

  monitoringSiteFileName = fileName;
  LineDataFile locFile;
  if (!locFile.open(monitoringSiteFileName.c_str())) {
    mlog << Warning << "\n" << method_name << " -> "
         << "can't open input ASCII file \"" << monitoringSiteFileName
         << "\" for reading\n\n";
    return false;
  }
  DataLine data_line;
  data_line.set_delimiter("|");
  // first line is header
  locFile >> data_line;

  // search for the columns we want
  int aqsidPtr = -1;
  int sidPtr = -1;
  int fullaqsidPtr = -1;
  int latPtr = -1;
  int lonPtr = -1;
  int elevPtr = -1;

  bool status = true;
  if (!_setPtr(data_line, "AQSID", aqsidPtr)) status = false;
  if (!_setPtr(data_line, "StationID", sidPtr)) status = false;
  if (!_setPtr(data_line, "FullAQSID", fullaqsidPtr)) status = false;
  if (!_setPtr(data_line, "Latitude", latPtr)) status = false;
  if (!_setPtr(data_line, "Longitude", lonPtr)) status = false;
  if (!_setPtr(data_line, "Elevation", elevPtr)) status = false;
  if (!status) {
    return false;
  }

  // this is inefficient, but will work
  int bad_line_count = 0;
  while (locFile >> data_line) {
    string aqsid = data_line[aqsidPtr];
    string fullaqsid = data_line[fullaqsidPtr];
    string stationid = data_line[sidPtr];
    double lat = atof(data_line[latPtr]);
    double lon = atof(data_line[lonPtr]);
    double elev = atof(data_line[elevPtr]);

    // assume the full aqsid are unique compared to the non-full simple aqsid or even the station id
    // look for the 'full'
    vector<string>::const_iterator it;
    it = find(monitoringSiteFullAqsid.begin(), monitoringSiteFullAqsid.end(), fullaqsid);
    if (it == monitoringSiteFullAqsid.end()) {
      monitoringSiteAqsid.push_back(aqsid);
      monitoringSiteFullAqsid.push_back(fullaqsid);
      monitoringSiteStationId.push_back(stationid);
      monitoringSiteLat.push_back(lat);
      monitoringSiteLon.push_back(lon);
      monitoringSiteElev.push_back(elev);
    } else {
      int index = (int)(it - monitoringSiteFullAqsid.begin());
      string aqsid2 = monitoringSiteAqsid[index];
      string stationid2 = monitoringSiteStationId[index];
      double lat2 = monitoringSiteLat[index];
      double lon2 = monitoringSiteLon[index];
      double elev2 = monitoringSiteElev[index];
      if (lat != lat2 || lon != lon2 || elev != elev2 || aqsid2 != aqsid || stationid2 != stationid) {
        mlog << Warning << "\n" << method_name << " -> "
             << "Multiple values seen for a single FullAQSID (" << fullaqsid << ")"
             << "Values used:  StationId:" << stationid << " Aqsid:" << aqsid << " Lat,Lon,Elev:"
             << lat << "," << lon << "," << elev
             << "Not    used   StationId:" << stationid2 << " Aqsid:" << aqsid2 << " Lat,Lon,Elev:"
             << lat2 << "," << lon2 << "," << elev2 << "\n\n";
      }
    }      
  }
  locFile.close();

#ifdef DEBUGGING
  for (size_t i=0; i<monitoringSiteAqsid.size(); ++i) {
    printf("%s %s %s %lf %lf %lf\n",
           monitoringSiteStationId[i].c_str(),
           monitoringSiteAqsid[i].c_str(),
           monitoringSiteFullAqsid[i].c_str(),
           monitoringSiteLat[i],
           monitoringSiteLon[i],
           monitoringSiteElev[i]);
  }
#endif
  return true;
}

////////////////////////////////////////////////////////////////////////

bool AirnowLocations::lookupLatLonElev(const string aqsid, double &lat, double &lon, double &elev) const
{
  string method_name = "AirnowLocations::lookupLatLonElev()";

  int index = -1;
  vector<string>::const_iterator it = find(monitoringSiteAqsid.begin(), monitoringSiteAqsid.end(), aqsid);
  if (it == monitoringSiteAqsid.end()) {
    it = find(monitoringSiteStationId.begin(), monitoringSiteStationId.end(), aqsid);
    if (it == monitoringSiteStationId.end()) {
      it = find(monitoringSiteFullAqsid.begin(), monitoringSiteFullAqsid.end(), aqsid);
      if (it == monitoringSiteFullAqsid.end()) {
        return false;
      } else {
        index = (int)(it - monitoringSiteFullAqsid.begin());
      }
    } else {
      index = (int)(it - monitoringSiteStationId.begin());
    }
  } else {
    index = (int)(it - monitoringSiteAqsid.begin());
  }    
  if (index < 0) {
    return false;
  }
  
  lat = monitoringSiteLat[index];
  lon = monitoringSiteLon[index];
  elev = monitoringSiteElev[index];
  return true;
}

////////////////////////////////////////////////////////////////////////

bool AirnowLocations::_setPtr(DataLine &data_line, const string &headerName, int &ptr) const
{
  for (int i=0; i<data_line.n_items(); ++i) {
    string s = data_line[i];
    if (s == headerName) {
      ptr = i;
      return true;
    }
  }
  mlog << Warning << "\nAirnowLocations::_setPtr() -> "
       << "Did not see '" << headerName << "' in the header line of file "
       << monitoringSiteFileName << "\n\n";
  return false;
}

////////////////////////////////////////////////////////////////////////
