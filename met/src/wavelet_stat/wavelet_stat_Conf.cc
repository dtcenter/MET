

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
   //     on March 3, 2011    2:57 pm  MST
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "wavelet_stat_Conf.h"
#include "vx_econfig/icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


static const int Panic = 1;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class wavelet_stat_Conf
   //


////////////////////////////////////////////////////////////////////////


wavelet_stat_Conf::wavelet_stat_Conf()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


wavelet_stat_Conf::~wavelet_stat_Conf()

{

clear();

}


////////////////////////////////////////////////////////////////////////


wavelet_stat_Conf::wavelet_stat_Conf(const wavelet_stat_Conf &)

{

cerr << "\n\n  wavelet_stat_Conf::wavelet_stat_Conf(const wavelet_stat_Conf &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


wavelet_stat_Conf & wavelet_stat_Conf::operator=(const wavelet_stat_Conf &)

{

cerr << "\n\n  wavelet_stat_Conf::operator=(const wavelet_stat_Conf &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void wavelet_stat_Conf::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void wavelet_stat_Conf::clear()

{

               _model_entry = (const SymbolTableEntry *) 0;

          _fcst_field_entry = (const SymbolTableEntry *) 0;

           _obs_field_entry = (const SymbolTableEntry *) 0;

         _fcst_thresh_entry = (const SymbolTableEntry *) 0;

          _obs_thresh_entry = (const SymbolTableEntry *) 0;

   _mask_missing_flag_entry = (const SymbolTableEntry *) 0;

    _grid_decomp_flag_entry = (const SymbolTableEntry *) 0;

            _tile_xll_entry = (const SymbolTableEntry *) 0;

            _tile_yll_entry = (const SymbolTableEntry *) 0;

            _tile_dim_entry = (const SymbolTableEntry *) 0;

        _wavelet_flag_entry = (const SymbolTableEntry *) 0;

           _wavelet_k_entry = (const SymbolTableEntry *) 0;

         _output_flag_entry = (const SymbolTableEntry *) 0;

        _met_data_dir_entry = (const SymbolTableEntry *) 0;

_fcst_raw_color_table_entry = (const SymbolTableEntry *) 0;

 _obs_raw_color_table_entry = (const SymbolTableEntry *) 0;

    _wvlt_color_table_entry = (const SymbolTableEntry *) 0;

   _fcst_raw_plot_min_entry = (const SymbolTableEntry *) 0;

   _fcst_raw_plot_max_entry = (const SymbolTableEntry *) 0;

    _obs_raw_plot_min_entry = (const SymbolTableEntry *) 0;

    _obs_raw_plot_max_entry = (const SymbolTableEntry *) 0;

       _wvlt_plot_min_entry = (const SymbolTableEntry *) 0;

       _wvlt_plot_max_entry = (const SymbolTableEntry *) 0;

            _grib_ptv_entry = (const SymbolTableEntry *) 0;

       _output_prefix_entry = (const SymbolTableEntry *) 0;

             _version_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void wavelet_stat_Conf::read(const char * _config_filename)

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

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"model\"\n\n";

   exit ( 1 );

}

_model_entry = _e;


_e = _m.find("fcst_field");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_field\"\n\n";

   exit ( 1 );

}

_fcst_field_entry = _e;


_e = _m.find("obs_field");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_field\"\n\n";

   exit ( 1 );

}

_obs_field_entry = _e;


_e = _m.find("fcst_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_thresh\"\n\n";

   exit ( 1 );

}

_fcst_thresh_entry = _e;


_e = _m.find("obs_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_thresh\"\n\n";

   exit ( 1 );

}

_obs_thresh_entry = _e;


_e = _m.find("mask_missing_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"mask_missing_flag\"\n\n";

   exit ( 1 );

}

_mask_missing_flag_entry = _e;


_e = _m.find("grid_decomp_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"grid_decomp_flag\"\n\n";

   exit ( 1 );

}

_grid_decomp_flag_entry = _e;


_e = _m.find("tile_xll");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"tile_xll\"\n\n";

   exit ( 1 );

}

_tile_xll_entry = _e;


_e = _m.find("tile_yll");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"tile_yll\"\n\n";

   exit ( 1 );

}

_tile_yll_entry = _e;


_e = _m.find("tile_dim");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"tile_dim\"\n\n";

   exit ( 1 );

}

_tile_dim_entry = _e;


_e = _m.find("wavelet_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"wavelet_flag\"\n\n";

   exit ( 1 );

}

_wavelet_flag_entry = _e;


_e = _m.find("wavelet_k");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"wavelet_k\"\n\n";

   exit ( 1 );

}

_wavelet_k_entry = _e;


_e = _m.find("output_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"output_flag\"\n\n";

   exit ( 1 );

}

_output_flag_entry = _e;


_e = _m.find("met_data_dir");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"met_data_dir\"\n\n";

   exit ( 1 );

}

_met_data_dir_entry = _e;


_e = _m.find("fcst_raw_color_table");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_color_table\"\n\n";

   exit ( 1 );

}

_fcst_raw_color_table_entry = _e;


_e = _m.find("obs_raw_color_table");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_color_table\"\n\n";

   exit ( 1 );

}

_obs_raw_color_table_entry = _e;


_e = _m.find("wvlt_color_table");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"wvlt_color_table\"\n\n";

   exit ( 1 );

}

_wvlt_color_table_entry = _e;


_e = _m.find("fcst_raw_plot_min");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_plot_min\"\n\n";

   exit ( 1 );

}

_fcst_raw_plot_min_entry = _e;


_e = _m.find("fcst_raw_plot_max");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_raw_plot_max\"\n\n";

   exit ( 1 );

}

_fcst_raw_plot_max_entry = _e;


_e = _m.find("obs_raw_plot_min");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_plot_min\"\n\n";

   exit ( 1 );

}

_obs_raw_plot_min_entry = _e;


_e = _m.find("obs_raw_plot_max");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_raw_plot_max\"\n\n";

   exit ( 1 );

}

_obs_raw_plot_max_entry = _e;


_e = _m.find("wvlt_plot_min");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"wvlt_plot_min\"\n\n";

   exit ( 1 );

}

_wvlt_plot_min_entry = _e;


_e = _m.find("wvlt_plot_max");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"wvlt_plot_max\"\n\n";

   exit ( 1 );

}

_wvlt_plot_max_entry = _e;


_e = _m.find("grib_ptv");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"grib_ptv\"\n\n";

   exit ( 1 );

}

_grib_ptv_entry = _e;


_e = _m.find("output_prefix");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"output_prefix\"\n\n";

   exit ( 1 );

}

_output_prefix_entry = _e;


_e = _m.find("version");

if ( !_e && Panic )  {

   cerr << "\n\n  wavelet_stat_Conf::read(const char *) -> can't get symbol table entry for variable \"version\"\n\n";

   exit ( 1 );

}

_version_entry = _e;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::model()

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


Result wavelet_stat_Conf::fcst_field(int _i0)

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


int wavelet_stat_Conf::n_fcst_field_elements()

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


Result wavelet_stat_Conf::obs_field(int _i0)

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


int wavelet_stat_Conf::n_obs_field_elements()

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


Result wavelet_stat_Conf::fcst_thresh(int _i0)

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


int wavelet_stat_Conf::n_fcst_thresh_elements()

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


Result wavelet_stat_Conf::obs_thresh(int _i0)

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


int wavelet_stat_Conf::n_obs_thresh_elements()

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


Result wavelet_stat_Conf::mask_missing_flag()

{

Result _temp_result;

if ( !_mask_missing_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_mask_missing_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::grid_decomp_flag()

{

Result _temp_result;

if ( !_grid_decomp_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_grid_decomp_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::tile_xll(int _i0)

{

Result _temp_result;

if ( !_tile_xll_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _tile_xll_entry->ai;


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


int wavelet_stat_Conf::n_tile_xll_elements()

{

if ( !_tile_xll_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _tile_xll_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::tile_yll(int _i0)

{

Result _temp_result;

if ( !_tile_yll_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _tile_yll_entry->ai;


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


int wavelet_stat_Conf::n_tile_yll_elements()

{

if ( !_tile_yll_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _tile_yll_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::tile_dim()

{

Result _temp_result;

if ( !_tile_dim_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_tile_dim_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::wavelet_flag()

{

Result _temp_result;

if ( !_wavelet_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_wavelet_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::wavelet_k()

{

Result _temp_result;

if ( !_wavelet_k_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_wavelet_k_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::output_flag(int _i0)

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


int wavelet_stat_Conf::n_output_flag_elements()

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


Result wavelet_stat_Conf::met_data_dir()

{

Result _temp_result;

if ( !_met_data_dir_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_met_data_dir_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::fcst_raw_color_table()

{

Result _temp_result;

if ( !_fcst_raw_color_table_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_raw_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::obs_raw_color_table()

{

Result _temp_result;

if ( !_obs_raw_color_table_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_raw_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::wvlt_color_table()

{

Result _temp_result;

if ( !_wvlt_color_table_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_wvlt_color_table_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::fcst_raw_plot_min()

{

Result _temp_result;

if ( !_fcst_raw_plot_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_raw_plot_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::fcst_raw_plot_max()

{

Result _temp_result;

if ( !_fcst_raw_plot_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_raw_plot_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::obs_raw_plot_min()

{

Result _temp_result;

if ( !_obs_raw_plot_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_raw_plot_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::obs_raw_plot_max()

{

Result _temp_result;

if ( !_obs_raw_plot_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_raw_plot_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::wvlt_plot_min()

{

Result _temp_result;

if ( !_wvlt_plot_min_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_wvlt_plot_min_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::wvlt_plot_max()

{

Result _temp_result;

if ( !_wvlt_plot_max_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_wvlt_plot_max_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result wavelet_stat_Conf::grib_ptv()

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


Result wavelet_stat_Conf::output_prefix()

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


Result wavelet_stat_Conf::version()

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


