

////////////////////////////////////////////////////////////////////////


#ifndef  __PB2NC_CONF_H__
#define  __PB2NC_CONF_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "PB2NCConfig_default"
   //
   //     on March 3, 2011    2:57 pm  MST
   //


////////////////////////////////////////////////////////////////////////


#include "vx_econfig/machine.h"
#include "vx_econfig/result.h"


////////////////////////////////////////////////////////////////////////


class pb2nc_Conf {

   private:

      void init_from_scratch();

      // void assign(const pb2nc_Conf &);


      pb2nc_Conf(const pb2nc_Conf &);
      pb2nc_Conf & operator=(const pb2nc_Conf &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _message_type_entry;

      const SymbolTableEntry * _station_id_entry;

      const SymbolTableEntry * _beg_ds_entry;

      const SymbolTableEntry * _end_ds_entry;

      const SymbolTableEntry * _mask_grid_entry;

      const SymbolTableEntry * _mask_poly_entry;

      const SymbolTableEntry * _beg_elev_entry;

      const SymbolTableEntry * _end_elev_entry;

      const SymbolTableEntry * _pb_report_type_entry;

      const SymbolTableEntry * _in_report_type_entry;

      const SymbolTableEntry * _instrument_type_entry;

      const SymbolTableEntry * _beg_level_entry;

      const SymbolTableEntry * _end_level_entry;

      const SymbolTableEntry * _obs_grib_code_entry;

      const SymbolTableEntry * _quality_mark_thresh_entry;

      const SymbolTableEntry * _event_stack_flag_entry;

      const SymbolTableEntry * _level_category_entry;

      const SymbolTableEntry * _tmp_dir_entry;

      const SymbolTableEntry * _version_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      pb2nc_Conf();
     ~pb2nc_Conf();

      void clear();

      void read(const char * config_filename);

         //
         //  Symbol Access
         //

      Result message_type(int);   //  1-dimensional array, indices from 0 to 0

      int n_message_type_elements();


      Result station_id(int);   //  1-dimensional array, indices from 0 to 0

      int n_station_id_elements();


      Result beg_ds();


      Result end_ds();


      Result mask_grid();


      Result mask_poly();


      Result beg_elev();


      Result end_elev();


      Result pb_report_type(int);   //  1-dimensional array, indices from 0 to 0

      int n_pb_report_type_elements();


      Result in_report_type(int);   //  1-dimensional array, indices from 0 to 0

      int n_in_report_type_elements();


      Result instrument_type(int);   //  1-dimensional array, indices from 0 to 0

      int n_instrument_type_elements();


      Result beg_level();


      Result end_level();


      Result obs_grib_code(int);   //  1-dimensional array, indices from 0 to 5

      int n_obs_grib_code_elements();


      Result quality_mark_thresh();


      Result event_stack_flag();


      Result level_category(int);   //  1-dimensional array, indices from 0 to 0

      int n_level_category_elements();


      Result tmp_dir();


      Result version();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __PB2NC_CONF_H__  */


////////////////////////////////////////////////////////////////////////


