// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "point_stat_Conf.h"

#include "vx_wrfdata.h"
#include "vx_met_util.h"
#include "vx_grib_classes.h"
#include "grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"
#include "vx_gsl_prob.h"
#include "result.h"

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
static const int n_out       = 15;

// Enumeration to store possible output flag values
enum OutputFlag {
   flag_no_out   = 0,
   flag_stat_out = 1,
   flag_txt_out  = 2
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
      int n_msg_typ;     // Number of verifying message types
      int n_mask;        // Total number of masking regions
      int n_mask_area;   // Number of masking areas
      int n_mask_sid;    // Number of masking station id's
      int n_wind_thresh; // Number of wind speed thresholds
      int n_interp;      // Number of interpolation methods
      int n_ci_alpha;    // Number of alpha values

      int max_n_scal_thresh;      // Maximum number of scalar thresholds
      int max_n_prob_fcst_thresh; // Maximum fcst prob thresholds
      int max_n_prob_obs_thresh;  // Maximum obs prob thresholds

   public:

      // Point-Stat configuration object
      point_stat_Conf conf;

      // Various objects to store the data that's parsed from the
      // Point-Stat configuration object
      GCPairData  *gc_pd;        // Array pair data [n_vx]
      ThreshArray *fcst_ta;      // Array for fcst thresholds [n_vx]
      ThreshArray *obs_ta;       // Array for obs thresholds [n_vx]
      ThreshArray  fcst_wind_ta; // Wind speed fcst thresholds
      ThreshArray  obs_wind_ta;  // Wind speed obs thresholds
      char       **msg_typ;      // Array of message types

      InterpMthd  *interp_mthd;  // Array for interpolation methods
      int         *interp_wdth;  // Array for interpolation widths
      WrfData     *mask_wd;      // Array for masking regions [n_masks]
      char       **mask_name;    // Masking region names [n_masks]

      StringArray  mask_sid;     // Masking station id's

      PointStatConfInfo();
     ~PointStatConfInfo();

      void clear();

      void read_config   (const char *, const char *, FileType);
      void process_config(FileType);
      void process_masks (const Grid &);
      void set_gc_pd     ();

      // Dump out the counts
      int get_n_vx()              const;
      int get_n_vx_scal()         const;
      int get_n_vx_vect()         const;
      int get_n_vx_prob()         const;
      int get_n_msg_typ()         const;
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

inline int PointStatConfInfo::get_n_vx()          const { return(n_vx);          }
inline int PointStatConfInfo::get_n_vx_scal()     const { return(n_vx_scal);     }
inline int PointStatConfInfo::get_n_vx_vect()     const { return(n_vx_vect);     }
inline int PointStatConfInfo::get_n_vx_prob()     const { return(n_vx_prob);     }
inline int PointStatConfInfo::get_n_msg_typ()     const { return(n_msg_typ);     }
inline int PointStatConfInfo::get_n_mask()        const { return(n_mask);        }
inline int PointStatConfInfo::get_n_mask_area()   const { return(n_mask_area);   }
inline int PointStatConfInfo::get_n_mask_sid()    const { return(n_mask_sid);    }
inline int PointStatConfInfo::get_n_wind_thresh() const { return(n_wind_thresh); }
inline int PointStatConfInfo::get_n_interp()      const { return(n_interp);      }
inline int PointStatConfInfo::get_n_ci_alpha()    const { return(n_ci_alpha);    }
inline int PointStatConfInfo::get_vflag()         const { return(n_vx_vect > 0); }
inline int PointStatConfInfo::get_pflag()         const { return(n_vx_prob > 0); }

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
