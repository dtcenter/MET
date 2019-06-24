// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nccf.cc
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
#include "var_info_nccf.h"

#include "vx_math.h"
#include "util_constants.h"
#include "vx_log.h"
#include "grib_strings.h"

///////////////////////////////////////////////////////////////////////////////

static bool is_grib_code_abbr_match(const ConcatString &, int);

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

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
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
   int i;

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   Dimension.clear();
   for(i=0; i<v.n_dimension(); i++) Dimension.add(v.dimension(i));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   Dimension.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoNcCF::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::add_dimension(int dim) {
   Dimension.add(dim);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   ConcatString tmp_str;
   char *ptr = 0;
   char *ptr2 = 0;
   char *ptr3 = 0;
   char *save_ptr = 0;
   const char *method_name = "VarInfoNcCF::set_magic() -> ";

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
      if (strchr(ptr, ',') != NULL) Dimension.clear();

      bool as_offset = true;
      // Parse the dimensions
      while ((ptr2 = strtok_r(ptr, ",", &save_ptr)) != NULL)
      {
         // Check for wildcards
         if (strchr(ptr2, '*') != NULL) {
            Dimension.add(vx_data2d_star);
         }
         else
         {
            // Check for a range of levels
            if ((ptr3 = strchr(ptr2, '-')) != NULL)
            {
               // Check if a range has already been supplied
               if (Dimension.has(range_flag))
               {
                  mlog << Error << "\n" << method_name
                       << "only one dimension can have a range for NetCDF variable \""
                       << MagicStr << "\".\n\n";
                  exit(1);
               }
               else
               {
                  // Store the dimension of the range and limits
                  Dimension.add(range_flag);
                  Level.set_lower(atoi(ptr2));
                  Level.set_upper(atoi(++ptr3));

                  // Assume pressure level type for a range of levels
                  Level.set_type(LevelType_Pres);
               }
            }
            // Check for a range of times
            else if ((ptr3 = strchr(ptr2, ':')) != NULL) {
               // Check if a range has already been supplied
               if (Dimension.has(range_flag))
               {
                  mlog << Error << "\nVarInfoNcCF::set_magic() -> "
                       << "only one dimension can have a range for NetCDF variable \""
                       << MagicStr << "\".\n\n";
                  exit(1);
               }
               else
               {
                  int increment = 0;
                  // Store the dimension of the range and limits
                  *ptr3++ = 0;
                  char *ptr_inc = strchr(ptr3, ':');
                  if (ptr_inc != NULL) *ptr_inc++ = 0;
                  mlog << Debug(7) << method_name
                       << " start: " << ptr2 << ", end: " << ptr3 << "\n";
                       
                  bool datestring_start = is_datestring(ptr2);
                  bool datestring_end   = is_datestring(ptr3);
                  unixtime time_lower = datestring_start
                      ? timestring_to_unix(ptr2) : atoi(ptr2);
                  unixtime time_upper = datestring_end
                      ? timestring_to_unix(ptr3) : atoi(ptr3);
                  if (ptr_inc != NULL) {
                    if (datestring_end && datestring_start) {
                      increment = timestring_to_sec(ptr_inc);
                      mlog << Debug(7) << method_name
                           << " increment: \"" << ptr_inc << "\" to "
                           << increment << " seconds.\n";
                    }
                    else increment = atoi(ptr_inc);
                  }
                  
                  Dimension.add(range_flag);
                  Level.set_lower(time_lower);
                  Level.set_upper(time_upper);
                  Level.set_increment(increment);

                  // Assume time level type for a range of levels
                  Level.set_type(LevelType_Time);
                  if (datestring_end && datestring_start)
                    as_offset = false;
               }
            }
            else
            {
               // Single level
               int level = atoi(ptr2);
               if (is_datestring(ptr2)) {
                  unixtime unix_time = timestring_to_unix(ptr2);
                  // Put unix time (from yyyymmdd[_hhmmss])
                  //if (unix_time > 0) {
                    level = unix_time;
                  //}
                  as_offset = false;
               }
               Dimension.add(level);
            }
         }

         // Set ptr to NULL for next call to strtok
         ptr = NULL;

      } // end while
      Level.set_time_as_offset(as_offset);

   } // end else

   // Check for "/PROB" to indicate a probability forecast
   if (strstr(MagicStr.c_str(), "/PROB") != NULL) PFlag = 1;

   // Set the long name
   tmp_str.format("%s(%s)", req_name().text(), Level.req_name().text());
   set_long_name(tmp_str.c_str());

   // Set the units
   set_units(na_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcCF::set_dict(Dictionary &dict){

   VarInfo::set_dict(dict);

   set_magic(dict.lookup_string("name"),
             dict.lookup_string("level"));
   set_req_name(dict.lookup_string("name").c_str());

   // Check for a probability boolean setting
   if(dict.lookup_bool(conf_key_prob, false)) {
      set_p_flag(true);
      return;
   }
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_precipitation() const {

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any precipitation variables.
   //
   return(has_prefix(grib_precipitation_abbr,
                     n_grib_precipitation_abbr,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_specific_humidity() const {

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any specific humidity variables.
   //
   return(has_prefix(grib_specific_humidity_abbr,
                     n_grib_specific_humidity_abbr,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_u_wind() const {
   return(is_grib_code_abbr_match(Name, ugrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_v_wind() const {
   return(is_grib_code_abbr_match(Name, vgrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_wind_speed() const {
   return(is_grib_code_abbr_match(Name, wind_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcCF::is_wind_direction() const {
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
