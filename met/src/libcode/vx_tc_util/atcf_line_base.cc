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

#include "atcf_line_base.h"
#include "atcf_offsets.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ATCFLineBase
//
////////////////////////////////////////////////////////////////////////

ATCFLineBase::ATCFLineBase() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ATCFLineBase::~ATCFLineBase() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ATCFLineBase::ATCFLineBase(const ATCFLineBase &l) {
   init_from_scratch();

   assign(l);
}

////////////////////////////////////////////////////////////////////////

ATCFLineBase & ATCFLineBase::operator=(const ATCFLineBase &l) {

   if(this == &l) return(*this);

   assign(l);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

bool ATCFLineBase::operator==(const ATCFLineBase &l) {
   return(get_line() == l.get_line());
}

////////////////////////////////////////////////////////////////////////

void ATCFLineBase::init_from_scratch() {

   DataLine::init_from_scratch();

   // ATCF lines are comma-delimited
   set_delimiter(",");

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLineBase::assign(const ATCFLineBase &l) {

   clear();

   DataLine::assign(l);

   Type = l.Type;
   Technique = l.Technique;
   IsBestTrack = l.IsBestTrack;
   IsOperTrack = l.IsOperTrack;

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLineBase::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString cs;

   out << prefix << "Line            = " << (*this) << "\n";
   out << prefix << "Type            = " << atcflinetype_to_string(Type) << "\n";
   cs = basin();
   out << prefix << "Basin           = \"" << cs.contents() << "\"\n";
   cs = cyclone_number();
   out << prefix << "CycloneNumber   = \"" << cs.contents() << "\"\n";
   out << prefix << "WarningTime     = " << unix_to_yyyymmdd_hhmmss(warning_time()) << "\n";
   out << prefix << "TechniqueNumber = " << technique_number() << "\n";
   out << prefix << "ForecastPeriod  = " << forecast_period() << "\n";
   out << prefix << "Valid           = " << unix_to_yyyymmdd_hhmmss(valid()) << "\n";
   cs = technique();
   out << prefix << "Technique       = \"" << cs.contents() << "\"\n";
   out << prefix << "IsBestTrack     = \"" << (IsBestTrack ? "TRUE" : "FALSE") << "\"\n";
   out << prefix << "IsOperTrack     = \"" << (IsOperTrack ? "TRUE" : "FALSE") << "\"\n";
   out << prefix << "Lat             = " << lat() << "\n";
   out << prefix << "Lon             = " << lon() << "\n";

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLineBase::clear() {
   DataLine::clear();
   Type = NoATCFLineType;
   Technique.clear();
   IsBestTrack = false;
   IsOperTrack = false;

   return;
}

////////////////////////////////////////////////////////////////////////

int ATCFLineBase::read_line(LineDataFile * ldf) {
   int status;

   clear();

   status = DataLine::read_line(ldf);

   // Check for bad return status or blank line
   if(!status || n_items() == 0) {
      clear();
      return(0);
   }

   // Set the line type from the technique number column
   Type = string_to_atcflinetype(get_item(TechniqueNumberOffset).c_str());

   return(1);
}

////////////////////////////////////////////////////////////////////////

int ATCFLineBase::is_header() const {
  if(basin().comparecase(0, 5, "BASIN") == 0) return(1);
   else                                  return(0);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::get_item(int i) const {
   ConcatString cs;

   cs = DataLine::get_item(i);

   // Strip off any whitespace
   cs.ws_strip();

   return(cs);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::get_line() const {
   ConcatString cs;

   if(N_items == 0) return(cs);

   for(int i=0; i<N_items-1; i++) cs << DataLine::get_item(i) << DataLine::get_delimiter();
   cs << DataLine::get_item(N_items-1);

   return(cs);
}

////////////////////////////////////////////////////////////////////////

ATCFLineType ATCFLineBase::type() const {
   return(Type);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::basin() const {
   return(get_item(BasinOffset));
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::cyclone_number() const {
   return(get_item(CycloneNumberOffset));   }

////////////////////////////////////////////////////////////////////////

unixtime ATCFLineBase::warning_time() const {
   return(parse_time(get_item(WarningTimeOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////

int ATCFLineBase::technique_number() const {
   return(parse_int(get_item(TechniqueNumberOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::technique() const {
   ConcatString cs;

   // Use Technique, if already set
   if(Technique.nonempty()) cs = Technique;
   else                     cs = get_item(TechniqueOffset);

   // Replace instances of AVN with GFS
   cs.replace("AVN", "GFS");

   return(cs);
}

////////////////////////////////////////////////////////////////////////

int ATCFLineBase::forecast_period() const {
   return(parse_int(get_item(ForecastPeriodOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////

double ATCFLineBase::lat() const {
   return(parse_lat(get_item(LatTenthsOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////

double ATCFLineBase::lon() const {
   return(parse_lon(get_item(LonTenthsOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////

unixtime ATCFLineBase::valid() const {
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

int ATCFLineBase::lead() const {
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

ConcatString ATCFLineBase::storm_id() const {

   unixtime ut = valid();
   ConcatString cs = define_storm_id(warning_time(), ut, ut,
                                     basin(), cyclone_number());

   return(cs);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
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

double parse_lat(const char *s) {
   double v;

   v = parse_int(s);

   // Convert tenths of a degree to degrees
   switch(s[strlen(s) - 1]) {
      case 'N': v *=  0.1; break;
      case 'S': v *= -0.1; break; // Degrees south is negative
      default:
         mlog << Warning
              << "\nint parse_lat(const char *) -> "
              << "bad latitude ... \"" << s
              << "\"\n\n";
         v = bad_data_double;
         break;
   }

   // Range check
   if(is_bad_data(v) || v < -90.0 || v > 90.0) v = bad_data_double;

   return(v);
}

////////////////////////////////////////////////////////////////////////

double parse_lon(const char *s) {
   double v;

   v = parse_int(s);

   // Convert tenths of a degree to degrees
   switch(s[strlen(s) - 1]) {
      case 'E': v *=  0.1; break;
      case 'W': v *= -0.1; break; // Degrees west is negative
      default:
         mlog << Warning
              << "\nint parse_lon(const char *) -> "
              << "bad longitude ... \"" << s
              << "\"\n\n";
         v = bad_data_double;
         break;
   }

   // Range check
   if(is_bad_data(v) || v < -360.0 || v > 360.0) v = bad_data_double;

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

ATCFLineType string_to_atcflinetype(const char *s) {
   ATCFLineType t;

        if(!s)                       t = NoATCFLineType;
   else if(is_number(s))             t = ATCFLineType_Track;  // ADECK
   else if(strlen(s) == 0)           t = ATCFLineType_Track;  // BDECK
   else if(strcasecmp(s, "TR") == 0) t = ATCFLineType_ProbTR;
   else if(strcasecmp(s, "IN") == 0) t = ATCFLineType_ProbIN;
   else if(strcasecmp(s, "RI") == 0) t = ATCFLineType_ProbRIRW;
   else if(strcasecmp(s, "WD") == 0) t = ATCFLineType_ProbWD;
   else if(strcasecmp(s, "PR") == 0) t = ATCFLineType_ProbPR;
   else if(strcasecmp(s, "GN") == 0) t = ATCFLineType_ProbGN;
   else if(strcasecmp(s, "GS") == 0) t = ATCFLineType_ProbGS;
   else                              t = NoATCFLineType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString atcflinetype_to_string(const ATCFLineType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case ATCFLineType_Track:    s = "Track";    break;
      case ATCFLineType_ProbTR:   s = "ProbTR";   break;
      case ATCFLineType_ProbIN:   s = "ProbIN";   break;
      case ATCFLineType_ProbRIRW: s = "ProbRIRW"; break;
      case ATCFLineType_ProbWD:   s = "ProbWD";   break;
      case ATCFLineType_ProbPR:   s = "ProbPR";   break;
      case ATCFLineType_ProbGN:   s = "ProbGN";   break;
      case ATCFLineType_ProbGS:   s = "ProbGS";   break;
      case NoATCFLineType:        s = na_str;     break;
      default:                    s = na_str;     break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////

ConcatString define_storm_id(unixtime init_ut, unixtime min_valid_ut,
                             unixtime max_valid_ut,
                             const ConcatString &basin,
                             const ConcatString &cyclone) {
   ConcatString storm_id;
   int year, mon, day, hr, minute, sec;
   unixtime ut;

   // Use timing information to determine the year.
        if(init_ut > 0)      ut = init_ut;
   else if(min_valid_ut > 0) ut = min_valid_ut;
   else                      ut = max_valid_ut;

   // Ensure that the storm_id components are valid
   if(!basin.empty() && !cyclone.empty() && ut > 0) {

      unix_to_mdyhms(ut, mon, day, year, hr, minute, sec);

      // Set StormId
      storm_id << basin << cyclone << year;
   }

   return(storm_id);
}

////////////////////////////////////////////////////////////////////////
