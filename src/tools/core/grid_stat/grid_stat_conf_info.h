// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_STAT_CONF_INFO_H__
#define  __GRID_STAT_CONF_INFO_H__

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

static const int i_fho    = 0;
static const int i_ctc    = 1;
static const int i_cts    = 2;

static const int i_mctc   = 3;
static const int i_mcts   = 4;
static const int i_cnt    = 5;

static const int i_sl1l2  = 6;
static const int i_sal1l2 = 7;
static const int i_vl1l2  = 8;

static const int i_val1l2 = 9;
static const int i_pct    = 10;
static const int i_pstd   = 11;

static const int i_pjc    = 12;
static const int i_prc    = 13;
static const int i_eclv   = 14;

static const int i_nbrctc = 15;
static const int i_nbrcts = 16;
static const int i_nbrcnt = 17;

static const int i_grad   = 18;

static const int i_vcnt   = 19;

static const int i_dmap   = 20;

static const int i_seeps  = 21;

static const int n_txt    = 22;

// Text file type
static const STATLineType txt_file_type[n_txt] = {

   STATLineType::fho,        //  0
   STATLineType::ctc,        //  1
   STATLineType::cts,        //  2

   STATLineType::mctc,       //  3
   STATLineType::mcts,       //  4
   STATLineType::cnt,        //  5

   STATLineType::sl1l2,      //  6
   STATLineType::sal1l2,     //  7
   STATLineType::vl1l2,      //  8

   STATLineType::val1l2,     //  9
   STATLineType::pct,        //  10
   STATLineType::pstd,       //  11

   STATLineType::pjc,        //  12
   STATLineType::prc,        //  13
   STATLineType::eclv,       //  14

   STATLineType::nbrctc,     //  15
   STATLineType::nbrcts,     //  16
   STATLineType::nbrcnt,     //  17

   STATLineType::grad,       //  18
   STATLineType::vcnt,       //  19
   STATLineType::dmap,       //  20
   STATLineType::seeps       //  21

};

////////////////////////////////////////////////////////////////////////

struct GridStatNcOutInfo {

   bool do_latlon;
   bool do_raw;
   bool do_diff;
   bool do_climo;
   bool do_climo_cdp;
   bool do_weight;
   bool do_nbrhd;
   bool do_fourier;
   bool do_gradient;
   bool do_distance_map;
   bool do_apply_mask;

      //////////////////////////////////////////////////////////////////

   GridStatNcOutInfo();

   void clear();   //  sets everything to true

   bool all_false() const;

   void set_all_false();
   void set_all_true();
};

////////////////////////////////////////////////////////////////////////

class GridStatVxOpt {

   private:

      void init_from_scratch();

   public:

      GridStatVxOpt();
     ~GridStatVxOpt();

      //////////////////////////////////////////////////////////////////

      VarInfo *        fcst_info;        // fcst VarInfo pointer (allocated)
      VarInfo *        obs_info;         // obs VarInfo pointer (allocated)

      ConcatString     desc;             // Description string
      ConcatString     var_name;         // nc_pairs_var_name string
      ConcatString     var_suffix;       // nc_pairs_var_suffix string
                                         // nc_pairs_var_str is deprecated

      StringArray      mpr_sa;           // MPR filtering columns
      ThreshArray      mpr_ta;           // MPR filtering thresholds

      ThreshArray      fcat_ta;          // fcst categorical thresholds
      ThreshArray      ocat_ta;          // obs categorical thresholds

      ThreshArray      fcnt_ta;          // fcst continuous thresholds
      ThreshArray      ocnt_ta;          // obs continuous thresholds
      SetLogic         cnt_logic;        // continuous threshold field logic

      ThreshArray      fwind_ta;         // fcst wind speed thresholds
      ThreshArray      owind_ta;         // obs wind speed thresholds
      SetLogic         wind_logic;       // wind speed field logic

      SingleThresh     seeps_p1_thresh;    // SEESP p1 threshold

      StringArray      mask_grid;        // Masking grid strings
      StringArray      mask_poly;        // Masking polyline strings

      StringArray      mask_name;        // Masking region names

      NumArray         eclv_points;      // ECLV points

      ClimoCDFInfo     cdf_info;         // Climo CDF info

      NumArray         ci_alpha;         // Alpha value for confidence intervals

      BootInfo         boot_info;        // Bootstrapping information
      InterpInfo       interp_info;      // Interpolation (smoothing) information
      NbrhdInfo        nbrhd_info;       // Neighborhood statistics information

      // Fourier Options
      IntArray         wave_1d_beg;      // Fourier 1-dimensional decomposition
      IntArray         wave_1d_end;      // beginning and ending wave numbers

      // Gradient Options
      IntArray         grad_dx;          // Gradient step size in the X direction
      IntArray         grad_dy;          // Gradient step size in the Y direction

      // Distance Map Options
      int              baddeley_p;        // Exponent for lp-norm
      double           baddeley_max_dist; // Maximum distance constant
      double           fom_alpha;         // FOM Alpha
      double           zhu_weight;        // Zhu Weight
      UserFunc_1Arg    beta_value_fx;     // G-Beta Value Function

      double           hss_ec_value;      // HSS expected correct value
      bool             rank_corr_flag;    // Flag for computing rank correlations

      // Output file options
      STATOutputType    output_flag[n_txt]; // Flag for each output line type
      GridStatNcOutInfo nc_info;            // Output NetCDF pairs file contents

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(GrdFileType, Dictionary &,
                          GrdFileType, Dictionary &);
      void parse_nc_info(Dictionary &);
      bool is_uv_match(const GridStatVxOpt &) const;

      void set_perc_thresh(const PairDataPoint &);

      // Compute the number of output lines for this task
      int n_txt_row(int i)     const;

      int get_n_mask()         const;
      int get_n_interp()       const;

      int get_n_cnt_thresh()   const;
      int get_n_cat_thresh()   const;
      int get_n_wind_thresh()  const;

      int get_n_fprob_thresh() const;
      int get_n_oprob_thresh() const;

      int get_n_eclv_points()  const;
      int get_n_cdf_bin()      const;
      int get_n_nbrhd_wdth()   const;
      int get_n_cov_thresh()   const;
      int get_n_wave_1d()      const;
      int get_n_grad()         const;
      int get_n_ci_alpha()     const;
};

////////////////////////////////////////////////////////////////////////

inline int  GridStatVxOpt::get_n_mask()        const { return(mask_name.n_elements());         }
inline int  GridStatVxOpt::get_n_interp()      const { return(interp_info.n_interp);           }
inline int  GridStatVxOpt::get_n_eclv_points() const { return(eclv_points.n_elements());       }
inline int  GridStatVxOpt::get_n_cdf_bin()     const { return(cdf_info.n_bin);                 }
inline int  GridStatVxOpt::get_n_nbrhd_wdth()  const { return(nbrhd_info.width.n_elements());  }
inline int  GridStatVxOpt::get_n_cov_thresh()  const { return(nbrhd_info.cov_ta.n_elements()); }
inline int  GridStatVxOpt::get_n_wave_1d()     const { return(wave_1d_beg.n_elements());       }
inline int  GridStatVxOpt::get_n_grad()        const { return(grad_dx.n_elements());           }
inline int  GridStatVxOpt::get_n_ci_alpha()    const { return(ci_alpha.n_elements());          }

////////////////////////////////////////////////////////////////////////

class GridStatConfInfo {

   private:

      void init_from_scratch();

      // Number of verification tasks
      int n_vx;

      // Flags indicating requested output types
      bool output_ascii_flag;               // Flag for ASCII output
      bool output_nc_flag;                  // Flag for NetCDF output

   public:

      GridStatConfInfo();
     ~GridStatConfInfo();

      //////////////////////////////////////////////////////////////////

      // Grid-Stat configuration object
      MetConfig conf;

      // Store data parsed from the Grid-Stat configuration object
      ConcatString model;                   // Model name
      ConcatString obtype;                  // Observation type

      GridStatVxOpt * vx_opt;               // Array of vx task options [n_vx] (allocated)

      std::map<ConcatString,MaskPlane> mask_map; // Mapping of mask names to MaskPlanes

      GridWeightType grid_weight_flag;      // Grid weighting flag
      ConcatString   tmp_dir;               // Directory for temporary files
      ConcatString   output_prefix;         // String to customize output file name
      ConcatString   version;               // Config file version
#ifdef WITH_UGRID
      bool ignore_ugrid_dataset;
      ConcatString ugrid_nc;                // NetCDF for coordinate variables of unstructured grid
      ConcatString ugrid_dataset;           // UGRid dataset name (mpas, lfric etc)
      ConcatString ugrid_map_config;        // User's configuration file which contains ugrid metadata mapping
      double ugrid_max_distance_km;         // max distance to be the closest neighbor to unstructured grid
#endif

      // Summary of output file options across all verification tasks
      STATOutputType    output_flag[n_txt]; // Flag for each output line type
      GridStatNcOutInfo nc_info;            // Output NetCDF pairs file contents

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config   (const char *, const char *);
#ifdef WITH_UGRID
      void read_ugrid_configs(StringArray ugrid_config_names, const char * user_config);
#endif
      void process_config(GrdFileType, GrdFileType);
      void process_flags ();
      void process_masks (const Grid &);

      // Dump out the counts
      int  get_n_vx()              const;
      int  get_compression_level();
      bool get_output_ascii_flag() const;
      bool get_output_nc_flag()    const;

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
      int get_max_n_cov_thresh()   const;

};

////////////////////////////////////////////////////////////////////////

inline int  GridStatConfInfo::get_n_vx()              const { return(n_vx);                  }
inline int  GridStatConfInfo::get_compression_level()       { return(conf.nc_compression()); }
inline bool GridStatConfInfo::get_output_ascii_flag() const { return(output_ascii_flag);     }
inline bool GridStatConfInfo::get_output_nc_flag()    const { return(output_nc_flag);        }

////////////////////////////////////////////////////////////////////////

#endif   /*  __GRID_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
