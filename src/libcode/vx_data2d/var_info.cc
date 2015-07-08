// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info.cc
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
#include <strings.h>

#include "var_info.h"

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_log.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfo
//
///////////////////////////////////////////////////////////////////////////////

VarInfo::VarInfo() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfo::~VarInfo() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfo::VarInfo(const VarInfo &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfo & VarInfo::operator=(const VarInfo &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::init_from_scratch() {

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::assign(const VarInfo &v) {

   // Copy
   MagicStr  = v.magic_str();
   ReqName   = v.req_name();
   Name      = v.name();
   LongName  = v.long_name();
   Units     = v.units();
   Level     = v.level();

   PFlag     = v.p_flag();
   PName     = v.p_name();
   PUnits    = v.p_units();
   PThreshLo = v.p_thresh_lo();
   PThreshHi = v.p_thresh_hi();

   VFlag     = v.v_flag();
   
   Init      = v.init();
   Valid     = v.valid();
   Lead      = v.lead();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::clear() {

   // Initialize
   MagicStr.clear();
   ReqName.clear();
   Name.clear();
   LongName.clear();
   Units.clear();
   Level.clear();

   PFlag = false;
   PName.clear();
   PUnits.clear();
   PThreshLo.clear();
   PThreshHi.clear();
   
   VFlag = false;
   
   Init  = (unixtime) 0;
   Valid = (unixtime) 0;
   Lead  = bad_data_int;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::dump(ostream &out) const {
   ConcatString init_str, valid_str, lead_str;

   init_str  = unix_to_yyyymmdd_hhmmss(Init);
   valid_str = unix_to_yyyymmdd_hhmmss(Valid);
   
   if(is_bad_data(Lead)) lead_str = na_str;
   else                  lead_str = sec_to_hhmmss(Lead);

   // Dump out the contents
   out << "VarInfo::dump():\n"
       << "  MagicStr = " << (MagicStr ? MagicStr.text() : "(nul)") << "\n"
       << "  ReqName  = " << (ReqName ? ReqName.text() : "(nul)") << "\n"
       << "  Name     = " << (Name ? Name.text() : "(nul)") << "\n"
       << "  LongName = " << (LongName ? LongName.text() : "(nul)") << "\n"
       << "  Units    = " << (Units ? Units.text() : "(nul)") << "\n"
       << "  PFlag    = " << PFlag << "\n"
       << "  PName    = " << (PName ? PName.text() : "(nul)") << "\n"
       << "  PUnits   = " << (PUnits ? PUnits.text() : "(nul)") << "\n"
       << "  VFlag    = " << VFlag << "\n"
       << "  Init     = " << init_str << " (" << Init << ")\n"
       << "  Valid    = " << valid_str << " (" << Valid << ")\n"
       << "  Lead     = " << lead_str << " (" << Lead << ")\n";

   Level.dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_req_name(const char *str) {
   ReqName = str;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_name(const char *str) {
   Name = str;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_long_name(const char *str) {
   LongName = str;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_units(const char *str) {
   Units = str;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_level_info(const LevelInfo &l) {
   Level = l;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_req_level_name(const char *str) {
   Level.set_req_name(str);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_level_name(const char *str) {
   Level.set_name(str);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_p_flag(bool f) {
   PFlag = f;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_p_name(const char *str) {
   PName = str;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_p_units(const char *str) {
   PUnits = str;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_p_thresh_lo(const SingleThresh &t) {
   PThreshLo = t;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_p_thresh_hi(const SingleThresh &t) {
   PThreshHi = t;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_v_flag(bool f) {
   VFlag = f;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_init(unixtime t) {
   Init = t;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_valid(unixtime t) {
   Valid = t;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_lead(int s) {
   Lead = s;
   return;
}

///////////////////////////////////////////////////////////////////////////////
 
void VarInfo::set_magic(const ConcatString &mag) {

   //  verify that there are no embedded spaces
   if( (unsigned int)mag.length() != strcspn(mag, " \t") ){
      mlog << Error << "\nVarInfo::set_magic() -> "
           << "embedded whitespace found in magic string \""
           << mag << "\".\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_dict(Dictionary &dict) {
   ConcatString s;
   
   // Set init time, if present
   s = dict.lookup_string(conf_key_init_time, false);
   if(dict.last_lookup_status()) set_init(timestring_to_unix(s));

   // Set valid time, if present
   s = dict.lookup_string(conf_key_valid_time, false);
   if(dict.last_lookup_status()) set_valid(timestring_to_unix(s));

   // Set lead time, if present
   s = dict.lookup_string(conf_key_lead_time, false);
   if(dict.last_lookup_status()) set_lead(timestring_to_sec(s));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_level_info_grib(Dictionary & dict){

   ConcatString field_level = dict.lookup_string(conf_key_level, false);
   LevelType lt;
   string lvl_type, lvl_val1, lvl_val2;
   double lvl1 = -1, lvl2 = -1;


   //  if the level string is specified, use it
   if( ! field_level.empty() ){

      //  parse the level string components
      int num_mat = 0;
      char** mat = NULL;
      const char* pat_mag = "([ALPRZ])([0-9\\.]+)(\\-[0-9\\.]+)?";
      if( 3 > (num_mat = regex_apply(pat_mag, 4, field_level.text(), mat)) ){
         mlog << Error << "\nVarInfo::set_level_info_grib() - failed to parse level string '"
              << field_level << "'\n\n";
         exit(1);
      }
      lvl_type = mat[1];
      lvl_val1 = mat[2];
      lvl1 = atof(lvl_val1.data());
      if( 4 == num_mat ){
         lvl_val2 = mat[3];
         lvl2 = atof( lvl_val2.substr(1, lvl_val2.length() - 1).data() );
      }
      regex_clean(mat);

      //  set the level type based on the letter abbreviation
      if      (lvl_type == "A") lt = LevelType_Accum;
      else if (lvl_type == "Z") lt = LevelType_Vert;
      else if (lvl_type == "P") lt = LevelType_Pres;
      else if (lvl_type == "R") lt = LevelType_RecNumber;
      else if (lvl_type == "L") lt = LevelType_None;
      else                      lt = LevelType_None;

   }

   //  if the field level is not specified, look for an use indexes
   else {

      //  read the level index information
      int    field_lvl_typ  = dict.lookup_int   (conf_key_GRIB_lvl_typ, false);
      double field_lvl_val1 = dict.lookup_double(conf_key_GRIB_lvl_val1, false);
      double field_lvl_val2 = dict.lookup_double(conf_key_GRIB_lvl_val2, false);

      //  if the level index information is not specified, bail
      if( is_bad_data(field_lvl_typ) ||
          is_bad_data(field_lvl_val1) ){
         mlog << Error << "\nVarInfo::set_level_info_grib() - either level or GRIB_lvl_typ, "
              << "GRIB_lvl_val1 and GRIB_lvl_val2 (if necessary) must be specified in field "
              << "information\n\n";
         exit(1);
      }

      //  set the level value strings
      lvl_val1 = str_format("%f", field_lvl_val1);
      lvl_val2 = str_format("%f", field_lvl_val2);

      lvl1 = field_lvl_val1;
      lvl2 = (is_bad_data(field_lvl_val2) ? -1 : field_lvl_val2);

      //  set the level type based on the indexes
      //  from: http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table4-5.shtml
      //        http://www.nco.ncep.noaa.gov/pmb/docs/on388/table3.html
      switch( field_lvl_typ ){
         case 1:
            lt = LevelType_Accum; lvl_type = "A"; break;
         case 100: case 108:
            lt = LevelType_Pres;  lvl_type = "P"; break;
         case 102: case 103:
            lt = LevelType_Vert;  lvl_type = "Z"; break;
         default:
            lt = LevelType_None;  lvl_type = "L"; break;
      }

   }

   //  arrange the level values appropriately
   lvl2 = ( !is_eq(lvl2, lvl1) ? lvl2 : -1.0 );
   if( lt == LevelType_Pres && !is_eq(lvl2, -1.0) && lvl2 < lvl1 ){
      double lvl_tmp = lvl2;
      lvl2 = lvl1;
      lvl1 = lvl_tmp;
   }

   //  format the level name:
   //  for accumulation intervals, use the full time string
   //  otherwise, just the numeric value
   ConcatString lvl_name;
   if( lt == LevelType_Accum )   lvl_name.format("%s%s",    lvl_type.data(), lvl_val1.data());
   else if( !is_eq(lvl2, -1.0) ) lvl_name.format("%s%d-%d", lvl_type.data(), (int)lvl2, (int)lvl1);
   else                          lvl_name.format("%s%d",    lvl_type.data(), (int)lvl1);

   //  set the level information
   Level.set_type(lt);
   Level.set_req_name(lvl_name);
   Level.set_name(lvl_name);

   //  set the lower limit
   if(lt == LevelType_Accum) Level.set_lower(timestring_to_sec( lvl_val1.data() ));
   else                      Level.set_lower(lvl1);

   //  if pressure ranges are not supported for the specified level type, bail
   if( !is_eq(lvl2, -1.0) && lt != LevelType_Pres && lt != LevelType_Vert && lt != LevelType_None ){
      mlog << Error << "\nVarInfo::set_level_info_grib() - "
           << "ranges of levels are only supported for pressure levels "
           << "(P), vertical levels (Z), and generic levels (L)\n\n";
      exit(1);
   }

   //  set the upper level value
   if(lt == LevelType_Accum) Level.set_upper(timestring_to_sec( lvl_val1.data() ));
   else                      Level.set_upper(is_eq(lvl2, -1.0) ? lvl1 : lvl2);

   //  if the field name is APCP, apply additional formatting
   ConcatString field_name = name();
   if( field_name == "APCP" ){
      int accum = atoi( sec_to_hhmmss( (int)Level.lower() ).text() );
      if( 0 == accum % 10000 ) set_name( str_format("%s_%02d", field_name.text(), accum/10000) );
      else                     set_name( str_format("%s_%06d", field_name.text(), accum)       );
   }

   //  set the magic string
   MagicStr = str_format("%s/%s", field_name.text(), Level.name().text());

   //  set the level type number, if specified
   Level.set_type_num(dict.lookup_int(conf_key_GRIB_lvl_typ, false));

}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_prob_info_grib(ConcatString prob_name, double thresh_lo, double thresh_hi){

   //  verify the probability thresholds
   if( is_eq(bad_data_double, thresh_lo) && is_eq(bad_data_double, thresh_hi) ){
      mlog << Error << "\nVarInfo::set_prob_info_grib() - at least one probability threshold "
           << "(thresh_lo and/or thresh_hi) must be defined\n\n";
      exit(1);
   }

   //  build and set threshold objects
   SingleThresh thr_lo, thr_hi;
   thr_lo.set(thresh_lo, is_bad_data(thresh_lo) ? thresh_na : thresh_gt);
   set_p_thresh_lo(thr_lo);
   thr_hi.set(thresh_hi, is_bad_data(thresh_hi) ? thresh_na : thresh_gt);
   set_p_thresh_hi(thr_hi);

   //  if the prob field name is APCP, apply additional formatting
   if( prob_name == "APCP" ){
      int accum = atoi( sec_to_hhmmss( (int)Level.lower() ).text() );
      if( 0 == accum % 10000 ) prob_name = str_format("%s_%02d", prob_name.text(), accum/10000);
      else                     prob_name = str_format("%s_%06d", prob_name.text(), accum);
   }

   //  build the corresponding field name and magic string
   ConcatString field_name;
   if( thresh_na != thr_lo.get_type() && thresh_na != thr_hi.get_type() ){
      field_name = str_format("PROB(%s%s%s)",
                              str_format("%.3f%s", thr_lo.get_value(), thresh_type_str[thr_lo.get_type()]).text(),
                              prob_name.text(),
                              str_format("%s%.3f", thresh_type_str[thr_hi.get_type()], thr_hi.get_value()).text());
      MagicStr = str_format("%s/%s/PROB", field_name.text(), Level.name().text());
   } else {
      SingleThresh thr( thresh_na != thr_lo.get_type() ? thr_lo : thr_hi );
      field_name = str_format("PROB(%s%s)",
                            prob_name.text(),
                            str_format("%s%.3f", thresh_type_str[thr.get_type()], thr.get_value()).text());
      MagicStr = str_format("%s/%s/PROB", field_name.text(), Level.name().text());
   }
   set_name     ( field_name );
   set_req_name ( field_name );

}

///////////////////////////////////////////////////////////////////////////////

