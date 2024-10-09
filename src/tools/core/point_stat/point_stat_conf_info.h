// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
static const int i_fho       =  0;
static const int i_ctc       =  1;
static const int i_cts       =  2;
static const int i_mctc      =  3;
static const int i_mcts      =  4;

static const int i_cnt       =  5;
static const int i_sl1l2     =  6;
static const int i_sal1l2    =  7;
static const int i_vl1l2     =  8;
static const int i_val1l2    =  9;

static const int i_pct       = 10;
static const int i_pstd      = 11;
static const int i_pjc       = 12;
static const int i_prc       = 13;
static const int i_ecnt      = 14;

static const int i_orank     = 15;
static const int i_rps       = 16;
static const int i_eclv      = 17;
static const int i_mpr       = 18;
static const int i_vcnt      = 19;
static const int i_seeps_mpr = 20;
static const int i_seeps     = 21;

static const int n_txt       = 22;

// Text file type
static const STATLineType txt_file_type[n_txt] = {

   STATLineType::fho,       //  0
   STATLineType::ctc,       //  1
   STATLineType::cts,       //  2
   STATLineType::mctc,      //  3
   STATLineType::mcts,      //  4

   STATLineType::cnt,       //  5
   STATLineType::sl1l2,     //  6
   STATLineType::sal1l2,    //  7
   STATLineType::vl1l2,     //  8
   STATLineType::val1l2,    //  9

   STATLineType::pct,       //  10   
   STATLineType::pstd,      //  11
   STATLineType::pjc,       //  12
   STATLineType::prc,       //  13
   STATLineType::ecnt,      //  14

   STATLineType::orank,     //  15
   STATLineType::rps,       //  16
   STATLineType::eclv,      //  17
   STATLineType::mpr,       //  18
   STATLineType::vcnt,      //  19

   STATLineType::seeps_mpr, //  20
   STATLineType::seeps      //  21
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

      VxPairDataPoint vx_pd;              // Matched pair data [n_msg_typ][n_mask][n_interp]

      int             beg_ds;             // Begin observation time window offset
      int             end_ds;             // End observation time window offset

      ThreshArray     fcat_ta;            // Array for fcst categorical thresholds
      ThreshArray     ocat_ta;            // Array for obs categorical thresholds

      ThreshArray     fcnt_ta;            // Array for fcst continuous thresholds
      ThreshArray     ocnt_ta;            // Array for obs continuous thresholds
      SetLogic        cnt_logic;          // Array of continuous threshold field logic

      ThreshArray     fwind_ta;           // Array for fcst wind speed thresholds
      ThreshArray     owind_ta;           // Array for obs wind speed thresholds
      SetLogic        wind_logic;         // Array of wind speed field logic

      bool            land_flag;          // Flag for land/sea mask filtering
      bool            topo_flag;          // Flag for topography filtering

      StringArray     mask_grid;          // Masking grid strings
      StringArray     mask_poly;          // Masking polyline strings
      StringArray     mask_sid;           // Masking station ID's

      StringArray     mpr_sa;             // MPR column names
      ThreshArray     mpr_ta;             // MPR column thresholds

      // Vector of MaskLatLon objects defining Lat/Lon Point masks
      std::vector<MaskLatLon> mask_llpnt;

      StringArray     mask_name;          // Masking names

      NumArray        eclv_points;        // ECLV points

      ClimoCDFInfo    cdf_info;           // Climo CDF info

      NumArray        ci_alpha;           // Alpha value for confidence intervals

      BootInfo        boot_info;          // Bootstrapping information
      InterpInfo      interp_info;        // Interpolation information
      HiRAInfo        hira_info;          // HiRA verification logic

      double          hss_ec_value;       // HSS expected correct value
      bool            rank_corr_flag;     // Flag for computing rank correlations

      StringArray     msg_typ;            // Array of message types

      DuplicateType   duplicate_flag;     // Duplicate observations
      ObsSummary      obs_summary;        // Summarize observations
      int             obs_perc;           // Summary percentile value

      // Output file options
      STATOutputType  output_flag[n_txt]; // Flag for each output line type

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(GrdFileType, Dictionary &, Dictionary &);
      void set_vx_pd(PointStatConfInfo *);
      bool is_uv_match(const PointStatVxOpt &) const;

      void set_perc_thresh(const PairDataPoint *);

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
      int get_n_hira_ens()     const;
      int get_n_hira_prob()    const;
      int get_n_cdf_bin()      const;
      int get_n_ci_alpha()     const;
};

////////////////////////////////////////////////////////////////////////

inline int PointStatVxOpt::get_n_msg_typ()     const { return msg_typ.n();          }
inline int PointStatVxOpt::get_n_mask()        const { return mask_name.n();        }
inline int PointStatVxOpt::get_n_interp()      const { return interp_info.n_interp; }

inline int PointStatVxOpt::get_n_eclv_points() const { return eclv_points.n();      }
inline int PointStatVxOpt::get_n_cdf_bin()     const { return cdf_info.n_bin;       }
inline int PointStatVxOpt::get_n_ci_alpha()    const { return ci_alpha.n();         }

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
      bool             grib_codes_set;

      // Land/sea mask and topography info for data filtering
      MaskPlane    land_mask;
      DataPlane    topo_dp;
      SingleThresh topo_use_obs_thresh;
      SingleThresh topo_interp_fcst_thresh;

      // Message type groups that should be processed together
      std::map<ConcatString,StringArray> msg_typ_group_map;

      // Mapping of mask names to DataPlanes
      std::map<ConcatString,MaskPlane>   mask_area_map;

      // Mapping of mask names to Station ID lists
      std::map<ConcatString,MaskSID>     mask_sid_map;

      ConcatString tmp_dir;                 // Directory for temporary files
      ConcatString output_prefix;           // String to customize output file name
      ConcatString version;                 // Config file version

      ConcatString seeps_climo_name;        // SEESP climo filename
      SingleThresh seeps_p1_thresh;         // SEESP p1 threshold

#ifdef WITH_UGRID
      bool ignore_ugrid_dataset;
      ConcatString ugrid_nc;                // NetCDF for coordinate variables of unstructured grid
      ConcatString ugrid_dataset;           // UGRid dataset name (mpas, lfric etc)
      ConcatString ugrid_map_config;        // User's configuration file which contains ugrid metadata mapping
      double ugrid_max_distance_km;         // max distance to be the closest neighbor to unstructured grid
#endif

      // Summary of output file options across all verification tasks
      STATOutputType output_flag[n_txt];    // Flag for each output line type

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);
#ifdef WITH_UGRID
      void read_ugrid_configs(StringArray ugrid_config_names, const char * user_config);
#endif

      void process_config(GrdFileType);
      void process_grib_codes();
      void process_flags();
      void process_masks(const Grid &);
      void process_geog(const Grid &, const char *);
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
      int get_max_n_hira_ens()     const;
      int get_max_n_hira_prob()    const;

      // Check for any verification of vectors
      bool get_vflag() const;
};

////////////////////////////////////////////////////////////////////////

inline int PointStatConfInfo::get_n_vx() const { return(n_vx); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __POINT_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
