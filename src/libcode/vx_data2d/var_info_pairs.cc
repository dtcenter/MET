// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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

bool VarInfoPairs::set_dict(Dictionary & dict, bool do_exit) {

   bool status = VarInfo::set_dict(dict, do_exit);

   //
   // Parse the required name and optional level strings
   //
   ConcatString nstr = dict.lookup_string(conf_key_name);
   ReqName = nstr;
   Name = nstr;

   ConcatString lstr = dict.lookup_string(conf_key_level, false);
   Level.set_req_name(lstr.c_str());
   Level.set_name(lstr.c_str());

   VarInfo::set_magic(nstr, lstr);

   return status;
}

///////////////////////////////////////////////////////////////////////////////
