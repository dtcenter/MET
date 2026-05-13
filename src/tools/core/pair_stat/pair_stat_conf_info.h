// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __PAIR_STAT_CONF_INFO_H__
#define  __PAIR_STAT_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"
#include "vx_analysis_util.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////

// Reference global 1/10-th degree grid for applying masking regions
static const LatLonData GlobalTenthData =
   { "GlobalTenthDegree", -90.0, -0.0, 0.1, 0.1, 1801, 3601 };
static const Grid grid_mask(GlobalTenthData);

// Reference valid time for climatology data
static unixtime vx_valid_ut = 0;

// Indices for the output flag types in the configuration file
static const int i_fho       =  0;
static const int i_ctc       =  1;
static const int i_cts       =  2;
static const int i_mctc      =  3;
static const int i_mcts      =  4;

static const int i_cnt       =  5;
static const int i_sl1l2     =  6;
static const int i_sal1l2    =  7;

static const int i_vcnt      =  8;
static const int i_vl1l2     =  9;
static const int i_val1l2    = 10;

static const int i_pct       = 11;
static const int i_pstd      = 12;
static const int i_pjc       = 13;
static const int i_prc       = 14;
static const int i_eclv      = 15;

static const int i_mpr       = 16;
static const int i_seeps_mpr = 17;
static const int i_seeps     = 18;

static const int n_txt       = 19;

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

   STATLineType::vcnt,      //  8
   STATLineType::vl1l2,     //  9 
   STATLineType::val1l2,    // 10 

   STATLineType::pct,       // 11   
   STATLineType::pstd,      // 12
   STATLineType::pjc,       // 13 
   STATLineType::prc,       // 14
   STATLineType::eclv,      // 15

   STATLineType::mpr,       // 16
   STATLineType::seeps_mpr, // 17 
   STATLineType::seeps      // 18 
};

///////////////////////////////////////////////////////////////////////////////

//
// Supported input pairs formats 
//

enum class PairsFormat {
   None,   // Default
   MPR,    // ASCII files containing MET MPR lines
   Python, // Stat MPR data via Python embedding
   IODA,   // IODA pairs file
};

///////////////////////////////////////////////////////////////////////////////

extern ConcatString pairsformat_to_string(const PairsFormat);
extern PairsFormat  string_to_pairsformat(const std::string &);

///////////////////////////////////////////////////////////////////////////////

class PairStatConfInfo; // forward reference

///////////////////////////////////////////////////////////////////////////////

class PairStatVxOpt {

   private:

      void init_from_scratch();

   public:

      PairStatVxOpt();

      //////////////////////////////////////////////////////////////////

      VxPairDataPoint vx_pd;              // Matched pair data [n_mask]
      std::vector<StatHdrInfo> vx_hdr;    // Track header inputs [n_mask]
      bool convert_censor_flag;           // Conversion and/or censoring requested

      Grid grid_climo;                    // Grid for climatology data

      ThreshArray     fcat_ta;            // Array for fcst categorical thresholds
      ThreshArray     ocat_ta;            // Array for obs categorical thresholds

      ThreshArray     fcnt_ta;            // Array for fcst continuous thresholds
      ThreshArray     ocnt_ta;            // Array for obs continuous thresholds
      SetLogic        cnt_logic;          // Array of continuous threshold field logic

      ThreshArray     fwind_ta;           // Array for fcst wind speed thresholds
      ThreshArray     owind_ta;           // Array for obs wind speed thresholds
      SetLogic        wind_logic;         // Array of wind speed field logic

      StringArray     mask_grid;          // Masking grid strings
      StringArray     mask_poly;          // Masking polyline strings
      StringArray     mask_sid;           // Masking station ID's

      // Matched pair inclusion thresholds
      std::map<ConcatString,ThreshArray> mpr_thr_inc_map;

      // Matched pair inclusion and exclusion strings
      std::map<ConcatString,StringArray> mpr_str_inc_map;
      std::map<ConcatString,StringArray> mpr_str_exc_map;

      // Matched pair time inclusion and exclusion logic
      IntArray  fcst_lead; // stored in seconds
      IntArray  obs_lead;  // stored in seconds

      unixtime  fcst_valid_beg;
      unixtime  fcst_valid_end;
      TimeArray fcst_valid_inc;
      TimeArray fcst_valid_exc;
      IntArray  fcst_valid_hour; // stored in seconds

      unixtime  obs_valid_beg;
      unixtime  obs_valid_end;
      TimeArray obs_valid_inc;
      TimeArray obs_valid_exc;
      IntArray  obs_valid_hour; // stored in seconds

      unixtime  fcst_init_beg;
      unixtime  fcst_init_end;
      TimeArray fcst_init_inc;
      TimeArray fcst_init_exc;
      IntArray  fcst_init_hour; // stored in seconds

      unixtime  obs_init_beg;
      unixtime  obs_init_end;
      TimeArray obs_init_inc;
      TimeArray obs_init_exc;
      IntArray  obs_init_hour;  // stored in seconds

      // Vector of MaskLatLon objects defining Lat/Lon Point masks
      std::vector<MaskLatLon> mask_llpnt;

      StringArray     mask_name;          // Masking names

      NumArray        eclv_points;        // ECLV points

      ClimoCDFInfo    cdf_info;           // Climo CDF info

      NumArray        ci_alpha;           // Alpha value for confidence intervals

      BootInfo        boot_info;          // Bootstrapping information

      double          hss_ec_value;       // HSS expected correct value
      bool            rank_corr_flag;     // Flag for computing rank correlations

      // Output file options
      STATOutputType output_flag[n_txt];  // Flag for each output line type

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(PairsFormat, Dictionary &, Dictionary &);
      void set_vx_pd(PairStatConfInfo *);
      bool is_uv_match(const PairStatVxOpt &) const;

      void set_perc_thresh(const PairDataPoint *);

      // Compute the number of output lines for this task
      int n_txt_row(int i)     const;

      int get_n_mask()         const;

      int get_n_cnt_thresh()   const;
      int get_n_cat_thresh()   const;
      int get_n_wind_thresh()  const;

      int get_n_fprob_thresh() const;
      int get_n_oprob_thresh() const;

      int get_n_eclv_points()  const;
      int get_n_cdf_bin()      const;
      int get_n_ci_alpha()     const;

      // Check for matches
      bool is_keeper_mpr(const STATLine &) const;
      bool is_keeper_ioda(const point_pair_t &) const;

      // Check time filters
      bool is_keeper_lead_time      (const int,      const int)      const;
      bool is_keeper_fcst_valid_time(const unixtime, const unixtime) const;
      bool is_keeper_obs_valid_time (const unixtime, const unixtime) const;
      bool is_keeper_fcst_init_time (const unixtime, const unixtime) const;
      bool is_keeper_obs_init_time  (const unixtime, const unixtime) const;

      // Add paired data
      bool add_mpr_line(STATLine);
      bool add_ioda_pair(point_pair_t);

      // Apply conversion and censoring logic
      void apply_convert_censor(STATLine &) const;
      void apply_convert_censor(point_pair_t &) const;
      void apply_convert_censor(const VarInfo *, double &) const;
};

////////////////////////////////////////////////////////////////////////

inline int PairStatVxOpt::get_n_mask()        const { return mask_name.n();        }
inline int PairStatVxOpt::get_n_eclv_points() const { return eclv_points.n();      }
inline int PairStatVxOpt::get_n_cdf_bin()     const { return cdf_info.n_bin;       }
inline int PairStatVxOpt::get_n_ci_alpha()    const { return ci_alpha.n();         }

////////////////////////////////////////////////////////////////////////

class PairStatConfInfo {

   private:

      void init_from_scratch();

      // Number of verification tasks
      int n_vx;

   public:

      PairStatConfInfo();

      //////////////////////////////////////////////////////////////////

      // Pair-Stat configuration object
      MetConfig conf;

      // Model name from the Pair-Stat config file
      ConcatString model;                   // Model name

      std::vector<PairStatVxOpt> vx_opt;    // Vector of vx options [n_vx]

      // Mapping of mask names to DataPlanes
      std::map<ConcatString,MaskPlane> mask_area_map;

      // Mapping of mask names to Station ID lists
      std::map<ConcatString,MaskSID> mask_sid_map;

      PointWeightInfo point_weight_info;    // Point weighting information

      ConcatString tmp_dir;                 // Directory for temporary files
      ConcatString version;                 // Config file version

      ConcatString seeps_climo_name;        // SEESP climo filename
      SingleThresh seeps_p1_thresh;         // SEESP p1 threshold

      // Summary of output file options across all verification tasks
      STATOutputType output_flag[n_txt];    // Flag for each output line type

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const StringArray &);

      void process_config(PairsFormat);
      void process_flags();
      void process_masks();
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

      // Add paired data
      bool add_mpr_line(STATLine);
};

////////////////////////////////////////////////////////////////////////

inline int PairStatConfInfo::get_n_vx() const { return n_vx; }

////////////////////////////////////////////////////////////////////////

#endif   /*  __PAIR_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
