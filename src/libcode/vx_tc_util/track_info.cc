// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
#include "track_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TrackInfo
//
////////////////////////////////////////////////////////////////////////

TrackInfo::TrackInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TrackInfo::~TrackInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TrackInfo::TrackInfo(const TrackInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

TrackInfo & TrackInfo::operator=(const TrackInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::init_from_scratch() {

   Point = (TrackPoint *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::clear() {

   IsSet           = false;
   IsBestTrack     = false;
   IsOperTrack     = false;
   CheckAnly       = false;
   IsAnlyTrack     = false;

   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   TechniqueNumber = bad_data_int;
   Technique.clear();
   Initials.clear();
   InitTime        = (unixtime) 0;
   MinValidTime    = (unixtime) 0;
   MaxValidTime    = (unixtime) 0;
   MinWarmCore     = (unixtime) 0;
   MaxWarmCore     = (unixtime) 0;
   DiagName.clear();
   TrackLines.clear();

   clear_points();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::clear_points() {

   if(Point) { delete [] Point; Point = (TrackPoint *) 0; }
   NPoints = NAlloc = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "StormId         = \"" << StormId.contents() << "\"\n";
   out << prefix << "IsBestTrack     = " << bool_to_string(IsBestTrack) << "\n";
   out << prefix << "IsOperTrack     = " << bool_to_string(IsOperTrack) << "\n";
   out << prefix << "CheckAnly       = " << bool_to_string(CheckAnly) << "\n";
   out << prefix << "IsAnlyTrack     = " << bool_to_string(IsAnlyTrack) << "\n";
   out << prefix << "Basin           = \"" << Basin.contents() << "\"\n";
   out << prefix << "Cyclone         = \"" << Cyclone.contents() << "\"\n";
   out << prefix << "StormName       = \"" << StormName.contents() << "\"\n";
   out << prefix << "TechniqueNumber = " << TechniqueNumber << "\n";
   out << prefix << "Technique       = \"" << Technique.contents() << "\"\n";
   out << prefix << "Initials        = \"" << Initials.contents() << "\"\n";
   out << prefix << "InitTime        = \"" << (InitTime     > 0 ? unix_to_yyyymmdd_hhmmss(InitTime).text()     : na_str) << "\n";
   out << prefix << "MinValidTime    = \"" << (MinValidTime > 0 ? unix_to_yyyymmdd_hhmmss(MinValidTime).text() : na_str) << "\n";
   out << prefix << "MaxValidTime    = \"" << (MaxValidTime > 0 ? unix_to_yyyymmdd_hhmmss(MaxValidTime).text() : na_str) << "\n";
   out << prefix << "MinWarmCore     = \"" << (MinWarmCore  > 0 ? unix_to_yyyymmdd_hhmmss(MinWarmCore).text()  : na_str) << "\n";
   out << prefix << "MaxWarmCore     = \"" << (MaxWarmCore  > 0 ? unix_to_yyyymmdd_hhmmss(MaxWarmCore).text()  : na_str) << "\n";
   out << prefix << "NDiag           = " << DiagName.n() << "\n";
   out << prefix << "NPoints         = " << NPoints << "\n";
   out << prefix << "NAlloc          = " << NAlloc << "\n";
   out << prefix << "NTrackLines     = " << TrackLines.n() << "\n";

   for(i=0; i<NPoints; i++) {
      out << prefix << "TrackPoint[" << i+1 << "]:" << "\n";
      Point[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfo::serialize() const {
   ConcatString s;

   s << "TrackInfo: "
     << "StormId = \"" << StormId.contents() << "\""
     << ", IsBest = " << bool_to_string(IsBestTrack)
     << ", IsOper = " << bool_to_string(IsOperTrack)
     << ", CheckAnly = " << bool_to_string(CheckAnly)
     << ", IsAnly = " << bool_to_string(IsAnlyTrack)
     << ", Basin = \"" << Basin.contents() << "\""
     << ", Cyclone = \"" << Cyclone.contents() << "\""
     << ", StormName = \"" << StormName.contents() << "\""
     << ", TechniqueNumber = " << TechniqueNumber
     << ", Technique = \"" << Technique.contents() << "\""
     << ", Initials = \"" << Initials.contents() << "\""
     << ", InitTime = " << (InitTime > 0 ? unix_to_yyyymmdd_hhmmss(InitTime).text() : na_str)
     << ", MinValidTime = " << (MinValidTime > 0 ? unix_to_yyyymmdd_hhmmss(MinValidTime).text() : na_str)
     << ", MaxValidTime = " << (MaxValidTime > 0 ? unix_to_yyyymmdd_hhmmss(MaxValidTime).text() : na_str)
     << ", MinWarmCore = " << (MinWarmCore > 0 ? unix_to_yyyymmdd_hhmmss(MinWarmCore).text() : na_str)
     << ", MaxWarmCore = " << (MaxWarmCore > 0 ? unix_to_yyyymmdd_hhmmss(MaxWarmCore).text() : na_str)
     << ", NDiag = " << DiagName.n()
     << ", NPoints = " << NPoints
     << ", NAlloc = " << NAlloc
     << ", NTrackLines = " << TrackLines.n();

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << "[" << n << "] " << serialize() << ", Points:\n";

   for(i=0; i<NPoints; i++)
      s << Point[i].serialize_r(i+1, indent_depth+1);

   return(s);

}

////////////////////////////////////////////////////////////////////////

void TrackInfo::assign(const TrackInfo &t) {
   int i;

   clear();

   IsSet           = true;
   IsBestTrack     = t.IsBestTrack;
   IsOperTrack     = t.IsOperTrack;
   CheckAnly       = t.CheckAnly;
   IsAnlyTrack     = t.IsAnlyTrack;

   StormId         = t.StormId;
   Basin           = t.Basin;
   Cyclone         = t.Cyclone;
   StormName       = t.StormName;
   TechniqueNumber = t.TechniqueNumber;
   Technique       = t.Technique;
   Initials        = t.Initials;
   InitTime        = t.InitTime;
   MinValidTime    = t.MinValidTime;
   MaxValidTime    = t.MaxValidTime;
   MinWarmCore     = t.MinWarmCore;
   MaxWarmCore     = t.MaxWarmCore;
   DiagName        = t.DiagName;
   TrackLines      = t.TrackLines;

   if(t.NPoints == 0) return;

   extend(t.NPoints);

   for(i=0; i<t.NPoints; i++) Point[i] = t.Point[i];

   NPoints = t.NPoints;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::extend(int n, bool exact) {
   int j, k;
   TrackPoint *new_line = (TrackPoint *) 0;

   // Check if enough memory is already allocated
   if(NAlloc >= n) return;

   // Compute the allocation size 
   if(!exact) {
      k = n/TrackInfoAllocInc;
      if(n%TrackInfoAllocInc) k++;
      n = k*TrackInfoAllocInc;
   }

   // Allocate a new TrackPoint array of the required length
   new_line = new TrackPoint [n];

   if(!new_line) {
      mlog << Error
           << "\nvoid TrackInfo::extend(int, bool) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   // Copy the array contents and delete the old one
   if(Point) {
      for(j=0; j<NPoints; j++) new_line[j] = Point[j];
      delete [] Point;  Point = (TrackPoint *) 0;
   }

   // Point to the new array
   Point     = new_line;
   new_line = (TrackPoint *) 0;

   // Store the allocated length
   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::initialize(const ATCFTrackLine &l, bool check_anly) {

   IsSet           = true;

   IsBestTrack     = l.is_best_track();
   IsOperTrack     = l.is_oper_track();

   CheckAnly       = check_anly;
   // IsAnlyTrack is determined by subsequent track data points.

   Basin           = l.basin();
   Cyclone         = l.cyclone_number();
   StormName       = l.storm_name();
   TechniqueNumber = l.technique_number();
   Technique       = l.technique();
   Initials        = l.initials();

   // For BEST tracks, keep InitTime unset.
   if(IsBestTrack) InitTime = (unixtime) 0;
   else            set_init(l.warning_time());

   // Set the valid time range
   MinValidTime = MaxValidTime = l.valid();

   // Create the storm id
   set_storm_id(l.storm_id().c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::set_point(int n, const TrackPoint &p) {

   // Check range
   if((n < 0) || (n >= NPoints)) {
      mlog << Error
           << "\nTrackInfo::set_point(int, const TrackPoint &) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   Point[n] = p;

   return;
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::lead_index(int l) const {
   int i;

   // Loop through the TrackPoints looking for a matching lead time
   for(i=0; i<NPoints; i++) {
      if(Point[i].lead() == l) break;
   }

   if(i == NPoints) i = -1;

   return(i);
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::valid_index(unixtime u) const {
   int i;

   // Loop through the TrackPoints looking for a matching valid time
   for(i=0; i<NPoints; i++) {
      if(Point[i].valid() == u) break;
   }

   if(i == NPoints) i = -1;

   return(i);
}

////////////////////////////////////////////////////////////////////////

const TrackPoint & TrackInfo::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= NPoints)) {
      mlog << Error
           << "\nTrackInfo::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Point[n]);
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::set_storm_id() {

   StormId = define_storm_id(InitTime, MinValidTime, MaxValidTime,
                             Basin, Cyclone);

   return;
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::duration() const {
   return(MaxValidTime == 0 || MinValidTime == 0 ? bad_data_int :
          MaxValidTime - MinValidTime);
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::warm_core_dur() const {
   return(MaxWarmCore == 0 || MinWarmCore == 0 ? bad_data_int :
          MaxWarmCore - MinWarmCore);
}

////////////////////////////////////////////////////////////////////////

const char * TrackInfo::diag_name(int i) const {
   return(i>=0 && i<DiagName.n() ? DiagName[i].c_str() : na_str);
}

////////////////////////////////////////////////////////////////////////

int TrackInfo::valid_inc() const {
   int i;
   NumArray ut_inc;

   // Compute list of time spacing between TrackPoints
   for(i=1; i<NPoints; i++)
      ut_inc.add((int) (Point[i].valid() - Point[i-1].valid()));

   // Return the most common spacing
   return(nint(ut_inc.mode()));
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::add(const TrackPoint &p) {

   extend(NPoints + 1, false);
   Point[NPoints++] = p;

   // Check the valid time range
   if(MinValidTime == (unixtime) 0 || p.valid() < MinValidTime)
      MinValidTime = p.valid();
   if(MaxValidTime == (unixtime) 0 || p.valid() > MaxValidTime)
      MaxValidTime = p.valid();

   // Check the warm core time range
   if(p.warm_core()) {
      if(MinWarmCore == (unixtime) 0 || p.valid() < MinWarmCore)
         MinWarmCore = p.valid();
      if(MaxWarmCore == (unixtime) 0 || p.valid() > MaxWarmCore)
         MaxWarmCore = p.valid();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::add(const ATCFTrackLine &l, bool check_dup, bool check_anly) {
   bool found = false;
   bool status = false;
   int i;

   // Initialize TrackInfo with ATCFTrackLine, if necessary
   if(!IsSet) initialize(l, check_anly);

   // Check if TrackPoint doesn't match this TrackInfo
   if(!is_match(l)) return(false);

   // Check if the storm name needs to be set or has changed
   ConcatString name = l.storm_name();
   if(StormName.length() == 0) StormName = name;
   else if(StormName.length() > 0 && name.length() > 0 &&
           StormName != name) {
      mlog << Debug(4)
           << "Updating " << StormId << " storm name from \""
           << StormName << "\" to \"" << name << "\".\n";
      StormName = name;
   }

   // Check that the TrackPoint valid time is increasing
   if(NPoints > 0) {
      if(l.valid() < Point[NPoints-1].valid()) {
         mlog << Warning
              << "\nTrackInfo::add(const ATCFTrackLine &) -> "
              << "skipping ATCFTrackLine since the valid time is not increasing ("
              << unix_to_yyyymmdd_hhmmss(l.valid()) << " < "
              << unix_to_yyyymmdd_hhmmss(Point[NPoints-1].valid())
              << "):\n" << l.get_line() << "\n\n";
         return(false);
      }
   }

   // Add ATCFTrackLine to an existing TrackPoint if possible
   for(i=NPoints-1; i>=0; i--) {
      if(Point[i].is_match(l)) {
         found = true;
         status = Point[i].set(l);
         break;
      }
   }

   // Otherwise, create a new point
   if(!found) {
      extend(NPoints + 1, false);
      status = Point[NPoints++].set(l);
   }

   // Check the valid time range
   if(MinValidTime == (unixtime) 0 || l.valid() < MinValidTime)
      MinValidTime = l.valid();
   if(MaxValidTime == (unixtime) 0 || l.valid() > MaxValidTime)
      MaxValidTime = l.valid();

   // Check the warm core time range
   if(l.warm_core()) {
      if(MinWarmCore == (unixtime) 0 || l.valid() < MinWarmCore)
         MinWarmCore = l.valid();
      if(MaxWarmCore == (unixtime) 0 || l.valid() > MaxWarmCore)
         MaxWarmCore = l.valid();
   }

   // Store the ATCFTrackLine that was just added
   if(check_dup) TrackLines.add(l.get_line());

   return(status);
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::add_watch_warn(const ConcatString &ww_sid,
                               WatchWarnType ww_type, unixtime ww_ut) {
   int i;

   // Check for a matching storm id
   if(storm_id() != ww_sid) return;

   // Loop over the TrackPoints
   for(i=0; i<NPoints; i++) Point[i].set_watch_warn(ww_type, ww_ut);

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::add_diag_data(DiagFile &diag_file, const StringArray &diag_name) {

   // Check for a match
   if(StormId   != diag_file.storm_id()  ||
      Technique != diag_file.technique() ||
      InitTime  != diag_file.init()) return(false);

   // If empty, store all diagnostics
   if(diag_name.n() > 0) DiagName = diag_name;
   else                  DiagName = diag_file.diag_name();

   int i_name, i_time, i_pnt;

   // Retrieve data for each diagnostic
   for(i_name=0; i_name<DiagName.n(); i_name++) {

      NumArray diag_val = diag_file.diag_val(DiagName[i_name]);

      // Add diagnostic values to the TrackPoints
      for(i_time=0; i_time<diag_file.n_time(); i_time++) {

         // Get the index of the TrackPoint for this lead time
         if((i_pnt = lead_index(nint(diag_file.lead(i_time)))) < 0) continue;

         // Store this diagnostic value in the TrackPoint
         Point[i_pnt].add_diag_value(diag_file.lat(i_time),
                                     diag_file.lon(i_time),
                                     diag_val[i_time]);

      } // end for i_time
   } // end for i_name

   return(true);
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::add_diag_value(int i_pnt, double val) {

   // Range check
   if(i_pnt < 0 || i_pnt >= NPoints) {
      mlog << Error << "\nTrackInfo::add_diag_value() -> "
           << "range check error for point " << i_pnt << "\n\n";
      exit(1);
   }

   Point[i_pnt].add_diag_value(Point[i_pnt].lat(),
                               Point[i_pnt].lon(),
                               val);

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::has(const ATCFTrackLine &l) const {
   return(TrackLines.has(l.get_line()));
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_match(const ATCFTrackLine &l) {
   bool match = true;
   int diff;

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

   return(match);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_match(const TrackInfo &t) const {
   bool match = true;

   // Check if track is for the same basin and storm with overlapping
   // valid times.
   if(Basin        != t.basin()     ||
      Cyclone      != t.cyclone()   ||
      MinValidTime  > t.valid_max() ||
      MaxValidTime  < t.valid_min())
      match = false;

   // Check that technique is defined
   if(Technique.empty() || t.technique().empty()) return(false);

   // Check that init times match for non-BEST, non-analysis tracks
   if(!IsBestTrack && !t.is_best_track() &&
      !IsAnlyTrack && !t.is_anly_track() &&
      InitTime != t.init()) {
      match = false;
   }

   return(match);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_interp() const {
   const char *s = Technique.c_str();
   int offset = (m_strlen(Technique.c_str()) - 1);

   // Return true if the last character of the model name is 'I'
   if (offset < 0) return false;
   else return(s[offset] == 'I');

}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TrackInfoArray
//
////////////////////////////////////////////////////////////////////////

TrackInfoArray::TrackInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TrackInfoArray::~TrackInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TrackInfoArray::TrackInfoArray(const TrackInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

TrackInfoArray & TrackInfoArray::operator=(const TrackInfoArray & t) {

   if(this == &t)  return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::clear() {

   Track.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "NTracks = " << Track.size() << "\n";

   for(i=0; i<Track.size(); i++) {
      out << prefix << "TrackInfo[" << i+1 << "]:" << "\n";
      Track[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfoArray::serialize() const {
   ConcatString s;

   s << "TrackInfoArray: "
     << "NTracks = " << n();

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << serialize() << ", Tracks:\n";

   for(i=0; i<Track.size(); i++)
      s << Track[i].serialize_r(i+1, indent_depth+1) << "\n";

   return(s);

}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::assign(const TrackInfoArray &t) {
   int i;

   clear();

   for(i=0; i<t.n(); i++) Track.push_back(t[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

const TrackInfo & TrackInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= Track.size())) {
      mlog << Error
           << "\nTrackInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Track[n]);
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::add(const TrackInfo &t) {

   Track.push_back(t);

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::set(int n, const TrackInfo &t) {

   // Check range
   if((n < 0) || (n >= Track.size())) {
      mlog << Error
           << "\nTrackInfoArray::set(int, const TrackInfo &) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   Track[n] = t;

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::add(const ATCFTrackLine &l, bool check_dup, bool check_anly) {
   bool found  = false;
   bool status = false;
   int i;

   // Check if this ATCFTrackLine already exists in the TrackInfoArray
   if(check_dup) {
      if(has(l)) {
         mlog << Warning
              << "\nTrackInfoArray::add(const ATCFTrackLine &) -> "
              << "skipping duplicate ATCFTrackLine:\n"
              << l.get_line() << "\n\n";
         return(false);
      }
   }

   // Add ATCFTrackLine to an existing track if possible
   for(i=Track.size()-1; i>=0; i--) {
      if(Track[i].is_match(l)) {
         found = true;
         status = Track[i].add(l, check_dup, check_anly);
         break;
      }
   }

   // Otherwise, create a new track
   if(!found) {
      TrackInfo t;
      t.add(l, check_dup, check_anly);
      Track.push_back(t);
      status = true;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::has(const ATCFTrackLine &l) const {
   bool found = false;
   int i;

   // Check if the TrackInfo data matches
   for(i=Track.size()-1; i>=0; i--) {
      if(Track[i].has(l)) {
         found = true;
         break;
      }
   }

   // Return whether the TrackInfo matches
   return(found);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::erase_storm_id(const ConcatString &s) {
   bool status = false;
   int i;

   // Erase all tracks with this storm id
   for(i=0; i<Track.size(); i++) {
      if(Track[i].storm_id() == s) {
         Track.erase(Track.begin()+i);
         i--;
         status = true;
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::add_diag_data(DiagFile &diag_file, const StringArray &diag_name) {
   bool match = false;

   // Set the names for each track
   for(int i=0; i<Track.size(); i++) {
      if(Track[i].add_diag_data(diag_file, diag_name)) {
         match = true;
         break;
      }
   }

   return(match);
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

TrackInfo consensus(const TrackInfoArray &tracks,
                    const ConcatString &model, int req,
                    const StringArray &req_list) {
   int i, j, i_pnt;
   bool skip;
   TrackInfo tavg;
   NumArray lead_list;

   // Variables for average TrackPoint
   int        pcnt;
   TrackPoint pavg, psum;
   QuadInfo   wavg;
   NumArray   plon, plat, pvmax, pmslp;
   double     lon_range, lon_shift, lon_avg;
   double     track_spread, vmax_stdev, mslp_stdev;
   
   // Check for at least one track
   if(tracks.n() == 0) {
      mlog << Error
           << "\nTrackInfoArray::consensus() -> "
           << "cannot compute a consensus for zero tracks!\n\n";
      exit(1);
   }

   // Initialize average track to the first track
   tavg.set_basin(tracks[0].basin().c_str());
   tavg.set_cyclone(tracks[0].cyclone().c_str());
   tavg.set_storm_name(tracks[0].storm_name().c_str());
   tavg.set_technique_number(tracks[0].technique_number());
   tavg.set_technique(model.c_str());
   tavg.set_init(tracks[0].init());
   tavg.set_storm_id();

   // Loop through the tracks and build a list of lead times
   for(i=0; i<tracks.n(); i++) {

      // Error out if these elements change
      if(tavg.basin()   != tracks[i].basin()   ||
         tavg.cyclone() != tracks[i].cyclone() ||
         tavg.init()    != tracks[i].init()) {
         mlog << Error
              << "\nTrackInfoArray::consensus() -> "
              << "the basin, cyclone number, and init time must "
              << "remain constant.\n\n";
         exit(1);
      }

      // Warning if the the technique number changes
      if(tavg.technique_number() != tracks[i].technique_number()) {
         mlog << Warning
              << "\nTrackInfoArray::consensus() -> "
              << "the technique number has changed ("
              << tavg.technique_number() << "!="
              << tracks[i].technique_number() << ").\n\n";
      }

      // Loop through the points for the lead times
      for(j=0; j<tracks[i].n_points(); j++) {

         // Add the lead time to the list
         if(!is_bad_data(tracks[i][j].lead())) {
            if(!lead_list.has(tracks[i][j].lead()))
               lead_list.add(tracks[i][j].lead());
         }
      } // end for j

      // Sort the lead times
      lead_list.sort_array();
   }

   // Loop through the lead times and construct a TrackPoint for each
   for(i=0, skip=false; i<lead_list.n(); i++) {

      // Initialize TrackPoint
      pavg.clear();
      psum.clear();
      plon.clear();
      plat.clear();
      pvmax.clear();
      pmslp.clear();
      pcnt = 0;

      // Loop through the tracks and get an average TrackPoint
      for(j=0; j<tracks.n(); j++) {

         // Get the index of the TrackPoint for this lead time
         i_pnt = tracks.Track[j].lead_index(nint(lead_list[i]));

         // Check for missing TrackPoint in a required member
         if(i_pnt < 0) {
            if(req_list.has(tracks.Track[j].technique())) {
               skip = true;
               break;
            }
            continue;
         }

         // Increment the TrackPoint count and sums
         pcnt++;
         if(pcnt == 1) psum  = tracks.Track[j][i_pnt];
         else          psum += tracks.Track[j][i_pnt];

         // Store the track point latitude, longitude v_max and mslp values         
         plon.add(tracks.Track[j][i_pnt].lon());
         plat.add(tracks.Track[j][i_pnt].lat());
         pvmax.add(tracks.Track[j][i_pnt].v_max());
         pmslp.add(tracks.Track[j][i_pnt].mslp());
      }

      // Check for missing required member and the minimum number of points
      if(skip == true || pcnt < req) continue;

      // Compute the average point
      pavg = psum;
      if(!is_bad_data(pavg.v_max())) pavg.set_v_max(psum.v_max()/pcnt);
      if(!is_bad_data(pavg.mslp()))  pavg.set_mslp(psum.mslp()/pcnt);
      if(!is_bad_data(pavg.radp()))  pavg.set_radp(psum.radp()/pcnt);
      if(!is_bad_data(pavg.rrp()))   pavg.set_rrp(psum.rrp()/pcnt);
      if(!is_bad_data(pavg.mrd()))   pavg.set_mrd(psum.mrd()/pcnt);
      if(!is_bad_data(pavg.gusts())) pavg.set_gusts(psum.gusts()/pcnt);
      if(!is_bad_data(pavg.eye()))   pavg.set_eye(psum.eye()/pcnt);
      if(!is_bad_data(pavg.speed())) pavg.set_speed(psum.speed()/pcnt);

      // Compute the range of longitude values
      lon_range = plon.max() - plon.min();

      // Sum the longitudes, shifting negative values if we've crossed
      // the international date line
      for(j=0, lon_avg=0; j<plon.n(); j++) {
         lon_shift = (lon_range > 180.0 && plon[j] < 0.0 ? 360.0 : 0.0);
         lon_avg += (plon[j] + lon_shift);
      }
      lon_avg /= pcnt;

      // Store the average lat/lon
      if(!is_bad_data(pavg.lat())) pavg.set_lat(psum.lat()/pcnt);
      if(!is_bad_data(pavg.lon())) pavg.set_lon(rescale_deg(lon_avg, -180.0, 180.0));

      // Save the number of members that went into the consensus
      if(pcnt > 0) pavg.set_num_members(pcnt);
      
      // Compute track spread and distance mean, convert to nautical-miles
      double track_spread, dist_mean;
      compute_gc_dist_stdev(pavg.lat(), pavg.lon(), plat, plon, track_spread, dist_mean);

      if(!is_bad_data(track_spread)) {
         track_spread *= tc_nautical_miles_per_km;
         pavg.set_spread(track_spread);
      }

      if(!is_bad_data(dist_mean)) {
         dist_mean *= tc_nautical_miles_per_km;
         pavg.set_dist_mean(dist_mean);
      }
      
      // Compute wind-speed (v_max) and pressure (mslp) standard deviation
      vmax_stdev = pvmax.stdev();
      mslp_stdev = pmslp.stdev();
      if(!is_bad_data(vmax_stdev)) pavg.set_v_max_stdev(vmax_stdev);
      if(!is_bad_data(mslp_stdev)) pavg.set_mslp_stdev(mslp_stdev);

      // Compute the average winds
      for(j=0; j<NWinds; j++) {

         // Initialize the wind QuadInfo sum
         wavg = psum[j];

         // Compute the average wind
         if(!is_bad_data(wavg.al_val())) wavg.set_al_val(wavg.al_val()/pcnt);
         if(!is_bad_data(wavg.ne_val())) wavg.set_ne_val(wavg.ne_val()/pcnt);
         if(!is_bad_data(wavg.se_val())) wavg.set_se_val(wavg.se_val()/pcnt);
         if(!is_bad_data(wavg.sw_val())) wavg.set_sw_val(wavg.sw_val()/pcnt);
         if(!is_bad_data(wavg.nw_val())) wavg.set_nw_val(wavg.nw_val()/pcnt);

         // Store the average wind
         pavg.set_wind(j, wavg);
      }

      // Compute consensus CycloneLevel
      if(!is_bad_data(pavg.v_max())) pavg.set_level(wind_speed_to_cyclonelevel(pavg.v_max()));

      // Add the current track point
      tavg.add(pavg);
   }

   // Return the consensus track
   return(tavg);
}

////////////////////////////////////////////////////////////////////////

void compute_gc_dist_stdev(const double lat, const double lon, const NumArray &lats, const NumArray &lons, double &spread, double &mean) {

   int i, count;
   NumArray dist_na;
   
   // Loop over member lat/lon track values, calculate great-circle distance between memmber values and consensus track
   for(i=0, count=0; i<lats.n_elements(); i++) {
      if( is_bad_data(lats[i]) || is_bad_data(lons[i]) || is_bad_data(lat) || is_bad_data(lon) ) continue;
      dist_na.add(gc_dist(lats[i], lons[i], lat, lon));
      count++;
   }
   
   // Compute spread (standard-deviation of the distances)
   // and the mean of the distnaces
   if(count == 0) {
      spread = bad_data_double;
      mean = bad_data_double;
   }
   else {
      spread = dist_na.stdev();
      mean = dist_na.mean();
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////
//
// Determine if the basin/cyclone/init of the storm match any of the
// storm id's specified in the list.  The storm id consists of:
//   2-character basin, 2-character cyclone, 4-character year
//
// To match all storms in a season, replace cyclone with "AL".
// To match multiple years, replace year with the the last two digits
// of the beginning and ending years.
//
////////////////////////////////////////////////////////////////////////

bool has_storm_id(const StringArray &storm_id,
                  const ConcatString &basin,
                  const ConcatString &cyclone,
                  unixtime ut) {
   int i, year, year_beg, year_end;
   unixtime ut_beg, ut_end;
   bool match = false;

   // Loop over the storm id entries
   for(i=0; i<storm_id.n(); i++) {

      // Check that the basin matches
      if(strncasecmp(storm_id[i].c_str(), basin.c_str(), 2) != 0) continue;

      // Check that the cyclone number matches
      if(strncasecmp(storm_id[i].c_str()+2, cyclone.c_str(), 2) != 0 &&
         strncasecmp(storm_id[i].c_str()+2,    "AL", 2) != 0) continue;

      // Parse the year
      year = atoi(storm_id[i].c_str()+4);

      // Handle a single year
      if(year >= 1900 && year < 2100) {
         year_beg = year_end = year;
      }
      // Handle a range of years
      else {
         year_beg = year/100;
         year_end = year%100;

         // Add the appropriate century
         if(year_beg > 50) year_beg += 1900;
         else              year_beg += 2000;
         if(year_end > 50) year_end += 1900;
         else              year_end += 2000;
      }

      // Check that the ut time falls in the specified time range
      ut_beg = mdyhms_to_unix(01, 01, year_beg,   0, 0, 0);
      ut_end = mdyhms_to_unix(01, 01, year_end+1, 0, 0, 0);
      if(ut < ut_beg || ut >= ut_end) continue;

      // Otherwise, it's a match
      match = true;
      break;
   }

   return(match);
}

////////////////////////////////////////////////////////////////////////

void latlon_to_xytk_err(double alat, double alon,
                        double blat, double blon,
                        double &x_err, double &y_err, double &tk_err) {
   double lat_err, lon_err, lat_avg;

   // Check for bad data
   if(is_bad_data(alat) || is_bad_data(alon) ||
      is_bad_data(blat) || is_bad_data(blon)) {
      x_err = y_err = tk_err = bad_data_double;
      return;
   }

   // Compute lat/lon errors
   lat_err = alat - blat;
   lon_err = rescale_lon(alon - blon);
   lat_avg = 0.5*(alat + blat);

   // Compute X/Y and track error
   x_err  = nautical_miles_per_deg*lon_err*cosd(lat_avg);
   y_err  = nautical_miles_per_deg*lat_err;
   tk_err = sqrt(x_err*x_err + y_err*y_err);

   return;
}

////////////////////////////////////////////////////////////////////////
