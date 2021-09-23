// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __GEN_ENS_PROD_CONF_INFO_H__
#define  __GEN_ENS_PROD_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"

////////////////////////////////////////////////////////////////////////

struct GenEnsProdNcOutInfo {

   bool do_latlon;
   bool do_mean;
   bool do_stdev;
   bool do_minus;
   bool do_plus;
   bool do_min;
   bool do_max;
   bool do_range;
   bool do_vld;
   bool do_freq;
   bool do_nep;
   bool do_nmep;

   GenEnsProdNcOutInfo();

   void clear();

   bool all_false() const;
   bool need_cat_thresh() const;

   void set_all_false();
   void set_all_true();
};

////////////////////////////////////////////////////////////////////////

class GenEnsProdConfInfo {

   private:

      void init_from_scratch();

      // Ensemble processing
      int n_ens_var;    // Number of ensemble fields to be processed
      int max_n_cat_ta; // Maximum number of ensemble thresholds
      int n_nbrhd;      // Number of neighborhood sizes

   public:

      GenEnsProdConfInfo();
     ~GenEnsProdConfInfo();

      //////////////////////////////////////////////////////////////////

      // Gen-Ens-Prod configuration object
      MetConfig conf;

      // Data parsed from the Gen-Ens-Prod configuration object
      ConcatString         model;           // Model name
      ConcatString         desc;            // Description

      vector<VarInfo *>    ens_info;        // Array of VarInfo pointers (allocated)
      vector<ClimoCDFInfo> cdf_info;        // Array of climo CDF info objects
      vector<ThreshArray>  ens_cat_ta;      // Array for ensemble categorical thresholds
      StringArray          ens_var_str;     // Array of ensemble variable name strings
      vector<GenEnsProdNcOutInfo> nc_info;  // Array of ensemble product outputs

      NbrhdInfo            nbrhd_prob;      // Neighborhood probability definition
      InterpInfo           nmep_smooth;     // Neighborhood maximum smoothing information

      double               vld_ens_thresh;  // Required ratio of valid input files
      double               vld_data_thresh; // Required ratio of valid data for each point

      gsl_rng *            rng_ptr;         // GSL random number generator (allocated)

      ConcatString         tmp_dir;         // Directory for temporary files
      ConcatString         version;         // Config file version

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config   (const ConcatString, const ConcatString);
      void process_config(GrdFileType);

      GenEnsProdNcOutInfo parse_nc_info(Dictionary *);

      // Accessor functions
      int get_n_ens_var()         const;
      int get_max_n_cat_ta()      const;
      int get_n_nbrhd()           const;
      int get_compression_level();
};

////////////////////////////////////////////////////////////////////////

inline int GenEnsProdConfInfo::get_n_ens_var()         const { return(n_ens_var);             }
inline int GenEnsProdConfInfo::get_max_n_cat_ta()      const { return(max_n_cat_ta);          }
inline int GenEnsProdConfInfo::get_n_nbrhd()           const { return(n_nbrhd);               }
inline int GenEnsProdConfInfo::get_compression_level()       { return(conf.nc_compression()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __GEN_ENS_PROD_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
