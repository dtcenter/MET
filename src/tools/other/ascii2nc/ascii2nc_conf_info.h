// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

//#include "tc_stat_job.h"

//#include "mask_poly.h"

#include "config_file.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class Ascii2NcConfInfo {

public:
     
  // TCStatJob to store filtering info
//      TCStatJob Filter;
      
  // Jobs array
//      StringArray Jobs;
      
  Ascii2NcConfInfo();
  ~Ascii2NcConfInfo();

  void read_config(const string &filename);

  time_t getSummarizeBeginTime() const
  {
    return _summarizeBegin;
  }
  
  time_t getSummarizeEndTime() const
  {
    return _summarizeEnd;
  }
  
  int getSummarizeIntervalSecs() const
  {
    return _summarizeInterval;
  }
  
protected:

  // TCPairs configuration object

  MetConfig Conf;

  // Config file version

  ConcatString _version;
      
  time_t _summarizeBegin;
  time_t _summarizeEnd;
  time_t _summarizeInterval;
  
  void clear();
  void init_from_scratch();
  void process_config();

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __ASCII2NC_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
