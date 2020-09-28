// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_grib.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <map>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_grib.h"

#include "util_constants.h"
#include "grib_strings.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoGrib
//
///////////////////////////////////////////////////////////////////////////////

VarInfoGrib::VarInfoGrib() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib::~VarInfoGrib() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib::VarInfoGrib(const VarInfoGrib &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib & VarInfoGrib::operator=(const VarInfoGrib &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::assign(const VarInfoGrib &v) {

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   PTV       = v.ptv();
   Code      = v.code();
   LvlType   = v.lvl_type();
   PCode     = v.p_code();
   Center    = v.center ();
   Subcenter = v.subcenter ();
   FieldRec  = v.field_rec ();
   TRI       = v.tri();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   PTV          = bad_data_int;
   Code         = bad_data_int;
   LvlType      = bad_data_int;
   PCode        = bad_data_int;
   Center       = bad_data_int;
   Subcenter    = bad_data_int;
   FieldRec    = bad_data_int;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoGrib::dump():\n"
       << "  PTV       = " << PTV       << "\n"
       << "  Code      = " << Code      << "\n"
       << "  LvlType   = " << LvlType   << "\n"
       << "  PCode     = " << PCode     << "\n"
       << "  Center    = " << Center    << "\n"
       << "  Subcenter = " << Subcenter << "\n";

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_ptv(int v) {
   PTV = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_center(int v) {
   Center = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_subcenter(int v) {
   Subcenter = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_code(int v) {
   Code = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_lvl_type(int v) {
   LvlType = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_p_code(int v) {
   PCode = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_field_rec(int v) {
   FieldRec = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_tri(int v) {
   TRI = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_magic(const ConcatString &nstr, const ConcatString &lstr) {

   // Validate the magic_string
   VarInfo::set_magic(nstr, lstr);

   // Store the magic string
   MagicStr << cs_erase << nstr << "/" << lstr;

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::add_grib_code (Dictionary &dict)
{
   ConcatString field_name = dict.lookup_string(conf_key_name,            false);
   int tab_match = -1;
   int field_ptv           = dict.lookup_int   (conf_key_GRIB1_ptv,       false);
   int field_code          = dict.lookup_int   (conf_key_GRIB1_code,      false);

   // if field_code is not found - try to look for the deprecated name (GRIB1_rec)
   if(field_code == bad_data_int)
   {
      field_code           = dict.lookup_int   (conf_key_GRIB1_rec,       false);
   }
   int field_center        = dict.lookup_int   (conf_key_GRIB1_center,    false);
   int field_subcenter     = dict.lookup_int   (conf_key_GRIB1_subcenter, false);
   Grib1TableEntry  tab;

   // if not specified, fill others with default values
   if(field_ptv       == bad_data_int) field_ptv       = default_grib1_ptv;
   if(field_center    == bad_data_int) field_center    = default_grib1_center;
   if(field_subcenter == bad_data_int) field_subcenter = default_grib1_subcenter;

   //  if the name is specified, use it
   if( !field_name.empty() ){

      //  look up the name in the grib tables
      if( !GribTable.lookup_grib1(field_name.c_str(), field_ptv, field_code, field_center, field_subcenter, tab, tab_match) )
      {
         //  if did not find with params from the header - try default
         if( !GribTable.lookup_grib1(field_name.c_str(), default_grib1_ptv, field_code, default_grib1_center, default_grib1_subcenter, tab, tab_match) )
         {
            mlog << Error << "\nVarInfoGrib::add_grib_code() -> "
                 << "unrecognized GRIB1 field abbreviation '"
                 << field_name << "' for table version " << field_ptv
                 << "\n\n";
            exit(1);
         }
      }
   }

      //  if the field name is not specified, look for and use indexes
   else {

      //  if either the field name or the indices are specified, bail
      if( bad_data_int == field_ptv || bad_data_int == field_code ){
         mlog << Error << "\nVarInfoGrib::add_grib_code() -> "
              << "either name or GRIB1_ptv and GRIB1_code must be "
              << "specified in field information\n\n";
         exit(1);
      }

      //  use the specified indexes to look up the field name
      if( !GribTable.lookup_grib1(field_code, field_ptv, field_center, field_subcenter,tab) ){
         //if did not find with params from the header - try default
         if( !GribTable.lookup_grib1(field_code, default_grib1_ptv, default_grib1_center, default_grib1_subcenter,tab) )
         {
            mlog << Error << "\nVarInfoGrib::add_grib_code() -> "
                 << "no parameter found with matching GRIB1_ptv ("
                 << field_ptv << ") " << "GRIB1_code (" << field_code
                 << "). Use the MET_GRIB_TABLES environment variable "
                 << "to define custom GRIB tables.\n\n";
            exit(1);
         }
      }
   }
   set_code      ( tab.code      );
   set_long_name ( tab.full_name.c_str() );
   set_units     ( tab.units.c_str()     );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_dict(Dictionary & dict) {

   VarInfo::set_dict(dict);

   int tab_match = -1;
   Grib1TableEntry tab;
   ConcatString field_name = dict.lookup_string(conf_key_name,            false);
   int field_ptv           = dict.lookup_int   (conf_key_GRIB1_ptv,       false);
   int field_code          = dict.lookup_int   (conf_key_GRIB1_code,      false);

   //  if field_code is not found - look for the deprecated name (GRIB1_rec)
   if(field_code == bad_data_int)
   {
      field_code           = dict.lookup_int   (conf_key_GRIB1_rec,       false);
   }
   int field_center        = dict.lookup_int   (conf_key_GRIB1_center,    false);
   int field_subcenter     = dict.lookup_int   (conf_key_GRIB1_subcenter, false);

   ConcatString ens_str    = dict.lookup_string (conf_key_GRIB_ens, false);

   if( !field_name.empty() ){
      set_name     ( field_name );
      set_req_name ( field_name.c_str() );
   }
   set_field_rec   (field_code);
   set_center      (field_center);
   set_subcenter   (field_subcenter);
   set_ptv         (field_ptv);
   set_ens         (ens_str.c_str());
   set_tri         (dict.lookup_int(conf_key_GRIB1_tri, false));

   //  call the parent to set the level information
   set_level_info_grib(dict);

   //  check for a probability dictionary setting
   Dictionary* dict_prob;
   if(NULL == (dict_prob = dict.lookup_dictionary(conf_key_prob, false, false))) return;

   //  gather information from the prob dictionary
   ConcatString prob_name = dict_prob->lookup_string(conf_key_name);
   field_ptv              = dict_prob->lookup_int   (conf_key_GRIB1_ptv,  false);
   field_code             = dict_prob->lookup_int   (conf_key_GRIB1_code, false);

   //  if field_code is not found - look for the deprecated name (GRIB1_rec)
   if(field_code == bad_data_int) {
      field_code          = dict_prob->lookup_int   (conf_key_GRIB1_rec, false);
   }
   double thresh_lo       = dict_prob->lookup_double(conf_key_thresh_lo, false);
   double thresh_hi       = dict_prob->lookup_double(conf_key_thresh_hi, false);

   //  look up the probability field abbreviation
   if( !GribTable.lookup_grib1(prob_name.c_str(), field_ptv, field_code, tab, tab_match) ){
      mlog << Error << "\nVarInfoGrib::set_dict() -> "
           << "unrecognized GRIB1 probability field abbreviation '"
           << prob_name << "'\n\n";
      exit(1);
   }

   set_p_flag  ( true      );
   set_p_code  ( tab.code  );
   set_p_units ( tab.units.c_str() );

   set_prob_info_grib(prob_name, thresh_lo, thresh_hi);

}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_precipitation() const {
   int i;
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsPrecipitation)) {
      return(SetAttrIsPrecipitation != 0);
   }

   //
   // The ReqName member contains the requested GRIB code abbreviation.
   // Check to see if it matches the GRIB precipitation abbreviations.
   //
   for(i=0; i<n_grib_precipitation_abbr; i++) {
      if( ReqName ==  grib_precipitation_abbr[i] ) {
         status = true;
         break;
      }
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_specific_humidity() const {
   int i;
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsSpecificHumidity)) {
      return(SetAttrIsSpecificHumidity != 0);
   }

   //
   // The ReqName meber contains the requested GRIB code abbreviation.
   // Check to see if it matches the GRIB specific humidity abbreviations.
   //
   for(i=0; i<n_grib_specific_humidity_abbr; i++) {
      if( ReqName == grib_specific_humidity_abbr[i] ) {
         status = true;
         break;
      }
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   return(Code == ugrd_grib_code || ReqName == ugrd_abbr_str);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   return(Code == vgrd_grib_code || ReqName == vgrd_abbr_str);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   return(Code == wind_grib_code || ReqName == wind_abbr_str);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindDirection)) {
      return(SetAttrIsWindDirection != 0);
   }

   return(Code == wdir_grib_code || ReqName == wdir_abbr_str);
}

///////////////////////////////////////////////////////////////////////////////
