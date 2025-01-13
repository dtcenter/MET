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
#include <map>
#include <time.h>

#include "file_handler.h"

////////////////////////////////////////////////////////////////////////
//
// U.S. Climate Reference Network (USCRN) Quality Controlled Datasets
//     URL: https://www.ncei.noaa.gov/access/crn/qcdatasets.html
//    Data: ftp://ftp.ncei.noaa.gov/pub/data/uscrn/products/{type}
//
////////////////////////////////////////////////////////////////////////

// List of USCRN {type} variants
enum class USCRNFormat {
   None,
   Monthly,
   Daily,
   Hourly,
   SubHourly,
   SoilAnom,
   Heat,
   Drought
};

// Column information
struct USCRNColInfo {
   int _offset;
   std::string _name;
   std::string _units;
   std::string _desc;
   int _qcOffset = -1;
};

// Metadata for USCRN variants
struct USCRNFormatInfo {
   std::string _formatName;
   std::string _filePrefix;
   std::string _fileSuffix;
   int _nCols;
   int _sidOffset;
   int _ymdOffset;
   int _hmOffset;
   int _lonOffset;
   int _latOffset;
   std::vector<USCRNColInfo> _obsInfo;
};

////////////////////////////////////////////////////////////////////////

class UscrnHandler : public FileHandler {

   public:

      UscrnHandler(const std::string &program_name);
      virtual ~UscrnHandler();

      virtual bool isFileType(LineDataFile &ascii_file) const;

      static std::string getFormatString() { return "uscrn"; }

   protected:

      ///////////////////////
      // Protected members
      ///////////////////////
 
      // Unchanging header information
      USCRNFormat _format;
      std::string _formatName;
      std::string _stationId;
      double _stationLat;
      double _stationLon;

      ///////////////////////
      // Protected methods
      ///////////////////////

      // Determine the USCRN format from the file name 
      USCRNFormat _getFileFormat(const LineDataFile &ascii_file) const;

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
