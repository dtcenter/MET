// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "tc_pairs_conf_info.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCPairsConfInfo
//
////////////////////////////////////////////////////////////////////////

TCPairsConfInfo::TCPairsConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCPairsConfInfo::~TCPairsConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCPairsConfInfo::init_from_scratch() {

   // Initialize pointers
   ConMembers = (StringArray *) 0;
  
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPairsConfInfo::clear() {

   // Deallocate memory
   if(ConMembers) { delete [] ConMembers; ConMembers = (StringArray *) 0; }
  
   Basin.clear();
   Cyclone.clear();
   Model.clear();
   NCon = 0;
   ConModel.clear();
   ConMinReq.clear();
   InitBeg = InitEnd = (unixtime) 0;
   InitHH.clear();   
   ValidBeg = ValidEnd = (unixtime) 0;
   InitMask.clear();
   ValidMask.clear();
   DLandFile.clear();
   Interp12 = true;
   CheckDup = true;
   MatchPoints = false;
   Version.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPairsConfInfo::read_config(const char *default_file_name,
                                  const char *user_file_name) {

   // Read the default config file
   Conf.read(default_file_name);

   // Read the user-specified config file
   Conf.read(user_file_name);

   // Process the configuration file
   process_config();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPairsConfInfo::process_config() {
   int i;
   StringArray sa;
   ConcatString poly_file;

   // Conf: Version
   Version = Conf.lookup_string("version");
   check_met_version(Version);

   // Conf: Basin
   Basin = Conf.lookup_string_array("basin");

   // Conf: Cyclone
   Cyclone = Conf.lookup_string_array("cyclone");

   // Conf: Model
   Model = Conf.lookup_string_array("model");

   // Conf: ConModel
   ConModel = Conf.lookup_string_array("con_model");

   // Conf: ConMembers
   sa = Conf.lookup_string_array("con_members");
   ConMembers = new StringArray [sa.n_elements()];
   for(i=0; i<sa.n_elements(); i++)
      ConMembers[i].parse_wsss(sa[i]);

   // Conf: ConMinReq
   ConMinReq = Conf.lookup_num_array("con_min_req");
   
   // Conf: NCon
   NCon = sa.n_elements();

   // Check for consistent consensus parameters.
   if(NCon != ConModel.n_elements() ||
      NCon != ConMinReq.n_elements()) {
      mlog << Error
           << "\nTCPairsConfInfo::process_config() -> "
           << "The \"ConModel\", \"ConMembers\", and \"ConMinReq\" "
           << "entries must all have the same length.\n\n";
      exit(1);
   }   

   // Conf: InitBeg, InitEnd
   InitBeg = timestring_to_unix(Conf.lookup_string("init_beg"));
   InitEnd = timestring_to_unix(Conf.lookup_string("init_end"));

   // Conf: InitHH
   sa = Conf.lookup_string_array("init_hh");
   for(i=0; i<sa.n_elements(); i++)
      InitHH.add(timestring_to_sec(sa[i]));
   
   // Conf: ValidBeg, ValidEnd
   ValidBeg = timestring_to_unix(Conf.lookup_string("valid_beg"));
   ValidEnd = timestring_to_unix(Conf.lookup_string("valid_end"));

   // Conf: InitMask
   if(nonempty(Conf.lookup_string("init_mask"))) {
      poly_file = replace_path(Conf.lookup_string("init_mask"));
      mlog << Debug(2)
           << "Initialization polyline: " << poly_file << "\n";
      InitMask.load(poly_file);
   }

   // Conf: ValidMask
   if(nonempty(Conf.lookup_string("valid_mask"))) {
      poly_file = replace_path(Conf.lookup_string("valid_mask"));
      mlog << Debug(2)
           << "Valid polyline: " << poly_file << "\n";
      ValidMask.load(poly_file);
   }

   // Conf: DLandFile
   DLandFile = Conf.lookup_string("dland_file");

   // Conf: Interp12
   Interp12 = Conf.lookup_bool("interp12");

   // Conf: CheckDup
   CheckDup = Conf.lookup_bool("check_dup");

   // Conf: MatchPoints
   MatchPoints = Conf.lookup_bool("match_points");
   
   return;
}

////////////////////////////////////////////////////////////////////////
