// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "prob_info_base.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbInfoBase
//
////////////////////////////////////////////////////////////////////////

ProbInfoBase::ProbInfoBase() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbInfoBase::~ProbInfoBase() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbInfoBase::ProbInfoBase(const ProbInfoBase & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbInfoBase & ProbInfoBase::operator=(const ProbInfoBase & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbInfoBase::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbInfoBase::clear() {

   Type = NoATCFLineType;
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   Technique.clear();
   InitTime = (unixtime) 0;
   ValidTime = (unixtime) 0;
   Lat = bad_data_double;
   Lon = bad_data_double;
   NProb = 0;
   Prob.clear();
   ProbItem.clear();
   ProbLines.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbInfoBase::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "Type            = \"" << atcflinetype_to_string(Type) << "\"\n";
   out << prefix << "StormId         = \"" << StormId.contents() << "\"\n";
   out << prefix << "Basin           = \"" << Basin.contents() << "\"\n";
   out << prefix << "Cyclone         = \"" << Cyclone.contents() << "\"\n";
   out << prefix << "Technique       = \"" << Technique.contents() << "\"\n";
   out << prefix << "InitTime        = \"" << (InitTime > 0 ? unix_to_yyyymmdd_hhmmss(InitTime).text() : na_str) << "\"\n";
   out << prefix << "ValidTime       = \"" << (ValidTime > 0 ? unix_to_yyyymmdd_hhmmss(ValidTime).text() : na_str) << "\"\n";
   out << prefix << "Lat             = "   << Lat << "\n";
   out << prefix << "Lon             = "   << Lon << "\n";
   out << prefix << "NProb           = "   << NProb << "\n";
   out << prefix << "Prob:"                << "\n";
   Prob.dump(out, indent_depth+1);
   out << prefix << "ProbItem:"            << "\n";
   ProbItem.dump(out, indent_depth+1);
   out << prefix << "ProbLines:"           << "\n";
   ProbLines.dump(out, indent_depth+1);

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString ProbInfoBase::serialize() const {
   ConcatString s;

   s << "ProbInfoBase: "
     << "Type = \"" << atcflinetype_to_string(Type) << "\""
     << ", StormId = \"" << StormId.contents() << "\""
     << ", Basin = \"" << Basin.contents() << "\""
     << ", Cyclone = \"" << Cyclone.contents() << "\""
     << ", Technique = \"" << Technique.contents() << "\""
     << ", InitTime = \"" << (InitTime > 0 ? unix_to_yyyymmdd_hhmmss(InitTime).text() : na_str) << "\""
     << ", ValidTime = \"" << (ValidTime > 0 ? unix_to_yyyymmdd_hhmmss(ValidTime).text() : na_str) << "\""
     << ", Lat = " << Lat
     << ", Lon = " << Lon
     << ", NProb = " << NProb;

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbInfoBase::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth), prefix2(indent_depth+1);
   ConcatString s;
   int i;

   s << prefix << "[" << n << "] " << serialize() << ", Probs:\n";

   for(i=0; i<NProb; i++) {
      s << prefix2
        << "[" << i+1 << "] " << Prob[i]
        << "% probability for " << ProbItem[i] << "\n";
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbInfoBase::assign(const ProbInfoBase &t) {

   clear();

   Type       = t.Type;
   StormId    = t.StormId;
   Basin      = t.Basin;
   Cyclone    = t.Cyclone;
   Technique  = t.Technique;
   InitTime   = t.InitTime;
   ValidTime  = t.ValidTime;
   Lat        = t.Lat;
   Lon        = t.Lon;
   NProb      = t.NProb;
   Prob       = t.Prob;
   ProbItem   = t.ProbItem;
   ProbLines  = t.ProbLines;

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbInfoBase::initialize(const ATCFProbLine &l) {

   clear();

   // Initialize the storm, location, and timing information.
   Type      = l.type();
   StormId   = l.storm_id();
   Basin     = l.basin();
   Cyclone   = l.cyclone_number();
   Technique = l.technique();
   InitTime  = l.warning_time();
   ValidTime = l.valid();
   Lat       = l.lat();
   Lon       = l.lon();

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbInfoBase::is_match(const ATCFProbLine &l) const {

   return(Type      == l.type() &&
          StormId   == l.storm_id() &&
          Basin     == l.basin() &&
          Cyclone   == l.cyclone_number() &&
          Technique == l.technique() &&
          InitTime  == l.warning_time() &&
          ValidTime == l.valid() &&
          Lat       == l.lat() &&
          Lon       == l.lon());
}

////////////////////////////////////////////////////////////////////////

bool ProbInfoBase::is_match(const TrackInfo &t) const {
   bool match = true;

   // Check if track is for the same basin and storm with overlapping
   // valid times.
   if(Basin        != t.basin()     ||
      Cyclone      != t.cyclone()   ||
      ValidTime     > t.valid_max() ||
      ValidTime     < t.valid_min())
      match = false;

   // Check that technique is defined
   if(Technique == "" || t.technique() == "") return(false);

   // Check that init times match for non-BEST, non-analysis tracks
   if(!t.is_best_track() &&
      !t.is_anly_track() &&
      InitTime != t.init()) {
      match = false;
   }

   return(match);
}

////////////////////////////////////////////////////////////////////////

bool ProbInfoBase::add(const ATCFProbLine &l, bool check_dup) {

   // Check for duplicates
   if(check_dup) {
      if(has(l)) {
         mlog << Warning
              << "\nProbInfoBase::add(const ATCFProbLine &l, bool check_dup) -> "
              << "skipping duplicate ATCFProbLine:\n"
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

void ProbInfoBase::set(const TCStatLine &l) {
   ConcatString cs;

   clear();

   // Store column information
   switch(l.type()) {

      case TCStatLineType_ProbRIRW:
         Type = ATCFLineType_ProbRIRW;
         break;

      default:
         mlog << Error << "\n\nvoid ProbInfoBase::set(const TCStatLine &l)-> "
              << "unexpected TCStatLineType \""
              << tcstatlinetype_to_string(l.type()) << "\"\n\n";
   }

   StormId = l.storm_id();
   Basin = l.basin();
   Cyclone = l.cyclone();
   Technique = l.amodel();
   InitTime = l.init();
   ValidTime = l.valid();
   Lat = atof(l.get_item("ALAT"));
   Lon = atof(l.get_item("ALON"));
   NProb = atoi(l.get_item("N_THRESH"));
   for(int i=1; i<=NProb; i++) {
      cs << cs_erase << "PROB_" << i;
      Prob.add(atof(l.get_item(cs.c_str())));
      cs << cs_erase << "THRESH_" << i;
      ProbItem.add(atof(l.get_item(cs.c_str())));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbInfoBase::has(const ATCFProbLine &l) const {
   return(ProbLines.has(l.get_line()));
}

////////////////////////////////////////////////////////////////////////
