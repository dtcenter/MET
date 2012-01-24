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
#include <stdlib.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_grib2.h"

#include "math_constants.h"
#include "vx_log.h"

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

   // This functionality is not supported for GRIB2.
   mlog << Error << "\nVarInfoGrib2::set_magic() -> "
        << "This function is not supported for the GRIB2 file type!\n\n";
   exit(1);

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_precipitation() const {

   // This functionality is not supported for GRIB2.
   mlog << Error << "\nVarInfoGrib2::is_precipitation() -> "
        << "This function is not yet implemented for the GRIB2 file type!\n\n";
   exit(1);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_specific_humidity() const {

   // This functionality is not supported for GRIB2.
   mlog << Error << "\nVarInfoGrib2::is_specific_humidity() -> "
        << "This function is not yet implemented for the GRIB2 file type!\n\n";
   exit(1);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_u_wind() const {

   // This functionality is not supported for GRIB2.
   mlog << Error << "\nVarInfoGrib2::is_u_wind() -> "
        << "This function is not yet implemented for the GRIB2 file type!\n\n";
   exit(1);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_v_wind() const {

   // This functionality is not supported for GRIB2.
   mlog << Error << "\nVarInfoGrib2::is_v_wind() -> "
        << "This function is not yet implemented for the GRIB2 file type!\n\n";
   exit(1);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_wind_speed() const {

   // This functionality is not supported for GRIB2.
   mlog << Error << "\nVarInfoGrib2::is_wind_speed() -> "
        << "This function is not yet implemented for the GRIB2 file type!\n\n";
   exit(1);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib2::is_wind_direction() const {

   // This functionality is not supported for GRIB2.
   mlog << Error << "\nVarInfoGrib2::is_wind_direction() -> "
        << "This function is not yet implemented for the GRIB2 file type!\n\n";
   exit(1);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////
