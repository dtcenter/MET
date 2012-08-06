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
   InitLine.clear();

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
   out << prefix << "InitLine:\n";
   InitLine.dump(out, indent_depth+1);
   
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
   InitLine      = t.InitLine;

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

   // Initialize the BDECK TrackInfo
   BDeck.set_basin(l.basin());
   BDeck.set_cyclone(l.cyclone());
   BDeck.set_storm_name(l.storm_name());
   BDeck.set_technique(l.bmodel());
   BDeck.set_init((unixtime) 0);

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

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPairInfo::add(const TCStatLine &l) {
   TrackPoint apoint, bpoint;
   TrackPoint *tp = (TrackPoint *) 0;
   QuadInfo wind;
   ConcatString cs;
   int i, j, k;
   
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
      tp->set_v_max(atof(l.get_item(cs)));
      cs << cs_erase << deck[i] << "MSLP";
      tp->set_mslp(atof(l.get_item(cs)));
      tp->set_level(string_to_cyclonelevel(l.get_item("LEVEL")));
      tp->set_watch_warn(string_to_watchwarntype(l.get_item("WATCH_WARN")));

      // Loop over the winds
      for(j=0; j<NWinds; j++) {

         // Initialize
         wind.clear();

         // Set the wind intensity and quadrant
         wind.set_intensity(WindIntensity[j]);
         cs << cs_erase << deck[i] << "QUAD_WIND_" << WindIntensity[j];
         wind.set_quadrant(string_to_quadranttype(l.get_item(cs)));

         // Set the wind radius values
         for(k=0; k<NQuadInfoValues; k++) {
            cs << cs_erase << deck[i] << "RAD" << k+1
               << "_WIND_" << WindIntensity[j];
            wind.set_value(k, atof(l.get_item(cs)));
         }

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

   // Store the TCStatLine for ADECK initialization time (lead == 0)
   if(apoint.lead() == 0) InitLine = l;

   return;
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
   NPairs = NAlloc  = 0;

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
