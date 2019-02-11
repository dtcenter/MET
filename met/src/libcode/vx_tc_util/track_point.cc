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

   // Check intensity
   if(is_bad_data(Intensity)) Intensity = t.intensity();
   else if(Intensity != t.intensity()) {
      mlog << Error
           << "\nQuadInfo::operator+=(const QuadInfo &t) -> "
           << "cannot call += for two different intensity values ("
           << Intensity << " != " << t.intensity() << ").\n\n";
      exit(1);
   }

   // Increment counts
   if(is_bad_data(ALVal) || is_bad_data(t.ALVal)) ALVal  = bad_data_double;
   else                                           ALVal += t.ALVal;
   if(is_bad_data(NEVal) || is_bad_data(t.NEVal)) NEVal  = bad_data_double;
   else                                           NEVal += t.NEVal;
   if(is_bad_data(SEVal) || is_bad_data(t.SEVal)) SEVal  = bad_data_double;
   else                                           SEVal += t.SEVal;
   if(is_bad_data(NWVal) || is_bad_data(t.NWVal)) NWVal  = bad_data_double;
   else                                           NWVal += t.NWVal;

   return(*this);
}

////////////////////////////////////////////////////////////////////////

bool QuadInfo::operator==(const QuadInfo &t) const {

   return(Intensity == t.Intensity &&
          is_eq(ALVal, t.ALVal)    &&
          is_eq(NEVal, t.NEVal)    &&
          is_eq(SEVal, t.SEVal)    &&
          is_eq(SWVal, t.SWVal)    &&
          is_eq(NWVal, t.NWVal));
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

   ALVal = bad_data_double;
   NEVal = bad_data_double;
   SEVal = bad_data_double;
   SWVal = bad_data_double;
   NWVal = bad_data_double;

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "Intensity = " << Intensity << "\n";
   out << prefix << "ALVal     = " << ALVal << "\n";
   out << prefix << "NEVal     = " << NEVal << "\n";
   out << prefix << "SEVal     = " << SEVal << "\n";
   out << prefix << "SWVal     = " << SWVal << "\n";
   out << prefix << "NWVal     = " << NWVal << "\n";
   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString QuadInfo::serialize() const {
   ConcatString s;

   s << "QuadInfo: "
     << "Intensity = " << Intensity
     << ", ALVal = " << ALVal
     << ", NEVal = " << NEVal
     << ", SEVal = " << SEVal
     << ", SWVal = " << SWVal
     << ", NWVal = " << NWVal;

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

   clear();

   Intensity = t.Intensity;
   ALVal     = t.ALVal;
   NEVal     = t.NEVal;
   SEVal     = t.SEVal;
   SWVal     = t.SWVal;
   NWVal     = t.NWVal;

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::set_wind(const ATCFTrackLine &l) {

   // If intensity is not yet set, retrieve it from the ATCFTrackLine
   if(is_bad_data(Intensity)) {
      Intensity = l.wind_intensity();
   }

   // Return if intensity doesn't match ATCFTrackLine intensity
   if(Intensity != l.wind_intensity()) return;

   clear();

   set_quad_vals(l.quadrant(),
                 l.radius1(), l.radius2(),
                 l.radius3(), l.radius4());

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::set_seas(const ATCFTrackLine &l) {

   // Return if Intensity doesn't match ATCFTrackLine wave height
   if(Intensity != l.wave_height()) return;

   clear();

   set_quad_vals(l.seas_code(),
                 l.seas_radius1(), l.seas_radius2(),
                 l.seas_radius3(), l.seas_radius4());

   return;
}

////////////////////////////////////////////////////////////////////////

void QuadInfo::set_quad_vals(QuadrantType ref_quad,
                             int rad1, int rad2, int rad3, int rad4) {

   // Switch logic based on the reference quadrant, counting clockwise.
   switch(ref_quad) {

     // Full circle radius is stored in the first radius
     case(FullCircle):
        ALVal = rad1;
        break;

     case(NE_Quadrant):
        NEVal = rad1;
        SEVal = rad2;
        SWVal = rad3;
        NWVal = rad4;
        break;

     case(SE_Quadrant):
        NEVal = rad4;
        SEVal = rad1;
        SWVal = rad2;
        NWVal = rad3;
        break;

     case(SW_Quadrant):
        NEVal = rad3;
        SEVal = rad4;
        SWVal = rad1;
        NWVal = rad2;
        break;

     case(NW_Quadrant):
        NEVal = rad2;
        SEVal = rad3;
        SWVal = rad4;
        NWVal = rad1;
        break;

     // Nothing to do
     case(NoQuadrantType):
        break;

     default:
       mlog << Error
            << "\nQuadInfo::set_quad_vals() -> "
            << "unexpected quadrant type encountered \""
            << quadranttype_to_string(ref_quad) << "\".\n\n";
       exit(1);
       break;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool QuadInfo::is_match_wind(const ATCFTrackLine &l) const {
   QuadInfo qi;

   // Parse the line into a QuadInfo object
   qi.set_wind(l);

   return(*this == qi);
}

////////////////////////////////////////////////////////////////////////

bool QuadInfo::is_match_seas(const ATCFTrackLine &l) const {
   QuadInfo qi;

   // Parse the line into a QuadInfo object
   qi.set_seas(l);

   return(*this == qi);
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
   if(is_bad_data(Lat)   || is_bad_data(p.lat()))   Lat    = bad_data_double;
   else                                             Lat   += p.lat();
   if(is_bad_data(Lon)   || is_bad_data(p.lon()))   Lon    = bad_data_double;
   else                                             Lon   += p.lon();
   if(is_bad_data(Vmax)  || is_bad_data(p.v_max())) Vmax   = bad_data_double;
   else                                             Vmax  += p.v_max();
   if(is_bad_data(MSLP)  || is_bad_data(p.mslp()))  MSLP   = bad_data_double;
   else                                             MSLP  += p.mslp();
   if(is_bad_data(RadP)  || is_bad_data(p.radp()))  RadP   = bad_data_double;
   else                                             RadP  += p.radp();
   if(is_bad_data(RRP)   || is_bad_data(p.rrp()))   RRP    = bad_data_double;
   else                                             RRP   += p.rrp();
   if(is_bad_data(MRD)   || is_bad_data(p.mrd()))   MRD    = bad_data_double;
   else                                             MRD   += p.mrd();
   if(is_bad_data(Gusts) || is_bad_data(p.gusts())) Gusts  = bad_data_double;
   else                                             Gusts += p.gusts();
   if(is_bad_data(Eye)   || is_bad_data(p.eye()))   Eye    = bad_data_double;
   else                                             Eye   += p.eye();
   if(is_bad_data(Speed) || is_bad_data(p.speed())) Speed  = bad_data_double;
   else                                             Speed += p.speed();

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
   RadP      = bad_data_double;
   RRP       = bad_data_double;
   MRD       = bad_data_double;
   Gusts     = bad_data_double;
   Eye       = bad_data_double;
   Direction = bad_data_double;
   Speed     = bad_data_double;
   Depth     = NoSystemsDepth;
   WatchWarn = NoWatchWarnType;

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

   out << prefix << "ValidTime = " << (ValidTime > 0 ? unix_to_yyyymmdd_hhmmss(ValidTime).text() : na_str) << "\n";
   out << prefix << "LeadTime  = " << (!is_bad_data(LeadTime) ? sec_to_hhmmss(LeadTime).text() : na_str) << "\n";
   out << prefix << "Lat       = " << Lat << "\n";
   out << prefix << "Lon       = " << Lon << "\n";
   out << prefix << "Vmax      = " << Vmax << "\n";
   out << prefix << "MSLP      = " << MSLP << "\n";
   out << prefix << "Level     = " << cyclonelevel_to_string(Level) << "\n";
   out << prefix << "RadP      = " << RadP << "\n";
   out << prefix << "RRP       = " << RRP << "\n";
   out << prefix << "MRD       = " << MRD << "\n";
   out << prefix << "Gusts     = " << Gusts << "\n";
   out << prefix << "Eye       = " << Eye << "\n";
   out << prefix << "Direction = " << Direction << "\n";
   out << prefix << "Speed     = " << Speed << "\n";
   out << prefix << "Depth     = " << systemsdepth_to_string(Depth) << "\n";

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
     << "ValidTime = " << (ValidTime > 0 ? unix_to_yyyymmdd_hhmmss(ValidTime).text() : na_str)
     << ", LeadTime = " << (!is_bad_data(LeadTime) ? sec_to_hhmmss(LeadTime).text() : na_str)
     << ", Lat = " << Lat
     << ", Lon = " << Lon
     << ", Vmax = " << Vmax
     << ", MSLP = " << MSLP
     << ", Level = " << cyclonelevel_to_string(Level)
     << ", RadP = " << RadP
     << ", RRP = " << RRP
     << ", MRD = " << MRD
     << ", Gusts = " << Gusts
     << ", Eye = " << Eye
     << ", Direction = " << Direction
     << ", Speed = " << Speed
     << ", Depth = " << systemsdepth_to_string(Depth)
     << ", WatchWarn = " << watchwarntype_to_string(WatchWarn);

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
   RadP      = t.RadP;
   RRP       = t.RRP;
   MRD       = t.MRD;
   Gusts     = t.Gusts;
   Eye       = t.Eye;
   Direction = t.Direction;
   Speed     = t.Speed;
   Depth     = t.Depth;
   WatchWarn = t.WatchWarn;

   for(i=0; i<NWinds; i++) Wind[i] = t.Wind[i];

   return;
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::initialize(const ATCFTrackLine &l) {

   IsSet     = true;

   ValidTime = l.valid();
   LeadTime  = l.lead();
   Lat       = l.lat();
   Lon       = l.lon();
   Vmax      = l.v_max();
   MSLP      = l.mslp();
   Level     = l.level();
   RadP      = l.isobar_pressure();
   RRP       = l.isobar_radius();
   MRD       = l.max_wind_radius();
   Gusts     = l.gusts();
   Eye       = l.eye_diameter();
   Direction = l.storm_direction();
   Speed     = l.storm_speed();
   Depth     = l.depth();

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

void TrackPoint::set_watch_warn(WatchWarnType ww_type, unixtime ww_ut) {

   // If the watch/warning time exceeds the TrackPoint time, set it
   if(ValidTime >= ww_ut) set_watch_warn(ww_type);

   return;
}

////////////////////////////////////////////////////////////////////////

bool TrackPoint::set(const ATCFTrackLine &l) {
   int i;

   // Initialize TrackPoint with ATCFTrackLine, if necessary
   if(!IsSet) initialize(l);

   // Attempt to set each WindInfo object with ATCFTrackLine
   for(i=0; i<NWinds; i++) Wind[i].set_wind(l);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void TrackPoint::set_wind(int n, const QuadInfo &w) {

   // Check the range
   if((n < 0) || (n >= NWinds)) {
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

bool TrackPoint::is_match(const ATCFTrackLine &l) const {
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
