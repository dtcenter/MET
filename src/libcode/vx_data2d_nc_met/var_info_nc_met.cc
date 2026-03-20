// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nc_met.cc
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
#include "var_info_nc_met.h"

#include "vx_math.h"
#include "util_constants.h"
#include "vx_log.h"
#include "grib_strings.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoNcMet
//
///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet::VarInfoNcMet() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet::~VarInfoNcMet() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet::VarInfoNcMet(const VarInfoNcMet &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet & VarInfoNcMet::operator=(const VarInfoNcMet &f) {

   if ( this == &f )  return *this;

   assign(f);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

VarInfo *VarInfoNcMet::clone() const {

   VarInfoNcMet *ret = new VarInfoNcMet(*this);

   return (VarInfo *)ret;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::assign(const VarInfoNcMet &v) {
   int i;

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   Dimension.clear();
   for(i=0; i<v.n_dimension(); i++) Dimension.add(v.dimension(i));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   Dimension.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::dump(ostream &out) const {

   VarInfo::dump(out);

   // Dump out the contents
   out << "VarInfoNcMet::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::add_dimension(int dim) {
   Dimension.add(dim);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   ConcatString tmp_str;
   char *ptr = (char *) nullptr;
   char *ptr2 = (char *) nullptr;
   char *ptr3 = (char *) nullptr;
   char *save_ptr = (char *) nullptr;

   // Store the magic string
   VarInfo::set_magic(nstr, lstr);

   // Set the requested name and default output name
   set_req_name(nstr.c_str());
   set_name(nstr);

   // Parse the level dimensions, if specified
   if(lstr.string().find_first_of("(") != std::string::npos) {

      // Initialize the temp string
      tmp_str = lstr;

      // Retreive the NetCDF level specification
      ptr = strtok_r((char*)tmp_str.c_str(), "()", &save_ptr);

      // Set the level name
      Level.set_req_name(ptr);
      Level.set_name(ptr);

      // If dimensions are specified, clear the default value
      if(strchr(ptr, ',') != nullptr) Dimension.clear();

      // Parse the dimensions
      while((ptr2 = strtok_r(ptr, ",", &save_ptr)) != nullptr) {

         // Check for wildcards
         if(strchr(ptr2, '*') != nullptr) Dimension.add(vx_data2d_star);
         else {

            // @value notation not supported in vx_data_nc_met
            if(*ptr2 == '@') {
               mlog << Warning << "\nVarInfoNcMet::set_magic() -> "
                    << "problem parsing \"" << MagicStr << "\" for the vx_data2d_nc_met library.\n"
                    << "NetCDF dimensions must be specified as 0-based integer indices rather "
                    << "than using the NetCDF dimension \"@value\" notation.\n\n";
            }

            // Check for a range of levels
            if((ptr3 = strchr(ptr2, '-')) != nullptr) {

               // Check if a range has already been supplied
               if(Dimension.has(range_flag)) {
                  mlog << Error << "\nVarInfoNcMet::set_magic() -> "
                       << "only one dimension can have a range for NetCDF variable \""
                       << MagicStr << "\".\n\n";
                  exit(1);
               }
               // Store the dimension of the range and limits
               else {
                  Dimension.add(range_flag);
                  Level.set_lower(atoi(ptr2));
                  Level.set_upper(atoi(++ptr3));
               }
            }
            // Single level
            else {
               Dimension.add(atoi(ptr2));
            }
         }

         // Set ptr to nullptr for next call to strtok
         ptr = nullptr;
      } // end while
   }
   // Otherwise, assume (*,*) level dimensions
   else {

      // MET #3087 Set the level name string:
      //   - If empty or '*' from Point2Grid, set to *,* to indicate gridded output
      //   - If nonempty, use the input level string to support U/V vector level matching
      if(lstr.empty() || lstr == "*") {
         Level.set_req_name("*,*");
         Level.set_name("*,*");
      }
      else {     
         Level.set_req_name(lstr.c_str());
         Level.set_name(lstr.c_str());
      }
      Dimension.clear();
      Dimension.add(vx_data2d_star);
      Dimension.add(vx_data2d_star);
   }

   // Check for "/PROB" to indicate a probability forecast
   if(strstr(MagicStr.c_str(), "/PROB") != nullptr) PFlag = 1;

   // Set the long name
   tmp_str.format("%s(%s)", req_name().text(), Level.req_name().text());
   set_long_name(tmp_str.c_str());

   // Set the units
   set_units(na_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::set_dict(Dictionary &dict, bool do_exit){

   bool status = VarInfo::set_dict(dict, do_exit);
   set_magic(Name, Level.name());
   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_precipitation() const {

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

bool VarInfoNcMet::is_specific_humidity() const {

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

bool VarInfoNcMet::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   return is_grib_code_abbr_match(Name, ugrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   return is_grib_code_abbr_match(Name, vgrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   return is_grib_code_abbr_match(Name, wind_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindDirection)) {
      return(SetAttrIsWindDirection != 0);
   }

   return is_grib_code_abbr_match(Name, wdir_grib_code);
}

///////////////////////////////////////////////////////////////////////////////
