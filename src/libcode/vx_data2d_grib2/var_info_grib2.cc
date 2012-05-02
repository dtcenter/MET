// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_grib2.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <map>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <regex.h>

#include "var_info_grib2.h"

#include "math_constants.h"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_data2d.h"
#include "vx_config.h"


map<string,string> map_id;
multimap<string,string> map_code;


///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoGrib2
//
///////////////////////////////////////////////////////////////////////////////

VarInfoGrib2::VarInfoGrib2() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib2::~VarInfoGrib2() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib2::VarInfoGrib2(const VarInfoGrib2 &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib2 & VarInfoGrib2::operator=(const VarInfoGrib2 &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   Discipline = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::assign(const VarInfoGrib2 &v) {

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   Discipline = v.discipline();
   MTable     = v.m_table();
   LTable     = v.l_table();
   Tmpl       = v.tmpl();
   ParmCat    = v.parm_cat();
   Parm       = v.parm();
   Process    = v.process();
   EnsType    = v.ens_type();
   DerType    = v.der_type();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   Discipline = bad_data_int;
   MTable     = bad_data_int;
   LTable     = bad_data_int;
   Tmpl       = bad_data_int;
   ParmCat    = bad_data_int;
   Parm       = bad_data_int;
   Process    = bad_data_int;
   EnsType    = bad_data_int;
   DerType    = bad_data_int;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoGrib2::dump():\n"
       << "  Discipline = " << Discipline << "\n"
       << "  MTable     = " << MTable     << "\n"
       << "  LTable     = " << LTable     << "\n"
       << "  Tmpl       = " << Tmpl       << "\n"
       << "  ParmCat    = " << ParmCat    << "\n"
       << "  Parm       = " << Parm       << "\n"
       << "  Process    = " << Process    << "\n"
       << "  EnsType    = " << EnsType    << "\n"
       << "  DerType    = " << DerType    << "\n";

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_record(int v) {
   Record = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_discipline(int v) {
   Discipline = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_m_table(int v) {
   MTable = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_l_table(int v) {
   LTable = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_tmpl(int v) {
   Tmpl = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_parm_cat(int v) {
   ParmCat = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_parm(int v) {
   Parm = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_process(int v) {
   Process = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_ens_type(int v) {
   EnsType = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_der_type(int v) {
   DerType = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_magic(const ConcatString &s) {

   // Validate the magic_string
   VarInfo::set_magic(s);

   // Store the magic string
   MagicStr = s;

   //  parse the magic string components
   int num_mat = 0;
   char** mat = NULL;
   const char* pat_mag = "([^/]+)/([ALPRZ])([0-9]+)(\\-[0-9]+)?";
   if( 4 > (num_mat = regex_apply(pat_mag, 5, s, mat)) ){
      mlog << Error << "\nVarInfoGrib2::set_magic() - failed to parse magic string '"
           << s << "'\n\n";
      exit(1);
   }

   //  designate the magic string components
   string parm_name  = mat[1];
   string lvl_type   = mat[2];
   string lvl1_str   = mat[3];
   string lvl2_str;
   int lvl1 = atoi(lvl1_str.data()), lvl2 = -1;
   if( 5 == num_mat ){
      lvl2_str = mat[4];
      lvl2 = atoi( lvl2_str.substr(1, lvl2_str.length() - 1).data() );
   }
   regex_clean(mat);

   //  determine if the requested parameter name is actually a grib2 parameter table code
   ConcatString parm_vals = "";
   if( 4 == regex_apply("^([0-9]+)_([0-9]+)_([0-9]+)$", 4, parm_name.data(), mat) ){

      //  make sure the parameter table codes are valid
      if( !g2_id_count(parm_name.data()) ){
         mlog << Error << "\nVarInfoGrib2::set_magic() - unrecognized GRIB2 parameter table indexes '"
              << parm_name.data() << "'\n\n";
         exit(1);
      }

      //  assign the parameter category and code
      parm_vals = g2_id_lookup(parm_name.data());
      set_discipline( atoi(mat[1]) );
      set_parm_cat  ( atoi(mat[2]) );
      set_parm      ( atoi(mat[3]) );
      set_name      ( g2_id_parm(parm_name.data()).c_str() );

      regex_clean(mat);
   }

   //  otherwise, attempt to find the parameter name in the map_code table
   else {

      int num_parm_idx = g2_code_count(parm_name.data());

      //  validate the table and field number
      if( !num_parm_idx ){
         mlog << Error << "\nVarInfoGrib2::set_magic() - unrecognized GRIB2 field abbreviation '"
              << parm_name.data() << "'\n\n";
         exit(1);
      }
      set_name( parm_name.data() );

      //  retrieve the index values for the parameter abbreviation
      string parm_code = "";
      multimap<string,string>::iterator it = map_code.find(parm_name);
      parm_code = (*it).second;
      parm_vals = g2_id_lookup( parm_code.c_str() );

      //  parse the table and field number
      if( 4 != regex_apply("^([0-9]+)_([0-9]+)_([0-9]+)$", 4, parm_code.c_str(), mat) ){
         mlog << Error << "\nVarInfoGrib2::set_magic() - failed to parse GRIB2 table code for string '"
              << Name << "'\n\n";
         exit(1);
      }

      //  if the parameter abbreviation maps to a single index, set the VarInfo data members
      set_discipline( 0 < num_parm_idx ? atoi(mat[1]) : -1 );
      set_parm_cat  ( 0 < num_parm_idx ? atoi(mat[2]) : -1 );
      set_parm      ( 0 < num_parm_idx ? atoi(mat[3]) : -1 );

      regex_clean(mat);

   }

   //  set the level type
   LevelType lt;
   if      (lvl_type == "A") lt = LevelType_Accum;
   else if (lvl_type == "Z") lt = LevelType_Vert;
   else if (lvl_type == "P") lt = LevelType_Pres;
   else if (lvl_type == "R") lt = LevelType_RecNumber;
   else if (lvl_type == "L") lt = LevelType_None;
   else                      lt = LevelType_None;
   Level.set_type(lt);

   //  arrange the level values appropriately
   lvl2 = ( lvl2 != lvl1 ? lvl2 : -1 );
   if( lvl_type == "P" && -1 != lvl2 && lvl2 < lvl1 ){
      int lvl_tmp = lvl2;
      lvl2 = lvl1;
      lvl1 = lvl_tmp;
   }

   //  set the level name
   ConcatString lvl_name;
   if( lvl2 != -1 ) lvl_name.format("%s%d-%d", lvl_type.data(), lvl2, lvl1);
   else             lvl_name.format("%s%d",    lvl_type.data(), lvl1);
   Level.set_req_name(lvl_name);
   Level.set_name(lvl_name);

   //  set the lower limit
   if(lt == LevelType_Accum) Level.set_lower(timestring_to_sec( lvl1_str.data() ));
   else                      Level.set_lower(lvl1);

   //  if pressure ranges are not supported for the specified level type, bail
   if( -1 != lvl2 && lt != LevelType_Pres && lt != LevelType_Vert && lt != LevelType_None ){
      mlog << Error << "\nVarInfoGrib2::set_magic() - "
           << "ranges of levels are only supported for pressure levels "
           << "(P), vertical levels (Z), and generic levels (L)\n\n";
      exit(1);
   }

   //  set the upper level value
   if(lt == LevelType_Accum) Level.set_upper(timestring_to_sec( lvl1_str.data() ));
   else                      Level.set_upper(-1 == lvl2 ? lvl1 : lvl2);

   //  if the level type is a record number, set the data member
   set_record( lt == LevelType_RecNumber ? lvl1 : -1 );

   //  set the name, units and long name
   set_units("");
   set_long_name("");
   if( parm_vals != "" ){

      //  parse the variable name from the table information
      if( 4 != regex_apply("^(.+)\\|(.+)\\|(.+)$", 4, parm_vals.text(), mat) ){
         mlog << Error << "\nVarInfoGrib2::set_magic() - failed to parse GRIB2 table "
              << "map_id information '" << parm_vals.text() << "'\n\n";
         exit(1);
      }

      //  set the var_info parameter info
      ConcatString name_parm  = mat[1]; name_parm.ws_strip();
      if( name_parm == "APCP" ){
         int accum = atoi( sec_to_hhmmss( (int)Level.lower() ).text() );
         if( 0 == accum % 10000 ) accum /= 10000;
         ConcatString name_apcp;
         name_apcp.format("%s_%02d", name_parm.text(), accum);
         name_parm = name_apcp.text();
      }
      set_name( name_parm );
      ConcatString units_parm = mat[2]; units_parm.ws_strip(); set_units    ( units_parm );
      ConcatString lname_parm = mat[3]; lname_parm.ws_strip(); set_long_name( lname_parm );

      regex_clean(mat);

   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib2::set_dict(Dictionary & dict) {

   VarInfo::set_dict(dict);

   int tab_match = -1;
   Grib2TableEntry tab;
   ConcatString field_name = dict.lookup_string(conf_key_name,           false);
   int field_disc          = dict.lookup_int   (conf_key_GRIB2_disc,     false);
   int field_parm_cat      = dict.lookup_int   (conf_key_GRIB2_parm_cat, false);
   int field_parm          = dict.lookup_int   (conf_key_GRIB2_parm,     false);

   //  if the name is specified, use it
   if( !field_name.empty() ){

      set_name( field_name );
      set_req_name( field_name );

      //  look up the name in the grib tables
      if( !GribTable.lookup_grib2(field_name, field_disc, field_parm_cat, field_parm,
                                  tab, tab_match) ){
         mlog << Error << "\nVarInfoGrib2::set_dict() - unrecognized GRIB2 field abbreviation '"
              << field_name << "'\n\n";
         exit(1);
      }

   }

   //  if the field name is not specified, look for and use indexes
   else {

      //  if either the field name or the indices are specified, bail
      if( bad_data_int == field_disc ||
          bad_data_int == field_parm_cat ||
          bad_data_int == field_parm ){
         mlog << Error << "\nVarInfoGrib2::set_dict() - either name or GRIB2_disc, GRIB2_parm_cat "
              << "and GRIB2_parm must be specified in field information\n\n";
         exit(1);
      }

      //  use the specified indexes to look up the field name
      if( !GribTable.lookup_grib2(field_disc, field_parm_cat, field_parm, tab) ){
         mlog << Error << "\nVarInfoGrib2::set_dict() - no parameter found with matching "
              << "GRIB2_disc ("     << field_disc     << ") "
              << "GRIB2_parm_cat (" << field_parm_cat << ") "
              << "GRIB2_parm ("     << field_parm     << ")\n\n";
         exit(1);
      }

      //  use the lookup parameter name
      field_name = tab.parm_name;
   }

   //  set the matched parameter lookup information
   set_name      ( field_name    );
   set_req_name  ( field_name    );
   set_discipline( tab.index_a   );
   set_parm_cat  ( tab.index_b   );
   set_parm      ( tab.index_c   );
   set_units     ( tab.units     );
   set_long_name ( tab.full_name );

   //  call the parent to set the level information
   set_level_info_grib(dict);

   //  if the level type is a record number, set the data member
   set_record( Level.type() == LevelType_RecNumber ? Level.lower() : -1 );

   //  if the field name is APCP, apply additional formatting
   if( field_name == "APCP" ){
      int accum = atoi( sec_to_hhmmss( (int)Level.lower() ).text() );
      if( 0 == accum % 10000 ) accum /= 10000;
      set_name( str_format("%s_%02d", field_name.text(), accum) );
   }

   //  set the magic string
   MagicStr = str_format("%s/%s", field_name.text(), Level.name().text());

   return;

}

///////////////////////////////////////////////////////////////////////////////

vector<string> g2_code_lookup(const char* code){
   vector<string> ret;

   pair<multimap<string,string>::iterator,multimap<string,string>::iterator> codes =
         map_code.equal_range(code);
   for( multimap<string,string>::iterator it = codes.first;
        it != codes.second;
        it++ ){
      ret.push_back( (*it).second );
   }

   return ret;
}

///////////////////////////////////////////////////////////////////////////////

string g2_id_parm(const char* id){

   //  verify the id
   if( !g2_id_count( id ) ){
      mlog << Error << "\nVarInfoGrib2::g2_id_parm - failed find lookup id '"
           << id << "'\n\n";
      exit(1);
   }

   //  parse the parameter name from the table information
   char** mat = NULL;
   const char* pat_mag = "^[^ ]+";
   if( 1 != regex_apply(pat_mag, 1, g2_id_lookup( id ), mat) ){
      mlog << Error << "\nVarInfoGrib2::g2_id_parm - failed to parse GRIB2 table "
           << "map_id information '" << g2_id_lookup( id ) << "'\n\n";
      exit(1);
   }

   string ret = mat[0];
   regex_clean(mat);
   return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_precipitation() const {
   return Discipline == 0 &&
          ParmCat    == 1 &&
          (
             Parm == 8  ||       //  APCP
             Parm == 9  ||       //  NCPCP
             Parm == 10          //  ACPCP
          );
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_specific_humidity() const {
   return Discipline == 0 &&
          ParmCat    == 1 &&
          Parm       == 0;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_u_wind() const {
   return Discipline == 0 &&
          ParmCat    == 2 &&
          Parm       == 2;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_v_wind() const {
   return Discipline == 0 &&
          ParmCat    == 2 &&
          Parm       == 3;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_wind_speed() const {
   return Discipline == 0 &&
          ParmCat    == 2 &&
          Parm       == 1;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_wind_direction() const {
   return Discipline == 0 &&
          ParmCat    == 2 &&
          Parm       == 0;
}

////////////////////////////////////////////////////////////////////////

LevelType VarInfoGrib2::g2_lty_to_level_type(int lt) {

   //  from: http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table4-5.shtml
   switch( lt ){
      case 8:
         return LevelType_Accum;
      case 100:
      case 108:
         return LevelType_Pres;
      case 102:
      case 103:
         return LevelType_Vert;
      default:
         return LevelType_None;
   }
}

////////////////////////////////////////////////////////////////////////

double VarInfoGrib2::g2_time_range_unit_to_sec(int ind) {

   //  from: http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table4-4.shtml
   double sec_per_unit = -1;
   switch(ind) {
      case 0:     sec_per_unit = sec_per_minute;             break;    // minute
      case 1:     sec_per_unit = sec_per_hour;               break;    // hour
      case 2:     sec_per_unit = sec_per_day;                break;    // day
      case 3:     sec_per_unit = sec_per_day*30.0;           break;    // month
      case 4:     sec_per_unit = sec_per_day*365.0;          break;    // year
      case 5:     sec_per_unit = sec_per_day*365.0*10.0;     break;    // decade
      case 6:     sec_per_unit = sec_per_day*365.0*30.0;     break;    // normal (30 years)
      case 7:     sec_per_unit = sec_per_day*365.0*100.0;    break;    // century
      case 10:    sec_per_unit = sec_per_hour*3.0;           break;    // 3 hours
      case 11:    sec_per_unit = sec_per_hour*6.0;           break;    // 6 hours
      case 12:    sec_per_unit = sec_per_hour*12.0;          break;    // 12 hours
      case 13:    sec_per_unit = 1.0;                        break;    // second
   }
   return sec_per_unit;
}

////////////////////////////////////////////////////////////////////////

void init_var_maps(){

   //PGO these will be parsed and read in from a flat file

   map_id.clear();

   map_id[       "0_0_0"] = "TMP         | K                   | Temperature";
   map_id[       "0_0_1"] = "VTMP        | K                   | Virtual Temperature";
   map_id[       "0_0_2"] = "POT         | K                   | Potential Temperature";
   map_id[       "0_0_3"] = "EPOT        | K                   | Pseudo-Adiabatic Potential Temperature (or Equivalent Potential Temperature)";
   map_id[       "0_0_4"] = "TMAX        | K                   | Maximum Temperature";
   map_id[       "0_0_5"] = "TMIN        | K                   | Minimum Temperature";
   map_id[       "0_0_6"] = "DPT         | K                   | Dew Point Temperature";
   map_id[       "0_0_7"] = "DEPR        | K                   | Dew Point Depression (or Deficit)";
   map_id[       "0_0_8"] = "LAPR        | K/m                 | Lapse Rate";
   map_id[       "0_0_9"] = "TMPA        | K                   | Temperature Anomaly";
   map_id[      "0_0_10"] = "LHTFL       | W/m^2               | Latent Heat Net Flux";
   map_id[      "0_0_11"] = "SHTFL       | W/m^2               | Sensible Heat Net Flux";
   map_id[      "0_0_12"] = "HEATX       | K                   | Heat Index";
   map_id[      "0_0_13"] = "WCF         | K                   | Wind Chill Factor";
   map_id[      "0_0_14"] = "MINDPD      | K                   | Minimum Dew Point depression";
   map_id[      "0_0_15"] = "VPTMP       | K                   | Virtual Potential Temperature";
   map_id[      "0_0_16"] = "SNOHF       | W/m^2               | Snow phase change heat flux";
   map_id[      "0_0_17"] = "SKINT       | K                   | Skin Temperature";
   map_id[     "0_0_192"] = "SNOHF       | W/m^2               | Snow Phase Change Heat Flux";
   map_id[     "0_0_193"] = "TTRAD       | K/s                 | Temperature tendency by all radiation";
   map_id[     "0_0_194"] = "REV         | -                   | Relative Error Variance";
   map_id[     "0_0_195"] = "LRGHR       | K/s                 | Large Scale Condensate Heating rate";
   map_id[     "0_0_196"] = "CNVHR       | K/s                 | Deep Convective Heating rate";
   map_id[     "0_0_197"] = "THFLX       | W/m^2               | Total Downward Heat Flux at Surface";
   map_id[     "0_0_198"] = "TTDIA       | K/s                 | Temperature Tendency By All Physics";
   map_id[     "0_0_199"] = "TTPHY       | K/s                 | Temperature Tendency By Non-radiation Physics";
   map_id[     "0_0_200"] = "TSD1D       | K                   | Standard Dev. of IR Temp. over 1x1 deg. area";
   map_id[     "0_0_201"] = "SHAHR       | K/s                 | Shallow Convective Heating rate";
   map_id[     "0_0_202"] = "VDFHR       | K/s                 | Vertical Diffusion Heating rate";
   map_id[     "0_0_203"] = "THZ0        | K                   | Potential temperature at top of viscous sublayer";
   map_id[     "0_0_204"] = "TCHP        | J/m2K               | Tropical Cyclone Heat Potential";
   map_id[       "0_1_0"] = "SPFH        | kg/kg               | Specific Humidity";
   map_id[       "0_1_1"] = "RH          | %                   | Relative Humidity";
   map_id[       "0_1_2"] = "MIXR        | kg/kg               | Humidity Mixing Ratio";
   map_id[       "0_1_3"] = "PWAT        | kg/m^2              | Precipitable Water";
   map_id[       "0_1_4"] = "VAPP        | Pa                  | Vapor Pressure";
   map_id[       "0_1_5"] = "SATD        | Pa                  | Saturation Deficit";
   map_id[       "0_1_6"] = "EVP         | kg/m^2              | Evaporation";
   map_id[       "0_1_7"] = "PRATE       | kg/m^2/s            | Precipitation Rate";
   map_id[       "0_1_8"] = "APCP        | kg/m^2              | Total Precipitation";
   map_id[       "0_1_9"] = "NCPCP       | kg/m^2              | Large-Scale Precipitation (non-convective)";
   map_id[      "0_1_10"] = "ACPCP       | kg/m^2              | Convective Precipitation";
   map_id[      "0_1_11"] = "SNOD        | m                   | Snow Depth";
   map_id[      "0_1_12"] = "SRWEQ       | kg/m^2/s            | Snowfall Rate Water Equivalent";
   map_id[      "0_1_13"] = "WEASD       | kg/m^2              | Water Equivalent of Accumulated Snow Depth";
   map_id[      "0_1_14"] = "SNOC        | kg/m^2              | Convect Snow";
   map_id[      "0_1_15"] = "SNOL        | kg/m^2              | Large-Scale Snow";
   map_id[      "0_1_16"] = "SNOM        | kg/m^2              | Snow Melt";
   map_id[      "0_1_17"] = "SNOAG       | day                 | Snow Age";
   map_id[      "0_1_18"] = "ABSH        | kg/m^3              | Absolute Humidity";
   map_id[      "0_1_19"] = "PTYPE       | -                   | Precipitation Type";
   map_id[      "0_1_20"] = "ILIQW       | kg/m^2              | Integrated Liquid Water";
   map_id[      "0_1_21"] = "TCOND       | kg/kg               | Condensate";
   map_id[      "0_1_22"] = "CLWMR       | kg/kg               | Cloud Mixing Ratio";
   map_id[      "0_1_23"] = "ICMR        | kg/kg               | Ice Water Mixing Ratio";
   map_id[      "0_1_24"] = "RWMR        | kg/kg               | Rain Mixing Ratio";
   map_id[      "0_1_25"] = "SNMR        | kg/kg               | Snow Mixing Ratio";
   map_id[      "0_1_26"] = "MCONV       | kg/kg/s             | Horizontal Moisture Convergence";
   map_id[      "0_1_27"] = "MAXRH       | %                   | Maximum Relative Humidity";
   map_id[      "0_1_28"] = "MAXAH       | kg/m^3              | Maximum Absolute Humidity";
   map_id[      "0_1_29"] = "ASNOW       | m                   | Total Snowfall";
   map_id[      "0_1_30"] = "PWCAT       | -                   | Precipitable Water Category";
   map_id[      "0_1_31"] = "HAIL        | m                   | Hail";
   map_id[      "0_1_32"] = "GRLE        | kg/kg               | Grauple";
   map_id[      "0_1_33"] = "CRAIN       | -                   | Categorical Rain";
   map_id[      "0_1_34"] = "CFRZR       | -                   | Categorical Freezing Rain";
   map_id[      "0_1_35"] = "CICEP       | -                   | Categorical Ice Pellets";
   map_id[      "0_1_36"] = "CSNOW       | -                   | Categorical Snow";
   map_id[      "0_1_37"] = "CPRAT       | kg/m^2/s            | Convective Precipitation Rate";
   map_id[      "0_1_38"] = "MCONV       | kg/kg/s             | Horizontal Moisture Divergence";
   map_id[      "0_1_39"] = "CPOFP       | %                   | Percent frozen precipitation";
   map_id[      "0_1_40"] = "PEVAP       | kg/m^2              | Potential Evaporation";
   map_id[      "0_1_41"] = "PEVPR       | W/m^2               | Potential Evaporation Rate";
   map_id[      "0_1_42"] = "SNOWC       | %                   | Snow Cover";
   map_id[      "0_1_43"] = "FRAIN       | Proportion          | Rain Fraction of Total Liquid Water";
   map_id[      "0_1_44"] = "RIME        | Numeric             | Rime Factor";
   map_id[      "0_1_45"] = "TCOLR       | kg/m^2              | Total Column Integrated Rain";
   map_id[      "0_1_46"] = "TCOLS       | kg/m^2              | Total Column Integrated Snow";
   map_id[      "0_1_47"] = "LSWP        | kg/m^2              | Large Scale Water Precipitation (Non-Convective)";
   map_id[      "0_1_48"] = "CWP         | kg/m^2              | Convective Water Precipitation";
   map_id[      "0_1_49"] = "TWATP       | kg/m^2              | Total Water Precipitation";
   map_id[      "0_1_50"] = "TSNOWP      | kg/m^2              | Total Snow Precipitation";
   map_id[      "0_1_51"] = "TCWAT       | kg/m^2              | Total Column Water (Vertically integrated total water (vapour+cloud water/ice)";
   map_id[      "0_1_52"] = "TPRATE      | kg/m^2/s            | Total Precipitation Rate";
   map_id[      "0_1_53"] = "TSRWE       | kg/m^2/s            | Total Snowfall Rate Water Equivalent";
   map_id[      "0_1_54"] = "LSPRATE     | kg/m^2/s            | Large Scale Precipitation Rate";
   map_id[      "0_1_55"] = "CSRWE       | kg/m^2/s            | Convective Snowfall Rate Water Equivalent";
   map_id[      "0_1_56"] = "LSSRWE      | kg/m^2/s            | Large Scale Snowfall Rate Water Equivalent";
   map_id[      "0_1_57"] = "TSRATE      | m/s                 | Total Snowfall Rate";
   map_id[      "0_1_58"] = "CSRATE      | m/s                 | Convective Snowfall Rate";
   map_id[      "0_1_59"] = "LSSRATE     | m/s                 | Large Scale Snowfall Rate";
   map_id[      "0_1_60"] = "SDWE        | kg/m^2              | Snow Depth Water Equivalent";
   map_id[      "0_1_61"] = "SDEN        | kg/m^3              | Snow Density";
   map_id[      "0_1_62"] = "SEVAP       | kg/m^2              | Snow Evaporation";
   map_id[      "0_1_64"] = "TCIWV       | kg/m^2              | Total Column Integrated Water Vapour";
   map_id[      "0_1_65"] = "RPRATE      | kg/m^2/s            | Rain Precipitation Rate";
   map_id[      "0_1_66"] = "SPRATE      | kg/m^2/s            | Snow Precipitation Rate";
   map_id[      "0_1_67"] = "FPRATE      | kg/m^2/s            | Freezing Rain Precipitation Rate";
   map_id[      "0_1_68"] = "IPRATE      | kg/m^2/s            | Ice Pellets Precipitation Rate";
   map_id[     "0_1_192"] = "CRAIN       | non-dim             | Categorical Rain (yes=1; no=0)";
   map_id[     "0_1_193"] = "CFRZR       | non-dim             | Categorical Freezing Rain (yes=1; no=0)";
   map_id[     "0_1_194"] = "CICEP       | non-dim             | Categorical Ice Pellets (yes=1; no=0)";
   map_id[     "0_1_195"] = "CSNOW       | non-dim             | Categorical Snow (yes=1; no=0)";
   map_id[     "0_1_196"] = "CPRAT       | kg/m^2/s            | Convective Precipitation Rate";
   map_id[     "0_1_197"] = "MCONV       | kg/kg/s             | Horizontal Moisture Divergence";
   map_id[     "0_1_198"] = "MINRH       | %                   | Minimum Relative Humidity";
   map_id[     "0_1_199"] = "PEVAP       | kg/m^2              | Potential Evaporation";
   map_id[     "0_1_200"] = "PEVPR       | W/m^2               | Potential Evaporation Rate";
   map_id[     "0_1_201"] = "SNOWC       | %                   | Snow Cover";
   map_id[     "0_1_202"] = "FRAIN       | non-dim             | Rain Fraction of Total Liquid Water";
   map_id[     "0_1_203"] = "RIME        | non-dim             | Rime Factor";
   map_id[     "0_1_204"] = "TCOLR       | kg/m^2              | Total Column Integrated Rain";
   map_id[     "0_1_205"] = "TCOLS       | kg/m^2              | Total Column Integrated Snow";
   map_id[     "0_1_206"] = "TIPD        | non-dim             | Total Icing Potential Diagnostic";
   map_id[     "0_1_207"] = "NCIP        | non-dim             | Number concentration for ice particles";
   map_id[     "0_1_208"] = "SNOT        | K                   | Snow temperature";
   map_id[     "0_1_209"] = "TCLSW       | kg/m^2              | Total column-integrated supercooled liquid water";
   map_id[     "0_1_210"] = "TCOLM       | kg/m^2              | Total column-integrated melting ice";
   map_id[     "0_1_211"] = "EMNP        | cm/day              | Evaporation - Precipitation";
   map_id[     "0_1_212"] = "SBSNO       | W/m^2               | Sublimation (evaporation from snow)";
   map_id[     "0_1_213"] = "CNVMR       | kg/kg/s             | Deep Convective Moistening Rate";
   map_id[     "0_1_214"] = "SHAMR       | kg/kg/s             | Shallow Convective Moistening Rate";
   map_id[     "0_1_215"] = "VDFMR       | kg/kg/s             | Vertical Diffusion Moistening Rate";
   map_id[     "0_1_216"] = "CONDP       | Pa                  | Condensation Pressure of Parcali Lifted From Indicate Surface";
   map_id[     "0_1_217"] = "LRGMR       | kg/kg/s             | Large scale moistening rate";
   map_id[     "0_1_218"] = "QZ0         | kg/kg               | Specific humidity at top of viscous sublayer";
   map_id[     "0_1_219"] = "QMAX        | kg/kg               | Maximum specific humidity at 2m";
   map_id[     "0_1_220"] = "QMIN        | kg/kg               | Minimum specific humidity at 2m";
   map_id[     "0_1_221"] = "ARAIN       | kg/m2               | Liquid precipitation (rainfall)";
   map_id[     "0_1_222"] = "SNOWT       | K                   | Snow temperature, depth-avg";
   map_id[     "0_1_223"] = "APCPN       | kg/m2               | Total precipitation (nearest grid point)";
   map_id[     "0_1_224"] = "ACPCPN      | kg/m2               | Convective precipitation (nearest grid point)";
   map_id[     "0_1_225"] = "FRZR        | kg/m2               | Freezing Rain";
   map_id[     "0_1_226"] = "PWTHER      | Numeric             | Predominant Weather";
   map_id[     "0_1_227"] = "FROZR       | kg/m2               | Frozen Rain";
   map_id[     "0_1_241"] = "TSNOW       | kg/m2               | Total Snow";
   map_id[       "0_2_0"] = "WDIR        | deg                 | Wind Direction (from which blowing)";
   map_id[       "0_2_1"] = "WIND        | m/s                 | Wind Speed";
   map_id[       "0_2_2"] = "UGRD        | m/s                 | U-Component of Wind";
   map_id[       "0_2_3"] = "VGRD        | m/s                 | V-Component of Wind";
   map_id[       "0_2_4"] = "STRM        | m^2/s               | Stream Function";
   map_id[       "0_2_5"] = "VPOT        | m^2/s               | Velocity Potential";
   map_id[       "0_2_6"] = "MNTSF       | m^2/s               | Montgomery Stream Function";
   map_id[       "0_2_7"] = "SGCVV       | 1/s                 | Sigma Coordinate Vertical Velocity";
   map_id[       "0_2_8"] = "VVEL        | Pa/s                | Vertical Velocity (Pressure)";
   map_id[       "0_2_9"] = "DZDT        | m/s                 | Vertical Velocity (Geometric)";
   map_id[      "0_2_10"] = "ABSV        | 1/s                 | Absolute Vorticity";
   map_id[      "0_2_11"] = "ABSD        | 1/s                 | Absolute Divergence";
   map_id[      "0_2_12"] = "RELV        | 1/s                 | Relative Vorticity";
   map_id[      "0_2_13"] = "RELD        | 1/s                 | Relative Divergence";
   map_id[      "0_2_14"] = "PVORT       | Km2kg-1s-1          | Potential Vorticity";
   map_id[      "0_2_15"] = "VUCSH       | 1/s                 | Vertical U-Component of Shear";
   map_id[      "0_2_16"] = "VVCSH       | 1/s                 | Vertical V-Component of Shear";
   map_id[      "0_2_17"] = "UFLX        | N/m^2               | Momentum Flux, U-Component";
   map_id[      "0_2_18"] = "VFLX        | N/m^2               | Momentum Flux, V-Component";
   map_id[      "0_2_19"] = "WMIXE       | J                   | Wind Mixing Energy";
   map_id[      "0_2_20"] = "BLYDP       | W/m^2               | Boundary Layer Dissipation";
   map_id[      "0_2_21"] = "MAXGUST     | m/s                 | Maximum Wind Speed";
   map_id[      "0_2_22"] = "GUST        | m/s                 | Wind Speed (Gust)";
   map_id[      "0_2_23"] = "UGUST       | m/s                 | U-Component of Wind (Gust)";
   map_id[      "0_2_24"] = "VGUST       | m/s                 | V-Component of Wind (Gust)";
   map_id[      "0_2_25"] = "VWSH        | 1/s                 | Vertical speed sheer";
   map_id[      "0_2_26"] = "MFLX        | N/m^2               | Horizontal Momentum Flux";
   map_id[      "0_2_27"] = "USTM        | m/s                 | U-Component Storm Motion";
   map_id[      "0_2_28"] = "VSTM        | m/s                 | V-Component Storm Motion";
   map_id[      "0_2_29"] = "CD          | Numeric             | Drag Coefficient";
   map_id[      "0_2_30"] = "FRICV       | m/s                 | Frictional Velocity";
   map_id[      "0_2_32"] = "ETACVV      | 1/s                 | Eta Coordinate Vertical Velocity";
   map_id[     "0_2_192"] = "VWSH        | 1/s                 | Vertical speed sheer";
   map_id[     "0_2_193"] = "MFLX        | N/m^2               | Horizontal Momentum Flux";
   map_id[     "0_2_194"] = "USTM        | m/s                 | U-Component Storm Motion";
   map_id[     "0_2_195"] = "VSTM        | m/s                 | V-Component Storm Motion";
   map_id[     "0_2_196"] = "CD          | non-dim             | Drag Coefficient";
   map_id[     "0_2_197"] = "FRICV       | m/s                 | Frictional Velocity";
   map_id[     "0_2_198"] = "LAUV        | deg                 | Latitude of U Wind Component of Velocity";
   map_id[     "0_2_199"] = "LOUV        | deg                 | Longitude of U Wind Component of Velocity";
   map_id[     "0_2_200"] = "LAVV        | deg                 | Latitude of V Wind Component of Velocity";
   map_id[     "0_2_201"] = "LOVV        | deg                 | Longitude of V Wind Component of Velocity";
   map_id[     "0_2_202"] = "LAPP        | deg                 | Latitude of Presure Point";
   map_id[     "0_2_203"] = "LOPP        | deg                 | Longitude of Presure Point";
   map_id[     "0_2_204"] = "VEDH        | m^2/s               | Vertical Eddy Diffusivity Heat exchange";
   map_id[     "0_2_205"] = "COVMZ       | m^2/s^2             | Covariance between Meridional and Zonal Components of the wind.";
   map_id[     "0_2_206"] = "COVTZ       | K*ms-1              | Covariance between Temperature and Zonal Components of the wind.";
   map_id[     "0_2_207"] = "COVTM       | K*ms-1              | Covariance between Temperature and Meridional Components of the wind.";
   map_id[     "0_2_208"] = "VDFUA       | ms-2                | Vertical Diffusion Zonal Acceleration";
   map_id[     "0_2_209"] = "VDFVA       | ms-2                | Vertical Diffusion Meridional Acceleration";
   map_id[     "0_2_210"] = "GWDU        | ms-2                | Gravity wave drag zonal acceleration";
   map_id[     "0_2_211"] = "GWDV        | ms-2                | Gravity wave drag meridional acceleration";
   map_id[     "0_2_212"] = "CNVU        | ms-2                | Convective zonal momentum mixing acceleration";
   map_id[     "0_2_213"] = "CNVV        | ms-2                | Convective meridional momentum mixing acceleration";
   map_id[     "0_2_214"] = "WTEND       | m/s2                | Tendency of vertical velocity";
   map_id[     "0_2_215"] = "OMGALF      | K                   | Omega (Dp/Dt) divide by density";
   map_id[     "0_2_216"] = "CNGWDU      | m/s2                | Convective Gravity wave drag zonal acceleration";
   map_id[     "0_2_217"] = "CNGWDV      | m/s2                | Convective Gravity wave drag meridional acceleration";
   map_id[     "0_2_218"] = "LMV         | -                   | Velocity Point Model Surface";
   map_id[     "0_2_219"] = "PVMWW       | 1/s/m               | Potential Vorticity (Mass-Weighted)";
   map_id[     "0_2_220"] = "MAXUVV      | m/s                 | Hourly Maximum of Upward Vertical Velocity  in the lowest 400hPa";
   map_id[     "0_2_221"] = "MAXDVV      | m/s                 | Hourly Maximum of Downward Vertical Velocity  in the lowest 400hPa";
   map_id[     "0_2_222"] = "MAXUW       | m/s                 | U Component of Hourly Maximum 10m Wind Speed";
   map_id[     "0_2_223"] = "MAXVW       | m/s                 | V Component of Hourly Maximum 10m Wind Speed";
   map_id[     "0_2_224"] = "VRATE       | m2/s                | Ventilation Rate";
   map_id[       "0_3_0"] = "PRES        | Pa                  | Pressure";
   map_id[       "0_3_1"] = "PRMSL       | Pa                  | Pressure Reduced to MSL";
   map_id[       "0_3_2"] = "PTEND       | Pa/s                | Pressure Tendency";
   map_id[       "0_3_3"] = "ICAHT       | m                   | ICAO Standard Atmosphere Reference Height";
   map_id[       "0_3_4"] = "GP          | m^2/s^2             | Geopotential";
   map_id[       "0_3_5"] = "HGT         | gpm                 | Geopotential Height";
   map_id[       "0_3_6"] = "DIST        | m                   | Geometric Height";
   map_id[       "0_3_7"] = "HSTDV       | m                   | Standard Deviation of Height";
   map_id[       "0_3_8"] = "PRESA       | Pa                  | Pressure Anomaly";
   map_id[       "0_3_9"] = "GPA         | gpm                 | Geopotential Height Anomaly";
   map_id[      "0_3_10"] = "DEN         | kg/m^3              | Density";
   map_id[      "0_3_11"] = "ALTS        | Pa                  | Altimeter Setting";
   map_id[      "0_3_12"] = "THICK       | m                   | Thickness";
   map_id[      "0_3_13"] = "PRESALT     | m                   | Pressure Altitude";
   map_id[      "0_3_14"] = "DENALT      | m                   | Density Altitude";
   map_id[      "0_3_15"] = "5WAVH       | gpm                 | 5-Wave Geopotential Height";
   map_id[      "0_3_16"] = "U-GWD       | N/m^2               | Zonal Flux of Gravity Wave Stress";
   map_id[      "0_3_17"] = "V-GWD       | N/m^2               | Meridional Flux of Gravity Wave Stress";
   map_id[      "0_3_18"] = "HPBL        | m                   | Planetary Boundary Layer Height";
   map_id[      "0_3_19"] = "5WAVA       | gpm                 | 5-Wave Geopotential Height Anomaly";
   map_id[      "0_3_20"] = "SDSGSO      | m                   | Standard Deviation Of Sub-Grid Scale Orography";
   map_id[      "0_3_21"] = "AOSGSO      | Rad                 | Angle Of Sub-Grid Scale Orography";
   map_id[      "0_3_22"] = "SSGSO       | Numeric             | Slope Of Sub-Grid Scale Orography";
   map_id[      "0_3_23"] = "GSGSO       | W/m^2               | Gravity Of Sub-Grid Scale Orography";
   map_id[      "0_3_24"] = "ASGSO       | Numeric             | Anisotropy Of Sub-Grid Scale Orography";
   map_id[      "0_3_25"] = "NLPRES      | Numeric             | Natural Logarithm Of Pressure in Pa";
   map_id[     "0_3_192"] = "MSLET       | Pa                  | MSLP (Eta model reduction)";
   map_id[     "0_3_193"] = "5WAVH       | gpm                 | 5-Wave Geopotential Height";
   map_id[     "0_3_194"] = "U-GWD       | N/m^2               | Zonal Flux of Gravity Wave Stress";
   map_id[     "0_3_195"] = "V-GWD       | N/m^2               | Meridional Flux of Gravity Wave Stress";
   map_id[     "0_3_196"] = "HPBL        | m                   | Planetary Boundary Layer Height";
   map_id[     "0_3_197"] = "5WAVA       | gpm                 | 5-Wave Geopotential Height Anomaly";
   map_id[     "0_3_198"] = "MSLMA       | Pa                  | MSLP (MAPS System Reduction)";
   map_id[     "0_3_199"] = "TSLSA       | Pa/s                | 3-hr pressure tendency (Std. Atmos. Reduction)";
   map_id[     "0_3_200"] = "PLPL        | Pa                  | Pressure of level from which parcel was lifted";
   map_id[     "0_3_201"] = "LPSX        | 1/m                 | X-gradient of Log Pressure";
   map_id[     "0_3_202"] = "LPSY        | 1/m                 | Y-gradient of Log Pressure";
   map_id[     "0_3_203"] = "HGTX        | 1/m                 | X-gradient of Height";
   map_id[     "0_3_204"] = "HGTY        | 1/m                 | Y-gradient of Height";
   map_id[     "0_3_205"] = "LAYTH       | m                   | Layer Thickness";
   map_id[     "0_3_206"] = "NLGSP       | ln(kPa)             | Natural Log of Surface Pressure";
   map_id[     "0_3_207"] = "CNVUMF      | kg/m2/s             | Convective updraft mass flux";
   map_id[     "0_3_208"] = "CNVDMF      | kg/m2/s             | Convective downdraft mass flux";
   map_id[     "0_3_209"] = "CNVDEMF     | kg/m2/s             | Convective detrainment mass flux";
   map_id[     "0_3_210"] = "LMH         | -                   | Mass Point Model Surface";
   map_id[     "0_3_211"] = "HGTN        | gpm                 | Geopotential Height (nearest grid point)";
   map_id[     "0_3_212"] = "PRESN       | Pa                  | Pressure (nearest grid point)";
   map_id[       "0_4_0"] = "NSWRS       | W/m^2               | Net Short-Wave Radiation Flux (Surface)";
   map_id[       "0_4_1"] = "NSWRT       | W/m^2               | Net Short-Wave Radiation Flux (Top of Atmosphere)";
   map_id[       "0_4_2"] = "SWAVR       | W/m^2               | Short-Wave Radiation Flux";
   map_id[       "0_4_3"] = "GRAD        | W/m^2               | Global Radiation Flux";
   map_id[       "0_4_4"] = "BRTMP       | K                   | Brightness Temperature";
   map_id[       "0_4_5"] = "LWRAD       | W/m/sr              | Radiance (with respect to wave number)";
   map_id[       "0_4_6"] = "SWRAD       | W/m^3/sr            | Radiance (with respect to wavelength)";
   map_id[       "0_4_7"] = "DSWRF       | W/m^2               | Downward Short-Wave Rad. Flux";
   map_id[       "0_4_8"] = "USWRF       | W/m^2               | Upward Short-Wave Rad. Flux";
   map_id[       "0_4_9"] = "NSWRF       | W/m^2               | Net Short Wave Radiation Flux";
   map_id[      "0_4_10"] = "PHOTAR      | W/m^2               | Photosynthetically Active Radiation";
   map_id[      "0_4_11"] = "NSWRFCS     | W/m^2               | Net Short-Wave Radiation Flux, Clear Sky";
   map_id[      "0_4_12"] = "DWUVR       | W/m^2               | Downward UV Radiation";
   map_id[      "0_4_50"] = "UVIUCS      | Numeric             | UV Index (Under Clear Sky)";
   map_id[      "0_4_51"] = "UVI         | W/m^2               | UV Index";
   map_id[     "0_4_192"] = "DSWRF       | W/m^2               | Downward Short-Wave Rad. Flux";
   map_id[     "0_4_193"] = "USWRF       | W/m^2               | Upward Short-Wave Rad. Flux";
   map_id[     "0_4_194"] = "DUVB        | W/m^2               | UV-B downward solar flux";
   map_id[     "0_4_195"] = "CDUVB       | W/m^2               | Clear sky UV-B downward solar flux";
   map_id[     "0_4_196"] = "CSDSF       | W/m^2               | Clear Sky Downward Solar Flux";
   map_id[     "0_4_197"] = "SWHR        | K/s                 | Solar Radiative Heating Rate";
   map_id[     "0_4_198"] = "CSUSF       | W/m^2               | Clear Sky Upward Solar Flux";
   map_id[     "0_4_199"] = "CFNSF       | W/m^2               | Cloud Forcing Net Solar Flux";
   map_id[     "0_4_200"] = "VBDSF       | W/m^2               | Visible Beam Downward Solar Flux";
   map_id[     "0_4_201"] = "VDDSF       | W/m^2               | Visible Diffuse Downward Solar Flux";
   map_id[     "0_4_202"] = "NBDSF       | W/m^2               | Near IR Beam Downward Solar Flux";
   map_id[     "0_4_203"] = "NDDSF       | W/m^2               | Near IR Diffuse Downward Solar Flux";
   map_id[     "0_4_204"] = "DTRF        | W/m^2               | Downward Total radiation Flux";
   map_id[     "0_4_205"] = "UTRF        | W/m^2               | Upward Total radiation Flux";
   map_id[       "0_5_0"] = "NLWRS       | W/m^2               | Net Long-Wave Radiation Flux (Surface)";
   map_id[       "0_5_1"] = "NLWRT       | W/m^2               | Net Long-Wave Radiation Flux (Top of Atmosphere)";
   map_id[       "0_5_2"] = "LWAVR       | W/m^2               | Long-Wave Radiation Flux";
   map_id[       "0_5_3"] = "DLWRF       | W/m^2               | Downward Long-Wave Rad. Flux";
   map_id[       "0_5_4"] = "ULWRF       | W/m^2               | Upward Long-Wave Rad. Flux";
   map_id[       "0_5_5"] = "NLWRF       | W/m^2               | Net Long-Wave Radiation Flux";
   map_id[       "0_5_6"] = "NLWRCS      | W/m^2               | Net Long-Wave Radiation Flux, Clear Sky";
   map_id[     "0_5_192"] = "DLWRF       | W/m^2               | Downward Long-Wave Rad. Flux";
   map_id[     "0_5_193"] = "ULWRF       | W/m^2               | Upward Long-Wave Rad. Flux";
   map_id[     "0_5_194"] = "LWHR        | K/s                 | Long-Wave Radiative Heating Rate";
   map_id[     "0_5_195"] = "CSULF       | W/m^2               | Clear Sky Upward Long Wave Flux";
   map_id[     "0_5_196"] = "CSDLF       | W/m^2               | Clear Sky Downward Long Wave Flux";
   map_id[     "0_5_197"] = "CFNLF       | W/m^2               | Cloud Forcing Net Long Wave Flux";
   map_id[       "0_6_0"] = "CICE        | kg/m^2              | Cloud Ice";
   map_id[       "0_6_1"] = "TCDC        | %                   | Total Cloud Cover";
   map_id[       "0_6_2"] = "CDCON       | %                   | Convective Cloud Cover";
   map_id[       "0_6_3"] = "LCDC        | %                   | Low Cloud Cover";
   map_id[       "0_6_4"] = "MCDC        | %                   | Medium Cloud Cover";
   map_id[       "0_6_5"] = "HCDC        | %                   | High Cloud Cover";
   map_id[       "0_6_6"] = "CWAT        | kg/m^2              | Cloud Water";
   map_id[       "0_6_7"] = "CDCA        | %                   | Cloud Amount";
   map_id[       "0_6_8"] = "CDCT        | -                   | Cloud Type";
   map_id[       "0_6_9"] = "TMAXT       | m                   | Thunderstorm Maximum Tops";
   map_id[      "0_6_10"] = "THUNC       | -                   | Thunderstorm Coverage";
   map_id[      "0_6_11"] = "CDCB        | m                   | Cloud Base";
   map_id[      "0_6_12"] = "CDCT        | m                   | Cloud Top";
   map_id[      "0_6_13"] = "CEIL        | m                   | Ceiling";
   map_id[      "0_6_14"] = "CDLYR       | %                   | Non-Convective Cloud Cover";
   map_id[      "0_6_15"] = "CWORK       | J/kg                | Cloud Work Function";
   map_id[      "0_6_16"] = "CUEFI       | Proportion          | Convective Cloud Efficiency";
   map_id[      "0_6_17"] = "TCOND       | kg/kg               | Total Condensate";
   map_id[      "0_6_18"] = "TCOLW       | kg/m^2              | Total Column-Integrated Cloud Water";
   map_id[      "0_6_19"] = "TCOLI       | kg/m^2              | Total Column-Integrated Cloud Ice";
   map_id[      "0_6_20"] = "TCOLC       | kg/m^2              | Total Column-Integrated Condensate";
   map_id[      "0_6_21"] = "FICE        | Proportion          | Ice fraction of total condensate";
   map_id[      "0_6_22"] = "CDCC        | %                   | Cloud Cover";
   map_id[      "0_6_23"] = "CDCIMR      | kg/kg               | Cloud Ice Mixing Ratio";
   map_id[      "0_6_24"] = "SUNS        | Numeric             | Sunshine";
   map_id[      "0_6_25"] = "CBHE        | %                   | Horizontal Extent of Cumulonimbus (CB)";
   map_id[      "0_6_33"] = "SUNSD       | s                   | Sunshine Duration";
   map_id[     "0_6_192"] = "CDLYR       | %                   | Non-Convective Cloud Cover";
   map_id[     "0_6_193"] = "CWORK       | J/kg                | Cloud Work Function";
   map_id[     "0_6_194"] = "CUEFI       | non-dim             | Convective Cloud Efficiency";
   map_id[     "0_6_195"] = "TCOND       | kg/kg               | Total Condensate";
   map_id[     "0_6_196"] = "TCOLW       | kg/m^2              | Total Column-Integrated Cloud Water";
   map_id[     "0_6_197"] = "TCOLI       | kg/m^2              | Total Column-Integrated Cloud Ice";
   map_id[     "0_6_198"] = "TCOLC       | kg/m^2              | Total Column-Integrated Condensate";
   map_id[     "0_6_199"] = "FICE        | non-dim             | Ice fraction of total condensate";
   map_id[     "0_6_200"] = "MFLUX       | Pa/s                | Convective Cloud Mass Flux";
   map_id[     "0_6_201"] = "SUNSD       | s                   | Sunshine Duration";
   map_id[       "0_7_0"] = "PLI         | K                   | Parcel Lifted Index (to 500 mb)";
   map_id[       "0_7_1"] = "BLI         | K                   | Best Lifted Index (to 500 mb)";
   map_id[       "0_7_2"] = "KX          | K                   | K Index";
   map_id[       "0_7_3"] = "KOX         | K                   | KO Index";
   map_id[       "0_7_4"] = "TOTALX      | K                   | Total Totals Index";
   map_id[       "0_7_5"] = "SX          | Numeric             | Sweat Index";
   map_id[       "0_7_6"] = "CAPE        | J/kg                | Convective Available Potential Energy";
   map_id[       "0_7_7"] = "CIN         | J/kg                | Convective Inhibition";
   map_id[       "0_7_8"] = "HLCY        | m^2/s^2             | Storm Relative Helicity";
   map_id[       "0_7_9"] = "EHLX        | Numeric             | Energy Helicity Index";
   map_id[      "0_7_10"] = "LFTX        | K                   | Surface Lifted Index";
   map_id[      "0_7_11"] = "4LFTX       | K                   | Best (4 layer) Lifted Index";
   map_id[      "0_7_12"] = "RI          | Numeric             | Richardson Number";
   map_id[     "0_7_192"] = "LFTX        | K                   | Surface Lifted Index";
   map_id[     "0_7_193"] = "4LFTX       | K                   | Best (4 layer) Lifted Index";
   map_id[     "0_7_194"] = "RI          | Numeric             | Richardson Number";
   map_id[     "0_7_195"] = "CWDI        | -                   | Convective Weather Detection Index";
   map_id[     "0_7_196"] = "UVI         | W/m^2               | Ultra Violet Index";
   map_id[     "0_7_197"] = "UPHL        | m^2/s^2             | Updraft Helicity";
   map_id[     "0_7_198"] = "LAI         | -                   | Leaf Area Index";
   map_id[     "0_7_199"] = "MXUPHL      | m2/s2               | Hourly Maximum of Updraft Helicity over layer 2km to 5 km AGL";
   map_id[      "0_13_0"] = "AEROT       | -                   | Aerosol Type";
   map_id[    "0_13_192"] = "PMTC        | 10^-6g/m^3          | Particulate matter (coarse)";
   map_id[    "0_13_193"] = "PMTF        | 10^-6g/m^3          | Particulate matter (fine)";
   map_id[    "0_13_194"] = "LPMTF       | log10(10^-6g/m^3)   | Particulate matter (fine)";
   map_id[    "0_13_195"] = "LIPMF       | log10(10^-6g/m^3)   | Integrated column particulate matter (fine)";
   map_id[      "0_14_0"] = "TOZNE       | Dobson              | Total Ozone";
   map_id[      "0_14_1"] = "O3MR        | kg/kg               | Ozone Mixing Ratio";
   map_id[      "0_14_2"] = "TCIOZ       | Dobson              | Total Column Integrated Ozone";
   map_id[    "0_14_192"] = "O3MR        | kg/kg               | Ozone Mixing Ratio";
   map_id[    "0_14_193"] = "OZCON       | ppb                 | Ozone Concentration (PPB)";
   map_id[    "0_14_194"] = "OZCAT       | non-dim             | Categorical Ozone Concentration";
   map_id[    "0_14_195"] = "VDFOZ       | kg/kg/s             | Ozone Vertical Diffusion";
   map_id[    "0_14_196"] = "POZ         | kg/kg/s             | Ozone Production";
   map_id[    "0_14_197"] = "TOZ         | kg/kg/s             | Ozone Tendency";
   map_id[    "0_14_198"] = "POZT        | kg/kg/s             | Ozone Production from Temperature Term";
   map_id[    "0_14_199"] = "POZO        | kg/kg/s             | Ozone Production from Column Ozone Term";
   map_id[    "0_14_200"] = "OZMAX1      | ppbV                | Ozone Daily Max from 1-hour Average";
   map_id[    "0_14_201"] = "OZMAX8      | ppbV                | Ozone Daily Max from 8-hour Average";
   map_id[    "0_14_202"] = "PDMAX1      | &#956g/m3           | PM 2.5 Daily Max from 1-hour Average";
   map_id[    "0_14_203"] = "PDMAX24     | &#956g/m3           | PM 2.5 Daily Max from 24-hour Average";
   map_id[      "0_15_0"] = "BSWID       | m/s                 | Base Spectrum Width";
   map_id[      "0_15_1"] = "BREF        | dB                  | Base Reflectivity";
   map_id[      "0_15_2"] = "BRVEL       | m/s                 | Base Radial Velocity";
   map_id[      "0_15_3"] = "VERIL       | kg/m                | Vertically Integrated Liquid (VIL)";
   map_id[      "0_15_4"] = "LMAXBR      | dB                  | Layer Maximum Base Reflectivity";
   map_id[      "0_15_5"] = "PREC        | kg/m^2              | Precipitation";
   map_id[      "0_15_6"] = "RDSP1       | -                   | Radar Spectra (1)";
   map_id[      "0_15_7"] = "RDSP2       | -                   | Radar Spectra (2)";
   map_id[      "0_15_8"] = "RDSP3       | -                   | Radar Spectra (3)";
   map_id[      "0_16_0"] = "REFZR       | mm^6/m^3            | Equivalent radar reflectivity factor for rain";
   map_id[      "0_16_1"] = "REFZI       | mm^6/m^3            | Equivalent radar reflectivity factor for snow";
   map_id[      "0_16_2"] = "REFZC       | mm^6/m^3            | Equivalent radar reflectivity factor for parameterized convection";
   map_id[      "0_16_3"] = "RETOP       | m                   | Echo Top";
   map_id[      "0_16_4"] = "REFD        | dB                  | Reflectivity";
   map_id[      "0_16_5"] = "REFC        | dB                  | Composite reflectivity";
   map_id[    "0_16_192"] = "REFZR       | mm^6/m^3            | Equivalent radar reflectivity factor for rain";
   map_id[    "0_16_193"] = "REFZI       | mm^6/m^3            | Equivalent radar reflectivity factor for snow";
   map_id[    "0_16_194"] = "REFZC       | mm^6/m^3            | Equivalent radar reflectivity factor for parameterized convection";
   map_id[    "0_16_195"] = "REFD        | dB                  | Reflectivity";
   map_id[    "0_16_196"] = "REFC        | dB                  | Composite reflectivity";
   map_id[    "0_16_197"] = "RETOP       | m                   | Echo Top";
   map_id[    "0_16_198"] = "MAXREF      | dB                  | Hourly Maximum of Simulated Reflectivity at 1 km AGL";
   map_id[    "0_17_192"] = "LTNG        | non-dim             | Lightning";
   map_id[      "0_18_0"] = "ACCES       | Bq/m^3              | Air Concentration of Cesium 137";
   map_id[      "0_18_1"] = "ACIOD       | Bq/m^3              | Air Concentration of Iodine 131";
   map_id[      "0_18_2"] = "ACRADP      | Bq/m^3              | Air Concentration of Radioactive Pollutant";
   map_id[      "0_18_3"] = "GDCES       | Bq/m^2              | Ground Deposition of Cesium 137";
   map_id[      "0_18_4"] = "GDIOD       | Bq/m^2              | Ground Deposition of Iodine 131";
   map_id[      "0_18_5"] = "GDRADP      | Bq/m^2              | Ground Deposition of Radioactive Pollutant";
   map_id[      "0_18_6"] = "TIACCP      | Bq/m^3              | Time Integrated Air Concentration of Cesium Pollutant";
   map_id[      "0_18_7"] = "TIACIP      | Bq/m^3              | Time Integrated Air Concentration of Iodine Pollutant";
   map_id[      "0_18_8"] = "TIACRP      | Bq/m^3              | Time Integrated Air Concentration of Radioactive Pollutant";
   map_id[      "0_19_0"] = "VIS         | m                   | Visibility";
   map_id[      "0_19_1"] = "ALBDO       | %                   | Albedo";
   map_id[      "0_19_2"] = "TSTM        | %                   | Thunderstorm Probability";
   map_id[      "0_19_3"] = "MIXHT       | m                   | Mixed Layer Depth";
   map_id[      "0_19_4"] = "VOLASH      | -                   | Volcanic Ash";
   map_id[      "0_19_5"] = "ICIT        | m                   | Icing Top";
   map_id[      "0_19_6"] = "ICIB        | m                   | Icing Base";
   map_id[      "0_19_7"] = "ICI         | -                   | Icing";
   map_id[      "0_19_8"] = "TURBT       | m                   | Turbulence Top";
   map_id[      "0_19_9"] = "TURBB       | m                   | Turbulence Base";
   map_id[     "0_19_10"] = "TURB        | -                   | Turbulence";
   map_id[     "0_19_11"] = "TKE         | J/kg                | Turbulent Kinetic Energy";
   map_id[     "0_19_12"] = "PBLREG      | -                   | Planetary Boundary Layer Regime";
   map_id[     "0_19_13"] = "CONTI       | -                   | Contrail Intensity";
   map_id[     "0_19_14"] = "CONTET      | -                   | Contrail Engine Type";
   map_id[     "0_19_15"] = "CONTT       | m                   | Contrail Top";
   map_id[     "0_19_16"] = "CONTB       | m                   | Contrail Base";
   map_id[     "0_19_17"] = "MXSALB      | %                   | Maximum Snow Albedo";
   map_id[     "0_19_18"] = "SNFALB      | %                   | Snow-Free Albedo";
   map_id[     "0_19_19"] = "SALBD       | %                   | Snow Albedo";
   map_id[     "0_19_20"] = "ICIP        | %                   | Icing";
   map_id[     "0_19_21"] = "CTP         | %                   | In-Cloud Turbulence";
   map_id[     "0_19_22"] = "CAT         | %                   | Clear Air Turbulence (CAT)";
   map_id[     "0_19_23"] = "SLDP        | %                   | Supercooled Large Droplet (SLD) Probability";
   map_id[    "0_19_192"] = "MXSALB      | %                   | Maximum Snow Albedo";
   map_id[    "0_19_193"] = "SNFALB      | %                   | Snow-Free Albedo";
   map_id[    "0_19_194"] = "SRCONO      | categorical         | Slight risk convective outlook";
   map_id[    "0_19_195"] = "MRCONO      | categorical         | Moderate risk convective outlook";
   map_id[    "0_19_196"] = "HRCONO      | categorical         | High risk convective outlook";
   map_id[    "0_19_197"] = "TORPROB     | %                   | Tornado probability";
   map_id[    "0_19_198"] = "HAILPROB    | %                   | Hail probability";
   map_id[    "0_19_199"] = "WINDPROB    | %                   | Wind probability";
   map_id[    "0_19_200"] = "STORPROB    | %                   | Significant Tornado probability";
   map_id[    "0_19_201"] = "SHAILPRO    | %                   | Significant Hail probability";
   map_id[    "0_19_202"] = "SWINDPRO    | %                   | Significant Wind probability";
   map_id[    "0_19_203"] = "TSTMC       | categorical         | Categorical Thunderstorm (1-yes, 0-no)";
   map_id[    "0_19_204"] = "MIXLY       | Integer             | Number of mixed layers next to surface";
   map_id[    "0_19_205"] = "FLGHT       | -                   | Flight Category";
   map_id[    "0_19_206"] = "CICEL       | -                   | Confidence - Ceiling";
   map_id[    "0_19_207"] = "CIVIS       | -                   | Confidence - Visibility";
   map_id[    "0_19_208"] = "CIFLT       | -                   | Confidence - Flight Category";
   map_id[    "0_19_209"] = "LAVNI       | -                   | Low-Level aviation interest";
   map_id[    "0_19_210"] = "HAVNI       | -                   | High-Level aviation interest";
   map_id[    "0_19_211"] = "SBSALB      | %                   | Visible, Black Sky Albedo";
   map_id[    "0_19_212"] = "SWSALB      | %                   | Visible, White Sky Albedo";
   map_id[    "0_19_213"] = "NBSALB      | %                   | Near IR, Black Sky Albedo";
   map_id[    "0_19_214"] = "NWSALB      | %                   | Near IR, White Sky Albedo";
   map_id[    "0_19_215"] = "PRSVR       | %                   | Total Probability of Severe Thunderstorms (Days 2,3)";
   map_id[    "0_19_216"] = "PRSIGSVR    | %                   | Total Probability of Extreme Severe Thunderstorms (Days 2,3)";
   map_id[    "0_19_217"] = "SIPD        | -                   | Supercooled Large Droplet (SLD) Icing";
   map_id[    "0_19_218"] = "EPSR        | -                   | Radiative emissivity";
   map_id[    "0_19_219"] = "TPFI        | -                   | Turbulence Potential Forecast Index";
   map_id[    "0_19_232"] = "VAFTD       | log10(kg/m3)        | Volcanic Ash Forecast Transport and Dispersion";
   map_id[      "0_20_0"] = "MASSDEN     | kg/m^3              | Mass Density (Concentration)";
   map_id[      "0_20_1"] = "COLMD       | kg/m^2              | Column-Integrated Mass Density";
   map_id[      "0_20_2"] = "MASSMR      | kg/kg               | Mass Mixing Ratio (Mass Fraction in Air)";
   map_id[      "0_20_3"] = "AEMFLX      | kg/m^2/s            | Atmosphere Emission Mass Flux";
   map_id[      "0_20_4"] = "ANPMFLX     | kg/m^2/s            | Atmosphere Net Production Mass Flux";
   map_id[      "0_20_5"] = "ANPEMFLX    | kg/m^2/s            | Atmosphere Net Production And Emision Mass Flux";
   map_id[      "0_20_6"] = "SDDMFLX     | kg/m^2/s            | Surface Dry Deposition Mass Flux";
   map_id[      "0_20_7"] = "SWDMFLX     | kg/m^2/s            | Surface Wet Deposition Mass Flux";
   map_id[      "0_20_8"] = "AREMFLX     | kg/m^2/s            | Atmosphere Re-Emission Mass Flux";
   map_id[     "0_20_50"] = "AIA         | mol                 | Amount in Atmosphere";
   map_id[     "0_20_51"] = "CONAIR      | molm-3              | Concentration In Air";
   map_id[     "0_20_52"] = "VMXR        | molmol-1            | Volume Mixing Ratio (Fraction in Air)";
   map_id[     "0_20_53"] = "CGPRC       | molm-3s-1           | Chemical Gross Production Rate of Concentration";
   map_id[     "0_20_54"] = "CGDRC       | molm-3s-1           | Chemical Gross Destruction Rate of Concentration";
   map_id[     "0_20_55"] = "SFLUX       | molm-2s-1           | Surface Flux";
   map_id[     "0_20_56"] = "COAIA       | mol/s               | Changes Of Amount in Atmosphere";
   map_id[     "0_20_57"] = "TYABA       | mol                 | Total Yearly Average Burden of The Atmosphere";
   map_id[     "0_20_58"] = "TYAAL       | mol/s               | Total Yearly Average Atmospheric Loss";
   map_id[    "0_20_100"] = "SADEN       | 1/m                 | Surface Area Density (Aerosol)";
   map_id[    "0_20_101"] = "AOTK        | m                   | Atmosphere Optical Thickness";
   map_id[    "0_20_131"] = "NO2TROP     | molcm-2             | Nitrogen Dioxide (NO2) Tropospheric Column";
   map_id[    "0_20_132"] = "NO2VCD      | molcm-2             | Nitrogen Dioxide (NO2) Vertical Column Density";
   map_id[    "0_20_133"] = "BROVCD      | molcm-2             | Bromine Monoxide (BrO) Vertical Column Density";
   map_id[    "0_20_134"] = "HCHOVCD     | molcm-2             | Formaldehyde (HCHO) Vertical Column Density";
   map_id[     "0_190_0"] = "var190m0    | CCITTIA5            | Arbitrary Text String";
   map_id[     "0_191_0"] = "TSEC        | s                   | Seconds prior to initial reference time (defined in Section 1)";
   map_id[   "0_191_192"] = "NLAT        | deg                 | Latitude (-90 to +90)";
   map_id[   "0_191_193"] = "ELON        | deg                 | East Longitude (0 - 360)";
   map_id[   "0_191_194"] = "TSEC        | s                   | Seconds prior to initial reference time";
   map_id[   "0_191_195"] = "MLYNO       | -                   | Model Layer number (From bottom up)";
   map_id[   "0_191_196"] = "NLATN       | deg                 | Latitude (nearest neighbor) (-90 to +90)";
   map_id[   "0_191_197"] = "ELONN       | deg                 | East Longitude (nearest neighbor) (0 - 360)";
   map_id[     "0_192_1"] = "COVMZ       | m2/s2               | Covariance between zonal and meridional components of the wind. Defined as [uv]-[u][v], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_2"] = "COVTZ       | K*m/s               | Covariance between izonal component of the wind and temperature. Defined as [uT]-[u][T], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_3"] = "COVTM       | K*m/s               | Covariance between meridional component of the wind and temperature. Defined as [vT]-[v][T], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_4"] = "COVTW       | K*m/s               | Covariance between temperature and vertical component of the wind. Defined as [wT]-[w][T], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_5"] = "COVZZ       | m2/s2               | Covariance between zonal and zonal components of the wind. Defined as [uu]-[u][u], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_6"] = "COVMM       | m2/s2               | Covariance between meridional and meridional components of the wind. Defined as [vv]-[v][v], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_7"] = "COVQZ       | kg/kg*m/s           | Covariance between specific humidity and zonal components of the wind. Defined as [uq]-[u][q], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_8"] = "COVQM       | kg/kg*m/s           | Covariance between specific humidity and meridional components of the wind. Defined as [vq]-[v][q], where [] indicates the mean over the indicated time span.";
   map_id[     "0_192_9"] = "COVTVV      | K*Pa/s              | Covariance between temperature and vertical components of the wind. Defined as [&#937T]-[&#937][T], where [] indicates the mean over the indicated time span.";
   map_id[    "0_192_10"] = "COVQVV      | kg/kg*Pa/s          | Covariance between specific humidity and vertical components of the wind. Defined as [&#937q]-[&#937][q], where [] indicates the mean over the indicated time span.";
   map_id[    "0_192_11"] = "COVPSPS     | Pa*Pa               | Covariance between surface pressure and surface pressure. Defined as [Psfc]-[Psfc][Psfc], where [] indicates the mean over the indicated time span.";
   map_id[    "0_192_12"] = "COVQQ       | kg/kg*kg/kg         | Covariance between specific humidity and specific humidy. Defined as [qq]-[q][q], where [] indicates the mean over the indicated time span.";
   map_id[    "0_192_13"] = "COVVVVV     | Pa2/s2              | Covariance between vertical and vertical components of the wind. Defined as [&#937&#937]-[&#937][&#937], where [] indicates the mean over the indicated time span.";
   map_id[    "0_192_14"] = "COVTT       | K*K                 | Covariance between temperature and temperature. Defined as [TT]-[T][T], where [] indicates the mean over the indicated time span.";
   map_id[       "1_0_0"] = "FFLDG       | kg/m^2              | Flash Flood Guidance (Encoded as an accumulation over a floating subinterval of time between the reference time and valid time)";
   map_id[       "1_0_1"] = "FFLDRO      | kg/m^2              | Flash Flood Runoff (Encoded as an accumulation over a floating subinterval of time)";
   map_id[       "1_0_2"] = "RSSC        | -                   | Remotely Sensed Snow Cover";
   map_id[       "1_0_3"] = "ESCT        | -                   | Elevation of Snow Covered Terrain";
   map_id[       "1_0_4"] = "SWEPON      | %                   | Snow Water Equivalent Percent of Normal";
   map_id[       "1_0_5"] = "BGRUN       | kg/m^2              | Baseflow-Groundwater Runoff";
   map_id[       "1_0_6"] = "SSRUN       | kg/m^2              | Storm Surface Runoff";
   map_id[     "1_0_192"] = "BGRUN       | kg/m^2              | Baseflow-Groundwater Runoff";
   map_id[     "1_0_193"] = "SSRUN       | kg/m^2              | Storm Surface Runoff";
   map_id[       "1_1_0"] = "CPPOP       | kg/m^2              | Conditional percent precipitation amount fractile for an overall period (encoded as an accumulation)";
   map_id[       "1_1_1"] = "PPOSP       | %                   | Percent Precipitation in a sub-period of an overall period (encoded as a percent accumulation over the sub-period)";
   map_id[       "1_1_2"] = "POP         | %                   | Probability of 0.01 inch of precipitation (POP)";
   map_id[     "1_1_192"] = "CPOZP       | %                   | Probability of Freezing Precipitation";
   map_id[     "1_1_193"] = "CPOFP       | %                   | Probability of Frozen Precipitation";
   map_id[     "1_1_194"] = "PPFFG       | %                   | Probability of precipitation exceeding flash flood guidance values";
   map_id[     "1_1_195"] = "CWR         | %                   | Probability of Wetting Rain, exceeding in 0.10 in a given time period";
   map_id[       "2_0_0"] = "LAND        | Proportion          | Land Cover (1=land, 0=sea)";
   map_id[       "2_0_1"] = "SFCR        | m                   | Surface Roughness";
   map_id[       "2_0_2"] = "TSOIL       | K                   | Soil Temperature";
   map_id[       "2_0_3"] = "SOILM       | kg/m^2              | Soil Moisture Content";
   map_id[       "2_0_4"] = "VEG         | %                   | Vegetation";
   map_id[       "2_0_5"] = "WATR        | kg/m^2              | Water Runoff";
   map_id[       "2_0_6"] = "EVAPT       | 1/kg^2/s            | Evapotranspiration";
   map_id[       "2_0_7"] = "MTERH       | m                   | Model Terrain Height";
   map_id[       "2_0_8"] = "LANDU       | -                   | Land Use";
   map_id[       "2_0_9"] = "SOILW       | Proportion          | Volumetric Soil Moisture Content";
   map_id[      "2_0_10"] = "GFLUX       | W/m^2               | Ground Heat Flux";
   map_id[      "2_0_11"] = "MSTAV       | %                   | Moisture Availability";
   map_id[      "2_0_12"] = "SFEXC       | kg/m^2/s            | Exchange Coefficient";
   map_id[      "2_0_13"] = "CNWAT       | kg/m^2              | Plant Canopy Surface Water";
   map_id[      "2_0_14"] = "BMIXL       | m                   | Blackadars Mixing Length Scale";
   map_id[      "2_0_15"] = "CCOND       | m/s                 | Canopy Conductance";
   map_id[      "2_0_16"] = "RSMIN       | s/m                 | Minimal Stomatal Resistance";
   map_id[      "2_0_17"] = "WILT        | Proportion          | Wilting Pointx";
   map_id[      "2_0_18"] = "RCS         | Proportion          | Solar parameter in canopy conductance";
   map_id[      "2_0_19"] = "RCT         | Proportion          | Temperature parameter in canopy conductance";
   map_id[      "2_0_20"] = "RCSOL       | Proportion          | Soil moisture parameter in canopy conductance";
   map_id[      "2_0_21"] = "RCQ         | Proportion          | Humidity parameter in canopy conductance";
   map_id[      "2_0_22"] = "SOILM       | kg/m^3              | Soil Moisture";
   map_id[      "2_0_23"] = "CISOILW     | kg/m^2              | Column-Integrated Soil Water";
   map_id[      "2_0_24"] = "HFLUX       | W/m^2               | Heat Flux";
   map_id[      "2_0_25"] = "VSOILM      | m3m-3               | Volumetric Soil Moisture";
   map_id[      "2_0_26"] = "WILT        | kg/m^3              | Wilting Point";
   map_id[      "2_0_27"] = "VWILTM      | m3m-3               | Volumetric Wilting Moisture";
   map_id[     "2_0_192"] = "SOILW       | Fraction            | Volumetric Soil Moisture Content";
   map_id[     "2_0_193"] = "GFLUX       | W/m^2               | Ground Heat Flux";
   map_id[     "2_0_194"] = "MSTAV       | %                   | Moisture Availability";
   map_id[     "2_0_195"] = "SFEXC       | (kg/m^3)(m/s)       | Exchange Coefficient";
   map_id[     "2_0_196"] = "CNWAT       | kg/m^2              | Plant Canopy Surface Water";
   map_id[     "2_0_197"] = "BMIXL       | m                   | Blackadars Mixing Length Scale";
   map_id[     "2_0_198"] = "VGTYP       | Integer(0-13)       | Vegetation Type";
   map_id[     "2_0_199"] = "CCOND       | m/s                 | Canopy Conductance";
   map_id[     "2_0_200"] = "RSMIN       | s/m                 | Minimal Stomatal Resistance";
   map_id[     "2_0_201"] = "WILT        | Fraction            | Wilting Point";
   map_id[     "2_0_202"] = "RCS         | Fraction            | Solar parameter in canopy conductance";
   map_id[     "2_0_203"] = "RCT         | Fraction            | Temperature parameter in canopy conductance";
   map_id[     "2_0_204"] = "RCQ         | Fraction            | Humidity parameter in canopy conductance";
   map_id[     "2_0_205"] = "RCSOL       | Fraction            | Soil moisture parameter in canopy conductance";
   map_id[     "2_0_206"] = "RDRIP       | -                   | Rate of water dropping from canopy to ground";
   map_id[     "2_0_207"] = "ICWAT       | %                   | Ice-free water surface";
   map_id[     "2_0_208"] = "AKHS        | m/s                 | Surface exchange coefficients for T and Q divided by delta z";
   map_id[     "2_0_209"] = "AKMS        | m/s                 | Surface exchange coefficients for U and V divided by delta z";
   map_id[     "2_0_210"] = "VEGT        | K                   | Vegetation canopy temperature";
   map_id[     "2_0_211"] = "SSTOR       | Kg/m2               | Surface water storage";
   map_id[     "2_0_212"] = "LSOIL       | Kg/m2               | Liquid soil moisture content (non-frozen)";
   map_id[     "2_0_213"] = "EWATR       | W/m2                | Open water evaporation (standing water)";
   map_id[     "2_0_214"] = "GWREC       | kg/m2               | Groundwater recharge";
   map_id[     "2_0_215"] = "QREC        | kg/m2               | Flood plain recharge";
   map_id[     "2_0_216"] = "SFCRH       | m                   | Roughness length for heat";
   map_id[     "2_0_217"] = "NDVI        | -                   | Normalized Difference Vegetation Index";
   map_id[     "2_0_218"] = "LANDN       | -                   | Land-sea coverage (nearest neighbor) [land=1,sea=0]";
   map_id[     "2_0_219"] = "AMIXL       | m                   | Asymptotic mixing length scale";
   map_id[     "2_0_220"] = "WVINC       | kg/m2               | Water vapor added by precip assimilation";
   map_id[     "2_0_221"] = "WCINC       | kg/m2               | Water condensate added by precip assimilation";
   map_id[     "2_0_222"] = "WVCONV      | kg/m2               | Water Vapor Flux Convergance (Vertical Int)";
   map_id[     "2_0_223"] = "WCCONV      | kg/m2               | Water Condensate Flux Convergance (Vertical Int)";
   map_id[     "2_0_224"] = "WVUFLX      | kg/m2               | Water Vapor Zonal Flux (Vertical Int)";
   map_id[     "2_0_225"] = "WVVFLX      | kg/m2               | Water Vapor Meridional Flux (Vertical Int)";
   map_id[     "2_0_226"] = "WCUFLX      | kg/m2               | Water Condensate Zonal Flux (Vertical Int)";
   map_id[     "2_0_227"] = "WCVFLX      | kg/m2               | Water Condensate Meridional Flux (Vertical Int)";
   map_id[     "2_0_228"] = "ACOND       | m/s                 | Aerodynamic conductance";
   map_id[     "2_0_229"] = "EVCW        | W/m2                | Canopy water evaporation";
   map_id[     "2_0_230"] = "TRANS       | W/m2                | Transpiration";
   map_id[       "2_3_0"] = "SOTYP       | -                   | Soil Type";
   map_id[       "2_3_1"] = "UPLST       | K                   | Upper Layer Soil Temperature";
   map_id[       "2_3_2"] = "UPLSM       | kg/m^3              | Upper Layer Soil Moisture";
   map_id[       "2_3_3"] = "LOWLSM      | kg/m^3              | Lower Layer Soil Moisture";
   map_id[       "2_3_4"] = "BOTLST      | K                   | Bottom Layer Soil Temperature";
   map_id[       "2_3_5"] = "SOILL       | Proportion          | Liquid Volumetric Soil Moisture (non-frozen)";
   map_id[       "2_3_6"] = "RLYRS       | Numeric             | Number of Soil Layers in Root Zone";
   map_id[       "2_3_7"] = "SMREF       | Proportion          | Transpiration Stress-onset (soil moisture)";
   map_id[       "2_3_8"] = "SMDRY       | Proportion          | Direct Evaporation Cease (soil moisture)";
   map_id[       "2_3_9"] = "POROS       | Proportion          | Soil Porosity";
   map_id[      "2_3_10"] = "LIQVSM      | m3m-3               | Liquid Volumetric Soil Moisture (Non-Frozen)";
   map_id[      "2_3_11"] = "VOLTSO      | m3m-3               | Volumetric Transpiration Stree-Onset(Soil Moisture)";
   map_id[      "2_3_12"] = "TRANSO      | kg/m^3              | Transpiration Stree-Onset(Soil Moisture)";
   map_id[      "2_3_13"] = "VOLDEC      | m3m-3               | Volumetric Direct Evaporation Cease(Soil Moisture)";
   map_id[      "2_3_14"] = "DIREC       | kg/m^3              | Direct Evaporation Cease(Soil Moisture)";
   map_id[      "2_3_15"] = "SOILP       | m3m-3               | Soil Porosity";
   map_id[      "2_3_16"] = "VSOSM       | m3m-3               | Volumetric Saturation Of Soil Moisture";
   map_id[      "2_3_17"] = "SATOSM      | kg/m^3              | Saturation Of Soil Moisture";
   map_id[     "2_3_192"] = "SOILL       | Proportion          | Liquid Volumetric Soil Moisture (non Frozen)";
   map_id[     "2_3_193"] = "RLYRS       | non-dim             | Number of Soil Layers in Root Zone";
   map_id[     "2_3_194"] = "SLTYP       | Index               | Surface Slope Type";
   map_id[     "2_3_195"] = "SMREF       | Proportion          | Transpiration Stress-onset (soil moisture)";
   map_id[     "2_3_196"] = "SMDRY       | Proportion          | Direct Evaporation Cease (soil moisture)";
   map_id[     "2_3_197"] = "POROS       | Proportion          | Soil Porosity";
   map_id[     "2_3_198"] = "EVBS        | W/m-2               | Direct evaporation from bare soil";
   map_id[     "2_3_199"] = "LSPA        | kg/m^2              | Land Surface Precipitation Accumulation";
   map_id[     "2_3_200"] = "BARET       | K                   | Bare soil surface skin temperature";
   map_id[     "2_3_201"] = "AVSFT       | K                   | Average surface skin temperature";
   map_id[     "2_3_202"] = "RADT        | K                   | Effective radiative skin temperature";
   map_id[     "2_3_203"] = "FLDCP       | fraction            | Field Capacity";
   map_id[       "2_4_0"] = "FIRECRA     | %                   | Fire Outlook Critical Risk Area";
   map_id[       "2_4_1"] = "FIREXCRA    | %                   | Fire Outlook Extreme Critical Risk Area";
   map_id[       "2_4_2"] = "FIREDLA     | %                   | Fire Outlook Dry Lightning Area";
   map_id[       "2_4_3"] = "HINDEX      | -                   | Haines Index";
   map_id[       "3_0_0"] = "SRAD        | Numeric             | Scaled Radiance";
   map_id[       "3_0_1"] = "SALBEDO     | Numeric             | Scaled Albedo";
   map_id[       "3_0_2"] = "SBTMP       | Numeric             | Scaled Brightness Temperature";
   map_id[       "3_0_3"] = "SPWAT       | Numeric             | Scaled Precipitable Water";
   map_id[       "3_0_4"] = "SLFTI       | Numeric             | Scaled Lifted Index";
   map_id[       "3_0_5"] = "SCTPRES     | Numeric             | Scaled Cloud Top Pressure";
   map_id[       "3_0_6"] = "SSTMP       | Numeric             | Scaled Skin Temperature";
   map_id[       "3_0_7"] = "CLOUDM      | -                   | Cloud Mask";
   map_id[       "3_0_8"] = "PIXST       | -                   | Pixel scene type";
   map_id[       "3_0_9"] = "FIREDI      | -                   | Fire Detection Indicator";
   map_id[       "3_1_0"] = "ESTP        | kg/m^2              | Estimated Precipitation";
   map_id[       "3_1_1"] = "IRRATE      | kg/m^2/s            | Instantaneous Rain Rate";
   map_id[       "3_1_2"] = "CTOPH       | m                   | Cloud Top Height";
   map_id[       "3_1_3"] = "CTOPHQI     | -                   | Cloud Top Height Quality Indicator";
   map_id[       "3_1_4"] = "ESTUGRD     | 1/m                 | Estimated u-Component of Wind";
   map_id[       "3_1_5"] = "ESTVGRD     | 1/m                 | Estimated v-Component of Wind";
   map_id[       "3_1_6"] = "NPIXU       | Numeric             | Number Of Pixels Used";
   map_id[       "3_1_7"] = "SOLZA       | Degree              | Solar Zenith Angle";
   map_id[       "3_1_8"] = "RAZA        | Degree              | Relative Azimuth Angle";
   map_id[       "3_1_9"] = "RFL06       | %                   | Reflectance in 0.6 Micron Channel";
   map_id[      "3_1_10"] = "RFL08       | %                   | Reflectance in 0.8 Micron Channel";
   map_id[      "3_1_11"] = "RFL16       | %                   | Reflectance in 1.6 Micron Channel";
   map_id[      "3_1_12"] = "RFL39       | %                   | Reflectance in 3.9 Micron Channel";
   map_id[      "3_1_13"] = "ATMDIV      | 1/s                 | Atmospheric Divergence";
   map_id[      "3_1_19"] = "WINDS       | m/s                 | Wind Speed";
   map_id[      "3_1_20"] = "AOT06       | -                   | Aerosol Optical Thickness at 0.635 um";
   map_id[      "3_1_21"] = "AOT08       | -                   | Aerosol Optical Thickness at 0.810 um";
   map_id[      "3_1_22"] = "AOT16       | -                   | Aerosol Optical Thickness at 1.640 um";
   map_id[      "3_1_23"] = "ANGCOE      | -                   | Angstrom Coefficien";
   map_id[     "3_1_192"] = "USCT        | m/s                 | Scatterometer Estimated U Wind Component";
   map_id[     "3_1_193"] = "VSCT        | m/s                 | Scatterometer Estimated V Wind Component";
   map_id[     "3_192_0"] = "SBT122      | K                   | Simulated Brightness Temperature for GOES 12, Channel 2";
   map_id[     "3_192_1"] = "SBT123      | K                   | Simulated Brightness Temperature for GOES 12, Channel 3";
   map_id[     "3_192_2"] = "SBT124      | K                   | Simulated Brightness Temperature for GOES 12, Channel 4";
   map_id[     "3_192_3"] = "SBT126      | K                   | Simulated Brightness Temperature for GOES 12, Channel 6";
   map_id[     "3_192_4"] = "SBC123      | Byte                | Simulated Brightness Counts for GOES 12, Channel 3";
   map_id[     "3_192_5"] = "SBC124      | Byte                | Simulated Brightness Counts for GOES 12, Channel 4";
   map_id[     "3_192_6"] = "SBT112      | K                   | Simulated Brightness Temperature for GOES 11, Channel 2";
   map_id[     "3_192_7"] = "SBT113      | K                   | Simulated Brightness Temperature for GOES 11, Channel 3";
   map_id[     "3_192_8"] = "SBT114      | K                   | Simulated Brightness Temperature for GOES 11, Channel 4";
   map_id[     "3_192_9"] = "SBT115      | K                   | Simulated Brightness Temperature for GOES 11, Channel 5";
   map_id[      "10_0_0"] = "WVSP1       | non-dim             | Wave Spectra (1)";
   map_id[      "10_0_1"] = "WVSP2       | non-dim             | Wave Spectra (2)";
   map_id[      "10_0_2"] = "WVSP3       | non-dim             | Wave Spectra (3)";
   map_id[      "10_0_3"] = "HTSGW       | m                   | Significant Height of Combined Wind Waves and Swell";
   map_id[      "10_0_4"] = "WVDIR       | deg                 | Direction of Wind Waves";
   map_id[      "10_0_5"] = "WVHGT       | m                   | Significant Height of Wind Waves";
   map_id[      "10_0_6"] = "WVPER       | s                   | Mean Period of Wind Waves";
   map_id[      "10_0_7"] = "SWDIR       | deg                 | Direction of Swell Waves";
   map_id[      "10_0_8"] = "SWELL       | m                   | Significant Height of Swell Waves";
   map_id[      "10_0_9"] = "SWPER       | s                   | Mean Period of Swell Waves";
   map_id[     "10_0_10"] = "DIRPW       | deg                 | Primary Wave Direction";
   map_id[     "10_0_11"] = "PERPW       | s                   | Primary Wave Mean Period";
   map_id[     "10_0_12"] = "DIRSW       | deg                 | Secondary Wave Direction";
   map_id[     "10_0_13"] = "PERSW       | s                   | Secondary Wave Mean Period";
   map_id[     "10_0_14"] = "WWSDIR      | deg                 | Direction of Combined Wind Waves and Swell";
   map_id[     "10_0_15"] = "MWSPER      | s                   | Mean Period of Combined Wind Waves and Swell";
   map_id[    "10_0_192"] = "WSTP        | -                   | Wave Steepness";
   map_id[      "10_1_0"] = "DIRC        | deg                 | Current Direction";
   map_id[      "10_1_1"] = "SPC         | m/s                 | Current Speed";
   map_id[      "10_1_2"] = "UOGRD       | m/s                 | U-Component of Current";
   map_id[      "10_1_3"] = "VOGRD       | m/s                 | V-Component of Current";
   map_id[    "10_1_192"] = "OMLU        | m/s                 | Ocean Mixed Layer U Velocity";
   map_id[    "10_1_193"] = "OMLV        | m/s                 | Ocean Mixed Layer V Velocity";
   map_id[    "10_1_194"] = "UBARO       | m/s                 | Barotropic U velocity";
   map_id[    "10_1_195"] = "VBARO       | m/s                 | Barotropic V velocity";
   map_id[      "10_2_0"] = "ICEC        | Proportion          | Ice Cover";
   map_id[      "10_2_1"] = "ICETK       | m                   | Ice Thickness";
   map_id[      "10_2_2"] = "DICED       | deg                 | Direction of Ice Drift";
   map_id[      "10_2_3"] = "SICED       | m/s                 | Speed of Ice Drift";
   map_id[      "10_2_4"] = "UICE        | m/s                 | U-Component of Ice Drift";
   map_id[      "10_2_5"] = "VICE        | m/s                 | V-Component of Ice Drift";
   map_id[      "10_2_6"] = "ICEG        | m/s                 | Ice Growth Rate";
   map_id[      "10_2_7"] = "ICED        | 1/s                 | Ice Divergence";
   map_id[      "10_2_8"] = "ICET        | K                   | Ice Temperature";
   map_id[      "10_3_0"] = "WTMP        | K                   | Water Temperature";
   map_id[      "10_3_1"] = "DSLM        | m                   | Deviation of Sea Level from Mean";
   map_id[    "10_3_192"] = "SURGE       | m                   | Storm Surge";
   map_id[    "10_3_193"] = "ETSRG       | m                   | Extra Tropical Storm Surge";
   map_id[    "10_3_194"] = "ELEVhtml    | m                   | Ocean Surface Elevation Relative to Geoid";
   map_id[    "10_3_195"] = "SSHG        | m                   | Sea Surface Height Relative to Geoid";
   map_id[    "10_3_196"] = "P2OMLT      | kg/m^3              | Ocean Mixed Layer Potential Density (Reference 2000m)<br";
   map_id[    "10_3_197"] = "AOHFLX      | W/m^2               | Net Air-Ocean Heat Flux";
   map_id[    "10_3_198"] = "ASHFL       | W/m^2               | Assimilative Heat Flux";
   map_id[    "10_3_199"] = "SSTT        | degreeperday        | Surface Temperature Trend";
   map_id[    "10_3_200"] = "SSST        | psuperday           | Surface Salinity Trend";
   map_id[    "10_3_201"] = "KENG        | J/kg                | Kinetic Energy";
   map_id[    "10_3_202"] = "SLTFL       | kg/m^2/s            | Salt Flux";
   map_id[    "10_3_242"] = "TCSRG20     | m                   | 20% Tropical Cyclone Storm Surge Exceedance";
   map_id[    "10_3_243"] = "TCSRG30     | m                   | 30% Tropical Cyclone Storm Surge Exceedance";
   map_id[    "10_3_244"] = "TCSRG40     | m                   | 40% Tropical Cyclone Storm Surge Exceedance";
   map_id[    "10_3_245"] = "TCSRG50     | m                   | 50% Tropical Cyclone Storm Surge Exceedance";
   map_id[    "10_3_246"] = "TCSRG60     | m                   | 60% Tropical Cyclone Storm Surge Exceedance";
   map_id[    "10_3_247"] = "TCSRG70     | m                   | 70% Tropical Cyclone Storm Surge Exceedance";
   map_id[    "10_3_248"] = "TCSRG80     | m                   | 80% Tropical Cyclone Storm Surge Exceedance";
   map_id[    "10_3_249"] = "TCSRG90     | m                   | 90% Tropical Cyclone Storm Surge Exceedance";
   map_id[      "10_4_0"] = "MTHD        | m                   | Main Thermocline Depth";
   map_id[      "10_4_1"] = "MTHA        | m                   | Main Thermocline Anomaly";
   map_id[      "10_4_2"] = "TTHDP       | m                   | Transient Thermocline Depth";
   map_id[      "10_4_3"] = "SALTY       | kg/kg               | Salinity";
   map_id[      "10_4_4"] = "OVHD        | m^2/s               | Ocean Vertical Heat Diffusivity";
   map_id[      "10_4_5"] = "OVSD        | m^2/s               | Ocean Vertical Salt Diffusivity";
   map_id[      "10_4_6"] = "OVMD        | m^2/s               | Ocean Vertical Momentum Diffusivity";
   map_id[    "10_4_192"] = "WTMPC       | degc                | 3-D Temperature";
   map_id[    "10_4_193"] = "SALIN       | -                   | 3-D Salinity";
   map_id[    "10_4_194"] = "BKENG       | J/kg                | Barotropic Kinectic Energy";
   map_id[    "10_4_195"] = "DBSS        | m                   | Geometric Depth Below Sea Surface";
   map_id[    "10_4_196"] = "INTFD       | m                   | Interface Depths";
   map_id[    "10_4_197"] = "OHC         | Jm-2                | Ocean Heat Content";
   map_id[    "10_191_0"] = "TSEC        | s                   | Seconds Prior To Initial Reference Time (Defined In Section 1)";
   map_id[    "10_191_1"] = "MOSF        | m^3/s               | Meridional Overturning Stream Function";
   map_id[ "255_255_255"] = "IMGD        | -                   | Image data";

   map_code.clear();

   map_code.insert( pair<string,string>(     "TMP", "0_0_0") );
   map_code.insert( pair<string,string>(    "VTMP", "0_0_1") );
   map_code.insert( pair<string,string>(     "POT", "0_0_2") );
   map_code.insert( pair<string,string>(    "EPOT", "0_0_3") );
   map_code.insert( pair<string,string>(    "TMAX", "0_0_4") );
   map_code.insert( pair<string,string>(    "TMIN", "0_0_5") );
   map_code.insert( pair<string,string>(     "DPT", "0_0_6") );
   map_code.insert( pair<string,string>(    "DEPR", "0_0_7") );
   map_code.insert( pair<string,string>(    "LAPR", "0_0_8") );
   map_code.insert( pair<string,string>(    "TMPA", "0_0_9") );
   map_code.insert( pair<string,string>(   "LHTFL", "0_0_10") );
   map_code.insert( pair<string,string>(   "SHTFL", "0_0_11") );
   map_code.insert( pair<string,string>(   "HEATX", "0_0_12") );
   map_code.insert( pair<string,string>(     "WCF", "0_0_13") );
   map_code.insert( pair<string,string>(  "MINDPD", "0_0_14") );
   map_code.insert( pair<string,string>(   "VPTMP", "0_0_15") );
   map_code.insert( pair<string,string>(   "SNOHF", "0_0_16") );
   map_code.insert( pair<string,string>(   "SKINT", "0_0_17") );
   map_code.insert( pair<string,string>(   "SNOHF", "0_0_192") );
   map_code.insert( pair<string,string>(   "TTRAD", "0_0_193") );
   map_code.insert( pair<string,string>(     "REV", "0_0_194") );
   map_code.insert( pair<string,string>(   "LRGHR", "0_0_195") );
   map_code.insert( pair<string,string>(   "CNVHR", "0_0_196") );
   map_code.insert( pair<string,string>(   "THFLX", "0_0_197") );
   map_code.insert( pair<string,string>(   "TTDIA", "0_0_198") );
   map_code.insert( pair<string,string>(   "TTPHY", "0_0_199") );
   map_code.insert( pair<string,string>(   "TSD1D", "0_0_200") );
   map_code.insert( pair<string,string>(   "SHAHR", "0_0_201") );
   map_code.insert( pair<string,string>(   "VDFHR", "0_0_202") );
   map_code.insert( pair<string,string>(    "THZ0", "0_0_203") );
   map_code.insert( pair<string,string>(    "TCHP", "0_0_204") );
   map_code.insert( pair<string,string>(    "SPFH", "0_1_0") );
   map_code.insert( pair<string,string>(      "RH", "0_1_1") );
   map_code.insert( pair<string,string>(    "MIXR", "0_1_2") );
   map_code.insert( pair<string,string>(    "PWAT", "0_1_3") );
   map_code.insert( pair<string,string>(    "VAPP", "0_1_4") );
   map_code.insert( pair<string,string>(    "SATD", "0_1_5") );
   map_code.insert( pair<string,string>(     "EVP", "0_1_6") );
   map_code.insert( pair<string,string>(   "PRATE", "0_1_7") );
   map_code.insert( pair<string,string>(    "APCP", "0_1_8") );
   map_code.insert( pair<string,string>(   "NCPCP", "0_1_9") );
   map_code.insert( pair<string,string>(   "ACPCP", "0_1_10") );
   map_code.insert( pair<string,string>(    "SNOD", "0_1_11") );
   map_code.insert( pair<string,string>(   "SRWEQ", "0_1_12") );
   map_code.insert( pair<string,string>(   "WEASD", "0_1_13") );
   map_code.insert( pair<string,string>(    "SNOC", "0_1_14") );
   map_code.insert( pair<string,string>(    "SNOL", "0_1_15") );
   map_code.insert( pair<string,string>(    "SNOM", "0_1_16") );
   map_code.insert( pair<string,string>(   "SNOAG", "0_1_17") );
   map_code.insert( pair<string,string>(    "ABSH", "0_1_18") );
   map_code.insert( pair<string,string>(   "PTYPE", "0_1_19") );
   map_code.insert( pair<string,string>(   "ILIQW", "0_1_20") );
   map_code.insert( pair<string,string>(   "TCOND", "0_1_21") );
   map_code.insert( pair<string,string>(   "CLWMR", "0_1_22") );
   map_code.insert( pair<string,string>(    "ICMR", "0_1_23") );
   map_code.insert( pair<string,string>(    "RWMR", "0_1_24") );
   map_code.insert( pair<string,string>(    "SNMR", "0_1_25") );
   map_code.insert( pair<string,string>(   "MCONV", "0_1_26") );
   map_code.insert( pair<string,string>(   "MAXRH", "0_1_27") );
   map_code.insert( pair<string,string>(   "MAXAH", "0_1_28") );
   map_code.insert( pair<string,string>(   "ASNOW", "0_1_29") );
   map_code.insert( pair<string,string>(   "PWCAT", "0_1_30") );
   map_code.insert( pair<string,string>(    "HAIL", "0_1_31") );
   map_code.insert( pair<string,string>(    "GRLE", "0_1_32") );
   map_code.insert( pair<string,string>(   "CRAIN", "0_1_33") );
   map_code.insert( pair<string,string>(   "CFRZR", "0_1_34") );
   map_code.insert( pair<string,string>(   "CICEP", "0_1_35") );
   map_code.insert( pair<string,string>(   "CSNOW", "0_1_36") );
   map_code.insert( pair<string,string>(   "CPRAT", "0_1_37") );
   map_code.insert( pair<string,string>(   "MCONV", "0_1_38") );
   map_code.insert( pair<string,string>(   "CPOFP", "0_1_39") );
   map_code.insert( pair<string,string>(   "PEVAP", "0_1_40") );
   map_code.insert( pair<string,string>(   "PEVPR", "0_1_41") );
   map_code.insert( pair<string,string>(   "SNOWC", "0_1_42") );
   map_code.insert( pair<string,string>(   "FRAIN", "0_1_43") );
   map_code.insert( pair<string,string>(    "RIME", "0_1_44") );
   map_code.insert( pair<string,string>(   "TCOLR", "0_1_45") );
   map_code.insert( pair<string,string>(   "TCOLS", "0_1_46") );
   map_code.insert( pair<string,string>(    "LSWP", "0_1_47") );
   map_code.insert( pair<string,string>(     "CWP", "0_1_48") );
   map_code.insert( pair<string,string>(   "TWATP", "0_1_49") );
   map_code.insert( pair<string,string>(  "TSNOWP", "0_1_50") );
   map_code.insert( pair<string,string>(   "TCWAT", "0_1_51") );
   map_code.insert( pair<string,string>(  "TPRATE", "0_1_52") );
   map_code.insert( pair<string,string>(   "TSRWE", "0_1_53") );
   map_code.insert( pair<string,string>( "LSPRATE", "0_1_54") );
   map_code.insert( pair<string,string>(   "CSRWE", "0_1_55") );
   map_code.insert( pair<string,string>(  "LSSRWE", "0_1_56") );
   map_code.insert( pair<string,string>(  "TSRATE", "0_1_57") );
   map_code.insert( pair<string,string>(  "CSRATE", "0_1_58") );
   map_code.insert( pair<string,string>( "LSSRATE", "0_1_59") );
   map_code.insert( pair<string,string>(    "SDWE", "0_1_60") );
   map_code.insert( pair<string,string>(    "SDEN", "0_1_61") );
   map_code.insert( pair<string,string>(   "SEVAP", "0_1_62") );
   map_code.insert( pair<string,string>(   "TCIWV", "0_1_64") );
   map_code.insert( pair<string,string>(  "RPRATE", "0_1_65") );
   map_code.insert( pair<string,string>(  "SPRATE", "0_1_66") );
   map_code.insert( pair<string,string>(  "FPRATE", "0_1_67") );
   map_code.insert( pair<string,string>(  "IPRATE", "0_1_68") );
   map_code.insert( pair<string,string>(   "CRAIN", "0_1_192") );
   map_code.insert( pair<string,string>(   "CFRZR", "0_1_193") );
   map_code.insert( pair<string,string>(   "CICEP", "0_1_194") );
   map_code.insert( pair<string,string>(   "CSNOW", "0_1_195") );
   map_code.insert( pair<string,string>(   "CPRAT", "0_1_196") );
   map_code.insert( pair<string,string>(   "MCONV", "0_1_197") );
   map_code.insert( pair<string,string>(   "MINRH", "0_1_198") );
   map_code.insert( pair<string,string>(   "PEVAP", "0_1_199") );
   map_code.insert( pair<string,string>(   "PEVPR", "0_1_200") );
   map_code.insert( pair<string,string>(   "SNOWC", "0_1_201") );
   map_code.insert( pair<string,string>(   "FRAIN", "0_1_202") );
   map_code.insert( pair<string,string>(    "RIME", "0_1_203") );
   map_code.insert( pair<string,string>(   "TCOLR", "0_1_204") );
   map_code.insert( pair<string,string>(   "TCOLS", "0_1_205") );
   map_code.insert( pair<string,string>(    "TIPD", "0_1_206") );
   map_code.insert( pair<string,string>(    "NCIP", "0_1_207") );
   map_code.insert( pair<string,string>(    "SNOT", "0_1_208") );
   map_code.insert( pair<string,string>(   "TCLSW", "0_1_209") );
   map_code.insert( pair<string,string>(   "TCOLM", "0_1_210") );
   map_code.insert( pair<string,string>(    "EMNP", "0_1_211") );
   map_code.insert( pair<string,string>(   "SBSNO", "0_1_212") );
   map_code.insert( pair<string,string>(   "CNVMR", "0_1_213") );
   map_code.insert( pair<string,string>(   "SHAMR", "0_1_214") );
   map_code.insert( pair<string,string>(   "VDFMR", "0_1_215") );
   map_code.insert( pair<string,string>(   "CONDP", "0_1_216") );
   map_code.insert( pair<string,string>(   "LRGMR", "0_1_217") );
   map_code.insert( pair<string,string>(     "QZ0", "0_1_218") );
   map_code.insert( pair<string,string>(    "QMAX", "0_1_219") );
   map_code.insert( pair<string,string>(    "QMIN", "0_1_220") );
   map_code.insert( pair<string,string>(   "ARAIN", "0_1_221") );
   map_code.insert( pair<string,string>(   "SNOWT", "0_1_222") );
   map_code.insert( pair<string,string>(   "APCPN", "0_1_223") );
   map_code.insert( pair<string,string>(  "ACPCPN", "0_1_224") );
   map_code.insert( pair<string,string>(    "FRZR", "0_1_225") );
   map_code.insert( pair<string,string>(  "PWTHER", "0_1_226") );
   map_code.insert( pair<string,string>(   "FROZR", "0_1_227") );
   map_code.insert( pair<string,string>(   "TSNOW", "0_1_241") );
   map_code.insert( pair<string,string>(    "WDIR", "0_2_0") );
   map_code.insert( pair<string,string>(    "WIND", "0_2_1") );
   map_code.insert( pair<string,string>(    "UGRD", "0_2_2") );
   map_code.insert( pair<string,string>(    "VGRD", "0_2_3") );
   map_code.insert( pair<string,string>(    "STRM", "0_2_4") );
   map_code.insert( pair<string,string>(    "VPOT", "0_2_5") );
   map_code.insert( pair<string,string>(   "MNTSF", "0_2_6") );
   map_code.insert( pair<string,string>(   "SGCVV", "0_2_7") );
   map_code.insert( pair<string,string>(    "VVEL", "0_2_8") );
   map_code.insert( pair<string,string>(    "DZDT", "0_2_9") );
   map_code.insert( pair<string,string>(    "ABSV", "0_2_10") );
   map_code.insert( pair<string,string>(    "ABSD", "0_2_11") );
   map_code.insert( pair<string,string>(    "RELV", "0_2_12") );
   map_code.insert( pair<string,string>(    "RELD", "0_2_13") );
   map_code.insert( pair<string,string>(   "PVORT", "0_2_14") );
   map_code.insert( pair<string,string>(   "VUCSH", "0_2_15") );
   map_code.insert( pair<string,string>(   "VVCSH", "0_2_16") );
   map_code.insert( pair<string,string>(    "UFLX", "0_2_17") );
   map_code.insert( pair<string,string>(    "VFLX", "0_2_18") );
   map_code.insert( pair<string,string>(   "WMIXE", "0_2_19") );
   map_code.insert( pair<string,string>(   "BLYDP", "0_2_20") );
   map_code.insert( pair<string,string>( "MAXGUST", "0_2_21") );
   map_code.insert( pair<string,string>(    "GUST", "0_2_22") );
   map_code.insert( pair<string,string>(   "UGUST", "0_2_23") );
   map_code.insert( pair<string,string>(   "VGUST", "0_2_24") );
   map_code.insert( pair<string,string>(    "VWSH", "0_2_25") );
   map_code.insert( pair<string,string>(    "MFLX", "0_2_26") );
   map_code.insert( pair<string,string>(    "USTM", "0_2_27") );
   map_code.insert( pair<string,string>(    "VSTM", "0_2_28") );
   map_code.insert( pair<string,string>(      "CD", "0_2_29") );
   map_code.insert( pair<string,string>(   "FRICV", "0_2_30") );
   map_code.insert( pair<string,string>(  "ETACVV", "0_2_32") );
   map_code.insert( pair<string,string>(    "VWSH", "0_2_192") );
   map_code.insert( pair<string,string>(    "MFLX", "0_2_193") );
   map_code.insert( pair<string,string>(    "USTM", "0_2_194") );
   map_code.insert( pair<string,string>(    "VSTM", "0_2_195") );
   map_code.insert( pair<string,string>(      "CD", "0_2_196") );
   map_code.insert( pair<string,string>(   "FRICV", "0_2_197") );
   map_code.insert( pair<string,string>(    "LAUV", "0_2_198") );
   map_code.insert( pair<string,string>(    "LOUV", "0_2_199") );
   map_code.insert( pair<string,string>(    "LAVV", "0_2_200") );
   map_code.insert( pair<string,string>(    "LOVV", "0_2_201") );
   map_code.insert( pair<string,string>(    "LAPP", "0_2_202") );
   map_code.insert( pair<string,string>(    "LOPP", "0_2_203") );
   map_code.insert( pair<string,string>(    "VEDH", "0_2_204") );
   map_code.insert( pair<string,string>(   "COVMZ", "0_2_205") );
   map_code.insert( pair<string,string>(   "COVTZ", "0_2_206") );
   map_code.insert( pair<string,string>(   "COVTM", "0_2_207") );
   map_code.insert( pair<string,string>(   "VDFUA", "0_2_208") );
   map_code.insert( pair<string,string>(   "VDFVA", "0_2_209") );
   map_code.insert( pair<string,string>(    "GWDU", "0_2_210") );
   map_code.insert( pair<string,string>(    "GWDV", "0_2_211") );
   map_code.insert( pair<string,string>(    "CNVU", "0_2_212") );
   map_code.insert( pair<string,string>(    "CNVV", "0_2_213") );
   map_code.insert( pair<string,string>(   "WTEND", "0_2_214") );
   map_code.insert( pair<string,string>(  "OMGALF", "0_2_215") );
   map_code.insert( pair<string,string>(  "CNGWDU", "0_2_216") );
   map_code.insert( pair<string,string>(  "CNGWDV", "0_2_217") );
   map_code.insert( pair<string,string>(     "LMV", "0_2_218") );
   map_code.insert( pair<string,string>(   "PVMWW", "0_2_219") );
   map_code.insert( pair<string,string>(  "MAXUVV", "0_2_220") );
   map_code.insert( pair<string,string>(  "MAXDVV", "0_2_221") );
   map_code.insert( pair<string,string>(   "MAXUW", "0_2_222") );
   map_code.insert( pair<string,string>(   "MAXVW", "0_2_223") );
   map_code.insert( pair<string,string>(   "VRATE", "0_2_224") );
   map_code.insert( pair<string,string>(    "PRES", "0_3_0") );
   map_code.insert( pair<string,string>(   "PRMSL", "0_3_1") );
   map_code.insert( pair<string,string>(   "PTEND", "0_3_2") );
   map_code.insert( pair<string,string>(   "ICAHT", "0_3_3") );
   map_code.insert( pair<string,string>(      "GP", "0_3_4") );
   map_code.insert( pair<string,string>(     "HGT", "0_3_5") );
   map_code.insert( pair<string,string>(    "DIST", "0_3_6") );
   map_code.insert( pair<string,string>(   "HSTDV", "0_3_7") );
   map_code.insert( pair<string,string>(   "PRESA", "0_3_8") );
   map_code.insert( pair<string,string>(     "GPA", "0_3_9") );
   map_code.insert( pair<string,string>(     "DEN", "0_3_10") );
   map_code.insert( pair<string,string>(    "ALTS", "0_3_11") );
   map_code.insert( pair<string,string>(   "THICK", "0_3_12") );
   map_code.insert( pair<string,string>( "PRESALT", "0_3_13") );
   map_code.insert( pair<string,string>(  "DENALT", "0_3_14") );
   map_code.insert( pair<string,string>(   "5WAVH", "0_3_15") );
   map_code.insert( pair<string,string>(   "U-GWD", "0_3_16") );
   map_code.insert( pair<string,string>(   "V-GWD", "0_3_17") );
   map_code.insert( pair<string,string>(    "HPBL", "0_3_18") );
   map_code.insert( pair<string,string>(   "5WAVA", "0_3_19") );
   map_code.insert( pair<string,string>(  "SDSGSO", "0_3_20") );
   map_code.insert( pair<string,string>(  "AOSGSO", "0_3_21") );
   map_code.insert( pair<string,string>(   "SSGSO", "0_3_22") );
   map_code.insert( pair<string,string>(   "GSGSO", "0_3_23") );
   map_code.insert( pair<string,string>(   "ASGSO", "0_3_24") );
   map_code.insert( pair<string,string>(  "NLPRES", "0_3_25") );
   map_code.insert( pair<string,string>(   "MSLET", "0_3_192") );
   map_code.insert( pair<string,string>(   "5WAVH", "0_3_193") );
   map_code.insert( pair<string,string>(   "U-GWD", "0_3_194") );
   map_code.insert( pair<string,string>(   "V-GWD", "0_3_195") );
   map_code.insert( pair<string,string>(    "HPBL", "0_3_196") );
   map_code.insert( pair<string,string>(   "5WAVA", "0_3_197") );
   map_code.insert( pair<string,string>(   "MSLMA", "0_3_198") );
   map_code.insert( pair<string,string>(   "TSLSA", "0_3_199") );
   map_code.insert( pair<string,string>(    "PLPL", "0_3_200") );
   map_code.insert( pair<string,string>(    "LPSX", "0_3_201") );
   map_code.insert( pair<string,string>(    "LPSY", "0_3_202") );
   map_code.insert( pair<string,string>(    "HGTX", "0_3_203") );
   map_code.insert( pair<string,string>(    "HGTY", "0_3_204") );
   map_code.insert( pair<string,string>(   "LAYTH", "0_3_205") );
   map_code.insert( pair<string,string>(   "NLGSP", "0_3_206") );
   map_code.insert( pair<string,string>(  "CNVUMF", "0_3_207") );
   map_code.insert( pair<string,string>(  "CNVDMF", "0_3_208") );
   map_code.insert( pair<string,string>( "CNVDEMF", "0_3_209") );
   map_code.insert( pair<string,string>(     "LMH", "0_3_210") );
   map_code.insert( pair<string,string>(    "HGTN", "0_3_211") );
   map_code.insert( pair<string,string>(   "PRESN", "0_3_212") );
   map_code.insert( pair<string,string>(   "NSWRS", "0_4_0") );
   map_code.insert( pair<string,string>(   "NSWRT", "0_4_1") );
   map_code.insert( pair<string,string>(   "SWAVR", "0_4_2") );
   map_code.insert( pair<string,string>(    "GRAD", "0_4_3") );
   map_code.insert( pair<string,string>(   "BRTMP", "0_4_4") );
   map_code.insert( pair<string,string>(   "LWRAD", "0_4_5") );
   map_code.insert( pair<string,string>(   "SWRAD", "0_4_6") );
   map_code.insert( pair<string,string>(   "DSWRF", "0_4_7") );
   map_code.insert( pair<string,string>(   "USWRF", "0_4_8") );
   map_code.insert( pair<string,string>(   "NSWRF", "0_4_9") );
   map_code.insert( pair<string,string>(  "PHOTAR", "0_4_10") );
   map_code.insert( pair<string,string>( "NSWRFCS", "0_4_11") );
   map_code.insert( pair<string,string>(   "DWUVR", "0_4_12") );
   map_code.insert( pair<string,string>(  "UVIUCS", "0_4_50") );
   map_code.insert( pair<string,string>(     "UVI", "0_4_51") );
   map_code.insert( pair<string,string>(   "DSWRF", "0_4_192") );
   map_code.insert( pair<string,string>(   "USWRF", "0_4_193") );
   map_code.insert( pair<string,string>(    "DUVB", "0_4_194") );
   map_code.insert( pair<string,string>(   "CDUVB", "0_4_195") );
   map_code.insert( pair<string,string>(   "CSDSF", "0_4_196") );
   map_code.insert( pair<string,string>(    "SWHR", "0_4_197") );
   map_code.insert( pair<string,string>(   "CSUSF", "0_4_198") );
   map_code.insert( pair<string,string>(   "CFNSF", "0_4_199") );
   map_code.insert( pair<string,string>(   "VBDSF", "0_4_200") );
   map_code.insert( pair<string,string>(   "VDDSF", "0_4_201") );
   map_code.insert( pair<string,string>(   "NBDSF", "0_4_202") );
   map_code.insert( pair<string,string>(   "NDDSF", "0_4_203") );
   map_code.insert( pair<string,string>(    "DTRF", "0_4_204") );
   map_code.insert( pair<string,string>(    "UTRF", "0_4_205") );
   map_code.insert( pair<string,string>(   "NLWRS", "0_5_0") );
   map_code.insert( pair<string,string>(   "NLWRT", "0_5_1") );
   map_code.insert( pair<string,string>(   "LWAVR", "0_5_2") );
   map_code.insert( pair<string,string>(   "DLWRF", "0_5_3") );
   map_code.insert( pair<string,string>(   "ULWRF", "0_5_4") );
   map_code.insert( pair<string,string>(   "NLWRF", "0_5_5") );
   map_code.insert( pair<string,string>(  "NLWRCS", "0_5_6") );
   map_code.insert( pair<string,string>(   "DLWRF", "0_5_192") );
   map_code.insert( pair<string,string>(   "ULWRF", "0_5_193") );
   map_code.insert( pair<string,string>(    "LWHR", "0_5_194") );
   map_code.insert( pair<string,string>(   "CSULF", "0_5_195") );
   map_code.insert( pair<string,string>(   "CSDLF", "0_5_196") );
   map_code.insert( pair<string,string>(   "CFNLF", "0_5_197") );
   map_code.insert( pair<string,string>(    "CICE", "0_6_0") );
   map_code.insert( pair<string,string>(    "TCDC", "0_6_1") );
   map_code.insert( pair<string,string>(   "CDCON", "0_6_2") );
   map_code.insert( pair<string,string>(    "LCDC", "0_6_3") );
   map_code.insert( pair<string,string>(    "MCDC", "0_6_4") );
   map_code.insert( pair<string,string>(    "HCDC", "0_6_5") );
   map_code.insert( pair<string,string>(    "CWAT", "0_6_6") );
   map_code.insert( pair<string,string>(    "CDCA", "0_6_7") );
   map_code.insert( pair<string,string>(    "CDCT", "0_6_8") );
   map_code.insert( pair<string,string>(   "TMAXT", "0_6_9") );
   map_code.insert( pair<string,string>(   "THUNC", "0_6_10") );
   map_code.insert( pair<string,string>(    "CDCB", "0_6_11") );
   map_code.insert( pair<string,string>(    "CDCT", "0_6_12") );
   map_code.insert( pair<string,string>(    "CEIL", "0_6_13") );
   map_code.insert( pair<string,string>(   "CDLYR", "0_6_14") );
   map_code.insert( pair<string,string>(   "CWORK", "0_6_15") );
   map_code.insert( pair<string,string>(   "CUEFI", "0_6_16") );
   map_code.insert( pair<string,string>(   "TCOND", "0_6_17") );
   map_code.insert( pair<string,string>(   "TCOLW", "0_6_18") );
   map_code.insert( pair<string,string>(   "TCOLI", "0_6_19") );
   map_code.insert( pair<string,string>(   "TCOLC", "0_6_20") );
   map_code.insert( pair<string,string>(    "FICE", "0_6_21") );
   map_code.insert( pair<string,string>(    "CDCC", "0_6_22") );
   map_code.insert( pair<string,string>(  "CDCIMR", "0_6_23") );
   map_code.insert( pair<string,string>(    "SUNS", "0_6_24") );
   map_code.insert( pair<string,string>(    "CBHE", "0_6_25") );
   map_code.insert( pair<string,string>(   "SUNSD", "0_6_33") );
   map_code.insert( pair<string,string>(   "CDLYR", "0_6_192") );
   map_code.insert( pair<string,string>(   "CWORK", "0_6_193") );
   map_code.insert( pair<string,string>(   "CUEFI", "0_6_194") );
   map_code.insert( pair<string,string>(   "TCOND", "0_6_195") );
   map_code.insert( pair<string,string>(   "TCOLW", "0_6_196") );
   map_code.insert( pair<string,string>(   "TCOLI", "0_6_197") );
   map_code.insert( pair<string,string>(   "TCOLC", "0_6_198") );
   map_code.insert( pair<string,string>(    "FICE", "0_6_199") );
   map_code.insert( pair<string,string>(   "MFLUX", "0_6_200") );
   map_code.insert( pair<string,string>(   "SUNSD", "0_6_201") );
   map_code.insert( pair<string,string>(     "PLI", "0_7_0") );
   map_code.insert( pair<string,string>(     "BLI", "0_7_1") );
   map_code.insert( pair<string,string>(      "KX", "0_7_2") );
   map_code.insert( pair<string,string>(     "KOX", "0_7_3") );
   map_code.insert( pair<string,string>(  "TOTALX", "0_7_4") );
   map_code.insert( pair<string,string>(      "SX", "0_7_5") );
   map_code.insert( pair<string,string>(    "CAPE", "0_7_6") );
   map_code.insert( pair<string,string>(     "CIN", "0_7_7") );
   map_code.insert( pair<string,string>(    "HLCY", "0_7_8") );
   map_code.insert( pair<string,string>(    "EHLX", "0_7_9") );
   map_code.insert( pair<string,string>(    "LFTX", "0_7_10") );
   map_code.insert( pair<string,string>(   "4LFTX", "0_7_11") );
   map_code.insert( pair<string,string>(      "RI", "0_7_12") );
   map_code.insert( pair<string,string>(    "LFTX", "0_7_192") );
   map_code.insert( pair<string,string>(   "4LFTX", "0_7_193") );
   map_code.insert( pair<string,string>(      "RI", "0_7_194") );
   map_code.insert( pair<string,string>(    "CWDI", "0_7_195") );
   map_code.insert( pair<string,string>(     "UVI", "0_7_196") );
   map_code.insert( pair<string,string>(    "UPHL", "0_7_197") );
   map_code.insert( pair<string,string>(     "LAI", "0_7_198") );
   map_code.insert( pair<string,string>(  "MXUPHL", "0_7_199") );
   map_code.insert( pair<string,string>(   "AEROT", "0_13_0") );
   map_code.insert( pair<string,string>(    "PMTC", "0_13_192") );
   map_code.insert( pair<string,string>(    "PMTF", "0_13_193") );
   map_code.insert( pair<string,string>(   "LPMTF", "0_13_194") );
   map_code.insert( pair<string,string>(   "LIPMF", "0_13_195") );
   map_code.insert( pair<string,string>(   "TOZNE", "0_14_0") );
   map_code.insert( pair<string,string>(    "O3MR", "0_14_1") );
   map_code.insert( pair<string,string>(   "TCIOZ", "0_14_2") );
   map_code.insert( pair<string,string>(    "O3MR", "0_14_192") );
   map_code.insert( pair<string,string>(   "OZCON", "0_14_193") );
   map_code.insert( pair<string,string>(   "OZCAT", "0_14_194") );
   map_code.insert( pair<string,string>(   "VDFOZ", "0_14_195") );
   map_code.insert( pair<string,string>(     "POZ", "0_14_196") );
   map_code.insert( pair<string,string>(     "TOZ", "0_14_197") );
   map_code.insert( pair<string,string>(    "POZT", "0_14_198") );
   map_code.insert( pair<string,string>(    "POZO", "0_14_199") );
   map_code.insert( pair<string,string>(  "OZMAX1", "0_14_200") );
   map_code.insert( pair<string,string>(  "OZMAX8", "0_14_201") );
   map_code.insert( pair<string,string>(  "PDMAX1", "0_14_202") );
   map_code.insert( pair<string,string>( "PDMAX24", "0_14_203") );
   map_code.insert( pair<string,string>(   "BSWID", "0_15_0") );
   map_code.insert( pair<string,string>(    "BREF", "0_15_1") );
   map_code.insert( pair<string,string>(   "BRVEL", "0_15_2") );
   map_code.insert( pair<string,string>(   "VERIL", "0_15_3") );
   map_code.insert( pair<string,string>(  "LMAXBR", "0_15_4") );
   map_code.insert( pair<string,string>(    "PREC", "0_15_5") );
   map_code.insert( pair<string,string>(   "RDSP1", "0_15_6") );
   map_code.insert( pair<string,string>(   "RDSP2", "0_15_7") );
   map_code.insert( pair<string,string>(   "RDSP3", "0_15_8") );
   map_code.insert( pair<string,string>(   "REFZR", "0_16_0") );
   map_code.insert( pair<string,string>(   "REFZI", "0_16_1") );
   map_code.insert( pair<string,string>(   "REFZC", "0_16_2") );
   map_code.insert( pair<string,string>(   "RETOP", "0_16_3") );
   map_code.insert( pair<string,string>(    "REFD", "0_16_4") );
   map_code.insert( pair<string,string>(    "REFC", "0_16_5") );
   map_code.insert( pair<string,string>(   "REFZR", "0_16_192") );
   map_code.insert( pair<string,string>(   "REFZI", "0_16_193") );
   map_code.insert( pair<string,string>(   "REFZC", "0_16_194") );
   map_code.insert( pair<string,string>(    "REFD", "0_16_195") );
   map_code.insert( pair<string,string>(    "REFC", "0_16_196") );
   map_code.insert( pair<string,string>(   "RETOP", "0_16_197") );
   map_code.insert( pair<string,string>(  "MAXREF", "0_16_198") );
   map_code.insert( pair<string,string>(    "LTNG", "0_17_192") );
   map_code.insert( pair<string,string>(   "ACCES", "0_18_0") );
   map_code.insert( pair<string,string>(   "ACIOD", "0_18_1") );
   map_code.insert( pair<string,string>(  "ACRADP", "0_18_2") );
   map_code.insert( pair<string,string>(   "GDCES", "0_18_3") );
   map_code.insert( pair<string,string>(   "GDIOD", "0_18_4") );
   map_code.insert( pair<string,string>(  "GDRADP", "0_18_5") );
   map_code.insert( pair<string,string>(  "TIACCP", "0_18_6") );
   map_code.insert( pair<string,string>(  "TIACIP", "0_18_7") );
   map_code.insert( pair<string,string>(  "TIACRP", "0_18_8") );
   map_code.insert( pair<string,string>(     "VIS", "0_19_0") );
   map_code.insert( pair<string,string>(   "ALBDO", "0_19_1") );
   map_code.insert( pair<string,string>(    "TSTM", "0_19_2") );
   map_code.insert( pair<string,string>(   "MIXHT", "0_19_3") );
   map_code.insert( pair<string,string>(  "VOLASH", "0_19_4") );
   map_code.insert( pair<string,string>(    "ICIT", "0_19_5") );
   map_code.insert( pair<string,string>(    "ICIB", "0_19_6") );
   map_code.insert( pair<string,string>(     "ICI", "0_19_7") );
   map_code.insert( pair<string,string>(   "TURBT", "0_19_8") );
   map_code.insert( pair<string,string>(   "TURBB", "0_19_9") );
   map_code.insert( pair<string,string>(    "TURB", "0_19_10") );
   map_code.insert( pair<string,string>(     "TKE", "0_19_11") );
   map_code.insert( pair<string,string>(  "PBLREG", "0_19_12") );
   map_code.insert( pair<string,string>(   "CONTI", "0_19_13") );
   map_code.insert( pair<string,string>(  "CONTET", "0_19_14") );
   map_code.insert( pair<string,string>(   "CONTT", "0_19_15") );
   map_code.insert( pair<string,string>(   "CONTB", "0_19_16") );
   map_code.insert( pair<string,string>(  "MXSALB", "0_19_17") );
   map_code.insert( pair<string,string>(  "SNFALB", "0_19_18") );
   map_code.insert( pair<string,string>(   "SALBD", "0_19_19") );
   map_code.insert( pair<string,string>(    "ICIP", "0_19_20") );
   map_code.insert( pair<string,string>(     "CTP", "0_19_21") );
   map_code.insert( pair<string,string>(     "CAT", "0_19_22") );
   map_code.insert( pair<string,string>(    "SLDP", "0_19_23") );
   map_code.insert( pair<string,string>(  "MXSALB", "0_19_192") );
   map_code.insert( pair<string,string>(  "SNFALB", "0_19_193") );
   map_code.insert( pair<string,string>(  "SRCONO", "0_19_194") );
   map_code.insert( pair<string,string>(  "MRCONO", "0_19_195") );
   map_code.insert( pair<string,string>(  "HRCONO", "0_19_196") );
   map_code.insert( pair<string,string>( "TORPROB", "0_19_197") );
   map_code.insert( pair<string,string>("HAILPROB", "0_19_198") );
   map_code.insert( pair<string,string>("WINDPROB", "0_19_199") );
   map_code.insert( pair<string,string>("STORPROB", "0_19_200") );
   map_code.insert( pair<string,string>("SHAILPRO", "0_19_201") );
   map_code.insert( pair<string,string>("SWINDPRO", "0_19_202") );
   map_code.insert( pair<string,string>(   "TSTMC", "0_19_203") );
   map_code.insert( pair<string,string>(   "MIXLY", "0_19_204") );
   map_code.insert( pair<string,string>(   "FLGHT", "0_19_205") );
   map_code.insert( pair<string,string>(   "CICEL", "0_19_206") );
   map_code.insert( pair<string,string>(   "CIVIS", "0_19_207") );
   map_code.insert( pair<string,string>(   "CIFLT", "0_19_208") );
   map_code.insert( pair<string,string>(   "LAVNI", "0_19_209") );
   map_code.insert( pair<string,string>(   "HAVNI", "0_19_210") );
   map_code.insert( pair<string,string>(  "SBSALB", "0_19_211") );
   map_code.insert( pair<string,string>(  "SWSALB", "0_19_212") );
   map_code.insert( pair<string,string>(  "NBSALB", "0_19_213") );
   map_code.insert( pair<string,string>(  "NWSALB", "0_19_214") );
   map_code.insert( pair<string,string>(   "PRSVR", "0_19_215") );
   map_code.insert( pair<string,string>("PRSIGSVR", "0_19_216") );
   map_code.insert( pair<string,string>(    "SIPD", "0_19_217") );
   map_code.insert( pair<string,string>(    "EPSR", "0_19_218") );
   map_code.insert( pair<string,string>(    "TPFI", "0_19_219") );
   map_code.insert( pair<string,string>(   "VAFTD", "0_19_232") );
   map_code.insert( pair<string,string>( "MASSDEN", "0_20_0") );
   map_code.insert( pair<string,string>(   "COLMD", "0_20_1") );
   map_code.insert( pair<string,string>(  "MASSMR", "0_20_2") );
   map_code.insert( pair<string,string>(  "AEMFLX", "0_20_3") );
   map_code.insert( pair<string,string>( "ANPMFLX", "0_20_4") );
   map_code.insert( pair<string,string>("ANPEMFLX", "0_20_5") );
   map_code.insert( pair<string,string>( "SDDMFLX", "0_20_6") );
   map_code.insert( pair<string,string>( "SWDMFLX", "0_20_7") );
   map_code.insert( pair<string,string>( "AREMFLX", "0_20_8") );
   map_code.insert( pair<string,string>(     "AIA", "0_20_50") );
   map_code.insert( pair<string,string>(  "CONAIR", "0_20_51") );
   map_code.insert( pair<string,string>(    "VMXR", "0_20_52") );
   map_code.insert( pair<string,string>(   "CGPRC", "0_20_53") );
   map_code.insert( pair<string,string>(   "CGDRC", "0_20_54") );
   map_code.insert( pair<string,string>(   "SFLUX", "0_20_55") );
   map_code.insert( pair<string,string>(   "COAIA", "0_20_56") );
   map_code.insert( pair<string,string>(   "TYABA", "0_20_57") );
   map_code.insert( pair<string,string>(   "TYAAL", "0_20_58") );
   map_code.insert( pair<string,string>(   "SADEN", "0_20_100") );
   map_code.insert( pair<string,string>(    "AOTK", "0_20_101") );
   map_code.insert( pair<string,string>( "NO2TROP", "0_20_131") );
   map_code.insert( pair<string,string>(  "NO2VCD", "0_20_132") );
   map_code.insert( pair<string,string>(  "BROVCD", "0_20_133") );
   map_code.insert( pair<string,string>( "HCHOVCD", "0_20_134") );
   map_code.insert( pair<string,string>("var190m0", "0_190_0") );
   map_code.insert( pair<string,string>(    "TSEC", "0_191_0") );
   map_code.insert( pair<string,string>(    "NLAT", "0_191_192") );
   map_code.insert( pair<string,string>(    "ELON", "0_191_193") );
   map_code.insert( pair<string,string>(    "TSEC", "0_191_194") );
   map_code.insert( pair<string,string>(   "MLYNO", "0_191_195") );
   map_code.insert( pair<string,string>(   "NLATN", "0_191_196") );
   map_code.insert( pair<string,string>(   "ELONN", "0_191_197") );
   map_code.insert( pair<string,string>(   "COVMZ", "0_192_1") );
   map_code.insert( pair<string,string>(   "COVTZ", "0_192_2") );
   map_code.insert( pair<string,string>(   "COVTM", "0_192_3") );
   map_code.insert( pair<string,string>(   "COVTW", "0_192_4") );
   map_code.insert( pair<string,string>(   "COVZZ", "0_192_5") );
   map_code.insert( pair<string,string>(   "COVMM", "0_192_6") );
   map_code.insert( pair<string,string>(   "COVQZ", "0_192_7") );
   map_code.insert( pair<string,string>(   "COVQM", "0_192_8") );
   map_code.insert( pair<string,string>(  "COVTVV", "0_192_9") );
   map_code.insert( pair<string,string>(  "COVQVV", "0_192_10") );
   map_code.insert( pair<string,string>( "COVPSPS", "0_192_11") );
   map_code.insert( pair<string,string>(   "COVQQ", "0_192_12") );
   map_code.insert( pair<string,string>( "COVVVVV", "0_192_13") );
   map_code.insert( pair<string,string>(   "COVTT", "0_192_14") );
   map_code.insert( pair<string,string>(   "FFLDG", "1_0_0") );
   map_code.insert( pair<string,string>(  "FFLDRO", "1_0_1") );
   map_code.insert( pair<string,string>(    "RSSC", "1_0_2") );
   map_code.insert( pair<string,string>(    "ESCT", "1_0_3") );
   map_code.insert( pair<string,string>(  "SWEPON", "1_0_4") );
   map_code.insert( pair<string,string>(   "BGRUN", "1_0_5") );
   map_code.insert( pair<string,string>(   "SSRUN", "1_0_6") );
   map_code.insert( pair<string,string>(   "BGRUN", "1_0_192") );
   map_code.insert( pair<string,string>(   "SSRUN", "1_0_193") );
   map_code.insert( pair<string,string>(   "CPPOP", "1_1_0") );
   map_code.insert( pair<string,string>(   "PPOSP", "1_1_1") );
   map_code.insert( pair<string,string>(     "POP", "1_1_2") );
   map_code.insert( pair<string,string>(   "CPOZP", "1_1_192") );
   map_code.insert( pair<string,string>(   "CPOFP", "1_1_193") );
   map_code.insert( pair<string,string>(   "PPFFG", "1_1_194") );
   map_code.insert( pair<string,string>(     "CWR", "1_1_195") );
   map_code.insert( pair<string,string>(    "LAND", "2_0_0") );
   map_code.insert( pair<string,string>(    "SFCR", "2_0_1") );
   map_code.insert( pair<string,string>(   "TSOIL", "2_0_2") );
   map_code.insert( pair<string,string>(   "SOILM", "2_0_3") );
   map_code.insert( pair<string,string>(     "VEG", "2_0_4") );
   map_code.insert( pair<string,string>(    "WATR", "2_0_5") );
   map_code.insert( pair<string,string>(   "EVAPT", "2_0_6") );
   map_code.insert( pair<string,string>(   "MTERH", "2_0_7") );
   map_code.insert( pair<string,string>(   "LANDU", "2_0_8") );
   map_code.insert( pair<string,string>(   "SOILW", "2_0_9") );
   map_code.insert( pair<string,string>(   "GFLUX", "2_0_10") );
   map_code.insert( pair<string,string>(   "MSTAV", "2_0_11") );
   map_code.insert( pair<string,string>(   "SFEXC", "2_0_12") );
   map_code.insert( pair<string,string>(   "CNWAT", "2_0_13") );
   map_code.insert( pair<string,string>(   "BMIXL", "2_0_14") );
   map_code.insert( pair<string,string>(   "CCOND", "2_0_15") );
   map_code.insert( pair<string,string>(   "RSMIN", "2_0_16") );
   map_code.insert( pair<string,string>(    "WILT", "2_0_17") );
   map_code.insert( pair<string,string>(     "RCS", "2_0_18") );
   map_code.insert( pair<string,string>(     "RCT", "2_0_19") );
   map_code.insert( pair<string,string>(   "RCSOL", "2_0_20") );
   map_code.insert( pair<string,string>(     "RCQ", "2_0_21") );
   map_code.insert( pair<string,string>(   "SOILM", "2_0_22") );
   map_code.insert( pair<string,string>( "CISOILW", "2_0_23") );
   map_code.insert( pair<string,string>(   "HFLUX", "2_0_24") );
   map_code.insert( pair<string,string>(  "VSOILM", "2_0_25") );
   map_code.insert( pair<string,string>(    "WILT", "2_0_26") );
   map_code.insert( pair<string,string>(  "VWILTM", "2_0_27") );
   map_code.insert( pair<string,string>(   "SOILW", "2_0_192") );
   map_code.insert( pair<string,string>(   "GFLUX", "2_0_193") );
   map_code.insert( pair<string,string>(   "MSTAV", "2_0_194") );
   map_code.insert( pair<string,string>(   "SFEXC", "2_0_195") );
   map_code.insert( pair<string,string>(   "CNWAT", "2_0_196") );
   map_code.insert( pair<string,string>(   "BMIXL", "2_0_197") );
   map_code.insert( pair<string,string>(   "VGTYP", "2_0_198") );
   map_code.insert( pair<string,string>(   "CCOND", "2_0_199") );
   map_code.insert( pair<string,string>(   "RSMIN", "2_0_200") );
   map_code.insert( pair<string,string>(    "WILT", "2_0_201") );
   map_code.insert( pair<string,string>(     "RCS", "2_0_202") );
   map_code.insert( pair<string,string>(     "RCT", "2_0_203") );
   map_code.insert( pair<string,string>(     "RCQ", "2_0_204") );
   map_code.insert( pair<string,string>(   "RCSOL", "2_0_205") );
   map_code.insert( pair<string,string>(   "RDRIP", "2_0_206") );
   map_code.insert( pair<string,string>(   "ICWAT", "2_0_207") );
   map_code.insert( pair<string,string>(    "AKHS", "2_0_208") );
   map_code.insert( pair<string,string>(    "AKMS", "2_0_209") );
   map_code.insert( pair<string,string>(    "VEGT", "2_0_210") );
   map_code.insert( pair<string,string>(   "SSTOR", "2_0_211") );
   map_code.insert( pair<string,string>(   "LSOIL", "2_0_212") );
   map_code.insert( pair<string,string>(   "EWATR", "2_0_213") );
   map_code.insert( pair<string,string>(   "GWREC", "2_0_214") );
   map_code.insert( pair<string,string>(    "QREC", "2_0_215") );
   map_code.insert( pair<string,string>(   "SFCRH", "2_0_216") );
   map_code.insert( pair<string,string>(    "NDVI", "2_0_217") );
   map_code.insert( pair<string,string>(   "LANDN", "2_0_218") );
   map_code.insert( pair<string,string>(   "AMIXL", "2_0_219") );
   map_code.insert( pair<string,string>(   "WVINC", "2_0_220") );
   map_code.insert( pair<string,string>(   "WCINC", "2_0_221") );
   map_code.insert( pair<string,string>(  "WVCONV", "2_0_222") );
   map_code.insert( pair<string,string>(  "WCCONV", "2_0_223") );
   map_code.insert( pair<string,string>(  "WVUFLX", "2_0_224") );
   map_code.insert( pair<string,string>(  "WVVFLX", "2_0_225") );
   map_code.insert( pair<string,string>(  "WCUFLX", "2_0_226") );
   map_code.insert( pair<string,string>(  "WCVFLX", "2_0_227") );
   map_code.insert( pair<string,string>(   "ACOND", "2_0_228") );
   map_code.insert( pair<string,string>(    "EVCW", "2_0_229") );
   map_code.insert( pair<string,string>(   "TRANS", "2_0_230") );
   map_code.insert( pair<string,string>(   "SOTYP", "2_3_0") );
   map_code.insert( pair<string,string>(   "UPLST", "2_3_1") );
   map_code.insert( pair<string,string>(   "UPLSM", "2_3_2") );
   map_code.insert( pair<string,string>(  "LOWLSM", "2_3_3") );
   map_code.insert( pair<string,string>(  "BOTLST", "2_3_4") );
   map_code.insert( pair<string,string>(   "SOILL", "2_3_5") );
   map_code.insert( pair<string,string>(   "RLYRS", "2_3_6") );
   map_code.insert( pair<string,string>(   "SMREF", "2_3_7") );
   map_code.insert( pair<string,string>(   "SMDRY", "2_3_8") );
   map_code.insert( pair<string,string>(   "POROS", "2_3_9") );
   map_code.insert( pair<string,string>(  "LIQVSM", "2_3_10") );
   map_code.insert( pair<string,string>(  "VOLTSO", "2_3_11") );
   map_code.insert( pair<string,string>(  "TRANSO", "2_3_12") );
   map_code.insert( pair<string,string>(  "VOLDEC", "2_3_13") );
   map_code.insert( pair<string,string>(   "DIREC", "2_3_14") );
   map_code.insert( pair<string,string>(   "SOILP", "2_3_15") );
   map_code.insert( pair<string,string>(   "VSOSM", "2_3_16") );
   map_code.insert( pair<string,string>(  "SATOSM", "2_3_17") );
   map_code.insert( pair<string,string>(   "SOILL", "2_3_192") );
   map_code.insert( pair<string,string>(   "RLYRS", "2_3_193") );
   map_code.insert( pair<string,string>(   "SLTYP", "2_3_194") );
   map_code.insert( pair<string,string>(   "SMREF", "2_3_195") );
   map_code.insert( pair<string,string>(   "SMDRY", "2_3_196") );
   map_code.insert( pair<string,string>(   "POROS", "2_3_197") );
   map_code.insert( pair<string,string>(    "EVBS", "2_3_198") );
   map_code.insert( pair<string,string>(    "LSPA", "2_3_199") );
   map_code.insert( pair<string,string>(   "BARET", "2_3_200") );
   map_code.insert( pair<string,string>(   "AVSFT", "2_3_201") );
   map_code.insert( pair<string,string>(    "RADT", "2_3_202") );
   map_code.insert( pair<string,string>(   "FLDCP", "2_3_203") );
   map_code.insert( pair<string,string>( "FIRECRA", "2_4_0") );
   map_code.insert( pair<string,string>("FIREXCRA", "2_4_1") );
   map_code.insert( pair<string,string>( "FIREDLA", "2_4_2") );
   map_code.insert( pair<string,string>(  "HINDEX", "2_4_3") );
   map_code.insert( pair<string,string>(    "SRAD", "3_0_0") );
   map_code.insert( pair<string,string>( "SALBEDO", "3_0_1") );
   map_code.insert( pair<string,string>(   "SBTMP", "3_0_2") );
   map_code.insert( pair<string,string>(   "SPWAT", "3_0_3") );
   map_code.insert( pair<string,string>(   "SLFTI", "3_0_4") );
   map_code.insert( pair<string,string>( "SCTPRES", "3_0_5") );
   map_code.insert( pair<string,string>(   "SSTMP", "3_0_6") );
   map_code.insert( pair<string,string>(  "CLOUDM", "3_0_7") );
   map_code.insert( pair<string,string>(   "PIXST", "3_0_8") );
   map_code.insert( pair<string,string>(  "FIREDI", "3_0_9") );
   map_code.insert( pair<string,string>(    "ESTP", "3_1_0") );
   map_code.insert( pair<string,string>(  "IRRATE", "3_1_1") );
   map_code.insert( pair<string,string>(   "CTOPH", "3_1_2") );
   map_code.insert( pair<string,string>( "CTOPHQI", "3_1_3") );
   map_code.insert( pair<string,string>( "ESTUGRD", "3_1_4") );
   map_code.insert( pair<string,string>( "ESTVGRD", "3_1_5") );
   map_code.insert( pair<string,string>(   "NPIXU", "3_1_6") );
   map_code.insert( pair<string,string>(   "SOLZA", "3_1_7") );
   map_code.insert( pair<string,string>(    "RAZA", "3_1_8") );
   map_code.insert( pair<string,string>(   "RFL06", "3_1_9") );
   map_code.insert( pair<string,string>(   "RFL08", "3_1_10") );
   map_code.insert( pair<string,string>(   "RFL16", "3_1_11") );
   map_code.insert( pair<string,string>(   "RFL39", "3_1_12") );
   map_code.insert( pair<string,string>(  "ATMDIV", "3_1_13") );
   map_code.insert( pair<string,string>(   "WINDS", "3_1_19") );
   map_code.insert( pair<string,string>(   "AOT06", "3_1_20") );
   map_code.insert( pair<string,string>(   "AOT08", "3_1_21") );
   map_code.insert( pair<string,string>(   "AOT16", "3_1_22") );
   map_code.insert( pair<string,string>(  "ANGCOE", "3_1_23") );
   map_code.insert( pair<string,string>(    "USCT", "3_1_192") );
   map_code.insert( pair<string,string>(    "VSCT", "3_1_193") );
   map_code.insert( pair<string,string>(  "SBT122", "3_192_0") );
   map_code.insert( pair<string,string>(  "SBT123", "3_192_1") );
   map_code.insert( pair<string,string>(  "SBT124", "3_192_2") );
   map_code.insert( pair<string,string>(  "SBT126", "3_192_3") );
   map_code.insert( pair<string,string>(  "SBC123", "3_192_4") );
   map_code.insert( pair<string,string>(  "SBC124", "3_192_5") );
   map_code.insert( pair<string,string>(  "SBT112", "3_192_6") );
   map_code.insert( pair<string,string>(  "SBT113", "3_192_7") );
   map_code.insert( pair<string,string>(  "SBT114", "3_192_8") );
   map_code.insert( pair<string,string>(  "SBT115", "3_192_9") );
   map_code.insert( pair<string,string>(   "WVSP1", "10_0_0") );
   map_code.insert( pair<string,string>(   "WVSP2", "10_0_1") );
   map_code.insert( pair<string,string>(   "WVSP3", "10_0_2") );
   map_code.insert( pair<string,string>(   "HTSGW", "10_0_3") );
   map_code.insert( pair<string,string>(   "WVDIR", "10_0_4") );
   map_code.insert( pair<string,string>(   "WVHGT", "10_0_5") );
   map_code.insert( pair<string,string>(   "WVPER", "10_0_6") );
   map_code.insert( pair<string,string>(   "SWDIR", "10_0_7") );
   map_code.insert( pair<string,string>(   "SWELL", "10_0_8") );
   map_code.insert( pair<string,string>(   "SWPER", "10_0_9") );
   map_code.insert( pair<string,string>(   "DIRPW", "10_0_10") );
   map_code.insert( pair<string,string>(   "PERPW", "10_0_11") );
   map_code.insert( pair<string,string>(   "DIRSW", "10_0_12") );
   map_code.insert( pair<string,string>(   "PERSW", "10_0_13") );
   map_code.insert( pair<string,string>(  "WWSDIR", "10_0_14") );
   map_code.insert( pair<string,string>(  "MWSPER", "10_0_15") );
   map_code.insert( pair<string,string>(    "WSTP", "10_0_192") );
   map_code.insert( pair<string,string>(    "DIRC", "10_1_0") );
   map_code.insert( pair<string,string>(     "SPC", "10_1_1") );
   map_code.insert( pair<string,string>(   "UOGRD", "10_1_2") );
   map_code.insert( pair<string,string>(   "VOGRD", "10_1_3") );
   map_code.insert( pair<string,string>(    "OMLU", "10_1_192") );
   map_code.insert( pair<string,string>(    "OMLV", "10_1_193") );
   map_code.insert( pair<string,string>(   "UBARO", "10_1_194") );
   map_code.insert( pair<string,string>(   "VBARO", "10_1_195") );
   map_code.insert( pair<string,string>(    "ICEC", "10_2_0") );
   map_code.insert( pair<string,string>(   "ICETK", "10_2_1") );
   map_code.insert( pair<string,string>(   "DICED", "10_2_2") );
   map_code.insert( pair<string,string>(   "SICED", "10_2_3") );
   map_code.insert( pair<string,string>(    "UICE", "10_2_4") );
   map_code.insert( pair<string,string>(    "VICE", "10_2_5") );
   map_code.insert( pair<string,string>(    "ICEG", "10_2_6") );
   map_code.insert( pair<string,string>(    "ICED", "10_2_7") );
   map_code.insert( pair<string,string>(    "ICET", "10_2_8") );
   map_code.insert( pair<string,string>(    "WTMP", "10_3_0") );
   map_code.insert( pair<string,string>(    "DSLM", "10_3_1") );
   map_code.insert( pair<string,string>(   "SURGE", "10_3_192") );
   map_code.insert( pair<string,string>(   "ETSRG", "10_3_193") );
   map_code.insert( pair<string,string>("ELEVhtml", "10_3_194") );
   map_code.insert( pair<string,string>(    "SSHG", "10_3_195") );
   map_code.insert( pair<string,string>(  "P2OMLT", "10_3_196") );
   map_code.insert( pair<string,string>(  "AOHFLX", "10_3_197") );
   map_code.insert( pair<string,string>(   "ASHFL", "10_3_198") );
   map_code.insert( pair<string,string>(    "SSTT", "10_3_199") );
   map_code.insert( pair<string,string>(    "SSST", "10_3_200") );
   map_code.insert( pair<string,string>(    "KENG", "10_3_201") );
   map_code.insert( pair<string,string>(   "SLTFL", "10_3_202") );
   map_code.insert( pair<string,string>( "TCSRG20", "10_3_242") );
   map_code.insert( pair<string,string>( "TCSRG30", "10_3_243") );
   map_code.insert( pair<string,string>( "TCSRG40", "10_3_244") );
   map_code.insert( pair<string,string>( "TCSRG50", "10_3_245") );
   map_code.insert( pair<string,string>( "TCSRG60", "10_3_246") );
   map_code.insert( pair<string,string>( "TCSRG70", "10_3_247") );
   map_code.insert( pair<string,string>( "TCSRG80", "10_3_248") );
   map_code.insert( pair<string,string>( "TCSRG90", "10_3_249") );
   map_code.insert( pair<string,string>(    "MTHD", "10_4_0") );
   map_code.insert( pair<string,string>(    "MTHA", "10_4_1") );
   map_code.insert( pair<string,string>(   "TTHDP", "10_4_2") );
   map_code.insert( pair<string,string>(   "SALTY", "10_4_3") );
   map_code.insert( pair<string,string>(    "OVHD", "10_4_4") );
   map_code.insert( pair<string,string>(    "OVSD", "10_4_5") );
   map_code.insert( pair<string,string>(    "OVMD", "10_4_6") );
   map_code.insert( pair<string,string>(   "WTMPC", "10_4_192") );
   map_code.insert( pair<string,string>(   "SALIN", "10_4_193") );
   map_code.insert( pair<string,string>(   "BKENG", "10_4_194") );
   map_code.insert( pair<string,string>(    "DBSS", "10_4_195") );
   map_code.insert( pair<string,string>(   "INTFD", "10_4_196") );
   map_code.insert( pair<string,string>(     "OHC", "10_4_197") );
   map_code.insert( pair<string,string>(    "TSEC", "10_191_0") );
   map_code.insert( pair<string,string>(    "MOSF", "10_191_1") );
   map_code.insert( pair<string,string>(    "IMGD", "255_255_255") );

}
