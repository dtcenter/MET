// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AIRNOWHANDLER_H__
#define  __AIRNOWHANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <time.h>
#include <vector>

#include "file_handler.h"
#include "airnow_locations.h"

////////////////////////////////////////////////////////////////////////


class AirnowHandler : public FileHandler
{

public:

  AirnowHandler(const string &program_name);
  virtual ~AirnowHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const;

  void setFormatVersion(int version);

  static string getFormatStringDailyV2()
  {
    return "airnowdaily_v2";
  }

  static string getFormatStringHourlyAqObs()
  {
    return "airnowhourlyaqobs";
  }

  static string getFormatStringHourly()
  {
    return "airnowhourly";
  }

  /////////////////////////
  // Protected constants //
  /////////////////////////

  //
  // The format versions map to integers 1 - 3
  //
  static const int AIRNOW_FORMAT_VERSION_HOURLY;
  static const int AIRNOW_FORMAT_VERSION_HOURLYAQOBS;
  static const int AIRNOW_FORMAT_VERSION_DAILYV2;
  static const int AIRNOW_FORMAT_VERSION_UNKNOWN;


  //
  // Number of columns expected for each type
  //
  static const int NUM_COLS_HOURLY;
  static const int NUM_COLS_HOURLYAQOBS;
  static const int NUM_COLS_DAILYV2;

protected:  

  /////////////////////////
  // Protected constants //
  /////////////////////////

  // Unchanging header information

  string _stationId;
  double _stationLat;
  double _stationLon;
  double _stationAlt;

  int format_version;

  StringArray header_names;

  // flag as to whether to strip leading and trailing quotes from column data
  bool doStripQuotes;

  // column pointers set for both daily v2 and hourly aqobs
  int stationIdPtr;
  int datePtr;
  int latPtr;
  int lonPtr;

  // column pointers for daily v2
  int varnamePtr;
  int unitsPtr;
  int valuePtr;
  int aqiPtr;
  int aqiCategoryPtr;

  // column pointers for hourlyaqobs
  int timePtr;
  int elevPtr;
  int ozoneAqiPtr;
  int ozoneMeasuredPtr;
  int ozonePtr;
  int ozoneUnitPtr;
  int pm10AqiPtr;
  int pm10MeasuredPtr;
  int pm10Ptr;
  int pm10UnitPtr;
  int pm25AqiPtr;
  int pm25MeasuredPtr;
  int pm25Ptr;
  int pm25UnitPtr;
  int no2AqiPtr;
  int no2MeasuredPtr;
  int no2Ptr;
  int no2UnitPtr;
  int coPtr;
  int coUnitPtr;
  int so2Ptr;
  int so2UnitPtr;
  
  string monitoringSiteFileName;

  AirnowLocations locations;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Set the header strings for the dailyv2 data format, which does not include header
  // information in the data

  void _setDailyv2HeaderInfo();
  void _setHourlyHeaderInfo();

  // Read and save the header information from the given file,  The file pointer
  // is assumed to be at the beginning of the file.  Sets the _stationId,
  // _stationLat, _stationLon and _stationAlt values.

  bool _readHeaderInfo(LineDataFile &ascii_file);
  
  bool _determineFileType(LineDataFile &ascii_file);

  void _addHourlyAqobsObs(const vector<string> &data_line, const string &header_type,
			  const string &stationId, const time_t &valid_time,
			  double lat, double lon, double elev,
			  int measuredPtr, int aqiPtr, int valuePtr,
			  int unitPtr, const string &varname);
  void _addHourlyAqobsObs(const vector<string> &data_line, const string &header_type,
			  const string &stationId, const time_t &valid_time,
			  double lat, double lon, double elev,
			  int valuePtr, int unitPtr, const string &varname);

  // Get the observation valid time from the given observation line

  time_t _getValidTime(const vector<string> &data_line) const;
  time_t _getValidTime(const DataLine &data_line) const;
  time_t _getValidTime(const string &dateStr, const string &timeStr) const;

  
  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file);
  bool _readObservationsHourlyAqobs(LineDataFile &ascii_file, int column_cnt, const string &delimiter,
				    const string &header_type);
  bool _readObservationsStandard(LineDataFile &ascii_file, int column_cnt, const string &delimiter,
				 const string &header_type);
  bool _parseObservationLineStandard(DataLine &data_line,
				     const string &filename,
				     int column_cnt,
				     const string &header_type);
  bool _parseObservationLineAqobs(const string &data_line, int column_cnt,
				  const string &header_type, int lineNumber,
				  const string &filename);

  void _initializeColumnPointers();

  string _extractColumn(const DataLine &data_line, int ptr) const;
  vector<string> _parseHourlyAqobsLine(const string &asciiLine);

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __AERONETHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


