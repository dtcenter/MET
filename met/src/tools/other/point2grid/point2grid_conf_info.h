// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __POINT_TO_GRID_CONF_INFO_H__
#define  __POINT_TO_GRID_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_analysis_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

class PointToGridConfInfo {

   private:

      void init_from_scratch();

   protected:
      map<ConcatString,ConcatString> var_name_map;
      map<ConcatString,ConcatString> def_var_name_map;

   public:

      // PointToGrid configuration object
      MetConfig conf;

      // Store data parsed from the PB2NC configuration object
      StringArray  message_type;        // Obseration message type
      unixtime     valid_time;          // valid time
      int          beg_ds;              // Time range of observations to be retained,
      int          end_ds;              // Defined relative to the PrepBufr center time (seconds)
      int          quality_mark_thresh; // Quality marks to be retained
      ConcatString version;             // Config file version

      PointToGridConfInfo();
     ~PointToGridConfInfo();

      void clear();

      void process_config();
      void read_config(const char *, const char *);
      ConcatString get_var_name(const ConcatString);
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __POINT_TO_GRID_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
