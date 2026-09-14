// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_ugrid.cc
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
#include "var_info_ugrid.h"

#include "vx_math.h"
#include "util_constants.h"
#include "vx_log.h"
#include "grib_strings.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoUGrid
//
///////////////////////////////////////////////////////////////////////////////

VarInfoUGrid::VarInfoUGrid() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoUGrid::~VarInfoUGrid() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoUGrid::VarInfoUGrid(const VarInfoUGrid &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoUGrid & VarInfoUGrid::operator=(const VarInfoUGrid &f) {

   if ( this == &f )  return *this;

   assign(f);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

unique_ptr<VarInfo> VarInfoUGrid::clone() const {
   return unique_ptr<VarInfo>(new VarInfoUGrid(*this));
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::assign(const VarInfoUGrid &v) {

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

void VarInfoUGrid::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoUGrid::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);
   out << "  Is_offset:\n";
   Is_offset.dump(out);
   out << "  Dim_value:\n";
   Dim_value.dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::set_default_levels(const ConcatString &lstr) {
   Level.set_req_name("0,*");
   Level.set_name("0,*");
   add_dimension(0);
   add_dimension(vx_data2d_star);
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   const char *method_name = "VarInfoUGrid::set_magic() -> ";

   // Store the magic string
   set_magic_pre(nstr, lstr);

   parse_level(lstr);

   // Assume None type (offset instead of pressure level) for a range of levels
   if (Level.type() == LevelType_Pres) Level.set_type(LevelType_None);   // like Ldd-dd

   set_magic_post(req_name(), Level.req_name());

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::set_dict(Dictionary &dict, bool do_exit){

   char lvl_type = ' ';
   ConcatString cfg_name = dict.lookup_string("name");
   ConcatString cfg_level = dict.lookup_string("level");
   const char * method_name = "VarInfoUGrid::set_dict() -> ";

   bool status = VarInfo::set_dict(dict);

   if (!cfg_level.empty()) lvl_type = cfg_level.char_at(0);
   if (lvl_type == 'A' || lvl_type == 'Z' || lvl_type == 'P' ||
       lvl_type == 'R' || lvl_type == 'L') {
      set_level_info_grib(dict);
      VarInfo::set_magic(cfg_name, cfg_level);
   }
   else set_magic(cfg_name, cfg_level);

   if (Level.lower() != Level.upper()) {
      ConcatString msg;
      msg << "\n" << method_name
          << "Multiple vertical levels ("
          << cfg_level  << ") for UGrid are not supported\n\n";
      handle_config_error(msg, do_exit);
      return false;
   }

   set_req_name(cfg_name.c_str());

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_precipitation() const {

   //
   // Check set_attrs entry
   //
   int flag = SetAttrIsPrecipitation;
   if(!is_bad_data(flag)) return is_flag_set(flag);

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any precipitation variables.
   //
   return has_prefix(grib_precipitation_abbr,
                     n_grib_precipitation_abbr,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_specific_humidity() const {

   //
   // Check set_attrs entry
   //
   int flag = SetAttrIsSpecificHumidity;
   if(!is_bad_data(flag)) return is_flag_set(flag);

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any specific humidity variables.
   //
   return has_prefix(grib_specific_humidity_abbr,
                     n_grib_specific_humidity_abbr,
                     Name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsUWind, WindInfo.u_wind);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   return is_grib_code_abbr_match(Name, ugrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsVWind, WindInfo.v_wind);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   return is_grib_code_abbr_match(Name, vgrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsWindSpeed, WindInfo.wind_speed);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   return is_grib_code_abbr_match(Name, wind_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   int flag = get_wind_flag(SetAttrIsWindDirection, WindInfo.wind_direction);
   if(!is_bad_data(flag)) return is_flag_set(flag);

   return is_grib_code_abbr_match(Name, wdir_grib_code);
}

///////////////////////////////////////////////////////////////////////////////
