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

#include "atcf_track_line.h"
#include "atcf_offsets.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ATCFTrackLine
//
////////////////////////////////////////////////////////////////////////

ATCFTrackLine::ATCFTrackLine() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ATCFTrackLine::~ATCFTrackLine() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ATCFTrackLine::ATCFTrackLine(const ATCFTrackLine &l) {
   init_from_scratch();

   assign(l);
}

////////////////////////////////////////////////////////////////////////

ATCFTrackLine & ATCFTrackLine::operator=(const ATCFTrackLine &l) {

   if(this == &l) return(*this);

   assign(l);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ATCFTrackLine::init_from_scratch() {

   ATCFLineBase::init_from_scratch();

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFTrackLine::assign(const ATCFTrackLine &l) {

   clear();

   ATCFLineBase::assign(l);

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFTrackLine::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString cs;

   ATCFLineBase::dump(out, indent_depth);

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
   out << prefix << "Initials        = \"" << cs.contents() << "\"\n";
   out << prefix << "StormDirection  = " << storm_direction() << "\n";
   out << prefix << "StormSpeed      = " << storm_speed() << "\n";
      cs = storm_name();
   out << prefix << "StormName       = \"" << cs.contents() << "\"\n";
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

void ATCFTrackLine::clear() {
   ATCFLineBase::clear();

   return;
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::read_line(LineDataFile * ldf) {
   int status;

   clear();

   status = ATCFLineBase::read_line(ldf);

   // Check for bad return status or blank line
   if(!status) return(0);

   // Check the line type
   if(Type != ATCFLineType_Track) {
      mlog << Warning
           << "\nint ATCFTrackLine::read_line(LineDataFile * ldf) -> "
           << "unexpected ATCF line type ("
           << atcflinetype_to_string(Type) << ")\n\n";
      return(0);
   }

   // Check for the minumum number of track line elements
   if(n_items() < MinATCFTrackElements) {
      mlog << Warning
           << "\nint ATCFTrackLine::read_line(LineDataFile * ldf) -> "
           << "found fewer than the expected number of elements ("
           << n_items() << "<" << MinATCFTrackElements
           << ") in ATCF track line:\n" << DataLine::get_line() << "\n\n";
      return(0);
   }

   return(1);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::v_max() const {
   return(VMaxOffset < N_items ?
          parse_int_check_zero(get_item(VMaxOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::mslp() const {
   return(MSLPOffset < N_items ?
          parse_int_check_zero(get_item(MSLPOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

CycloneLevel ATCFTrackLine::level() const {
   return(LevelOffset < N_items ?
          string_to_cyclonelevel(get_item(LevelOffset).c_str()) :
          NoCycloneLevel);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::wind_intensity() const {
   return(WindIntensityOffset < N_items ?
          parse_int_check_zero(get_item(WindIntensityOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

QuadrantType ATCFTrackLine::quadrant() const {
   return(QuadrantOffset < N_items ?
          string_to_quadranttype(get_item(QuadrantOffset).c_str()) :
          NoQuadrantType);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::radius1() const {
   return(Radius1Offset < N_items ?
          parse_int_check_zero(get_item(Radius1Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::radius2() const {
   return(Radius2Offset < N_items ?
          parse_int_check_zero(get_item(Radius2Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::radius3() const {
   return(Radius3Offset < N_items ?
          parse_int_check_zero(get_item(Radius3Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::radius4() const {
   return(Radius4Offset < N_items ?
          parse_int_check_zero(get_item(Radius4Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::isobar_pressure() const {
   return(IsobarPressureOffset < N_items ?
          parse_int_check_zero(get_item(IsobarPressureOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::isobar_radius() const {
   return(IsobarRadiusOffset < N_items ?
          parse_int_check_zero(get_item(IsobarRadiusOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::max_wind_radius() const {
   return(MaxWindRadiusOffset < N_items ?
          parse_int_check_zero(get_item(MaxWindRadiusOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::gusts() const {
   return(GustsOffset < N_items ?
          parse_int_check_zero(get_item(GustsOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::eye_diameter() const {
   return(EyeDiameterOffset < N_items ?
          parse_int_check_zero(get_item(EyeDiameterOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

SubregionCode ATCFTrackLine::subregion() const {
   return(SubRegionOffset < N_items ?
          string_to_subregioncode(get_item(SubRegionOffset).c_str()) :
          NoSubregionCode);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::max_seas() const {
   return(MaxSeasOffset < N_items ?
          parse_int_check_zero(get_item(MaxSeasOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFTrackLine::initials() const {
   return(InitialsOffset < N_items ?
          (string)get_item(InitialsOffset) :
          (string)"");
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::storm_direction() const {
   return(StormDirectionOffset < N_items ?
          parse_int(get_item(StormDirectionOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::storm_speed() const {
   return(StormSpeedOffset < N_items ?
          parse_int(get_item(StormSpeedOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFTrackLine::storm_name() const {
   return(StormNameOffset < N_items ?
          (string)get_item(StormNameOffset) :
          (string)"");
}

////////////////////////////////////////////////////////////////////////

SystemsDepth ATCFTrackLine::depth() const {
   return(DepthOffset < N_items ?
          string_to_systemsdepth(get_item(DepthOffset).c_str()) :
          NoSystemsDepth);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::wave_height() const {
   return(WaveHeightOffset < N_items ?
          parse_int_check_zero(get_item(WaveHeightOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

QuadrantType ATCFTrackLine::seas_code() const {
   return(SeasCodeOffset < N_items ?
          string_to_quadranttype(get_item(SeasCodeOffset).c_str()) :
          NoQuadrantType);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius1() const {
   return(SeasRadius1Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius1Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius2() const {
   return(SeasRadius2Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius2Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius3() const {
   return(SeasRadius3Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius3Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius4() const {
   return(SeasRadius4Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius4Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
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

CycloneLevel wind_speed_to_cyclonelevel(int s) {
   CycloneLevel l;

   // Apply logic to convert wind speed to CycloneLevel
        if(s <= 33) l = TropicalDepression;
   else if(s <= 63) l = TropicalStorm;
   else             l = Hurricane;

   return(l);
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
