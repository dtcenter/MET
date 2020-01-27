// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
//
// Code for class TCStatLine
//
////////////////////////////////////////////////////////////////////////

TCStatLine::TCStatLine() {

   init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

TCStatLine::~TCStatLine() {

   clear();

}

////////////////////////////////////////////////////////////////////////

TCStatLine::TCStatLine(const TCStatLine & L) {

   init_from_scratch();

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

void TCStatLine::init_from_scratch() {

   DataLine::init_from_scratch();

   clear();

   return;

}

////////////////////////////////////////////////////////////////////////

void TCStatLine::assign(const TCStatLine & L) {

   DataLine::assign(L);

   Type    = L.Type;
   HdrLine = L.HdrLine;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatLine::clear() {

   DataLine::clear();

   Type    = NoTCStatLineType;
   HdrLine = (AsciiHeaderLine *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::read_line(LineDataFile * ldf) {
   int status, offset;

   clear();

   status = DataLine::read_line(ldf);

   //
   // Check for bad read status or zero length
   //
   if(!status || n_items() == 0) {
      clear();
      return(0);
   }

   //
   // Check for a header line
   //
   if(strcmp(get_item(0), "VERSION") == 0) {
      Type = TCStatLineType_Header;
      return(1);
   }

   //
   // Determine the LINE_TYPE column offset
   //
   offset = METHdrTable.col_offset(get_item(0), "TCST", na_str, "LINE_TYPE");

   if(is_bad_data(offset) || n_items() < (offset + 1)) {
      Type = NoTCStatLineType;
      return(0);
   }

   //
   // Load the matching header line and store the line type
   //
   HdrLine = METHdrTable.header(get_item(0), "TCST", get_item(offset));

   Type = string_to_tcstatlinetype(get_item(offset));

   return(1);
}

////////////////////////////////////////////////////////////////////////

bool TCStatLine::is_ok() const {
   return(DataLine::is_ok());
}

////////////////////////////////////////////////////////////////////////

bool TCStatLine::is_header() const {
   return(Type == TCStatLineType_Header);
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatLine::get(const char *col_str, bool check_na) const {

  ConcatString cs = (string)get_item(col_str, check_na);

   //
   // If not found, check derivable timing columns
   //
   if( cs == bad_data_str ) {

      if(strcasecmp(col_str, conf_key_init_hour) == 0)
         cs = sec_to_hhmmss(init_hour());
      else if(strcasecmp(col_str, conf_key_valid_hour) == 0)
         cs = sec_to_hhmmss(valid_hour());
   }

   return(cs);
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::get_item(const char *col_str, bool check_na) const {
   int offset = bad_data_int;
   int dim = bad_data_int;

   //
   // Parse the variable length dimension
   //
   if(HdrLine->is_var_length()) {
      dim = atoi(get_item(HdrLine->var_index_offset()));
   }

   //
   // Search for matching header column
   //
   offset = HdrLine->col_offset(col_str, dim);

   //
   // If not found, check extra header columns
   //
   if(is_bad_data(offset)) {
      if(!get_file()->header().has(col_str, offset)) offset = bad_data_int;
   }

   //
   // Return bad data string for no match
   //
   if(is_bad_data(offset)) return(bad_data_str);
   else                    return(get_item(offset, check_na));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::get_item(int offset, bool check_na) const {

   // Range check
   if(offset < 0 || offset >= N_items) return(bad_data_str);

   const char * c = DataLine::get_item(offset);

   // Check for the NA string and interpret it as bad data
   if(check_na && strcmp(c, na_str) == 0) return(bad_data_str);
   else                                   return(c);
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::version() const {
   return(get_item("VERSION", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::amodel() const {
   return(get_item("AMODEL", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::bmodel() const {
   return(get_item("BMODEL", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::desc() const {
   return(get_item("DESC", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::storm_id() const {
   return(get_item("STORM_ID", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::basin() const {
   return(get_item("BASIN", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::cyclone() const {
   return(get_item("CYCLONE", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::storm_name() const {
   return(get_item("STORM_NAME", false));
}

////////////////////////////////////////////////////////////////////////

unixtime TCStatLine::init() const {
   unixtime t;
   const char * c = get_item("INIT");

   t = timestring_to_unix(c);

   return(t);
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::init_hour() const {
   return(init()%sec_per_day);
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::lead() const {
   int s;
   const char * c = get_item("LEAD");

   s = timestring_to_sec(c);

   return(s);
}

////////////////////////////////////////////////////////////////////////

unixtime TCStatLine::valid() const {
   unixtime t;
   const char * c = get_item("VALID");

   t = timestring_to_unix(c);

   return(t);
}

////////////////////////////////////////////////////////////////////////

int TCStatLine::valid_hour() const {
   return(valid()%sec_per_day);
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::init_mask() const {
   return(get_item("INIT_MASK", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::valid_mask() const {
   return(get_item("VALID_MASK", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::initials() const {
   return(get_item("INITIALS", false));
}

////////////////////////////////////////////////////////////////////////

const char * TCStatLine::line_type() const {
   return(get_item("LINE_TYPE", false));
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatLine::header() const {
   ConcatString hdr;
   const char *sep = ":";

   hdr << bmodel() << sep
       << basin() << sep
       << cyclone() << sep
       << (init() > 0 ? unix_to_yyyymmdd_hhmmss(init()).text() : na_str) << sep
       << (!is_bad_data(lead()) ? sec_to_hhmmss(lead()).text() : na_str) << sep
       << (valid() > 0 ? unix_to_yyyymmdd_hhmmss(valid()).text() : na_str);

   return(hdr);
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

TCStatLineType string_to_tcstatlinetype(const char *s) {
   TCStatLineType t;

        if(strcmp(s, TCStatLineType_TCMPR_Str)    == 0) t = TCStatLineType_TCMPR;
   else if(strcmp(s, TCStatLineType_ProbRIRW_Str) == 0) t = TCStatLineType_ProbRIRW;
   else if(strcmp(s, TCStatLineType_Header_Str)   == 0) t = TCStatLineType_Header;
   else                                                 t = NoTCStatLineType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString tcstatlinetype_to_string(const TCStatLineType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case TCStatLineType_TCMPR:    s = TCStatLineType_TCMPR_Str;    break;
      case TCStatLineType_ProbRIRW: s = TCStatLineType_ProbRIRW_Str; break;
      case TCStatLineType_Header:   s = TCStatLineType_Header_Str;   break;
      default:                      s = na_str;                      break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////
