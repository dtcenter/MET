// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AERONETHANDLER_H__
#define  __AERONETHANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <time.h>

#include "file_handler.h"


////////////////////////////////////////////////////////////////////////


class AeronetHandler : public FileHandler
{

public:

  AeronetHandler(const string &program_name);
  virtual ~AeronetHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const;

  void setFormatVersion(int version);

  static string getFormatString()
  {
    return "aeronet";
  }

  static string getFormatString_v2()
  {
    return "aeronetv2";
  }

  static string getFormatString_v3()
  {
    return "aeronetv3";
  }

  /////////////////////////
  // Protected constants //
  /////////////////////////

  // The number of columns in the third header line in the file.  This line
  // is used to determine if this is an AERONET file since the first line has
  // an indeterminate number of tokens.

  static const int NUM_HDR_COLS;

  // The number of columns in the observation lines in the file.

  static const int NUM_OBS_COLS;

  // The header type for these observations

  static const string HEADER_TYPE;

  // Grib codes for the different fields

  static const int AOT_GRIB_CODE;

protected:  

  /////////////////////////
  // Protected constants //
  /////////////////////////

  // Unchanging header information

  string _stationId;
  double _stationLat;
  double _stationLon;
  double _stationAlt;

  StringArray var_names;
  
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Read and save the header information from the given file,  The file pointer
  // is assumed to be at the beginning of the file.  Sets the _stationId,
  // _stationLat, _stationLon and _stationAlt values.

  bool _readHeaderInfo(LineDataFile &ascii_file);
  
  // Get the observation valid time from the given observation line

  time_t _getValidTime(const DataLine &data_line) const;
  
  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file);

  // Extract the height from the field name
  double extract_height(string hdr_field);
  
  // Get the number of headers
  int get_header_count_v3(StringArray hdr_tokens);
  
  // Make the variable name from header (field name)
  string make_var_name_from_header(string hdr_field);
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __AERONETHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


