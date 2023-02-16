// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

#include "apply_mask.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

void parse_conf_diag_info_map(Dictionary *,
        map<ConcatString,DiagInfo> &);

void parse_conf_diag_convert_map(Dictionary *,
        map< ConcatString,map<ConcatString,UserFunc_1Arg> > &);

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
   SkipConsensusMembers.clear();
   InitBeg = InitEnd = (unixtime) 0;
   InitInc.clear();
   InitExc.clear();
   InitHour.clear();
   ValidBeg = ValidEnd = (unixtime) 0;
   ValidInc.clear();
   ValidExc.clear();
   WriteValid.clear();
   InitMaskName.clear();
   InitPolyMask.clear();
   InitGridMask.clear();
   InitAreaMask.clear();
   ValidMaskName.clear();
   ValidPolyMask.clear();
   ValidGridMask.clear();
   ValidAreaMask.clear();
   LeadReq.clear();
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
   DiagInfoMap.clear();
   DiagConvertMap.clear();
   BasinMap.clear();
   Version.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPairsConfInfo::read_config(const char *default_file_name,
                                  const char *user_file_name) {

   // Read the config file constants
   Conf.read(replace_path(config_const_filename).c_str());

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
   check_met_version(Version.c_str());

   // Conf: Desc
   Desc = parse_conf_string(&Conf, conf_key_desc);

   // Conf: Model
   Model = parse_conf_tc_model(&Conf);

   // Conf: StormId
   StormId = Conf.lookup_string_array(conf_key_storm_id);

   // Conf: Basin
   Basin = Conf.lookup_string_array(conf_key_basin);

   // Conf: Cyclone
   Cyclone = Conf.lookup_string_array(conf_key_cyclone);

   // Conf: StormName
   StormName = Conf.lookup_string_array(conf_key_storm_name);

   // Conf: InitBeg, InitEnd
   InitBeg = Conf.lookup_unixtime(conf_key_init_beg);
   InitEnd = Conf.lookup_unixtime(conf_key_init_end);

   // Conf: InitInc
   sa = Conf.lookup_string_array(conf_key_init_inc);
   for(i=0; i<sa.n_elements(); i++)
      InitInc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: InitExc
   sa = Conf.lookup_string_array(conf_key_init_exc);
   for(i=0; i<sa.n_elements(); i++)
      InitExc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: InitHour
   sa = Conf.lookup_string_array(conf_key_init_hour);
   for(i=0; i<sa.n_elements(); i++)
      InitHour.add(timestring_to_sec(sa[i].c_str()));

   // Conf: ValidBeg, ValidEnd
   ValidBeg = Conf.lookup_unixtime(conf_key_valid_beg);
   ValidEnd = Conf.lookup_unixtime(conf_key_valid_end);

   // Conf: ValidInc
   sa = Conf.lookup_string_array(conf_key_valid_inc);
   for(i=0; i<sa.n_elements(); i++)
      ValidInc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: ValidExc
   sa = Conf.lookup_string_array(conf_key_valid_exc);
   for(i=0; i<sa.n_elements(); i++)
      ValidExc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: WriteValid
   sa = Conf.lookup_string_array(conf_key_write_valid);
   for(i=0; i<sa.n_elements(); i++)
      WriteValid.add(timestring_to_unix(sa[i].c_str()));

   // Conf: LeadReq
   sa = Conf.lookup_string_array(conf_key_lead_req);
   for(i=0; i<sa.n_elements(); i++){
      LeadReq.add(timestring_to_sec(sa[i].c_str()));
   }

   // Conf: InitMask
   if(nonempty(Conf.lookup_string(conf_key_init_mask).c_str())) {
      poly_file = replace_path(Conf.lookup_string(conf_key_init_mask));
      mlog << Debug(2)
           << "Init Points Masking File: " << poly_file << "\n";
      parse_poly_mask(poly_file, InitPolyMask, InitGridMask,
                      InitAreaMask, InitMaskName);
   }

   // Conf: ValidMask
   if(nonempty(Conf.lookup_string(conf_key_valid_mask).c_str())) {
      poly_file = replace_path(Conf.lookup_string(conf_key_valid_mask));
      mlog << Debug(2)
           << "Valid Point Masking File: " << poly_file << "\n";
      parse_poly_mask(poly_file, ValidPolyMask, ValidGridMask,
                      ValidAreaMask, ValidMaskName);
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

      // Conf: Consensus: name, members, required, min_req, write_members
      Consensus[i].Name     = (*dict)[i]->dict_value()->lookup_string(conf_key_name);
      Consensus[i].Members  = (*dict)[i]->dict_value()->lookup_string_array(conf_key_members);
      Consensus[i].Required = (*dict)[i]->dict_value()->lookup_num_array(conf_key_required);
      Consensus[i].MinReq   = (*dict)[i]->dict_value()->lookup_int(conf_key_min_req);
      Consensus[i].WriteMembers = true;
      Consensus[i].WriteMembers = (*dict)[i]->dict_value()->lookup_bool(conf_key_write_members);

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

      // If WriteMembers is false, save the Members to skip output for
      if(!Consensus[i].WriteMembers) {
         for(j=0; j<Consensus[i].Members.n_elements(); j++) {
            SkipConsensusMembers.add(Consensus[i].Members[j]);
         }
      }
   }

   // Conf: LagTime
   sa = Conf.lookup_string_array(conf_key_lag_time);
   for(i=0; i<sa.n_elements(); i++)
      LagTime.add(timestring_to_sec(sa[i].c_str()));

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

   // Conf: DiagInfoMap
   parse_conf_diag_info_map(dict, DiagInfoMap);

   // Conf: DiagConvertMap
   parse_conf_diag_convert_map(dict, DiagConvertMap);

   // Conf: BasinMap
   BasinMap = parse_conf_key_value_map(dict, conf_key_basin_map);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
////////////////////////////////////////////////////////////////////////

void DiagInfo::clear() {

   TrackSource.clear();
   FieldSource.clear();
   MatchToTrack.clear();
   DiagName.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_conf_diag_info_map(Dictionary *dict, map<ConcatString,DiagInfo> &source_map) {
   int i;
   Dictionary *map_dict = (Dictionary *) 0;
   ConcatString diag_source;
   DiagInfo cur_info;

   const char *method_name = "parse_conf_diag_info_map() -> ";

   if(!dict) {
      mlog << Error << "\n" << method_name
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: diag_info_map
   map_dict = dict->lookup_array(conf_key_diag_info_map);

   // Loop through the array entries
   for(i=0; i<map_dict->n_entries(); i++) {

      // Initialize the current map
      cur_info.clear();

      // Lookup the metadata entries
      diag_source           = (*map_dict)[i]->dict_value()->lookup_string(conf_key_diag_source);
      cur_info.TrackSource  = (*map_dict)[i]->dict_value()->lookup_string(conf_key_track_source);
      cur_info.FieldSource  = (*map_dict)[i]->dict_value()->lookup_string(conf_key_field_source);
      cur_info.MatchToTrack = (*map_dict)[i]->dict_value()->lookup_string_array(conf_key_match_to_track);
      cur_info.DiagName     = (*map_dict)[i]->dict_value()->lookup_string_array(conf_key_diag_name);

      // diag_source entries must be unique
      if(source_map.count(diag_source) > 0) {
         mlog << Error << "\n" << method_name
              << "multiple entries found for diag_source \"" << diag_source
              << "\" in \"" << conf_key_diag_info_map << "\"!\n\n";
         exit(1);
      }
      // Add a new source entry
      else {
         source_map[diag_source] = cur_info;
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_conf_diag_convert_map(Dictionary *dict,
        map< ConcatString,map<ConcatString,UserFunc_1Arg> > &source_map) {
   int i, j;
   Dictionary *map_dict = (Dictionary *) 0;
   map<ConcatString,UserFunc_1Arg> cur_map;
   ConcatString diag_source, key;
   StringArray sa;
   UserFunc_1Arg fx;

   const char *method_name = "parse_conf_diag_convert_map() -> ";

   if(!dict) {
      mlog << Error << "\n" << method_name
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: diag_convert_map
   map_dict = dict->lookup_array(conf_key_diag_convert_map);

   // Loop through the array entries
   for(i=0; i<map_dict->n_entries(); i++) {

      // Initialize the current map
      cur_map.clear();

      // Lookup the source, key, and convert function
      diag_source = (*map_dict)[i]->dict_value()->lookup_string(conf_key_diag_source);
      sa          = (*map_dict)[i]->dict_value()->lookup_string_array(conf_key_key);
      fx.clear();
      fx.set((*map_dict)[i]->dict_value()->lookup(conf_key_convert));

      // Check the function
      if(!fx.is_set()) {
         mlog << Error << "\n" << method_name
              << "lookup for \"" << conf_key_convert << "\" failed in the \""
              << conf_key_diag_convert_map << "\" map!\n\n";
         exit(1);
      }

      // Add entry to the current map for each string
      for(j=0; j<sa.n(); j++) {
         cur_map.insert(pair<ConcatString,UserFunc_1Arg>(sa[j],fx));
      }

      // Append to the existing source entry
      if(source_map.count(diag_source) > 0) {
         for(map<ConcatString,UserFunc_1Arg>::iterator it = cur_map.begin();
             it != cur_map.end(); it++) {
            source_map.at(diag_source).insert(pair<ConcatString,UserFunc_1Arg>(it->first, it->second));
         }
      }
      // Add a new source entry
      else {
         source_map.insert(pair< ConcatString,map<ConcatString,UserFunc_1Arg> >(diag_source, cur_map));
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
