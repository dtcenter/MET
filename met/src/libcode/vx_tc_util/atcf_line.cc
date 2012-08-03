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

#include "atcf_line.h"

////////////////////////////////////////////////////////////////////////

extern unixtime     parse_time(const char *);
extern int          parse_lat (const char *);
extern int          parse_lon (const char *);
extern int          parse_int (const char *);
extern ConcatString parse_str (const char *);
  
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

ATCFLine::ATCFLine(const ATCFLine &t) {
   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ATCFLine & ATCFLine::operator=(const ATCFLine &t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

bool ATCFLine::operator==(const ATCFLine &t) {
   bool match = false;

   if(this == &t) return(true);

   if(Line == t.Line) match = true;
   else               match = false;

   return(match);
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::clear() {

   Line.clear();
   Basin.clear();
   CycloneNumber.clear();
   WarningTime     = (unixtime) 0;
   TechniqueNumber = bad_data_int;
   ForecastPeriod  = bad_data_int;
   Technique.clear();
   LatTenths       = bad_data_int;
   LonTenths       = bad_data_int;
   Vmax            = bad_data_int;
   MSLP            = bad_data_int;
   Level           = NoCycloneLevel;
   WindIntensity   = bad_data_int;
   Quadrant        = NoQuadrantType;
   Radius1         = bad_data_int;
   Radius2         = bad_data_int;
   Radius3         = bad_data_int;
   Radius4         = bad_data_int;
   IsobarPressure  = bad_data_int;
   IsobarRadius    = bad_data_int;
   MaxWindRadius   = bad_data_int;
   Gusts           = bad_data_int;
   EyeDiameter     = bad_data_int;
   SubRegion       = NoSubregionCode;
   MaxSeas         = bad_data_int;
   Forecaster.clear();
   StormDirection  = bad_data_int;
   StormSpeed      = bad_data_int;
   StormName.clear();
   Depth           = NoSystemsDepth;
   WaveHeight      = bad_data_int;
   SeasCode        = NoQuadrantType;
   SeasRadius1     = bad_data_int;
   SeasRadius2     = bad_data_int;
   SeasRadius3     = bad_data_int;
   SeasRadius4     = bad_data_int;

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "Line            = " << Line << "\n";
   out << prefix << "Basin           = \"" << (Basin ? Basin.text() : "(nul)") << "\"\n";
   out << prefix << "CycloneNumber   = \"" << (CycloneNumber ? CycloneNumber.text() : "(nul)") << "\"\n";
   out << prefix << "WarningTime     = " << unix_to_yyyymmdd_hhmmss(WarningTime) << "\n";
   out << prefix << "TechniqueNumber = " << TechniqueNumber << "\n";
   out << prefix << "ForecastPeriod  = " << ForecastPeriod << "\n";
   out << prefix << "Valid           = " << unix_to_yyyymmdd_hhmmss(valid()) << "\n";
   out << prefix << "Technique       = \"" << (Technique ? Technique.text() : "(nul)") << "\"\n";
   out << prefix << "Lat             = " << lat() << "\n";
   out << prefix << "Lon             = " << lon() << "\n";
   out << prefix << "Vmax            = " << Vmax << "\n";
   out << prefix << "MSLP            = " << MSLP << "\n";
   out << prefix << "Level           = " << cyclonelevel_to_string(Level) << "\n";
   out << prefix << "WindIntensity   = " << WindIntensity << "\n";
   out << prefix << "Quadrant        = " << quadranttype_to_string(Quadrant) << "\n";
   out << prefix << "Radius1         = " << Radius1 << "\n";
   out << prefix << "Radius2         = " << Radius2 << "\n";
   out << prefix << "Radius3         = " << Radius3 << "\n";
   out << prefix << "Radius4         = " << Radius4 << "\n";
   out << prefix << "IsobarPressure  = " << IsobarPressure << "\n";
   out << prefix << "IsobarRadius    = " << IsobarRadius << "\n";
   out << prefix << "MaxWindRadius   = " << MaxWindRadius << "\n";
   out << prefix << "Gusts           = " << Gusts << "\n";
   out << prefix << "EyeDiameter     = " << EyeDiameter << "\n";
   out << prefix << "SubRegion       = " << subregioncode_to_string(SubRegion) << "\n";
   out << prefix << "MaxSeas         = " << MaxSeas << "\n";
   out << prefix << "Forecaster      = \"" << (Forecaster ? Forecaster.text() : "(nul)") << "\"\n";
   out << prefix << "StormDirection  = " << StormDirection << "\n";
   out << prefix << "StormSpeed      = " << StormSpeed << "\n";
   out << prefix << "StormName       = \"" << (StormName ? StormName.text() : "(nul)") << "\"\n";
   out << prefix << "Depth           = " << systemsdepth_to_string(Depth) << "\n";
   out << prefix << "WaveHeight      = " << WaveHeight << "\n";
   out << prefix << "SeasCode        = " << quadranttype_to_string(SeasCode) << "\n";
   out << prefix << "SeasRadius1     = " << SeasRadius1 << "\n";
   out << prefix << "SeasRadius2     = " << SeasRadius2 << "\n";
   out << prefix << "SeasRadius3     = " << SeasRadius3 << "\n";
   out << prefix << "SeasRadius4     = " << SeasRadius4 << "\n";
   out << flush;
   
   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLine::assign(const ATCFLine &t) {
  
   clear();

   Line            = t.Line;
   Basin           = t.Basin;
   CycloneNumber   = t.CycloneNumber;
   WarningTime     = t.WarningTime;
   TechniqueNumber = t.TechniqueNumber;
   Technique       = t.Technique;
   ForecastPeriod  = t.ForecastPeriod;
   LatTenths       = t.LatTenths;
   LonTenths       = t.LonTenths;
   Vmax            = t.Vmax;
   MSLP            = t.MSLP;
   Level           = t.Level;
   WindIntensity   = t.WindIntensity;
   Quadrant        = t.Quadrant;
   Radius1         = t.Radius1;
   Radius2         = t.Radius2;
   Radius3         = t.Radius3;
   Radius4         = t.Radius4;
   IsobarPressure  = t.IsobarPressure;
   IsobarRadius    = t.IsobarRadius;
   MaxWindRadius   = t.MaxWindRadius;
   Gusts           = t.Gusts;
   EyeDiameter     = t.EyeDiameter;
   SubRegion       = t.SubRegion;
   MaxSeas         = t.MaxSeas;
   Forecaster      = t.Forecaster;
   StormDirection  = t.StormDirection;
   StormSpeed      = t.StormSpeed;
   StormName       = t.StormName;
   Depth           = t.Depth;
   WaveHeight      = t.WaveHeight;
   SeasCode        = t.SeasCode;
   SeasRadius1     = t.SeasRadius1;
   SeasRadius2     = t.SeasRadius2;
   SeasRadius3     = t.SeasRadius3;
   SeasRadius4     = t.SeasRadius4;
   
   return;
}

////////////////////////////////////////////////////////////////////////

unixtime ATCFLine::valid() const {
   unixtime u = 0;
   
   // Compute the valid time if WarningTime and ForecastPeriod are valid
   if(WarningTime > 0 && !is_bad_data(ForecastPeriod)) {
      u = WarningTime + sec_per_hour * ForecastPeriod;
   }
   
   // Add minutes for the BEST track
   if(strcasecmp(Technique, BestTrackStr) == 0 &&
      !is_bad_data(TechniqueNumber)) {
      u += sec_per_minute * TechniqueNumber;
   }
   
   return(u);
}

////////////////////////////////////////////////////////////////////////

int ATCFLine::lead() const {
   int s = bad_data_int;

   // Lead time for the BEST track is 0
   if(strcasecmp(Technique, BestTrackStr) == 0) {
      s = 0;
   }
   else if(!is_bad_data(ForecastPeriod)) {
      s = sec_per_hour * ForecastPeriod;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
////////////////////////////////////////////////////////////////////////

bool operator>>(istream &in, ATCFLine &t) {
   ConcatString element;
   StringArray a;
   const char delim [] = ",";
   int j;

   // Initialize
   t.clear();

   // Read line into ConcatString
   if(!t.Line.read_line(in)) return(false);
   t.Line.chomp('\n');

   // Split the comma-delimited line
   a = t.Line.split(delim);

   // Check for a blank line
   if(a.n_elements() == 0) return(false);

   // Check for the minumum number of elements
   if(a.n_elements() < MinATCFElements) {
      mlog << Warning
           << "\nbool operator>>(istream & in, ATCFLine & t) -> "
           << "found fewer than the expected number of elements ("
           << a.n_elements() << "<" << MinATCFElements << ") in ATCF line:\n"
           << t.Line << "\n\n";
      return(false);
   }

   // Remove whitespace from each element of the StringArray
   for(j=0; j<a.n_elements(); j++) {
      element.erase();
      element << a[j];
      element.ws_strip();
      a.set(j, element);
   }

   // Loop through and parse each element
   for(j=0; j<a.n_elements(); j++) {

      switch(j) {
        case (0): t.Basin           = parse_str(a[j]);               break;
        case (1): t.CycloneNumber   = parse_str(a[j]);               break;
        case (2): t.WarningTime     = parse_time(a[j]);              break;
        case (3): t.TechniqueNumber = parse_int(a[j]);               break;

        // Replace instances of AVN with GFS
        case (4): t.Technique       = parse_str(a[j]);
                  t.Technique.replace("AVN", "GFS");                 break;

        case (5): t.ForecastPeriod  = parse_int(a[j]);               break;
        case (6): t.LatTenths       = parse_lat(a[j]);               break;
        case (7): t.LonTenths       = parse_lon(a[j]);               break;
        case (8): t.Vmax            = parse_int(a[j]);               break;
        case (9): t.MSLP            = parse_int(a[j]);               break;
        case(10): t.Level           = string_to_cyclonelevel(a[j]);  break;
        case(11): t.WindIntensity   = parse_int(a[j]);               break;
        case(12): t.Quadrant        = string_to_quadranttype(a[j]);  break;
        case(13): t.Radius1         = parse_int(a[j]);               break;
        case(14): t.Radius2         = parse_int(a[j]);               break;
        case(15): t.Radius3         = parse_int(a[j]);               break;
        case(16): t.Radius4         = parse_int(a[j]);               break;
        case(17): t.IsobarPressure  = parse_int(a[j]);               break;
        case(18): t.IsobarRadius    = parse_int(a[j]);               break;
        case(19): t.MaxWindRadius   = parse_int(a[j]);               break;
        case(20): t.Gusts           = parse_int(a[j]);               break;
        case(21): t.EyeDiameter     = parse_int(a[j]);               break;
        case(22): t.SubRegion       = string_to_subregioncode(a[j]); break;
        case(23): t.MaxSeas         = parse_int(a[j]);               break;
        case(24): t.Forecaster      = parse_str(a[j]);               break;
        case(25): t.StormDirection  = parse_int(a[j]);               break;
        case(26): t.StormSpeed      = parse_int(a[j]);               break;
        case(27): t.StormName       = parse_str(a[j]);               break;
        case(28): t.Depth           = string_to_systemsdepth(a[j]);  break;
        case(29): t.WaveHeight      = parse_int(a[j]);               break;
        case(30): t.SeasCode        = string_to_quadranttype(a[j]);  break;
        case(31): t.SeasRadius1     = parse_int(a[j]);               break;
        case(32): t.SeasRadius2     = parse_int(a[j]);               break;
        case(33): t.SeasRadius3     = parse_int(a[j]);               break;
        case(34): t.SeasRadius4     = parse_int(a[j]);               break;
        default:                                                     break;
      } // end switch
   } // end for

   // Quality control checks
   if(t.Vmax <= 0) t.Vmax = bad_data_int;
   if(t.MSLP <= 0) t.MSLP = bad_data_int;

   return (true);
}

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

ConcatString parse_str(const char *s) {
   ConcatString v;

   if(strlen(s) > 0) v = s;

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
