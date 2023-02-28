// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_data2d_factory.h"

#include "mode_field_info.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Mode_Field_Info
   //


////////////////////////////////////////////////////////////////////////


Mode_Field_Info::Mode_Field_Info()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Mode_Field_Info::~Mode_Field_Info()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Mode_Field_Info::Mode_Field_Info(const Mode_Field_Info & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


Mode_Field_Info & Mode_Field_Info::operator=(const Mode_Field_Info & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Mode_Field_Info::init_from_scratch()

{

dict = 0;

conf = 0;

var_info = 0;

clear();


return;

}


////////////////////////////////////////////////////////////////////////


void Mode_Field_Info::clear()

{

index = -1;

dict = 0;   //  not allocated, so don't delete

conf = 0;   //  not allocated, so don't delete

gft = FileType_None;

Multivar = false;

FO = (char) 0;

conv_radius = 0;

vld_thresh = 0.0;

if ( var_info )  { delete var_info;  var_info = 0; }

conv_radius_array.clear();

conv_thresh_array.clear();

merge_thresh_array.clear();

conv_thresh.clear();

merge_thresh.clear();

merge_flag = MergeType_Engine;

// raw_pi.clear();

}


////////////////////////////////////////////////////////////////////////


void Mode_Field_Info::assign(const Mode_Field_Info & i)

{

set(i.Multivar, i.index, i.dict, i.conf, i.gft, i.FO, true);

return;

}


////////////////////////////////////////////////////////////////////////


void Mode_Field_Info::set (const bool _multivar, int _index, Dictionary * _dict, MetConfig * _conf, GrdFileType type, char _fo, bool do_clear)

{

VarInfoFactory info_factory;

if ( do_clear )  clear();

Multivar = _multivar;

index = _index;

dict = _dict;

conf = _conf;

var_info = info_factory.new_var_info(type);

if ( _multivar )  {

   var_info->set_dict(*dict);

} else {

   var_info->set_dict(*(dict->lookup_dictionary(conf_key_field)));

}

FO = _fo;

gft = type;

if ( dict->lookup(conf_key_raw_thresh) )  {

   mlog << Error 
        << "\nMode_Field_Info::process_config() -> "
        << "the \"" << conf_key_raw_thresh << "\" entry is deprecated in MET "
        << met_version << "!  Use \"" << conf_key_censor_thresh << "\" and \""
        << conf_key_censor_val << "\" instead.\n\n";

   exit(1);

}

if ( dict->lookup(conf_key_conv_radius) )  {

   conv_radius_array  = dict->lookup_int_array(conf_key_conv_radius);

}

if ( dict->lookup(conf_key_conv_thresh) )  {

   conv_thresh_array  = dict->lookup_thresh_array(conf_key_conv_thresh);

}

if ( dict->lookup(conf_key_merge_thresh) )  {

   merge_thresh_array = dict->lookup_thresh_array(conf_key_merge_thresh);

}

if ( dict->lookup(conf_key_vld_thresh) )  {

   vld_thresh         = dict->lookup_double(conf_key_vld_thresh);

}

if ( dict->lookup(conf_key_merge_flag) )  {

   merge_flag         = int_to_mergetype(dict->lookup_int(conf_key_merge_flag));

}


filter_attr_map    = parse_conf_filter_attr_map(dict);

if ( FO == 'F' )  raw_pi = parse_conf_plot_info(conf->lookup_dictionary(conf_key_fcst_raw_plot));
else              raw_pi = parse_conf_plot_info(conf->lookup_dictionary(conf_key_obs_raw_plot));




   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Mode_Field_Info::set_merge_thresh_by_index (int k)

{

if ( (k < 0) || (k >= merge_thresh_array.n_elements()) )  {

   mlog << Error << "\nMode_Field_Info::set_fcst_merge_thresh_by_index(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

merge_thresh = merge_thresh_array[k];


return;

}


////////////////////////////////////////////////////////////////////////


bool Mode_Field_Info::need_merge_thresh () const

{

bool status = (merge_flag == MergeType_Both) || (merge_flag == MergeType_Thresh);

return ( status );

}


////////////////////////////////////////////////////////////////////////



