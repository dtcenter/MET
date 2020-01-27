// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

class Ascii2NcConfInfo {

public:
     
  Ascii2NcConfInfo();
  ~Ascii2NcConfInfo();

  void read_config(const string &default_filename,
		   const string &user_filename);

  TimeSummaryInfo getSummaryInfo() const
  {
     return _timeSummaryInfo;
  }
  
  map<ConcatString,ConcatString> getMessageTypeMap() const
  {
     return _messageTypeMap;
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
  map<ConcatString,ConcatString> _messageTypeMap;
  

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
