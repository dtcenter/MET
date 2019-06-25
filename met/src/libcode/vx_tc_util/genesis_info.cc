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

#include "math_constants.h"

#include "genesis_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class GenesisInfo
//
////////////////////////////////////////////////////////////////////////

GenesisInfo::GenesisInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

GenesisInfo::~GenesisInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

GenesisInfo::GenesisInfo(const GenesisInfo & t) {

   clear();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

GenesisInfo & GenesisInfo::operator=(const GenesisInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::clear() {

   IsSet       = false;
   IsBestTrack = false;

   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   TechniqueNumber = bad_data_int;
   Technique.clear();
   Initials.clear();

   GenTime         = (unixtime) 0;
   InitTime        = (unixtime) 0;
   LeadTime        = bad_data_int;

   Lat             = bad_data_double;
   Lon             = bad_data_double;
   DLand           = bad_data_double;

   NPoints         = 0;
   MinValidTime    = (unixtime) 0;
   MaxValidTime    = (unixtime) 0;
   MinWarmCoreTime = (unixtime) 0;
   MaxWarmCoreTime = (unixtime) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::assign(const GenesisInfo &t) {
   int i;

   clear();

   IsSet           = true;
   IsBestTrack     = t.IsBestTrack;

   StormId         = t.StormId;
   Basin           = t.Basin;
   Cyclone         = t.Cyclone;
   TechniqueNumber = t.TechniqueNumber;
   Technique       = t.Technique;
   Initials        = t.Initials;

   GenTime         = t.GenTime;
   InitTime        = t.InitTime;
   LeadTime        = t.LeadTime;

   Lat             = t.Lat;
   Lon             = t.Lon;
   DLand           = t.DLand;

   NPoints         = t.NPoints;
   MinValidTime    = t.MinValidTime;
   MaxValidTime    = t.MaxValidTime;
   MinWarmCoreTime = t.MinWarmCoreTime;
   MaxWarmCoreTime = t.MaxWarmCoreTime;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "IsSet           = " << bool_to_string(IsSet) << "\n";
   out << prefix << "IsBestTrack     = " << bool_to_string(IsBestTrack) << "\n";
   out << prefix << "StormId         = \"" << StormId.contents() << "\"\n";
   out << prefix << "Basin           = \"" << Basin.contents() << "\"\n";
   out << prefix << "Cyclone         = \"" << Cyclone.contents() << "\"\n";
   out << prefix << "TechniqueNumber = " << TechniqueNumber << "\n";
   out << prefix << "Technique       = \"" << Technique.contents() << "\"\n";
   out << prefix << "Initials        = \"" << Initials.contents() << "\"\n";
   out << prefix << "GenTime         = \""
                 << (GenTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(GenTime).text() :
                     na_str) << "\"\n";
   out << prefix << "InitTime        = \""
                 << (InitTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(InitTime).text() :
                     na_str) << "\"\n";
   out << prefix << "LeadTime        = \""
                 << sec_to_hhmmss(LeadTime).text() << "\"\n";
   out << prefix << "Lat             = " << Lat << "\n";
   out << prefix << "Lon             = " << Lon << "\n";
   out << prefix << "DLand           = " << DLand << "\n";
   out << prefix << "NPoints         = " << NPoints << "\n";
   out << prefix << "MinValidTime    = \""
                 << (MinValidTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MinValidTime).text() :
                     na_str) << "\"\n";
   out << prefix << "MaxValidTime    = \""
                 << (MaxValidTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MaxValidTime).text() :
                     na_str) << "\"\n";
   out << prefix << "MinWarmCoreTime = \""
                 << (MinWarmCoreTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MinWarmCoreTime).text() :
                     na_str) << "\"\n";
   out << prefix << "MaxWarmCoreTime = \""
                 << (MaxWarmCoreTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MaxWarmCoreTime).text() :
                     na_str) << "\"\n";

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::serialize() const {
   ConcatString s;

   s << "GenesisInfo: "
     << "IsSet = " << bool_to_string(IsSet)
     << ", IsBestTrack = " << bool_to_string(IsBestTrack)
     << ", StormId = \"" << StormId.contents() << "\""
     << ", Basin = \"" << Basin.contents() << "\""
     << ", Cyclone = \"" << Cyclone.contents() << "\""
     << ", TechniqueNumber = " << TechniqueNumber
     << ", Technique = \"" << Technique.contents() << "\""
     << ", Initials = \"" << Initials.contents() << "\""
     << ", GenTime = \"" << (GenTime > 0 ?
           unix_to_yyyymmdd_hhmmss(GenTime).text() : na_str) << "\""
     << ", InitTime = \"" << (InitTime > 0 ?
           unix_to_yyyymmdd_hhmmss(InitTime).text() : na_str) << "\""
     << ", LeadTime = \"" << sec_to_hhmmss(LeadTime).text() << "\""
     << ", Lat = " << Lat
     << ", Lon = " << Lon
     << ", DLand = " << DLand
     << ", NPoints = " << NPoints
     << ", MinValidTime = \"" << (MinValidTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MinValidTime).text() : na_str) << "\""
     << ", MaxValidTime = \"" << (MaxValidTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MaxValidTime).text() : na_str) << "\""
     << ", MinWarmCoreTime = \"" << (MinWarmCoreTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MinWarmCoreTime).text() : na_str) << "\""
     << ", MaxWarmCoreTime = \"" << (MaxWarmCoreTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MaxWarmCoreTime).text() : na_str) << "\"";

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << "[" << n << "] " << serialize();

   return(s);

}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::initialize(const ATCFTrackLine &l) {

   IsSet           = true;
   IsBestTrack     = l.is_best_track();

   Basin           = l.basin();
   Cyclone         = l.cyclone_number();
   TechniqueNumber = l.technique_number();
   Technique       = l.technique();
   Initials        = l.initials();

   // For BEST tracks, keep InitTime = LeadTime = 0.
   if(IsBestTrack) {
      InitTime = (unixtime) 0;
      LeadTime = 0;
   }
   else {
      // JHG start working here!
      set_init(l.warning_time());
   }

   // Set the valid time range
   MinValidTime = MaxValidTime = l.valid();

   // Create the storm id
   set_storm_id();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::set_storm_id() {

   StormId = define_storm_id(InitTime, MinValidTime, MaxValidTime,
                             Basin, Cyclone);

   return;
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfo::add(const ATCFTrackLine &l) {
// JHG
   bool found = false;
   bool status = false;
   int i;

   // Initialize GenesisInfo with ATCFTrackLine, if necessary
   if(!IsSet) initialize(l);

   // Otherwise, check if TrackPoint doesn't match this GenesisInfo
   else if(!is_match(l)) return(false);

   // Update min/max valid times
   if(MinValidTime == 0 || l.valid() < MinValidTime) {
      MinValidTime = l.valid();
   }
   if(MaxValidTime == 0 || l.valid() > MaxValidTime) {
      MaxValidTime = l.valid();
   }
/* JHG
   // Update min/max warm core times
   if(l.is_warm_core()) {
      if(MinWarmCoreTime == 0 || l.valid() < MinWarmCoreTime) {
         MinWarmCoreTime = l.valid();
      }
      if(MaxWarmCoreTime == 0 || l.valid() < MaxWarmCoreTime) {
         MaxWarmCoreTime = l.valid();
      }
   }
*/
   return(true);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfo::is_match(const ATCFTrackLine &l) {
   bool match = true;
   int diff;
/* JHG
   // Make sure the technique is defined.
   if(Technique.empty()) return(false);

   // Make sure the basin, cyclone, and technique stay constant.
   if(Basin     != l.basin() ||
      Cyclone   != l.cyclone_number() ||
      Technique != l.technique()) return(false);

   // Check for an analysis track where the technique number matches,
   // the lead time remains zero, and the valid time changes.
   if(CheckAnly && !IsBestTrack && !IsAnlyTrack && NPoints > 0) {

      if(TechniqueNumber          == l.technique_number() &&
         Point[NPoints-1].lead()  == 0 &&
         l.lead()                 == 0 &&
         Point[NPoints-1].valid() != l.valid()) {

         // Set analysis track flag and reset InitTime to 0.
         IsAnlyTrack = true;
         InitTime    = (unixtime) 0;
      }
   }

   // Apply matching logic for BEST and analysis tracks
   if(IsBestTrack || IsAnlyTrack) {

      // Subsequent track point times cannot differ by too much
      if(NPoints > 0) {
         diff = (int) (l.warning_time() - Point[NPoints-1].valid());
         if(abs(diff) > MaxBestTrackTimeInc) match = false;
      }

      // Analysis tracks technique numbers must stay constant.
      if(IsAnlyTrack && !IsBestTrack &&
         TechniqueNumber != l.technique_number()) match = false;
   }

   // Apply matching logic for non-BEST, non-analysis tracks
   else {

      // Check that the technique number and init time stay constant.
      if(TechniqueNumber != l.technique_number() ||
         InitTime        != l.warning_time())
         match = false;
   }
*/
   return(match);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class GenesisInfoArray
//
////////////////////////////////////////////////////////////////////////

GenesisInfoArray::GenesisInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GenesisInfoArray::~GenesisInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

GenesisInfoArray::GenesisInfoArray(const GenesisInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

GenesisInfoArray & GenesisInfoArray::operator=(const GenesisInfoArray & t) {

   if(this == &t)  return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::clear() {

   Genesis.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "NGenesis = " << (int) Genesis.size() << "\n";

   for(i=0; i<Genesis.size(); i++) {
      out << prefix << "GenesisInfo[" << i+1 << "]:" << "\n";
      Genesis[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfoArray::serialize() const {
   ConcatString s;

   s << "GenesisInfoArray: "
     << "NGenesis = " << (int) Genesis.size();

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << serialize() << ", Genesis:\n";

   for(i=0; i<Genesis.size(); i++) {
      s << Genesis[i].serialize_r(i+1, indent_depth+1);
   }

   return(s);

}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::assign(const GenesisInfoArray &t) {
   int i;

   clear();

   if(t.Genesis.size() == 0) return;

   for(i=0; i<t.Genesis.size(); i++) Genesis.push_back(Genesis[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

const GenesisInfo & GenesisInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= Genesis.size())) {
      mlog << Error
           << "\nGenesisInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Genesis[n]);
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::add(const GenesisInfo &t) {

   Genesis.push_back(t);

   return;
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::add(const TrackInfo &t) {

   // JHG work here
   // Check if this genesis event already exists
   // If not, add it and return true.
   // If yes, return false.
   /*
   bool found  = false;
   bool status = false;
   int i;

   // Check if this ATCFTrackLine already exists in the GenesisInfoArray
   if(check_dup) {
      if(has(l)) {
         mlog << Warning
              << "\nGenesisInfoArray::add(const ATCFTrackLine &) -> "
              << "skipping duplicate ATCFTrackLine:\n"
              << l.get_line() << "\n\n";
         return(false);
      }
   }

   // Add ATCFTrackLine to an existing track if possible
   for(i=NTracks-1; i>=0; i--) {
      if(Track[i].is_match(l)) {
         found = true;
         status = Track[i].add(l, check_dup, check_anly);
         break;
      }
   }

   // Otherwise, create a new track
   if(!found) {
      extend(NTracks + 1);
      status = Track[NTracks++].add(l, check_dup, check_anly);
   }

   return(status);
   */
   return(true);
}

////////////////////////////////////////////////////////////////////////
