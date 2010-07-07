

////////////////////////////////////////////////////////////////////////


#ifndef  __WWMCACONFIG_H__
#define  __WWMCACONFIG_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created by econfig_codegen from config file "./config/config"
   //
   //     on July 7, 2010    11:15 am  MDT
   //


////////////////////////////////////////////////////////////////////////


#include "machine.h"
#include "result.h"


////////////////////////////////////////////////////////////////////////


class WwmcaConfig {

   private:

      void init_from_scratch();

      // void assign(const WwmcaConfig &);


      WwmcaConfig(const WwmcaConfig &);
      WwmcaConfig & operator=(const WwmcaConfig &);

         //
         //  symbol table entries for variables (not allocated)
         //


      const SymbolTableEntry * _To_Grid_entry;

      const SymbolTableEntry * _interp_method_entry;

      const SymbolTableEntry * _interp_width_entry;

      const SymbolTableEntry * _good_percent_entry;

      const SymbolTableEntry * _variable_name_entry;

      const SymbolTableEntry * _grib_code_entry;

      const SymbolTableEntry * _units_entry;

      const SymbolTableEntry * _long_name_entry;

      const SymbolTableEntry * _level_entry;



         //
         //  the machine that "runs" the config file
         //


      Machine _m;


   public:

      WwmcaConfig();
     ~WwmcaConfig();

      void clear();

      void read(const char * config_filename);

      void st_dump(ostream &, int = 0) const;   //  dump machine symbol table

         //
         //  Symbol Presence
         //

      bool has_To_Grid() const;

      bool has_interp_method() const;

      bool has_interp_width() const;

      bool has_good_percent() const;

      bool has_variable_name() const;

      bool has_grib_code() const;

      bool has_units() const;

      bool has_long_name() const;

      bool has_level() const;

         //
         //  Symbol Access
         //

      Result To_Grid();

      Result interp_method();

      Result interp_width();

      Result good_percent();

      Result variable_name();

      Result grib_code();

      Result units();

      Result long_name();

      Result level();

};


////////////////////////////////////////////////////////////////////////


inline bool WwmcaConfig::has_To_Grid() const { return ( _To_Grid_entry != 0 ); };

inline bool WwmcaConfig::has_interp_method() const { return ( _interp_method_entry != 0 ); };

inline bool WwmcaConfig::has_interp_width() const { return ( _interp_width_entry != 0 ); };

inline bool WwmcaConfig::has_good_percent() const { return ( _good_percent_entry != 0 ); };

inline bool WwmcaConfig::has_variable_name() const { return ( _variable_name_entry != 0 ); };

inline bool WwmcaConfig::has_grib_code() const { return ( _grib_code_entry != 0 ); };

inline bool WwmcaConfig::has_units() const { return ( _units_entry != 0 ); };

inline bool WwmcaConfig::has_long_name() const { return ( _long_name_entry != 0 ); };

inline bool WwmcaConfig::has_level() const { return ( _level_entry != 0 ); };


////////////////////////////////////////////////////////////////////////


#endif   /*  __WWMCACONFIG_H__  */


////////////////////////////////////////////////////////////////////////


