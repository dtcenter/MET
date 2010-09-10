

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "GridStatConfig_default"
   //
   //     on September 10, 2010    2:59 pm  MDT
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "grid_stat_Conf.h"
#include "vx_econfig/icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


static const int Panic = 1;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class grid_stat_Conf
   //


////////////////////////////////////////////////////////////////////////


grid_stat_Conf::grid_stat_Conf()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


grid_stat_Conf::~grid_stat_Conf()

{

clear();

}


////////////////////////////////////////////////////////////////////////


grid_stat_Conf::grid_stat_Conf(const grid_stat_Conf &)

{

cerr << "\n\n  grid_stat_Conf::grid_stat_Conf(const grid_stat_Conf &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


grid_stat_Conf & grid_stat_Conf::operator=(const grid_stat_Conf &)

{

cerr << "\n\n  grid_stat_Conf::operator=(const grid_stat_Conf &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void grid_stat_Conf::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void grid_stat_Conf::clear()

{

           _model_entry = (const SymbolTableEntry *) 0;

      _fcst_field_entry = (const SymbolTableEntry *) 0;

       _obs_field_entry = (const SymbolTableEntry *) 0;

     _fcst_thresh_entry = (const SymbolTableEntry *) 0;

      _obs_thresh_entry = (const SymbolTableEntry *) 0;

_fcst_wind_thresh_entry = (const SymbolTableEntry *) 0;

 _obs_wind_thresh_entry = (const SymbolTableEntry *) 0;

       _mask_grid_entry = (const SymbolTableEntry *) 0;

       _mask_poly_entry = (const SymbolTableEntry *) 0;

        _ci_alpha_entry = (const SymbolTableEntry *) 0;

   _boot_interval_entry = (const SymbolTableEntry *) 0;

   _boot_rep_prop_entry = (const SymbolTableEntry *) 0;

      _n_boot_rep_entry = (const SymbolTableEntry *) 0;

        _boot_rng_entry = (const SymbolTableEntry *) 0;

       _boot_seed_entry = (const SymbolTableEntry *) 0;

   _interp_method_entry = (const SymbolTableEntry *) 0;

    _interp_width_entry = (const SymbolTableEntry *) 0;

     _interp_flag_entry = (const SymbolTableEntry *) 0;

   _interp_thresh_entry = (const SymbolTableEntry *) 0;

       _nbr_width_entry = (const SymbolTableEntry *) 0;

      _nbr_thresh_entry = (const SymbolTableEntry *) 0;

      _cov_thresh_entry = (const SymbolTableEntry *) 0;

     _output_flag_entry = (const SymbolTableEntry *) 0;

  _rank_corr_flag_entry = (const SymbolTableEntry *) 0;

        _grib_ptv_entry = (const SymbolTableEntry *) 0;

         _tmp_dir_entry = (const SymbolTableEntry *) 0;

   _output_prefix_entry = (const SymbolTableEntry *) 0;

         _version_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void grid_stat_Conf::read(const char * _config_filename)

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

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"model\"\n\n";

   exit ( 1 );

}

_model_entry = _e;


_e = _m.find("fcst_field");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_field\"\n\n";

   exit ( 1 );

}

_fcst_field_entry = _e;


_e = _m.find("obs_field");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_field\"\n\n";

   exit ( 1 );

}

_obs_field_entry = _e;


_e = _m.find("fcst_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_thresh\"\n\n";

   exit ( 1 );

}

_fcst_thresh_entry = _e;


_e = _m.find("obs_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_thresh\"\n\n";

   exit ( 1 );

}

_obs_thresh_entry = _e;


_e = _m.find("fcst_wind_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_wind_thresh\"\n\n";

   exit ( 1 );

}

_fcst_wind_thresh_entry = _e;


_e = _m.find("obs_wind_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_wind_thresh\"\n\n";

   exit ( 1 );

}

_obs_wind_thresh_entry = _e;


_e = _m.find("mask_grid");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_grid\"\n\n";

   exit ( 1 );

}

_mask_grid_entry = _e;


_e = _m.find("mask_poly");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_poly\"\n\n";

   exit ( 1 );

}

_mask_poly_entry = _e;


_e = _m.find("ci_alpha");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"ci_alpha\"\n\n";

   exit ( 1 );

}

_ci_alpha_entry = _e;


_e = _m.find("boot_interval");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_interval\"\n\n";

   exit ( 1 );

}

_boot_interval_entry = _e;


_e = _m.find("boot_rep_prop");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_rep_prop\"\n\n";

   exit ( 1 );

}

_boot_rep_prop_entry = _e;


_e = _m.find("n_boot_rep");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"n_boot_rep\"\n\n";

   exit ( 1 );

}

_n_boot_rep_entry = _e;


_e = _m.find("boot_rng");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_rng\"\n\n";

   exit ( 1 );

}

_boot_rng_entry = _e;


_e = _m.find("boot_seed");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_seed\"\n\n";

   exit ( 1 );

}

_boot_seed_entry = _e;


_e = _m.find("interp_method");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_method\"\n\n";

   exit ( 1 );

}

_interp_method_entry = _e;


_e = _m.find("interp_width");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_width\"\n\n";

   exit ( 1 );

}

_interp_width_entry = _e;


_e = _m.find("interp_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_flag\"\n\n";

   exit ( 1 );

}

_interp_flag_entry = _e;


_e = _m.find("interp_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_thresh\"\n\n";

   exit ( 1 );

}

_interp_thresh_entry = _e;


_e = _m.find("nbr_width");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"nbr_width\"\n\n";

   exit ( 1 );

}

_nbr_width_entry = _e;


_e = _m.find("nbr_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"nbr_thresh\"\n\n";

   exit ( 1 );

}

_nbr_thresh_entry = _e;


_e = _m.find("cov_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"cov_thresh\"\n\n";

   exit ( 1 );

}

_cov_thresh_entry = _e;


_e = _m.find("output_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"output_flag\"\n\n";

   exit ( 1 );

}

_output_flag_entry = _e;


_e = _m.find("rank_corr_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"rank_corr_flag\"\n\n";

   exit ( 1 );

}

_rank_corr_flag_entry = _e;


_e = _m.find("grib_ptv");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"grib_ptv\"\n\n";

   exit ( 1 );

}

_grib_ptv_entry = _e;


_e = _m.find("tmp_dir");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"tmp_dir\"\n\n";

   exit ( 1 );

}

_tmp_dir_entry = _e;


_e = _m.find("output_prefix");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"output_prefix\"\n\n";

   exit ( 1 );

}

_output_prefix_entry = _e;


_e = _m.find("version");

if ( !_e && Panic )  {

   cerr << "\n\n  grid_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"version\"\n\n";

   exit ( 1 );

}

_version_entry = _e;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::model()

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


Result grid_stat_Conf::fcst_field(int _i0)

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


int grid_stat_Conf::n_fcst_field_elements()

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


Result grid_stat_Conf::obs_field(int _i0)

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


int grid_stat_Conf::n_obs_field_elements()

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


Result grid_stat_Conf::fcst_thresh(int _i0)

{

Result _temp_result;

if ( !_fcst_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_thresh_entry->ai;


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


int grid_stat_Conf::n_fcst_thresh_elements()

{

if ( !_fcst_thresh_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_thresh_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::obs_thresh(int _i0)

{

Result _temp_result;

if ( !_obs_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_thresh_entry->ai;


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


int grid_stat_Conf::n_obs_thresh_elements()

{

if ( !_obs_thresh_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_thresh_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::fcst_wind_thresh(int _i0)

{

Result _temp_result;

if ( !_fcst_wind_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_wind_thresh_entry->ai;


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


int grid_stat_Conf::n_fcst_wind_thresh_elements()

{

if ( !_fcst_wind_thresh_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_wind_thresh_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::obs_wind_thresh(int _i0)

{

Result _temp_result;

if ( !_obs_wind_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_wind_thresh_entry->ai;


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


int grid_stat_Conf::n_obs_wind_thresh_elements()

{

if ( !_obs_wind_thresh_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_wind_thresh_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::mask_grid(int _i0)

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


int grid_stat_Conf::n_mask_grid_elements()

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


Result grid_stat_Conf::mask_poly(int _i0)

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


int grid_stat_Conf::n_mask_poly_elements()

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


Result grid_stat_Conf::ci_alpha(int _i0)

{

Result _temp_result;

if ( !_ci_alpha_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _ci_alpha_entry->ai;


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


int grid_stat_Conf::n_ci_alpha_elements()

{

if ( !_ci_alpha_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _ci_alpha_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::boot_interval()

{

Result _temp_result;

if ( !_boot_interval_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_boot_interval_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::boot_rep_prop()

{

Result _temp_result;

if ( !_boot_rep_prop_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_boot_rep_prop_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::n_boot_rep()

{

Result _temp_result;

if ( !_n_boot_rep_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_n_boot_rep_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::boot_rng()

{

Result _temp_result;

if ( !_boot_rng_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_boot_rng_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::boot_seed()

{

Result _temp_result;

if ( !_boot_seed_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_boot_seed_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::interp_method(int _i0)

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


int grid_stat_Conf::n_interp_method_elements()

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


Result grid_stat_Conf::interp_width(int _i0)

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


int grid_stat_Conf::n_interp_width_elements()

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


Result grid_stat_Conf::interp_flag()

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


Result grid_stat_Conf::interp_thresh()

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


Result grid_stat_Conf::nbr_width(int _i0)

{

Result _temp_result;

if ( !_nbr_width_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _nbr_width_entry->ai;


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


int grid_stat_Conf::n_nbr_width_elements()

{

if ( !_nbr_width_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _nbr_width_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::nbr_thresh()

{

Result _temp_result;

if ( !_nbr_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_nbr_thresh_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::cov_thresh(int _i0)

{

Result _temp_result;

if ( !_cov_thresh_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _cov_thresh_entry->ai;


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


int grid_stat_Conf::n_cov_thresh_elements()

{

if ( !_cov_thresh_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _cov_thresh_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::output_flag(int _i0)

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


int grid_stat_Conf::n_output_flag_elements()

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


Result grid_stat_Conf::rank_corr_flag()

{

Result _temp_result;

if ( !_rank_corr_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_rank_corr_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result grid_stat_Conf::grib_ptv()

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


Result grid_stat_Conf::tmp_dir()

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


Result grid_stat_Conf::output_prefix()

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


Result grid_stat_Conf::version()

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


