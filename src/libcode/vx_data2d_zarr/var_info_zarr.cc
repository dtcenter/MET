// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_zarr.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <regex.h>

#include "var_info_zarr.h"

#include "vx_log.h"
#include "vx_util.h"
#include "vx_data2d.h"
#include "vx_config.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

static bool check_set_attr(int);

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoZarr
//
///////////////////////////////////////////////////////////////////////////////

VarInfoZarr::VarInfoZarr() {
   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoZarr::~VarInfoZarr() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoZarr::VarInfoZarr(const VarInfoZarr &f) {
   init_from_scratch();
   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoZarr & VarInfoZarr::operator=(const VarInfoZarr &f) {
   if(this == &f) return *this;
   assign(f);
   return *this;
}

///////////////////////////////////////////////////////////////////////////////

VarInfo *VarInfoZarr::clone() const {
   VarInfoZarr *ret = new VarInfoZarr(*this);
   return (VarInfo *)ret;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoZarr::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoZarr::assign(const VarInfoZarr &v) {

   // First call the parent's assign
   VarInfo::assign(v);

   // TODO: Copy class members here

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoZarr::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // TODO: Initialize class members here

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoZarr::dump(ostream &out) const {

   // TODO: Dump out the class member contents here
   out << "VarInfoZarr::dump():\n";

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoZarr::set_dict(Dictionary & dict, bool do_exit) {

   VarInfo::set_dict(dict);

   // TODO: Parse class members here

   return true;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoZarr::is_precipitation() const {
   return check_set_attr(SetAttrIsPrecipitation);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoZarr::is_specific_humidity() const {
   return check_set_attr(SetAttrIsSpecificHumidity);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoZarr::is_u_wind() const {
   return check_set_attr(SetAttrIsUWind);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoZarr::is_v_wind() const {
   return check_set_attr(SetAttrIsVWind);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoZarr::is_wind_speed() const {
   return check_set_attr(SetAttrIsWindSpeed);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoZarr::is_wind_direction() const {
   return check_set_attr(SetAttrIsWindDirection);
}

////////////////////////////////////////////////////////////////////////

static bool check_set_attr(int attr_val) {
   return (!is_bad_data(attr_val) ?
           attr_val != 0 : false);
}

////////////////////////////////////////////////////////////////////////

