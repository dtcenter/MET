// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
   PTV     = v.ptv();
   Code    = v.code();
   LvlType = v.lvl_type();
   PCode   = v.p_code();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   PTV     = default_grib_ptv;
   Code    = bad_data_int;
   LvlType = bad_data_int;
   PCode   = bad_data_int;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoGrib::dump():\n"
       << "  PTV     = " << PTV     << "\n"
       << "  Code    = " << Code    << "\n"
       << "  LvlType = " << LvlType << "\n"
       << "  PCode   = " << PCode   << "\n";

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_ptv(int v) {
   PTV = v;
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

void VarInfoGrib::set_pair(const ConcatString &key, const ConcatString &val) {

   // First call the parent's set_pair function.
   VarInfo::set_pair(key, val);

   // Look for GRIB keywords.
   if(strcasecmp(key, CONFIG_GRIB_PTV    ) == 0) { PTV     = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB_Code   ) == 0) { Code    = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB_LvlType) == 0) { LvlType = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB_PCode  ) == 0) { PCode   = atoi(val); }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_magic(const ConcatString & s) {
   ConcatString tmp_str, tmp2_str, tmp3_str;
   char *ptr = (char *) 0, *ptr2 = (char *) 0, *save_ptr = (char *) 0;
   double prob_lo, prob_hi, tmp_dbl;
   char junk[max_str_len];
   char junk2[max_str_len];
   LevelType lt;

   // Validate the magic string
   VarInfo::set_magic(s);

   // Store the magic string
   MagicStr = s;

   // Initialize the temp string
   memset(junk, 0, sizeof(junk));
   strncpy(junk, (const char *) s, sizeof(junk) - 1);

   // Retreive the GRIB code value
   if((ptr = strtok_r(junk, "/", &save_ptr)) == NULL) {
      mlog << Error << "\nVarInfoGrib::set_magic() -> "
           << "bad GRIB code specified \""
           << s << "\".\n\n";
      exit(1);
   }

   // Store the code value and parse any probability info
   Code = str_to_grib_code(ptr, PCode, prob_lo, prob_hi);

   // Retrieve the level value
   if((ptr = strtok_r(NULL, "/", &save_ptr)) == NULL) {
      mlog << Error << "\nVarInfoGrib::set_magic() -> "
           << "each GRIB code specified must be followed by an "
           << "accumulation, level, or presssure level indicator \""
           << s << "\".\n\n";
      exit(1);
   }

   // Check the level indicator type
   if(*ptr != 'A' && *ptr != 'Z' &&
      *ptr != 'P' && *ptr != 'R' &&
      *ptr != 'L') {
      mlog << Error << "\nVarInfoGrib::set_magic() -> "
           << "each GRIB code specified (" << s
           << ") must be followed by level information "
           << "that begins with:\n"
           << "\t\'A\' for an accumulation interval\n"
           << "\t\'Z\' for a vertical level\n"
           << "\t\'P\' for a pressure level\n"
           << "\t\'R\' for a record number\n"
           << "\t\'L\' for a generic level\n\n";
      exit(1);
   }

   // Set the level type
   if(      *ptr == 'A') lt = LevelType_Accum;
   else if (*ptr == 'Z') lt = LevelType_Vert;
   else if (*ptr == 'P') lt = LevelType_Pres;
   else if (*ptr == 'R') lt = LevelType_RecNumber;
   else if (*ptr == 'L') lt = LevelType_None;
   else                  lt = LevelType_None;
   Level.set_type(lt);

   // Store the level name
   Level.set_req_name(ptr);
   Level.set_name(ptr);

   // Advance the pointer past the 'A', 'Z', 'P', 'R', or 'L'
   ptr++;

   // For accumulation intervals store the level as the number of seconds
   if(lt == LevelType_Accum) Level.set_lower(timestring_to_sec(ptr));
   else                      Level.set_lower(atoi(ptr));

   // Look for a '-' and a second level indicator
   ptr2 = strchr(ptr, '-');
   if(ptr2 != NULL) {
      if(lt == LevelType_Accum) Level.set_upper(timestring_to_sec(++ptr2));
      else                      Level.set_upper(atoi(++ptr2));
   }
   else{
      Level.set_upper(Level.lower());
   }

   // Allow ranges for Pressure, Vertical, and No level types.
   if(lt != LevelType_Pres &&
      lt != LevelType_Vert &&
      lt != LevelType_None &&
      !is_eq(Level.lower(), Level.upper())) {
      mlog << Error << "\nVarInfoGrib::set_magic() -> "
           << "ranges of levels are only supported for pressure levels "
           << "(P), vertical levels (Z), and generic levels (L).\n\n";
      exit(1);
   }

   // For pressure levels, check the order of the upper and lower limits
   // and define lower < upper
   if(lt == LevelType_Pres) {

      // If the levels are the same, reset the level name
      if(is_eq(Level.lower(), Level.upper())) {
         tmp2_str << cs_erase << 'P' << nint(Level.lower());
         Level.set_req_name(tmp2_str);
         Level.set_name(tmp2_str);
      }
      // Switch Level.lower() and Level.upper()
      else if(Level.lower() > Level.upper()) {
         tmp_dbl = Level.lower();
         Level.set_lower(Level.upper());
         Level.set_upper(tmp_dbl);
      }
      // Reset the level name to be high - low
      else {
         tmp2_str << cs_erase << 'P' << nint(Level.upper()) << '-' << nint(Level.lower());
         Level.set_req_name(tmp2_str);
         Level.set_name(tmp2_str);
      }
   }

   // Check for "/PROB" to indicate a probability forecast
   if((ptr = strtok_r(NULL, "/", &save_ptr)) != NULL) {

      if(strncasecmp(ptr, "PROB", strlen("PROB")) == 0) PFlag = 1;
      else {
         mlog << Warning << "\nVarInfoGrib::set_magic() -> "
              << "unrecognized flag value \"" << ptr
              << "\" for GRIB code \"" << s << "\".\n";
      }
   }

   // Set the name
   tmp_str = get_grib_code_abbr(Code, PTV);
   set_req_name(tmp_str);

   if(Level.type() == LevelType_Accum) {
      int intAccum = nint(Level.upper());

      // For an hourly accumulation interval, append _HH
      if(intAccum % sec_per_hour == 0) {
         tmp_str << '_' << HH(intAccum/sec_per_hour);
      }

      // For any other accumulation interval, append _HHMMSS
      else {
         tmp_str << '_' << sec_to_hhmmss(intAccum);
      }
   }
   set_name(tmp_str);

   // Set the long name
   tmp_str = get_grib_code_name(Code, PTV);
   set_long_name(tmp_str);

   // Set the units
   tmp_str = get_grib_code_unit(Code, PTV);
   set_units(tmp_str);

   // For a non-zero accumulation interval, append a timestring
   if(lt == LevelType_Accum && Level.lower() > 0) {

      // Append the accumulation interval, _HH or _HHMMSS
      if( (int) Level.lower() % sec_per_hour == 0)
         tmp2_str << cs_erase << HH((int) Level.lower()/sec_per_hour);
      else
         tmp2_str = sec_to_hhmmss(nint(Level.lower()));

      // Store the abbreviation string
      tmp3_str << cs_erase << tmp_str << '_' << tmp2_str;
      tmp_str = tmp3_str;
   }

   // For probability fields, append probability information
   if(PFlag) {

      // Set the probability name
      tmp3_str = get_grib_code_abbr(PCode, PTV);
      set_p_name(tmp3_str);

      // Set the probability units
      tmp3_str = get_grib_code_unit(PCode, PTV);
      set_p_units(tmp3_str);

      // Both thresholds specified
      if(!is_bad_data(prob_lo) && !is_bad_data(prob_hi)) {
         PThreshLo.set(prob_lo, thresh_gt);
         PThreshHi.set(prob_hi, thresh_lt);
         sprintf(junk2, "%s(%.5f<%s<%.5f)", name().text(), prob_lo, p_name().text(), prob_hi);
         tmp_str = junk2;
      }
      // Lower threshold specified
      else if(!is_bad_data(prob_lo) && is_bad_data(prob_hi)) {
         PThreshLo.set(prob_lo, thresh_gt);
         sprintf(junk2, "%s(%s>%.5f)", name().text(), p_name().text(), prob_lo);
         tmp_str = junk2;
      }
      // Upper threshold specified
      else if(is_bad_data(prob_lo) && !is_bad_data(prob_hi)) {
         PThreshHi.set(prob_hi, thresh_gt);
         sprintf(junk2, "%s(%s<%.5f)", name().text(), p_name().text(), prob_hi);
         tmp_str = junk2;
      }
   }
   else {
      tmp_str = name();
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_dict(Dictionary & dict) {

   double prob_lo, prob_hi;
   LevelType lt;

   //  store the fcst/obs dictionary
   Dict = dict;

   //  determine the GRIB1 parameter table version, defaulting to 2
   int ptv = dict.lookup_int("GRIB1_ptv");
   PTV = (bad_data_int == ptv? 2 : ptv);

   //  store the code value and parse any probability info
   ConcatString parm = dict.lookup_string("name", false);
   if( "" != parm )  Code = str_to_grib_code(parm, PCode, prob_lo, prob_hi);
   else              Code = dict.lookup_int("GRIB1_rec");


}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_precipitation() const {
   int i;
   bool status = false;

   //
   // The ReqName member contains the requested GRIB code abbreviation.
   // Check to see if it matches the GRIB precipitation abbreviations.
   //
   for(i=0; i<n_grib_precipitation_abbr; i++) {
      if(strcmp(ReqName, grib_precipitation_abbr[i]) == 0) {
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
   // The ReqName meber contains the requested GRIB code abbreviation.
   // Check to see if it matches the GRIB specific humidity abbreviations.
   //
   for(i=0; i<n_grib_specific_humidity_abbr; i++) {
      if(strcmp(ReqName, grib_specific_humidity_abbr[i]) == 0) {
         status = true;
         break;
      }
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_u_wind() const {
   return(Code == ugrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_v_wind() const {
   return(Code == vgrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_wind_speed() const {
   return(Code == wind_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_wind_direction() const {
   return(Code == wdir_grib_code);
}

///////////////////////////////////////////////////////////////////////////////
