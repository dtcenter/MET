// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nc_cf.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_nc_cf.h"

#include "vx_math.h"
#include "util_constants.h"
#include "vx_log.h"
#include "grib_strings.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoNcCF
//
///////////////////////////////////////////////////////////////////////////////

VarInfoNcCF::VarInfoNcCF() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcCF::~VarInfoNcCF() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcCF::VarInfoNcCF(const VarInfoNcCF &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcCF & VarInfoNcCF::operator=(const VarInfoNcCF &f) {

   if ( this == &f )  return *this;

   assign(f);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

VarInfo *VarInfoNcCF::clone() const {

   VarInfoNcCF *ret = new VarInfoNcCF(*this);

   return ret;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::assign(const VarInfoNcCF &v) {

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   clear_dimension();
   for(int i=0; i<v.n_dimension(); i++) {
      add_dimension(v.dimension(i), v.is_offset(i), v.dim_value(i));
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   clear_dimension();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoNcCF::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);
   out << "  Is_offset:\n";
   Is_offset.dump(out);
   out << "  Dim_value:\n";
   Dim_value.dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::set_default_levels(const ConcatString &lstr) {
   Level.set_req_name("*,*");
   Level.set_name("*,*");
   add_dimension(vx_data2d_star);
   add_dimension(vx_data2d_star);
}

///////////////////////////////////////////////////////////////////////////////
void VarInfoNcCF::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   const char *method_name = "VarInfoNcCF::set_magic() -> ";

   // Store the magic string
   set_magic_pre(nstr, lstr);

   // If there's no level specification, assume (*, *)
   parse_level(lstr);

   set_magic_post(req_name(), Level.req_name());

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::set_dict(Dictionary &dict, bool do_exit){

   bool status = VarInfo::set_dict(dict,do_exit);

   set_magic(dict.lookup_string("name"),
             dict.lookup_string("level"));
   set_req_name(dict.lookup_string("name").c_str());

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_precipitation() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsPrecipitation)) {
      return(SetAttrIsPrecipitation != 0);
   }

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any precipitation variables.
   //
   return has_prefix(grib_precipitation_abbr,
                     n_grib_precipitation_abbr,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_specific_humidity() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsSpecificHumidity)) {
      return(SetAttrIsSpecificHumidity != 0);
   }

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any specific humidity variables.
   //
   return has_prefix(grib_specific_humidity_abbr,
                     n_grib_specific_humidity_abbr,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   return is_grib_code_abbr_match(Name, ugrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   return is_grib_code_abbr_match(Name, vgrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   return is_grib_code_abbr_match(Name, wind_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindDirection)) {
      return(SetAttrIsWindDirection != 0);
   }

   return is_grib_code_abbr_match(Name, wdir_grib_code);
}

///////////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous utility functions
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
