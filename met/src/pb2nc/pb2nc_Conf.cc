

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


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "pb2nc_Conf.h"
#include "vx_econfig/icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


static const int Panic = 1;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class pb2nc_Conf
   //


////////////////////////////////////////////////////////////////////////


pb2nc_Conf::pb2nc_Conf()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


pb2nc_Conf::~pb2nc_Conf()

{

clear();

}


////////////////////////////////////////////////////////////////////////


pb2nc_Conf::pb2nc_Conf(const pb2nc_Conf &)

{

cerr << "\n\n  pb2nc_Conf::pb2nc_Conf(const pb2nc_Conf &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


pb2nc_Conf & pb2nc_Conf::operator=(const pb2nc_Conf &)

{

cerr << "\n\n  pb2nc_Conf::operator=(const pb2nc_Conf &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void pb2nc_Conf::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void pb2nc_Conf::clear()

{

       _message_type_entry = (const SymbolTableEntry *) 0;

         _station_id_entry = (const SymbolTableEntry *) 0;

             _beg_ds_entry = (const SymbolTableEntry *) 0;

             _end_ds_entry = (const SymbolTableEntry *) 0;

          _mask_grid_entry = (const SymbolTableEntry *) 0;

          _mask_poly_entry = (const SymbolTableEntry *) 0;

           _beg_elev_entry = (const SymbolTableEntry *) 0;

           _end_elev_entry = (const SymbolTableEntry *) 0;

     _pb_report_type_entry = (const SymbolTableEntry *) 0;

     _in_report_type_entry = (const SymbolTableEntry *) 0;

    _instrument_type_entry = (const SymbolTableEntry *) 0;

          _beg_level_entry = (const SymbolTableEntry *) 0;

          _end_level_entry = (const SymbolTableEntry *) 0;

      _obs_grib_code_entry = (const SymbolTableEntry *) 0;

_quality_mark_thresh_entry = (const SymbolTableEntry *) 0;

   _event_stack_flag_entry = (const SymbolTableEntry *) 0;

     _level_category_entry = (const SymbolTableEntry *) 0;

            _tmp_dir_entry = (const SymbolTableEntry *) 0;

            _version_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void pb2nc_Conf::read(const char * _config_filename)

{

const SymbolTableEntry * _e = (const SymbolTableEntry *) 0;

   //
   //  read the config file into the machine
   //

_m.read(_config_filename);

   //
   //  lookup the entries in the symbol table
   //

_e = _m.find("message_type");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"message_type\"\n\n";

   exit ( 1 );

}

_message_type_entry = _e;


_e = _m.find("station_id");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"station_id\"\n\n";

   exit ( 1 );

}

_station_id_entry = _e;


_e = _m.find("beg_ds");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"beg_ds\"\n\n";

   exit ( 1 );

}

_beg_ds_entry = _e;


_e = _m.find("end_ds");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"end_ds\"\n\n";

   exit ( 1 );

}

_end_ds_entry = _e;


_e = _m.find("mask_grid");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_grid\"\n\n";

   exit ( 1 );

}

_mask_grid_entry = _e;


_e = _m.find("mask_poly");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_poly\"\n\n";

   exit ( 1 );

}

_mask_poly_entry = _e;


_e = _m.find("beg_elev");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"beg_elev\"\n\n";

   exit ( 1 );

}

_beg_elev_entry = _e;


_e = _m.find("end_elev");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"end_elev\"\n\n";

   exit ( 1 );

}

_end_elev_entry = _e;


_e = _m.find("pb_report_type");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"pb_report_type\"\n\n";

   exit ( 1 );

}

_pb_report_type_entry = _e;


_e = _m.find("in_report_type");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"in_report_type\"\n\n";

   exit ( 1 );

}

_in_report_type_entry = _e;


_e = _m.find("instrument_type");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"instrument_type\"\n\n";

   exit ( 1 );

}

_instrument_type_entry = _e;


_e = _m.find("beg_level");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"beg_level\"\n\n";

   exit ( 1 );

}

_beg_level_entry = _e;


_e = _m.find("end_level");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"end_level\"\n\n";

   exit ( 1 );

}

_end_level_entry = _e;


_e = _m.find("obs_grib_code");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_grib_code\"\n\n";

   exit ( 1 );

}

_obs_grib_code_entry = _e;


_e = _m.find("quality_mark_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"quality_mark_thresh\"\n\n";

   exit ( 1 );

}

_quality_mark_thresh_entry = _e;


_e = _m.find("event_stack_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"event_stack_flag\"\n\n";

   exit ( 1 );

}

_event_stack_flag_entry = _e;


_e = _m.find("level_category");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"level_category\"\n\n";

   exit ( 1 );

}

_level_category_entry = _e;


_e = _m.find("tmp_dir");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"tmp_dir\"\n\n";

   exit ( 1 );

}

_tmp_dir_entry = _e;


_e = _m.find("version");

if ( !_e && Panic )  {

   cerr << "\n\n  pb2nc_Conf::read(const char *) -> can't get symbol table entry for variable \"version\"\n\n";

   exit ( 1 );

}

_version_entry = _e;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::message_type(int _i0)

{

Result _temp_result;

if ( !_message_type_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _message_type_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int pb2nc_Conf::n_message_type_elements()

{

if ( !_message_type_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _message_type_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::station_id(int _i0)

{

Result _temp_result;

if ( !_station_id_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _station_id_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int pb2nc_Conf::n_station_id_elements()

{

if ( !_station_id_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _station_id_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::beg_ds()

{

Result _temp_result;

if ( !_beg_ds_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_beg_ds_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::end_ds()

{

Result _temp_result;

if ( !_end_ds_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_end_ds_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::mask_grid()

{

Result _temp_result;

if ( !_mask_grid_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_grid_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::mask_poly()

{

Result _temp_result;

if ( !_mask_poly_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_poly_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::beg_elev()

{

Result _temp_result;

if ( !_beg_elev_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_beg_elev_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::end_elev()

{

Result _temp_result;

if ( !_end_elev_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_end_elev_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::pb_report_type(int _i0)

{

Result _temp_result;

if ( !_pb_report_type_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _pb_report_type_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int pb2nc_Conf::n_pb_report_type_elements()

{

if ( !_pb_report_type_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _pb_report_type_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::in_report_type(int _i0)

{

Result _temp_result;

if ( !_in_report_type_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _in_report_type_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int pb2nc_Conf::n_in_report_type_elements()

{

if ( !_in_report_type_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _in_report_type_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::instrument_type(int _i0)

{

Result _temp_result;

if ( !_instrument_type_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _instrument_type_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int pb2nc_Conf::n_instrument_type_elements()

{

if ( !_instrument_type_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _instrument_type_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::beg_level()

{

Result _temp_result;

if ( !_beg_level_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_beg_level_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::end_level()

{

Result _temp_result;

if ( !_end_level_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_end_level_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::obs_grib_code(int _i0)

{

Result _temp_result;

if ( !_obs_grib_code_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_grib_code_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int pb2nc_Conf::n_obs_grib_code_elements()

{

if ( !_obs_grib_code_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_grib_code_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::quality_mark_thresh()

{

Result _temp_result;

if ( !_quality_mark_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_quality_mark_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::event_stack_flag()

{

Result _temp_result;

if ( !_event_stack_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_event_stack_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::level_category(int _i0)

{

Result _temp_result;

if ( !_level_category_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _level_category_entry->ai;


   //
   //  load up the indices
   //

_indices[0] = _i0;


_v = _a->get(_indices);


_m.run( *_v );


_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);

   //
   //  done
   //

return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


int pb2nc_Conf::n_level_category_elements()

{

if ( !_level_category_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _level_category_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::tmp_dir()

{

Result _temp_result;

if ( !_tmp_dir_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_tmp_dir_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result pb2nc_Conf::version()

{

Result _temp_result;

if ( !_version_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_version_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


