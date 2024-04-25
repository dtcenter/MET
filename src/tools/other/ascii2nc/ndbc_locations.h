// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NDBCLOCATIONS_H__
#define  __NDBCLOCATIONS_H__


////////////////////////////////////////////////////////////////////////
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////


class NdbcLocations
{

public:

  NdbcLocations();
  virtual ~NdbcLocations();
  bool initialize(const std::string &locationsFile);
  bool lookupLatLonElev(const std::string aqsid, double &lat, double &lon, double &elev) const;
  void print(void) const;

private:
  
  bool _setPtr(DataLine &data_line, const std::string &headerName, int &ptr) const;

  std::string fileName;

  std::vector<std::string> StationId;
  std::vector<double> Lat;
  std::vector<double> Lon;
  std::vector<double> Elev;

};


////////////////////////////////////////////////////////////////////////


#endif 


////////////////////////////////////////////////////////////////////////


