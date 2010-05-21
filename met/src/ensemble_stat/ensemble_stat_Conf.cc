

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


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "ensemble_stat_Conf.h"
#include "vx_econfig/icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


static const int Panic = 1;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ensemble_stat_Conf
   //


////////////////////////////////////////////////////////////////////////


ensemble_stat_Conf::ensemble_stat_Conf()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ensemble_stat_Conf::~ensemble_stat_Conf()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ensemble_stat_Conf::ensemble_stat_Conf(const ensemble_stat_Conf &)

{

cerr << "\n\n  ensemble_stat_Conf::ensemble_stat_Conf(const ensemble_stat_Conf &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


ensemble_stat_Conf & ensemble_stat_Conf::operator=(const ensemble_stat_Conf &)

{

cerr << "\n\n  ensemble_stat_Conf::operator=(const ensemble_stat_Conf &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ensemble_stat_Conf::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ensemble_stat_Conf::clear()

{

          _model_entry = (const SymbolTableEntry *) 0;

      _ens_field_entry = (const SymbolTableEntry *) 0;

     _ens_thresh_entry = (const SymbolTableEntry *) 0;

 _vld_ens_thresh_entry = (const SymbolTableEntry *) 0;

_vld_data_thresh_entry = (const SymbolTableEntry *) 0;

     _fcst_field_entry = (const SymbolTableEntry *) 0;

      _obs_field_entry = (const SymbolTableEntry *) 0;

         _beg_ds_entry = (const SymbolTableEntry *) 0;

         _end_ds_entry = (const SymbolTableEntry *) 0;

   _message_type_entry = (const SymbolTableEntry *) 0;

      _mask_grid_entry = (const SymbolTableEntry *) 0;

      _mask_poly_entry = (const SymbolTableEntry *) 0;

       _mask_sid_entry = (const SymbolTableEntry *) 0;

  _interp_method_entry = (const SymbolTableEntry *) 0;

   _interp_width_entry = (const SymbolTableEntry *) 0;

    _interp_flag_entry = (const SymbolTableEntry *) 0;

  _interp_thresh_entry = (const SymbolTableEntry *) 0;

    _output_flag_entry = (const SymbolTableEntry *) 0;

       _rng_type_entry = (const SymbolTableEntry *) 0;

       _rng_seed_entry = (const SymbolTableEntry *) 0;

       _grib_ptv_entry = (const SymbolTableEntry *) 0;

  _output_prefix_entry = (const SymbolTableEntry *) 0;

        _version_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ensemble_stat_Conf::read(const char * _config_filename)

{

const SymbolTableEntry * _e = (const SymbolTableEntry *) 0;

   //
   //  read the config file into the machine
   //

_m.read(_config_filename);

   //
   //  lookup the entries in the symbol table
   //

_e = _m.find("model");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"model\"\n\n";

   exit ( 1 );

}

_model_entry = _e;


_e = _m.find("ens_field");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"ens_field\"\n\n";

   exit ( 1 );

}

_ens_field_entry = _e;


_e = _m.find("ens_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"ens_thresh\"\n\n";

   exit ( 1 );

}

_ens_thresh_entry = _e;


_e = _m.find("vld_ens_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"vld_ens_thresh\"\n\n";

   exit ( 1 );

}

_vld_ens_thresh_entry = _e;


_e = _m.find("vld_data_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"vld_data_thresh\"\n\n";

   exit ( 1 );

}

_vld_data_thresh_entry = _e;


_e = _m.find("fcst_field");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_field\"\n\n";

   exit ( 1 );

}

_fcst_field_entry = _e;


_e = _m.find("obs_field");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_field\"\n\n";

   exit ( 1 );

}

_obs_field_entry = _e;


_e = _m.find("beg_ds");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"beg_ds\"\n\n";

   exit ( 1 );

}

_beg_ds_entry = _e;


_e = _m.find("end_ds");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"end_ds\"\n\n";

   exit ( 1 );

}

_end_ds_entry = _e;


_e = _m.find("message_type");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"message_type\"\n\n";

   exit ( 1 );

}

_message_type_entry = _e;


_e = _m.find("mask_grid");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_grid\"\n\n";

   exit ( 1 );

}

_mask_grid_entry = _e;


_e = _m.find("mask_poly");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_poly\"\n\n";

   exit ( 1 );

}

_mask_poly_entry = _e;


_e = _m.find("mask_sid");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_sid\"\n\n";

   exit ( 1 );

}

_mask_sid_entry = _e;


_e = _m.find("interp_method");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_method\"\n\n";

   exit ( 1 );

}

_interp_method_entry = _e;


_e = _m.find("interp_width");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_width\"\n\n";

   exit ( 1 );

}

_interp_width_entry = _e;


_e = _m.find("interp_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_flag\"\n\n";

   exit ( 1 );

}

_interp_flag_entry = _e;


_e = _m.find("interp_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_thresh\"\n\n";

   exit ( 1 );

}

_interp_thresh_entry = _e;


_e = _m.find("output_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"output_flag\"\n\n";

   exit ( 1 );

}

_output_flag_entry = _e;


_e = _m.find("rng_type");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"rng_type\"\n\n";

   exit ( 1 );

}

_rng_type_entry = _e;


_e = _m.find("rng_seed");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"rng_seed\"\n\n";

   exit ( 1 );

}

_rng_seed_entry = _e;


_e = _m.find("grib_ptv");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"grib_ptv\"\n\n";

   exit ( 1 );

}

_grib_ptv_entry = _e;


_e = _m.find("output_prefix");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"output_prefix\"\n\n";

   exit ( 1 );

}

_output_prefix_entry = _e;


_e = _m.find("version");

if ( !_e && Panic )  {

   cerr << "\n\n  ensemble_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"version\"\n\n";

   exit ( 1 );

}

_version_entry = _e;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::model()

{

Result _temp_result;

if ( !_model_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_model_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::ens_field(int _i0)

{

Result _temp_result;

if ( !_ens_field_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _ens_field_entry->ai;


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


int ensemble_stat_Conf::n_ens_field_elements()

{

if ( !_ens_field_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _ens_field_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::ens_thresh(int _i0)

{

Result _temp_result;

if ( !_ens_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _ens_thresh_entry->ai;


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


int ensemble_stat_Conf::n_ens_thresh_elements()

{

if ( !_ens_thresh_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _ens_thresh_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::vld_ens_thresh()

{

Result _temp_result;

if ( !_vld_ens_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_vld_ens_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::vld_data_thresh()

{

Result _temp_result;

if ( !_vld_data_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_vld_data_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::fcst_field(int _i0)

{

Result _temp_result;

if ( !_fcst_field_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_field_entry->ai;


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


int ensemble_stat_Conf::n_fcst_field_elements()

{

if ( !_fcst_field_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_field_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::obs_field(int _i0)

{

Result _temp_result;

if ( !_obs_field_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_field_entry->ai;


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


int ensemble_stat_Conf::n_obs_field_elements()

{

if ( !_obs_field_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_field_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::beg_ds()

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


Result ensemble_stat_Conf::end_ds()

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


Result ensemble_stat_Conf::message_type(int _i0)

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


int ensemble_stat_Conf::n_message_type_elements()

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


Result ensemble_stat_Conf::mask_grid(int _i0)

{

Result _temp_result;

if ( !_mask_grid_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _mask_grid_entry->ai;


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


int ensemble_stat_Conf::n_mask_grid_elements()

{

if ( !_mask_grid_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _mask_grid_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::mask_poly(int _i0)

{

Result _temp_result;

if ( !_mask_poly_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _mask_poly_entry->ai;


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


int ensemble_stat_Conf::n_mask_poly_elements()

{

if ( !_mask_poly_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _mask_poly_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::mask_sid()

{

Result _temp_result;

if ( !_mask_sid_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_sid_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::interp_method(int _i0)

{

Result _temp_result;

if ( !_interp_method_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _interp_method_entry->ai;


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


int ensemble_stat_Conf::n_interp_method_elements()

{

if ( !_interp_method_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _interp_method_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::interp_width(int _i0)

{

Result _temp_result;

if ( !_interp_width_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _interp_width_entry->ai;


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


int ensemble_stat_Conf::n_interp_width_elements()

{

if ( !_interp_width_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _interp_width_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::interp_flag()

{

Result _temp_result;

if ( !_interp_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_interp_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::interp_thresh()

{

Result _temp_result;

if ( !_interp_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_interp_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::output_flag(int _i0)

{

Result _temp_result;

if ( !_output_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _output_flag_entry->ai;


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


int ensemble_stat_Conf::n_output_flag_elements()

{

if ( !_output_flag_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _output_flag_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::rng_type()

{

Result _temp_result;

if ( !_rng_type_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_rng_type_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::rng_seed()

{

Result _temp_result;

if ( !_rng_seed_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_rng_seed_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::grib_ptv()

{

Result _temp_result;

if ( !_grib_ptv_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_grib_ptv_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::output_prefix()

{

Result _temp_result;

if ( !_output_prefix_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_output_prefix_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result ensemble_stat_Conf::version()

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


