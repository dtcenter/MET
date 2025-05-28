// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __IODADATA_CONF_INFO_H__
#define  __IODADATA_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_analysis_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

static constexpr  int string_data_len = 512;

static constexpr  char metadata_group_name[] = "MetaData";
static constexpr  char qc_group_name[] = "QCFlags";
static constexpr  char qc_postfix[] = "PreQC";
static constexpr  char obs_group_name[] = "ObsValue";
static constexpr  char derived_obs_group_name[] = "DerivedObsValue";

static constexpr  char DEF_DATA_CONFIG_NAME[] = "MET_BASE/config/IODADataConfig_default";

////////////////////////////////////////////////////////////////////////


class IODADataConfInfo {

   private:

      void init_from_scratch();

   public:

      // IODAData configuration object
      MetConfig conf;

      ThreshArray  missing_thresh;      // Fill value thresh array

      std::map<ConcatString,ConcatString> obs_name_map;
      std::map<ConcatString,StringArray>  metadata_map;
      std::map<ConcatString,StringArray>  obs_to_qc_map;

      IODADataConfInfo();
     ~IODADataConfInfo();

      void clear();

      std::map<ConcatString,ConcatString> getObsVarMap() const { return obs_name_map; }

      void read_data_config(const char *, const char *);
      void process_data_config();
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __IODADATA_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
