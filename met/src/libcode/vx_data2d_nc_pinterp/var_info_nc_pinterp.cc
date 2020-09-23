// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nc_pinterp.cc
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
#include "var_info_nc_pinterp.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoNcPinterp
//
///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp::VarInfoNcPinterp() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp::~VarInfoNcPinterp() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp::VarInfoNcPinterp(const VarInfoNcPinterp &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcPinterp & VarInfoNcPinterp::operator=(const VarInfoNcPinterp &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::assign(const VarInfoNcPinterp &v) {
   int i;

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   Dimension.clear();
   for(i=0; i<v.n_dimension(); i++) Dimension.add(v.dimension(i));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   Dimension.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoNcPinterp::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::add_dimension(int dim) {
   Dimension.add(dim);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::set_dimension(int i_dim, int dim) {
   Dimension[i_dim] = dim;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::set_magic(const ConcatString &nstr, const ConcatString &lstr) {
   ConcatString tmp_str;
   char *ptr = (char *) 0, *ptr2 = (char *) 0, *ptr3 = (char *) 0, *save_ptr = (char *) 0;

   // Validate the magic string
   VarInfo::set_magic(nstr, lstr);

   // Store the magic string
   MagicStr << cs_erase << nstr << lstr;

   // Set the requested name and default output name
   set_req_name(nstr.c_str());
   set_name(nstr);

   // If there's no level specification, assume (0,*, *)
   if(strchr(lstr.c_str(), '(') == NULL) {
      Level.set_req_name("0,*,*");
      Level.set_name("0,*,*");
      Dimension.clear();
      Dimension.add(0);
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
                  mlog << Error << "\nVarInfoNcPinterp::set_magic() -> "
                       << "only one dimension can have a range for NetCDF variable \""
                       << MagicStr << "\".\n\n";
                  exit(1);
               }
               // Store the dimension of the range and limits
               else {
                  Dimension.add(range_flag);
                  Level.set_lower(atoi(ptr2));
                  Level.set_upper(atoi(++ptr3));

                  // Assume pressure level type for a range of levels
                  Level.set_type(LevelType_Pres);
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
   tmp_str.format("%s(%s)", name().text(), Level.name().text());
   set_long_name(tmp_str.c_str());

   // Set the units
   set_units(na_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcPinterp::set_dict(Dictionary & dict) {

   VarInfo::set_dict(dict);

   set_magic(dict.lookup_string("name"),
             dict.lookup_string("level"));
   set_req_name( dict.lookup_string("name").c_str() );

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_precipitation() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsPrecipitation)) {
      return(SetAttrIsPrecipitation != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // precipitation variables.
   //
   return(has_prefix(pinterp_precipitation_names,
                     n_pinterp_precipitation_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_specific_humidity() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsSpecificHumidity)) {
      return(SetAttrIsSpecificHumidity != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // specific humidity variables.
   //
   return(has_prefix(pinterp_specific_humidity_names,
                     n_pinterp_specific_humidity_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_u_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // U-component of wind variables.
   //
   return(has_prefix(pinterp_u_wind_names,
                     n_pinterp_u_wind_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_v_wind() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // V-component of wind variables.
   //
   return(has_prefix(pinterp_v_wind_names,
                     n_pinterp_v_wind_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_wind_speed() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // wind speed variables.
   //
   return(has_prefix(pinterp_wind_speed_names,
                     n_pinterp_wind_speed_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_wind_direction() const {

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindDirection)) {
      return(SetAttrIsWindDirection != 0);
   }

   return(false);
}


///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcPinterp::is_grid_relative() const {

   //
   // Check to see if the VarInfo name matches any of expected Pinterp
   // variables that should be rotated from grid-relative to earth-relative.
   //
   return(has_prefix(pinterp_grid_relative_names,
                     n_pinterp_grid_relative_names,
                     Name.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
