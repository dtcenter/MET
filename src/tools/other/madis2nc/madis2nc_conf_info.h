// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __ASCII2NC_CONF_INFO_H__
#define  __ASCII2NC_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class Madis2NcConfInfo {

public:
     
  Madis2NcConfInfo();
  ~Madis2NcConfInfo();

  void read_config(const std::string &default_filename,
                   const std::string &user_filename);

  ConcatString get_grib_var_name(const int grib_code);
  ConcatString get_grib_var_unit(const int grib_code);

  TimeSummaryInfo getSummaryInfo() const
  {
     return _timeSummaryInfo;
  }
  
  int get_compression_level() { return _conf.nc_compression(); }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  // TCPairs configuration object

  MetConfig _conf;

  // Config file version

  ConcatString _version;
  TimeSummaryInfo _timeSummaryInfo;

  std::map<int,ConcatString> grib_name_map;
  std::map<int,ConcatString> grib_unit_map;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void clear();
  void init_from_scratch();

  void process_config();

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __ASCII2NC_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
