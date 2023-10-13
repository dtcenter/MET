// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_data2d_factory.h"

#include "mode_field_info.h"

using std::string;

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

if ( dict->lookup(conf_key_vld_thresh) )  {

   vld_thresh         = dict->lookup_double(conf_key_vld_thresh);

}

// For the multivar case, more complex logic regarding merge_flag and merge_thresh
// If individual entry has a merge_flag, it must have a merge_thresh (unless merge_flag=NONE)
// If individual entry has no merge_flag, check the parent and go with that flag and thresh
// If individual entry has no merge_flag, but individual entry has a merge_thresh, it's an error

if ( _multivar ) {
   // set defaults to no merging
   merge_thresh.clear();
   merge_flag = MergeType_None;

   // pull out the name
   string name = var_info->name();
   
   if ( dict->lookup(conf_key_merge_flag, false)) {
      // this individual entry has merge_flag
      merge_flag         = int_to_mergetype(dict->lookup_int(conf_key_merge_flag,
                                                             default_dictionary_error_out,
                                                             default_dictionary_print_warning,
                                                             false));
      string merge_name = mergetype_to_string(merge_flag);
      if (dict->lookup(conf_key_merge_thresh, false)) {
         // the individual entry also has a merge_thresh, this is good
         merge_thresh_array = dict->lookup_thresh_array(conf_key_merge_thresh, 
                                                        default_dictionary_error_out,
                                                        default_dictionary_print_warning,
                                                        false);
      } else {
         // get the parent's merge_thresh, just to have something.  Error out if the merge_flag is not none
         // becAuse that is inconsistent
         merge_thresh_array = dict->lookup_thresh_array(conf_key_merge_thresh);

         if (merge_flag != MergeType_None) {
            mlog << Error << "\nMode_Field_Info::set() -> "
                 << "Field:" << name << ". "
                 << " When 'merge_flag' is explicitly set, 'merge_thresh' must be explicitly set for multivariate mode\n\n";
            exit ( 1 );
         }
         
      } 
   } else {
      // individual entry does not have a merge_flag, try parent
      if ( dict->lookup(conf_key_merge_flag, true)) {
         // the parent does have a merge flag
         merge_flag = int_to_mergetype(dict->lookup_int(conf_key_merge_flag));
         string merge_name = mergetype_to_string(merge_flag);
         if (dict->lookup(conf_key_merge_thresh, false)) {
            // individual entry has a merge_thresh but no merge_flag, this is not good
            mlog << Error << "\nMode_Field_Info::set() -> "
                 << "Field:" << name << ". "
                 << "When 'merge_flag' is not explicitly set, 'merge_thresh' cannot explicitly set for multivariate mode\n\n";
            exit ( 1 );
         } else {
            // individual entry doesn't have a merge_thresh, parent has a merge_flag
            // expect parent to have a merge_thresh
            merge_thresh_array = dict->lookup_thresh_array(conf_key_merge_thresh);
            if (merge_thresh_array.n() == 0 && merge_flag != MergeType_None) {
               // parent has a merge_flag but no merge_thresh
               mlog << Error << "\nMode_Field_Info::set() -> "
                    << "Field:" << name << ". using parent merge_flag: " << merge_name
                    << " Parent has no 'merge_thresh', not allowed in multivariate mode\n\n";
               exit ( 1 );
            } else {
               string thresh_str = merge_thresh_array.thresh()->get_str();
               mlog  << Debug(2) << "Field:" << name << ". Using parent merge_flag: "
                     << merge_name << " and parent merge_thresh:" << thresh_str
                     << "\n";
            }
         }
      }
   }
 } else {
   if ( dict->lookup(conf_key_merge_flag, true)) {
      merge_flag         = int_to_mergetype(dict->lookup_int(conf_key_merge_flag));
   }
   merge_thresh_array = dict->lookup_thresh_array(conf_key_merge_thresh);
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



void  Mode_Field_Info::set_conv_thresh_by_merge_index  (int k)
{
if ( (k < 0) || (k >= merge_thresh_array.n_elements()) )  {

   mlog << Error << "\nMode_Field_Info::set_conv_thresh_by_merge_index(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

conv_thresh = merge_thresh_array[k];


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



