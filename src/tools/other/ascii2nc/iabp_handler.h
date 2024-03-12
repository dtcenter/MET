// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __IABP_HANDLER_H__
#define  __IABP_HANDLER_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <time.h>

#include "file_handler.h"

////////////////////////////////////////////////////////////////////////

// struct obsVarInfo {
//    int _gribCode;
//    string _varName;
// };

////////////////////////////////////////////////////////////////////////
//
// International Arctic Buoy Programme
//    https://iabp.apl.uw.edu/data.html
// Files pulled from:
//    https://iabp.apl.uw.edu/WebData/
//    
//
// Dataset file names:
//    <BUOY>.dat  where <BUOY> is an integer.
//
// Dataset Conents:
//
//    Buoy data files are updated daily and made available individually at the WebData URL above.
//    Values provided are confined to surface temperature,atmospheric temperature, and barometric pressure when these values are available.
//    All buoy files contain at least dates and positions.
//    Each file has a header line and one or more data lines.
//
// Header Line example  (BP Ts and Ta are not always present, any subset could be there or not):
//
//     BuoyID     Year     Hour     Min     DOY     POS_DOY     Lat     Lon     BP     Ts     Ta
//
// Record Lines (many per file, typically):
//      Each file contains data from all buoys on a given date.
//      Data includes: BuoyID, year, hour, minute, Day of Year, Position Day of year,
//                     latitude, longitude, [Barometric Pressure], [Surface Temp], and [Atmospheric Temp].
//      The last 3 values are optional and depend on the header line.
//
// Record line Example
//
//     5318   2014   02   20   28.0970   28.0930   72.75970   -165.25190   1016.62   -999.00   -13.92
//    
// It looks like the missing data value is -999.0, but this might not always be the case, it seems to not be documented.
//

////////////////////////////////////////////////////////////////////////

class IabpHandler : public FileHandler {

   public:

      IabpHandler(const string &program_name);
      virtual ~IabpHandler();

      virtual bool isFileType(LineDataFile &ascii_file) const;

      static string getFormatString() { return "iabp"; }

   protected:

      /////////////////////////
      // Protected constants
      /////////////////////////

      // The number of columns in the second header line in the file.  This line
      // is used to determine if this is a IABP file since the first line has
      // an indeterminate number of tokens.

      static const int MIN_NUM_HDR_COLS;

      // // The number of columns in the observation lines in the file.

      // static const int NUM_OBS_COLS;

      ///////////////////////
      // Protected members
      ///////////////////////

      // Unchanging file name information
      // obsVarInfo _obsVarInfo;

      // Store list of unqiue output variable names
      StringArray _varNames;

      // pointers based on header content, could be fixed
      // but check just in case
      int _idPtr;
      int _yearPtr;
      int _hourPtr;
      int _minutePtr;
      int _doyPtr;
      int _posdoyPtr;
      int _latPtr;
      int _lonPtr;
      int _bpPtr;
      int _tsPtr;
      int _taPtr;

      // depends on which of the data is present in a file
      int _numColumns;

      string _buoyId;
      time_t _validTime;
      double _stationLat;
      double _stationLon;
      double _stationElv;
      double _bp;
      double _ts;
      double _ta;

      ///////////////////////
      // Protected methods
      ///////////////////////

      // Read and save the header information from the given file
      bool _readHeaderInfo(LineDataFile &ascii_file);

      // Get the valid time from the observation line
      // time_t _getValidTime(const DataLine &data_line) const;

      // Read the observations and add them to the
      // _observations vector
      virtual bool _readObservations(LineDataFile &ascii_file);

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __IABP_HANDLER_H__  */

////////////////////////////////////////////////////////////////////////
