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

#include "track_pair_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TrackPairInfo
//
////////////////////////////////////////////////////////////////////////

TrackPairInfo::TrackPairInfo() {
  
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TrackPairInfo::~TrackPairInfo() {
  
   clear();
}

////////////////////////////////////////////////////////////////////////

TrackPairInfo::TrackPairInfo(const TrackPairInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

TrackPairInfo & TrackPairInfo::operator=(const TrackPairInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::init_from_scratch() {

   // Initialize pointers
   Line = (TCStatLine *) 0;
  
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::clear() {

   NPoints = 0;
   ADeck.clear();
   BDeck.clear();
   ADeckDLand.clear();
   BDeckDLand.clear();
   TrackErr.clear();
   XErr.clear();
   YErr.clear();
   AlongTrackErr.clear();
   CrossTrackErr.clear();
   Keep.clear();

   if(Line) { delete [] Line; Line = (TCStatLine *) 0; }
   NLines = NAlloc = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "NPoints = " << NPoints << "\n";
   out << prefix << "ADeck:\n";
   ADeck.dump(out, indent_depth+1);
   out << prefix << "BDeck:\n";
   BDeck.dump(out, indent_depth+1);
   out << prefix << "ADeckDLand:\n";
   ADeckDLand.dump(out, indent_depth+1);
   out << prefix << "BDeckDLand:\n";
   BDeckDLand.dump(out, indent_depth+1);
   out << prefix << "TrackErr:\n";
   TrackErr.dump(out, indent_depth+1);
   out << prefix << "XErr:\n";
   XErr.dump(out, indent_depth+1);
   out << prefix << "YErr:\n";
   YErr.dump(out, indent_depth+1);
   out << prefix << "AlongTrackErr:\n";
   AlongTrackErr.dump(out, indent_depth+1);
   out << prefix << "CrossTrackErr:\n";
   CrossTrackErr.dump(out, indent_depth+1);
   out << prefix << "Keep:\n";
   Keep.dump(out, indent_depth+1);
   out << prefix << "NLines = " << NLines << "\n";
   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackPairInfo::serialize() const {
   ConcatString s;

   s << "TrackPairInfo: "
     << "NPoints = " << NPoints;

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString TrackPairInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth), prefix2(indent_depth+1);
   ConcatString s;
   int i;

   s << prefix << "[" << n << "] " << serialize() << "\n";

   s << prefix2 << "ADeck = " << ADeck.serialize() << "\n"
     << prefix2 << "BDeck = " << BDeck.serialize() << "\n";

   for(i=0; i<NPoints; i++) {
     s << prefix2
       << "[" << i+1 << "] "
       << "ADeckDLand = " << ADeckDLand[i]
       << ", BDeckDLand = " << BDeckDLand[i]
       << ", TrackErr = " << TrackErr[i]
       << ", XErr = " << XErr[i]
       << ", YErr = " << YErr[i]
       << ", AlongTrackErr = " << AlongTrackErr[i]
       << ", CrossTrackErr = " << CrossTrackErr[i]
       << "\n";
   }
   
   return(s);
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::assign(const TrackPairInfo &t) {
   int i;

   clear();

   NPoints       = t.NPoints;
   ADeck         = t.ADeck;
   BDeck         = t.BDeck;
   ADeckDLand    = t.ADeckDLand;
   BDeckDLand    = t.BDeckDLand;
   TrackErr      = t.TrackErr;
   XErr          = t.XErr;
   YErr          = t.YErr;
   AlongTrackErr = t.AlongTrackErr;
   CrossTrackErr = t.CrossTrackErr;

   extend(t.NLines);

   for(i=0; i<t.NLines; i++) Line[i] = t.Line[i];

   NLines = t.NLines;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::extend(int n) {
   int j, k;
   TCStatLine *new_line = (TCStatLine *) 0;

   // Check if enough memory is already allocated
   if(NAlloc >= n) return;

   // Check how many allocations are required
   k = n/TrackPairLineAllocInc;
   if(n%TrackPairLineAllocInc) k++;
   n = k*TrackPairLineAllocInc;

   // Allocate a new TCStatLine array of the required length
   new_line = new TCStatLine [n];

   if(!new_line) {
      mlog << Error
           << "\nvoid TrackPairInfo::extend(int) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   // Copy the array contents and delete the old one
   if(Line) {
      for(j=0; j<NLines; j++) new_line[j] = Line[j];
      delete [] Line; Line = (TCStatLine *) 0;
   }

   // Point to the new array
   Line     = new_line;
   new_line = (TCStatLine *) 0;

   // Store the allocated length
   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::initialize(const TrackInfo &a, const TrackInfo &b) {

   // Initialize
   ADeck = a;
   BDeck = b;

   // Delete existing points
   ADeck.clear_points();
   BDeck.clear_points();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::initialize(const TCStatLine &l) {

   // Initialize the ADECK TrackInfo
   ADeck.set_basin(l.basin());
   ADeck.set_cyclone(l.cyclone());
   ADeck.set_storm_name(l.storm_name());
   ADeck.set_technique(l.amodel());
   ADeck.set_init(l.init());
   ADeck.set_storm_id();

   // Initialize the BDECK TrackInfo
   BDeck.set_basin(l.basin());
   BDeck.set_cyclone(l.cyclone());
   BDeck.set_storm_name(l.storm_name());
   BDeck.set_technique(l.bmodel());
   BDeck.set_init((unixtime) 0);
   BDeck.set_storm_id();
   
   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::set_keep(int i, int val) {

   Keep.set(i, val);

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::add(const TrackPoint &a, const TrackPoint &b,
                        double adland, double bdland,
                        double tkerr, double xerr, double yerr,
                        double altkerr, double crtkerr) {

   // Increment the count
   NPoints++;

   // Add the data
   ADeck.add(a);
   BDeck.add(b);
   ADeckDLand.add(adland);
   BDeckDLand.add(bdland);
   TrackErr.add(tkerr);
   XErr.add(xerr);
   YErr.add(yerr);
   AlongTrackErr.add(altkerr);
   CrossTrackErr.add(crtkerr);
   Keep.add(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::add(const TCStatLine &l) {
   TrackPoint apoint, bpoint;
   TrackPoint *tp = (TrackPoint *) 0;
   QuadInfo wind;
   ConcatString cs;
   int i, j;
   
   // Check the line type
   if(l.type() != TCStatLineType_TCMPR) return;

   // Increment the count
   NPoints++;

   // Initialize the ADECK/BDECK tracks
   if(NPoints == 1) initialize(l);
   
   // Loop over the ADECK/BDECK tracks
   const char deck[] = { 'A', 'B' };
   for(i=0; i<2; i++) {

      // Point at the current track point
      if(deck[i] == 'A') tp = &apoint;
      else               tp = &bpoint;

      // Populate the TrackPoint object
      tp->set_valid(l.valid());
      tp->set_lead(l.lead());
      cs << cs_erase << deck[i] << "LAT";
      tp->set_lat(atof(l.get_item(cs)));
      cs << cs_erase << deck[i] << "LON";
      tp->set_lon(atof(l.get_item(cs)));
      cs << cs_erase << deck[i] << "MAX_WIND";
      tp->set_v_max(atoi(l.get_item(cs)));
      cs << cs_erase << deck[i] << "MSLP";
      tp->set_mslp(atoi(l.get_item(cs)));
      tp->set_level(string_to_cyclonelevel(l.get_item("LEVEL")));
      tp->set_watch_warn(string_to_watchwarntype(l.get_item("WATCH_WARN")));

      // Loop over the winds
      for(j=0; j<NWinds; j++) {

         // Initialize
         wind.clear();

         // Set the wind intensity and quadrant
         wind.set_intensity(WindIntensity[j]);

         // Set the wind radius values
         cs << cs_erase << deck[i] << "AL_WIND_" << WindIntensity[j];
         wind.set_al_val(atof(l.get_item(cs)));
         cs << cs_erase << deck[i] << "NE_WIND_" << WindIntensity[j];
         wind.set_ne_val(atof(l.get_item(cs)));
         cs << cs_erase << deck[i] << "SE_WIND_" << WindIntensity[j];
         wind.set_se_val(atof(l.get_item(cs)));
         cs << cs_erase << deck[i] << "SW_WIND_" << WindIntensity[j];
         wind.set_sw_val(atof(l.get_item(cs)));
         cs << cs_erase << deck[i] << "NW_WIND_" << WindIntensity[j];
         wind.set_nw_val(atof(l.get_item(cs)));

         // Add wind to TrackInfo
         tp->set_wind(j, wind);
      } // end for j
   } // end for i
   
   // Add the TrackPoints to the ADECK/BDECK tracks
   ADeck.add(apoint);
   BDeck.add(bpoint);
   
   ADeckDLand.add(atof(l.get_item("ADLAND")));
   BDeckDLand.add(atof(l.get_item("BDLAND")));
   TrackErr.add(atof(l.get_item("TK_ERR")));
   XErr.add(atof(l.get_item("X_ERR")));
   YErr.add(atof(l.get_item("Y_ERR")));
   AlongTrackErr.add(atof(l.get_item("ALTK_ERR")));
   CrossTrackErr.add(atof(l.get_item("CRTK_ERR")));
   Keep.add(1);

   // Store the input line
   extend(NLines + 1);
   Line[NLines++] = l;

   return;
}

////////////////////////////////////////////////////////////////////////

int TrackPairInfo::i_init() const {
   int i, i_match;

   // Find the point where ADeck has lead = 0
   for(i=0, i_match=bad_data_int; i<NPoints; i++)
      if(ADeck[i].lead() == 0) break;

   if(i < NPoints) i_match = i;

   return(i_match);
}

////////////////////////////////////////////////////////////////////////

WatchWarnType TrackPairInfo::track_watch_warn() const {
   WatchWarnType ww_cur, ww_type;
   int i;

   // Initialize
   ww_type = ww_cur = NoWatchWarnType;
   
   // Loop over the track points and pick the watch/warning type of
   // most interest
   for(i=0; i<NPoints; i++) {
      ww_cur  = ww_max(ADeck[i].watch_warn(), BDeck[i].watch_warn());
      ww_type = ww_max(ww_type, ww_cur);
   }

   return(ww_type);
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::add_watch_warn(const ConcatString &storm_id,
                                   WatchWarnType ww_type,
                                   unixtime ww_ut) {

   // Add watch/warning information to the ADECK and BDECK tracks
   ADeck.add_watch_warn(storm_id, ww_type, ww_ut);
   BDeck.add_watch_warn(storm_id, ww_type, ww_ut);
   
   return;
}

////////////////////////////////////////////////////////////////////////
//
// Apply the water only logic to the track to determine which track
// points should be kept.  Once either the ADECK or BDECK track
// encounters land, discard the remainder of the track.
//
////////////////////////////////////////////////////////////////////////

int TrackPairInfo::check_water_only() {
   int i, n_rej;
   bool hit_land = false;

   // Loop over the track points
   for(i=0, n_rej=0; i<NPoints; i++) {

      // Check if the current track point is over land
      if((!is_bad_data(ADeckDLand[i]) && ADeckDLand[i] <= 0) ||
         (!is_bad_data(BDeckDLand[i]) && BDeckDLand[i] <= 0)) {
         hit_land = true;
      }

      // If the track has encountered land previously,
      // set keep status to false
      if(hit_land && Keep[i] != 0) {
         Keep.set(i, 0);
         n_rej++;
      }
   } // end for i

   return(n_rej);
}

////////////////////////////////////////////////////////////////////////
//
// Apply the rapid intensification logic to the track to determine
// which track points meet the criteria.  Set the keep status to false
// for those track points not meeting the criteria and return the number
// of points that were rejected.
//
////////////////////////////////////////////////////////////////////////

int TrackPairInfo::check_rapid_inten(const SingleThresh &st) {
   int i, j, n_rej;
   unixtime delta_ut;
   double cur_vmax, min_vmax;

   // Loop over the track points
   for(i=0, n_rej=0; i<NPoints; i++) {

      // Store the current wind speed maximum
      cur_vmax = BDeck[i].v_max();
      min_vmax = bad_data_double;

      // Search other track points for the minimum wind speed
      // in the time window
      for(j=0; j<NPoints; j++) {

         // Check if this point falls in the time window
         delta_ut = BDeck[i].valid() - BDeck[j].valid();
         if(delta_ut >= 0 &&
            delta_ut <= sec_per_hour*RapidIntenHourOffset) {

            // Check for the minimum wind speed
            if(is_bad_data(min_vmax) ||
               (!is_bad_data(BDeck[j].v_max()) &&
                BDeck[j].v_max() < min_vmax))
               min_vmax = BDeck[j].v_max();
         }
      } // end for j

      // If the vmax delta does not meet the threshold criteria,
      // set keep status to false
      if(!st.check(cur_vmax - min_vmax) && Keep[i] != 0) {
        Keep.set(i, 0);
        n_rej++;
      }
   }

   return(n_rej);
}

////////////////////////////////////////////////////////////////////////
//
// Apply the landfall logic to the track to determine which track
// points meet the criteria.  Set the keep status to false for those
// track points not meeting the criteria and return the number of
// points that were rejected.
//
////////////////////////////////////////////////////////////////////////

int TrackPairInfo::check_landfall(const int landfall_beg,
                                  const int landfall_end) {
   int i, n_rej;

   // Loop over the track points
   for(i=0, n_rej=0; i<NPoints; i++) {

      // If the current point is over land or landfall did not occur
      // in this time window, discard the point. Subtract and switch
      // the time window bounds since they are defined relative to
      // landfall, not the track point valid time.
      if(BDeckDLand[i] <= 0 ||
         !landfall_window(BDeck[i].valid() - landfall_end,
                          BDeck[i].valid() - landfall_beg)) {

         // Discard this point
         if(Keep[i] != 0) {
            Keep.set(i, 0);
            n_rej++;
         }
      }
   }

   return(n_rej);
}

////////////////////////////////////////////////////////////////////////
//
// Check to see if a landfall event occurred during the time window.
// Define landfall as the time of the last BEST track point before
// its distance to land switches from positive to negative.
//
////////////////////////////////////////////////////////////////////////

bool TrackPairInfo::landfall_window(unixtime beg_ut, unixtime end_ut) const {
   int i;
   bool found = false;

   // Loop over the track points looking for landfall.
   for(i=0; i<NPoints; i++) {

      // Skip bad data values or points outside of the time window
      if(is_bad_data(BDeckDLand[i]) ||
         BDeck[i].valid() < beg_ut ||
         BDeck[i].valid() > end_ut) continue;

      // Check for distance switching from positive to negative
      if(i+1 < NPoints &&
         !is_bad_data(BDeckDLand[i+1]) &&
         BDeckDLand[i]   >= 0 &&
         BDeckDLand[i+1] <= 0) {
         found = true;
         break;
      }
   }

   return(found);
}

////////////////////////////////////////////////////////////////////////
//
// Return a subset of the current track containing only those points
// marked for retention.
//
////////////////////////////////////////////////////////////////////////

TrackPairInfo TrackPairInfo::keep_subset() const {
   int i;
   TrackPairInfo tpi;

   // Loop over the points
   for(i=0; i<NLines; i++) {

      // Check the retention status
      if(Keep[i] == 0) continue;

      // Add the current track pair point
      tpi.add(Line[i]);
   }

   return(tpi);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TrackPairInfoArray
//
////////////////////////////////////////////////////////////////////////

TrackPairInfoArray::TrackPairInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TrackPairInfoArray::~TrackPairInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TrackPairInfoArray::TrackPairInfoArray(const TrackPairInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

TrackPairInfoArray & TrackPairInfoArray::operator=(const TrackPairInfoArray & t) {

   if(this == &t)  return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfoArray::init_from_scratch() {

   Pair = (TrackPairInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfoArray::clear() {

   if(Pair) { delete [] Pair; Pair = (TrackPairInfo *) 0; }
   NPairs = NAlloc = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "NPairs = " << NPairs << "\n";
   out << prefix << "NAlloc = " << NAlloc << "\n";

   for(i=0; i<NPairs; i++) {
      out << prefix << "TrackPairInfo[" << i+1 << "]:" << "\n";
      Pair[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackPairInfoArray::serialize() const {
   ConcatString s;

   s << "TrackPairInfoArray: "
     << "NPairs = " << NPairs
     << ", NAlloc = " << NAlloc;

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackPairInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << serialize() << ", Pairs:\n";

   for(i=0; i<NPairs; i++)
      s << Pair[i].serialize_r(i+1, indent_depth+1);

   return(s);

}

////////////////////////////////////////////////////////////////////////

void TrackPairInfoArray::assign(const TrackPairInfoArray &t) {
   int i;

   clear();

   if(t.NPairs == 0) return;

   extend(t.NPairs);

   for(i=0; i<t.NPairs; i++) Pair[i] = t.Pair[i];

   NPairs = t.NPairs;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfoArray::extend(int n) {
   int j, k;
   TrackPairInfo *new_info = (TrackPairInfo *) 0;

   // Check if enough memory is already allocated
   if(NAlloc >= n) return;

   // Check how many allocations are required
   k = n/TrackPairInfoAllocInc;
   if(n%TrackPairInfoAllocInc) k++;
   n = k*TrackPairInfoAllocInc;

   // Allocate a new TrackPairInfo array of the required length
   new_info = new TrackPairInfo [n];

   if(!new_info) {
      mlog << Error
           << "\nvoid TrackPairInfoArray::extend(int) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   // Copy the array contents and delete the old one
   if(Pair) {
      for(j=0; j<NPairs; j++) new_info[j] = Pair[j];
      delete [] Pair;  Pair = (TrackPairInfo *) 0;
   }

   // Point to the new array
   Pair     = new_info;
   new_info = (TrackPairInfo *) 0;

   // Store the allocated length
   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

const TrackPairInfo & TrackPairInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= NPairs)) {
      mlog << Error
           << "\nTrackPairInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Pair[n]);
}

////////////////////////////////////////////////////////////////////////

int TrackPairInfoArray::n_points() const {
   int i, n;

   for(i=0,n=0; i<NPairs; i++) n += Pair[i].adeck().n_points();

   return(n);
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfoArray::add(const TrackPairInfo &p) {

   extend(NPairs + 1);
   Pair[NPairs++] = p;

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfoArray::add_watch_warn(const ConcatString &ww_sid,
                                        WatchWarnType ww_type,
                                        unixtime ww_ut) {
   int i;

   // Loop through the track pairs
   for(i=0; i<NPairs; i++) Pair[i].add_watch_warn(ww_sid, ww_type, ww_ut);

   return;
}

///////////////////////////////////////////////////////////////////////
