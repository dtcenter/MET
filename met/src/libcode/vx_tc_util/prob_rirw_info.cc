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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "prob_rirw_info.h"
#include "atcf_offsets.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbRIRWInfo
//
////////////////////////////////////////////////////////////////////////

ProbRIRWInfo::ProbRIRWInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbRIRWInfo::~ProbRIRWInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbRIRWInfo::ProbRIRWInfo(const ProbRIRWInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbRIRWInfo & ProbRIRWInfo::operator=(const ProbRIRWInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWInfo::clear() {

   ProbInfoBase::clear();

   Value = bad_data_double;
   Initials.clear();
   RIRWBeg = bad_data_int;
   RIRWEnd = bad_data_int;

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   ProbInfoBase::dump(out, indent_depth);

   out << prefix << "Value           = "   << Value << "\n";
   out << prefix << "Initials        = \"" << Initials.contents() << "\"\n";
   out << prefix << "RIRWBeg         = "   << RIRWBeg << "\n";
   out << prefix << "RIRWEnd         = "   << RIRWEnd << "\n";

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIRWInfo::serialize() const {
   ConcatString s;

   s << ProbInfoBase::serialize()
     << ", ProbRIRWInfo: "
     << "Value = " << Value
     << ", Initials = \"" << Initials << "\""
     << ", RIRWBeg = " << RIRWBeg
     << ", RIRWEnd = " << RIRWEnd;

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIRWInfo::serialize_r(int n, int indent_depth) const {
   ConcatString s;

   s << ProbInfoBase::serialize_r(n, indent_depth);

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWInfo::assign(const ProbRIRWInfo &p) {

   clear();

   ProbInfoBase::assign(p);

   Value    = p.Value;
   Initials = p.Initials;
   RIRWBeg  = p.RIRWBeg;
   RIRWEnd  = p.RIRWEnd;

   return;
}

////////////////////////////////////////////////////////////////////////

int ProbRIRWInfo::rirw_window() const {
   return((is_bad_data(rirw_beg()) || is_bad_data(rirw_end()) ?
           bad_data_int : rirw_end() - rirw_beg()));
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWInfo::initialize(const ATCFProbLine &l) {

   clear();

   ProbInfoBase::initialize(l);

   Value    = parse_int(l.get_item(ProbRIRWValueOffset).c_str());
   Initials =           l.get_item(ProbRIRWInitialsOffset);
   RIRWBeg  = parse_int(l.get_item(ProbRIRWBegOffset).c_str());
   RIRWEnd  = parse_int(l.get_item(ProbRIRWEndOffset).c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbRIRWInfo::is_match(const ATCFProbLine &l) const {

   if(!ProbInfoBase::is_match(l)) return(false);

   return(Value   == parse_int(l.get_item(ProbRIRWValueOffset).c_str()) &&
          RIRWBeg == parse_int(l.get_item(ProbRIRWBegOffset).c_str())   &&
          RIRWEnd == parse_int(l.get_item(ProbRIRWEndOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////

bool ProbRIRWInfo::add(const ATCFProbLine &l, bool check_dup) {

   // Check for duplicates
   if(check_dup) {
      if(has(l)) {
         mlog << Warning
              << "\nProbRIRWInfo::add(const ATCFProbLine &l, bool check_dup) -> "
              << "skipping duplicate ATCF line:\n"
              << l.get_line() << "\n\n";
         return(false);
      }
   }

   // Initialize the header information, if necessary
   if(Type == NoATCFLineType) initialize(l);

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

void ProbRIRWInfo::set(const TCStatLine &l) {

   ProbInfoBase::set(l);

   // Store column information
   Value    = atof(l.get_item("AWIND_END"));
   Initials = l.get_item("Initials", false);
   RIRWBeg  = atoi(l.get_item("RIRW_BEG"));
   RIRWEnd  = atoi(l.get_item("RIRW_END"));

   return;
}

////////////////////////////////////////////////////////////////////////
