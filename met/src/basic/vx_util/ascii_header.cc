// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"

#include "ascii_header.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class AsciiHeader
//
////////////////////////////////////////////////////////////////////////

AsciiHeader::AsciiHeader() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

AsciiHeader::~AsciiHeader() {
   clear();
}

////////////////////////////////////////////////////////////////////////

AsciiHeader::AsciiHeader(const AsciiHeader &a) {
   init_from_scratch();
   assign(a);
}

////////////////////////////////////////////////////////////////////////

AsciiHeader::AsciiHeader(const char *version, const char *hdr, const char *data) {
   init_from_scratch();
   set(version, hdr, data);
}


////////////////////////////////////////////////////////////////////////

AsciiHeader & AsciiHeader::operator=(const AsciiHeader & a) {

   if(this == &a) return(*this);
   assign(a);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::clear() {

   Version.clear();
   HdrCols.clear();
   DataCols.clear();

   NHdrCols  = 0;
   NDataCols = 0;

   HdrCols.set_ignore_case(true);
   DataCols.set_ignore_case(true);

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::set(const char *version, const char *hdr, const char *data) {

   clear();

   set_version(version);
   set_hdr(hdr);
   set_data(data);

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::set_version(const char *version) {

   Version = version;

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::set_hdr(const char *hdr) {

   HdrCols.parse_wsss(hdr);

   NHdrCols = HdrCols.n_elements();

   return;
}

////////////////////////////////////////////////////////////////////////

void AsciiHeader::set_data(const char *data) {

   DataCols.parse_wsss(data);

   NDataCols = DataCols.n_elements();

   return;
}

////////////////////////////////////////////////////////////////////////

const char *AsciiHeader::col_name(int i) const {

   // Check range
   if(i < 0 || i >= NHdrCols + NDataCols) {
     mlog << Error << "\nAsciiHeader::col_name() -> "
          << "range check error!\n\n";
     exit(1);
   }

   if(i<NHdrCols) return(HdrCols[i]);
   else           return(DataCols[i-NHdrCols]);

   return(na_str);
}

////////////////////////////////////////////////////////////////////////

int AsciiHeader::col_index(const char *col_name) const {
   int i;

   // Check the header columns and then the data columns

   if(HdrCols.has(col_name, i))  return(i);

   if(DataCols.has(col_name, i)) return(i);

   mlog << Error << "\nAsciiHeader::col_index() -> "
        << "cannot find \"" << col_name << "\" header column.\n\n";
   exit(1);

   return(i);
}

////////////////////////////////////////////////////////////////////////
