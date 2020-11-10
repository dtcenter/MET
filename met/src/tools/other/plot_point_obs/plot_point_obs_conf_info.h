// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __PLOT_POINT_OBS_CONF_INFO_H__
#define  __PLOT_POINT_OBS_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <set>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_analysis_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

#include "observation.h"

////////////////////////////////////////////////////////////////////////

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/PlotPointObsConfig_default";

////////////////////////////////////////////////////////////////////////

struct LocationInfo {
  double lat;
  double lon;
  double val;
};

extern bool operator<(const LocationInfo&, const LocationInfo&);

////////////////////////////////////////////////////////////////////////

class PlotPointObsOpt {

   private:

      void init_from_scratch();

   public:

      PlotPointObsOpt();
     ~PlotPointObsOpt();

      //////////////////////////////////////////////////////////////////

      // String filtering options

      StringArray msg_typ;
      StringArray sid_inc;
      StringArray sid_exc;
      StringArray obs_var;
      StringArray obs_qty;

      // Time filtering options

      unixtime valid_beg;
      unixtime valid_end;

      // Value filtering options

      SingleThresh lat_thresh;
      SingleThresh lon_thresh;
      SingleThresh elv_thresh;
      SingleThresh hgt_thresh;
      SingleThresh prs_thresh;
      SingleThresh obs_thresh;

      // Data processing options

      UserFunc_1Arg convert_fx;
      ThreshArray   censor_thresh;
      NumArray      censor_val;
      
      // Plotting options

      UserFunc_1Arg dotsize_fx;
      NumArray      line_color;
      double        line_width;
      NumArray      fill_color;
      PlotInfo      fill_plot_info;

      //////////////////////////////////////////////////////////////////

      // Unique collection of locations
      int n_obs;
      bool obs_value_flag;
      set<LocationInfo> locations;

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(Dictionary &);

      bool add(const Observation &);
};

////////////////////////////////////////////////////////////////////////

class PlotPointObsConfInfo {

   private:

      void init_from_scratch();

   public:

      PlotPointObsConfInfo();
     ~PlotPointObsConfInfo();
    
      //////////////////////////////////////////////////////////////////

      // PlotPointObs configuration object
      MetConfig conf;
    
      //////////////////////////////////////////////////////////////////

      // Options to plot a field of gridded data
      bool     grid_data_flag;
      VarInfo *grid_data_info;
      PlotInfo grid_plot_info;

      // Options for plotting point data
      vector<PlotPointObsOpt> point_opts;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *);

      void process_config(GrdFileType);

      bool add(const Observation &);
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __PLOT_POINT_OBS_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////

