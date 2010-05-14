

////////////////////////////////////////////////////////////////////////


#ifndef  __POINT_STAT_CONF_H__
#define  __POINT_STAT_CONF_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "PointStatConfig_default"
   //
   //     on May 14, 2010    1:57 pm  MDT
   //


////////////////////////////////////////////////////////////////////////


#include "vx_econfig/machine.h"
#include "vx_econfig/result.h"


////////////////////////////////////////////////////////////////////////


class point_stat_Conf {

   private:

      void init_from_scratch();

      // void assign(const point_stat_Conf &);


      point_stat_Conf(const point_stat_Conf &);
      point_stat_Conf & operator=(const point_stat_Conf &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _model_entry;

      const SymbolTableEntry * _beg_ds_entry;

      const SymbolTableEntry * _end_ds_entry;

      const SymbolTableEntry * _fcst_field_entry;

      const SymbolTableEntry * _obs_field_entry;

      const SymbolTableEntry * _fcst_thresh_entry;

      const SymbolTableEntry * _obs_thresh_entry;

      const SymbolTableEntry * _fcst_wind_thresh_entry;

      const SymbolTableEntry * _obs_wind_thresh_entry;

      const SymbolTableEntry * _message_type_entry;

      const SymbolTableEntry * _mask_grid_entry;

      const SymbolTableEntry * _mask_poly_entry;

      const SymbolTableEntry * _mask_sid_entry;

      const SymbolTableEntry * _ci_alpha_entry;

      const SymbolTableEntry * _boot_interval_entry;

      const SymbolTableEntry * _boot_rep_prop_entry;

      const SymbolTableEntry * _n_boot_rep_entry;

      const SymbolTableEntry * _boot_rng_entry;

      const SymbolTableEntry * _boot_seed_entry;

      const SymbolTableEntry * _interp_method_entry;

      const SymbolTableEntry * _interp_width_entry;

      const SymbolTableEntry * _interp_thresh_entry;

      const SymbolTableEntry * _output_flag_entry;

      const SymbolTableEntry * _rank_corr_flag_entry;

      const SymbolTableEntry * _grib_ptv_entry;

      const SymbolTableEntry * _tmp_dir_entry;

      const SymbolTableEntry * _output_prefix_entry;

      const SymbolTableEntry * _version_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      point_stat_Conf();
     ~point_stat_Conf();

      void clear();

      void read(const char * config_filename);

         //
         //  Symbol Access
         //

      Result model();


      Result beg_ds();


      Result end_ds();


      Result fcst_field(int);   //  1-dimensional array, indices from 0 to 5

      int n_fcst_field_elements();


      Result obs_field(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_field_elements();


      Result fcst_thresh(int);   //  1-dimensional array, indices from 0 to 5

      int n_fcst_thresh_elements();


      Result obs_thresh(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_thresh_elements();


      Result fcst_wind_thresh(int);   //  1-dimensional array, indices from 0 to 1

      int n_fcst_wind_thresh_elements();


      Result obs_wind_thresh(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_wind_thresh_elements();


      Result message_type(int);   //  1-dimensional array, indices from 0 to 1

      int n_message_type_elements();


      Result mask_grid(int);   //  1-dimensional array, indices from 0 to 1

      int n_mask_grid_elements();


      Result mask_poly(int);   //  1-dimensional array, indices from 0 to 0

      int n_mask_poly_elements();


      Result mask_sid();


      Result ci_alpha(int);   //  1-dimensional array, indices from 0 to 1

      int n_ci_alpha_elements();


      Result boot_interval();


      Result boot_rep_prop();


      Result n_boot_rep();


      Result boot_rng();


      Result boot_seed();


      Result interp_method(int);   //  1-dimensional array, indices from 0 to 1

      int n_interp_method_elements();


      Result interp_width(int);   //  1-dimensional array, indices from 0 to 1

      int n_interp_width_elements();


      Result interp_thresh();


      Result output_flag(int);   //  1-dimensional array, indices from 0 to 13

      int n_output_flag_elements();


      Result rank_corr_flag();


      Result grib_ptv();


      Result tmp_dir();


      Result output_prefix();


      Result version();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __POINT_STAT_CONF_H__  */


////////////////////////////////////////////////////////////////////////


