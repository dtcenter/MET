// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __SERIES_ANALYSIS_CONF_INFO_H__
#define  __SERIES_ANALYSIS_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////

class SeriesAnalysisConfInfo {

   private:

      void init_from_scratch();

      // Counts based on the contents of the config file
      int n_fcst;                          // Number of forecast fields
      int n_obs;                           // Number of observation fields

   public:

      // Series-Analysis configuration object
      MetConfig conf;

      // Store data parsed from the Series-Analysis configuration object
      ConcatString     model;              // Model name
      ConcatString     desc;               // Description
      ConcatString     obtype;             // Observation type

      VarInfo **       fcst_info;          // Array of pointers for fcst VarInfo [n_fcst]
      VarInfo **       obs_info;           // Array of pointers for obs VarInfo [n_obs]

      ThreshArray      fcat_ta;            // Categorical fcst thresholds
      ThreshArray      ocat_ta;            // Categorical obs thresholds

      ThreshArray      fcnt_ta;            // Continuous fcst thresholds
      ThreshArray      ocnt_ta;            // Continuous obs thresholds
      SetLogic         cnt_logic;          // Continuous threshold field logic

      NumArray         ci_alpha;           // Alpha value for confidence intervals
      BootIntervalType boot_interval;      // Bootstrap CI type
      double           boot_rep_prop;      // Bootstrap replicate proportion
      int              n_boot_rep;         // Number of bootstrap replicates
      ConcatString     boot_rng;           // GSL random number generator
      ConcatString     boot_seed;          // GSL RNG seed value

      ConcatString     mask_grid_file;     // Path for masking grid area
      ConcatString     mask_grid_name;     // Name of masking grid area
      ConcatString     mask_poly_file;     // Path for masking poly area
      ConcatString     mask_poly_name;     // Name of masking poly area
      MaskPlane        mask_area;

      int              block_size;         // Number of grid points to read concurrently
      double           vld_data_thresh;    // Minimum valid data ratio for each point
      bool             rank_corr_flag;     // Flag for computing rank correlations

      ConcatString     tmp_dir;            // Directory for temporary files
      ConcatString     version;            // Config file version

      // Mapping of line type to output statistics
      map<STATLineType,StringArray> output_stats;

      SeriesAnalysisConfInfo();
     ~SeriesAnalysisConfInfo();

      void clear();

      void read_config   (const char *, const char *);
      void process_config(GrdFileType, GrdFileType);
      void process_masks (const Grid &);
      int get_compression_level();

      // Dump out the counts
      int get_n_fcst() const;
      int get_n_obs()  const;
};

////////////////////////////////////////////////////////////////////////

inline int SeriesAnalysisConfInfo::get_n_fcst() const { return(n_fcst); }
inline int SeriesAnalysisConfInfo::get_n_obs()  const { return(n_obs);  }
inline int SeriesAnalysisConfInfo::get_compression_level()  { return conf.nc_compression(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __SERIES_ANALYSIS_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
