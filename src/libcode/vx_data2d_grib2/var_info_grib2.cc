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

#include "var_info.h"
#include "var_info_grib2.h"

#include "math_constants.h"
#include "vx_log.h"
#include "vx_util.h"

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

   init_var_maps();

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

void VarInfoGrib2::set_pair(const ConcatString &key, const ConcatString &val) {

   // First call the parent's set_pair function.
   VarInfo::set_pair(key, val);

   // Look for GRIB2 keywords.
   if(strcasecmp(key, CONFIG_GRIB2_Discipline) == 0) { Discipline = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_MTable    ) == 0) { MTable     = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_LTable    ) == 0) { LTable     = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_Tmpl      ) == 0) { Tmpl       = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_ParmCat   ) == 0) { ParmCat    = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_Parm      ) == 0) { Parm       = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_Process   ) == 0) { Process    = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_EnsType   ) == 0) { EnsType    = atoi(val); }
   if(strcasecmp(key, CONFIG_GRIB2_DerType   ) == 0) { DerType    = atoi(val); }

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
      mlog << Error << "\nVarInfoGrib2::set_magic - failed to parse magic string '"
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
   if( 3 == regex_apply("^([0-9]+)_([0-9]+)$", 3, parm_name.data(), mat) ){

      //  make sure the parameter table codes are valid
      if( !map_id.count(parm_name) ){
         mlog << Error << "\nVarInfoGrib2::set_magic - unrecognized GRIB2 parameter table indexes '"
              << parm_name.data() << "'\n\n";
         exit(1);
      }

      //  assign the parameter category and code
      ParmCat  = atoi(mat[1]);
      Parm     = atoi(mat[2]);
      regex_clean(mat);

      //  parse the variable name from the table information
      if( 2 != regex_apply("^([^\\|]+)\\|.*", 2, map_id[parm_name].data(), mat) ){
         mlog << Error << "\nVarInfoGrib2::set_magic - failed to parse GRIB2 table map_id information '"
              << map_id[parm_name].data() << "'\n\n";
         exit(1);
      }
      Name = mat[0];
      regex_clean(mat);

   }

   //  otherwise, attempt to find the parameter name in the map_code table
   else {

      //  validate the table and field number
      if( !map_code.count(parm_name) ){
         mlog << Error << "\nVarInfoGrib2::set_magic - unrecognized GRIB2 field abbreviation '"
              << parm_name.data() << "'\n\n";
         exit(1);
      }
      Name = parm_name.data();

      //  parse the table and field number
      if( 3 != regex_apply("^([0-9]+)_([0-9]+)$", 3, map_code[parm_name].data(), mat) ){
         mlog << Error << "\nVarInfoGrib2::set_magic - failed to parse GRIB2 table code for string '"
              << Name << "'\n\n";
         exit(1);
      }
      ParmCat  = atoi(mat[1]);
      Parm     = atoi(mat[2]);
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

   //  set the name
   ConcatString lvl_name = "";
   lvl_name << lvl_type.data() << lvl1_str.data();
   if( -1 != lvl2 ) lvl_name << "-" << lvl2_str.data();
   Level.set_req_name(lvl_name);
   Level.set_name(lvl_name);

   //  arrange the pressure level values appropriately
   lvl2 = ( lvl2 != lvl1 ? lvl2 : -1 );
   if( -1 != lvl2 && lvl2 < lvl1 ){
      int lvl_tmp = lvl2;
      lvl2 = lvl1;
      lvl1 = lvl_tmp;
   }

   //  set the lower limit
   if(lt == LevelType_Accum) Level.set_lower(timestring_to_sec( lvl1_str.data() ));
   else                      Level.set_lower(lvl1);

   //  if an upper limit is specified, verify and set it
   if( -1 != lvl2 ){

      //  if pressure ranges are not supported for the specified level type, bail
      if( lt != LevelType_Pres && lt != LevelType_Vert && lt != LevelType_None ){
         mlog << Error << "\nVarInfoGrib2::set_magic() - "
              << "ranges of levels are only supported for pressure levels "
              << "(P), vertical levels (Z), and generic levels (L)\n\n";
         exit(1);
      }

   }

   Level.set_upper( -1 == lvl2 ? lvl1 : lvl2 );

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_precipitation() const {

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_specific_humidity() const {

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_u_wind() const {

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_v_wind() const {

   // This functionality is not supported for GRIB2.
   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_wind_speed() const {

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_wind_direction() const {

   return(false);
}

////////////////////////////////////////////////////////////////////////

LevelType VarInfoGrib2::g2_lty_to_level_type(int lt) {

   //  from: http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table4-5.shtml
   switch( lt ){
      case 100:
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

void VarInfoGrib2::init_var_maps(){

   //PGO these will be parsed and read in from a flat file

   map_id[   "0_0"] = "TMP         | K                   | Temperature";
   map_id[   "0_1"] = "VTMP        | K                   | Virtual Temperature";
   map_id[   "0_2"] = "POT         | K                   | Potential Temperature";
   map_id[   "0_3"] = "EPOT        | K                   | Pseudo-Adiabatic Potential Temperature (or Equivalent Potential Temperature)";
   map_id[   "0_4"] = "TMAX        | K                   | Maximum Temperature";
   map_id[   "0_5"] = "TMIN        | K                   | Minimum Temperature";
   map_id[   "0_6"] = "DPT         | K                   | Dew Point Temperature";
   map_id[   "0_7"] = "DEPR        | K                   | Dew Point Depression (or Deficit)";
   map_id[   "0_8"] = "LAPR        | K/m                 | Lapse Rate";
   map_id[   "0_9"] = "TMPA        | K                   | Temperature Anomaly";
   map_id[  "0_10"] = "LHTFL       | W/m^2               | Latent Heat Net Flux";
   map_id[  "0_11"] = "SHTFL       | W/m^2               | Sensible Heat Net Flux";
   map_id[  "0_12"] = "HEATX       | K                   | Heat Index";
   map_id[  "0_13"] = "WCF         | K                   | Wind Chill Factor";
   map_id[  "0_14"] = "MINDPD      | K                   | Minimum Dew Point depression";
   map_id[  "0_15"] = "VPTMP       | K                   | Virtual Potential Temperature";
   map_id[  "0_16"] = "SNOHF       | W/m^2               | Snow phase change heat flux";
   map_id[  "0_17"] = "SKINT       | K                   | Skin Temperature";
   map_id[ "0_192"] = "SNOHF       | W/m^2               | Snow Phase Change Heat Flux";
   map_id[ "0_193"] = "TTRAD       | K/s                 | Temperature tendency by all radiation";
   map_id[ "0_194"] = "REV         | -                   | Relative Error Variance";
   map_id[ "0_195"] = "LRGHR       | K/s                 | Large Scale Condensate Heating rate";
   map_id[ "0_196"] = "CNVHR       | K/s                 | Deep Convective Heating rate";
   map_id[ "0_197"] = "THFLX       | W/m^2               | Total Downward Heat Flux at Surface";
   map_id[ "0_198"] = "TTDIA       | K/s                 | Temperature Tendency By All Physics";
   map_id[ "0_199"] = "TTPHY       | K/s                 | Temperature Tendency By Non-radiation Physics";
   map_id[ "0_200"] = "TSD1D       | K                   | Standard Dev. of IR Temp. over 1x1 deg. area";
   map_id[ "0_201"] = "SHAHR       | K/s                 | Shallow Convective Heating rate";
   map_id[ "0_202"] = "VDFHR       | K/s                 | Vertical Diffusion Heating rate";
   map_id[ "0_203"] = "THZ0        | K                   | Potential temperature at top of viscous sublayer";
   map_id[ "0_204"] = "TCHP        | J/m2K               | Tropical Cyclone Heat Potential";
   map_id[   "1_0"] = "SPFH        | kg/kg               | Specific Humidity";
   map_id[   "1_1"] = "RH          | %                   | Relative Humidity";
   map_id[   "1_2"] = "MIXR        | kg/kg               | Humidity Mixing Ratio";
   map_id[   "1_3"] = "PWAT        | kg/m^2              | Precipitable Water";
   map_id[   "1_4"] = "VAPP        | Pa                  | Vapor Pressure";
   map_id[   "1_5"] = "SATD        | Pa                  | Saturation Deficit";
   map_id[   "1_6"] = "EVP         | kg/m^2              | Evaporation";
   map_id[   "1_7"] = "PRATE       | kg/m^2/s            | Precipitation Rate";
   map_id[   "1_8"] = "APCP        | kg/m^2              | Total Precipitation";
   map_id[   "1_9"] = "NCPCP       | kg/m^2              | Large-Scale Precipitation (non-convective)";
   map_id[  "1_10"] = "ACPCP       | kg/m^2              | Convective Precipitation";
   map_id[  "1_11"] = "SNOD        | m                   | Snow Depth";
   map_id[  "1_12"] = "SRWEQ       | kg/m^2/s            | Snowfall Rate Water Equivalent";
   map_id[  "1_13"] = "WEASD       | kg/m^2              | Water Equivalent of Accumulated Snow Depth";
   map_id[  "1_14"] = "SNOC        | kg/m^2              | Convect Snow";
   map_id[  "1_15"] = "SNOL        | kg/m^2              | Large-Scale Snow";
   map_id[  "1_16"] = "SNOM        | kg/m^2              | Snow Melt";
   map_id[  "1_17"] = "SNOAG       | day                 | Snow Age";
   map_id[  "1_18"] = "ABSH        | kg/m^3              | Absolute Humidity";
   map_id[  "1_19"] = "PTYPE       | -                   | Precipitation Type";
   map_id[  "1_20"] = "ILIQW       | kg/m^2              | Integrated Liquid Water";
   map_id[  "1_21"] = "TCOND       | kg/kg               | Condensate";
   map_id[  "1_22"] = "CLWMR       | kg/kg               | Cloud Mixing Ratio";
   map_id[  "1_23"] = "ICMR        | kg/kg               | Ice Water Mixing Ratio";
   map_id[  "1_24"] = "RWMR        | kg/kg               | Rain Mixing Ratio";
   map_id[  "1_25"] = "SNMR        | kg/kg               | Snow Mixing Ratio";
   map_id[  "1_26"] = "MCONV       | kg/kg/s             | Horizontal Moisture Convergence";
   map_id[  "1_27"] = "MAXRH       | %                   | Maximum Relative Humidity";
   map_id[  "1_28"] = "MAXAH       | kg/m^3              | Maximum Absolute Humidity";
   map_id[  "1_29"] = "ASNOW       | m                   | Total Snowfall";
   map_id[  "1_30"] = "PWCAT       | -                   | Precipitable Water Category";
   map_id[  "1_31"] = "HAIL        | m                   | Hail";
   map_id[  "1_32"] = "GRLE        | kg/kg               | Grauple";
   map_id[  "1_33"] = "CRAIN       | -                   | Categorical Rain";
   map_id[  "1_34"] = "CFRZR       | -                   | Categorical Freezing Rain";
   map_id[  "1_35"] = "CICEP       | -                   | Categorical Ice Pellets";
   map_id[  "1_36"] = "CSNOW       | -                   | Categorical Snow";
   map_id[  "1_37"] = "CPRAT       | kg/m^2/s            | Convective Precipitation Rate";
   map_id[  "1_38"] = "MCONV       | kg/kg/s             | Horizontal Moisture Divergence";
   map_id[  "1_39"] = "CPOFP       | %                   | Percent frozen precipitation";
   map_id[  "1_40"] = "PEVAP       | kg/m^2              | Potential Evaporation";
   map_id[  "1_41"] = "PEVPR       | W/m^2               | Potential Evaporation Rate";
   map_id[  "1_42"] = "SNOWC       | %                   | Snow Cover";
   map_id[  "1_43"] = "FRAIN       | Proportion          | Rain Fraction of Total Liquid Water";
   map_id[  "1_44"] = "RIME        | Numeric             | Rime Factor";
   map_id[  "1_45"] = "TCOLR       | kg/m^2              | Total Column Integrated Rain";
   map_id[  "1_46"] = "TCOLS       | kg/m^2              | Total Column Integrated Snow";
   map_id[  "1_47"] = "LSWP        | kg/m^2              | Large Scale Water Precipitation (Non-Convective)";
   map_id[  "1_48"] = "CWP         | kg/m^2              | Convective Water Precipitation";
   map_id[  "1_49"] = "TWATP       | kg/m^2              | Total Water Precipitation";
   map_id[  "1_50"] = "TSNOWP      | kg/m^2              | Total Snow Precipitation";
   map_id[  "1_51"] = "TCWAT       | kg/m^2              | Total Column Water (Vertically integrated total water (vapour+cloud water/ice)";
   map_id[  "1_52"] = "TPRATE      | kg/m^2/s            | Total Precipitation Rate";
   map_id[  "1_53"] = "TSRWE       | kg/m^2/s            | Total Snowfall Rate Water Equivalent";
   map_id[  "1_54"] = "LSPRATE     | kg/m^2/s            | Large Scale Precipitation Rate";
   map_id[  "1_55"] = "CSRWE       | kg/m^2/s            | Convective Snowfall Rate Water Equivalent";
   map_id[  "1_56"] = "LSSRWE      | kg/m^2/s            | Large Scale Snowfall Rate Water Equivalent";
   map_id[  "1_57"] = "TSRATE      | m/s                 | Total Snowfall Rate";
   map_id[  "1_58"] = "CSRATE      | m/s                 | Convective Snowfall Rate";
   map_id[  "1_59"] = "LSSRATE     | m/s                 | Large Scale Snowfall Rate";
   map_id[  "1_60"] = "SDWE        | kg/m^2              | Snow Depth Water Equivalent";
   map_id[  "1_61"] = "SDEN        | kg/m^3              | Snow Density";
   map_id[  "1_62"] = "SEVAP       | kg/m^2              | Snow Evaporation";
   map_id[  "1_64"] = "TCIWV       | kg/m^2              | Total Column Integrated Water Vapour";
   map_id[  "1_65"] = "RPRATE      | kg/m^2/s            | Rain Precipitation Rate";
   map_id[  "1_66"] = "SPRATE      | kg/m^2/s            | Snow Precipitation Rate";
   map_id[  "1_67"] = "FPRATE      | kg/m^2/s            | Freezing Rain Precipitation Rate";
   map_id[  "1_68"] = "IPRATE      | kg/m^2/s            | Ice Pellets Precipitation Rate";
   map_id[ "1_192"] = "CRAIN       | non-dim             | Categorical Rain (yes=1; no=0)";
   map_id[ "1_193"] = "CFRZR       | non-dim             | Categorical Freezing Rain (yes=1; no=0)";
   map_id[ "1_194"] = "CICEP       | non-dim             | Categorical Ice Pellets (yes=1; no=0)";
   map_id[ "1_195"] = "CSNOW       | non-dim             | Categorical Snow (yes=1; no=0)";
   map_id[ "1_196"] = "CPRAT       | kg/m^2/s            | Convective Precipitation Rate";
   map_id[ "1_197"] = "MCONV       | kg/kg/s             | Horizontal Moisture Divergence";
   map_id[ "1_198"] = "MINRH       | %                   | Minimum Relative Humidity";
   map_id[ "1_199"] = "PEVAP       | kg/m^2              | Potential Evaporation";
   map_id[ "1_200"] = "PEVPR       | W/m^2               | Potential Evaporation Rate";
   map_id[ "1_201"] = "SNOWC       | %                   | Snow Cover";
   map_id[ "1_202"] = "FRAIN       | non-dim             | Rain Fraction of Total Liquid Water";
   map_id[ "1_203"] = "RIME        | non-dim             | Rime Factor";
   map_id[ "1_204"] = "TCOLR       | kg/m^2              | Total Column Integrated Rain";
   map_id[ "1_205"] = "TCOLS       | kg/m^2              | Total Column Integrated Snow";
   map_id[ "1_206"] = "TIPD        | non-dim             | Total Icing Potential Diagnostic";
   map_id[ "1_207"] = "NCIP        | non-dim             | Number concentration for ice particles";
   map_id[ "1_208"] = "SNOT        | K                   | Snow temperature";
   map_id[ "1_209"] = "TCLSW       | kg/m^2              | Total column-integrated supercooled liquid water";
   map_id[ "1_210"] = "TCOLM       | kg/m^2              | Total column-integrated melting ice";
   map_id[ "1_211"] = "EMNP        | cm/day              | Evaporation - Precipitation";
   map_id[ "1_212"] = "SBSNO       | W/m^2               | Sublimation (evaporation from snow)";
   map_id[ "1_213"] = "CNVMR       | kg/kg/s             | Deep Convective Moistening Rate";
   map_id[ "1_214"] = "SHAMR       | kg/kg/s             | Shallow Convective Moistening Rate";
   map_id[ "1_215"] = "VDFMR       | kg/kg/s             | Vertical Diffusion Moistening Rate";
   map_id[ "1_216"] = "CONDP       | Pa                  | Condensation Pressure of Parcali Lifted From Indicate Surface";
   map_id[ "1_217"] = "LRGMR       | kg/kg/s             | Large scale moistening rate";
   map_id[ "1_218"] = "QZ0         | kg/kg               | Specific humidity at top of viscous sublayer";
   map_id[ "1_219"] = "QMAX        | kg/kg               | Maximum specific humidity at 2m";
   map_id[ "1_220"] = "QMIN        | kg/kg               | Minimum specific humidity at 2m";
   map_id[ "1_221"] = "ARAIN       | kg/m2               | Liquid precipitation (rainfall)";
   map_id[ "1_222"] = "SNOWT       | K                   | Snow temperature, depth-avg";
   map_id[ "1_223"] = "APCPN       | kg/m2               | Total precipitation (nearest grid point)";
   map_id[ "1_224"] = "ACPCPN      | kg/m2               | Convective precipitation (nearest grid point)";
   map_id[ "1_225"] = "FRZR        | kg/m2               | Freezing Rain";
   map_id[ "1_226"] = "PWTHER      | Numeric             | Predominant Weather";
   map_id[ "1_227"] = "FROZR       | kg/m2               | Frozen Rain";
   map_id[ "1_241"] = "TSNOW       | kg/m2               | Total Snow";
   map_id[   "2_0"] = "WDIR        | deg                 | Wind Direction (from which blowing)";
   map_id[   "2_1"] = "WIND        | m/s                 | Wind Speed";
   map_id[   "2_2"] = "UGRD        | m/s                 | U-Component of Wind";
   map_id[   "2_3"] = "VGRD        | m/s                 | V-Component of Wind";
   map_id[   "2_4"] = "STRM        | m^2/s               | Stream Function";
   map_id[   "2_5"] = "VPOT        | m^2/s               | Velocity Potential";
   map_id[   "2_6"] = "MNTSF       | m^2/s               | Montgomery Stream Function";
   map_id[   "2_7"] = "SGCVV       | 1/s                 | Sigma Coordinate Vertical Velocity";
   map_id[   "2_8"] = "VVEL        | Pa/s                | Vertical Velocity (Pressure)";
   map_id[   "2_9"] = "DZDT        | m/s                 | Vertical Velocity (Geometric)";
   map_id[  "2_10"] = "ABSV        | 1/s                 | Absolute Vorticity";
   map_id[  "2_11"] = "ABSD        | 1/s                 | Absolute Divergence";
   map_id[  "2_12"] = "RELV        | 1/s                 | Relative Vorticity";
   map_id[  "2_13"] = "RELD        | 1/s                 | Relative Divergence";
   map_id[  "2_14"] = "PVORT       | Km2kg-1s-1          | Potential Vorticity";
   map_id[  "2_15"] = "VUCSH       | 1/s                 | Vertical U-Component of Shear";
   map_id[  "2_16"] = "VVCSH       | 1/s                 | Vertical V-Component of Shear";
   map_id[  "2_17"] = "UFLX        | N/m^2               | Momentum Flux, U-Component";
   map_id[  "2_18"] = "VFLX        | N/m^2               | Momentum Flux, V-Component";
   map_id[  "2_19"] = "WMIXE       | J                   | Wind Mixing Energy";
   map_id[  "2_20"] = "BLYDP       | W/m^2               | Boundary Layer Dissipation";
   map_id[  "2_21"] = "MAXGUST     | m/s                 | Maximum Wind Speed";
   map_id[  "2_22"] = "GUST        | m/s                 | Wind Speed (Gust)";
   map_id[  "2_23"] = "UGUST       | m/s                 | U-Component of Wind (Gust)";
   map_id[  "2_24"] = "VGUST       | m/s                 | V-Component of Wind (Gust)";
   map_id[  "2_25"] = "VWSH        | 1/s                 | Vertical speed sheer";
   map_id[  "2_26"] = "MFLX        | N/m^2               | Horizontal Momentum Flux";
   map_id[  "2_27"] = "USTM        | m/s                 | U-Component Storm Motion";
   map_id[  "2_28"] = "VSTM        | m/s                 | V-Component Storm Motion";
   map_id[  "2_29"] = "CD          | Numeric             | Drag Coefficient";
   map_id[  "2_30"] = "FRICV       | m/s                 | Frictional Velocity";
   map_id[  "2_32"] = "ETACVV      | 1/s                 | Eta Coordinate Vertical Velocity";
   map_id[ "2_192"] = "VWSH        | 1/s                 | Vertical speed sheer";
   map_id[ "2_193"] = "MFLX        | N/m^2               | Horizontal Momentum Flux";
   map_id[ "2_194"] = "USTM        | m/s                 | U-Component Storm Motion";
   map_id[ "2_195"] = "VSTM        | m/s                 | V-Component Storm Motion";
   map_id[ "2_196"] = "CD          | non-dim             | Drag Coefficient";
   map_id[ "2_197"] = "FRICV       | m/s                 | Frictional Velocity";
   map_id[ "2_198"] = "LAUV        | deg                 | Latitude of U Wind Component of Velocity";
   map_id[ "2_199"] = "LOUV        | deg                 | Longitude of U Wind Component of Velocity";
   map_id[ "2_200"] = "LAVV        | deg                 | Latitude of V Wind Component of Velocity";
   map_id[ "2_201"] = "LOVV        | deg                 | Longitude of V Wind Component of Velocity";
   map_id[ "2_202"] = "LAPP        | deg                 | Latitude of Presure Point";
   map_id[ "2_203"] = "LOPP        | deg                 | Longitude of Presure Point";
   map_id[ "2_204"] = "VEDH        | m^2/s               | Vertical Eddy Diffusivity Heat exchange";
   map_id[ "2_205"] = "COVMZ       | m^2/s^2             | Covariance between Meridional and Zonal Components of the wind.";
   map_id[ "2_206"] = "COVTZ       | K*ms-1              | Covariance between Temperature and Zonal Components of the wind.";
   map_id[ "2_207"] = "COVTM       | K*ms-1              | Covariance between Temperature and Meridional Components of the wind.";
   map_id[ "2_208"] = "VDFUA       | ms-2                | Vertical Diffusion Zonal Acceleration";
   map_id[ "2_209"] = "VDFVA       | ms-2                | Vertical Diffusion Meridional Acceleration";
   map_id[ "2_210"] = "GWDU        | ms-2                | Gravity wave drag zonal acceleration";
   map_id[ "2_211"] = "GWDV        | ms-2                | Gravity wave drag meridional acceleration";
   map_id[ "2_212"] = "CNVU        | ms-2                | Convective zonal momentum mixing acceleration";
   map_id[ "2_213"] = "CNVV        | ms-2                | Convective meridional momentum mixing acceleration";
   map_id[ "2_214"] = "WTEND       | m/s2                | Tendency of vertical velocity";
   map_id[ "2_215"] = "OMGALF      | K                   | Omega (Dp/Dt) divide by density";
   map_id[ "2_216"] = "CNGWDU      | m/s2                | Convective Gravity wave drag zonal acceleration";
   map_id[ "2_217"] = "CNGWDV      | m/s2                | Convective Gravity wave drag meridional acceleration";
   map_id[ "2_218"] = "LMV         | -                   | Velocity Point Model Surface";
   map_id[ "2_219"] = "PVMWW       | 1/s/m               | Potential Vorticity (Mass-Weighted)";
   map_id[ "2_220"] = "MAXUVV      | m/s                 | Hourly Maximum of Upward Vertical Velocity  in the lowest 400hPa";
   map_id[ "2_221"] = "MAXDVV      | m/s                 | Hourly Maximum of Downward Vertical Velocity  in the lowest 400hPa";
   map_id[ "2_222"] = "MAXUW       | m/s                 | U Component of Hourly Maximum 10m Wind Speed";
   map_id[ "2_223"] = "MAXVW       | m/s                 | V Component of Hourly Maximum 10m Wind Speed";
   map_id[ "2_224"] = "VRATE       | m2/s                | Ventilation Rate";
   map_id[   "3_0"] = "PRES        | Pa                  | Pressure";
   map_id[   "3_1"] = "PRMSL       | Pa                  | Pressure Reduced to MSL";
   map_id[   "3_2"] = "PTEND       | Pa/s                | Pressure Tendency";
   map_id[   "3_3"] = "ICAHT       | m                   | ICAO Standard Atmosphere Reference Height";
   map_id[   "3_4"] = "GP          | m^2/s^2             | Geopotential";
   map_id[   "3_5"] = "HGT         | gpm                 | Geopotential Height";
   map_id[   "3_6"] = "DIST        | m                   | Geometric Height";
   map_id[   "3_7"] = "HSTDV       | m                   | Standard Deviation of Height";
   map_id[   "3_8"] = "PRESA       | Pa                  | Pressure Anomaly";
   map_id[   "3_9"] = "GPA         | gpm                 | Geopotential Height Anomaly";
   map_id[  "3_10"] = "DEN         | kg/m^3              | Density";
   map_id[  "3_11"] = "ALTS        | Pa                  | Altimeter Setting";
   map_id[  "3_12"] = "THICK       | m                   | Thickness";
   map_id[  "3_13"] = "PRESALT     | m                   | Pressure Altitude";
   map_id[  "3_14"] = "DENALT      | m                   | Density Altitude";
   map_id[  "3_15"] = "5WAVH       | gpm                 | 5-Wave Geopotential Height";
   map_id[  "3_16"] = "U-GWD       | N/m^2               | Zonal Flux of Gravity Wave Stress";
   map_id[  "3_17"] = "V-GWD       | N/m^2               | Meridional Flux of Gravity Wave Stress";
   map_id[  "3_18"] = "HPBL        | m                   | Planetary Boundary Layer Height";
   map_id[  "3_19"] = "5WAVA       | gpm                 | 5-Wave Geopotential Height Anomaly";
   map_id[  "3_20"] = "SDSGSO      | m                   | Standard Deviation Of Sub-Grid Scale Orography";
   map_id[  "3_21"] = "AOSGSO      | Rad                 | Angle Of Sub-Grid Scale Orography";
   map_id[  "3_22"] = "SSGSO       | Numeric             | Slope Of Sub-Grid Scale Orography";
   map_id[  "3_23"] = "GSGSO       | W/m^2               | Gravity Of Sub-Grid Scale Orography";
   map_id[  "3_24"] = "ASGSO       | Numeric             | Anisotropy Of Sub-Grid Scale Orography";
   map_id[  "3_25"] = "NLPRES      | Numeric             | Natural Logarithm Of Pressure in Pa";
   map_id[ "3_192"] = "MSLET       | Pa                  | MSLP (Eta model reduction)";
   map_id[ "3_193"] = "5WAVH       | gpm                 | 5-Wave Geopotential Height";
   map_id[ "3_194"] = "U-GWD       | N/m^2               | Zonal Flux of Gravity Wave Stress";
   map_id[ "3_195"] = "V-GWD       | N/m^2               | Meridional Flux of Gravity Wave Stress";
   map_id[ "3_196"] = "HPBL        | m                   | Planetary Boundary Layer Height";
   map_id[ "3_197"] = "5WAVA       | gpm                 | 5-Wave Geopotential Height Anomaly";
   map_id[ "3_198"] = "MSLMA       | Pa                  | MSLP (MAPS System Reduction)";
   map_id[ "3_199"] = "TSLSA       | Pa/s                | 3-hr pressure tendency (Std. Atmos. Reduction)";
   map_id[ "3_200"] = "PLPL        | Pa                  | Pressure of level from which parcel was lifted";
   map_id[ "3_201"] = "LPSX        | 1/m                 | X-gradient of Log Pressure";
   map_id[ "3_202"] = "LPSY        | 1/m                 | Y-gradient of Log Pressure";
   map_id[ "3_203"] = "HGTX        | 1/m                 | X-gradient of Height";
   map_id[ "3_204"] = "HGTY        | 1/m                 | Y-gradient of Height";
   map_id[ "3_205"] = "LAYTH       | m                   | Layer Thickness";
   map_id[ "3_206"] = "NLGSP       | ln(kPa)             | Natural Log of Surface Pressure";
   map_id[ "3_207"] = "CNVUMF      | kg/m2/s             | Convective updraft mass flux";
   map_id[ "3_208"] = "CNVDMF      | kg/m2/s             | Convective downdraft mass flux";
   map_id[ "3_209"] = "CNVDEMF     | kg/m2/s             | Convective detrainment mass flux";
   map_id[ "3_210"] = "LMH         | -                   | Mass Point Model Surface";
   map_id[ "3_211"] = "HGTN        | gpm                 | Geopotential Height (nearest grid point)";
   map_id[ "3_212"] = "PRESN       | Pa                  | Pressure (nearest grid point)";
   map_id[   "4_0"] = "NSWRS       | W/m^2               | Net Short-Wave Radiation Flux (Surface)";
   map_id[   "4_1"] = "NSWRT       | W/m^2               | Net Short-Wave Radiation Flux (Top of Atmosphere)";
   map_id[   "4_2"] = "SWAVR       | W/m^2               | Short-Wave Radiation Flux";
   map_id[   "4_3"] = "GRAD        | W/m^2               | Global Radiation Flux";
   map_id[   "4_4"] = "BRTMP       | K                   | Brightness Temperature";
   map_id[   "4_5"] = "LWRAD       | W/m/sr              | Radiance (with respect to wave number)";
   map_id[   "4_6"] = "SWRAD       | W/m^3/sr            | Radiance (with respect to wavelength)";
   map_id[   "4_7"] = "DSWRF       | W/m^2               | Downward Short-Wave Rad. Flux";
   map_id[   "4_8"] = "USWRF       | W/m^2               | Upward Short-Wave Rad. Flux";
   map_id[   "4_9"] = "NSWRF       | W/m^2               | Net Short Wave Radiation Flux";
   map_id[  "4_10"] = "PHOTAR      | W/m^2               | Photosynthetically Active Radiation";
   map_id[  "4_11"] = "NSWRFCS     | W/m^2               | Net Short-Wave Radiation Flux, Clear Sky";
   map_id[  "4_12"] = "DWUVR       | W/m^2               | Downward UV Radiation";
   map_id[  "4_50"] = "UVIUCS      | Numeric             | UV Index (Under Clear Sky)";
   map_id[  "4_51"] = "UVI         | W/m^2               | UV Index";
   map_id[ "4_192"] = "DSWRF       | W/m^2               | Downward Short-Wave Rad. Flux";
   map_id[ "4_193"] = "USWRF       | W/m^2               | Upward Short-Wave Rad. Flux";
   map_id[ "4_194"] = "DUVB        | W/m^2               | UV-B downward solar flux";
   map_id[ "4_195"] = "CDUVB       | W/m^2               | Clear sky UV-B downward solar flux";
   map_id[ "4_196"] = "CSDSF       | W/m^2               | Clear Sky Downward Solar Flux";
   map_id[ "4_197"] = "SWHR        | K/s                 | Solar Radiative Heating Rate";
   map_id[ "4_198"] = "CSUSF       | W/m^2               | Clear Sky Upward Solar Flux";
   map_id[ "4_199"] = "CFNSF       | W/m^2               | Cloud Forcing Net Solar Flux";
   map_id[ "4_200"] = "VBDSF       | W/m^2               | Visible Beam Downward Solar Flux";
   map_id[ "4_201"] = "VDDSF       | W/m^2               | Visible Diffuse Downward Solar Flux";
   map_id[ "4_202"] = "NBDSF       | W/m^2               | Near IR Beam Downward Solar Flux";
   map_id[ "4_203"] = "NDDSF       | W/m^2               | Near IR Diffuse Downward Solar Flux";
   map_id[ "4_204"] = "DTRF        | W/m^2               | Downward Total radiation Flux";
   map_id[ "4_205"] = "UTRF        | W/m^2               | Upward Total radiation Flux";
   map_id[   "5_0"] = "NLWRS       | W/m^2               | Net Long-Wave Radiation Flux (Surface)";
   map_id[   "5_1"] = "NLWRT       | W/m^2               | Net Long-Wave Radiation Flux (Top of Atmosphere)";
   map_id[   "5_2"] = "LWAVR       | W/m^2               | Long-Wave Radiation Flux";
   map_id[   "5_3"] = "DLWRF       | W/m^2               | Downward Long-Wave Rad. Flux";
   map_id[   "5_4"] = "ULWRF       | W/m^2               | Upward Long-Wave Rad. Flux";
   map_id[   "5_5"] = "NLWRF       | W/m^2               | Net Long-Wave Radiation Flux";
   map_id[   "5_6"] = "NLWRCS      | W/m^2               | Net Long-Wave Radiation Flux, Clear Sky";
   map_id[ "5_192"] = "DLWRF       | W/m^2               | Downward Long-Wave Rad. Flux";
   map_id[ "5_193"] = "ULWRF       | W/m^2               | Upward Long-Wave Rad. Flux";
   map_id[ "5_194"] = "LWHR        | K/s                 | Long-Wave Radiative Heating Rate";
   map_id[ "5_195"] = "CSULF       | W/m^2               | Clear Sky Upward Long Wave Flux";
   map_id[ "5_196"] = "CSDLF       | W/m^2               | Clear Sky Downward Long Wave Flux";
   map_id[ "5_197"] = "CFNLF       | W/m^2               | Cloud Forcing Net Long Wave Flux";
   map_id[   "6_0"] = "CICE        | kg/m^2              | Cloud Ice";
   map_id[   "6_1"] = "TCDC        | %                   | Total Cloud Cover";
   map_id[   "6_2"] = "CDCON       | %                   | Convective Cloud Cover";
   map_id[   "6_3"] = "LCDC        | %                   | Low Cloud Cover";
   map_id[   "6_4"] = "MCDC        | %                   | Medium Cloud Cover";
   map_id[   "6_5"] = "HCDC        | %                   | High Cloud Cover";
   map_id[   "6_6"] = "CWAT        | kg/m^2              | Cloud Water";
   map_id[   "6_7"] = "CDCA        | %                   | Cloud Amount";
   map_id[   "6_8"] = "CDCT        | -                   | Cloud Type";
   map_id[   "6_9"] = "TMAXT       | m                   | Thunderstorm Maximum Tops";
   map_id[  "6_10"] = "THUNC       | -                   | Thunderstorm Coverage";
   map_id[  "6_11"] = "CDCB        | m                   | Cloud Base";
   map_id[  "6_12"] = "CDCT        | m                   | Cloud Top";
   map_id[  "6_13"] = "CEIL        | m                   | Ceiling";
   map_id[  "6_14"] = "CDLYR       | %                   | Non-Convective Cloud Cover";
   map_id[  "6_15"] = "CWORK       | J/kg                | Cloud Work Function";
   map_id[  "6_16"] = "CUEFI       | Proportion          | Convective Cloud Efficiency";
   map_id[  "6_17"] = "TCOND       | kg/kg               | Total Condensate";
   map_id[  "6_18"] = "TCOLW       | kg/m^2              | Total Column-Integrated Cloud Water";
   map_id[  "6_19"] = "TCOLI       | kg/m^2              | Total Column-Integrated Cloud Ice";
   map_id[  "6_20"] = "TCOLC       | kg/m^2              | Total Column-Integrated Condensate";
   map_id[  "6_21"] = "FICE        | Proportion          | Ice fraction of total condensate";
   map_id[  "6_22"] = "CDCC        | %                   | Cloud Cover";
   map_id[  "6_23"] = "CDCIMR      | kg/kg               | Cloud Ice Mixing Ratio";
   map_id[  "6_24"] = "SUNS        | Numeric             | Sunshine";
   map_id[  "6_25"] = "CBHE        | %                   | Horizontal Extent of Cumulonimbus (CB)";
   map_id[  "6_33"] = "SUNSD       | s                   | Sunshine Duration";
   map_id[ "6_192"] = "CDLYR       | %                   | Non-Convective Cloud Cover";
   map_id[ "6_193"] = "CWORK       | J/kg                | Cloud Work Function";
   map_id[ "6_194"] = "CUEFI       | non-dim             | Convective Cloud Efficiency";
   map_id[ "6_195"] = "TCOND       | kg/kg               | Total Condensate";
   map_id[ "6_196"] = "TCOLW       | kg/m^2              | Total Column-Integrated Cloud Water";
   map_id[ "6_197"] = "TCOLI       | kg/m^2              | Total Column-Integrated Cloud Ice";
   map_id[ "6_198"] = "TCOLC       | kg/m^2              | Total Column-Integrated Condensate";
   map_id[ "6_199"] = "FICE        | non-dim             | Ice fraction of total condensate";
   map_id[ "6_200"] = "MFLUX       | Pa/s                | Convective Cloud Mass Flux";
   map_id[ "6_201"] = "SUNSD       | s                   | Sunshine Duration";
   map_id[   "7_0"] = "PLI         | K                   | Parcel Lifted Index (to 500 mb)";
   map_id[   "7_1"] = "BLI         | K                   | Best Lifted Index (to 500 mb)";
   map_id[   "7_2"] = "KX          | K                   | K Index";
   map_id[   "7_3"] = "KOX         | K                   | KO Index";
   map_id[   "7_4"] = "TOTALX      | K                   | Total Totals Index";
   map_id[   "7_5"] = "SX          | Numeric             | Sweat Index";
   map_id[   "7_6"] = "CAPE        | J/kg                | Convective Available Potential Energy";
   map_id[   "7_7"] = "CIN         | J/kg                | Convective Inhibition";
   map_id[   "7_8"] = "HLCY        | m^2/s^2             | Storm Relative Helicity";
   map_id[   "7_9"] = "EHLX        | Numeric             | Energy Helicity Index";
   map_id[  "7_10"] = "LFTX        | K                   | Surface Lifted Index";
   map_id[  "7_11"] = "4LFTX       | K                   | Best (4 layer) Lifted Index";
   map_id[  "7_12"] = "RI          | Numeric             | Richardson Number";
   map_id[ "7_192"] = "LFTX        | K                   | Surface Lifted Index";
   map_id[ "7_193"] = "4LFTX       | K                   | Best (4 layer) Lifted Index";
   map_id[ "7_194"] = "RI          | Numeric             | Richardson Number";
   map_id[ "7_195"] = "CWDI        | -                   | Convective Weather Detection Index";
   map_id[ "7_196"] = "UVI         | W/m^2               | Ultra Violet Index";
   map_id[ "7_197"] = "UPHL        | m^2/s^2             | Updraft Helicity";
   map_id[ "7_198"] = "LAI         | -                   | Leaf Area Index";
   map_id[ "7_199"] = "MXUPHL      | m2/s2               | Hourly Maximum of Updraft Helicity over layer 2km to 5 km AGL";
   map_id[   "8_0"] = "AEROT       | -                   | Aerosol Type";
   map_id[ "8_192"] = "PMTC        | 10^-6g/m^3          | Particulate matter (coarse)";
   map_id[ "8_193"] = "PMTF        | 10^-6g/m^3          | Particulate matter (fine)";
   map_id[ "8_194"] = "LPMTF       | log10(10^-6g/m^3)   | Particulate matter (fine)";
   map_id[ "8_195"] = "LIPMF       | log10(10^-6g/m^3)   | Integrated column particulate matter (fine)";
   map_id[   "9_0"] = "TOZNE       | Dobson              | Total Ozone";
   map_id[   "9_1"] = "O3MR        | kg/kg               | Ozone Mixing Ratio";
   map_id[   "9_2"] = "TCIOZ       | Dobson              | Total Column Integrated Ozone";
   map_id[ "9_192"] = "O3MR        | kg/kg               | Ozone Mixing Ratio";
   map_id[ "9_193"] = "OZCON       | ppb                 | Ozone Concentration (PPB)";
   map_id[ "9_194"] = "OZCAT       | non-dim             | Categorical Ozone Concentration";
   map_id[ "9_195"] = "VDFOZ       | kg/kg/s             | Ozone Vertical Diffusion";
   map_id[ "9_196"] = "POZ         | kg/kg/s             | Ozone Production";
   map_id[ "9_197"] = "TOZ         | kg/kg/s             | Ozone Tendency";
   map_id[ "9_198"] = "POZT        | kg/kg/s             | Ozone Production from Temperature Term";
   map_id[ "9_199"] = "POZO        | kg/kg/s             | Ozone Production from Column Ozone Term";
   map_id[ "9_200"] = "OZMAX1      | ppbV                | Ozone Daily Max from 1-hour Average";
   map_id[ "9_201"] = "OZMAX8      | ppbV                | Ozone Daily Max from 8-hour Average";
   map_id[ "9_202"] = "PDMAX1      | &#956g/m3           | PM 2.5 Daily Max from 1-hour Average";
   map_id[ "9_203"] = "PDMAX24     | &#956g/m3           | PM 2.5 Daily Max from 24-hour Average";
   map_id[  "10_0"] = "BSWID       | m/s                 | Base Spectrum Width";
   map_id[  "10_1"] = "BREF        | dB                  | Base Reflectivity";
   map_id[  "10_2"] = "BRVEL       | m/s                 | Base Radial Velocity";
   map_id[  "10_3"] = "VERIL       | kg/m                | Vertically Integrated Liquid (VIL)";
   map_id[  "10_4"] = "LMAXBR      | dB                  | Layer Maximum Base Reflectivity";
   map_id[  "10_5"] = "PREC        | kg/m^2              | Precipitation";
   map_id[  "10_6"] = "RDSP1       | -                   | Radar Spectra (1)";
   map_id[  "10_7"] = "RDSP2       | -                   | Radar Spectra (2)";
   map_id[  "10_8"] = "RDSP3       | -                   | Radar Spectra (3)";
   map_id[  "11_0"] = "REFZR       | mm^6/m^3            | Equivalent radar reflectivity factor for rain";
   map_id[  "11_1"] = "REFZI       | mm^6/m^3            | Equivalent radar reflectivity factor for snow";
   map_id[  "11_2"] = "REFZC       | mm^6/m^3            | Equivalent radar reflectivity factor for parameterized convection";
   map_id[  "11_3"] = "RETOP       | m                   | Echo Top";
   map_id[  "11_4"] = "REFD        | dB                  | Reflectivity";
   map_id[  "11_5"] = "REFC        | dB                  | Composite reflectivity";
   map_id["11_192"] = "REFZR       | mm^6/m^3            | Equivalent radar reflectivity factor for rain";
   map_id["11_193"] = "REFZI       | mm^6/m^3            | Equivalent radar reflectivity factor for snow";
   map_id["11_194"] = "REFZC       | mm^6/m^3            | Equivalent radar reflectivity factor for parameterized convection";
   map_id["11_195"] = "REFD        | dB                  | Reflectivity";
   map_id["11_196"] = "REFC        | dB                  | Composite reflectivity";
   map_id["11_197"] = "RETOP       | m                   | Echo Top";
   map_id["11_198"] = "MAXREF      | dB                  | Hourly Maximum of Simulated Reflectivity at 1 km AGL";
   map_id["12_192"] = "LTNG        | non-dim             | Lightning";
   map_id[  "13_0"] = "ACCES       | Bq/m^3              | Air Concentration of Cesium 137";
   map_id[  "13_1"] = "ACIOD       | Bq/m^3              | Air Concentration of Iodine 131";
   map_id[  "13_2"] = "ACRADP      | Bq/m^3              | Air Concentration of Radioactive Pollutant";
   map_id[  "13_3"] = "GDCES       | Bq/m^2              | Ground Deposition of Cesium 137";
   map_id[  "13_4"] = "GDIOD       | Bq/m^2              | Ground Deposition of Iodine 131";
   map_id[  "13_5"] = "GDRADP      | Bq/m^2              | Ground Deposition of Radioactive Pollutant";
   map_id[  "13_6"] = "TIACCP      | Bq/m^3              | Time Integrated Air Concentration of Cesium Pollutant";
   map_id[  "13_7"] = "TIACIP      | Bq/m^3              | Time Integrated Air Concentration of Iodine Pollutant";
   map_id[  "13_8"] = "TIACRP      | Bq/m^3              | Time Integrated Air Concentration of Radioactive Pollutant";
   map_id[  "14_0"] = "VIS         | m                   | Visibility";
   map_id[  "14_1"] = "ALBDO       | %                   | Albedo";
   map_id[  "14_2"] = "TSTM        | %                   | Thunderstorm Probability";
   map_id[  "14_3"] = "MIXHT       | m                   | Mixed Layer Depth";
   map_id[  "14_4"] = "VOLASH      | -                   | Volcanic Ash";
   map_id[  "14_5"] = "ICIT        | m                   | Icing Top";
   map_id[  "14_6"] = "ICIB        | m                   | Icing Base";
   map_id[  "14_7"] = "ICI         | -                   | Icing";
   map_id[  "14_8"] = "TURBT       | m                   | Turbulence Top";
   map_id[  "14_9"] = "TURBB       | m                   | Turbulence Base";
   map_id[ "14_10"] = "TURB        | -                   | Turbulence";
   map_id[ "14_11"] = "TKE         | J/kg                | Turbulent Kinetic Energy";
   map_id[ "14_12"] = "PBLREG      | -                   | Planetary Boundary Layer Regime";
   map_id[ "14_13"] = "CONTI       | -                   | Contrail Intensity";
   map_id[ "14_14"] = "CONTET      | -                   | Contrail Engine Type";
   map_id[ "14_15"] = "CONTT       | m                   | Contrail Top";
   map_id[ "14_16"] = "CONTB       | m                   | Contrail Base";
   map_id[ "14_17"] = "MXSALB      | %                   | Maximum Snow Albedo";
   map_id[ "14_18"] = "SNFALB      | %                   | Snow-Free Albedo";
   map_id[ "14_19"] = "SALBD       | %                   | Snow Albedo";
   map_id[ "14_20"] = "ICIP        | %                   | Icing";
   map_id[ "14_21"] = "CTP         | %                   | In-Cloud Turbulence";
   map_id[ "14_22"] = "CAT         | %                   | Clear Air Turbulence (CAT)";
   map_id[ "14_23"] = "SLDP        | %                   | Supercooled Large Droplet (SLD) Probability";
   map_id["14_192"] = "MXSALB      | %                   | Maximum Snow Albedo";
   map_id["14_193"] = "SNFALB      | %                   | Snow-Free Albedo";
   map_id["14_194"] = "SRCONO      | categorical         | Slight risk convective outlook";
   map_id["14_195"] = "MRCONO      | categorical         | Moderate risk convective outlook";
   map_id["14_196"] = "HRCONO      | categorical         | High risk convective outlook";
   map_id["14_197"] = "TORPROB     | %                   | Tornado probability";
   map_id["14_198"] = "HAILPROB    | %                   | Hail probability";
   map_id["14_199"] = "WINDPROB    | %                   | Wind probability";
   map_id["14_200"] = "STORPROB    | %                   | Significant Tornado probability";
   map_id["14_201"] = "SHAILPRO    | %                   | Significant Hail probability";
   map_id["14_202"] = "SWINDPRO    | %                   | Significant Wind probability";
   map_id["14_203"] = "TSTMC       | categorical         | Categorical Thunderstorm (1-yes, 0-no)";
   map_id["14_204"] = "MIXLY       | Integer             | Number of mixed layers next to surface";
   map_id["14_205"] = "FLGHT       | -                   | Flight Category";
   map_id["14_206"] = "CICEL       | -                   | Confidence - Ceiling";
   map_id["14_207"] = "CIVIS       | -                   | Confidence - Visibility";
   map_id["14_208"] = "CIFLT       | -                   | Confidence - Flight Category";
   map_id["14_209"] = "LAVNI       | -                   | Low-Level aviation interest";
   map_id["14_210"] = "HAVNI       | -                   | High-Level aviation interest";
   map_id["14_211"] = "SBSALB      | %                   | Visible, Black Sky Albedo";
   map_id["14_212"] = "SWSALB      | %                   | Visible, White Sky Albedo";
   map_id["14_213"] = "NBSALB      | %                   | Near IR, Black Sky Albedo";
   map_id["14_214"] = "NWSALB      | %                   | Near IR, White Sky Albedo";
   map_id["14_215"] = "PRSVR       | %                   | Total Probability of Severe Thunderstorms (Days 2,3)";
   map_id["14_216"] = "PRSIGSVR    | %                   | Total Probability of Extreme Severe Thunderstorms (Days 2,3)";
   map_id["14_217"] = "SIPD        | -                   | Supercooled Large Droplet (SLD) Icing";
   map_id["14_218"] = "EPSR        | -                   | Radiative emissivity";
   map_id["14_219"] = "TPFI        | -                   | Turbulence Potential Forecast Index";
   map_id["14_232"] = "VAFTD       | log10(kg/m3)        | Volcanic Ash Forecast Transport and Dispersion";
   map_id[  "15_0"] = "MASSDEN     | kg/m^3              | Mass Density (Concentration)";
   map_id[  "15_1"] = "COLMD       | kg/m^2              | Column-Integrated Mass Density";
   map_id[  "15_2"] = "MASSMR      | kg/kg               | Mass Mixing Ratio (Mass Fraction in Air)";
   map_id[  "15_3"] = "AEMFLX      | kg/m^2/s            | Atmosphere Emission Mass Flux";
   map_id[  "15_4"] = "ANPMFLX     | kg/m^2/s            | Atmosphere Net Production Mass Flux";
   map_id[  "15_5"] = "ANPEMFLX    | kg/m^2/s            | Atmosphere Net Production And Emision Mass Flux";
   map_id[  "15_6"] = "SDDMFLX     | kg/m^2/s            | Surface Dry Deposition Mass Flux";
   map_id[  "15_7"] = "SWDMFLX     | kg/m^2/s            | Surface Wet Deposition Mass Flux";
   map_id[  "15_8"] = "AREMFLX     | kg/m^2/s            | Atmosphere Re-Emission Mass Flux";
   map_id[ "15_50"] = "AIA         | mol                 | Amount in Atmosphere";
   map_id[ "15_51"] = "CONAIR      | molm-3              | Concentration In Air";
   map_id[ "15_52"] = "VMXR        | molmol-1            | Volume Mixing Ratio (Fraction in Air)";
   map_id[ "15_53"] = "CGPRC       | molm-3s-1           | Chemical Gross Production Rate of Concentration";
   map_id[ "15_54"] = "CGDRC       | molm-3s-1           | Chemical Gross Destruction Rate of Concentration";
   map_id[ "15_55"] = "SFLUX       | molm-2s-1           | Surface Flux";
   map_id[ "15_56"] = "COAIA       | mol/s               | Changes Of Amount in Atmosphere";
   map_id[ "15_57"] = "TYABA       | mol                 | Total Yearly Average Burden of The Atmosphere";
   map_id[ "15_58"] = "TYAAL       | mol/s               | Total Yearly Average Atmospheric Loss";
   map_id["15_100"] = "SADEN       | 1/m                 | Surface Area Density (Aerosol)";
   map_id["15_101"] = "AOTK        | m                   | Atmosphere Optical Thickness";
   map_id["15_131"] = "NO2TROP     | molcm-2             | Nitrogen Dioxide (NO2) Tropospheric Column";
   map_id["15_132"] = "NO2VCD      | molcm-2             | Nitrogen Dioxide (NO2) Vertical Column Density";
   map_id["15_133"] = "BROVCD      | molcm-2             | Bromine Monoxide (BrO) Vertical Column Density";
   map_id["15_134"] = "HCHOVCD     | molcm-2             | Formaldehyde (HCHO) Vertical Column Density";
   map_id[  "16_0"] = "var190m0    | CCITTIA5            | Arbitrary Text String";
   map_id[  "16_0"] = "TSEC        | s                   | Seconds prior to initial reference time (defined in Section 1)";
   map_id["16_192"] = "NLAT        | deg                 | Latitude (-90 to +90)";
   map_id["16_193"] = "ELON        | deg                 | East Longitude (0 - 360)";
   map_id["16_194"] = "TSEC        | s                   | Seconds prior to initial reference time";
   map_id["16_195"] = "MLYNO       | -                   | Model Layer number (From bottom up)";
   map_id["16_196"] = "NLATN       | deg                 | Latitude (nearest neighbor) (-90 to +90)";
   map_id["16_197"] = "ELONN       | deg                 | East Longitude (nearest neighbor) (0 - 360)";
   map_id[  "17_1"] = "COVMZ       | m2/s2               | Covariance between zonal and meridional components of the wind. Defined as [uv]-[u][v], where [] indicates the mean over the indicated time span.";
   map_id[  "17_2"] = "COVTZ       | K*m/s               | Covariance between izonal component of the wind and temperature. Defined as [uT]-[u][T], where [] indicates the mean over the indicated time span.";
   map_id[  "17_3"] = "COVTM       | K*m/s               | Covariance between meridional component of the wind and temperature. Defined as [vT]-[v][T], where [] indicates the mean over the indicated time span.";
   map_id[  "17_4"] = "COVTW       | K*m/s               | Covariance between temperature and vertical component of the wind. Defined as [wT]-[w][T], where [] indicates the mean over the indicated time span.";
   map_id[  "17_5"] = "COVZZ       | m2/s2               | Covariance between zonal and zonal components of the wind. Defined as [uu]-[u][u], where [] indicates the mean over the indicated time span.";
   map_id[  "17_6"] = "COVMM       | m2/s2               | Covariance between meridional and meridional components of the wind. Defined as [vv]-[v][v], where [] indicates the mean over the indicated time span.";
   map_id[  "17_7"] = "COVQZ       | kg/kg*m/s           | Covariance between specific humidity and zonal components of the wind. Defined as [uq]-[u][q], where [] indicates the mean over the indicated time span.";
   map_id[  "17_8"] = "COVQM       | kg/kg*m/s           | Covariance between specific humidity and meridional components of the wind. Defined as [vq]-[v][q], where [] indicates the mean over the indicated time span.";
   map_id[  "17_9"] = "COVTVV      | K*Pa/s              | Covariance between temperature and vertical components of the wind. Defined as [&#937T]-[&#937][T], where [] indicates the mean over the indicated time span.";
   map_id[ "17_10"] = "COVQVV      | kg/kg*Pa/s          | Covariance between specific humidity and vertical components of the wind. Defined as [&#937q]-[&#937][q], where [] indicates the mean over the indicated time span.";
   map_id[ "17_11"] = "COVPSPS     | Pa*Pa               | Covariance between surface pressure and surface pressure. Defined as [Psfc]-[Psfc][Psfc], where [] indicates the mean over the indicated time span.";
   map_id[ "17_12"] = "COVQQ       | kg/kg*kg/kg         | Covariance between specific humidity and specific humidy. Defined as [qq]-[q][q], where [] indicates the mean over the indicated time span.";
   map_id[ "17_13"] = "COVVVVV     | Pa2/s2              | Covariance between vertical and vertical components of the wind. Defined as [&#937&#937]-[&#937][&#937], where [] indicates the mean over the indicated time span.";
   map_id[ "17_14"] = "COVTT       | K*K                 | Covariance between temperature and temperature. Defined as [TT]-[T][T], where [] indicates the mean over the indicated time span.";

   map_code[     "TMP"] = "0_0";
   map_code[    "VTMP"] = "0_1";
   map_code[     "POT"] = "0_2";
   map_code[    "EPOT"] = "0_3";
   map_code[    "TMAX"] = "0_4";
   map_code[    "TMIN"] = "0_5";
   map_code[     "DPT"] = "0_6";
   map_code[    "DEPR"] = "0_7";
   map_code[    "LAPR"] = "0_8";
   map_code[    "TMPA"] = "0_9";
   map_code[   "LHTFL"] = "0_10";
   map_code[   "SHTFL"] = "0_11";
   map_code[   "HEATX"] = "0_12";
   map_code[     "WCF"] = "0_13";
   map_code[  "MINDPD"] = "0_14";
   map_code[   "VPTMP"] = "0_15";
   map_code[   "SNOHF"] = "0_16";
   map_code[   "SKINT"] = "0_17";
   map_code[   "SNOHF"] = "0_192";
   map_code[   "TTRAD"] = "0_193";
   map_code[     "REV"] = "0_194";
   map_code[   "LRGHR"] = "0_195";
   map_code[   "CNVHR"] = "0_196";
   map_code[   "THFLX"] = "0_197";
   map_code[   "TTDIA"] = "0_198";
   map_code[   "TTPHY"] = "0_199";
   map_code[   "TSD1D"] = "0_200";
   map_code[   "SHAHR"] = "0_201";
   map_code[   "VDFHR"] = "0_202";
   map_code[    "THZ0"] = "0_203";
   map_code[    "TCHP"] = "0_204";
   map_code[    "SPFH"] = "1_0";
   map_code[      "RH"] = "1_1";
   map_code[    "MIXR"] = "1_2";
   map_code[    "PWAT"] = "1_3";
   map_code[    "VAPP"] = "1_4";
   map_code[    "SATD"] = "1_5";
   map_code[     "EVP"] = "1_6";
   map_code[   "PRATE"] = "1_7";
   map_code[    "APCP"] = "1_8";
   map_code[   "NCPCP"] = "1_9";
   map_code[   "ACPCP"] = "1_10";
   map_code[    "SNOD"] = "1_11";
   map_code[   "SRWEQ"] = "1_12";
   map_code[   "WEASD"] = "1_13";
   map_code[    "SNOC"] = "1_14";
   map_code[    "SNOL"] = "1_15";
   map_code[    "SNOM"] = "1_16";
   map_code[   "SNOAG"] = "1_17";
   map_code[    "ABSH"] = "1_18";
   map_code[   "PTYPE"] = "1_19";
   map_code[   "ILIQW"] = "1_20";
   map_code[   "TCOND"] = "1_21";
   map_code[   "CLWMR"] = "1_22";
   map_code[    "ICMR"] = "1_23";
   map_code[    "RWMR"] = "1_24";
   map_code[    "SNMR"] = "1_25";
   map_code[   "MCONV"] = "1_26";
   map_code[   "MAXRH"] = "1_27";
   map_code[   "MAXAH"] = "1_28";
   map_code[   "ASNOW"] = "1_29";
   map_code[   "PWCAT"] = "1_30";
   map_code[    "HAIL"] = "1_31";
   map_code[    "GRLE"] = "1_32";
   map_code[   "CRAIN"] = "1_33";
   map_code[   "CFRZR"] = "1_34";
   map_code[   "CICEP"] = "1_35";
   map_code[   "CSNOW"] = "1_36";
   map_code[   "CPRAT"] = "1_37";
   map_code[   "MCONV"] = "1_38";
   map_code[   "CPOFP"] = "1_39";
   map_code[   "PEVAP"] = "1_40";
   map_code[   "PEVPR"] = "1_41";
   map_code[   "SNOWC"] = "1_42";
   map_code[   "FRAIN"] = "1_43";
   map_code[    "RIME"] = "1_44";
   map_code[   "TCOLR"] = "1_45";
   map_code[   "TCOLS"] = "1_46";
   map_code[    "LSWP"] = "1_47";
   map_code[     "CWP"] = "1_48";
   map_code[   "TWATP"] = "1_49";
   map_code[  "TSNOWP"] = "1_50";
   map_code[   "TCWAT"] = "1_51";
   map_code[  "TPRATE"] = "1_52";
   map_code[   "TSRWE"] = "1_53";
   map_code[ "LSPRATE"] = "1_54";
   map_code[   "CSRWE"] = "1_55";
   map_code[  "LSSRWE"] = "1_56";
   map_code[  "TSRATE"] = "1_57";
   map_code[  "CSRATE"] = "1_58";
   map_code[ "LSSRATE"] = "1_59";
   map_code[    "SDWE"] = "1_60";
   map_code[    "SDEN"] = "1_61";
   map_code[   "SEVAP"] = "1_62";
   map_code[   "TCIWV"] = "1_64";
   map_code[  "RPRATE"] = "1_65";
   map_code[  "SPRATE"] = "1_66";
   map_code[  "FPRATE"] = "1_67";
   map_code[  "IPRATE"] = "1_68";
   map_code[   "CRAIN"] = "1_192";
   map_code[   "CFRZR"] = "1_193";
   map_code[   "CICEP"] = "1_194";
   map_code[   "CSNOW"] = "1_195";
   map_code[   "CPRAT"] = "1_196";
   map_code[   "MCONV"] = "1_197";
   map_code[   "MINRH"] = "1_198";
   map_code[   "PEVAP"] = "1_199";
   map_code[   "PEVPR"] = "1_200";
   map_code[   "SNOWC"] = "1_201";
   map_code[   "FRAIN"] = "1_202";
   map_code[    "RIME"] = "1_203";
   map_code[   "TCOLR"] = "1_204";
   map_code[   "TCOLS"] = "1_205";
   map_code[    "TIPD"] = "1_206";
   map_code[    "NCIP"] = "1_207";
   map_code[    "SNOT"] = "1_208";
   map_code[   "TCLSW"] = "1_209";
   map_code[   "TCOLM"] = "1_210";
   map_code[    "EMNP"] = "1_211";
   map_code[   "SBSNO"] = "1_212";
   map_code[   "CNVMR"] = "1_213";
   map_code[   "SHAMR"] = "1_214";
   map_code[   "VDFMR"] = "1_215";
   map_code[   "CONDP"] = "1_216";
   map_code[   "LRGMR"] = "1_217";
   map_code[     "QZ0"] = "1_218";
   map_code[    "QMAX"] = "1_219";
   map_code[    "QMIN"] = "1_220";
   map_code[   "ARAIN"] = "1_221";
   map_code[   "SNOWT"] = "1_222";
   map_code[   "APCPN"] = "1_223";
   map_code[  "ACPCPN"] = "1_224";
   map_code[    "FRZR"] = "1_225";
   map_code[  "PWTHER"] = "1_226";
   map_code[   "FROZR"] = "1_227";
   map_code[   "TSNOW"] = "1_241";
   map_code[    "WDIR"] = "2_0";
   map_code[    "WIND"] = "2_1";
   map_code[    "UGRD"] = "2_2";
   map_code[    "VGRD"] = "2_3";
   map_code[    "STRM"] = "2_4";
   map_code[    "VPOT"] = "2_5";
   map_code[   "MNTSF"] = "2_6";
   map_code[   "SGCVV"] = "2_7";
   map_code[    "VVEL"] = "2_8";
   map_code[    "DZDT"] = "2_9";
   map_code[    "ABSV"] = "2_10";
   map_code[    "ABSD"] = "2_11";
   map_code[    "RELV"] = "2_12";
   map_code[    "RELD"] = "2_13";
   map_code[   "PVORT"] = "2_14";
   map_code[   "VUCSH"] = "2_15";
   map_code[   "VVCSH"] = "2_16";
   map_code[    "UFLX"] = "2_17";
   map_code[    "VFLX"] = "2_18";
   map_code[   "WMIXE"] = "2_19";
   map_code[   "BLYDP"] = "2_20";
   map_code[ "MAXGUST"] = "2_21";
   map_code[    "GUST"] = "2_22";
   map_code[   "UGUST"] = "2_23";
   map_code[   "VGUST"] = "2_24";
   map_code[    "VWSH"] = "2_25";
   map_code[    "MFLX"] = "2_26";
   map_code[    "USTM"] = "2_27";
   map_code[    "VSTM"] = "2_28";
   map_code[      "CD"] = "2_29";
   map_code[   "FRICV"] = "2_30";
   map_code[  "ETACVV"] = "2_32";
   map_code[    "VWSH"] = "2_192";
   map_code[    "MFLX"] = "2_193";
   map_code[    "USTM"] = "2_194";
   map_code[    "VSTM"] = "2_195";
   map_code[      "CD"] = "2_196";
   map_code[   "FRICV"] = "2_197";
   map_code[    "LAUV"] = "2_198";
   map_code[    "LOUV"] = "2_199";
   map_code[    "LAVV"] = "2_200";
   map_code[    "LOVV"] = "2_201";
   map_code[    "LAPP"] = "2_202";
   map_code[    "LOPP"] = "2_203";
   map_code[    "VEDH"] = "2_204";
   map_code[   "COVMZ"] = "2_205";
   map_code[   "COVTZ"] = "2_206";
   map_code[   "COVTM"] = "2_207";
   map_code[   "VDFUA"] = "2_208";
   map_code[   "VDFVA"] = "2_209";
   map_code[    "GWDU"] = "2_210";
   map_code[    "GWDV"] = "2_211";
   map_code[    "CNVU"] = "2_212";
   map_code[    "CNVV"] = "2_213";
   map_code[   "WTEND"] = "2_214";
   map_code[  "OMGALF"] = "2_215";
   map_code[  "CNGWDU"] = "2_216";
   map_code[  "CNGWDV"] = "2_217";
   map_code[     "LMV"] = "2_218";
   map_code[   "PVMWW"] = "2_219";
   map_code[  "MAXUVV"] = "2_220";
   map_code[  "MAXDVV"] = "2_221";
   map_code[   "MAXUW"] = "2_222";
   map_code[   "MAXVW"] = "2_223";
   map_code[   "VRATE"] = "2_224";
   map_code[    "PRES"] = "3_0";
   map_code[   "PRMSL"] = "3_1";
   map_code[   "PTEND"] = "3_2";
   map_code[   "ICAHT"] = "3_3";
   map_code[      "GP"] = "3_4";
   map_code[     "HGT"] = "3_5";
   map_code[    "DIST"] = "3_6";
   map_code[   "HSTDV"] = "3_7";
   map_code[   "PRESA"] = "3_8";
   map_code[     "GPA"] = "3_9";
   map_code[     "DEN"] = "3_10";
   map_code[    "ALTS"] = "3_11";
   map_code[   "THICK"] = "3_12";
   map_code[ "PRESALT"] = "3_13";
   map_code[  "DENALT"] = "3_14";
   map_code[   "5WAVH"] = "3_15";
   map_code[   "U-GWD"] = "3_16";
   map_code[   "V-GWD"] = "3_17";
   map_code[    "HPBL"] = "3_18";
   map_code[   "5WAVA"] = "3_19";
   map_code[  "SDSGSO"] = "3_20";
   map_code[  "AOSGSO"] = "3_21";
   map_code[   "SSGSO"] = "3_22";
   map_code[   "GSGSO"] = "3_23";
   map_code[   "ASGSO"] = "3_24";
   map_code[  "NLPRES"] = "3_25";
   map_code[   "MSLET"] = "3_192";
   map_code[   "5WAVH"] = "3_193";
   map_code[   "U-GWD"] = "3_194";
   map_code[   "V-GWD"] = "3_195";
   map_code[    "HPBL"] = "3_196";
   map_code[   "5WAVA"] = "3_197";
   map_code[   "MSLMA"] = "3_198";
   map_code[   "TSLSA"] = "3_199";
   map_code[    "PLPL"] = "3_200";
   map_code[    "LPSX"] = "3_201";
   map_code[    "LPSY"] = "3_202";
   map_code[    "HGTX"] = "3_203";
   map_code[    "HGTY"] = "3_204";
   map_code[   "LAYTH"] = "3_205";
   map_code[   "NLGSP"] = "3_206";
   map_code[  "CNVUMF"] = "3_207";
   map_code[  "CNVDMF"] = "3_208";
   map_code[ "CNVDEMF"] = "3_209";
   map_code[     "LMH"] = "3_210";
   map_code[    "HGTN"] = "3_211";
   map_code[   "PRESN"] = "3_212";
   map_code[   "NSWRS"] = "4_0";
   map_code[   "NSWRT"] = "4_1";
   map_code[   "SWAVR"] = "4_2";
   map_code[    "GRAD"] = "4_3";
   map_code[   "BRTMP"] = "4_4";
   map_code[   "LWRAD"] = "4_5";
   map_code[   "SWRAD"] = "4_6";
   map_code[   "DSWRF"] = "4_7";
   map_code[   "USWRF"] = "4_8";
   map_code[   "NSWRF"] = "4_9";
   map_code[  "PHOTAR"] = "4_10";
   map_code[ "NSWRFCS"] = "4_11";
   map_code[   "DWUVR"] = "4_12";
   map_code[  "UVIUCS"] = "4_50";
   map_code[     "UVI"] = "4_51";
   map_code[   "DSWRF"] = "4_192";
   map_code[   "USWRF"] = "4_193";
   map_code[    "DUVB"] = "4_194";
   map_code[   "CDUVB"] = "4_195";
   map_code[   "CSDSF"] = "4_196";
   map_code[    "SWHR"] = "4_197";
   map_code[   "CSUSF"] = "4_198";
   map_code[   "CFNSF"] = "4_199";
   map_code[   "VBDSF"] = "4_200";
   map_code[   "VDDSF"] = "4_201";
   map_code[   "NBDSF"] = "4_202";
   map_code[   "NDDSF"] = "4_203";
   map_code[    "DTRF"] = "4_204";
   map_code[    "UTRF"] = "4_205";
   map_code[   "NLWRS"] = "5_0";
   map_code[   "NLWRT"] = "5_1";
   map_code[   "LWAVR"] = "5_2";
   map_code[   "DLWRF"] = "5_3";
   map_code[   "ULWRF"] = "5_4";
   map_code[   "NLWRF"] = "5_5";
   map_code[  "NLWRCS"] = "5_6";
   map_code[   "DLWRF"] = "5_192";
   map_code[   "ULWRF"] = "5_193";
   map_code[    "LWHR"] = "5_194";
   map_code[   "CSULF"] = "5_195";
   map_code[   "CSDLF"] = "5_196";
   map_code[   "CFNLF"] = "5_197";
   map_code[    "CICE"] = "6_0";
   map_code[    "TCDC"] = "6_1";
   map_code[   "CDCON"] = "6_2";
   map_code[    "LCDC"] = "6_3";
   map_code[    "MCDC"] = "6_4";
   map_code[    "HCDC"] = "6_5";
   map_code[    "CWAT"] = "6_6";
   map_code[    "CDCA"] = "6_7";
   map_code[    "CDCT"] = "6_8";
   map_code[   "TMAXT"] = "6_9";
   map_code[   "THUNC"] = "6_10";
   map_code[    "CDCB"] = "6_11";
   map_code[    "CDCT"] = "6_12";
   map_code[    "CEIL"] = "6_13";
   map_code[   "CDLYR"] = "6_14";
   map_code[   "CWORK"] = "6_15";
   map_code[   "CUEFI"] = "6_16";
   map_code[   "TCOND"] = "6_17";
   map_code[   "TCOLW"] = "6_18";
   map_code[   "TCOLI"] = "6_19";
   map_code[   "TCOLC"] = "6_20";
   map_code[    "FICE"] = "6_21";
   map_code[    "CDCC"] = "6_22";
   map_code[  "CDCIMR"] = "6_23";
   map_code[    "SUNS"] = "6_24";
   map_code[    "CBHE"] = "6_25";
   map_code[   "SUNSD"] = "6_33";
   map_code[   "CDLYR"] = "6_192";
   map_code[   "CWORK"] = "6_193";
   map_code[   "CUEFI"] = "6_194";
   map_code[   "TCOND"] = "6_195";
   map_code[   "TCOLW"] = "6_196";
   map_code[   "TCOLI"] = "6_197";
   map_code[   "TCOLC"] = "6_198";
   map_code[    "FICE"] = "6_199";
   map_code[   "MFLUX"] = "6_200";
   map_code[   "SUNSD"] = "6_201";
   map_code[     "PLI"] = "7_0";
   map_code[     "BLI"] = "7_1";
   map_code[      "KX"] = "7_2";
   map_code[     "KOX"] = "7_3";
   map_code[  "TOTALX"] = "7_4";
   map_code[      "SX"] = "7_5";
   map_code[    "CAPE"] = "7_6";
   map_code[     "CIN"] = "7_7";
   map_code[    "HLCY"] = "7_8";
   map_code[    "EHLX"] = "7_9";
   map_code[    "LFTX"] = "7_10";
   map_code[   "4LFTX"] = "7_11";
   map_code[      "RI"] = "7_12";
   map_code[    "LFTX"] = "7_192";
   map_code[   "4LFTX"] = "7_193";
   map_code[      "RI"] = "7_194";
   map_code[    "CWDI"] = "7_195";
   map_code[     "UVI"] = "7_196";
   map_code[    "UPHL"] = "7_197";
   map_code[     "LAI"] = "7_198";
   map_code[  "MXUPHL"] = "7_199";
   map_code[   "AEROT"] = "8_0";
   map_code[    "PMTC"] = "8_192";
   map_code[    "PMTF"] = "8_193";
   map_code[   "LPMTF"] = "8_194";
   map_code[   "LIPMF"] = "8_195";
   map_code[   "TOZNE"] = "9_0";
   map_code[    "O3MR"] = "9_1";
   map_code[   "TCIOZ"] = "9_2";
   map_code[    "O3MR"] = "9_192";
   map_code[   "OZCON"] = "9_193";
   map_code[   "OZCAT"] = "9_194";
   map_code[   "VDFOZ"] = "9_195";
   map_code[     "POZ"] = "9_196";
   map_code[     "TOZ"] = "9_197";
   map_code[    "POZT"] = "9_198";
   map_code[    "POZO"] = "9_199";
   map_code[  "OZMAX1"] = "9_200";
   map_code[  "OZMAX8"] = "9_201";
   map_code[  "PDMAX1"] = "9_202";
   map_code[ "PDMAX24"] = "9_203";
   map_code[   "BSWID"] = "10_0";
   map_code[    "BREF"] = "10_1";
   map_code[   "BRVEL"] = "10_2";
   map_code[   "VERIL"] = "10_3";
   map_code[  "LMAXBR"] = "10_4";
   map_code[    "PREC"] = "10_5";
   map_code[   "RDSP1"] = "10_6";
   map_code[   "RDSP2"] = "10_7";
   map_code[   "RDSP3"] = "10_8";
   map_code[   "REFZR"] = "11_0";
   map_code[   "REFZI"] = "11_1";
   map_code[   "REFZC"] = "11_2";
   map_code[   "RETOP"] = "11_3";
   map_code[    "REFD"] = "11_4";
   map_code[    "REFC"] = "11_5";
   map_code[   "REFZR"] = "11_192";
   map_code[   "REFZI"] = "11_193";
   map_code[   "REFZC"] = "11_194";
   map_code[    "REFD"] = "11_195";
   map_code[    "REFC"] = "11_196";
   map_code[   "RETOP"] = "11_197";
   map_code[  "MAXREF"] = "11_198";
   map_code[    "LTNG"] = "12_192";
   map_code[   "ACCES"] = "13_0";
   map_code[   "ACIOD"] = "13_1";
   map_code[  "ACRADP"] = "13_2";
   map_code[   "GDCES"] = "13_3";
   map_code[   "GDIOD"] = "13_4";
   map_code[  "GDRADP"] = "13_5";
   map_code[  "TIACCP"] = "13_6";
   map_code[  "TIACIP"] = "13_7";
   map_code[  "TIACRP"] = "13_8";
   map_code[     "VIS"] = "14_0";
   map_code[   "ALBDO"] = "14_1";
   map_code[    "TSTM"] = "14_2";
   map_code[   "MIXHT"] = "14_3";
   map_code[  "VOLASH"] = "14_4";
   map_code[    "ICIT"] = "14_5";
   map_code[    "ICIB"] = "14_6";
   map_code[     "ICI"] = "14_7";
   map_code[   "TURBT"] = "14_8";
   map_code[   "TURBB"] = "14_9";
   map_code[    "TURB"] = "14_10";
   map_code[     "TKE"] = "14_11";
   map_code[  "PBLREG"] = "14_12";
   map_code[   "CONTI"] = "14_13";
   map_code[  "CONTET"] = "14_14";
   map_code[   "CONTT"] = "14_15";
   map_code[   "CONTB"] = "14_16";
   map_code[  "MXSALB"] = "14_17";
   map_code[  "SNFALB"] = "14_18";
   map_code[   "SALBD"] = "14_19";
   map_code[    "ICIP"] = "14_20";
   map_code[     "CTP"] = "14_21";
   map_code[     "CAT"] = "14_22";
   map_code[    "SLDP"] = "14_23";
   map_code[  "MXSALB"] = "14_192";
   map_code[  "SNFALB"] = "14_193";
   map_code[  "SRCONO"] = "14_194";
   map_code[  "MRCONO"] = "14_195";
   map_code[  "HRCONO"] = "14_196";
   map_code[ "TORPROB"] = "14_197";
   map_code["HAILPROB"] = "14_198";
   map_code["WINDPROB"] = "14_199";
   map_code["STORPROB"] = "14_200";
   map_code["SHAILPRO"] = "14_201";
   map_code["SWINDPRO"] = "14_202";
   map_code[   "TSTMC"] = "14_203";
   map_code[   "MIXLY"] = "14_204";
   map_code[   "FLGHT"] = "14_205";
   map_code[   "CICEL"] = "14_206";
   map_code[   "CIVIS"] = "14_207";
   map_code[   "CIFLT"] = "14_208";
   map_code[   "LAVNI"] = "14_209";
   map_code[   "HAVNI"] = "14_210";
   map_code[  "SBSALB"] = "14_211";
   map_code[  "SWSALB"] = "14_212";
   map_code[  "NBSALB"] = "14_213";
   map_code[  "NWSALB"] = "14_214";
   map_code[   "PRSVR"] = "14_215";
   map_code["PRSIGSVR"] = "14_216";
   map_code[    "SIPD"] = "14_217";
   map_code[    "EPSR"] = "14_218";
   map_code[    "TPFI"] = "14_219";
   map_code[   "VAFTD"] = "14_232";
   map_code[ "MASSDEN"] = "15_0";
   map_code[   "COLMD"] = "15_1";
   map_code[  "MASSMR"] = "15_2";
   map_code[  "AEMFLX"] = "15_3";
   map_code[ "ANPMFLX"] = "15_4";
   map_code["ANPEMFLX"] = "15_5";
   map_code[ "SDDMFLX"] = "15_6";
   map_code[ "SWDMFLX"] = "15_7";
   map_code[ "AREMFLX"] = "15_8";
   map_code[     "AIA"] = "15_50";
   map_code[  "CONAIR"] = "15_51";
   map_code[    "VMXR"] = "15_52";
   map_code[   "CGPRC"] = "15_53";
   map_code[   "CGDRC"] = "15_54";
   map_code[   "SFLUX"] = "15_55";
   map_code[   "COAIA"] = "15_56";
   map_code[   "TYABA"] = "15_57";
   map_code[   "TYAAL"] = "15_58";
   map_code[   "SADEN"] = "15_100";
   map_code[    "AOTK"] = "15_101";
   map_code[ "NO2TROP"] = "15_131";
   map_code[  "NO2VCD"] = "15_132";
   map_code[  "BROVCD"] = "15_133";
   map_code[ "HCHOVCD"] = "15_134";
   map_code["var190m0"] = "16_0";
   map_code[    "TSEC"] = "16_0";
   map_code[    "NLAT"] = "16_192";
   map_code[    "ELON"] = "16_193";
   map_code[    "TSEC"] = "16_194";
   map_code[   "MLYNO"] = "16_195";
   map_code[   "NLATN"] = "16_196";
   map_code[   "ELONN"] = "16_197";
   map_code[   "COVMZ"] = "17_1";
   map_code[   "COVTZ"] = "17_2";
   map_code[   "COVTM"] = "17_3";
   map_code[   "COVTW"] = "17_4";
   map_code[   "COVZZ"] = "17_5";
   map_code[   "COVMM"] = "17_6";
   map_code[   "COVQZ"] = "17_7";
   map_code[   "COVQM"] = "17_8";
   map_code[  "COVTVV"] = "17_9";
   map_code[  "COVQVV"] = "17_10";
   map_code[ "COVPSPS"] = "17_11";
   map_code[   "COVQQ"] = "17_12";
   map_code[ "COVVVVV"] = "17_13";
   map_code[   "COVTT"] = "17_14";

}
