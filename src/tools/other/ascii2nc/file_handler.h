// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __FILE_HANDLER_H__
#define  __FILE_HANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <map>
#include <time.h>
#include <vector>

#include <netcdf>

#include "mask_poly.h"
#include "mask_filters.h"
#include "vx_grid.h"
#include "vx_config.h"
#include "vx_util.h"
#include "nc_utils.h"
#include "write_netcdf.h"

#include "observation.h"
#include "vx_summary.h"

#include "nc_obs_util.h"
#include "nc_point_obs_out.h"
#include "nc_summary.h"

////////////////////////////////////////////////////////////////////////


class FileHandler
{

public:

  FileHandler(const std::string &program_name);
  virtual ~FileHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const = 0;

  void setGridMask(Grid        &g);
  void setAreaMask(MaskPlane   &a);
  void setPolyMask(MaskPoly    &p);
  void setSIDMask (StringArray &s);
  void setMessageTypeMap(std::map<ConcatString, ConcatString> m);

  virtual bool readAsciiFiles(const std::vector< ConcatString > &ascii_filename_list);
  bool writeNetcdfFile(const std::string &nc_filename);

  bool summarizeObs(const TimeSummaryInfo &summary_info);

  void setCompressionLevel(int compressoion_level);
  void setSummaryInfo(bool new_do_summary);
  void setSummaryInfo(const TimeSummaryInfo &summary_info);
  void setValidTimeRange(const time_t &valid_beg, const time_t valid_end);

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const float FILL_VALUE;


  ///////////////////////
  // Protected members //
  ///////////////////////

  std::string _programName;

  // Variables for writing output NetCDF file

  netCDF::NcFile   *_ncFile;
  MetNcPointObsOut nc_point_obs;

  long _nhdr;

  int _hdrNum;
  int _obsNum;

  MaskFilters filters;

  std::map<ConcatString, ConcatString> _messageTypeMap;

  // List of observations read from the ascii files

  std::vector< Observation > _observations;
  bool  use_var_id;
  StringArray obs_names;
  StringArray obs_units;

  bool do_monitor;
  int start_time, end_time;

  int deflate_level;

  bool _dataSummarized;
  bool do_summary;
  TimeSummaryInfo _summaryInfo;
  SummaryObs summary_obs;

  time_t valid_beg_ut, valid_end_ut;
  int num_observations_in_range;
  int num_observations_out_of_range;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Count the number of headers needed for the netCDF file.  All of the
  // observations must be loaded into the _observations vector before calling
  // this method.

  void _countHeaders();

  time_t _getValidTime(const std::string &time_string) const;

  // Read the observations from the given file.

  virtual bool _readObservations(LineDataFile &ascii_file) = 0;

  // Apply filtering logic to the observations and add them to the
  // _observations vector.

  bool _addObservations(const Observation &obs);

  // Write the observations in the _observations vector into the current
  // netCDF file.

  bool _writeObservations();

  void _closeNetcdf();
  bool _openNetcdf(const std::string &nc_filename);
  void debug_print_observations(std::vector< Observation >, std::string);

  bool _keep_valid_time(const time_t &valid_time) const;

};

inline void FileHandler::setCompressionLevel(int compressoion_level) { deflate_level = compressoion_level; }
inline void FileHandler::setGridMask(Grid        &g) { filters.set_grid_mask(&g); }
inline void FileHandler::setAreaMask(MaskPlane   &a) { filters.set_area_mask(&a); }
inline void FileHandler::setPolyMask(MaskPoly    &p) { filters.set_poly_mask(&p); }
inline void FileHandler::setSIDMask (StringArray &s) { filters.set_sid_mask(&s); }
inline void FileHandler::setMessageTypeMap(std::map<ConcatString, ConcatString> m) {
   _messageTypeMap = m;
}


////////////////////////////////////////////////////////////////////////


#endif   /*  __FILE_HANDLER_H__  */


////////////////////////////////////////////////////////////////////////


