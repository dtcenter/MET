// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_grib.cc
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
#include <string.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_python.h"

#include "util_constants.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"
#include "grdfiletype_to_string.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoPython
//
///////////////////////////////////////////////////////////////////////////////

VarInfoPython::VarInfoPython() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoPython::~VarInfoPython() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoPython::VarInfoPython(const VarInfoPython &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoPython & VarInfoPython::operator=(const VarInfoPython &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPython::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();


   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPython::assign(const VarInfoPython &v) {

   // First call the parent's assign
   VarInfo::assign(v);

   Type = v.Type;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPython::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   Type = FileType_None;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoPython::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoPython::dump():\n"
       << "  Type    = " << grdfiletype_to_string(Type)  << "\n";

   return;
}


///////////////////////////////////////////////////////////////////////////////


void VarInfoPython::set_file_type(const GrdFileType t)

{

   if ( (t == FileType_Python_Numpy) || (t == FileType_Python_Xarray) ) {

      Type = t;

      return;

   }

   mlog << Error
        << "VarInfoPython::set_file_type(const GrdFileType) -> bad type ... "
        << grdfiletype_to_string(t) << "\n\n";

   return;
}


///////////////////////////////////////////////////////////////////////////////


void VarInfoPython::set_magic(const ConcatString &nstr, const ConcatString &lstr) {

   // Validate the magic_string
   VarInfo::set_magic(nstr, lstr);

   // Store the magic string
   MagicStr << cs_erase << nstr << "/" << lstr;

   return;
}


///////////////////////////////////////////////////////////////////////////////


void VarInfoPython::set_dict(Dictionary & dict) {

   VarInfo::set_dict(dict);

   //
   //  the "name" entry is required and specifies the python command to be run
   //  store it as the ReqName
   //

   ReqName = dict.lookup_string(conf_key_name, true);

   //
   //  check for a probability boolean setting
   //

   if(dict.lookup_bool(conf_key_prob, false)) {
      set_p_flag(true);
      return;
   }

   return;
}


///////////////////////////////////////////////////////////////////////////////
