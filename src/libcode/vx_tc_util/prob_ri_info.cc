// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "prob_ri_info.h"
#include "atcf_offsets.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbRIInfo
//
////////////////////////////////////////////////////////////////////////

ProbRIInfo::ProbRIInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbRIInfo::~ProbRIInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbRIInfo::ProbRIInfo(const ProbRIInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbRIInfo & ProbRIInfo::operator=(const ProbRIInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbRIInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIInfo::clear() {

   ProbInfoBase::clear();

   Value = bad_data_double;
   Initials.clear();
   RIBeg = bad_data_int;
   RIEnd = bad_data_int;

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   ProbInfoBase::dump(out, indent_depth);

   out << prefix << "Value           = "   << Value << "\n";
   out << prefix << "Initials        = \"" << (Initials ? Initials.text() : "(nul)") << "\"\n";
   out << prefix << "RIBeg           = "   << RIBeg << "\n";
   out << prefix << "RIEnd           = "   << RIEnd << "\n";

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIInfo::serialize() const {
   ConcatString s;

   s << ProbInfoBase::serialize()
     << ", ProbRIInfo: "
     << "Value = " << Value
     << ", Initials = \"" << Initials << "\""
     << ", RIBeg = " << RIBeg
     << ", RIEnd = " << RIEnd;

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIInfo::serialize_r(int n, int indent_depth) const {
   ConcatString s;

   s << ProbInfoBase::serialize_r(n, indent_depth);

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbRIInfo::assign(const ProbRIInfo &p) {
   int i;

   clear();

   ProbInfoBase::assign(p);

   Value    = p.Value;
   Initials = p.Initials;
   RIBeg    = p.RIBeg;
   RIEnd    = p.RIEnd;

   return;
}

////////////////////////////////////////////////////////////////////////

int ProbRIInfo::ri_window() const {
   return((is_bad_data(ri_beg()) || is_bad_data(ri_end()) ?
           bad_data_int : ri_end() - ri_beg()));
}

////////////////////////////////////////////////////////////////////////

void ProbRIInfo::initialize(const ATCFProbLine &l) {

   clear();

   ProbInfoBase::initialize(l);

   Value    = parse_int(l.get_item(ProbRIValueOffset));
   Initials =           l.get_item(ProbRIInitialsOffset, false);
   RIBeg    = parse_int(l.get_item(ProbRIBegOffset));
   RIEnd    = parse_int(l.get_item(ProbRIEndOffset));

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbRIInfo::is_match(const ATCFProbLine &l) const {

   if(!ProbInfoBase::is_match(l)) return(false);

   return(Value == parse_int(l.get_item(ProbRIValueOffset)) &&
          RIBeg == parse_int(l.get_item(ProbRIBegOffset))   &&
          RIEnd == parse_int(l.get_item(ProbRIEndOffset)));
}

////////////////////////////////////////////////////////////////////////

bool ProbRIInfo::add(const ATCFProbLine &l, bool check_dup) {

   // Check for duplicates
   if(check_dup) {
      if(has(l)) {
         mlog << Warning
              << "\nProbRIInfo::add(const ATCFProbLine &l, bool check_dup) -> "
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

void ProbRIInfo::set(const TCStatLine &l) {

   ProbInfoBase::set(l);

   // Store column information
   Value    = atof(l.get_item("AWIND_END"));
   Initials = l.get_item("Initials", false);
   RIBeg    = atoi(l.get_item("RI_BEG"));
   RIEnd    = atoi(l.get_item("RI_END"));

   return;
}

////////////////////////////////////////////////////////////////////////
