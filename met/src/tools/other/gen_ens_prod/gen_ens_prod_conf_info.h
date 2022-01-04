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
   bool do_climo;
   bool do_climo_cdp;

   GenEnsProdNcOutInfo();

   void clear();

   bool all_false() const;

   void set_all_false();
   void set_all_true();
};


////////////////////////////////////////////////////////////////////////

struct InputInfo {
    VarInfo * var_info;         // Variable information to read
    int file_index;             // Index in file_list of file to read
    StringArray * file_list;    // Array of files (unallocated)
};

////////////////////////////////////////////////////////////////////////

class EnsVarInfo {

   private:
      vector<InputInfo> inputs;     // Vector of InputInfo
      VarInfo * ctrl_info;          // Field info for control member
   public:
      EnsVarInfo();
      ~EnsVarInfo();

      void add_input(InputInfo);
      int inputs_n();

      void set_ctrl(VarInfo *);
      VarInfo * get_ctrl(int);

      // Get VarInfo from first InputInfo if requested VarInfo is NULL
      VarInfo * get_var_info(int index=0);
      ConcatString get_file(int index=0);
      int get_file_index(int index=0);

      ConcatString nc_var_str;      // Ensemble variable name strings
      ThreshArray cat_ta;           // Ensemble categorical thresholds
      GenEnsProdNcOutInfo nc_info;  // Ensemble product outputs
      ConcatString raw_magic_str;   // Magic string w/o var substitution
};

////////////////////////////////////////////////////////////////////////

class GenEnsProdConfInfo {

   private:

      void init_from_scratch();

      // Ensemble processing
      int n_var;     // Number of ensemble fields to be processed
      int max_n_cat; // Maximum number of ensemble thresholds

   public:

      GenEnsProdConfInfo();
     ~GenEnsProdConfInfo();

      //////////////////////////////////////////////////////////////////

      // Gen-Ens-Prod configuration object
      MetConfig conf;

      // Data parsed from the Gen-Ens-Prod configuration object
      ConcatString         model;           // Model name
      ConcatString         desc;            // Description
      ConcatString         control_id;      // Control ID

      vector<EnsVarInfo *> ens_input;       // Vector of EnsVarInfo pointers (allocated)
      vector<ClimoCDFInfo> cdf_info;        // Array of climo CDF info objects
      StringArray          ens_member_ids;  // Array of ensemble member ID strings

      NbrhdInfo            nbrhd_prob;      // Neighborhood probability definition
      InterpInfo           nmep_smooth;     // Neighborhood maximum smoothing information

      double               vld_ens_thresh;  // Required ratio of valid input files
      double               vld_data_thresh; // Required ratio of valid data for each point

      ConcatString         version;         // Config file version

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config   (const ConcatString, const ConcatString);
      void process_config(GrdFileType, StringArray *);

      GenEnsProdNcOutInfo parse_nc_info(Dictionary *);

      // Accessor functions
      int get_n_var()     const;
      int get_max_n_cat() const;
      int get_n_nbrhd()   const;
      int get_compression_level();
};

////////////////////////////////////////////////////////////////////////

inline int GenEnsProdConfInfo::get_n_var()       const { return(n_var);                 }
inline int GenEnsProdConfInfo::get_max_n_cat()   const { return(max_n_cat);             }
inline int GenEnsProdConfInfo::get_n_nbrhd()     const { return(nbrhd_prob.width.n());  }
inline int GenEnsProdConfInfo::get_compression_level() { return(conf.nc_compression()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __GEN_ENS_PROD_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
