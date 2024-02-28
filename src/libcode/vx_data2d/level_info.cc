// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   level_info.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <map>

#include "is_bad_data.h"

#include "level_info.h"
#include "leveltype_to_string.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class LevelInfo
//
///////////////////////////////////////////////////////////////////////////////

LevelInfo::LevelInfo() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

LevelInfo::~LevelInfo() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

LevelInfo::LevelInfo(const LevelInfo &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

LevelInfo & LevelInfo::operator=(const LevelInfo &f) {

   if ( this == &f )  return *this;

   assign(f);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::init_from_scratch() {

   clear();

}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::assign(const LevelInfo &l) {

   clear();

   // Copy
   Type    = l.type();
   TypeNum = l.type_num();
   ReqName = l.req_name();
   Name    = l.name();
   Units   = l.units();
   Upper   = l.upper();
   Lower   = l.lower();
   Increment = l.increment();
   Is_offset = l.is_offset();

}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::clear() {

   // Initialize
   Type = LevelType_None;
   TypeNum = bad_data_int;
   ReqName.clear();
   Name.clear();
   Units.clear();
   Upper  = 0.0;
   Lower  = 0.0;
   Increment = 0.0;
   Is_offset = true;

}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::dump(ostream &out) const {

   // Dump out the contents
   out << "LevelInfo::dump():\n"
       << "  Type      = " << leveltype_to_string(Type) << "\n"
       << "  TypeNum   = " << TypeNum << "\n"
       << "  ReqName   = " << ReqName.contents() << "\n"
       << "  Name      = " << Name.contents() << "\n"
       << "  Units     = " << Units.contents() << "\n"
       << "  Upper     = " << Upper << "\n"
       << "  Increment = " << Increment << "\n";

}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_type(LevelType lt) {
   Type = lt;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_type_num(int i) {
   TypeNum = i;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_req_name(const char *str) {
   ReqName = str;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_name(const char *str) {
   Name = str;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_units(const char *str) {
   Units = str;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_upper(double u) {
   Upper = u;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_lower(double l) {
   Lower = l;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_range(double l, double u) {
   Lower = l;
   Upper = u;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_increment(double i) {
   Increment = i;
}

///////////////////////////////////////////////////////////////////////////////

void LevelInfo::set_is_offset(bool b) {
   Is_offset = b;
}

///////////////////////////////////////////////////////////////////////////////
