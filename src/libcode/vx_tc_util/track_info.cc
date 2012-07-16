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

   Basin.clear();
   Cyclone.clear();
   TechniqueNumber = bad_data_int;
   Technique.clear();
   InitTime        = (unixtime) 0;
   MinValidTime    = (unixtime) 0;
   MaxValidTime    = (unixtime) 0;

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

   out << prefix << "Basin           = \"" << (Basin ? Basin.text() : "(nul)") << "\"\n";
   out << prefix << "Cyclone         = \"" << (Cyclone ? Cyclone.text() : "(nul)") << "\"\n";
   out << prefix << "TechniqueNumber = " << TechniqueNumber << "\n";
   out << prefix << "Technique       = \"" << (Technique ? Technique.text() : "(nul)") << "\"\n";
   out << prefix << "InitTime        = " << unix_to_yyyymmdd_hhmmss(InitTime) << "\n";
   out << prefix << "MinValidTime    = " << unix_to_yyyymmdd_hhmmss(MinValidTime) << "\n";
   out << prefix << "MaxValidTime    = " << unix_to_yyyymmdd_hhmmss(MaxValidTime) << "\n";
   out << prefix << "NPoints         = " << NPoints << "\n";
   out << prefix << "NAlloc          = " << NAlloc << "\n";
      
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
     << "Basin = \"" << (Basin ? Basin.text() : "(nul)") << "\""
     << ", Cyclone = \"" << (Cyclone ? Cyclone.text() : "(nul)") << "\""
     << ", TechniqueNumber = " << TechniqueNumber
     << ", Technique = \"" << (Technique ? Technique.text() : "(nul)") << "\""
     << ", InitTime = " << unix_to_yyyymmdd_hhmmss(InitTime)
     << ", MinValidTime = " << unix_to_yyyymmdd_hhmmss(MinValidTime)
     << ", MaxValidTime = " << unix_to_yyyymmdd_hhmmss(MaxValidTime)
     << ", NPoints = " << NPoints
     << ", NAlloc = " << NAlloc;

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
   
   Basin           = t.Basin;
   Cyclone         = t.Cyclone;
   TechniqueNumber = t.TechniqueNumber;
   Technique       = t.Technique;
   InitTime        = t.InitTime;
   MinValidTime    = t.MinValidTime;
   MaxValidTime    = t.MaxValidTime;

   if(t.NPoints == 0) return;

   extend(t.NPoints);
   
   for(i=0; i<t.NPoints; i++) Point[i] = t.Point[i];

   NPoints = t.NPoints;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::extend(int n) {
   int j, k;
   TrackPoint *new_line = (TrackPoint *) 0;

   // Check if enough memory is already allocated
   if(NAlloc >= n) return;

   // Check how many allocations are required  
   k = n/TrackInfoAllocInc;
   if(n%TrackInfoAllocInc) k++;
   n = k*TrackInfoAllocInc;

   // Allocate a new TrackPoint array of the required length
   new_line = new TrackPoint [n];

   if(!new_line) {
      mlog << Error
           << "\nvoid TrackInfo::extend(int) -> "
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

void TrackInfo::initialize(const TrackLine &l) {

   IsSet           = true;

   Basin           = l.basin();
   Cyclone         = l.cyclone_number();
   TechniqueNumber = l.technique_number();
   Technique       = l.technique();

   // For BEST tracks, keep InitTime unset
   if(Technique) {
      if(strcasecmp(Technique, BestTrackStr) == 0) InitTime = (unixtime) 0;
      else                                         set_init(l.warning_time());
   }

   // Set the valid time range
   MinValidTime = MaxValidTime = l.valid();
   
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

   extend(NPoints + 1);
   Point[NPoints++] = p;

   // Check the valid time range
   if(MinValidTime == (unixtime) 0 || p.valid() < MinValidTime)
      MinValidTime = p.valid();
   if(MaxValidTime == (unixtime) 0 || p.valid() > MaxValidTime)
      MaxValidTime = p.valid();

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::add(const TrackLine &l) {
   bool found = false;
   bool status = false;
   int i;

   // Initialize TrackInfo with TrackLine, if necessary
   if(!IsSet) initialize(l);

   // Check if TrackPoint doesn't match this TrackInfo
   if(!is_match(l)) return(false);

   // Check that the TrackPoint valid time is increasing
   if(NPoints > 0) {
      if(l.valid() < Point[NPoints-1].valid()) {
         mlog << Warning
              << "\nTrackInfo::add(const TrackPoint &) -> "
              << "skipping TrackLine since the valid time is not increasing ("
              << unix_to_yyyymmdd_hhmmss(l.valid()) << " < "
              << unix_to_yyyymmdd_hhmmss(Point[NPoints-1].valid())
              << "):\n" << l.line() << "\n\n";
         return(false);
      }
   }

   // Add TrackLine to an existing TrackPoint if possible
   for(i=0; i<NPoints; i++) {
      if(Point[i].is_match(l)) {
         found = true;
         status = Point[i].set(l);
         break;
      }
   }

   // Otherwise, create a new point
   if(!found) {
      extend(NPoints + 1);
      status = Point[NPoints++].set(l);
   }

   // Check the valid time range
   if(MinValidTime == (unixtime) 0 || l.valid() < MinValidTime)
      MinValidTime = l.valid();
   if(MaxValidTime == (unixtime) 0 || l.valid() > MaxValidTime)
      MaxValidTime = l.valid();

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::has(const TrackLine &l) const {
   int found = false;
   int i;

   // Check if the TrackPoint data matches
   for(i=0; i<NPoints; i++) {
      if(Point[i].has(l)) {
         found = true;
         break;
      }
   }

   // Return whether the TrackPoint matches and the TrackInfo matches
   return(found && is_match(l));
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_match(const TrackLine &l) const {
   bool match = true;
   int diff;

   // Make sure technique is defined
   if(!Technique) return(false);
   
   // Apply matching logic for BEST tracks
   if(strcasecmp(Technique, BestTrackStr) == 0) {

      // Check basin, cyclone, and technique
      if(Basin     != l.basin()          ||
         Cyclone   != l.cyclone_number() ||
         Technique != l.technique())
         match = false;

      // Subsequent track point times cannot differ by too much
      if(NPoints > 0) {
         diff = (int) (l.warning_time() - Point[NPoints-1].valid());
         if(abs(diff) >
            MaxBestTrackTimeInc)
            match = false;
      }
   }
   
   // Apply matching logic for non-BEST tracks
   else {

      // Check basin, cyclone, technique number, technique
      // and check that the init time remains constant
      if(Basin           != l.basin()            ||
         Cyclone         != l.cyclone_number()   ||
         TechniqueNumber != l.technique_number() ||
         Technique       != l.technique()        ||
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
   if(!Technique || !t.technique()) return(false);
   
   // If neither is a BEST track, check that the init times matches
   if(strcasecmp(Technique,     BestTrackStr) != 0 &&
      strcasecmp(t.technique(), BestTrackStr) != 0 &&
      InitTime != t.init())
      match = false;

   return(match);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_interp() const {
   const char *s = Technique;
   
   s += (strlen(Technique) - 1);

   // Return true if the last character of the model name is 'I'
   return(*s == 'I');
}

////////////////////////////////////////////////////////////////////////

bool TrackInfo::is_6hour() const {
   const char *s = Technique;

   s += (strlen(Technique) - 1);

   // Return true if the last character of the model name is '2'
   return(*s == '2');
}

////////////////////////////////////////////////////////////////////////

void TrackInfo::merge_points(const TrackInfo &t) {
   TrackInfo new_t;
   int i, j;
   
   // Initialize to the current TrackInfo
   new_t = *this;

   // Delete the TrackPoints
   new_t.clear_points();

   // Loop through the TrackPoints,
   // assuming they are in chronological order
   i = j = 0;
   while(i<NPoints || j<t.n_points()) {

      // Check range
      if(i >= NPoints)      { new_t.add(t[j]);     j++; continue; }
      if(j >= t.n_points()) { new_t.add(Point[i]); i++; continue; }
      
      // Add the TrackPoint, who's valid time is less
           if(Point[i].valid() <= t[j].valid()) { new_t.add(Point[i]); i++; }
      else if(Point[i].valid() >  t[j].valid())  { new_t.add(t[j]);    j++; }

   } // end while

   *this = new_t;

   return;
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

   Track = (TrackInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::clear() {

   if(Track) { delete [] Track; Track = (TrackInfo *) 0; }
   NTracks = NAlloc  = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "NTracks = " << NTracks << "\n";
   out << prefix << "NAlloc  = " << NAlloc << "\n";

   for(i=0; i<NTracks; i++) {
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
     << "NTracks = " << NTracks
     << ", NAlloc = " << NAlloc;

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << serialize() << ", Tracks:\n";

   for(i=0; i<NTracks; i++)
      s << Track[i].serialize_r(i+1, indent_depth+1);

   return(s);

}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::assign(const TrackInfoArray &t) {
   int i;

   clear();

   if(t.NTracks == 0) return;

   extend(t.NTracks);

   for(i=0; i<t.NTracks; i++) Track[i] = t.Track[i];

   NTracks = t.NTracks;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::extend(int n) {
   int j, k;
   TrackInfo *new_info = (TrackInfo *) 0;

   // Check if enough memory is already allocated
   if(NAlloc >= n) return;

   // Check how many allocations are required
   k = n/TrackInfoArrayAllocInc;
   if(n%TrackInfoArrayAllocInc) k++;
   n = k*TrackInfoArrayAllocInc;

   // Allocate a new TrackInfo array of the required length
   new_info = new TrackInfo [n];

   if(!new_info) {
      mlog << Error
           << "\nvoid TrackInfoArray::extend(int) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   // Copy the array contents and delete the old one
   if(Track) {
      for(j=0; j<NTracks; j++) new_info[j] = Track[j];
      delete [] Track;  Track = (TrackInfo *) 0;
   }

   // Point to the new array
   Track     = new_info;
   new_info = (TrackInfo *) 0;

   // Store the allocated length
   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

const TrackInfo & TrackInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= NTracks)) {
      mlog << Error
           << "\nTrackInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Track[n]);
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::add(const TrackInfo &t) {

   extend(NTracks + 1);
   Track[NTracks++] = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackInfoArray::set(int n, const TrackInfo &t) {

   // Check range
   if((n < 0) || (n >= NTracks)) {
      mlog << Error
           << "\nTrackInfoArray::set(int, const TrackInfo &) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   Track[n] = t;

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::add(const TrackLine &l, bool check_has) {
   bool found  = false;
   bool status = false;
   int i;

   // Check if this TrackLine already exists in the TrackInfoArray
   if(check_has) {
      if(has(l)) {
         mlog << Warning
              << "\nTrackInfoArray::add(const TrackLine &) -> "
              << "skipping duplicate TrackLine:\n"
              << l.line() << "\n\n";
         return(false);
      }
   }

   // Add TrackLine to an existing track if possible
   for(i=0; i<NTracks; i++) {
      if(Track[i].is_match(l)) {
         found = true;
         status = Track[i].add(l);
         break;
      }
   }

   // Otherwise, create a new track
   if(!found) {
      extend(NTracks + 1);
      status = Track[NTracks++].add(l);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool TrackInfoArray::has(const TrackLine &l) const {
   int found = false;
   int i;

   // Check if the TrackInfo data matches
   for(i=0; i<NTracks; i++) {
      if(Track[i].has(l)) {
         found = true;
         break;
      }
   }

   // Return whether the TrackInfo matches
   return(found);
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

TrackInfo consensus(const TrackInfoArray &tarr,
                    const ConcatString &model, int req) {
   int i, j, k, i_pnt;
   TrackInfo tavg;
   NumArray lead_list;

   // Variables for average TrackPoint
   int        pcnt;
   TrackPoint pavg, psum;
   QuadInfo   wavg;

   // Check for at least one track
   if(tarr.n_tracks() == 0) {
      mlog << Error
           << "\nTrackInfoArray::consensus() -> "
           << "cannot compute a consensus for zero tracks!\n\n";
      exit(1);
   }

   // Initialize average track to the first track
   tavg.set_basin(tarr.Track[0].basin());
   tavg.set_cyclone(tarr.Track[0].cyclone());
   tavg.set_technique_number(tarr.Track[0].technique_number());
   tavg.set_technique(model);
   tavg.set_init(tarr.Track[0].init());

   // Loop through the tracks and build a list of lead times
   for(i=0; i<tarr.n_tracks(); i++) {

      // Error out if these elements change
      if(tavg.basin()   != tarr.Track[i].basin()   ||
         tavg.cyclone() != tarr.Track[i].cyclone() ||
         tavg.init()    != tarr.Track[i].init()) {
         mlog << Error
              << "\nTrackInfoArray::consensus() -> "
              << "the basin, cyclone number, and init time must "
              << "remain constant.\n\n";
         exit(1);
      }

      // Warning if the the technique number changes
      if(tavg.technique_number() != tarr.Track[i].technique_number()) {
         mlog << Warning
              << "\nTrackInfoArray::consensus() -> "
              << "the technique number has changed ("
              << tavg.technique_number() << "!="
              << tarr.Track[i].technique_number() << ").\n\n";
      }

      // Loop through the points for the lead times
      for(j=0; j<tarr.Track[i].n_points(); j++) {

         // Add the lead time to the list
         if(!is_bad_data(tarr.Track[i][j].lead())) {
            if(!lead_list.has(tarr.Track[i][j].lead()))
               lead_list.add(tarr.Track[i][j].lead());
         }
      } // end for j

      // Sort the lead times
      lead_list.sort_array();
   }

   // Loop through the lead times and construct a TrackPoint for each
   for(i=0; i<lead_list.n_elements(); i++) {

      // Initialize TrackPoint
      pavg.clear();
      psum.clear();
      pcnt = 0;

      // Loop through the tracks and get an average TrackPoint
      for(j=0; j<tarr.n_tracks(); j++) {

         // Get the index of the TrackPoint for this lead time
         i_pnt = tarr.Track[j].lead_index(lead_list[i]);
         if(i_pnt < 0) continue;

         // Keep track of the TrackPoint count and sums
         pcnt++;
         psum += tarr.Track[j][i_pnt];
      }

      // Check for the minimum number of points
      if(pcnt < req) continue;

      // Compute the average point
      pavg = psum;
      pavg.set_lat(psum.lat()/pcnt);
      pavg.set_lon(psum.lon()/pcnt);
      pavg.set_v_max(nint(psum.v_max()/pcnt));
      pavg.set_mslp(nint(psum.mslp()/pcnt));

      // Compute the average winds
      for(j=0; j<NWinds; j++) {

         // Initialize the wind QuadInfo sum
         wavg = pavg[j];

         // Compute the average wind
         for(k=0; k<NQuadInfoValues; k++) wavg.set_value(k, nint(wavg[k]/pcnt));

         // Store the average wind
         pavg.set_wind(j, wavg);
      }

      // Compute consensus CycloneLevel
      pavg.set_level(wind_speed_to_cyclone_level(pavg.v_max()));

      // Add the current track point
      tavg.add(pavg);
   }

   // Return the consensus track
   return(tavg);
}

////////////////////////////////////////////////////////////////////////
