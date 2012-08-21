// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <cstdio>
#include <cmath>

#include "tc_stat_line.h"
#include "tc_columns.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static const char *TCStatLineType_TCMPR_Str = "TCMPR";

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatLine
//
////////////////////////////////////////////////////////////////////////

TCStatLine::TCStatLine() {

}

////////////////////////////////////////////////////////////////////////

TCStatLine::~TCStatLine() {

}

////////////////////////////////////////////////////////////////////////

TCStatLine::TCStatLine(const TCStatLine & L) {

   assign(L);

   return;
}

////////////////////////////////////////////////////////////////////////

TCStatLine & TCStatLine::operator=(const TCStatLine & L) {

   if(this == &L) return(*this);

   assign(L);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCStatLine::assign(const TCStatLine & L) {

   DataLine::assign(L);

   Type = L.Type;

   return;
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::read_line(LineDataFile * ldf) {
   int status;

   status = DataLine::read_line(ldf);

   if(!status) {

      clear();

      Type = NoTCStatLineType;

      return(0);
   }

   determine_line_type();

   return(1);
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::is_ok() const {

   if(!(DataLine::is_ok())) return(0);

   // Get the first item in the line
   const char *c = get_item(0);
   
   // Check if this is a header line   
   if(strcmp(c, "VERSION") == 0) return(0);

   return(1);
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::get_item(int k) const {

   const char * c = DataLine::get_item(k);

   // Check for the NA string and interpret it as bad data
   if(strcmp(c, na_str) == 0) return(bad_data_str);
   else                       return(c);
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::get_item(const char * col_name) const {
   int offset = determine_column_offset(Type, col_name);

   return(get_item(offset));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::version() const {
   return(get_item("VERSION"));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::amodel() const {
   return(get_item("AMODEL"));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::bmodel() const {
   return(get_item("BMODEL"));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::basin() const {
   return(get_item("BASIN"));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::cyclone() const {
   return(get_item("CYCLONE"));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::storm_name() const {
   return(get_item("STORM_NAME"));
}

////////////////////////////////////////////////////////////////////////

unixtime TCStatLine::init() const {
   const char * c = get_item("INIT");
   unixtime ut;

   if(strcmp(c, bad_data_str) == 0) ut = (unixtime) 0;
   else                             ut = timestring_to_unix(c);

   return(ut);
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::init_hour() const {
   return(init()%sec_per_day);
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::lead() const {
   const char * c = get_item("LEAD");
   int s;

   if(strcmp(c, bad_data_str) == 0) s = bad_data_int;
   else                             s = timestring_to_sec(c);

   return(s);
}

////////////////////////////////////////////////////////////////////////

unixtime TCStatLine::valid() const {
   const char * c = get_item("VALID");
   unixtime ut;

   if(strcmp(c, bad_data_str) == 0) ut = (unixtime) 0;
   else                             ut = timestring_to_unix(c);

   return(ut);
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::valid_hour() const {
   return(valid()%sec_per_day);
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::init_mask() const {
   return(get_item("INIT_MASK"));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::valid_mask() const {
   return(get_item("VALID_MASK"));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::line_type() const {
   int offset = get_tc_col_offset(tc_header_cols, n_tc_header_cols,
                                  "LINE_TYPE");

   return(get_item(offset));
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatLine::header() const {
   ConcatString hdr;
   const char *sep = ":";

   hdr << bmodel()                         << sep
       << basin()                          << sep
       << cyclone()                        << sep
       << unix_to_yyyymmdd_hhmmss(init())  << sep
       << sec_to_hhmmss(lead())            << sep
       << unix_to_yyyymmdd_hhmmss(valid());

   return(hdr);
}

////////////////////////////////////////////////////////////////////////

void TCStatLine::determine_line_type() {
   const char *c = line_type();

   Type = string_to_tcstatlinetype(c);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

TCStatLineType string_to_tcstatlinetype(const char *s) {
   TCStatLineType t;

   if(strcmp(s, TCStatLineType_TCMPR_Str) == 0) t = TCStatLineType_TCMPR;
   else                                         t = NoTCStatLineType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString tcstatlinetype_to_string(const TCStatLineType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case TCStatLineType_TCMPR: s = TCStatLineType_TCMPR_Str; break;
      default:                   s = na_str;                   break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

int determine_column_offset(TCStatLineType type, const char *c) {
   int offset;

   switch(type) {

      case TCStatLineType_TCMPR:
         offset = get_tc_mpr_col_offset(c);
         break;

      default:
         mlog << Error << "\ndetermine_column_offset() -> "
              << "unexpected line type value of " << type << "\n\n";
         exit(1);
         break;
   };

   return(offset);
}

////////////////////////////////////////////////////////////////////////
