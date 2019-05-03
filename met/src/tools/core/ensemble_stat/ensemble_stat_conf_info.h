// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
static const int i_rhist    = 0;
static const int i_phist    = 1;
static const int i_orank    = 2;
static const int i_ssvar    = 3;
static const int i_relp     = 4;
static const int n_txt      = 5;

// Text file type
static const STATLineType txt_file_type[n_txt] = {
   stat_rhist, stat_phist, stat_orank, stat_ssvar, stat_relp
};

// Indices for the ensemble flag types in the configuration file
static const int i_nc_mean   = 0;
static const int i_nc_stdev  = 1;
static const int i_nc_minus  = 2;
static const int i_nc_plus   = 3;
static const int i_nc_min    = 4;
static const int i_nc_max    = 5;
static const int i_nc_range  = 6;
static const int i_nc_vld    = 7;
static const int i_nc_freq   = 8;
static const int i_nc_orank  = 9;
static const int i_nc_weight = 10;
static const int n_nc        = 11;

////////////////////////////////////////////////////////////////////////

class EnsembleStatConfInfo {

   private:

      void init_from_scratch();

      // Ensemble processing
      int n_ens_var;    // Number of ensemble fields to be processed
      int max_n_thresh; // Maximum number of ensemble thresholds

      // Ensemble verification
      int n_vx;         // Number of ensemble fields to be verified
      int n_interp;     // Number of interpolation methods
      int n_mask;       // Number of masking regions
      int n_mask_area;  // Number of masking areas
      int n_mask_sid;   // Number of masking station ID lists

   public:

      // Ensemble-Stat configuration object
      MetConfig conf;

      // Store data parsed from the Ensemble-Stat configuration object
      ConcatString         model;               // Model name
      ConcatString         desc;                // Description
      ConcatString         obtype;              // Observation type
      RegridInfo           regrid_info;         // Regridding information
      int                  beg_ds;              // Begin observation time window offset
      int                  end_ds;              // End observation time window offset
      VarInfo **           ens_info;            // Array of pointers for ensemble [n_ens_var]
      ThreshArray *        ens_ta;              // Array for ensemble thresholds [n_ens_var]
      double               vld_ens_thresh;      // Minimum valid input file ratio
      double               vld_data_thresh;     // Minimum valid data ratio for each point
      VxPairDataEnsemble * vx_pd;               // Array for ensemble pair data [n_vx]
      StringArray *        msg_typ;             // Array of message types [n_vx]
      StringArray *        sid_exc;             // Array of station ID's to exclude [n_vx]
      StringArray *        obs_qty;             // Observation quality flags for filtering [n_vx]
      ThreshArray *        othr_ta;             // Arrays of observation filetering thresholds [n_vx]
      StringArray          mask_name;           // Masking region names [n_mask]
      ConcatString         ens_ssvar_file;      // Ensemble mean file name
      DataPlane *          mask_dp;             // Array for masking regions [n_mask_area]
      StringArray *        mask_sid;            // Masking station id's [n_mask_sid]
      NumArray             ci_alpha;            // Alpha value for confidence intervals
      FieldType            interp_field;        // How to apply interpolation options
      double               interp_thresh;       // Proportion of valid data values
      InterpMthd *         interp_mthd;         // Array for interpolation methods [n_interp]
      IntArray             interp_wdth;         // Array for interpolation widths [n_interp]
      GridTemplateFactory::GridTemplates interp_shape;  //Shape for interpolation

      STATOutputType       output_flag[n_txt];  // Flag for each output line type
      bool                 ensemble_flag[n_nc]; // Boolean for each ensemble field type
      ConcatString         rng_type;            // GSL random number generator
      ConcatString         rng_seed;            // GSL RNG seed value
      GridWeightType       grid_weight_flag;    // Grid weighting flag
      ConcatString         output_prefix;       // String to customize output file names
      ConcatString         version;             // Config file version

      bool                 ens_ssvar_flag;      // Indicator for calculation of ensemble spread/skill
      ConcatString         ens_ssvar_mean;      // Ensemble mean for spread/skill calculations

      NumArray             phist_bin_size;      // PHIST bin sizes

      EnsembleStatConfInfo();
     ~EnsembleStatConfInfo();

      void clear();

      void read_config   (const char *, const char *);
      void process_config(GrdFileType, GrdFileType, bool, bool);
      void process_masks (const Grid &);
      void set_vx_pd     (const IntArray &);

      // Dump out the counts
      int get_n_ens_var()      const;
      int get_max_n_thresh()   const;
      int get_n_vx()           const;
      int get_n_msg_typ(int i) const;
      int get_n_othr(int i)    const;
      int get_n_interp()       const;
      int get_n_mask()         const;
      int get_n_mask_area()    const;
      int get_n_mask_sid()     const;

      // Compute the maximum number of output lines possible based
      // on the contents of the configuration file
      int n_txt_row(int i);
      int n_stat_row();
      int get_compression_level();
};

////////////////////////////////////////////////////////////////////////

inline int EnsembleStatConfInfo::get_n_ens_var()      const { return(n_ens_var);    }
inline int EnsembleStatConfInfo::get_max_n_thresh()   const { return(max_n_thresh); }
inline int EnsembleStatConfInfo::get_n_vx()           const { return(n_vx);         }
inline int EnsembleStatConfInfo::get_n_interp()       const { return(n_interp);     }
inline int EnsembleStatConfInfo::get_n_mask()         const { return(n_mask);       }
inline int EnsembleStatConfInfo::get_n_mask_area()    const { return(n_mask_area);  }
inline int EnsembleStatConfInfo::get_n_mask_sid()     const { return(n_mask_sid);   }
inline int EnsembleStatConfInfo::get_compression_level()    { return conf.nc_compression(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __ENSEMBLE_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
