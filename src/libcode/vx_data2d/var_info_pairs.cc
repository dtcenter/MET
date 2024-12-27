// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_pairs.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_pairs.h"

#include "util_constants.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoPairs
//
///////////////////////////////////////////////////////////////////////////////

VarInfoPairs::VarInfoPairs() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoPairs::~VarInfoPairs() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoPairs::VarInfoPairs(const VarInfoPairs &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoPairs & VarInfoPairs::operator=(const VarInfoPairs &f) {

   if ( this == &f )  return *this;

   assign(f);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

VarInfo *VarInfoPairs::clone() const {

   VarInfoPairs *ret = new VarInfoPairs(*this);

   return (VarInfo *)ret;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPairs::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPairs::assign(const VarInfoPairs &v) {

   // First call the parent's assign
   VarInfo::assign(v);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPairs::clear() {

   // First call the parent's clear
   VarInfo::clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPairs::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoPairs::dump():\n";

   VarInfo::dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPairs::set_dict(Dictionary & dict) {
   VarInfo::set_dict(dict);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoPairs::is_precipitation() const {
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsPrecipitation)) {
      return(SetAttrIsPrecipitation != 0);
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoPairs::is_specific_humidity() const {
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsSpecificHumidity)) {
      return(SetAttrIsSpecificHumidity != 0);
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoPairs::is_u_wind() const {
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsUWind)) {
      return(SetAttrIsUWind != 0);
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoPairs::is_v_wind() const {
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsVWind)) {
      return(SetAttrIsVWind != 0);
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoPairs::is_wind_speed() const {
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindSpeed)) {
      return(SetAttrIsWindSpeed != 0);
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoPairs::is_wind_direction() const {
   bool status = false;

   //
   // Check set_attrs entry
   //
   if(!is_bad_data(SetAttrIsWindDirection)) {
      return(SetAttrIsWindDirection != 0);
   }

   return status;
}

///////////////////////////////////////////////////////////////////////////////
