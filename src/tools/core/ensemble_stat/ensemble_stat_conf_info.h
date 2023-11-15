// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __ENSEMBLE_STAT_CONF_INFO_H__
#define  __ENSEMBLE_STAT_CONF_INFO_H__

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

////////////////////////////////////////////////////////////////////////

// Indices for the output flag types in the configuration file
static const int i_ecnt     = 0;
static const int i_rps      = 1;
static const int i_rhist    = 2;
static const int i_phist    = 3;
static const int i_orank    = 4;
static const int i_ssvar    = 5;
static const int i_relp     = 6;
static const int i_pct      = 7;
static const int i_pstd     = 8;
static const int i_pjc      = 9;
static const int i_prc      = 10;
static const int i_eclv     = 11;
static const int n_txt      = 12;

// Text file type
static const STATLineType txt_file_type[n_txt] = {
   stat_ecnt,  stat_rps,   stat_rhist, stat_phist,
   stat_orank, stat_ssvar, stat_relp,  stat_pct,
   stat_pstd,  stat_pjc,   stat_prc,   stat_eclv
};

////////////////////////////////////////////////////////////////////////

struct EnsembleStatNcOutInfo {

   bool do_latlon;
   bool do_mean;
   bool do_raw;
   bool do_rank;
   bool do_pit;
   bool do_vld;
   bool do_weight;

      //////////////////////////////////////////////////////////////////

   EnsembleStatNcOutInfo();

   void clear();   //  sets everything to true

   bool all_false() const;

   void set_all_false();
   void set_all_true();
};

////////////////////////////////////////////////////////////////////////

class EnsembleStatConfInfo; // forward reference

////////////////////////////////////////////////////////////////////////

class EnsembleStatVxOpt {

   private:

      void init_from_scratch();

   public:

      EnsembleStatVxOpt();
     ~EnsembleStatVxOpt();

      //////////////////////////////////////////////////////////////////

      VxPairDataEnsemble vx_pd;          // Ensemble pair data

      ConcatString   var_str;            // nc_pairs_var_str string
      ConcatString   control_id;         // Control ID

      int            beg_ds;             // Begin observation time window offset
      int            end_ds;             // End observation time window offset

      StringArray    mask_grid;          // Masking grid strings
      StringArray    mask_poly;          // Masking polyline strings
      StringArray    mask_sid;           // Masking station ID's

      // Vector of MaskLatLon objects defining Lat/Lon Point masks
      std::vector<MaskLatLon> mask_llpnt;

      StringArray    mask_name;          // Masking region names
      StringArray    mask_name_area;     // Masking area (grid + poly) region names

      StringArray    msg_typ;            // Verifying message types

      ThreshArray    othr_ta;            // Observation filtering thresholds

      NumArray       eclv_points;        // ECLV points

      ClimoCDFInfo   cdf_info;           // Climo CDF info

      NumArray       ci_alpha;           // Alpha value for confidence intervals

      InterpInfo     interp_info;        // Interpolation (smoothing) information

      double         ssvar_bin_size;     // SSVAR bin size
      double         phist_bin_size;     // PHIST bin size

      ThreshArray    fcat_ta;            // Forecast categorical probability-definition thresholds, including RPS
      ThreshArray    ocat_ta;            // Observation categorical event-definition thresholds
      ThreshArray    fpct_ta;            // Forecast PCT thresholds

      DuplicateType  duplicate_flag;     // Duplicate observations
      ObsSummary     obs_summary;        // Summarize observations
      int            obs_perc;           // Summary percentile value
      bool           skip_const;         // Skip points with constant data values
      ObsErrorInfo   obs_error;          // Observation error handling

      // Output file options
      STATOutputType output_flag[n_txt]; // Flag for each output line type
      EnsembleStatNcOutInfo nc_info;     // Output NetCDF pairs file contents

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(GrdFileType, Dictionary &,
                          GrdFileType, Dictionary &,
                          gsl_rng *, bool, bool,
                          StringArray, StringArray *,
                          bool, ConcatString);
      void parse_nc_info(Dictionary &);
      void set_vx_pd(EnsembleStatConfInfo *, int);

      void set_perc_thresh(const PairDataEnsemble *);

      // Compute the number of output lines for this task
      int n_txt_row(int i) const;

      int get_n_msg_typ()   const;
      int get_n_interp()    const;
      int get_n_mask()      const;
      int get_n_mask_area() const;

      int get_n_obs_thresh()      const;
      int get_n_prob_cat_thresh() const;
      int get_n_prob_pct_thresh() const;

      int get_n_eclv_points() const;
      int get_n_cdf_bin()     const;
      int get_n_ci_alpha()    const;
};

////////////////////////////////////////////////////////////////////////

inline int EnsembleStatVxOpt::get_n_msg_typ()     const { return(msg_typ.n());          }
inline int EnsembleStatVxOpt::get_n_interp()      const { return(interp_info.n_interp); }
inline int EnsembleStatVxOpt::get_n_mask()        const { return(mask_name.n());        }
inline int EnsembleStatVxOpt::get_n_mask_area()   const { return(mask_name_area.n());   }

inline int EnsembleStatVxOpt::get_n_obs_thresh()      const { return(othr_ta.n());      }
inline int EnsembleStatVxOpt::get_n_prob_pct_thresh() const { return(fpct_ta.n());      }

inline int EnsembleStatVxOpt::get_n_eclv_points() const { return(eclv_points.n());      }
inline int EnsembleStatVxOpt::get_n_cdf_bin()     const { return(cdf_info.n_bin);       }
inline int EnsembleStatVxOpt::get_n_ci_alpha()    const { return(ci_alpha.n());         }

////////////////////////////////////////////////////////////////////////

class EnsembleStatConfInfo {

   private:

      void init_from_scratch();

      // Ensemble verification
      int n_vx;              // Number of ensemble fields to be verified
      int max_hira_size;     // Maximum size of a HiRA neighborhoods

   public:

      EnsembleStatConfInfo();
     ~EnsembleStatConfInfo();

      //////////////////////////////////////////////////////////////////

      // Ensemble-Stat configuration object
      MetConfig conf;

      // Store data parsed from the Ensemble-Stat configuration object
      ConcatString         model;           // Model name
      ConcatString         obtype;          // Observation type

      StringArray          ens_member_ids;  // Array of ensemble member ID strings
      ConcatString         control_id;      // Control ID

      EnsembleStatVxOpt  * vx_opt;          // Array of vx task options [n_vx] (allocated)
      bool                 grib_codes_set;

      double               vld_ens_thresh;  // Required ratio of valid input files
      double               vld_data_thresh; // Required ratio of valid data for each point

      // Message type groups that should be processed together
      std::map<ConcatString,StringArray> msg_typ_group_map;
      StringArray                        msg_typ_sfc;

      // Mapping of mask names to MaskPlanes
      std::map<ConcatString,MaskPlane>   mask_area_map;

      // Mapping of mask names to Station ID lists
      std::map<ConcatString,StringArray> mask_sid_map;

      gsl_rng *rng_ptr;                     // GSL random number generator (allocated)

      GridWeightType grid_weight_flag;      // Grid weighting flag
      ConcatString   tmp_dir;               // Directory for temporary files
      ConcatString   output_prefix;         // String to customize output file name
      ConcatString   version;               // Config file version

      STATOutputType output_flag[n_txt];    // Summary of output_flag options
      EnsembleStatNcOutInfo nc_info;        // Summary of output NetCDF file contents

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config   (const ConcatString , const ConcatString);
      void process_config(GrdFileType, GrdFileType, bool, bool,
                          StringArray *, bool);
      void process_grib_codes();
      void process_flags ();
      void process_masks (const Grid &);
      void set_vx_pd     (const IntArray &, int);

      // Dump out the counts
      int get_n_vx() const;

      // Compute the maximum number of output lines possible based
      // on the contents of the configuration file
      int n_txt_row(int i) const;
      int n_stat_row()     const;

      // Maximum across all verification tasks
      int get_max_hira_size()         const;
      int get_max_n_prob_cat_thresh() const;
      int get_max_n_prob_pct_thresh() const;
      int get_max_n_eclv_points()     const;

      int get_compression_level();
};

////////////////////////////////////////////////////////////////////////

inline int EnsembleStatConfInfo::get_n_vx()             const { return n_vx;                  }
inline int EnsembleStatConfInfo::get_max_hira_size()    const { return max_hira_size;         }
inline int EnsembleStatConfInfo::get_compression_level()      { return conf.nc_compression(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __ENSEMBLE_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
