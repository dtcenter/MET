// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __IODA2NC_CONF_INFO_H__
#define  __IODA2NC_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_analysis_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

class IODA2NCConfInfo {

   private:

      void init_from_scratch();

   public:

      // IODA2NC configuration object
      MetConfig conf;

      // Store data parsed from the IODA2NC configuration object
      StringArray  message_type;        // Obseration message type
      StringArray  station_id;          // Observation location station id
      int          beg_ds;              // Time range of observations to be retained,
      int          end_ds;              // Defined relative to the PrepBufr center time (seconds)
      Grid         grid_mask;           // Grid masking region
      MaskPlane    area_mask;           // Data masking region
      MaskPoly     poly_mask;           // Lat/Lon polyline masking region
      unixtime     valid_beg_ut;
      unixtime     valid_end_ut;
      double       beg_elev;            // Range of observing location elevations to be retained
      double       end_elev;
      double       beg_level;           // Range of level values to be retained
      double       end_level;
      StringArray  obs_var;             // IODA variiable names
      int          quality_mark_thresh; // Quality marks to be retained
      ThreshArray  missing_thresh;      // Fill value thresh array
      ConcatString version;             // Config file version

      std::map<ConcatString,ConcatString> obs_name_map;
      std::map<ConcatString,ConcatString> message_type_map;
      std::map<ConcatString,StringArray>  metadata_map;
      std::map<ConcatString,StringArray>  obs_to_qc_map;
      StringArray                    surface_message_types;
      TimeSummaryInfo                timeSummaryInfo;

      IODA2NCConfInfo();
     ~IODA2NCConfInfo();

      void clear();

      std::map<ConcatString,ConcatString> getObsVarMap() const { return obs_name_map; }
      std::map<ConcatString,ConcatString> getMessageTypeMap() const { return message_type_map; }
      TimeSummaryInfo getSummaryInfo() const { return timeSummaryInfo; };

      void read_config(const char *, const char *);
      void process_config();
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __IODA2NC_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
