

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __FILEHANDLER_H__
#define  __FILEHANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <map>
#include <time.h>
#include <vector>

#include "netcdf.hh"

#include "vx_util.h"

#include "observation.h"
#include "summary_key.h"

////////////////////////////////////////////////////////////////////////


class FileHandler
{

public:

  FileHandler(const string &program_name);
  virtual ~FileHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const = 0;
  
  bool readAsciiFiles(const vector< ConcatString > &ascii_filename_list);
  bool writeNetcdfFile(const string &nc_filename);
  
  bool summarizeObs(const time_t start_time, const time_t end_time,
		    const int interval_secs);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int HDR_ARRAY_LEN;
  static const int OBS_ARRAY_LEN;

  static const float FILL_VALUE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _programName;
  
  // Variables for writing output NetCDF file

  NcFile *_ncFile;
  NcVar  *_hdrTypeVar;
  NcVar  *_hdrStationIdVar;
  NcVar  *_hdrValidTimeVar;
  NcVar  *_hdrArrayVar;
  NcVar  *_obsQualityVar;
  NcVar  *_obsArrayVar;

  int _nhdr;

  int _hdrNum;
  int _obsNum;

  // List of observations read from the ascii files

  vector< Observation > _observations;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Count the number of headers needed for the netCDF file.  All of the
  // observations must be loaded into the _observations vector before calling
  // this method.

  void _countHeaders();
  
  time_t _getValidTime(const string &time_string) const;
  
  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file) = 0;
 
  // Write the observations in the _observations vector into the current
  // netCDF file.

  bool _writeObservations();
  
  void _closeNetcdf();
  bool _openNetcdf(const string &nc_filename);
  bool _writeHdrInfo(const ConcatString &hdr_typ,
		     const ConcatString &hdr_sid,
		     const ConcatString &hdr_vld,
		     double lat, double lon, double elv);
  bool _writeObsInfo(int gc, float prs, float hgt, float obs,
		     const ConcatString &qty);
  

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __FILEHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


