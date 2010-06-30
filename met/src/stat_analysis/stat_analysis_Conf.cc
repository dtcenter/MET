

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated
   //
   //     Do not edit by hand
   //
   //
   //     Created from config file "STATAnalysisConfig_default"
   //
   //     on June 21, 2010    10:19 am  MDT
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "stat_analysis_Conf.h"
#include "vx_econfig/icodecell_to_result.h"


////////////////////////////////////////////////////////////////////////


static const int Panic = 1;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class stat_analysis_Conf
   //


////////////////////////////////////////////////////////////////////////


stat_analysis_Conf::stat_analysis_Conf()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


stat_analysis_Conf::~stat_analysis_Conf()

{

clear();

}


////////////////////////////////////////////////////////////////////////


stat_analysis_Conf::stat_analysis_Conf(const stat_analysis_Conf &)

{

cerr << "\n\n  stat_analysis_Conf::stat_analysis_Conf(const stat_analysis_Conf &) -> should never be called!\n\n";

exit ( 1 );

//  init_from_scratch();

//  assign(a);

}


////////////////////////////////////////////////////////////////////////


stat_analysis_Conf & stat_analysis_Conf::operator=(const stat_analysis_Conf &)

{

cerr << "\n\n  stat_analysis_Conf::operator=(const stat_analysis_Conf &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &a )  return ( * this );

// assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void stat_analysis_Conf::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void stat_analysis_Conf::clear()

{

         _model_entry = (const SymbolTableEntry *) 0;

     _fcst_lead_entry = (const SymbolTableEntry *) 0;

      _obs_lead_entry = (const SymbolTableEntry *) 0;

_fcst_valid_beg_entry = (const SymbolTableEntry *) 0;

_fcst_valid_end_entry = (const SymbolTableEntry *) 0;

 _obs_valid_beg_entry = (const SymbolTableEntry *) 0;

 _obs_valid_end_entry = (const SymbolTableEntry *) 0;

 _fcst_init_beg_entry = (const SymbolTableEntry *) 0;

 _fcst_init_end_entry = (const SymbolTableEntry *) 0;

  _obs_init_beg_entry = (const SymbolTableEntry *) 0;

  _obs_init_end_entry = (const SymbolTableEntry *) 0;

_fcst_init_hour_entry = (const SymbolTableEntry *) 0;

 _obs_init_hour_entry = (const SymbolTableEntry *) 0;

      _fcst_var_entry = (const SymbolTableEntry *) 0;

       _obs_var_entry = (const SymbolTableEntry *) 0;

      _fcst_lev_entry = (const SymbolTableEntry *) 0;

       _obs_lev_entry = (const SymbolTableEntry *) 0;

        _obtype_entry = (const SymbolTableEntry *) 0;

       _vx_mask_entry = (const SymbolTableEntry *) 0;

   _interp_mthd_entry = (const SymbolTableEntry *) 0;

   _interp_pnts_entry = (const SymbolTableEntry *) 0;

   _fcst_thresh_entry = (const SymbolTableEntry *) 0;

    _obs_thresh_entry = (const SymbolTableEntry *) 0;

    _cov_thresh_entry = (const SymbolTableEntry *) 0;

         _alpha_entry = (const SymbolTableEntry *) 0;

     _line_type_entry = (const SymbolTableEntry *) 0;

          _jobs_entry = (const SymbolTableEntry *) 0;

     _out_alpha_entry = (const SymbolTableEntry *) 0;

 _boot_interval_entry = (const SymbolTableEntry *) 0;

 _boot_rep_prop_entry = (const SymbolTableEntry *) 0;

    _n_boot_rep_entry = (const SymbolTableEntry *) 0;

      _boot_rng_entry = (const SymbolTableEntry *) 0;

     _boot_seed_entry = (const SymbolTableEntry *) 0;

_rank_corr_flag_entry = (const SymbolTableEntry *) 0;

      _vif_flag_entry = (const SymbolTableEntry *) 0;

       _tmp_dir_entry = (const SymbolTableEntry *) 0;

       _version_entry = (const SymbolTableEntry *) 0;


_m.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void stat_analysis_Conf::read(const char * _config_filename)

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

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"model\"\n\n";

   exit ( 1 );

}

_model_entry = _e;


_e = _m.find("fcst_lead");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_lead\"\n\n";

   exit ( 1 );

}

_fcst_lead_entry = _e;


_e = _m.find("obs_lead");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_lead\"\n\n";

   exit ( 1 );

}

_obs_lead_entry = _e;


_e = _m.find("fcst_valid_beg");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_valid_beg\"\n\n";

   exit ( 1 );

}

_fcst_valid_beg_entry = _e;


_e = _m.find("fcst_valid_end");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_valid_end\"\n\n";

   exit ( 1 );

}

_fcst_valid_end_entry = _e;


_e = _m.find("obs_valid_beg");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_valid_beg\"\n\n";

   exit ( 1 );

}

_obs_valid_beg_entry = _e;


_e = _m.find("obs_valid_end");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_valid_end\"\n\n";

   exit ( 1 );

}

_obs_valid_end_entry = _e;


_e = _m.find("fcst_init_beg");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_init_beg\"\n\n";

   exit ( 1 );

}

_fcst_init_beg_entry = _e;


_e = _m.find("fcst_init_end");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_init_end\"\n\n";

   exit ( 1 );

}

_fcst_init_end_entry = _e;


_e = _m.find("obs_init_beg");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_init_beg\"\n\n";

   exit ( 1 );

}

_obs_init_beg_entry = _e;


_e = _m.find("obs_init_end");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_init_end\"\n\n";

   exit ( 1 );

}

_obs_init_end_entry = _e;


_e = _m.find("fcst_init_hour");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_init_hour\"\n\n";

   exit ( 1 );

}

_fcst_init_hour_entry = _e;


_e = _m.find("obs_init_hour");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_init_hour\"\n\n";

   exit ( 1 );

}

_obs_init_hour_entry = _e;


_e = _m.find("fcst_var");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_var\"\n\n";

   exit ( 1 );

}

_fcst_var_entry = _e;


_e = _m.find("obs_var");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_var\"\n\n";

   exit ( 1 );

}

_obs_var_entry = _e;


_e = _m.find("fcst_lev");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_lev\"\n\n";

   exit ( 1 );

}

_fcst_lev_entry = _e;


_e = _m.find("obs_lev");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_lev\"\n\n";

   exit ( 1 );

}

_obs_lev_entry = _e;


_e = _m.find("obtype");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obtype\"\n\n";

   exit ( 1 );

}

_obtype_entry = _e;


_e = _m.find("vx_mask");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"vx_mask\"\n\n";

   exit ( 1 );

}

_vx_mask_entry = _e;


_e = _m.find("interp_mthd");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_mthd\"\n\n";

   exit ( 1 );

}

_interp_mthd_entry = _e;


_e = _m.find("interp_pnts");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"interp_pnts\"\n\n";

   exit ( 1 );

}

_interp_pnts_entry = _e;


_e = _m.find("fcst_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"fcst_thresh\"\n\n";

   exit ( 1 );

}

_fcst_thresh_entry = _e;


_e = _m.find("obs_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"obs_thresh\"\n\n";

   exit ( 1 );

}

_obs_thresh_entry = _e;


_e = _m.find("cov_thresh");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"cov_thresh\"\n\n";

   exit ( 1 );

}

_cov_thresh_entry = _e;


_e = _m.find("alpha");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"alpha\"\n\n";

   exit ( 1 );

}

_alpha_entry = _e;


_e = _m.find("line_type");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"line_type\"\n\n";

   exit ( 1 );

}

_line_type_entry = _e;


_e = _m.find("jobs");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"jobs\"\n\n";

   exit ( 1 );

}

_jobs_entry = _e;


_e = _m.find("out_alpha");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"out_alpha\"\n\n";

   exit ( 1 );

}

_out_alpha_entry = _e;


_e = _m.find("boot_interval");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_interval\"\n\n";

   exit ( 1 );

}

_boot_interval_entry = _e;


_e = _m.find("boot_rep_prop");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_rep_prop\"\n\n";

   exit ( 1 );

}

_boot_rep_prop_entry = _e;


_e = _m.find("n_boot_rep");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"n_boot_rep\"\n\n";

   exit ( 1 );

}

_n_boot_rep_entry = _e;


_e = _m.find("boot_rng");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_rng\"\n\n";

   exit ( 1 );

}

_boot_rng_entry = _e;


_e = _m.find("boot_seed");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"boot_seed\"\n\n";

   exit ( 1 );

}

_boot_seed_entry = _e;


_e = _m.find("rank_corr_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"rank_corr_flag\"\n\n";

   exit ( 1 );

}

_rank_corr_flag_entry = _e;


_e = _m.find("vif_flag");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"vif_flag\"\n\n";

   exit ( 1 );

}

_vif_flag_entry = _e;


_e = _m.find("tmp_dir");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"tmp_dir\"\n\n";

   exit ( 1 );

}

_tmp_dir_entry = _e;


_e = _m.find("version");

if ( !_e && Panic )  {

   cerr << "\n\n  stat_analysis_Conf::read(const char *) -> can't get symbol table entry for variable \"version\"\n\n";

   exit ( 1 );

}

_version_entry = _e;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::model(int _i0)

{

Result _temp_result;

if ( !_model_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _model_entry->ai;


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


int stat_analysis_Conf::n_model_elements()

{

if ( !_model_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _model_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_lead(int _i0)

{

Result _temp_result;

if ( !_fcst_lead_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_lead_entry->ai;


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


int stat_analysis_Conf::n_fcst_lead_elements()

{

if ( !_fcst_lead_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_lead_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_lead(int _i0)

{

Result _temp_result;

if ( !_obs_lead_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_lead_entry->ai;


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


int stat_analysis_Conf::n_obs_lead_elements()

{

if ( !_obs_lead_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_lead_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_valid_beg()

{

Result _temp_result;

if ( !_fcst_valid_beg_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_valid_beg_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_valid_end()

{

Result _temp_result;

if ( !_fcst_valid_end_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_valid_end_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_valid_beg()

{

Result _temp_result;

if ( !_obs_valid_beg_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_valid_beg_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_valid_end()

{

Result _temp_result;

if ( !_obs_valid_end_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_valid_end_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_init_beg()

{

Result _temp_result;

if ( !_fcst_init_beg_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_init_beg_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_init_end()

{

Result _temp_result;

if ( !_fcst_init_end_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_fcst_init_end_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_init_beg()

{

Result _temp_result;

if ( !_obs_init_beg_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_init_beg_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_init_end()

{

Result _temp_result;

if ( !_obs_init_end_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_obs_init_end_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_init_hour(int _i0)

{

Result _temp_result;

if ( !_fcst_init_hour_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_init_hour_entry->ai;


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


int stat_analysis_Conf::n_fcst_init_hour_elements()

{

if ( !_fcst_init_hour_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_init_hour_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_init_hour(int _i0)

{

Result _temp_result;

if ( !_obs_init_hour_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_init_hour_entry->ai;


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


int stat_analysis_Conf::n_obs_init_hour_elements()

{

if ( !_obs_init_hour_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_init_hour_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_var(int _i0)

{

Result _temp_result;

if ( !_fcst_var_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_var_entry->ai;


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


int stat_analysis_Conf::n_fcst_var_elements()

{

if ( !_fcst_var_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_var_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_var(int _i0)

{

Result _temp_result;

if ( !_obs_var_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_var_entry->ai;


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


int stat_analysis_Conf::n_obs_var_elements()

{

if ( !_obs_var_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_var_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_lev(int _i0)

{

Result _temp_result;

if ( !_fcst_lev_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _fcst_lev_entry->ai;


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


int stat_analysis_Conf::n_fcst_lev_elements()

{

if ( !_fcst_lev_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _fcst_lev_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obs_lev(int _i0)

{

Result _temp_result;

if ( !_obs_lev_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obs_lev_entry->ai;


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


int stat_analysis_Conf::n_obs_lev_elements()

{

if ( !_obs_lev_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obs_lev_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::obtype(int _i0)

{

Result _temp_result;

if ( !_obtype_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _obtype_entry->ai;


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


int stat_analysis_Conf::n_obtype_elements()

{

if ( !_obtype_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _obtype_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::vx_mask(int _i0)

{

Result _temp_result;

if ( !_vx_mask_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _vx_mask_entry->ai;


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


int stat_analysis_Conf::n_vx_mask_elements()

{

if ( !_vx_mask_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _vx_mask_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::interp_mthd(int _i0)

{

Result _temp_result;

if ( !_interp_mthd_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _interp_mthd_entry->ai;


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


int stat_analysis_Conf::n_interp_mthd_elements()

{

if ( !_interp_mthd_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _interp_mthd_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::interp_pnts(int _i0)

{

Result _temp_result;

if ( !_interp_pnts_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _interp_pnts_entry->ai;


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


int stat_analysis_Conf::n_interp_pnts_elements()

{

if ( !_interp_pnts_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _interp_pnts_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::fcst_thresh(int _i0)

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


int stat_analysis_Conf::n_fcst_thresh_elements()

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


Result stat_analysis_Conf::obs_thresh(int _i0)

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


int stat_analysis_Conf::n_obs_thresh_elements()

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


Result stat_analysis_Conf::cov_thresh(int _i0)

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


int stat_analysis_Conf::n_cov_thresh_elements()

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


Result stat_analysis_Conf::alpha(int _i0)

{

Result _temp_result;

if ( !_alpha_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _alpha_entry->ai;


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


int stat_analysis_Conf::n_alpha_elements()

{

if ( !_alpha_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _alpha_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::line_type(int _i0)

{

Result _temp_result;

if ( !_line_type_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _line_type_entry->ai;


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


int stat_analysis_Conf::n_line_type_elements()

{

if ( !_line_type_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _line_type_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::jobs(int _i0)

{

Result _temp_result;

if ( !_jobs_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;
const IcodeVector * _v = (const IcodeVector *) 0;
int _indices[max_array_dim];
const ArrayInfo * _a = _jobs_entry->ai;


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


int stat_analysis_Conf::n_jobs_elements()

{

if ( !_jobs_entry && !Panic )  return ( 0 );

int _n;
const ArrayInfo * _a = _jobs_entry->ai;


_n = _a->size(0);


   //
   //  done
   //

return ( _n );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::out_alpha()

{

Result _temp_result;

if ( !_out_alpha_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_out_alpha_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::boot_interval()

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


Result stat_analysis_Conf::boot_rep_prop()

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


Result stat_analysis_Conf::n_boot_rep()

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


Result stat_analysis_Conf::boot_rng()

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


Result stat_analysis_Conf::boot_seed()

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


Result stat_analysis_Conf::rank_corr_flag()

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


Result stat_analysis_Conf::vif_flag()

{

Result _temp_result;

if ( !_vif_flag_entry && !Panic )  return ( _temp_result );

IcodeCell _cell;

_m.run( *_vif_flag_entry );

_cell = _m.pop();

icodecell_to_result(_cell, _temp_result);


return ( _temp_result );

}


////////////////////////////////////////////////////////////////////////


Result stat_analysis_Conf::tmp_dir()

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


Result stat_analysis_Conf::version()

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


