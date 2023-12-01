// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __ISMN_HANDLER_H__
#define  __ISMN_HANDLER_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <time.h>

#include "file_handler.h"

////////////////////////////////////////////////////////////////////////

struct obsVarInfo {
   int _gribCode;
   string _varName;
};

////////////////////////////////////////////////////////////////////////
//
// International Soil Moisture Network Data
//    https://ismn.bafg.de/en/data/header-value-files
//
// Dataset Filename Convention:
//    CSE_Network_Station_Variablename_depthfrom_depthto_sensorname_startdate_enddate.ext
//
//    - CSE: Continental Scale Experiment (CSE) acronym, if not applicable use Networkname
//    - Network: Network abbreviation (e.g., OZNET)
//    - Station: Station name (e.g., Widgiewa)
//    - Variablename: Name of the variable in the file (e.g., Soil-Moisture)
//      - p: preciptation in mm/h
//        - Store as GRIB Code 59 (PRATE in kg/m^2/s)
//        - Convert from mm/h to kg/m^2/s
//      - sd: snow depth in mm
//        - Store as GRIB Code 66 (SNOD in m)
//        - Convert from mm to m
//      - sm: soil moisture in kg^3/kg^3
//        - Store as GRIB Code 144 (SOILW as a fraction)
//      - su: soil suction in kPa
//        - Store as GRIB Code -1 (undefined, SMS in kPa)
//      - sweq: snow water equivalent in mm
//        - Store as GRIB Code 65 (WEASD kg/m^2)
//      - ta: air temperature in C
//        - Store as GRIB Code 11 (TMP in K)
//        - Convert from C to K
//      - ts: soil temperature in C
//        - Store as GRIB Code 85 (TSOIL in K)
//        - Convert from C to K
//      - tsf: surface temperature in C
//        - Store as GRIB Code 148 (AVSFT in K)
//        - Convert from C to K
//    - depthfrom: Depth in the ground in which the variable was observed (upper boundary)
//    - depthto: Depth in the ground in which the variable was observed (lower boundary)
//    - sensorname: Name of the sensor used
//    - startdate: Date of the first dataset in the file (format YYYYMMDD)
//    - enddate: Date of the last dataset in the file (format YYYYMMDD)
//    - ext: Extension .stm (Soil Temperature and Soil Moisture Data Set see CEOP standard)
//
//    Example: OZNET_OZNET_Widgiewa_Soil-Temperature_0.150000_0.150000_20010103_20090812.stm
//
// Dataset Conents:
//    Example:
//       REMEDHUS REMEDHUS Zamarron 41.24100 -5.54300 855.00 0.05 0.05
//       2005/03/16 00:00 10.30 U M
//       2005/03/16 01:00 9.80 U M
//       ...
//
// Header Line:
//    CSE, Network, Station,
//    Latitude (degrees north), Longitude (degrees east),
//    Elevation (msl), Depth from, Depth to,
//    Sensor name (Note: contains embedded whitespace)
//
// Record Lines:
//    YYYY/MM/DD HH:MM, Variable value,
//    ISMN Quality Flag, Data Provider Quality Flag
//
////////////////////////////////////////////////////////////////////////

class IsmnHandler : public FileHandler {

   public:

      IsmnHandler(const string &program_name);
      virtual ~IsmnHandler();

      virtual bool isFileType(LineDataFile &ascii_file) const;

      static string getFormatString() { return "ismn"; }

   protected:

      /////////////////////////
      // Protected constants
      /////////////////////////

      // The number of columns in the second header line in the file.  This line
      // is used to determine if this is a ISMN file since the first line has
      // an indeterminate number of tokens.

      static const int MIN_NUM_HDR_COLS;

      // The number of columns in the observation lines in the file.

      static const int NUM_OBS_COLS;

      ///////////////////////
      // Protected members
      ///////////////////////

      // Unchanging file name information
      obsVarInfo _obsVarInfo;

      // Unchanging header information
      string _networkName;
      string _stationId;
      double _stationLat;
      double _stationLon;
      double _stationElv;
      double _depth;

      ///////////////////////
      // Protected methods
      ///////////////////////

      // Read and save the header information from the given file
      bool _readHeaderInfo(LineDataFile &ascii_file);

      // Get the valid time from the observation line
      time_t _getValidTime(const DataLine &data_line) const;

      // Read the observations and add them to the
      // _observations vector
      virtual bool _readObservations(LineDataFile &ascii_file);

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __ISMN_HANDLER_H__  */

////////////////////////////////////////////////////////////////////////
