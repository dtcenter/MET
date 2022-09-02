// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AIRNOWLOCATIONS_H__
#define  __AIRNOWLOCATIONS_H__


////////////////////////////////////////////////////////////////////////
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////


class AirnowLocations
{

public:

  AirnowLocations();
  virtual ~AirnowLocations();
  bool initialize(const string &locationsFile);
  bool lookupLatLonElev(const string aqsid, double &lat, double &lon, double &elev) const;

private:
  
  bool _setPtr(DataLine &data_line, const string &headerName, int &ptr) const;

  string monitoringSiteFileName;

  // all the AQSID's from the monitoring site file, set only for Hourly format
  // associated location information from the monitoring site file, in the same order
  vector<string> monitoringSiteStationId;
  vector<string> monitoringSiteAqsid;
  vector<string> monitoringSiteFullAqsid;
  vector<double> monitoringSiteLat;
  vector<double> monitoringSiteLon;
  vector<double> monitoringSiteElev;

};


////////////////////////////////////////////////////////////////////////


#endif 


////////////////////////////////////////////////////////////////////////


