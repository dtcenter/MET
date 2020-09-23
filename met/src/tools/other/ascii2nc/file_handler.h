// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include <netcdf>
using namespace netCDF;

#include "mask_poly.h"
#include "vx_grid.h"
#include "vx_config.h"
#include "vx_util.h"
#include "nc_utils.h"
#include "write_netcdf.h"

#include "observation.h"
#include "vx_summary.h"

#include "nc_obs_util.h"
#include "nc_summary.h"

////////////////////////////////////////////////////////////////////////


class FileHandler
{

public:

  FileHandler(const string &program_name);
  virtual ~FileHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const = 0;

  void setGridMask(Grid        &g);
  void setAreaMask(MaskPlane   &a);
  void setPolyMask(MaskPoly    &p);
  void setSIDMask (StringArray &s);
  void setMessageTypeMap(map<ConcatString, ConcatString> m);

  virtual bool readAsciiFiles(const vector< ConcatString > &ascii_filename_list);
  bool writeNetcdfFile(const string &nc_filename);

  bool summarizeObs(const TimeSummaryInfo &summary_info);

  void setCompressionLevel(int compressoion_level);
  void setSummaryInfo(bool new_do_summary);
  void setSummaryInfo(const TimeSummaryInfo &summary_info);


protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const float FILL_VALUE;


  ///////////////////////
  // Protected members //
  ///////////////////////

  string _programName;

  // Variables for writing output NetCDF file

  NcFile *_ncFile;
  NetcdfObsVars obs_vars;
  NcObsOutputData nc_out_data;

  long _nhdr;

  int _hdrNum;
  int _obsNum;

  int _gridMaskNum;
  int _areaMaskNum;
  int _polyMaskNum;
  int _sidMaskNum;

  Grid        *_gridMask;
  MaskPlane   *_areaMask;
  MaskPoly    *_polyMask;
  StringArray *_sidMask;

  map<ConcatString, ConcatString> _messageTypeMap;

  // List of observations read from the ascii files

  vector< Observation > _observations;
  bool  use_var_id;
  StringArray obs_names;

  bool do_monitor;
  int start_time, end_time;

  int deflate_level;

  bool _dataSummarized;
  bool do_summary;
  TimeSummaryInfo _summaryInfo;
  SummaryObs summary_obs;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Count the number of headers needed for the netCDF file.  All of the
  // observations must be loaded into the _observations vector before calling
  // this method.

  void _countHeaders();

  time_t _getValidTime(const string &time_string) const;

  // Read the observations from the given file.

  virtual bool _readObservations(LineDataFile &ascii_file) = 0;

  // Apply filtering logic to the observations and add them to the
  // _observations vector.

  bool _addObservations(const Observation &obs);

  // Write the observations in the _observations vector into the current
  // netCDF file.

  bool _writeObservations();

  void _closeNetcdf();
  bool _openNetcdf(const string &nc_filename);
//  bool _writeHdrInfo(const ConcatString &hdr_typ,
//                     const ConcatString &hdr_sid,
//                     const time_t hdr_vld,
//                     double lat, double lon, double elv);
//  bool _writeObsInfo(int gc, float prs, float hgt, float obs,
//                     const ConcatString &qty);
  void debug_print_observations(vector< Observation >, string);
};

inline void FileHandler::setCompressionLevel(int compressoion_level) { deflate_level = compressoion_level; }
inline void FileHandler::setGridMask(Grid        &g) { _gridMask = &g; }
inline void FileHandler::setAreaMask(MaskPlane   &a) { _areaMask = &a; }
inline void FileHandler::setPolyMask(MaskPoly    &p) { _polyMask = &p; }
inline void FileHandler::setSIDMask (StringArray &s) { _sidMask  = &s; }
inline void FileHandler::setMessageTypeMap(map<ConcatString, ConcatString> m) {
   _messageTypeMap = m;
}


////////////////////////////////////////////////////////////////////////


#endif   /*  __FILEHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


