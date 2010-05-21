

////////////////////////////////////////////////////////////////////////


#ifndef  __ENSEMBLE_STAT_CONF_H__
#define  __ENSEMBLE_STAT_CONF_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "EnsembleStatConfig_default"
   //
   //     on May 21, 2010    2:39 pm  MDT
   //


////////////////////////////////////////////////////////////////////////


#include "vx_econfig/machine.h"
#include "vx_econfig/result.h"


////////////////////////////////////////////////////////////////////////


class ensemble_stat_Conf {

   private:

      void init_from_scratch();

      // void assign(const ensemble_stat_Conf &);


      ensemble_stat_Conf(const ensemble_stat_Conf &);
      ensemble_stat_Conf & operator=(const ensemble_stat_Conf &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _model_entry;

      const SymbolTableEntry * _ens_field_entry;

      const SymbolTableEntry * _ens_thresh_entry;

      const SymbolTableEntry * _vld_ens_thresh_entry;

      const SymbolTableEntry * _vld_data_thresh_entry;

      const SymbolTableEntry * _fcst_field_entry;

      const SymbolTableEntry * _obs_field_entry;

      const SymbolTableEntry * _beg_ds_entry;

      const SymbolTableEntry * _end_ds_entry;

      const SymbolTableEntry * _message_type_entry;

      const SymbolTableEntry * _mask_grid_entry;

      const SymbolTableEntry * _mask_poly_entry;

      const SymbolTableEntry * _mask_sid_entry;

      const SymbolTableEntry * _interp_method_entry;

      const SymbolTableEntry * _interp_width_entry;

      const SymbolTableEntry * _interp_flag_entry;

      const SymbolTableEntry * _interp_thresh_entry;

      const SymbolTableEntry * _output_flag_entry;

      const SymbolTableEntry * _rng_type_entry;

      const SymbolTableEntry * _rng_seed_entry;

      const SymbolTableEntry * _grib_ptv_entry;

      const SymbolTableEntry * _output_prefix_entry;

      const SymbolTableEntry * _version_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      ensemble_stat_Conf();
     ~ensemble_stat_Conf();

      void clear();

      void read(const char * config_filename);

         //
         //  Symbol Access
         //

      Result model();


      Result ens_field(int);   //  1-dimensional array, indices from 0 to 1

      int n_ens_field_elements();


      Result ens_thresh(int);   //  1-dimensional array, indices from 0 to 1

      int n_ens_thresh_elements();


      Result vld_ens_thresh();


      Result vld_data_thresh();


      Result fcst_field(int);   //  1-dimensional array, indices from 0 to 1

      int n_fcst_field_elements();


      Result obs_field(int);   //  1-dimensional array, indices from 0 to 0

      int n_obs_field_elements();


      Result beg_ds();


      Result end_ds();


      Result message_type(int);   //  1-dimensional array, indices from 0 to 1

      int n_message_type_elements();


      Result mask_grid(int);   //  1-dimensional array, indices from 0 to 1

      int n_mask_grid_elements();


      Result mask_poly(int);   //  1-dimensional array, indices from 0 to 0

      int n_mask_poly_elements();


      Result mask_sid();


      Result interp_method(int);   //  1-dimensional array, indices from 0 to 1

      int n_interp_method_elements();


      Result interp_width(int);   //  1-dimensional array, indices from 0 to 1

      int n_interp_width_elements();


      Result interp_flag();


      Result interp_thresh();


      Result output_flag(int);   //  1-dimensional array, indices from 0 to 12

      int n_output_flag_elements();


      Result rng_type();


      Result rng_seed();


      Result grib_ptv();


      Result output_prefix();


      Result version();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __ENSEMBLE_STAT_CONF_H__  */


////////////////////////////////////////////////////////////////////////


