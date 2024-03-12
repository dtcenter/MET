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

#include "ndbc_locations.h"

using namespace std;


////////////////////////////////////////////////////////////////////////

static bool _parseLine(const string &sline, const string &key, string &value);
static bool _parseLineForDouble(const string &sline, const string &key, double &value);

////////////////////////////////////////////////////////////////////////


//
//  Code for class NdbcLocations
//


////////////////////////////////////////////////////////////////////////


NdbcLocations::NdbcLocations() 
{
  //
  // this is a default
  //
  fileName = "activestations.xml";
}


////////////////////////////////////////////////////////////////////////


NdbcLocations::~NdbcLocations()
{
}


////////////////////////////////////////////////////////////////////////

bool NdbcLocations::initialize(const string &fName)
{
  string method_name = "NdbcLocations::initialize() ";

  fileName = fName;
  LineDataFile locFile;
  if (!locFile.open(fileName.c_str())) {
    mlog << Error << method_name << "->"
	 << "can't open input ASCII file \"" << fileName 
	 << "\" for reading\n\n";
    return false;
  }
  DataLine data_line;
  string sidKey = "station id=";
  string latKey = "lat=";
  string lonKey = "lon=";
  string elevKey = "elev=";
  
  while (locFile >> data_line) {
    string sline = data_line.get_line();
    string stationId;
    double lat, lon, elev;
    if (!_parseLine(sline, sidKey, stationId)) {
      // assume not a line we want
      continue;
    }
    if (!_parseLineForDouble(sline, latKey, lat)) {
      mlog << Warning << method_name << "-> "
	   << "parsing out lat from line '" << sline << "'\n"
	   << "in file \"" << fileName << "\n\n";
      continue;
    }
    if (!_parseLineForDouble(sline, lonKey, lon)) {
      mlog << Warning << method_name << "-> "
	   << "parsing out lon from line '" << sline << "'\n"
	   << "in file \"" << fileName << "\n\n";
      continue;
    }      
    if (!_parseLineForDouble(sline, elevKey, elev)) {
      // elev can be missing
      elev = bad_data_double;
    }

    // store lower case only, for later comparisons
    std::transform(stationId.begin(), stationId.end(), stationId.begin(), ::tolower);
    StationId.push_back(stationId);
    Lat.push_back(lat);
    Lon.push_back(lon);
    Elev.push_back(elev);
  }
  locFile.close();
  return true;
}

////////////////////////////////////////////////////////////////////////

bool NdbcLocations::lookupLatLonElev(const string aqsid, double &lat, double &lon, double &elev) const
{
  string method_name = "NdbcLocations::lookupLatLonElev()";

  string locId = aqsid;
  std::transform(locId.begin(), locId.end(), locId.begin(), ::tolower);

  vector<string>::const_iterator it;
  it = find(StationId.begin(), StationId.end(), locId);
  if (it == StationId.end()) {
    return false;
  }
  int index = (int)(it - StationId.begin());
  lat = Lat[index];
  lon = Lon[index];
  elev = Elev[index];
  return true;
}

////////////////////////////////////////////////////////////////////////

void NdbcLocations::print(void) const
{
  for (size_t i=0; i<StationId.size(); ++i) {
    printf("%s %lf %lf %lf\n", StationId[i].c_str(), Lat[i], Lon[i], Elev[i]);
  }
}

////////////////////////////////////////////////////////////////////////

bool _parseLine(const string &sline, const string &key, string &value)
{
  // look for key
  std::size_t found = sline.find(key);
  if (found==std::string::npos) {
    return false;
  }
  
  // expect " delimited string right after this
  size_t len = key.size();
  size_t p0 = found + len;
  size_t p1 = sline.find_first_of("\"",  p0);
  if (p1 != p0) {
    return false;
  }
  size_t p2 = sline.find_first_of("\"", p0+1);
  value = sline.substr(p0+1, p2-p0-1);
  return true;
}

////////////////////////////////////////////////////////////////////////

bool _parseLineForDouble(const string &sline, const string &key, double &value)
{
  string svalue;
  double lvalue;
  if (_parseLine(sline, key, svalue)) {
    // the string should be a double
    if (sscanf(svalue.c_str(), "%lf", &lvalue) == 1) {
      value = lvalue;
      return true;
    }
  }
  return false;
}
