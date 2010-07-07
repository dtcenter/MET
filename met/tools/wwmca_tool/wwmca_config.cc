

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


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "wwmca_config.h"
#include "icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class WwmcaConfig
   //


////////////////////////////////////////////////////////////////////////


WwmcaConfig::WwmcaConfig()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


WwmcaConfig::~WwmcaConfig()

{

clear();

}


////////////////////////////////////////////////////////////////////////


WwmcaConfig::WwmcaConfig(const WwmcaConfig &)

{

cerr << "\n\n  WwmcaConfig::WwmcaConfig(const WwmcaConfig &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


WwmcaConfig & WwmcaConfig::operator=(const WwmcaConfig &)

{

cerr << "\n\n  WwmcaConfig::operator=(const WwmcaConfig &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void WwmcaConfig::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaConfig::clear()

{

      _To_Grid_entry = (const SymbolTableEntry *) 0;

_interp_method_entry = (const SymbolTableEntry *) 0;

 _interp_width_entry = (const SymbolTableEntry *) 0;

 _good_percent_entry = (const SymbolTableEntry *) 0;

_variable_name_entry = (const SymbolTableEntry *) 0;

    _grib_code_entry = (const SymbolTableEntry *) 0;

        _units_entry = (const SymbolTableEntry *) 0;

    _long_name_entry = (const SymbolTableEntry *) 0;

        _level_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaConfig::read(const char * _config_filename)

{

clear();


   //
   //  read the config file into the machine
   //

_m.read(_config_filename);

   //
   //  lookup the entries in the symbol table
   //

_To_Grid_entry = _m.find("To_Grid");

_interp_method_entry = _m.find("interp_method");

_interp_width_entry = _m.find("interp_width");

_good_percent_entry = _m.find("good_percent");

_variable_name_entry = _m.find("variable_name");

_grib_code_entry = _m.find("grib_code");

_units_entry = _m.find("units");

_long_name_entry = _m.find("long_name");

_level_entry = _m.find("level");


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaConfig::st_dump(ostream & _out, int _depth) const

{

_m.st_dump(_out, _depth);

return;

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::To_Grid()

{

if ( !_To_Grid_entry )  {

   cerr << "\n\n   WwmcaConfig::To_Grid() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_To_Grid_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::interp_method()

{

if ( !_interp_method_entry )  {

   cerr << "\n\n   WwmcaConfig::interp_method() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_interp_method_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::interp_width()

{

if ( !_interp_width_entry )  {

   cerr << "\n\n   WwmcaConfig::interp_width() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_interp_width_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::good_percent()

{

if ( !_good_percent_entry )  {

   cerr << "\n\n   WwmcaConfig::good_percent() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_good_percent_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::variable_name()

{

if ( !_variable_name_entry )  {

   cerr << "\n\n   WwmcaConfig::variable_name() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_variable_name_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::grib_code()

{

if ( !_grib_code_entry )  {

   cerr << "\n\n   WwmcaConfig::grib_code() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_grib_code_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::units()

{

if ( !_units_entry )  {

   cerr << "\n\n   WwmcaConfig::units() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_units_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::long_name()

{

if ( !_long_name_entry )  {

   cerr << "\n\n   WwmcaConfig::long_name() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_long_name_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result WwmcaConfig::level()

{

if ( !_level_entry )  {

   cerr << "\n\n   WwmcaConfig::level() -> no symbol table entry found!\n\n";

   exit ( 1 );

}


Result _temp_result;
IcodeCell _cell;

_m.run( *_level_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


