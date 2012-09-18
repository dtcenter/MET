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
   Consensus = (ConsensusInfo *) 0;
  
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPairsConfInfo::clear() {

   // Deallocate memory
   if(Consensus) { delete [] Consensus; Consensus = (ConsensusInfo *) 0; }

   Model.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   InitBeg = InitEnd = (unixtime) 0;
   InitHour.clear();
   ValidBeg = ValidEnd = (unixtime) 0;
   InitMask.clear();
   ValidMask.clear();
   CheckDup = true;
   Interp12 = true;
   NConsensus = 0;
   LagTime.clear();
   BestBaseline.clear();
   OperBaseline.clear();
   MatchPoints = false;
   DLandFile.clear();
   WatchWarnFile.clear();
   Version.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPairsConfInfo::read_config(const char *default_file_name,
                                  const char *user_file_name) {

   // Read the config file constants
   Conf.read(replace_path(config_const_filename));
  
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
   int i, j;
   StringArray sa;
   ConcatString poly_file;
   Dictionary *con_dict = (Dictionary *) 0;

   // Conf: Version
   Version = Conf.lookup_string("version");
   check_met_version(Version);
   
   // Conf: Model
   Model = Conf.lookup_string_array("model");

   // Conf: StormId
   StormId = Conf.lookup_string_array("storm_id");

   // Conf: Basin
   Basin = Conf.lookup_string_array("basin");

   // Conf: Cyclone
   Cyclone = Conf.lookup_string_array("cyclone");

   // Conf: StormName
   Cyclone = Conf.lookup_string_array("storm_name");

   // Conf: InitBeg, InitEnd
   InitBeg = timestring_to_unix(Conf.lookup_string("init_beg"));
   InitEnd = timestring_to_unix(Conf.lookup_string("init_end"));

   // Conf: InitInc
   sa = Conf.lookup_string_array("init_inc");
   for(i=0; i<sa.n_elements(); i++)
      InitInc.add(timestring_to_unix(sa[i]));
   
   // Conf: InitExc
   sa = Conf.lookup_string_array("init_exc");
   for(i=0; i<sa.n_elements(); i++)
      InitExc.add(timestring_to_unix(sa[i]));
   
   // Conf: InitHour
   sa = Conf.lookup_string_array("init_hour");
   for(i=0; i<sa.n_elements(); i++)
      InitHour.add(timestring_to_sec(sa[i]));

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

   // Conf: CheckDup
   CheckDup = Conf.lookup_bool("check_dup");

   // Conf: Interp12
   Interp12 = Conf.lookup_bool("interp12");

   // Conf: Consensus
   con_dict = Conf.lookup_array("consensus");

   // Set the consensus count
   NConsensus = con_dict->n_entries();
   Consensus  = new ConsensusInfo [NConsensus];

   // Loop over the consensus entries
   for(i=0; i<NConsensus; i++) {

      // Conf: Consensus: name, members, required, min_req
      Consensus[i].Name     = (*con_dict)[i]->dict_value()->lookup_string("name");
      Consensus[i].Members  = (*con_dict)[i]->dict_value()->lookup_string_array("members");
      Consensus[i].Required = (*con_dict)[i]->dict_value()->lookup_num_array("required");
      Consensus[i].MinReq   = (*con_dict)[i]->dict_value()->lookup_int("min_req");

      // If required is empty, default to 0
      if(Consensus[i].Required.n_elements() == 0) {
         for(j=0; j<Consensus[i].Members.n_elements(); j++) {
            Consensus[i].Required.add(0);
         }
      }
      else if(Consensus[i].Required.n_elements() !=
              Consensus[i].Members.n_elements()) {
         mlog << Error
              << "\nTCPairsConfInfo::process_config() -> "
              << "\"consensus.required\" must either be empty "
              << "or the same length as \"consensus.members\".\n\n";
         exit(1);
      }
   }

   // Conf: LagTime
   sa = Conf.lookup_string_array("lag_time");
   for(i=0; i<sa.n_elements(); i++)
      LagTime.add(timestring_to_sec(sa[i]));

   // Conf: BestBaseline
   BestBaseline = Conf.lookup_string_array("best_baseline");

   // Conf: OperBaseline
   OperBaseline = Conf.lookup_string_array("oper_baseline");

   // Conf: MatchPoints
   MatchPoints = Conf.lookup_bool("match_points");   

   // Conf: DLandFile
   DLandFile = Conf.lookup_string("dland_file");

   // Conf: WatchWarnFile
   WatchWarnFile = Conf.lookup_string("watch_warn_file");
   
   return;
}

////////////////////////////////////////////////////////////////////////
