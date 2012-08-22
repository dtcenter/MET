// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
static const int i_mpr       = 14;
static const int n_txt       = 15;

// Text file type
static const STATLineType txt_file_type[n_txt] = {
   stat_fho,    stat_ctc,    stat_cts,
   stat_mctc,   stat_mcts,   stat_cnt,
   stat_sl1l2,  stat_sal1l2, stat_vl1l2,
   stat_val1l2, stat_pct,    stat_pstd,
   stat_pjc,    stat_prc,    stat_mpr
};

////////////////////////////////////////////////////////////////////////

class PointStatConfInfo {

   private:

      void init_from_scratch();

      // Counts based on the contents of the config file
      int n_vx;          // Number of fields to be verified
      int n_vx_scal;     // Number of scalar fields to be verified
      int n_vx_vect;     // Number of vector fields to be verified
      int n_vx_prob;     // Number of probability fields to be verified
      int n_mask;        // Total number of masking regions
      int n_mask_area;   // Number of masking areas
      int n_interp;      // Number of interpolation methods

      int max_n_scal_thresh;      // Maximum number of scalar thresholds
      int max_n_prob_fcst_thresh; // Maximum fcst prob thresholds
      int max_n_prob_obs_thresh;  // Maximum obs prob thresholds

   public:

      // Point-Stat configuration object
      MetConfig conf;

      // Store data parsed from the Point-Stat configuration object
      ConcatString      model;              // Model name
      int               beg_ds;             // Begin observation time window offset
      int               end_ds;             // End observation time window offset
      VxPairDataPoint * vx_pd;              // Array pair data [n_vx]
      ThreshArray *     fcst_ta;            // Array for fcst thresholds [n_vx]
      ThreshArray *     obs_ta;             // Array for obs thresholds [n_vx]      
      ThreshArray       fcst_wind_ta;       // Wind speed fcst thresholds
      ThreshArray       obs_wind_ta;        // Wind speed obs thresholds
      StringArray *     msg_typ;            // Array of message types [n_vx]
      StringArray       mask_name;          // Masking region names [n_mask]
      DataPlane *       mask_dp;            // Array for masking regions [n_mask_area]
      StringArray       mask_sid;           // Masking station id's [n_mask_sid]
      StringArray       obs_qty;            // Observation quality flags [obs_qty]
      NumArray          ci_alpha;           // Alpha value for confidence intervals
      BootIntervalType  boot_interval;      // Bootstrap CI type
      double            boot_rep_prop;      // Bootstrap replicate proportion
      int               n_boot_rep;         // Number of bootstrap replicates
      ConcatString      boot_rng;           // GSL random number generator
      ConcatString      boot_seed;          // GSL RNG seed value
      double            interp_thresh;      // Proportion of valid data values
      InterpMthd *      interp_mthd;        // Array for interpolation methods [n_interp]
      IntArray          interp_wdth;        // Array for interpolation widths [n_interp]      
      STATOutputType    output_flag[n_txt]; // Flag for each output line type
      DuplicateType     duplicate_flag;     // Duplicate observation behavior      
      bool              rank_corr_flag;     // Flag for computing rank correlations
      ConcatString      tmp_dir;            // Directory for temporary files
      ConcatString      output_prefix;      // String to customize output file names
      ConcatString      version;            // Config file version

      PointStatConfInfo();
     ~PointStatConfInfo();

      void clear();

      void read_config   (const char *, const char *);
      void process_config(GrdFileType);
      void process_masks (const Grid &);
      void set_vx_pd     ();

      // Dump out the counts
      int get_n_vx()              const;
      int get_n_vx_scal()         const;
      int get_n_vx_vect()         const;
      int get_n_vx_prob()         const;
      int get_n_msg_typ(int i)    const;
      int get_n_mask()            const;
      int get_n_mask_area()       const;
      int get_n_mask_sid()        const;
      int get_n_wind_thresh()     const;
      int get_n_interp()          const;
      int get_n_ci_alpha()        const;
      int get_vflag()             const;
      int get_pflag()             const;

      int get_max_n_scal_thresh()      const;
      int get_max_n_prob_fcst_thresh() const;
      int get_max_n_prob_obs_thresh()  const;

      // Compute the maximum number of output lines possible based
      // on the contents of the configuration file
      int n_txt_row (int i);
      int n_stat_row();
};

////////////////////////////////////////////////////////////////////////

inline int PointStatConfInfo::get_n_vx()          const { return(n_vx);                      }
inline int PointStatConfInfo::get_n_vx_scal()     const { return(n_vx_scal);                 }
inline int PointStatConfInfo::get_n_vx_vect()     const { return(n_vx_vect);                 }
inline int PointStatConfInfo::get_n_vx_prob()     const { return(n_vx_prob);                 }
inline int PointStatConfInfo::get_n_mask()        const { return(n_mask);                    }
inline int PointStatConfInfo::get_n_mask_area()   const { return(n_mask_area);               }
inline int PointStatConfInfo::get_n_mask_sid()    const { return(mask_sid.n_elements());     }
inline int PointStatConfInfo::get_n_wind_thresh() const { return(fcst_wind_ta.n_elements()); }
inline int PointStatConfInfo::get_n_interp()      const { return(n_interp);                  }
inline int PointStatConfInfo::get_n_ci_alpha()    const { return(ci_alpha.n_elements());     }
inline int PointStatConfInfo::get_vflag()         const { return(n_vx_vect > 0);             }
inline int PointStatConfInfo::get_pflag()         const { return(n_vx_prob > 0);             }

inline int PointStatConfInfo::get_max_n_scal_thresh() const {
   return(max_n_scal_thresh);
}
inline int PointStatConfInfo::get_max_n_prob_fcst_thresh() const {
   return(max_n_prob_fcst_thresh);
}
inline int PointStatConfInfo::get_max_n_prob_obs_thresh() const {
   return(max_n_prob_obs_thresh);
}

////////////////////////////////////////////////////////////////////////

#endif   /*  __POINT_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
