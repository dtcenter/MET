// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

#include "atcf_line.h"
#include "atcf_offsets.h"

////////////////////////////////////////////////////////////////////////

extern unixtime     parse_time           (const char *);
extern int          parse_lat            (const char *);
extern int          parse_lon            (const char *);
extern int          parse_int            (const char *);
extern int          parse_int_check_zero (const char *);
  
////////////////////////////////////////////////////////////////////////
//
//  Code for class ATCFLine
//
////////////////////////////////////////////////////////////////////////
  
ATCFLine::ATCFLine() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ATCFLine::~ATCFLine() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ATCFLine::ATCFLine(const ATCFLine &l) {
   init_from_scratch();

   assign(l);
}

////////////////////////////////////////////////////////////////////////

ATCFLine & ATCFLine::operator=(const ATCFLine &l) {

   if(this == &l) return(*this);

   assign(l);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

bool ATCFLine::operator==(const ATCFLine &l) {
   return(get_line() == l.get_line());
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::init_from_scratch() {

   DataLine::init_from_scratch();

   // ATCF lines are comma-delimited
   set_delimiter(",");
   
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::assign(const ATCFLine &l) {

   clear();

   DataLine::assign(l);

   Technique = l.Technique;
   IsBestTrack = l.IsBestTrack;
   IsOperTrack = l.IsOperTrack;

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString cs;

   out << prefix << "Line            = " << (*this) << "\n";
   cs = basin();
   out << prefix << "Basin           = \"" << (cs ? cs.text() : "(nul)") << "\"\n";
   cs = cyclone_number();
   out << prefix << "CycloneNumber   = \"" << (cs ? cs.text() : "(nul)") << "\"\n";
   out << prefix << "WarningTime     = " << unix_to_yyyymmdd_hhmmss(warning_time()) << "\n";
   out << prefix << "TechniqueNumber = " << technique_number() << "\n";
   out << prefix << "ForecastPeriod  = " << forecast_period() << "\n";
   out << prefix << "Valid           = " << unix_to_yyyymmdd_hhmmss(valid()) << "\n";
   cs = technique();
   out << prefix << "Technique       = \"" << (cs ? cs.text() : "(nul)") << "\"\n";
   out << prefix << "IsBestTrack     = \"" << (IsBestTrack ? "TRUE" : "FALSE") << "\"\n";
   out << prefix << "IsOperTrack     = \"" << (IsOperTrack ? "TRUE" : "FALSE") << "\"\n";
   out << prefix << "Lat             = " << lat() << "\n";
   out << prefix << "Lon             = " << lon() << "\n";
   out << prefix << "Vmax            = " << v_max() << "\n";
   out << prefix << "MSLP            = " << mslp() << "\n";
   out << prefix << "Level           = " << cyclonelevel_to_string(level()) << "\n";
   out << prefix << "WindIntensity   = " << wind_intensity() << "\n";
   out << prefix << "Quadrant        = " << quadranttype_to_string(quadrant()) << "\n";
   out << prefix << "Radius1         = " << radius1() << "\n";
   out << prefix << "Radius2         = " << radius2() << "\n";
   out << prefix << "Radius3         = " << radius3() << "\n";
   out << prefix << "Radius4         = " << radius4() << "\n";
   out << prefix << "IsobarPressure  = " << isobar_pressure() << "\n";
   out << prefix << "IsobarRadius    = " << isobar_radius() << "\n";
   out << prefix << "MaxWindRadius   = " << max_wind_radius() << "\n";
   out << prefix << "Gusts           = " << gusts() << "\n";
   out << prefix << "EyeDiameter     = " << eye_diameter() << "\n";
   out << prefix << "SubRegion       = " << subregioncode_to_string(subregion()) << "\n";
   out << prefix << "MaxSeas         = " << max_seas() << "\n";
   cs = initials();
   out << prefix << "Initials        = \"" << (cs ? cs.text() : "(nul)") << "\"\n";
   out << prefix << "StormDirection  = " << storm_direction() << "\n";
   out << prefix << "StormSpeed      = " << storm_speed() << "\n";
   cs = storm_name();
   out << prefix << "StormName       = \"" << (cs ? cs.text() : "(nul)") << "\"\n";
   out << prefix << "Depth           = " << systemsdepth_to_string(depth()) << "\n";
   out << prefix << "WaveHeight      = " << wave_height() << "\n";
   out << prefix << "SeasCode        = " << quadranttype_to_string(seas_code()) << "\n";
   out << prefix << "SeasRadius1     = " << seas_radius1() << "\n";
   out << prefix << "SeasRadius2     = " << seas_radius2() << "\n";
   out << prefix << "SeasRadius3     = " << seas_radius3() << "\n";
   out << prefix << "SeasRadius4     = " << seas_radius4() << "\n";
   out << flush;
   
   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::clear() {
   DataLine::clear();
   Technique.clear();
   IsBestTrack = false;
   IsOperTrack = false;

   return;
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::read_line(LineDataFile * ldf) {
   int status;

   status = DataLine::read_line(ldf);

   // Check for bad return status or blank line
   if(!status || n_items() == 0) {
      clear();
      return(0);
   }

   // Check for the minumum number of elements
   if(n_items() < MinATCFElements) {
      mlog << Warning
           << "\nint ATCFLine::read_line(LineDataFile * ldf) -> "
           << "found fewer than the expected number of elements ("
           << n_items() << "<" << MinATCFElements << ") in ATCF line:\n"
           << Line << "\n\n";
      return(false);
   }

   return(1);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::is_header() const {
   if(strcasecmp(basin(), "BASIN") == 0) return(1);
   else                                  return(0);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLine::get_item(int i) const {
   ConcatString cs;

   cs = DataLine::get_item(i);

   // Strip off any whitespace
   cs.ws_strip();
   
   return(cs);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLine::get_line() const {
   ConcatString cs;

   if(N_items == 0) return(cs);
   
   for(int i=0; i<N_items-1; i++) cs << DataLine::get_item(i) << Delimiter;
   cs << DataLine::get_item(N_items-1);
   
   return(cs);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLine::basin() const {
   return(get_item(BasinOffset));
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLine::cyclone_number() const {
   return(get_item(CycloneNumberOffset));   }

////////////////////////////////////////////////////////////////////////

unixtime ATCFLine::warning_time() const {
   return(parse_time(get_item(WarningTimeOffset)));
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::technique_number() const {
   return(parse_int(get_item(TechniqueNumberOffset)));
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLine::technique() const {
   ConcatString cs;

   // Use Technique, if already set
   if(Technique) cs = Technique;
   else          cs = get_item(TechniqueOffset);

   // Replace instances of AVN with GFS
   cs.replace("AVN", "GFS");

   return(cs);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::forecast_period() const {
   return(parse_int(get_item(ForecastPeriodOffset)));
}

////////////////////////////////////////////////////////////////////////

double ATCFLine::lat() const {
   double v = parse_lat(get_item(LatTenthsOffset));

   return(0.1*v);
}

////////////////////////////////////////////////////////////////////////

double ATCFLine::lon() const {
   double v = parse_lon(get_item(LonTenthsOffset));

   return(0.1*v);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::v_max() const {
   return(VMaxOffset < N_items ?
          parse_int_check_zero(get_item(VMaxOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::mslp() const {
   return(MSLPOffset < N_items ?
          parse_int_check_zero(get_item(MSLPOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

CycloneLevel ATCFLine::level() const {
   return(LevelOffset < N_items ?
          string_to_cyclonelevel(get_item(LevelOffset)) :
          NoCycloneLevel);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::wind_intensity() const {
   return(WindIntensityOffset < N_items ?
          parse_int_check_zero(get_item(WindIntensityOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

QuadrantType ATCFLine::quadrant() const {
   return(QuadrantOffset < N_items ?
          string_to_quadranttype(get_item(QuadrantOffset)) :
          NoQuadrantType);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::radius1() const {
   return(Radius1Offset < N_items ?
          parse_int_check_zero(get_item(Radius1Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::radius2() const {
   return(Radius2Offset < N_items ?
          parse_int_check_zero(get_item(Radius2Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::radius3() const {
   return(Radius3Offset < N_items ?
          parse_int_check_zero(get_item(Radius3Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::radius4() const {
   return(Radius4Offset < N_items ?
          parse_int_check_zero(get_item(Radius4Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::isobar_pressure() const {
   return(IsobarPressureOffset < N_items ?
          parse_int_check_zero(get_item(IsobarPressureOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::isobar_radius() const {
   return(IsobarRadiusOffset < N_items ?
          parse_int_check_zero(get_item(IsobarRadiusOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::max_wind_radius() const {
   return(MaxWindRadiusOffset < N_items ?
          parse_int_check_zero(get_item(MaxWindRadiusOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::gusts() const {
   return(GustsOffset < N_items ?
          parse_int_check_zero(get_item(GustsOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::eye_diameter() const {
   return(EyeDiameterOffset < N_items ?
          parse_int_check_zero(get_item(EyeDiameterOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

SubregionCode ATCFLine::subregion() const {
   return(SubRegionOffset < N_items ?
          string_to_subregioncode(get_item(SubRegionOffset)) :
          NoSubregionCode);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::max_seas() const {
   return(MaxSeasOffset < N_items ?
          parse_int_check_zero(get_item(MaxSeasOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLine::initials() const {
   return(InitialsOffset < N_items ?
          get_item(InitialsOffset) :
          "");
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::storm_direction() const {
   return(StormDirectionOffset < N_items ?
          parse_int(get_item(StormDirectionOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::storm_speed() const {
   return(StormSpeedOffset < N_items ?
          parse_int(get_item(StormSpeedOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLine::storm_name() const {
   return(StormNameOffset < N_items ?
          get_item(StormNameOffset) :
          "");
}

////////////////////////////////////////////////////////////////////////

SystemsDepth ATCFLine::depth() const {
   return(DepthOffset < N_items ?
          string_to_systemsdepth(get_item(DepthOffset)) :
          NoSystemsDepth);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::wave_height() const {
   return(WaveHeightOffset < N_items ?
          parse_int_check_zero(get_item(WaveHeightOffset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

QuadrantType ATCFLine::seas_code() const {
   return(SeasCodeOffset < N_items ?
          string_to_quadranttype(get_item(SeasCodeOffset)) :
          NoQuadrantType);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::seas_radius1() const {
   return(SeasRadius1Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius1Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::seas_radius2() const {
   return(SeasRadius2Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius2Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::seas_radius3() const {
   return(SeasRadius3Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius3Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::seas_radius4() const {
   return(SeasRadius4Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius4Offset)) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

unixtime ATCFLine::valid() const {
   unixtime wt = warning_time();
   double   fp = forecast_period();
   int      tn = technique_number();
   unixtime ut = 0;
 
   // Compute the valid time if WarningTime and ForecastPeriod are valid
   if(wt > 0 && !is_bad_data(fp)) {
      ut = wt + sec_per_hour * fp;
   }

   // Add minutes for the BEST track
   if(is_best_track() && !is_bad_data(tn)) {
      ut += sec_per_minute * tn;
   }

   return(ut);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::lead() const {
   double fp = forecast_period();
   int    s  = bad_data_int;

   // Lead time for the BEST track is 0
   if(is_best_track()) {
      s = 0;
   }
   else if(!is_bad_data(fp)) {
      s = sec_per_hour * fp;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
////////////////////////////////////////////////////////////////////////

CycloneLevel wind_speed_to_cyclone_level(int s) {
   CycloneLevel l;

   // Apply logic to convert wind speed to CycloneLevel
        if(s <= 33) l = TropicalDepression;
   else if(s <= 63) l = TropicalStorm;
   else             l = Hurricane;

   return(l);
}

////////////////////////////////////////////////////////////////////////

unixtime parse_time(const char *s) {
   int year, month, day, hour;

   if(sscanf(s, "%4d%2d%2d%2d", &year, &month, &day, &hour) != 4) {
      mlog << Error
           << "\nunixtime parse_time(const char *) -> "
           << "bad time format ... \"" << s << "\"\n\n";
      exit(1);
   }

   return(mdyhms_to_unix(month, day, year, hour, 0, 0));
}

////////////////////////////////////////////////////////////////////////

int parse_lat(const char *s) {
   int v;
   
   v = parse_int(s);

   switch(s[strlen(s) - 1]) {
      case 'N':           break;
      case 'S': v *= -1;  break; // Degrees south is negative
      default:
         mlog << Error
              << "\nint parse_lat(const char *) -> "
              << "bad latitude ... \"" << s << "\"\n\n";
         exit(1);
         break;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

int parse_lon(const char *s) {
   int v;

   v = parse_int(s);

   switch(s[strlen(s) - 1]) {
      case 'E':           break;
      case 'W': v *= -1;  break; // Degrees west is negative
      default:
         mlog << Error
              << "\nint parse_lat(const char *) -> "
              << "bad latitude ... \"" << s << "\"\n\n";
         exit(1);
         break;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

int parse_int(const char *s) {
   int v;
   
   if(strlen(s) > 0) v = atoi(s);
   else              v = bad_data_int;

   return(v);
}

////////////////////////////////////////////////////////////////////////

int parse_int_check_zero(const char *s) {

   int v = parse_int(s);

   // Interpret values of 0 as bad data
   if(v == 0) v = bad_data_int;

   return(v);
}

////////////////////////////////////////////////////////////////////////

WatchWarnType ww_max(const WatchWarnType t1, const WatchWarnType t2) {
   WatchWarnType t;

   // Order of watch/warning interest:
   //   HurricaneWarn,
   //   TropicalStormWarn,
   //   HurricaneWatch,
   //   TropicalStormWatch,
   //   GaleWarn,
   //   StormWarn

        if(t1 == HurricaneWarn      ||
           t2 == HurricaneWarn       ) t = HurricaneWarn;
   else if(t1 == TropicalStormWarn  ||
           t2 == TropicalStormWarn   ) t = TropicalStormWarn;
   else if(t1 == HurricaneWatch     ||
           t2 == HurricaneWatch      ) t = HurricaneWatch;
   else if(t1 == TropicalStormWatch ||
           t2 == TropicalStormWatch  ) t = TropicalStormWatch;
   else if(t1 == GaleWarn           ||
           t2 == GaleWarn            ) t = GaleWarn;
   else if(t1 == StormWarn          ||
           t2 == StormWarn           ) t = StormWarn;
   else                                t = NoWatchWarnType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

WatchWarnType int_to_watchwarntype(int i) {
   WatchWarnType t;

        if(i == 1) t = TropicalStormWatch;
   else if(i == 2) t = TropicalStormWarn;
   else if(i == 3) t = GaleWarn;
   else if(i == 4) t = StormWarn;
   else if(i == 5) t = HurricaneWatch;
   else if(i == 6) t = HurricaneWarn;
   else            t = NoWatchWarnType;

   return(t);
}


////////////////////////////////////////////////////////////////////////

WatchWarnType string_to_watchwarntype(const char *s) {
   WatchWarnType t;

        if(strcasecmp(s, "TSWATCH") == 0) t = TropicalStormWatch;
   else if(strcasecmp(s, "TSWARN")  == 0) t = TropicalStormWarn;
   else if(strcasecmp(s, "GLWARN")  == 0) t = GaleWarn;
   else if(strcasecmp(s, "STWARN")  == 0) t = StormWarn;
   else if(strcasecmp(s, "HUWATCH") == 0) t = HurricaneWatch;
   else if(strcasecmp(s, "HUWARN")  == 0) t = HurricaneWarn;
   else                                   t = NoWatchWarnType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString watchwarntype_to_string(const WatchWarnType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case TropicalStormWatch: s = "TSWATCH"; break;
      case TropicalStormWarn:  s = "TSWARN";  break;
      case GaleWarn:           s = "GLWARN";  break;
      case StormWarn:          s = "STWARN";  break;
      case HurricaneWatch:     s = "HUWATCH"; break;
      case HurricaneWarn:      s = "HUWARN";  break;
      case NoWatchWarnType:    s = na_str;    break;
      default:                 s = na_str;    break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////

CycloneLevel string_to_cyclonelevel(const char *s) {
   CycloneLevel l;

        if(strcmp(s, "DB") == 0) l = Disturbance;
   else if(strcmp(s, "TD") == 0) l = TropicalDepression;
   else if(strcmp(s, "TS") == 0) l = TropicalStorm;
   else if(strcmp(s, "TY") == 0) l = Typhoon;
   else if(strcmp(s, "ST") == 0) l = SuperTyphoon;
   else if(strcmp(s, "TC") == 0) l = TropicalCyclone;
   else if(strcmp(s, "HU") == 0) l = Hurricane;
   else if(strcmp(s, "SD") == 0) l = SubtropicalDepression;
   else if(strcmp(s, "SS") == 0) l = SubtropicalStorm;
   else if(strcmp(s, "EX") == 0) l = ExtratropicalSystem;
   else if(strcmp(s, "IN") == 0) l = Inland;
   else if(strcmp(s, "DS") == 0) l = Dissipating;
   else if(strcmp(s, "LO") == 0) l = Low;
   else if(strcmp(s, "WV") == 0) l = TropicalWave;
   else if(strcmp(s, "ET") == 0) l = Extrapolated;
   else /*           "XX"     */ l = NoCycloneLevel;

   return(l);
}

////////////////////////////////////////////////////////////////////////

ConcatString cyclonelevel_to_string(const CycloneLevel t) {
   const char *s = (const char *) 0;

   switch(t) {
      case Disturbance:           s = "DB";   break;
      case TropicalDepression:    s = "TD";   break;
      case TropicalStorm:         s = "TS";   break;
      case Typhoon:               s = "TY";   break;
      case SuperTyphoon:          s = "ST";   break;
      case TropicalCyclone:       s = "TC";   break;
      case Hurricane:             s = "HU";   break;
      case SubtropicalDepression: s = "SD";   break;
      case SubtropicalStorm:      s = "SS";   break;
      case ExtratropicalSystem:   s = "EX";   break;
      case Inland:                s = "IN";   break;
      case Dissipating:           s = "DS";   break;
      case Low:                   s = "LO";   break;
      case TropicalWave:          s = "WV";   break;
      case Extrapolated:          s = "ET";   break;
      case NoCycloneLevel:        s = na_str; break;
      default:                    s = na_str; break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////

QuadrantType string_to_quadranttype(const char *s) {
   QuadrantType t;

        if(strcmp(s, "AAA") == 0) t =  FullCircle;
   else if(strcmp(s, "NNQ") == 0) t =  N_Quadrant;
   else if(strcmp(s, "EEQ") == 0) t =  E_Quadrant;
   else if(strcmp(s, "SSQ") == 0) t =  S_Quadrant;
   else if(strcmp(s, "WWQ") == 0) t =  W_Quadrant;
   else if(strcmp(s, "NEQ") == 0) t = NE_Quadrant;
   else if(strcmp(s, "SEQ") == 0) t = SE_Quadrant;
   else if(strcmp(s, "SWQ") == 0) t = SW_Quadrant;
   else if(strcmp(s, "NWQ") == 0) t = NW_Quadrant;
   else                           t = NoQuadrantType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString quadranttype_to_string(const QuadrantType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case FullCircle:     s = "AAA";  break;
      case N_Quadrant:     s = "NNQ";  break;
      case E_Quadrant:     s = "EEQ";  break;
      case S_Quadrant:     s = "SSQ";  break;
      case W_Quadrant:     s = "WWQ";  break;
      case NE_Quadrant:    s = "NEQ";  break;
      case SE_Quadrant:    s = "SEQ";  break;
      case SW_Quadrant:    s = "SWQ";  break;
      case NW_Quadrant:    s = "NWQ";  break;
      case NoQuadrantType: s = na_str; break;
      default:             s = na_str; break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////

SubregionCode string_to_subregioncode(const char *s) {
   SubregionCode c;

        if(strcmp(s, "A") == 0) c = Arabian_Sea;
   else if(strcmp(s, "B") == 0) c = Bay_of_Bengal;
   else if(strcmp(s, "C") == 0) c = Central_Pacific;
   else if(strcmp(s, "E") == 0) c = Eastern_Pacific;
   else if(strcmp(s, "L") == 0) c = Atlantic;
   else if(strcmp(s, "P") == 0) c = South_Pacific;
   else if(strcmp(s, "Q") == 0) c = South_Atlantic;
   else if(strcmp(s, "S") == 0) c = South_IO;
   else if(strcmp(s, "W") == 0) c = Western_Pacific;
   else                            c = NoSubregionCode;

   return(c);
}

////////////////////////////////////////////////////////////////////////

ConcatString subregioncode_to_string(const SubregionCode t) {
   const char *s = (const char *) 0;

   switch(t) {
      case Arabian_Sea:     s = "A";    break;
      case Bay_of_Bengal:   s = "B";    break;
      case Central_Pacific: s = "C";    break;
      case Eastern_Pacific: s = "E";    break;
      case Atlantic:        s = "L";    break;
      case South_Pacific:   s = "P";    break;
      case South_Atlantic:  s = "Q";    break;
      case South_IO:        s = "S";    break;
      case Western_Pacific: s = "W";    break;
      case NoSubregionCode: s = na_str; break;
      default:              s = na_str; break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////

SystemsDepth string_to_systemsdepth(const char *s) {
  SystemsDepth d;

        if(strcmp(s, "D") == 0) d = DeepDepth;
   else if(strcmp(s, "M") == 0) d = MediumDepth;
   else if(strcmp(s, "S") == 0) d = ShallowDepth;
   else                         d = NoSystemsDepth;

   return(d);
}

////////////////////////////////////////////////////////////////////////

ConcatString systemsdepth_to_string(const SystemsDepth t) {
   const char *s = (const char *) 0;

   switch(t) {
      case DeepDepth:      s = "D";    break;
      case MediumDepth:    s = "M";    break;
      case ShallowDepth:   s = "S";    break;
      case NoSystemsDepth: s = na_str; break;
      default:             s = na_str; break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////
