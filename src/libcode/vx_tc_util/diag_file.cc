// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

static const char default_lsdiag_technique[] = "AVNO";

static const int lsdiag_wdth[] = {
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5
};
static int n_lsdiag_wdth = sizeof(lsdiag_wdth)/sizeof(*lsdiag_wdth);

static const int tcdiag_fill_value = 9999;
static const int lsdiag_fill_value = 9999;

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

   return(LeadTime[i]);
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

   return(Lat[i]);
}

////////////////////////////////////////////////////////////////////////

double DiagFile::lon(int i) const {

   // Check range
   if(i < 0 || i >= Lon.n()) {
      mlog << Error << "\nDiagFile::lon(int) -> "
           << "range check error for index value " << i << "\n\n";
      exit(1);
   }

   return(Lon[i]);
}

////////////////////////////////////////////////////////////////////////

bool DiagFile::has_diag(const string &str) const {
   return(DiagName.has(str));
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

   return(DiagVal[i]);
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

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void DiagFile::init_from_scratch() {

   // Initialize values
   FileType = DiagFileType_None;
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

void DiagFile::set_technique(const string &str) {

   Technique = str;

   // Replace instances of AVN with GFS
   if(strstr(Technique.c_str(), "AVN") != NULL) {
      Technique.replace("AVN", "GFS");
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::read_tcdiag(const std::string &path, const std::string &model_name) {
   int i;
   NumArray data;

   FileType = TCDiagFileType;

   // Initialize the technique name
   set_technique(model_name);

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
            if(Technique.empty()) set_technique(dl[1]);
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
         for(int i=2; i<dl.n_items(); i++) {
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

         // Finished parsing header
         break;
      }
   } // end while

   // Check for the expected number of items
   if(NTime != LeadTime.n() || NTime != Lat.n() || NTime != Lon.n()) {
      mlog << Error << "\nDiagFile::read_tcdiag() -> "
           << "the NTIME value (" << NTime
           << ") does not match the actual number of times ("
           << LeadTime.n() << "), latitudes (" << Lat.n()
           << "), or longitudes (" << Lon.n() << ")!\n\n";
      exit(1);
   }

   // Store the diagnostics data
   while(dl.read_line(this)) {

      // Skip empty lines
      if(dl.n_items() == 0) continue;

      // Quit reading at the COMMENTS section
      if((cs = dl[1]) == "COMMENTS") break;

      // first column contains the name
      cs = dl[0];

      // Skip certain lines
      if(cs.startswith("----") || cs.startswith("TIME") ||
         cs.startswith("NLEV") || cs.startswith("NVAR")) continue;

      // Parse the data values
      data.erase();
      for(i=2; i<dl.n_items(); i++) {
         data.add(atoi(dl[i]) == tcdiag_fill_value ?
                  bad_data_double : atof(dl[i]));
      }

      // Check for the expected number of items
      if(NTime != data.n()) {
         mlog << Error << "\nDiagFile::read_tcdiag() -> "
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
        << StormId << " " << Technique << " " << unix_to_yyyymmddhh(InitTime)
        << " TC diagnostics file: " << path << "\n";

   // Close the input file
   close();

   return;
}

////////////////////////////////////////////////////////////////////////

void DiagFile::read_lsdiag(const std::string &path, const std::string &model_name) {
   int i;
   NumArray data;

   FileType = LSDiagFileType;

   // Store the default lsdiag technique, unless otherwise specified
   if(model_name.size() > 0) set_technique(model_name);
   else                      set_technique(default_lsdiag_technique);

   // Open the file
   open(path.c_str());

   // Parse the header information from the first line
   DataLine dl;
   ConcatString cs;
   dl.read_line(this);

   // Check for the expected number of items
   if(dl.n_items() != 9 || strncasecmp(dl[8], "HEAD", strlen("HEAD") != 0)) {
      mlog << Error << "\nDiagFile::open_lsdiag() -> "
           << "unexpected header line in file: " << path << "\n\n";
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

   // Parse time and location info
   while(read_fwf_line(dl, lsdiag_wdth, n_lsdiag_wdth)) {

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

         // Finished parsing header
         break;
      }
   } // end while

   // Store the diagnostics data
   while(read_fwf_line(dl, lsdiag_wdth, n_lsdiag_wdth)) {

      // Skip empty lines
      if(dl.n_items() == 0) continue;

      // Check the 24th column
      cs = dl[23];

      // Quit reading at the LAST line
      if(cs == "LAST") break;

      // Parse the data values
      data.erase();
      for(i=2; i<23; i++) {
         data.add(atoi(dl[i]) == lsdiag_fill_value ?
                  bad_data_double : atof(dl[i]));
      }

      // Check for the expected number of items
      if(NTime != data.n()) {
         mlog << Error << "\nDiagFile::read_lsdiag() -> "
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
        << StormId << " " << Technique << " " << unix_to_yyyymmddhh(InitTime)
        << " LS diagnostics file: " << path << "\n";

   // Close the input file
   close();

   return;
}

////////////////////////////////////////////////////////////////////////
