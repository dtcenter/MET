// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

static bool is_grib_code_abbr_match(const ConcatString &, int);

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

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
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
   int i;

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   clear_dimension();
   for(i=0; i<v.n_dimension(); i++) {
      add_dimension(v.dimension(i), v.is_offset(i), v.dim_value(i));
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
//   clear_dimension();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::clear_dimension() {
   Dimension.clear();
   Is_offset.clear();
   Dim_value.clear();
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

void VarInfoUGrid::add_dimension(int dim, bool as_offset, double dim_value) {
   Dimension.add(dim);
   Is_offset.add(as_offset);
   Dim_value.add(dim_value);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   ConcatString tmp_str;
   char *ptr = 0;
   char *ptr2 = 0;
   char *ptr3 = 0;
   char *save_ptr = 0;
   const char *method_name = "VarInfoUGrid::set_magic() -> ";

   // Store the magic string
   VarInfo::set_magic(nstr, lstr);

   // Set the requested name and default output name
   set_req_name(nstr.c_str());
   set_name(nstr);

   // If there's no level specification, assume (*, *)
   if(strchr(lstr.c_str(), '(') == nullptr) {
      Level.set_req_name("*,*");
      Level.set_name("*,*");
      clear_dimension();
      add_dimension(vx_data2d_star);
      add_dimension(vx_data2d_star);
   }
   else {

      // Initialize the temp string
      tmp_str = lstr;

      // Parse the level specification
      // Retreive the NetCDF level specification
      ptr = strtok_r((char*)tmp_str.c_str(), "()", &save_ptr);

      // Set the level name
      Level.set_req_name(ptr);
      Level.set_name(ptr);

      // If dimensions are specified, clear the default value
      if (strchr(ptr, ',') != nullptr) clear_dimension();

      // Parse the dimensions
      bool as_offset = true;
      while ((ptr2 = strtok_r(ptr, ",", &save_ptr)) != nullptr) {
         // Check for wildcards
         if (strchr(ptr2, '*') != nullptr) { add_dimension(vx_data2d_star);
         }
         else {
            as_offset = (*ptr2 != '@');
            if (!as_offset) ptr2++;

            // Check for a range of levels
            if ((ptr3 = strchr(ptr2, '-')) != nullptr) {

               // Check if a range has already been supplied
               if (Dimension.has(range_flag)) {
                  mlog << Error << "\n" << method_name
                       << "only one dimension can have a range for NetCDF variable \""
                       << MagicStr << "\".\n\n";
                  exit(1);
               }
               else {
                  // Store the dimension of the range and limits
                  *ptr3++ = 0;
                  add_dimension(range_flag, as_offset);
                  Level.set_lower(as_offset ? atoi(ptr2) : atof(ptr2));
                  Level.set_upper(as_offset ? atoi(ptr3) : atof(ptr3));

                  // Assume pressure level type for a range of levels
                  Level.set_type(LevelType_Pres);
                  Level.set_is_offset(as_offset);
               }
            }
            // Check for a range of times
            else if ((ptr3 = strchr(ptr2, ':')) != nullptr) {
               // Check if a range has already been supplied
               if (Dimension.has(range_flag)) {
                  mlog << Error << "\n" << method_name
                       << "only one dimension can have a range for NetCDF variable \""
                       << MagicStr << "\".\n\n";
                  exit(1);
               }
               else {
                  int increment = 1;
                  // Store the dimension of the range and limits
                  *ptr3++ = 0;
                  char *ptr_inc = strchr(ptr3, ':');
                  if (ptr_inc != nullptr) *ptr_inc++ = 0;
                  mlog << Debug(7) << method_name
                       << " start: " << ptr2 << ", end: " << ptr3 << "\n";

                  bool datestring_start = is_datestring(ptr2);
                  bool datestring_end   = is_datestring(ptr3);
                  if (datestring_start != datestring_end) {
                     mlog << Error << "\n" << method_name
                          << "the time value and an index/offset can not be mixed for NetCDF variable \""
                          << MagicStr << "\".\n\n";
                     exit(1);
                  }
                  if (datestring_start && datestring_end) as_offset = false;

                  unixtime time_lower = datestring_start
                                        ? timestring_to_unix(ptr2)
                                        : (as_offset ? atoi(ptr2) : atof(ptr2));
                  unixtime time_upper = datestring_end
                                        ? timestring_to_unix(ptr3)
                                        : (as_offset ? atoi(ptr3) : atof(ptr3));
                  if (ptr_inc != nullptr) {
                     if (as_offset) increment = atoi(ptr_inc);
                     else {
                        increment = is_float(ptr_inc)
                                    ? atof(ptr_inc) : timestring_to_sec(ptr_inc);
                        mlog << Debug(7) << method_name
                             << " increment: \"" << ptr_inc << "\" to "
                             << increment << " seconds.\n";
                     }
                  }

                  add_dimension(range_flag, as_offset);
                  Level.set_lower(time_lower);
                  Level.set_upper(time_upper);
                  Level.set_increment(increment);

                  // Assume time level type for a range of levels
                  Level.set_type(LevelType_Time);
                  Level.set_is_offset(as_offset);
               }
            }
            else {
               // Single level
               int level = 0;
               double level_value = bad_data_double;
               if (is_datestring(ptr2)) {
                  unixtime unix_time = timestring_to_unix(ptr2);
                  level = vx_data2d_dim_by_value;
                  level_value = unix_time;
                  as_offset = false;
               }
               else if (is_number(ptr2)) {
                  if (as_offset) level = atoi(ptr2);
                  else {
                     level = vx_data2d_dim_by_value;
                     level_value = atof(ptr2);
                  }
               }
               else if (is_datestring(ptr2)) {
                  unixtime unix_time = timestring_to_unix(ptr2);
                  level = vx_data2d_dim_by_value;
                  level_value = unix_time;
                  as_offset = false;
               }
               else {
                  mlog << Error << "\n" << method_name
                       << "trouble parsing NetCDF dimension value \""
                       << ptr2 << "\"!\n\n";
                  exit(1);
               }
               if (as_offset) add_dimension(level, as_offset);
               else add_dimension(level, as_offset, level_value);
            }
         }

         // Set ptr to nullptr for next call to strtok
         ptr = nullptr;

      } // end while

   } // end else

   // Check for "/PROB" to indicate a probability forecast
   if (strstr(MagicStr.c_str(), "/PROB") != nullptr) PFlag = 1;

   // Set the long name
   tmp_str.format("%s(%s)", req_name().text(), Level.req_name().text());
   set_long_name(tmp_str.c_str());

   // Set the units
   set_units(na_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoUGrid::set_dict(Dictionary &dict){

   VarInfo::set_dict(dict);

   set_magic(dict.lookup_string("name"),
             dict.lookup_string("level"));
   set_req_name(dict.lookup_string("name").c_str());

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_precipitation() const {

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

bool VarInfoUGrid::is_specific_humidity() const {

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

bool VarInfoUGrid::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   return(is_grib_code_abbr_match(Name, ugrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   return(is_grib_code_abbr_match(Name, vgrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   return(is_grib_code_abbr_match(Name, wind_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoUGrid::is_wind_direction() const {

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
