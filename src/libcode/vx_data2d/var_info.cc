// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
 
void VarInfo::set_pair(const ConcatString &key, const ConcatString &val) {

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
