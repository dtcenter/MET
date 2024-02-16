// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
  bool initialize(const std::string &locationsFile);
  bool lookupLatLonElev(const std::string aqsid, double &lat, double &lon, double &elev) const;

private:
  
  bool _setPtr(DataLine &data_line, const std::string &headerName, int &ptr) const;

  std::string monitoringSiteFileName;

  // all the AQSID's from the monitoring site file, set only for Hourly format.
  // and all the associated location information from the monitoring site file, in the same order
  // 3 different AQSID's are found in the lookup file:  stationId, Aqsid, FullAqsid
  // 
  std::vector<std::string> monitoringSiteStationId;
  std::vector<std::string> monitoringSiteAqsid;
  std::vector<std::string> monitoringSiteFullAqsid;
  std::vector<double> monitoringSiteLat;
  std::vector<double> monitoringSiteLon;
  std::vector<double> monitoringSiteElev;

};


////////////////////////////////////////////////////////////////////////


#endif 


////////////////////////////////////////////////////////////////////////


