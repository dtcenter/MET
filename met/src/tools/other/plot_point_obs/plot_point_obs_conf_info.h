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

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_analysis_util.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

// Default configuration file name
static const char * default_config_filename =
   "MET_BASE/config/PlotPointObsConfig_default";

////////////////////////////////////////////////////////////////////////

struct ObsInfo {
  double lat;
  double lon;
  double val;
};

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
      PlotInfo      fill_ctable;
      bool          fill_ctable_flag;
      bool          fill_colorbar_flag;

      //////////////////////////////////////////////////////////////////

      vector<ObsInfo> point_locations;

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(Dictionary &);

      // JHG add obs record here
      bool add();
};

////////////////////////////////////////////////////////////////////////

class PlotPointObsConfInfo {

   private:

      void init_from_scratch();

   public:

      PlotPointObsConfInfo();
     ~PlotPointObsConfInfo();

      //////////////////////////////////////////////////////////////////

      // Options to plot a field of gridded data
      bool     data_plane_flag;
      VarInfo *data_plane_info;
      PlotInfo data_ctable;
      bool     data_ctable_flag;
      bool     data_colorbar_flag;

      //////////////////////////////////////////////////////////////////

      // PlotPointObs configuration object
      MetConfig conf;

      // Array of plotting options
      vector<PlotPointObsOpt> plot_opts;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config   (const char *);
      void process_config();

      // JHG add obs record here
      bool add();
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __PLOT_POINT_OBS_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////

