// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

using namespace std;

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

///////////////////////////////////////////////////////////////////////////////

static bool is_grib_code_abbr_match(const ConcatString &, int);

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

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
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
   char *ptr = (char *) 0, *ptr2 = (char *) 0, *ptr3 = (char *) 0, *save_ptr = (char *) 0;

   // Validate the magic string
   VarInfo::set_magic(nstr, lstr);

   // Store the magic string
   MagicStr << cs_erase << nstr << lstr;

   // Set the requested name and default output name
   set_req_name(nstr.c_str());
   set_name(nstr);

   // If there's no level specification, assume (*, *)
   if(strchr(lstr.c_str(), '(') == NULL) {
      Level.set_req_name("*,*");
      Level.set_name("*,*");
      Dimension.clear();
      Dimension.add(vx_data2d_star);
      Dimension.add(vx_data2d_star);
   }
   // Parse the level specification
   else {

      // Initialize the temp string
      tmp_str = lstr;

      // Retreive the NetCDF level specification
      ptr = strtok_r((char*)tmp_str.c_str(), "()", &save_ptr);

      // Set the level name
      Level.set_req_name(ptr);
      Level.set_name(ptr);

      // If dimensions are specified, clear the default value
      if(strchr(ptr, ',') != NULL) Dimension.clear();

      // Parse the dimensions
      while((ptr2 = strtok_r(ptr, ",", &save_ptr)) != NULL) {

         // Check for wildcards
         if(strchr(ptr2, '*') != NULL) Dimension.add(vx_data2d_star);
         else {

            // Check for a range of levels
            if((ptr3 = strchr(ptr2, '-')) != NULL) {

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

         // Set ptr to NULL for next call to strtok
         ptr = NULL;
      } // end while

   } // end else

   // Check for "/PROB" to indicate a probability forecast
   if(strstr(MagicStr.c_str(), "/PROB") != NULL) PFlag = 1;

   // Set the long name
   tmp_str.format("%s(%s)", req_name().text(), Level.req_name().text());
   set_long_name(tmp_str.c_str());

   // Set the units
   set_units(na_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::set_dict(Dictionary &dict){

   VarInfo::set_dict(dict);

   set_magic(dict.lookup_string("name"),
             dict.lookup_string("level"));
   set_req_name(dict.lookup_string("name").c_str());

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
   return(has_prefix(grib_precipitation_abbr,
                     n_grib_precipitation_abbr,
                     Name.c_str()));
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
   return(has_prefix(grib_specific_humidity_abbr,
                     n_grib_specific_humidity_abbr,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   return(is_grib_code_abbr_match(Name, ugrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   return(is_grib_code_abbr_match(Name, vgrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   return(is_grib_code_abbr_match(Name, wind_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindDirection)) {
      return(SetAttrIsWindDirection != 0);
   }

   return(is_grib_code_abbr_match(Name, wdir_grib_code));
}

///////////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous utility functions
//
///////////////////////////////////////////////////////////////////////////////

bool is_grib_code_abbr_match(const ConcatString &str, int grib_code) {
   ConcatString abbr_str;
   bool match = false;

   if(str.empty()) return(false);

   //
   // Use the default GRIB1 parameter table version number 2
   //
   abbr_str = get_grib_code_abbr(grib_code, 2);

   //
   // Consider it a match if the search string begins with the GRIB code
   // abbreviation, ignoring case.
   //
   if(strncasecmp(str.c_str(), abbr_str.c_str(), abbr_str.length()) == 0) match = true;

   return(match);
}

///////////////////////////////////////////////////////////////////////////////
