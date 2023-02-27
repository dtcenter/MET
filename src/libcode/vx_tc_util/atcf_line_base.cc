// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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
#include <map>

#include "vx_math.h"

#include "atcf_line_base.h"
#include "atcf_offsets.h"

////////////////////////////////////////////////////////////////////////

// Only print the AVN to GFS conversion message once
static bool print_avn_to_gfs_message = true;

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

   // Initialize pointers
   BasinMap      = (map<ConcatString,ConcatString> *) 0;
   BestTechnique = (StringArray *) 0;
   OperTechnique = (StringArray *) 0;
   TechSuffix    = (ConcatString *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLineBase::assign(const ATCFLineBase &l) {

   clear();

   DataLine::assign(l);

   BasinMap      = l.BasinMap;
   BestTechnique = l.BestTechnique;
   OperTechnique = l.OperTechnique;
   TechSuffix    = l.TechSuffix;

   Type          = l.Type;
   Basin         = l.Basin;
   Technique     = l.Technique;
   IsBestTrack   = l.IsBestTrack;
   IsOperTrack   = l.IsOperTrack;

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLineBase::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString cs;

   out << prefix << "Line            = " << (*this) << "\n";
   out << prefix << "Type            = " << atcflinetype_to_string(Type) << "\n";
   out << prefix << "Basin           = \"" << Basin.contents() << "\"\n";
   cs = cyclone_number();
   out << prefix << "CycloneNumber   = \"" << cs.contents() << "\"\n";
   out << prefix << "WarningTime     = " << unix_to_yyyymmdd_hhmmss(warning_time()) << "\n";
   out << prefix << "TechniqueNumber = " << technique_number() << "\n";
   out << prefix << "ForecastPeriod  = " << forecast_period() << "\n";
   out << prefix << "Valid           = " << unix_to_yyyymmdd_hhmmss(valid()) << "\n";
   cs = technique();
   out << prefix << "Technique       = \"" << cs.contents() << "\"\n";
   out << prefix << "IsBestTrack     = \"" << bool_to_string(IsBestTrack) << "\"\n";
   out << prefix << "IsOperTrack     = \"" << bool_to_string(IsOperTrack) << "\"\n";
   out << prefix << "Lat             = " << lat() << "\n";
   out << prefix << "Lon             = " << lon() << "\n";

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFLineBase::clear() {
   DataLine::clear();

   // Do not reset pointers:
   // BasinMap, BestTechnique, OperTechnique, TechSuffix

   Type = NoATCFLineType;
   Basin.clear();
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

   // Set the basin
   Basin = get_item(BasinOffset);

   // Update the basin name, if specified
   if(BasinMap) {
      if(BasinMap->count(Basin) > 0) Basin = BasinMap->at(Basin);
   }

   // Check for BEST and Operational tracks, if specified
   if(BestTechnique) IsBestTrack = BestTechnique->has(technique());
   if(OperTechnique) IsOperTrack = OperTechnique->has(technique());

   // Append the technique suffix, if specified
   if(TechSuffix) {
      if(TechSuffix->length() > 0) {
         ConcatString cs;
         cs << get_item(TechniqueOffset) << TechSuffix->c_str();
         Technique = cs;
      }
   }

   return(1);
}

////////////////////////////////////////////////////////////////////////

bool ATCFLineBase::is_header() const {
    if(basin().comparecase(0, 5, "BASIN") == 0) return(true);
    else                                        return(false);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::get_item(int i) const {
   ConcatString cs;
   int i_col = i;

   // For ATCFLineType_GenTrack:
   //    Columns 1 and 2 are consistent:
   //       Use offsets 0 and 1
   //    Column 3 for is an EXTRA column for this line type:
   //       Add special handling in storm_id()
   //    Columns 4-20 are the same as columns 3-19 of ATCFLineType_Track:
   //       Shift those column indices by 1.
   if(Type == ATCFLineType_GenTrack && i >= 2 && i <= 18) i_col++;

   cs = DataLine::get_item(i_col);

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
   return(Basin);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::cyclone_number() const {
   return(get_item(CycloneNumberOffset));
}

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
   if(strstr(cs.c_str(), "AVN") != NULL) {
      if(print_avn_to_gfs_message) {
         mlog << Debug(1)
              << "When reading ATCF track data, all instances of "
              << "\"AVN\" are automatically replaced with \"GFS\" "
              << "(ATCF ID = " << cs << ").\n";
         print_avn_to_gfs_message = false;
      }
      cs.replace("AVN", "GFS");
   }

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
   if(IsBestTrack && !is_bad_data(tn)) {
      ut += sec_per_minute * tn;
   }

   return(ut);
}

////////////////////////////////////////////////////////////////////////

int ATCFLineBase::valid_hour() const {
   return(unix_to_sec_of_day(valid()));
}

////////////////////////////////////////////////////////////////////////

int ATCFLineBase::lead() const {
   double fp = forecast_period();
   int    s  = bad_data_int;

   // Lead time for the BEST track is 0
   if(IsBestTrack) {
      s = 0;
   }
   else if(!is_bad_data(fp)) {
      s = sec_per_hour * fp;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ATCFLineBase::storm_id() const {
   ConcatString cs;

   // For ATCFLineType_GenTrack, use the contents of the extra 3rd column
   // Call DataLine::get_item() to avoid the column shifting logic
   if(Type == ATCFLineType_GenTrack) {
      cs = DataLine::get_item(GenStormIdOffset);
   }
   else {
      unixtime ut = valid();
      cs = define_storm_id(warning_time(), ut, ut, basin(),
                           cyclone_number());
   }

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
   int buf_len = m_strlen(s);
   if (buf_len > 0) {
      switch(s[buf_len - 1]) {
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
   int buf_len = m_strlen(s);
   if (buf_len > 0) {
      switch(s[buf_len - 1]) {
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
   }

   // Range check
   if(is_bad_data(v) || v < -360.0 || v > 360.0) v = bad_data_double;

   return(v);
}

////////////////////////////////////////////////////////////////////////

int parse_int(const char *s, const int bad_data) {
   int v;

   if(m_strlen(s) > 0) v = atoi(s);
   else              v = bad_data_int;

   // Check bad data value
   if(v == bad_data) v = bad_data_int;

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Interpret values of 0 as bad data
//
////////////////////////////////////////////////////////////////////////

int parse_int_check_zero(const char *s) {
   return(parse_int(s, 0));
}

////////////////////////////////////////////////////////////////////////

ATCFLineType string_to_atcflinetype(const char *s) {
   ATCFLineType t;

        if(!s)                       t = NoATCFLineType;
   // YYYYMMDDHH in the 4th column for Genesis Tracks
   else if(is_yyyymmddhh(s))         t = ATCFLineType_GenTrack;
   else if(is_number(s))             t = ATCFLineType_Track;  // ADECK
   else if(m_strlen(s) == 0)         t = ATCFLineType_Track;  // BDECK
   else if(strcasecmp(s, "TR") == 0) t = ATCFLineType_ProbTR;
   else if(strcasecmp(s, "IN") == 0) t = ATCFLineType_ProbIN;
   else if(strcasecmp(s, "RI") == 0) t = ATCFLineType_ProbRI;
   else if(strcasecmp(s, "RW") == 0) t = ATCFLineType_ProbRW;
   else if(strcasecmp(s, "WR") == 0) t = ATCFLineType_ProbWR;
   else if(strcasecmp(s, "PR") == 0) t = ATCFLineType_ProbPR;
   else if(strcasecmp(s, "GN") == 0) t = ATCFLineType_ProbGN;
   else if(strcasecmp(s, "GS") == 0) t = ATCFLineType_ProbGS;
   else if(strcasecmp(s, "ER") == 0) t = ATCFLineType_ProbER;
   else                              t = NoATCFLineType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString atcflinetype_to_string(const ATCFLineType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case ATCFLineType_Track:    s = "Track";    break;
      case ATCFLineType_GenTrack: s = "GenTrack"; break;
      case ATCFLineType_ProbTR:   s = "ProbTR";   break;
      case ATCFLineType_ProbIN:   s = "ProbIN";   break;
      case ATCFLineType_ProbRI:   s = "ProbRI";   break;
      case ATCFLineType_ProbRW:   s = "ProbRW";   break;
      case ATCFLineType_ProbWR:   s = "ProbWR";   break;
      case ATCFLineType_ProbPR:   s = "ProbPR";   break;
      case ATCFLineType_ProbGN:   s = "ProbGN";   break;
      case ATCFLineType_ProbGS:   s = "ProbGS";   break;
      case ATCFLineType_ProbER:   s = "ProbER";   break;
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
