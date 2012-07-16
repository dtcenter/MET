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

#include "vx_math.h"

#include "track_point.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class QuadInfo
//
////////////////////////////////////////////////////////////////////////

QuadInfo::QuadInfo() {
  
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

QuadInfo::~QuadInfo() {
  
   clear();
}

////////////////////////////////////////////////////////////////////////

QuadInfo::QuadInfo(const QuadInfo &t) {
  
   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

QuadInfo & QuadInfo::operator=(const QuadInfo &t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

QuadInfo & QuadInfo::operator+=(const QuadInfo &t) {
   int i;

   // Check intensity
   if(is_bad_data(Intensity)) Intensity = t.intensity();
   else if(Intensity != t.intensity()) {
      mlog << Error
           << "\nQuadInfo::operator+=(const QuadInfo &t) -> "
           << "cannot call += for two different intensity values ("
           << Intensity << " != " << t.intensity() << ").\n\n";
      exit(1);
   }

   // Check quadrant
   if(Quadrant == NoQuadrantType) Quadrant = t.quadrant();
   else if(Quadrant != t.quadrant()) {
      mlog << Error
           << "\nQuadInfo::operator+=(const QuadInfo &t) -> "
           << "cannot call += for two different quadrants ("
           << quadranttype_to_string(Quadrant) << " != "
           << quadranttype_to_string(t.quadrant()) << ").\n\n";
      exit(1);
   }

   // Increment counts
   for(i=0; i<NQuadInfoValues; i++) Value[i] += t[i];

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::init_from_scratch() {

   // Only initialize Intensity here
   Intensity = bad_data_int;
  
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::clear() {
   int i;

   Quadrant  = NoQuadrantType;

   // Initialize the values to 0 rather than bad data
   for(i=0; i<NQuadInfoValues; i++) Value[i] = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "Intensity = " << Intensity << "\n";
   out << prefix << "Quadrant  = " << quadranttype_to_string(Quadrant) << "\n";
   for(i=0; i<NQuadInfoValues; i++)
      out << prefix << "Value[" << i+1 << "]  = " << Value[i] << "\n";
   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString QuadInfo::serialize() const {
   ConcatString s;
   int i;

   s << "QuadInfo: "
     << "Intensity = " << Intensity
     << ", Quadrant = " << quadranttype_to_string(Quadrant);
   for(i=0; i<NQuadInfoValues; i++)
      s << ", Value[" << i+1 << "] = " << Value[i];

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString QuadInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;

   s << prefix << "[" << n << "] " << serialize() << "\n";

   return(s);
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::assign(const QuadInfo &t) {
   int i;

   clear();

   Intensity = t.Intensity;
   Quadrant  = t.Quadrant;
   for(i=0; i<NQuadInfoValues; i++) Value[i] = t[i];
   
   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::set_wind(const TrackLine &l) {

   // Return if Intensity doesn't match TrackLine intensity
   if(Intensity != l.wind_intensity()) return;
  
   clear();

   Quadrant  = l.quadrant();
   Value[0]  = l.radius1();
   Value[1]  = l.radius2();
   Value[2]  = l.radius3();
   Value[3]  = l.radius4();

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::set_seas(const TrackLine &l) {

   // Return if Intensity doesn't match TrackLine wave height
   if(Intensity != l.wave_height()) return;;
  
   clear();

   Intensity = l.wave_height();
   Quadrant  = l.seas_code();
   Value[0]  = l.seas_radius1();
   Value[1]  = l.seas_radius2();
   Value[2]  = l.seas_radius3();
   Value[3]  = l.seas_radius4();

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::set_value(int n, int v) {

   // Check the range
   if((n < 0) || (n >= NQuadInfoValues)) {
      mlog << Error
           << "\nQuadInfo::set_value(int, int) -> "
           << "range check error (" << n << ").\n\n";
      exit(1);
   }

   // Set the value
   Value[n] = v;

   return;
}

////////////////////////////////////////////////////////////////////////

const int QuadInfo::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= NQuadInfoValues)) {
      mlog << Error
           << "\nQuadInfo::operator[](int) -> "
           << "range check error (" << n << ").\n\n";
      exit(1);
   }

   return(Value[n]);
}

////////////////////////////////////////////////////////////////////////

bool QuadInfo::has_wind(const TrackLine &l) const {

   return(is_match_wind(l));
}

////////////////////////////////////////////////////////////////////////

bool QuadInfo::has_seas(const TrackLine &l) const {

   return(is_match_seas(l));
}

////////////////////////////////////////////////////////////////////////

bool QuadInfo::is_match_wind(const TrackLine &l) const {
   bool match = true;

   // Check storm and model info
   if(Intensity != l.wind_intensity() ||
      Quadrant  != l.quadrant()       ||
      Value[0]  != l.radius1()        ||
      Value[1]  != l.radius2()        ||
      Value[2]  != l.radius3()        ||
      Value[3]  != l.radius4())
      match = false;

   return(match);
}

////////////////////////////////////////////////////////////////////////

bool QuadInfo::is_match_seas(const TrackLine &l) const {
   bool match = true;

   // Check storm and model info
   if(Intensity != l.wind_intensity() ||
      Quadrant  != l.seas_code()      ||
      Value[0]  != l.seas_radius1()   ||
      Value[1]  != l.seas_radius2()   ||
      Value[2]  != l.seas_radius3()   ||
      Value[3]  != l.seas_radius4())
      match = false;

   return(match);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TrackPoint
//
////////////////////////////////////////////////////////////////////////

TrackPoint::TrackPoint() {
  
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TrackPoint::~TrackPoint() {
  
   clear();
}

////////////////////////////////////////////////////////////////////////

TrackPoint::TrackPoint(const TrackPoint &p) {
  
   init_from_scratch();

   assign(p);
}

////////////////////////////////////////////////////////////////////////

TrackPoint & TrackPoint::operator=(const TrackPoint &p) {

   if(this == &p) return(*this);

   assign(p);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

TrackPoint & TrackPoint::operator+=(const TrackPoint &p) {
   int i;
  
   // Check valid time
   if(ValidTime == (unixtime) 0) ValidTime = p.valid();
   else if(ValidTime != p.valid()) {
      mlog << Error
           << "\nTrackPoint::operator+=(const TrackPoint &p) -> "
           << "cannot call += for two different valid times ("
           << unix_to_yyyymmdd_hhmmss(ValidTime) << " != "
           << unix_to_yyyymmdd_hhmmss(p.valid()) << ").\n\n";
      exit(1);
   }

   // Check lead time
   if(is_bad_data(LeadTime)) LeadTime = p.lead();
   else if(LeadTime != p.lead()) {
      mlog << Error
           << "\nTrackPoint::operator+=(const TrackPoint &p) -> "
           << "cannot call += for two different lead times ("
           << sec_to_hhmmss(LeadTime) << " != "
           << sec_to_hhmmss(p.lead()) << ").\n\n";
      exit(1);
   }
   
   // Increment counts
   if(is_bad_data(Lat))  Lat   = p.lat();
   else                  Lat  += p.lat();
   if(is_bad_data(Lon))  Lon   = p.lon();
   else                  Lon  += p.lon();
   if(is_bad_data(Vmax)) Vmax  = p.v_max();
   else                  Vmax += p.v_max();
   if(is_bad_data(MSLP)) MSLP  = p.mslp();
   else                  MSLP += p.mslp();

   for(i=0; i<NWinds; i++) Wind[i] += p[i];

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::init_from_scratch() {
  
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::clear() {
   int i;

   IsSet     = false;
  
   ValidTime = (unixtime) 0;
   LeadTime  = bad_data_int;
   Lat       = bad_data_double;
   Lon       = bad_data_double;
   Vmax      = bad_data_int;
   MSLP      = bad_data_int;
   Level     = NoCycloneLevel;

   // Call clear for each Wind object and then set intensity value
   for(i=0; i<NWinds; i++) {
      Wind[i].clear();
      Wind[i].set_intensity(WindIntensity[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "ValidTime = " << unix_to_yyyymmdd_hhmmss(ValidTime) << "\n";
   out << prefix << "LeadTime  = " << sec_to_hhmmss(LeadTime) << "\n";
   out << prefix << "Lat       = " << Lat << "\n";
   out << prefix << "Lon       = " << Lon << "\n";
   out << prefix << "Vmax      = " << Vmax << "\n";
   out << prefix << "MSLP      = " << MSLP << "\n";
   out << prefix << "Level     = " << cyclonelevel_to_string(Level) << "\n";

   for(i=0; i<NWinds; i++) {
      out << prefix << "Wind[" << i+1 << "]:" << "\n";
      Wind[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString TrackPoint::serialize() const {
   ConcatString s;

   s << "TrackPoint: "
     << "ValidTime = " << unix_to_yyyymmdd_hhmmss(ValidTime)
     << ", LeadTime = " << sec_to_hhmmss(LeadTime)
     << ", Lat = " << Lat
     << ", Lon = " << Lon
     << ", Vmax = " << Vmax
     << ", MSLP = " << MSLP
     << ", Level = " << cyclonelevel_to_string(Level);

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString TrackPoint::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << "[" << n << "] " << serialize() << ", Winds:\n";

   for(i=0; i<NWinds; i++)
      s << Wind[i].serialize_r(i+1, indent_depth+1);

   return(s);
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::assign(const TrackPoint &t) {
   int i;
  
   clear();

   IsSet     = true;
   
   ValidTime = t.ValidTime;
   LeadTime  = t.LeadTime;
   Lat       = t.Lat;
   Lon       = t.Lon;
   Vmax      = t.Vmax;
   MSLP      = t.MSLP;
   Level     = t.Level;
   
   for(i=0; i<NWinds; i++) Wind[i] = t.Wind[i];
   
   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::initialize(const TrackLine &l) {

   IsSet     = true;

   ValidTime = l.valid();
   LeadTime  = l.lead();
   Lat       = l.lat();
   Lon       = l.lon();
   Vmax      = l.v_max();
   MSLP      = l.mslp();
   Level     = l.level();

   return;
}

////////////////////////////////////////////////////////////////////////

const QuadInfo & TrackPoint::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= NWinds)) {
      mlog << Error
           << "\nTrackPoint::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Wind[n]);
}

////////////////////////////////////////////////////////////////////////

bool TrackPoint::set(const TrackLine &l) {
   int i;

   // Initialize TrackPoint with TrackLine, if necessary
   if(!IsSet) initialize(l);

   // Attempt to set each WindInfo object with TrackLine
   for(i=0; i<NWinds; i++) Wind[i].set_wind(l);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::set_wind(int n, const QuadInfo &w) {

   // Check the range
   if((n < 0) || (n >= NQuadInfoValues)) {
      mlog << Error
           << "\nQuadInfo::set_wind(int, const QuadInfo) -> "
           << "range check error (" << n << ").\n\n";
      exit(1);
   }

   // Set the value
   Wind[n] = w;

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackPoint::has(const TrackLine &l) const {
   int found = false;
   int i;

   // Check if the QuadInfo data matches
   for(i=0; i<NWinds; i++) {
      if(Wind[i].has_wind(l)) {
         found = true;
         break;
      }
   }

   // Return whether the QuadInfo matches and the TrackPoint matches
   return(found && is_match(l));
}

////////////////////////////////////////////////////////////////////////

bool TrackPoint::is_match(const TrackLine &l) const {
   bool match = true;

   // Check storm and model info
   if(ValidTime != l.valid()     ||
      LeadTime  != l.lead()      ||
      Lat       != l.lat()       ||
      Lon       != l.lon()       ||
      Vmax      != l.v_max()     ||
      MSLP      != l.mslp()      ||
      Level     != l.level())
      match = false;

   return(match);
}

////////////////////////////////////////////////////////////////////////
