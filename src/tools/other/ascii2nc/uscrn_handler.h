// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __USCRN_HANDLER_H__
#define  __USCRN_HANDLER_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <time.h>

#include "file_handler.h"

////////////////////////////////////////////////////////////////////////

struct UscrnObsVarInfo {
   int _gribCode;
   std::string _varName;
};

////////////////////////////////////////////////////////////////////////
//
// U.S. Climate Reference Network (USCRN) Quality Controlled Datasets
//     URL: https://www.ncei.noaa.gov/access/crn/qcdatasets.html
//    Data: ftp://ftp.ncei.noaa.gov/pub/data/uscrn/products/{type}
//
// Where {type} is:
//
// 1. "monthly01" with files named "CRNM0102-{Location}.txt
//   - Contains 15 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/monthly01/headers.txt
//
// 2. "daily01" with files named "{YYYY}/CRND0103-{YYYY}-{Location}.txt
//   - Contains 28 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/daily01/headers.txt
//
// 3. "hourly02" with files named "{YYYY}/CRNH0203-{YYYY}-{Location}.txt
//   - Contains 38 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/hourly02/headers.txt
//
// 4. "subhourly01" with files named "{YYYY}/CRNS0101-{MM}-{YYYY}-{Location}.txt
//   - Contains 23 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/subhourly01/headers.txt 
//
// 5. "drought01" with files named "CRNDI0101-{Location}.csv
//   - Contains 32 NAMED columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/drought01/readme.txt
//
// 6. "soil/soilclim01" with files named "CRNSMC0101-{Location}.csv"
//   - Contains 30 NAMED columns.
//
// 7. "soil/soilanom01" with files named "CRNSSM0101-{Location}.csv"
//   - Contains 32 NAMED columns.
//
// 8. "soil01" with files named "SOIL01_{Location}.txt"
//   - Contains 15 columns defined by:
//     https://www.ncei.noaa.gov/pub/data/uscrn/products/soil01/headers.txt
//
// Where:
//   - {YYYY} is the 4-digit year.
//   - {MM} is the 2-digit month.
//   - {Location} is a 2-letter US state name and site name followed by direction
//     and distance from that location.
// 
////////////////////////////////////////////////////////////////////////

class UscrnHandler : public FileHandler {

   public:

      UscrnHandler(const std::string &program_name);
      virtual ~UscrnHandler();

      virtual bool isFileType(LineDataFile &ascii_file) const;

      static std::string getFormatString() { return "uscrn"; }

   protected:

      /////////////////////////
      // Protected constants
      /////////////////////////

      // The number of columns in the second header line in the file.  This line
      // is used to determine if this is a USCRN file since the first line has
      // an indeterminate number of tokens.

      static const int MIN_NUM_HDR_COLS;

      // The number of columns in the observation lines in the file.

      static const int NUM_OBS_COLS;

      ///////////////////////
      // Protected members
      ///////////////////////

      // Unchanging file name information
      UscrnObsVarInfo _obsVarInfo;

      // Store list of unqiue output variable names
      StringArray _varNames;

      // Unchanging header information
      std::string _networkName;
      std::string _stationId;
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

#endif   /*  __USCRN_HANDLER_H__  */

////////////////////////////////////////////////////////////////////////
