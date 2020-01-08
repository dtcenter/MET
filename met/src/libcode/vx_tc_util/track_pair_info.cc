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
   ADeckRIRW.clear();
   BDeckRIRW.clear();
   ADeckPrvInt.clear();
   BDeckPrvInt.clear();
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
   out << prefix << "ADeckRIRW:\n";
   ADeckRIRW.dump(out, indent_depth+1);
   out << prefix << "BDeckRIRW:\n";
   BDeckRIRW.dump(out, indent_depth+1);
   out << prefix << "ADeckPrvInt:\n";
   ADeckPrvInt.dump(out, indent_depth+1);
   out << prefix << "BDeckPrvInt:\n";
   BDeckPrvInt.dump(out, indent_depth+1);
   out << prefix << "Keep:\n";
   Keep.dump(out, indent_depth+1);
   out << prefix << "NLines = " << NLines << "\n";
   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString TrackPairInfo::case_info() const {
   ConcatString s;

   s << "TrackPairInfo: STORMID = " << ADeck.storm_id()
     << ", ADECK = " << ADeck.technique()
     << ", BDECK = " << BDeck.technique()
     << ", INIT = " << unix_to_yyyymmdd_hhmmss(ADeck.init())
     << ", NPoints = " << NPoints;

   return(s);
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
   ADeckRIRW     = t.ADeckRIRW;
   BDeckRIRW     = t.BDeckRIRW;
   ADeckPrvInt   = t.ADeckPrvInt;
   BDeckPrvInt   = t.BDeckPrvInt;
   Keep          = t.Keep;

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
   ADeck.set_storm_id(l.storm_id());
   ADeck.set_basin(l.basin());
   ADeck.set_cyclone(l.cyclone());
   ADeck.set_storm_name(l.storm_name());
   ADeck.set_technique(l.amodel());
   ADeck.set_initials(l.initials());
   ADeck.set_init(l.init());

   // Initialize the BDECK TrackInfo
   BDeck.set_storm_id(l.storm_id());
   BDeck.set_basin(l.basin());
   BDeck.set_cyclone(l.cyclone());
   BDeck.set_storm_name(l.storm_name());
   BDeck.set_technique(l.bmodel());
   BDeck.set_initials(l.initials());
   BDeck.set_init((unixtime) 0);

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
   ADeckRIRW.add(bad_data_double);
   BDeckRIRW.add(bad_data_double);
   ADeckPrvInt.add(bad_data_double);
   BDeckPrvInt.add(bad_data_double);
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
      tp->set_lat(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "LON";
      tp->set_lon(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "MAX_WIND";
      tp->set_v_max(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "MSLP";
      tp->set_mslp(atof(l.get_item(cs.c_str())));
      tp->set_level(string_to_cyclonelevel(l.get_item("LEVEL")));
      cs << cs_erase << deck[i] << "RADP";
      tp->set_radp(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "RRP";
      tp->set_rrp(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "MRD";
      tp->set_mrd(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "GUSTS";
      tp->set_gusts(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "EYE";
      tp->set_eye(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "DIR";
      tp->set_direction(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "SPEED";
      tp->set_speed(atof(l.get_item(cs.c_str())));
      cs << cs_erase << deck[i] << "DEPTH";
      tp->set_eye(string_to_systemsdepth(l.get_item(cs.c_str())));
      tp->set_watch_warn(string_to_watchwarntype(l.get_item("WATCH_WARN")));

      // Loop over the winds
      for(j=0; j<NWinds; j++) {

         // Initialize
         wind.clear();

         // Set the wind intensity and quadrant
         wind.set_intensity(WindIntensity[j]);

         // Set the wind radius values
         cs << cs_erase << deck[i] << "AL_WIND_" << WindIntensity[j];
         wind.set_al_val(atof(l.get_item(cs.c_str())));
         cs << cs_erase << deck[i] << "NE_WIND_" << WindIntensity[j];
         wind.set_ne_val(atof(l.get_item(cs.c_str())));
         cs << cs_erase << deck[i] << "SE_WIND_" << WindIntensity[j];
         wind.set_se_val(atof(l.get_item(cs.c_str())));
         cs << cs_erase << deck[i] << "SW_WIND_" << WindIntensity[j];
         wind.set_sw_val(atof(l.get_item(cs.c_str())));
         cs << cs_erase << deck[i] << "NW_WIND_" << WindIntensity[j];
         wind.set_nw_val(atof(l.get_item(cs.c_str())));

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
   ADeckRIRW.add(bad_data_double);
   BDeckRIRW.add(bad_data_double);
   ADeckPrvInt.add(bad_data_double);
   BDeckPrvInt.add(bad_data_double);
   Keep.add(1);

   // Store the input line
   extend(NLines + 1);
   Line[NLines++] = l;

   return;
}

////////////////////////////////////////////////////////////////////////

unixtime TrackPairInfo::valid(int i) const {
   unixtime t;

   // Use the ADeck valid time, if defined.
   t = (ADeck[i].valid() > 0 ? ADeck[i].valid() : BDeck[i].valid());

   return(t);
}

////////////////////////////////////////////////////////////////////////

int TrackPairInfo::i_init() const {
   int i, i_match;

   // Find the point where the valid time matches the ADECK
   // initialization time
   for(i=0, i_match=bad_data_int; i<NPoints; i++) {
      if(ADeck.init() == valid(i)) break;
   }

   if(i < NPoints) i_match = i;

   return(i_match);
}

////////////////////////////////////////////////////////////////////////

WatchWarnType TrackPairInfo::watch_warn(int i) const {
   WatchWarnType ww_type = NoWatchWarnType;

   // Only check points common to both the ADECK and BDECK tracks
   if(!is_bad_data(ADeck[i].lat()) && !is_bad_data(ADeck[i].lon()) &&
      !is_bad_data(BDeck[i].lat()) && !is_bad_data(BDeck[i].lon())) {
      ww_type = ww_max(ADeck[i].watch_warn(), BDeck[i].watch_warn());
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

      // Check if the current track point is over land.  Require that
      // both ADECK and BDECK values are present.
      if(!is_bad_data(ADeckDLand[i]) && !is_bad_data(BDeckDLand[i]) &&
         (ADeckDLand[i] <= 0 || BDeckDLand[i] <= 0)) {
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

int TrackPairInfo::check_rirw(const TrackType track_type,
                              const int sec_adeck, const int sec_bdeck,
                              const bool exact_adeck, const bool exact_bdeck,
                              const SingleThresh &st_adeck, const SingleThresh &st_bdeck) {
   int i, j, n_rej;
   unixtime delta_ut;
   int acur, aprv, bcur, bprv;

   // Nothing to do.
   if(track_type == TrackType_None) return(0);

   // Check threshold type for non-exact intensity differences.
   if(!exact_adeck &&
      st_adeck.get_type() != thresh_lt && st_adeck.get_type() != thresh_le &&
      st_adeck.get_type() != thresh_gt && st_adeck.get_type() != thresh_ge) {
      mlog << Error << "\ncheck_rirw() -> "
           << "for non-exact intensity differences the RI/RW threshold ("
           << st_adeck.get_str() << ") must be of type <, <=, >, or >=.\n\n";
      exit(1);
   }

   // Check threshold type for non-exact intensity differences.
   if(!exact_bdeck &&
      st_bdeck.get_type() != thresh_lt && st_bdeck.get_type() != thresh_le &&
      st_bdeck.get_type() != thresh_gt && st_bdeck.get_type() != thresh_ge) {
      mlog << Error << "\ncheck_rirw() -> "
           << "for non-exact intensity differences the RI/RW threshold ("
           << st_bdeck.get_str() << ") must be of type <, <=, >, or >=.\n\n";
      exit(1);
   }

   // Loop over the track points
   for(i=0, n_rej=0; i<NPoints; i++) {

      // Initialize
      ADeckRIRW.set(i, bad_data_double);
      BDeckRIRW.set(i, bad_data_double);
      ADeckPrvInt.set(i, bad_data_double);
      BDeckPrvInt.set(i, bad_data_double);

      // Store the current wind speed maximum
      acur = ADeck[i].v_max();
      bcur = BDeck[i].v_max();
      aprv = bad_data_int;
      bprv = bad_data_int;

      // Search other track points for previous times
      for(j=0; j<NPoints; j++) {

         // Compute time offset
         delta_ut = valid(i) - valid(j);

         // Skip the current and future track points.
         if(delta_ut <= 0) continue;

         // Check for an exact ADeck time difference.
         if(exact_adeck && delta_ut == sec_adeck) aprv = ADeck[j].v_max();

         // Check all ADeck points in the time window
         if(!exact_adeck && delta_ut <= sec_adeck) {

            // Initialize the previous intensity.
            if(is_bad_data(aprv)) aprv = ADeck[j].v_max();

            // Update the previous ADeck intensity.
            if(!is_bad_data(aprv) && !is_bad_data(ADeck[j].v_max())) {

               // For greater-than type thresholds, find the minimum value in the window.
               if((st_adeck.get_type() == thresh_gt || st_adeck.get_type() == thresh_ge) &&
                  ADeck[j].v_max() < aprv) aprv = ADeck[j].v_max();

               // For less-than type thresholds, find the maximum value in the window.
               if((st_adeck.get_type() == thresh_lt || st_adeck.get_type() == thresh_le) &&
                  ADeck[j].v_max() > aprv) aprv = ADeck[j].v_max();
            }
         }

         // Check for an exact BDeck time difference.
         if(exact_bdeck && delta_ut == sec_bdeck) bprv = BDeck[j].v_max();

         // Check all BDeck points in the time window
         if(!exact_bdeck && delta_ut <= sec_bdeck) {

            // Initialize the previous intensity.
            if(is_bad_data(bprv)) bprv = BDeck[j].v_max();

            // Update the previous BDeck intensity.
            if(!is_bad_data(bprv) && !is_bad_data(BDeck[j].v_max())) {

               // For greater-than type thresholds, find the minimum value in the window.
               if((st_bdeck.get_type() == thresh_gt || st_bdeck.get_type() == thresh_ge) &&
                  BDeck[j].v_max() < bprv) bprv = BDeck[j].v_max();

               // For less-than type thresholds, find the maximum value in the window.
               if((st_bdeck.get_type() == thresh_lt || st_bdeck.get_type() == thresh_le) &&
                  BDeck[j].v_max() > bprv) bprv = BDeck[j].v_max();
            }
         }
      } // end for j

      // Apply ADeck logic
      if(!is_bad_data(acur) && !is_bad_data(aprv)) { 
         if(st_adeck.check(acur - aprv)) ADeckRIRW.set(i, 1);
         else                            ADeckRIRW.set(i, 0);
      }

      // Apply BDeck logic
      if(!is_bad_data(bcur) && !is_bad_data(bprv)) {
         if(st_bdeck.check(bcur - bprv)) BDeckRIRW.set(i, 1);
         else                            BDeckRIRW.set(i, 0);
      }

      // Store the previous vmax values
      ADeckPrvInt.set(i, aprv);
      BDeckPrvInt.set(i, bprv);

      // Print debug message when rapid intensification is found
      if(is_eq(ADeckRIRW[i], 1.0) &&
         (track_type == TrackType_ADeck || track_type == TrackType_Both)) {
         mlog << Debug(4)
              << "Found ADECK RI/RW: " << case_info()
              << ", VALID = " << unix_to_yyyymmdd_hhmmss(ADeck[i].valid()) << ", "
              << "LEAD = " << (is_bad_data(ADeck[i].lead()) ? na_str : sec_to_hhmmss(ADeck[i].lead()).text()) << ", "
              << sec_to_hhmmss(sec_adeck)
              << (exact_adeck ? " exact " : " maximum " ) << "ADECK change "
              << aprv << " to " << acur << ", "
              << acur - aprv << st_adeck.get_str() << "\n";
      }
      if(is_eq(BDeckRIRW[i], 1.0) &&
         (track_type == TrackType_BDeck || track_type == TrackType_Both)) {
         mlog << Debug(4)
              << "Found BDECK RI/RW: " << case_info()
              << ", VALID = " << unix_to_yyyymmdd_hhmmss(BDeck[i].valid()) << ", "
              << "LEAD = " << (is_bad_data(BDeck[i].lead()) ? na_str : sec_to_hhmmss(BDeck[i].lead()).text()) << ", "
              << sec_to_hhmmss(sec_bdeck)
              << (exact_bdeck ? " exact " : " maximum " ) << "BDECK change "
              << bprv << " to " << bcur << ", "
              << bcur - bprv << st_bdeck.get_str() << "\n";
      }

      // Don't increment counts for track points we're not already keeping
      if(!Keep[i]) continue;

      // Update the keep status
      if((track_type == TrackType_ADeck && !is_eq(ADeckRIRW[i], 1.0)) ||
         (track_type == TrackType_BDeck && !is_eq(BDeckRIRW[i], 1.0)) ||
         (track_type == TrackType_Both  && !is_eq(ADeckRIRW[i], 1.0) && !is_eq(BDeckRIRW[i], 1.0))) {
         Keep.set(i, 0);
         n_rej++;
      }
   } // end for i

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
