

////////////////////////////////////////////////////////////////////////


#ifndef  __WAVELET_STAT_CONF_H__
#define  __WAVELET_STAT_CONF_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "WaveletStatConfig_default"
   //
   //     on March 26, 2009    10:43 am  MST
   //


////////////////////////////////////////////////////////////////////////


#include "vx_econfig/machine.h"
#include "vx_econfig/result.h"


////////////////////////////////////////////////////////////////////////


class wavelet_stat_Conf {

   private:

      void init_from_scratch();

      // void assign(const wavelet_stat_Conf &);


      wavelet_stat_Conf(const wavelet_stat_Conf &);
      wavelet_stat_Conf & operator=(const wavelet_stat_Conf &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _model_entry;

      const SymbolTableEntry * _field_entry;

      const SymbolTableEntry * _thresh_entry;

      const SymbolTableEntry * _mask_missing_flag_entry;

      const SymbolTableEntry * _grid_decomp_flag_entry;

      const SymbolTableEntry * _tile_xll_entry;

      const SymbolTableEntry * _tile_yll_entry;

      const SymbolTableEntry * _tile_dim_entry;

      const SymbolTableEntry * _wavelet_flag_entry;

      const SymbolTableEntry * _wavelet_k_entry;

      const SymbolTableEntry * _output_flag_entry;

      const SymbolTableEntry * _met_data_dir_entry;

      const SymbolTableEntry * _raw_color_table_entry;

      const SymbolTableEntry * _wvlt_color_table_entry;

      const SymbolTableEntry * _raw_plot_min_entry;

      const SymbolTableEntry * _raw_plot_max_entry;

      const SymbolTableEntry * _wvlt_plot_min_entry;

      const SymbolTableEntry * _wvlt_plot_max_entry;

      const SymbolTableEntry * _grib_ptv_entry;

      const SymbolTableEntry * _output_prefix_entry;

      const SymbolTableEntry * _version_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      wavelet_stat_Conf();
     ~wavelet_stat_Conf();

      void clear();

      void read(const char * config_filename);

         //
         //  Symbol Access
         //

      Result model();


      Result field(int);   //  1-dimensional array, indices from 0 to 1

      int n_field_elements();


      Result thresh(int);   //  1-dimensional array, indices from 0 to 1

      int n_thresh_elements();


      Result mask_missing_flag();


      Result grid_decomp_flag();


      Result tile_xll(int);   //  1-dimensional array, indices from 0 to 1

      int n_tile_xll_elements();


      Result tile_yll(int);   //  1-dimensional array, indices from 0 to 1

      int n_tile_yll_elements();


      Result tile_dim();


      Result wavelet_flag();


      Result wavelet_k();


      Result output_flag(int);   //  1-dimensional array, indices from 0 to 3

      int n_output_flag_elements();


      Result met_data_dir();


      Result raw_color_table();


      Result wvlt_color_table();


      Result raw_plot_min();


      Result raw_plot_max();


      Result wvlt_plot_min();


      Result wvlt_plot_max();


      Result grib_ptv();


      Result output_prefix();


      Result version();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __WAVELET_STAT_CONF_H__  */


////////////////////////////////////////////////////////////////////////


