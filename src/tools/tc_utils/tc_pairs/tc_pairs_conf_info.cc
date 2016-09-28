// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

   Desc.clear();
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
   Interp12 = Interp12Type_Replace;
   NConsensus = 0;
   LagTime.clear();
   BestTechnique.clear();
   BestBaseline.clear();
   OperTechnique.clear();
   OperBaseline.clear();
   AnlyTrack = TrackType_None;
   MatchPoints = false;
   DLandFile.clear();
   WatchWarnFile.clear();
   WatchWarnOffset = bad_data_int;
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
   Dictionary *dict = (Dictionary *) 0;

   // Conf: Version
   Version = Conf.lookup_string(conf_key_version);
   check_met_version(Version);

   // Conf: Desc
   Desc = parse_conf_string(&Conf, conf_key_desc);

   // Conf: Model
   Model = Conf.lookup_string_array(conf_key_model);

   // Conf: StormId
   StormId = Conf.lookup_string_array(conf_key_storm_id);

   // Conf: Basin
   Basin = Conf.lookup_string_array(conf_key_basin);

   // Conf: Cyclone
   Cyclone = Conf.lookup_string_array(conf_key_cyclone);

   // Conf: StormName
   StormName = Conf.lookup_string_array(conf_key_storm_name);

   // Conf: InitBeg, InitEnd
   InitBeg = timestring_to_unix(Conf.lookup_string(conf_key_init_beg));
   InitEnd = timestring_to_unix(Conf.lookup_string(conf_key_init_end));

   // Conf: InitInc
   sa = Conf.lookup_string_array(conf_key_init_inc);
   for(i=0; i<sa.n_elements(); i++)
      InitInc.add(timestring_to_unix(sa[i]));

   // Conf: InitExc
   sa = Conf.lookup_string_array(conf_key_init_exc);
   for(i=0; i<sa.n_elements(); i++)
      InitExc.add(timestring_to_unix(sa[i]));

   // Conf: InitHour
   sa = Conf.lookup_string_array(conf_key_init_hour);
   for(i=0; i<sa.n_elements(); i++)
      InitHour.add(timestring_to_sec(sa[i]));

   // Conf: ValidBeg, ValidEnd
   ValidBeg = timestring_to_unix(Conf.lookup_string(conf_key_valid_beg));
   ValidEnd = timestring_to_unix(Conf.lookup_string(conf_key_valid_end));

   // Conf: InitMask
   if(nonempty(Conf.lookup_string(conf_key_init_mask))) {
      poly_file = replace_path(Conf.lookup_string(conf_key_init_mask));
      mlog << Debug(2)
           << "Initialization polyline: " << poly_file << "\n";
      InitMask.load(poly_file);
   }

   // Conf: ValidMask
   if(nonempty(Conf.lookup_string(conf_key_valid_mask))) {
      poly_file = replace_path(Conf.lookup_string(conf_key_valid_mask));
      mlog << Debug(2)
           << "Valid polyline: " << poly_file << "\n";
      ValidMask.load(poly_file);
   }

   // Conf: CheckDup
   CheckDup = Conf.lookup_bool(conf_key_check_dup);

   // Conf: Interp12
   Interp12 = int_to_interp12type(Conf.lookup_int(conf_key_interp12));

   // Conf: Consensus
   dict = Conf.lookup_array(conf_key_consensus);

   // Set the consensus count
   NConsensus = dict->n_entries();
   Consensus  = new ConsensusInfo [NConsensus];

   // Loop over the consensus entries
   for(i=0; i<NConsensus; i++) {

      // Conf: Consensus: name, members, required, min_req
      Consensus[i].Name     = (*dict)[i]->dict_value()->lookup_string(conf_key_name);
      Consensus[i].Members  = (*dict)[i]->dict_value()->lookup_string_array(conf_key_members);
      Consensus[i].Required = (*dict)[i]->dict_value()->lookup_num_array(conf_key_required);
      Consensus[i].MinReq   = (*dict)[i]->dict_value()->lookup_int(conf_key_min_req);

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
   sa = Conf.lookup_string_array(conf_key_lag_time);
   for(i=0; i<sa.n_elements(); i++)
      LagTime.add(timestring_to_sec(sa[i]));

   // Conf: BestTechnique
   BestTechnique = Conf.lookup_string_array(conf_key_best_technique);
   BestTechnique.set_ignore_case(true);

   // Conf: BestBaseline
   BestBaseline = Conf.lookup_string_array(conf_key_best_baseline);

   if(BestTechnique.n() > 1 && BestBaseline.n() > 0) {
      mlog << Warning
           << "\nTCPairsConfInfo::process_config() -> "
           << "deriving \"best_baseline\" models from multiple "
           << "\"best_technique\" models may cause the baseline "
           << "technique names to be used multiple times.\n\n";
   }

   // Conf: OperTechnique
   OperTechnique = Conf.lookup_string_array(conf_key_oper_technique);
   OperTechnique.set_ignore_case(true);

   // Conf: OperBaseline
   OperBaseline = Conf.lookup_string_array(conf_key_oper_baseline);

   if(OperTechnique.n() > 1 && OperBaseline.n() > 0) {
      mlog << Warning
           << "\nTCPairsConfInfo::process_config() -> "
           << "deriving \"oper_baseline\" models from multiple "
           << "\"oper_technique\" models may cause the baseline "
           << "technique names to be used multiple times.\n\n";
   }

   // Conf: AnlyTrack
   AnlyTrack = int_to_tracktype(Conf.lookup_int(conf_key_anly_track));

   // Conf: MatchPoints
   MatchPoints = Conf.lookup_bool(conf_key_match_points);

   // Conf: DLandFile
   DLandFile = Conf.lookup_string(conf_key_dland_file);

   // Conf: Watch/Warning dictionary
   dict = Conf.lookup_dictionary(conf_key_watch_warn);

   // Conf: WatchWarnFile
   WatchWarnFile = dict->lookup_string(conf_key_file_name);

   // Conf: WatchWarnOffset
   WatchWarnOffset = dict->lookup_int(conf_key_time_offset);

   return;
}

////////////////////////////////////////////////////////////////////////
