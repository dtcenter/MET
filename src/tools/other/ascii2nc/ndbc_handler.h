// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NDBC_HANDLER_H__
#define  __NDBC_HANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <time.h>
#include <vector>

#include "file_handler.h"
#include "ndbc_locations.h"


////////////////////////////////////////////////////////////////////////


class NdbcHandler : public FileHandler
{
private:
  //
  // private simple class, storage for column header names and pointers to a column
  // (in case the order ever changes, this handles that possibility)
  //
  class Column
  {
  public:
    std::string name;  // name of the header as found in the file
    int ptr;      // column index for that data (0,1,..)
    inline Column(const std::string &n) : name(n), ptr(-1) {}
    inline ~Column() {}
    inline void clear(void) {ptr = -1;}
    inline bool nameEquals(const std::string &s) const {return name == s;}
    inline void setPtr(int ipt) {ptr = ipt;}
    inline bool notSet(void) const {return ptr == -1;}
  };
  

public:

  NdbcHandler(const std::string &program_name);
  virtual ~NdbcHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const;

  void setFormatVersion(int version);

  static std::string getFormatStringStandard()
  {
    return "ndbc_standard";
  }

  /////////////////////////
  // Protected constants //
  /////////////////////////

  //
  // The format versions map to integers (only 1 so far)
  //
  static const int NDBC_FORMAT_VERSION_STANDARD;
  static const int NDBC_FORMAT_VERSION_UNKNOWN;

  //
  // Number of columns expected
  //
  static const int NUM_COLS_STANDARD;

  //
  // Number of columns not time related
  //
  static const int NUM_DATA_COLS_STANDARD;

protected:  

  /////////////////////////
  // Protected constants //
  /////////////////////////

  //
  // Unchanging header information for a file (each file is one station)
  //
  std::string stationId;
  double stationLat;
  double stationLon;
  double stationAlt;

  int format_version;

  // column pointers set in real time for time information
  int column_pointer_year;
  int column_pointer_month;
  int column_pointer_day;
  int column_pointer_hour;
  int column_pointer_minute;

  // storage for non time column information (name/pointer)
  std::vector<Column> column;

  // the lookup file for location information
  std::string locationsFileName;

  // the lookup object
  NdbcLocations locations;

  // a count of how many stations were not in the lookup file
  int numMissingStations;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  //
  // parse the file name to identify the station, then lookup and set the
  // location
  //
  bool _setStationInfo(const std::string &filename);

  // Read and save the header information from the given file,  The file pointer
  // is assumed to be at the beginning of the file.  Sets all the pointers based
  // on what is in the header line
  bool _readHeaderInfo(LineDataFile &ascii_file);
  
  bool _determineFileType(LineDataFile &ascii_file);


  //
  // Get the observation valid time from the given observation line
  //
  time_t _getValidTime(const DataLine &data_line) const;

  
  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file);

  //
  // parse one line from the ascii file and add to observations

  bool _parseObservationLineStandard(DataLine &data_line,
                                     const std::string &filename);

  std::string _extractColumn(const DataLine &data_line, int ptr) const;
  double _extractDouble(const DataLine &data_line, int ptr) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __NDBC_HANDLER_H */


////////////////////////////////////////////////////////////////////////


