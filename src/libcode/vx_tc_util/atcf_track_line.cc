// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_math.h"

#include "atcf_track_line.h"
#include "atcf_offsets.h"

using namespace std;

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

   if(this == &l) return *this;

   assign(l);

   return *this;
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
   if(!status) return 0;

   // Check the line type
   if(Type != ATCFLineType::Track &&
      Type != ATCFLineType::GenTrack) {
      mlog << Warning
           << "\nint ATCFTrackLine::read_line(LineDataFile * ldf) -> "
           << "unexpected ATCF line type ("
           << atcflinetype_to_string(Type) << ")\n\n";
      return 0;
   }

   // Check for the minumum number of track line elements
   if((Type == ATCFLineType::Track &&
       n_items() < MinATCFTrackElements) ||
      (Type == ATCFLineType::GenTrack &&
       n_items() < MinATCFGenTrackElements)) {
      mlog << Warning
           << "\nint ATCFTrackLine::read_line(LineDataFile * ldf) -> "
           << "found fewer than the expected number of elements ("
           << n_items() << ") in ATCF track line:\n" << DataLine::get_line()
           << "\n\n";
      return 0;
   }

   return 1;
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
          CycloneLevel::None);
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
          QuadrantType::None);
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

int ATCFTrackLine::storm_direction() const {
   if(Type == ATCFLineType::Track &&
      StormDirectionOffset < N_items) {
      return parse_int(get_item(StormDirectionOffset).c_str());
   }
   else if(Type == ATCFLineType::GenTrack &&
           StormDirectionOffset < N_items) {
      return parse_int(get_item(GenStormDirectionOffset).c_str());
   }
   return bad_data_int;
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::storm_speed() const {
   if(Type == ATCFLineType::Track &&
      StormSpeedOffset < N_items) {
      return parse_int(get_item(StormSpeedOffset).c_str());
   }
   else if(Type == ATCFLineType::GenTrack &&
           StormSpeedOffset < N_items) {
      return parse_int(get_item(GenStormSpeedOffset).c_str());
   }
   return bad_data_int;
}

////////////////////////////////////////////////////////////////////////
//
// For ATCFLineType::Track, only valid when UserDefined = THERMO PARAMS
// For ATCFLineType::GenTrack, get the warm core column
//
////////////////////////////////////////////////////////////////////////

bool ATCFTrackLine::warm_core() const {
   if(Type == ATCFLineType::Track &&
      WarmCoreOffset < N_items) {
      return(get_item(UserDefinedOffset).comparecase(ThermoParams_Str) == 0 &&
             get_item(WarmCoreOffset).comparecase("Y") == 0);
   }
   else if(Type == ATCFLineType::GenTrack &&
           GenWarmCoreOffset < N_items) {
      return(get_item(GenWarmCoreOffset).comparecase("Y") == 0);
   }
   return false;
}

////////////////////////////////////////////////////////////////////////
//
// Specific to ATCFLineType::Track
//
////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::gusts() const {
   return(Type == ATCFLineType::Track && GustsOffset < N_items ?
          parse_int_check_zero(get_item(GustsOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::eye_diameter() const {
   return(Type == ATCFLineType::Track && EyeDiameterOffset < N_items ?
          parse_int_check_zero(get_item(EyeDiameterOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

SubregionCode ATCFTrackLine::subregion() const {
   return(Type == ATCFLineType::Track && SubRegionOffset < N_items ?
          string_to_subregioncode(get_item(SubRegionOffset).c_str()) :
          SubregionCode::None);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::max_seas() const {
   return(Type == ATCFLineType::Track && MaxSeasOffset < N_items ?
          parse_int_check_zero(get_item(MaxSeasOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFTrackLine::initials() const {
   return(Type == ATCFLineType::Track && InitialsOffset < N_items ?
          (string) get_item(InitialsOffset) :
          (string) "");
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFTrackLine::storm_name() const {
   return(Type == ATCFLineType::Track && StormNameOffset < N_items ?
          (string) get_item(StormNameOffset) :
          (string) "");
}

////////////////////////////////////////////////////////////////////////

SystemsDepth ATCFTrackLine::depth() const {
   return(Type == ATCFLineType::Track && DepthOffset < N_items ?
          string_to_systemsdepth(get_item(DepthOffset).c_str()) :
          SystemsDepth::None);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::wave_height() const {
   return(Type == ATCFLineType::Track && WaveHeightOffset < N_items ?
          parse_int_check_zero(get_item(WaveHeightOffset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

QuadrantType ATCFTrackLine::seas_code() const {
   return(Type == ATCFLineType::Track && SeasCodeOffset < N_items ?
          string_to_quadranttype(get_item(SeasCodeOffset).c_str()) :
          QuadrantType::None);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius1() const {
   return(Type == ATCFLineType::Track && SeasRadius1Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius1Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius2() const {
   return(Type == ATCFLineType::Track && SeasRadius2Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius2Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius3() const {
   return(Type == ATCFLineType::Track && SeasRadius3Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius3Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::seas_radius4() const {
   return(Type == ATCFLineType::Track && SeasRadius4Offset < N_items ?
          parse_int_check_zero(get_item(SeasRadius4Offset).c_str()) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////
//
// Specific to ATCFLineType::Track
//
////////////////////////////////////////////////////////////////////////

double ATCFTrackLine::parameter_b() const {
   int v = (Type == ATCFLineType::GenTrack && GenParameterBOffset < N_items ?
            parse_int(get_item(GenParameterBOffset).c_str(), -999) :
            bad_data_int);
   return(!is_bad_data(v) ? v/10.0 : bad_data_double);
}

////////////////////////////////////////////////////////////////////////

double ATCFTrackLine::therm_wind_lower() const {
   int v = (Type == ATCFLineType::GenTrack && GenThermWindLowerOffset < N_items ?
            parse_int(get_item(GenThermWindLowerOffset).c_str(), -9999) :
            bad_data_int);
   return(!is_bad_data(v) ? v/10.0 : bad_data_double);
}

////////////////////////////////////////////////////////////////////////

double ATCFTrackLine::therm_wind_upper() const {
   int v = (Type == ATCFLineType::GenTrack && GenThermWindUpperOffset < N_items ?
            parse_int(get_item(GenThermWindUpperOffset).c_str(), -9999) :
            bad_data_int);
   return(!is_bad_data(v) ? v/10.0 : bad_data_double);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::mean_850_vort() const {
   return(Type == ATCFLineType::GenTrack && GenMean850VortOffset < N_items ?
          parse_int(get_item(GenThermWindUpperOffset).c_str(), -9999) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::max_850_vort() const {
   return(Type == ATCFLineType::GenTrack && GenMax850VortOffset < N_items ?
          parse_int(get_item(GenThermWindUpperOffset).c_str(), -9999) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::mean_700_vort() const {
   return(Type == ATCFLineType::GenTrack && GenMean700VortOffset < N_items ?
          parse_int(get_item(GenThermWindUpperOffset).c_str(), -9999) :
          bad_data_int);
}

////////////////////////////////////////////////////////////////////////

int ATCFTrackLine::max_700_vort() const {
   return(Type == ATCFLineType::GenTrack && GenMax700VortOffset < N_items ?
          parse_int(get_item(GenThermWindUpperOffset).c_str(), -9999) :
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

        if(t1 == WatchWarnType::HurricaneWarn      ||
           t2 == WatchWarnType::HurricaneWarn       ) t = WatchWarnType::HurricaneWarn;
   else if(t1 == WatchWarnType::TropicalStormWarn  ||
           t2 == WatchWarnType::TropicalStormWarn   ) t = WatchWarnType::TropicalStormWarn;
   else if(t1 == WatchWarnType::HurricaneWatch     ||
           t2 == WatchWarnType::HurricaneWatch      ) t = WatchWarnType::HurricaneWatch;
   else if(t1 == WatchWarnType::TropicalStormWatch ||
           t2 == WatchWarnType::TropicalStormWatch  ) t = WatchWarnType::TropicalStormWatch;
   else if(t1 == WatchWarnType::GaleWarn           ||
           t2 == WatchWarnType::GaleWarn            ) t = WatchWarnType::GaleWarn;
   else if(t1 == WatchWarnType::StormWarn          ||
           t2 == WatchWarnType::StormWarn           ) t = WatchWarnType::StormWarn;
   else                                               t = WatchWarnType::None;

   return t;
}

////////////////////////////////////////////////////////////////////////

WatchWarnType int_to_watchwarntype(int i) {
   WatchWarnType t;

        if(i == 1) t = WatchWarnType::TropicalStormWatch;
   else if(i == 2) t = WatchWarnType::TropicalStormWarn;
   else if(i == 3) t = WatchWarnType::GaleWarn;
   else if(i == 4) t = WatchWarnType::StormWarn;
   else if(i == 5) t = WatchWarnType::HurricaneWatch;
   else if(i == 6) t = WatchWarnType::HurricaneWarn;
   else            t = WatchWarnType::None;

   return t;
}


////////////////////////////////////////////////////////////////////////

WatchWarnType string_to_watchwarntype(const char *s) {
   WatchWarnType t;

        if(strcasecmp(s, "TSWATCH") == 0) t = WatchWarnType::TropicalStormWatch;
   else if(strcasecmp(s, "TSWARN")  == 0) t = WatchWarnType::TropicalStormWarn;
   else if(strcasecmp(s, "GLWARN")  == 0) t = WatchWarnType::GaleWarn;
   else if(strcasecmp(s, "STWARN")  == 0) t = WatchWarnType::StormWarn;
   else if(strcasecmp(s, "HUWATCH") == 0) t = WatchWarnType::HurricaneWatch;
   else if(strcasecmp(s, "HUWARN")  == 0) t = WatchWarnType::HurricaneWarn;
   else                                   t = WatchWarnType::None;

   return t;
}

////////////////////////////////////////////////////////////////////////

ConcatString watchwarntype_to_string(const WatchWarnType t) {
   const char *s = (const char *) nullptr;

   switch(t) {
      case WatchWarnType::TropicalStormWatch: s = "TSWATCH"; break;
      case WatchWarnType::TropicalStormWarn:  s = "TSWARN";  break;
      case WatchWarnType::GaleWarn:           s = "GLWARN";  break;
      case WatchWarnType::StormWarn:          s = "STWARN";  break;
      case WatchWarnType::HurricaneWatch:     s = "HUWATCH"; break;
      case WatchWarnType::HurricaneWarn:      s = "HUWARN";  break;
      case WatchWarnType::None:               s = na_str;    break;
      default:                                s = na_str;    break;
   }

   return ConcatString(s);
}

////////////////////////////////////////////////////////////////////////

CycloneLevel string_to_cyclonelevel(const char *s) {
   CycloneLevel l;

        if(strcmp(s, "DB") == 0) l = CycloneLevel::Disturbance;
   else if(strcmp(s, "TD") == 0) l = CycloneLevel::TropicalDepression;
   else if(strcmp(s, "TS") == 0) l = CycloneLevel::TropicalStorm;
   else if(strcmp(s, "TY") == 0) l = CycloneLevel::Typhoon;
   else if(strcmp(s, "ST") == 0) l = CycloneLevel::SuperTyphoon;
   else if(strcmp(s, "TC") == 0) l = CycloneLevel::TropicalCyclone;
   else if(strcmp(s, "HU") == 0) l = CycloneLevel::Hurricane;
   else if(strcmp(s, "SD") == 0) l = CycloneLevel::SubtropicalDepression;
   else if(strcmp(s, "SS") == 0) l = CycloneLevel::SubtropicalStorm;
   else if(strcmp(s, "EX") == 0) l = CycloneLevel::ExtratropicalSystem;
   else if(strcmp(s, "IN") == 0) l = CycloneLevel::Inland;
   else if(strcmp(s, "DS") == 0) l = CycloneLevel::Dissipating;
   else if(strcmp(s, "LO") == 0) l = CycloneLevel::Low;
   else if(strcmp(s, "WV") == 0) l = CycloneLevel::TropicalWave;
   else if(strcmp(s, "ET") == 0) l = CycloneLevel::Extrapolated;
   else /*           "XX"     */ l = CycloneLevel::None;

   return l;
}

////////////////////////////////////////////////////////////////////////

ConcatString cyclonelevel_to_string(const CycloneLevel t) {
   const char *s = (const char *) nullptr;

   switch(t) {
      case CycloneLevel::Disturbance:           s = "DB";   break;
      case CycloneLevel::TropicalDepression:    s = "TD";   break;
      case CycloneLevel::TropicalStorm:         s = "TS";   break;
      case CycloneLevel::Typhoon:               s = "TY";   break;
      case CycloneLevel::SuperTyphoon:          s = "ST";   break;
      case CycloneLevel::TropicalCyclone:       s = "TC";   break;
      case CycloneLevel::Hurricane:             s = "HU";   break;
      case CycloneLevel::SubtropicalDepression: s = "SD";   break;
      case CycloneLevel::SubtropicalStorm:      s = "SS";   break;
      case CycloneLevel::ExtratropicalSystem:   s = "EX";   break;
      case CycloneLevel::Inland:                s = "IN";   break;
      case CycloneLevel::Dissipating:           s = "DS";   break;
      case CycloneLevel::Low:                   s = "LO";   break;
      case CycloneLevel::TropicalWave:          s = "WV";   break;
      case CycloneLevel::Extrapolated:          s = "ET";   break;
      case CycloneLevel::None:                  s = na_str; break;
      default:                                  s = na_str; break;
   }

   return ConcatString(s);
}

////////////////////////////////////////////////////////////////////////

CycloneLevel wind_speed_to_cyclonelevel(int s) {
   CycloneLevel l;

   // Apply logic to convert wind speed to CycloneLevel
        if(s <= 33) l = CycloneLevel::TropicalDepression;
   else if(s <= 63) l = CycloneLevel::TropicalStorm;
   else             l = CycloneLevel::Hurricane;

   return l;
}

////////////////////////////////////////////////////////////////////////

QuadrantType string_to_quadranttype(const char *s) {
   QuadrantType t;

        if(strcmp(s, "AAA") == 0) t = QuadrantType::FullCircle;
   else if(strcmp(s, "NNQ") == 0) t = QuadrantType::N;
   else if(strcmp(s, "EEQ") == 0) t = QuadrantType::E;
   else if(strcmp(s, "SSQ") == 0) t = QuadrantType::S;
   else if(strcmp(s, "WWQ") == 0) t = QuadrantType::W;
   else if(strcmp(s, "NEQ") == 0) t = QuadrantType::NE;
   else if(strcmp(s, "SEQ") == 0) t = QuadrantType::SE;
   else if(strcmp(s, "SWQ") == 0) t = QuadrantType::SW;
   else if(strcmp(s, "NWQ") == 0) t = QuadrantType::NW;
   else                           t = QuadrantType::None;

   return t;
}

////////////////////////////////////////////////////////////////////////

ConcatString quadranttype_to_string(const QuadrantType t) {
   const char *s = (const char *) nullptr;

   switch(t) {
      case QuadrantType::FullCircle:     s = "AAA";  break;
      case QuadrantType::N:     s = "NNQ";  break;
      case QuadrantType::E:     s = "EEQ";  break;
      case QuadrantType::S:     s = "SSQ";  break;
      case QuadrantType::W:     s = "WWQ";  break;
      case QuadrantType::NE:    s = "NEQ";  break;
      case QuadrantType::SE:    s = "SEQ";  break;
      case QuadrantType::SW:    s = "SWQ";  break;
      case QuadrantType::NW:    s = "NWQ";  break;
      case QuadrantType::None:  s = na_str; break;
      default:                  s = na_str; break;
   }

   return ConcatString(s);
}

////////////////////////////////////////////////////////////////////////

SubregionCode string_to_subregioncode(const char *s) {
   SubregionCode c;

        if(strcmp(s, "A") == 0) c = SubregionCode::Arabian_Sea;
   else if(strcmp(s, "B") == 0) c = SubregionCode::Bay_of_Bengal;
   else if(strcmp(s, "C") == 0) c = SubregionCode::Central_Pacific;
   else if(strcmp(s, "E") == 0) c = SubregionCode::Eastern_Pacific;
   else if(strcmp(s, "L") == 0) c = SubregionCode::Atlantic;
   else if(strcmp(s, "P") == 0) c = SubregionCode::South_Pacific;
   else if(strcmp(s, "Q") == 0) c = SubregionCode::South_Atlantic;
   else if(strcmp(s, "S") == 0) c = SubregionCode::South_IO;
   else if(strcmp(s, "W") == 0) c = SubregionCode::Western_Pacific;
   else                         c = SubregionCode::None;

   return c;
}

////////////////////////////////////////////////////////////////////////

ConcatString subregioncode_to_string(const SubregionCode t) {
   const char *s = (const char *) nullptr;

   switch(t) {
      case SubregionCode::Arabian_Sea:     s = "A";    break;
      case SubregionCode::Bay_of_Bengal:   s = "B";    break;
      case SubregionCode::Central_Pacific: s = "C";    break;
      case SubregionCode::Eastern_Pacific: s = "E";    break;
      case SubregionCode::Atlantic:        s = "L";    break;
      case SubregionCode::South_Pacific:   s = "P";    break;
      case SubregionCode::South_Atlantic:  s = "Q";    break;
      case SubregionCode::South_IO:        s = "S";    break;
      case SubregionCode::Western_Pacific: s = "W";    break;
      case SubregionCode::None           : s = na_str; break;
      default:                             s = na_str; break;
   }

   return ConcatString(s);
}

////////////////////////////////////////////////////////////////////////

SystemsDepth string_to_systemsdepth(const char *s) {
  SystemsDepth d;

        if(strcmp(s, "D") == 0) d = SystemsDepth::Deep;
   else if(strcmp(s, "M") == 0) d = SystemsDepth::Medium;
   else if(strcmp(s, "S") == 0) d = SystemsDepth::Shallow;
   else                         d = SystemsDepth::None;

   return d;
}

////////////////////////////////////////////////////////////////////////

ConcatString systemsdepth_to_string(const SystemsDepth t) {
   const char *s = (const char *) nullptr;

   switch(t) {
      case SystemsDepth::Deep:    s = "D";    break;
      case SystemsDepth::Medium:  s = "M";    break;
      case SystemsDepth::Shallow: s = "S";    break;
      case SystemsDepth::None:    s = na_str; break;
      default:                    s = na_str; break;
   }

   return ConcatString(s);
}

////////////////////////////////////////////////////////////////////////
