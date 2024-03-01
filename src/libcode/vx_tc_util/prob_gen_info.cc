// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "nav.h"

#include "prob_gen_info.h"
#include "atcf_offsets.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbGenInfo
//
////////////////////////////////////////////////////////////////////////

ProbGenInfo::ProbGenInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbGenInfo::~ProbGenInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbGenInfo::ProbGenInfo(const ProbGenInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbGenInfo & ProbGenInfo::operator=(const ProbGenInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbGenInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbGenInfo::clear() {

   ProbInfoBase::clear();

   Initials.clear();
   GenOrDis.clear();
   GenesisTime = (unixtime) 0;
   GenesisLead = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbGenInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   ProbInfoBase::dump(out, indent_depth);

   out << prefix << "Initials        = \"" << Initials.contents() << "\"\n";
   out << prefix << "GenOrDis        = \"" << GenOrDis.contents() << "\"\n";
   out << prefix << "GenesisTime     = "   << unix_to_yyyymmdd_hhmmss(GenesisTime) << "\n";
   out << prefix << "GenesisLead     = "   << sec_to_hhmmss(GenesisLead) << "\n";

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString ProbGenInfo::serialize() const {
   ConcatString s;

   s << ProbInfoBase::serialize()
     << ", ProbGenInfo: "
     << "Initials = \"" << Initials << "\""
     << ", GenOrDis = \"" << GenOrDis << "\""
     << ", GenesisTime = " << unix_to_yyyymmdd_hhmmss(GenesisTime)
     << ", GenesisLead = " << sec_to_hhmmss(GenesisLead) << "\n";

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbGenInfo::serialize_r(int n, int indent_depth) const {
   ConcatString s;

   s << ProbInfoBase::serialize_r(n, indent_depth);

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbGenInfo::assign(const ProbGenInfo &p) {

   clear();

   ProbInfoBase::assign(p);

   Initials    = p.Initials;
   GenOrDis    = p.GenOrDis;
   GenesisTime = p.GenesisTime;
   GenesisLead = p.GenesisLead;

   return;
}

////////////////////////////////////////////////////////////////////////

int ProbGenInfo::genesis_fhr() const {
   return(is_bad_data(GenesisLead) ?
          bad_data_int :
          nint((double) GenesisLead/sec_per_hour));
}

////////////////////////////////////////////////////////////////////////

void ProbGenInfo::initialize(const ATCFProbLine &l, double dland) {

   clear();

   ProbInfoBase::initialize(l, dland);

   Initials = l.get_item(ProbGenInitialsOffset);
   GenOrDis = l.get_item(ProbGenOrDisOffset);

   // Store an empty string as unixtime 0
   GenesisTime  = (l.get_item(ProbGenTimeOffset).empty() ?
                   (unixtime) 0 :
                   parse_time(l.get_item(ProbGenTimeOffset).c_str()));
   GenesisLead = (GenesisTime == 0 ? bad_data_int :
                  GenesisTime - InitTime);

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbGenInfo::is_match(const ATCFProbLine &l) const {

   if(!ProbInfoBase::is_match(l)) return(false);

   unixtime gen_ut = (l.get_item(ProbGenTimeOffset).empty() ?
                      (unixtime) 0 :
                      parse_time(l.get_item(ProbGenTimeOffset).c_str()));

   return(GenesisTime == gen_ut);
}

////////////////////////////////////////////////////////////////////////

bool ProbGenInfo::add(const ATCFProbLine &l, double dland, bool check_dup) {

   // Check for duplicates
   if(check_dup) {
      if(has(l)) {
         mlog << Warning
              << "\nProbGenInfo::add(const ATCFProbLine &l, bool check_dup) -> "
              << "skipping duplicate ATCF line:\n"
              << l.get_line() << "\n\n";
         return(false);
      }
   }

   // Initialize the header information, if necessary
   if(Type == NoATCFLineType) initialize(l, dland);

   // Check for matching header information
   if(!is_match(l)) return(false);

   // Add probability information
   NProb++;
   Prob.add(l.prob());
   ProbItem.add(l.prob_item());

   // Store the ATCFProbLine that was just added
   if(check_dup) ProbLines.add(l.get_line());

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool ProbGenInfo::is_match(const TrackPoint &p, const double rad,
                           const int beg, const int end) const {

   // Check for matching in time and space
   return(p.valid() >= (GenesisTime + beg) &&
          p.valid() <= (GenesisTime + end) &&
          gc_dist(Lat, Lon, p.lat(), p.lon()) <= rad);
}

////////////////////////////////////////////////////////////////////////

bool ProbGenInfo::is_match(const GenesisInfo &gi, const double rad,
                           const int beg, const int end) const {

   // Input genesis point
   const TrackPoint *p = gi.genesis();

   return(p ? is_match(*p, rad, beg, end) : false);
}

////////////////////////////////////////////////////////////////////////
