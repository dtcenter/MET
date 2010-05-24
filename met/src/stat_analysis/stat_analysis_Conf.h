

////////////////////////////////////////////////////////////////////////


#ifndef  __STAT_ANALYSIS_CONF_H__
#define  __STAT_ANALYSIS_CONF_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "STATAnalysisConfig_default"
   //
   //     on May 24, 2010    11:17 am  MDT
   //


////////////////////////////////////////////////////////////////////////


#include "vx_econfig/machine.h"
#include "vx_econfig/result.h"


////////////////////////////////////////////////////////////////////////


class stat_analysis_Conf {

   private:

      void init_from_scratch();

      // void assign(const stat_analysis_Conf &);


      stat_analysis_Conf(const stat_analysis_Conf &);
      stat_analysis_Conf & operator=(const stat_analysis_Conf &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _model_entry;

      const SymbolTableEntry * _fcst_lead_entry;

      const SymbolTableEntry * _obs_lead_entry;

      const SymbolTableEntry * _fcst_valid_beg_entry;

      const SymbolTableEntry * _fcst_valid_end_entry;

      const SymbolTableEntry * _obs_valid_beg_entry;

      const SymbolTableEntry * _obs_valid_end_entry;

      const SymbolTableEntry * _fcst_init_beg_entry;

      const SymbolTableEntry * _fcst_init_end_entry;

      const SymbolTableEntry * _obs_init_beg_entry;

      const SymbolTableEntry * _obs_init_end_entry;

      const SymbolTableEntry * _fcst_init_hour_entry;

      const SymbolTableEntry * _obs_init_hour_entry;

      const SymbolTableEntry * _fcst_var_entry;

      const SymbolTableEntry * _obs_var_entry;

      const SymbolTableEntry * _fcst_lev_entry;

      const SymbolTableEntry * _obs_lev_entry;

      const SymbolTableEntry * _obtype_entry;

      const SymbolTableEntry * _vx_mask_entry;

      const SymbolTableEntry * _interp_mthd_entry;

      const SymbolTableEntry * _interp_pnts_entry;

      const SymbolTableEntry * _fcst_thresh_entry;

      const SymbolTableEntry * _obs_thresh_entry;

      const SymbolTableEntry * _cov_thresh_entry;

      const SymbolTableEntry * _alpha_entry;

      const SymbolTableEntry * _line_type_entry;

      const SymbolTableEntry * _jobs_entry;

      const SymbolTableEntry * _out_alpha_entry;

      const SymbolTableEntry * _boot_interval_entry;

      const SymbolTableEntry * _boot_rep_prop_entry;

      const SymbolTableEntry * _n_boot_rep_entry;

      const SymbolTableEntry * _boot_rng_entry;

      const SymbolTableEntry * _boot_seed_entry;

      const SymbolTableEntry * _rank_corr_flag_entry;

      const SymbolTableEntry * _tmp_dir_entry;

      const SymbolTableEntry * _version_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      stat_analysis_Conf();
     ~stat_analysis_Conf();

      void clear();

      void read(const char * config_filename);

         //
         //  Symbol Access
         //

      Result model(int);   //  1-dimensional array, indices from 0 to 0

      int n_model_elements();


      Result fcst_lead(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_lead_elements();


      Result obs_lead(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_lead_elements();


      Result fcst_valid_beg();


      Result fcst_valid_end();


      Result obs_valid_beg();


      Result obs_valid_end();


      Result fcst_init_beg();


      Result fcst_init_end();


      Result obs_init_beg();


      Result obs_init_end();


      Result fcst_init_hour(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_init_hour_elements();


      Result obs_init_hour(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_init_hour_elements();


      Result fcst_var(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_var_elements();


      Result obs_var(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_var_elements();


      Result fcst_lev(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_lev_elements();


      Result obs_lev(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_lev_elements();


      Result obtype(int);   //  1-dimensional array, indices from 0 to 0

      int n_obtype_elements();


      Result vx_mask(int);   //  1-dimensional array, indices from 0 to 0

      int n_vx_mask_elements();


      Result interp_mthd(int);   //  1-dimensional array, indices from 0 to 0

      int n_interp_mthd_elements();


      Result interp_pnts(int);   //  1-dimensional array, indices from 0 to 0

      int n_interp_pnts_elements();


      Result fcst_thresh(int);   //  1-dimensional array, indices from 0 to 0

      int n_fcst_thresh_elements();


      Result obs_thresh(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_thresh_elements();


      Result cov_thresh(int);   //  1-dimensional array, indices from 0 to 0

      int n_cov_thresh_elements();


      Result alpha(int);   //  1-dimensional array, indices from 0 to 0

      int n_alpha_elements();


      Result line_type(int);   //  1-dimensional array, indices from 0 to 0

      int n_line_type_elements();


      Result jobs(int);   //  1-dimensional array, indices from 0 to 1

      int n_jobs_elements();


      Result out_alpha();


      Result boot_interval();


      Result boot_rep_prop();


      Result n_boot_rep();


      Result boot_rng();


      Result boot_seed();


      Result rank_corr_flag();


      Result tmp_dir();


      Result version();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __STAT_ANALYSIS_CONF_H__  */


////////////////////////////////////////////////////////////////////////


