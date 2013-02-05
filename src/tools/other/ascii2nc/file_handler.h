

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
#include <vector>

#include "netcdf.hh"

#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


class FileHandler
{

public:

  FileHandler(const string &program_name);
  virtual ~FileHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const = 0;
  
  bool processFiles(const vector< ConcatString > &ascii_filename_list,
		    const string &nc_filename);
//  bool processFiles(const ConcatString &ascii_filename,
//		   const string &nc_filename);
  
protected:

  static const int   HDR_ARRAY_LEN;
  static const int   OBS_ARRAY_LEN;
  static const float FILL_VALUE;
  

  // Prepare the headers for adding to the netCDF file.  Update _nhdr to
  // contain the number of header records the file will contain.

  virtual bool _prepareHeaders(LineDataFile &ascii_file) = 0;

  // Process the observations in the file.  Assumes that _nhdr contains the
  // number of header records for this file.

  virtual bool _processObs(LineDataFile &ascii_file,
			   const string &nc_filename) = 0;
  

  void _closeNetcdf();
  bool _openNetcdf(const string &nc_filename);
  bool _writeHdrInfo(const ConcatString &hdr_typ,
		     const ConcatString &hdr_sid,
		     const ConcatString &hdr_vld,
		     double lat, double lon, double elv);
  bool _writeObsInfo(int gc, float prs, float hgt, float obs,
		     const ConcatString &qty);
  

  string _programName;
  string _asciiFilename;
  
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
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __FILEHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


