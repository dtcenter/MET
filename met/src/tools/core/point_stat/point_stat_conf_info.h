// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __POINT_STAT_CONF_INFO_H__
#define  __POINT_STAT_CONF_INFO_H__

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

// Indices for the output flag types in the configuration file
static const int i_fho       = 0;
static const int i_ctc       = 1;
static const int i_cts       = 2;
static const int i_mctc      = 3;
static const int i_mcts      = 4;
static const int i_cnt       = 5;
static const int i_sl1l2     = 6;
static const int i_sal1l2    = 7;
static const int i_vl1l2     = 8;
static const int i_val1l2    = 9;
static const int i_pct       = 10;
static const int i_pstd      = 11;
static const int i_pjc       = 12;
static const int i_prc       = 13;
static const int i_eclv      = 14;
static const int i_mpr       = 15;
static const int n_txt       = 16;

// Text file type
static const STATLineType txt_file_type[n_txt] = {
   stat_fho,    stat_ctc,    stat_cts,
   stat_mctc,   stat_mcts,   stat_cnt,
   stat_sl1l2,  stat_sal1l2, stat_vl1l2,
   stat_val1l2, stat_pct,    stat_pstd,
   stat_pjc,    stat_prc,    stat_eclv,
   stat_mpr
};

////////////////////////////////////////////////////////////////////////

class PointStatConfInfo; // forward reference

////////////////////////////////////////////////////////////////////////

class PointStatVxOpt {

   private:

      void init_from_scratch();

   public:

      PointStatVxOpt();
     ~PointStatVxOpt();

      //////////////////////////////////////////////////////////////////

      VxPairDataPoint   vx_pd;              // Matched pair data [n_msg_typ][n_mask][n_interp]

      RegridInfo        regrid_info;        // Regridding information

      int               beg_ds;             // Begin observation time window offset
      int               end_ds;             // End observation time window offset

      ThreshArray       fcat_ta;            // Array for fcst categorical thresholds
      ThreshArray       ocat_ta;            // Array for obs categorical thresholds

      ThreshArray       fcnt_ta;            // Array for fcst continuous thresholds
      ThreshArray       ocnt_ta;            // Array for obs continuous thresholds
      SetLogic          cnt_logic;          // Array of continuous threshold field logic

      ThreshArray       fwind_ta;           // Array for fcst wind speed thresholds
      ThreshArray       owind_ta;           // Array for obs wind speed thresholds
      SetLogic          wind_logic;         // Array of wind speed field logic

      StringArray       mask_grid;          // Masking grid strings
      StringArray       mask_poly;          // Masking polyline strings
      StringArray       mask_sid;           // Masking station ID's

      StringArray       mask_name;          // Masking names

      NumArray          eclv_points;        // ECLV points

      ThreshArray       climo_cdf_ta;       // Climo CDF thresh array

      NumArray          ci_alpha;           // Alpha value for confidence intervals

      BootInfo          boot_info;          // Bootstrapping information
      InterpInfo        interp_info;        // Interpolation information
      HiRAInfo          hira_info;          // HiRA verification logic

      bool              rank_corr_flag;     // Flag for computing rank correlations

      StringArray       msg_typ;            // Array of message types

      DuplicateType     duplicate_flag;     // Duplicate observations
      ObsSummary        obs_summary;        // Summarize observations
      int               obs_perc;           // Summary percentile value

      // Output file options
      STATOutputType    output_flag[n_txt]; // Flag for each output line type

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(GrdFileType, Dictionary &, Dictionary &, bool);
      void set_vx_pd(PointStatConfInfo *);

      // Compute the number of output lines for this task
      int n_txt_row(int i)     const;

      int get_n_msg_typ()      const;
      int get_n_mask()         const;
      int get_n_interp()       const;

      int get_n_cnt_thresh()   const;
      int get_n_cat_thresh()   const;
      int get_n_wind_thresh()  const;

      int get_n_fprob_thresh() const;
      int get_n_oprob_thresh() const;

      int get_n_eclv_points()  const;
      int get_n_cdf_bin()      const;
      int get_n_ci_alpha()     const;
};

////////////////////////////////////////////////////////////////////////

inline int PointStatVxOpt::get_n_msg_typ()     const { return(msg_typ.n_elements());         }
inline int PointStatVxOpt::get_n_mask()        const { return(mask_name.n_elements());       }
inline int PointStatVxOpt::get_n_interp()      const { return(interp_info.n_interp);         }

inline int PointStatVxOpt::get_n_cnt_thresh()  const { return(fcnt_ta.n_elements());         }
inline int PointStatVxOpt::get_n_cat_thresh()  const { return(fcat_ta.n_elements());         }
inline int PointStatVxOpt::get_n_wind_thresh() const { return(fwind_ta.n_elements());        }

inline int PointStatVxOpt::get_n_eclv_points() const { return(eclv_points.n_elements());     }
inline int PointStatVxOpt::get_n_cdf_bin()     const { return(climo_cdf_ta.n_elements() - 1);}
inline int PointStatVxOpt::get_n_ci_alpha()    const { return(ci_alpha.n_elements());        }

////////////////////////////////////////////////////////////////////////

class PointStatConfInfo {

   private:

      void init_from_scratch();

      // Number of verification tasks
      int n_vx;

   public:

      PointStatConfInfo();
     ~PointStatConfInfo();

      //////////////////////////////////////////////////////////////////

      // Point-Stat configuration object
      MetConfig conf;

      // Store data parsed from the Point-Stat configuration object
      ConcatString model;                   // Model name

      PointStatVxOpt * vx_opt;              // Array of vx task options [n_vx] (allocated)

      // Message type groups that should be processed toegher
      map<ConcatString,StringArray> msg_typ_group_map;
      StringArray                   msg_typ_sfc;

      // Mapping of mask names to DataPlanes
      map<ConcatString,DataPlane>   mask_dp_map;

      // Mapping of mask names to Station ID lists
      map<ConcatString,StringArray> mask_sid_map;

      ConcatString tmp_dir;                 // Directory for temporary files
      ConcatString output_prefix;           // String to customize output file name
      ConcatString version;                 // Config file version

      // Summary of output file options across all verification tasks
      STATOutputType output_flag[n_txt];    // Flag for each output line type

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);
      void process_config(GrdFileType, bool);
      void process_flags();
      void process_masks(const Grid &);
      void set_vx_pd();

      // Dump out the counts
      int get_n_vx() const;

      // Compute the maximum number of output lines possible based
      // on the contents of the configuration file
      int n_txt_row(int i) const;
      int n_stat_row()     const;

      // Maximum across all verification tasks
      int get_max_n_cat_thresh()   const;
      int get_max_n_cnt_thresh()   const;
      int get_max_n_wind_thresh()  const;
      int get_max_n_fprob_thresh() const;
      int get_max_n_oprob_thresh() const;
      int get_max_n_eclv_points()  const;

      // Check for any verification of vectors
      bool get_vflag() const;
};

////////////////////////////////////////////////////////////////////////

inline int PointStatConfInfo::get_n_vx() const { return(n_vx); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __POINT_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
