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

#include "vx_cal.h"
#include "is_bad_data.h"

#include "var_info.h"
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
      out << "VarInfo::dump():\n";
   if(MagicStr)
      out << "  MagicStr = " << MagicStr << "\n";
   else
      out << "  MagicStr = (nul)\n";
   if(ReqName)
      out << "  ReqName  = " << ReqName << "\n";
   else
      out << "  ReqName  = (nul)\n";
   if(Name)
      out << "  Name     = " << Name << "\n";
   else
      out << "  Name     = (nul)\n";
   if(LongName)
      out << "  LongName = " << LongName << "\n";
   else
      out << "  LongName = (nul)\n";
   if(Units)
      out << "  Units    = " << Units << "\n";
   else
      out << "  Units    = (nul)\n";
      out << "  PFlag    = " << PFlag << "\n";
   if(PName)
      out << "  PName    = " << PName << "\n";
   else
      out << "  PName    = (nul)\n";
   if(PUnits)
      out << "  PUnits   = " << PUnits << "\n";
   else
      out << "  PUnits   = (nul)\n";
      out << "  VFlag    = " << VFlag << "\n"
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

   // Look for the common keywords.
   if(strcasecmp(key, CONFIG_Init    ) == 0) { Init  = timestring_to_unix(val);      }
   if(strcasecmp(key, CONFIG_Valid   ) == 0) { Valid = timestring_to_unix(val);      }
   if(strcasecmp(key, CONFIG_Lead    ) == 0) { Lead  = timestring_to_sec(val);       }

   return;
}

///////////////////////////////////////////////////////////////////////////////
