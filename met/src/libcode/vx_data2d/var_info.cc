// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

ConcatString parse_set_attr_string(Dictionary &dict, const char *key, bool check_ws=false);
int          parse_set_attr_flag  (Dictionary &dict, const char *key);

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
   MagicStr  = v.MagicStr;
   ReqName   = v.ReqName;
   Name      = v.Name;
   Units     = v.Units;
   Level     = v.Level;
   LongName  = v.LongName;
   Ensemble  = v.Ensemble;

   PFlag     = v.PFlag;
   PName     = v.PName;
   PUnits    = v.PUnits;
   PThreshLo = v.PThreshLo;
   PThreshHi = v.PThreshHi;
   PAsScalar = v.PAsScalar;

   UVIndex   = v.UVIndex;

   Init      = v.Init;
   Valid     = v.Valid;
   Lead      = v.Lead;

   ConvertFx = v.ConvertFx;

   CensorThresh = v.CensorThresh;
   CensorVal    = v.CensorVal;

   nBins = v.nBins;
   Range = v.Range;

   Regrid = v.Regrid;

   SetAttrName = v.SetAttrName;
   SetAttrUnits = v.SetAttrUnits;
   SetAttrLevel = v.SetAttrLevel;
   SetAttrLongName = v.SetAttrLongName;

   SetAttrGrid = v.SetAttrGrid;

   SetAttrInit = v.SetAttrInit;
   SetAttrValid = v.SetAttrValid;
   SetAttrLead = v.SetAttrLead;
   SetAttrAccum = v.SetAttrAccum;

   SetAttrIsPrecipitation = v.SetAttrIsPrecipitation;
   SetAttrIsSpecificHumidity = v.SetAttrIsSpecificHumidity;
   SetAttrIsUWind = v.SetAttrIsUWind;
   SetAttrIsVWind = v.SetAttrIsVWind;
   SetAttrIsGridRelative = v.SetAttrIsGridRelative;
   SetAttrIsWindSpeed = v.SetAttrIsWindSpeed;
   SetAttrIsWindDirection = v.SetAttrIsWindDirection;
   SetAttrIsProb = v.SetAttrIsProb;

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
   PAsScalar = false;

   UVIndex = -1;

   Init  = (unixtime) 0;
   Valid = (unixtime) 0;
   Lead  = bad_data_int;
   Ensemble.clear ();

   ConvertFx.clear();

   CensorThresh.clear();
   CensorVal.clear();

   nBins = 0;
   Range.clear();

   Regrid.clear();

   SetAttrName.clear();
   SetAttrUnits.clear();
   SetAttrLevel.clear();
   SetAttrLongName.clear();

   SetAttrGrid.clear();

   SetAttrInit = (unixtime) 0;
   SetAttrValid = (unixtime) 0;
   SetAttrLead = bad_data_int;
   SetAttrAccum = bad_data_int;

   SetAttrIsPrecipitation = bad_data_int;
   SetAttrIsSpecificHumidity = bad_data_int;
   SetAttrIsUWind = bad_data_int;
   SetAttrIsVWind = bad_data_int;
   SetAttrIsGridRelative = bad_data_int;
   SetAttrIsWindSpeed = bad_data_int;
   SetAttrIsWindDirection = bad_data_int;
   SetAttrIsProb = bad_data_int;

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
       << "  MagicStr     = " << MagicStr.contents() << "\n"
       << "  ReqName      = " << ReqName.contents() << "\n"
       << "  Name         = " << Name.contents() << "\n"
       << "  LongName     = " << LongName.contents() << "\n"
       << "  Units        = " << Units.contents() << "\n"
       << "  PFlag        = " << PFlag << "\n"
       << "  PName        = " << PName.contents() << "\n"
       << "  PUnits       = " << PUnits.contents() << "\n"
       << "  PAsScalar    = " << PAsScalar << "\n"
       << "  UVIndex      = " << UVIndex << "\n"
       << "  Init         = " << init_str << " (" << Init << ")\n"
       << "  Valid        = " << valid_str << " (" << Valid << ")\n"
       << "  Ensemble     = " << Ensemble.contents() << "\n"
       << "  Lead         = " << lead_str << " (" << Lead << ")\n"
       << "  ConvertFx    = " << (ConvertFx.is_set() ? "IsSet" : "(nul)") << "\n"
       << "  CensorThresh = " << CensorThresh.get_str() << "\n"
       << "  CensorVal    = " << CensorVal.serialize() << "\n"
       << "  nBins        = " << nBins << "\n"
       << "  Range        = " << Range.serialize() << "\n"
       << "  Regrid       = " << interpmthd_to_string(Regrid.method) << "\n";

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

void VarInfo::set_name(const string str) {
   Name = str;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_ens(const char *str) {
   Ensemble = str;
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

void VarInfo::set_p_as_scalar(bool f) {
   PAsScalar = f;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_uv_index(int i) {
   UVIndex = i;
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

void VarInfo::set_censor_thresh(const ThreshArray &a) {
   CensorThresh = a;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_censor_val(const NumArray &a) {
   CensorVal = a;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_n_bins(const int &n) {
   nBins = n;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_range(const NumArray &a) {
   Range = a;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_regrid(const RegridInfo &ri) {
   Regrid = ri;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_magic(const ConcatString &nstr, const ConcatString &lstr) {

   // Check for embedded whitespace
   if((unsigned int) nstr.length() != strcspn(nstr.c_str(), " \t") ||
      (unsigned int) lstr.length() != strcspn(lstr.c_str(), " \t")) {
      mlog << Error << "\nVarInfo::set_magic() -> "
           << "embedded whitespace found in the nstr \"" << nstr
           << "\" or lstr \"" << lstr << "\".\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_dict(Dictionary &dict) {
   ThreshArray ta;
   NumArray na;
   ConcatString s;
   bool b;
   int n;

   // Set init time, if present
   s = dict.lookup_string(conf_key_init_time, false);
   if(dict.last_lookup_status()) set_init(timestring_to_unix(s.c_str()));

   // Set valid time, if present
   s = dict.lookup_string(conf_key_valid_time, false);
   if(dict.last_lookup_status()) set_valid(timestring_to_unix(s.c_str()));

   // Set lead time, if present
   s = dict.lookup_string(conf_key_lead_time, false);
   if(dict.last_lookup_status()) set_lead(timestring_to_sec(s.c_str()));

   // Parse prob as a boolean, if present
   const DictionaryEntry * e = dict.lookup(conf_key_prob);
   if(e) {
      if(e->type() == BooleanType) set_p_flag(e->b_value());
   }

   // Parse prob_as_scalar, if present
   b = dict.lookup_bool(conf_key_prob_as_scalar, false);
   if(dict.last_lookup_status()) set_p_as_scalar(b);

   // Lookup conversion function, if present
   ConvertFx.set(dict.lookup(conf_key_convert));

   // Parse censor_thresh, if present
   ta = dict.lookup_thresh_array(conf_key_censor_thresh, false);
   if(dict.last_lookup_status()) set_censor_thresh(ta);

   // Parse censor_val, if present
   na = dict.lookup_num_array(conf_key_censor_val, false);
   if(dict.last_lookup_status()) set_censor_val(na);

   // Check for equal number of censor thresholds and values
   if(ta.n_elements() != na.n_elements()) {
      mlog << Error << "\nVarInfo::set_dict() -> "
           << "The number of censor thresholds in \""
           << conf_key_censor_thresh << "\" (" << ta.n_elements()
           << ") must match the number of replacement values in \""
           << conf_key_censor_val << "\" (" << na.n_elements() << ").\n\n";
      exit(1);
   }

   // Parse n_bins, if present
   n = dict.lookup_int(conf_key_n_bins, false);
   if(dict.last_lookup_status()) set_n_bins(n);

   // Parse range, if present
   na = dict.lookup_num_array(conf_key_range_flag, false);
   if(dict.last_lookup_status()) set_range(na);

   // Parse regrid, if present
   Regrid = parse_conf_regrid(&dict, false);

   // Parse set_attr strings
   SetAttrName =
      parse_set_attr_string(dict, conf_key_set_attr_name, true);
   SetAttrUnits =
      parse_set_attr_string(dict, conf_key_set_attr_units, true);
   SetAttrLevel =
      parse_set_attr_string(dict, conf_key_set_attr_level, true);
   SetAttrLongName =
      parse_set_attr_string(dict, conf_key_set_attr_long_name);

   // Parse set_attr grid
   s = parse_set_attr_string(dict, conf_key_set_attr_grid);
   if(s.nonempty()) {

      // Parse as a white-space separated string
      StringArray sa;
      sa.parse_wsss(s);

      // Search for a named grid
      if(sa.n() == 1 && find_grid_by_name(sa[0].c_str(), SetAttrGrid)) {
      }
      // Parse grid definition
      else if(sa.n() > 1 && parse_grid_def(sa, SetAttrGrid)) {
      }
      else {
         mlog << Warning << "\nVarInfo::set_dict() -> "
              << "unsupported " << conf_key_set_attr_grid
              << " definition string (" << s
              << ")!\n\n";
      }
   }

   // Parse set_attr times
   s = parse_set_attr_string(dict, conf_key_set_attr_init);
   if(s.nonempty()) SetAttrInit = timestring_to_unix(s.c_str());
   s = parse_set_attr_string(dict, conf_key_set_attr_valid);
   if(s.nonempty()) SetAttrValid = timestring_to_unix(s.c_str());
   s = parse_set_attr_string(dict, conf_key_set_attr_lead);
   if(s.nonempty()) SetAttrLead = timestring_to_sec(s.c_str());
   s = parse_set_attr_string(dict, conf_key_set_attr_accum);
   if(s.nonempty()) SetAttrAccum = timestring_to_sec(s.c_str());

   // Parse set_attr flags
   SetAttrIsPrecipitation =
      parse_set_attr_flag(dict, conf_key_is_precipitation);
   SetAttrIsSpecificHumidity =
      parse_set_attr_flag(dict, conf_key_is_specific_humidity);
   SetAttrIsUWind =
      parse_set_attr_flag(dict, conf_key_is_u_wind);
   SetAttrIsVWind =
      parse_set_attr_flag(dict, conf_key_is_v_wind);
   SetAttrIsWindSpeed =
      parse_set_attr_flag(dict, conf_key_is_wind_speed);
   SetAttrIsWindDirection =
      parse_set_attr_flag(dict, conf_key_is_wind_direction);
   SetAttrIsGridRelative =
      parse_set_attr_flag(dict, conf_key_is_grid_relative);
   SetAttrIsProb =
      parse_set_attr_flag(dict, conf_key_is_prob);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfo::set_level_info_grib(Dictionary & dict){

   ConcatString field_level = dict.lookup_string(conf_key_level, false);
   LevelType lt;
   string lvl_type, lvl_val1, lvl_val2;
   double lvl1;
   double lvl2 = -1;

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

      //  store as a generic level type
      lt = LevelType_None;
      lvl_type= "L";

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
   else if( !is_eq(lvl2, -1.0) ) lvl_name.format("%s%g-%g", lvl_type.data(), lvl2, lvl1);
   else                          lvl_name.format("%s%g",    lvl_type.data(), lvl1);

   //  set the level information
   Level.set_type(lt);
   Level.set_req_name(lvl_name.c_str());
   Level.set_name(lvl_name.c_str());

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
   thr_hi.set(thresh_hi, is_bad_data(thresh_hi) ? thresh_na : thresh_lt);
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
   set_req_name ( field_name.c_str() );

}

///////////////////////////////////////////////////////////////////////////////

bool VarInfo::is_prob() {

   //
   // Check set_attr entry
   //
   if(!is_bad_data(SetAttrIsProb)) {
      return(SetAttrIsProb != 0);
   }

   return(PFlag && !PAsScalar);
}

///////////////////////////////////////////////////////////////////////////////

ConcatString parse_set_attr_string(Dictionary &dict, const char *key,
                                    bool check_ws) {
   ConcatString cs;

   cs = dict.lookup_string(key, false);
   if(cs.nonempty()) {
      mlog << Debug(3) << "Parsed " << key << " = \"" << cs << "\"\n";
      if(check_ws && check_reg_exp(ws_reg_exp, cs.c_str())) {
         mlog << Error << "\nparse_set_attr_string() -> "
              << "the \"" << key << "\" config file entry (" << cs
              << ") cannot contain embedded whitespace!\n\n";
         exit(1);
      }
   }

   return(cs);
}

///////////////////////////////////////////////////////////////////////////////

int parse_set_attr_flag(Dictionary &dict, const char *key) {
   int v = bad_data_int;

   bool b = dict.lookup_bool(key, false);
   if(dict.last_lookup_status()) {
      mlog << Debug(3) << "Parsed " << key << " = \""
           << bool_to_string(b) << "\"\n";
      v = (int) b;
   }

   return(v);
}

///////////////////////////////////////////////////////////////////////////////
