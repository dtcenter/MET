// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nc_wrf.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <regex>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_nc_wrf.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoNcWrf
//
///////////////////////////////////////////////////////////////////////////////

VarInfoNcWrf::VarInfoNcWrf() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcWrf::~VarInfoNcWrf() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcWrf::VarInfoNcWrf(const VarInfoNcWrf &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcWrf & VarInfoNcWrf::operator=(const VarInfoNcWrf &f) {

   if ( this == &f )  return *this;

   assign(f);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

unique_ptr<VarInfo> VarInfoNcWrf::clone() const {
   return unique_ptr<VarInfo>(new VarInfoNcWrf(*this)); 
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcWrf::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcWrf::assign(const VarInfoNcWrf &v) {
   int i;

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   clear_dimension();
   for(i=0; i<v.n_dimension(); i++) {
      add_dimension(v.dimension(i), v.is_offset(i), v.dim_value(i));
   }

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcWrf::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   clear_dimension();

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcWrf::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoNcWrf::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);
   out << "  Is_offset:\n";
   Is_offset.dump(out);
   out << "  Dim_value:\n";
   Dim_value.dump(out);

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcWrf::set_default_levels(const ConcatString &lstr) {
   Level.set_req_name("0,*,*");
   Level.set_name("0,*,*");
   add_dimension(0);
   add_dimension(vx_data2d_star);
   add_dimension(vx_data2d_star);
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcWrf::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   const char *method_name = "VarInfoNcWrf::set_magic() -> ";

   // Store the magic string
   set_magic_pre(nstr, lstr);

   // If there's no level specification, assume (0,*,*)
   // Parse the level specification
   parse_level(lstr);

   set_magic_post(req_name(), Level.req_name());

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcWrf::set_dict(Dictionary & dict, bool do_exit) {

   bool status = VarInfo::set_dict(dict);

   set_magic(dict.lookup_string("name"),
             dict.lookup_string("level"));
   set_req_name( dict.lookup_string("name").c_str() );

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcWrf::is_precipitation() const {

   //
   // Check set_attrs entry
   //
   int flag = SetAttrIsPrecipitation;
   if(!is_bad_data(flag)) return is_flag_set(flag);

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // precipitation variables.
   //
   return has_prefix(pinterp_precipitation_names,
                     n_pinterp_precipitation_names,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcWrf::is_specific_humidity() const {

   //
   // Check set_attrs entry
   //
   int flag = SetAttrIsSpecificHumidity;
   if(!is_bad_data(flag)) return is_flag_set(flag);

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // specific humidity variables.
   //
   return has_prefix(pinterp_specific_humidity_names,
                     n_pinterp_specific_humidity_names,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcWrf::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsUWind, WindInfo.u_wind);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   // Check if the VarInfo name is U or U<n> where <n> is an integer
   if(regex_match(Name.c_str(), regex("^U[0-9]*$"))) return true;

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // U-component of wind variables.
   //
   return has_prefix(pinterp_u_wind_names,
                     n_pinterp_u_wind_names,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcWrf::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsVWind, WindInfo.v_wind);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   // Check if the VarInfo name is V or V<n> where <n> is an integer
   if(regex_match(Name.c_str(), regex("^V[0-9]*$"))) return true;

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // V-component of wind variables.
   //
   return has_prefix(pinterp_v_wind_names,
                     n_pinterp_v_wind_names,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcWrf::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsWindSpeed, WindInfo.wind_speed);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // wind speed variables.
   //
   return has_prefix(pinterp_wind_speed_names,
                     n_pinterp_wind_speed_names,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcWrf::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsWindDirection, WindInfo.wind_direction);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   return false;
}

///////////////////////////////////////////////////////////////////////////////
