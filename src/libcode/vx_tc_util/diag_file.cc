// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"

#include "diag_file.h"

////////////////////////////////////////////////////////////////////////

static const int ships_wdth[] = {
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5
};
static int n_ships_wdth = sizeof(ships_wdth)/sizeof(*ships_wdth);

static const int cira_fill_value  = 9999;
static const int ships_fill_value = 9999;

static const char ships_skip_str[] = "TIME,DELV,MTPW,IR00,IRM1,IRM3,PC00,PCM1,PCM3,PSLV,IRXX";

////////////////////////////////////////////////////////////////////////
//
//  Code for class DiagFile
//
////////////////////////////////////////////////////////////////////////

DiagFile::DiagFile() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

DiagFile::~DiagFile() {
   close();
}

////////////////////////////////////////////////////////////////////////

int DiagFile::lead(int i) const {

   // Check range
   if(i < 0 || i >= LeadTime.n()) {
      mlog << Error << "\nDiagFile::lead(int) -> "
           << "range check error for index value " << i << "\n\n";
      exit(1);
   }

   return LeadTime[i];
}

////////////////////////////////////////////////////////////////////////

unixtime DiagFile::valid(int i) const {

   // Check range
   if(i < 0 || i >= LeadTime.n()) {
      mlog << Error << "\nDiagFile::valid(int) -> "
           << "range check error for index value " << i << "\n\n";
      exit(1);
   }

   return(InitTime == 0 || is_bad_data(LeadTime[i]) ?
          0 : InitTime + LeadTime[i]);
}

////////////////////////////////////////////////////////////////////////

double DiagFile::lat(int i) const {

   // Check range
   if(i < 0 || i >= Lat.n()) {
      mlog << Error << "\nDiagFile::lat(int) -> "
           << "range check error for index value " << i << "\n\n";
      exit(1);
   }

   return Lat[i];
}

////////////////////////////////////////////////////////////////////////

double DiagFile::lon(int i) const {

   // Check range
   if(i < 0 || i >= Lon.n()) {
      mlog << Error << "\nDiagFile::lon(int) -> "
           << "range check error for index value " << i << "\n\n";
      exit(1);
   }

   return Lon[i];
}

////////////////////////////////////////////////////////////////////////

bool DiagFile::has_diag(const string &str) const {
   return DiagName.has(str);
}

////////////////////////////////////////////////////////////////////////

const NumArray & DiagFile::diag_val(const string &str) const {
   int i;

   // Find the index of the name
   if(!DiagName.has(str, i)) {
      mlog << Error << "\nDiagFile::diag_val() -> "
           << "requested diagnostic name \"" << str << "\" not found!\n\n";
      exit(1);
   }

   return DiagVal[i];
}

////////////////////////////////////////////////////////////////////////

DiagFile::DiagFile(const DiagFile &) {
   mlog << Error << "\nDiagFile::DiagFile(const DiagFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

DiagFile & DiagFile::operator=(const DiagFile &) {
   mlog << Error << "\nDiagFile::operator=(const DiagFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

void DiagFile::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::clear() {

   // Initialize values
   DiagSource = DiagType_None;
   TrackSource.clear();
   FieldSource.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   Technique.clear();
   InitTime = (unixtime) 0;

   NTime = 0;
   LeadTime.clear();
   Lat.clear();
   Lon.clear();
   DiagName.clear();
   DiagVal.clear();

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::set_technique(const StringArray &sa) {

   for(int i=0; i<sa.n(); i++) add_technique(sa[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::add_technique(const string &str) {

   // Replace instances of AVN with GFS
   ConcatString cs(str);
   cs.replace("AVN", "GFS");
   Technique.add(cs);

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::read(const ConcatString &path, const ConcatString &diag_source,
                    const ConcatString &track_source, const ConcatString &field_source,
                    const StringArray &match_to_track,
                    const std::map<ConcatString,UserFunc_1Arg> *convert_map) {

   DiagType type = string_to_diagtype(diag_source.c_str());

   // Read diagnostics based on the source type
   if(type == DiagType_CIRA_RT) {
      read_cira_rt(path, convert_map);
   }
   else if(type == DiagType_SHIPS_RT) {
      read_ships_rt(path, convert_map);
   }
   else {
      mlog << Error << "\nDiagFile::read() -> "
           << "diagnostics of type \"" << diag_source
           << "\" are not currently supported!\n\n";
      exit(1);
   }

   // Store the metadata
   TrackSource = track_source;
   FieldSource = field_source;
   set_technique(match_to_track);

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::read_cira_rt(const ConcatString &path,
                           const map<ConcatString,UserFunc_1Arg> *convert_map) {
   int i;
   double v_in, v_out;
   NumArray data;
   const UserFunc_1Arg *fx_ptr = 0;

   // Initialize
   clear();

   // Store the file type
   DiagSource = DiagType_CIRA_RT;

   // Open the file
   open(path.c_str());

   // Parse the header information
   DataLine dl;
   ConcatString cs;
   while(dl.read_line(this)) {

      // Skip empty lines
      if(dl.n_items() == 0) continue;

      // First column
      cs = dl[0];

      // Parse header lines
      if(cs == "*") {

         // First header line: Technique InitTime (ATCFID YYYMMDDHH)
         if(InitTime == 0) {
            if(Technique.n() == 0) add_technique(dl[1]);
            InitTime = timestring_to_unix(dl[2]);
         }
         // Second header line: Basin Cyclone Number (BBCC)
         else if(StormId.empty()) {
            string bbcc = dl[1];
            Basin   = bbcc.substr(0, 2);
            Cyclone = bbcc.substr(2, 2);
            int mon, day, yr, hr, min, sec;
            unix_to_mdyhms(InitTime, mon, day, yr, hr, min, sec);
            StormId << Basin << Cyclone << yr;
         }
      }
      // Parse time and location info
      else if(cs == "NTIME") {
         NTime = atoi(dl[1]);
      }
      else if(cs == "TIME") {
         for(i=2; i<dl.n_items(); i++) {
            LeadTime.add(atoi(dl[i])*sec_per_hour);
         }
      }
      else if(cs == "LAT") {
         for(i=2; i<dl.n_items(); i++) {
            Lat.add(atof(dl[i]));
         }
      }
      else if(cs == "LON") {
         for(i=2; i<dl.n_items(); i++) {
            Lon.add(rescale_lon(atof(dl[i])));
         }

         // Finished parsing the metadata
         break;
      }
   } // end while

   // Check for the expected number of items
   if(NTime != LeadTime.n() || NTime != Lat.n() || NTime != Lon.n()) {
      mlog << Error << "\nDiagFile::read_cira_rt() -> "
           << "the NTIME value (" << NTime
           << ") does not match the actual number of times ("
           << LeadTime.n() << "), latitudes (" << Lat.n()
           << "), or longitudes (" << Lon.n() << "): "
           << path << "\n\n";
      exit(1);
   }

   // Rewind to beginning in case data rows precede the header
   rewind();

   // Store the diagnostics data
   while(dl.read_line(this)) {

      // Skip empty lines
      if(dl.n_items() == 0) continue;

      // First column contains the diagnostic name
      cs = dl[0];

      // Skip non-diagnostic data lines
      if(cs.startswith("*")     || cs.startswith("----") ||
         cs.startswith("NTIME") || cs.startswith("TIME") ||
         cs.startswith("NLEV")  || cs.startswith("NVAR")) continue;

      // Check for a conversion function based on the diagnostic name or units
      if(convert_map) {
              if(convert_map->count(cs) > 0)    fx_ptr = &convert_map->at(cs);
         else if(convert_map->count(dl[1]) > 0) fx_ptr = &convert_map->at(dl[1]);
         else                                   fx_ptr = (UserFunc_1Arg *) nullptr;
      }
      else {
         fx_ptr = (UserFunc_1Arg *) nullptr;
      }

      // Parse the data values
      data.erase();
      for(int i=2; i<dl.n_items(); i++) {

         // Read as float since LAT and LON as specified that way
         v_in = atof(dl[i]);

         // Check for bad data and apply conversions
         if(atoi(dl[i]) == cira_fill_value) v_out = bad_data_double;
         else if(fx_ptr)                      v_out = (*fx_ptr)(v_in);
         else                                 v_out = v_in;

         // Store the value
         data.add(v_out);
      }

      // Check for the expected number of items
      if(NTime != data.n()) {
         mlog << Error << "\nDiagFile::read_cira_rt() -> "
              << "the number of \"" << cs << "\" diagnostic values ("
              << data.n() << ") does not match the expected number ("
              << NTime << "): " << path << "\n\n";
         exit(1);
      }
      // Store the name and values
      else {
         DiagName.add(cs);
         DiagVal.push_back(data);
      }
   } // end while

   mlog << Debug(4) << "Parsed " << DiagName.n() << " diagnostic values from "
        << StormId << " " << write_css(Technique) << " " << unix_to_yyyymmddhh(InitTime)
        << " TC diagnostics file: " << path << "\n";

   // Close the input file
   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::read_ships_rt(const ConcatString &path,
                             const map<ConcatString,UserFunc_1Arg> *convert_map) {
   int i, v_int;
   double v_dbl;
   NumArray data;
   const UserFunc_1Arg *fx_ptr = 0;

   // Initialize
   clear();

   // Store the file type
   DiagSource = DiagType_SHIPS_RT;

   // Open the file
   open(path.c_str());

   // Diagnostic names to ignore
   StringArray skip_diag_sa;
   skip_diag_sa.parse_css(ships_skip_str);

   // Parse the header information from the first line
   DataLine dl;
   ConcatString cs;
   dl.read_line(this);

   // Check for the expected number of items
   if(dl.n_items() != 9 || strncasecmp(dl[8], "HEAD", strlen("HEAD") != 0)) {
      mlog << Error << "\nDiagFile::read_ships_rt() -> "
           << "unexpected header line: " << path << "\n\n";
      exit(1);
   }

   // Parse storm information
   StormId = dl[7];
   Basin   = StormId.string().substr(0, 2);
   Cyclone = StormId.string().substr(2, 2);

   // Parse timing info
   cs = dl[1];
   int yr   = atoi(StormId.string().substr(4, 4).c_str());
   int mon  = atoi(cs.string().substr(2, 2).c_str());
   int day  = atoi(cs.string().substr(4, 2).c_str());
   int hr   = atoi(dl[2]);
   InitTime = mdyhms_to_unix(mon, day, yr, hr, 0, 0);

   // Store the location of the beginning of the data
   int data_start_location = in->tellg();

   // Parse time and location info
   while(read_fwf_line(dl, ships_wdth, n_ships_wdth)) {

      // Fixed width column 24 has the data name
      cs = dl[23];

      // Strip any whitespace from the fixed-width column
      cs.ws_strip();

      if(cs == "TIME") {
         for(i=2; i<23; i++) {
            LeadTime.add(atoi(dl[i])*sec_per_hour);
         }
         NTime = LeadTime.n();
      }
      else if(cs == "LAT") {
         // Tenths of degree north
         for(i=2; i<23; i++) {
            Lat.add(atof(dl[i])/10.0);
         }
      }
      else if(cs == "LON") {
         for(i=2; i<23; i++) {
            // Tenths of degrees west (convert to east)
            Lon.add(rescale_lon(-1.0*atof(dl[i])/10.0));
         }

         // Finished parsing the metadata
         break;
      }
   } // end while

   // Rewind to the beginning of the data
   in->seekg(data_start_location);

   // Store the diagnostics data
   while(read_fwf_line(dl, ships_wdth, n_ships_wdth)) {

      // Skip empty lines
      if(dl.n_items() == 0) continue;

      // The 24th column contains the diagnostic name
      cs = dl[23];

      // Strip any whitespace from the fixed-width column
      cs.ws_strip();

      // Check for diagnostic names to skip
      if(skip_diag_sa.has(cs)) continue;

      // Quit reading at the LAST line
      if(cs == "LAST") break;

      // Check for a conversion function
      if(convert_map) {
         if(convert_map->count(cs) > 0) fx_ptr = &convert_map->at(cs);
         else                           fx_ptr = (UserFunc_1Arg *) nullptr;
      }
      else {
         fx_ptr = (UserFunc_1Arg *) nullptr;
      }

      // Parse the data values
      data.erase();
      for(i=2; i<23; i++) {

         v_int = atoi(dl[i]);

         // Check for bad data and apply conversions
         if(v_int == ships_fill_value) v_dbl = bad_data_double;
         else if(fx_ptr)                v_dbl = (*fx_ptr)(v_int);
         else                           v_dbl = (double) v_int;

         // Store the value
         data.add(v_dbl);
      }

      // Check for the expected number of items
      if(NTime != data.n()) {
         mlog << Error << "\nDiagFile::read_ships_rt() -> "
              << "the number of \"" << cs << "\" diagnostic values ("
              << data.n() << ") does not match the expected number ("
              << NTime << ")!\n\n";
         exit(1);
      }
      // Store the name and values
      else {
         DiagName.add(cs);
         DiagVal.push_back(data);
      }
   } // end while

   mlog << Debug(4) << "Parsed " << DiagName.n() << " diagnostic values from "
        << StormId << " " << write_css(Technique) << " " << unix_to_yyyymmddhh(InitTime)
        << " LS diagnostics file: " << path << "\n";

   // Close the input file
   close();

   return;
}

////////////////////////////////////////////////////////////////////////
