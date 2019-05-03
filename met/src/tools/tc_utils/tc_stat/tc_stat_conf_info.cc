// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "tc_stat_conf_info.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCStatConfInfo
//
////////////////////////////////////////////////////////////////////////

TCStatConfInfo::TCStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCStatConfInfo::~TCStatConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCStatConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatConfInfo::clear() {

   Version.clear();
   Filter.clear();
   Jobs.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatConfInfo::read_config(const char *default_file_name,
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

void TCStatConfInfo::process_config() {
   int i;
   StringArray sa, sa_val, sa_new;
   ThreshArray ta_val, ta_new;
   ConcatString poly_file;

   // Conf: Version
   Version = Conf.lookup_string(conf_key_version);
   check_met_version(Version.c_str());

   // Conf: TCStatJob::AModel
   Filter.AModel = Conf.lookup_string_array(conf_key_amodel);

   // Conf: TCStatJob::BModel
   Filter.BModel = Conf.lookup_string_array(conf_key_bmodel);

   // Conf: TCStatJob::Desc
   Filter.Desc = Conf.lookup_string_array(conf_key_desc);

   // Conf: TCStatJob::StormId
   Filter.StormId = Conf.lookup_string_array(conf_key_storm_id);

   // Conf: TCStatJob::Basin
   Filter.Basin = Conf.lookup_string_array(conf_key_basin);

   // Conf: TCStatJob::Cyclone
   Filter.Cyclone = Conf.lookup_string_array(conf_key_cyclone);

   // Conf: TCStatJob::StormName
   Filter.StormName = Conf.lookup_string_array(conf_key_storm_name);

   // Conf: TCStatJob::InitBeg, TCStatJob::InitEnd
   Filter.InitBeg = Conf.lookup_unixtime(conf_key_init_beg);
   Filter.InitEnd = Conf.lookup_unixtime(conf_key_init_end);

   // Conf: TCStatJob::InitInc
   sa = Conf.lookup_string_array(conf_key_init_inc);
   for(i=0; i<sa.n_elements(); i++)
      Filter.InitInc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::InitExc
   sa = Conf.lookup_string_array(conf_key_init_exc);
   for(i=0; i<sa.n_elements(); i++)
      Filter.InitExc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::ValidBeg, TCStatJob::ValidEnd
   Filter.ValidBeg = Conf.lookup_unixtime(conf_key_valid_beg);
   Filter.ValidEnd = Conf.lookup_unixtime(conf_key_valid_end);

   // Conf: TCStatJob::ValidInc
   sa = Conf.lookup_string_array(conf_key_valid_inc);
   for(i=0; i<sa.n_elements(); i++)
      Filter.ValidInc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::ValidExc
   sa = Conf.lookup_string_array(conf_key_valid_exc);
   for(i=0; i<sa.n_elements(); i++)
      Filter.ValidExc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::InitHour
   sa = Conf.lookup_string_array(conf_key_init_hour);
   for(i=0; i<sa.n_elements(); i++)
      Filter.InitHour.add(timestring_to_sec(sa[i].c_str()));

   // Conf: TCStatJob::ValidHour
   sa = Conf.lookup_string_array(conf_key_valid_hour);
   for(i=0; i<sa.n_elements(); i++)
      Filter.ValidHour.add(timestring_to_sec(sa[i].c_str()));

   // Conf: TCStatJob::Lead
   sa = Conf.lookup_string_array(conf_key_lead);
   for(i=0; i<sa.n_elements(); i++)
      Filter.Lead.add(timestring_to_sec(sa[i].c_str()));

   // Conf: TCStatJob::LeadReq
   sa = Conf.lookup_string_array(conf_key_lead_req);
   for(i=0; i<sa.n_elements(); i++)
      Filter.LeadReq.add(timestring_to_sec(sa[i].c_str()));

   // Conf: TCStatJob::InitMask
   Filter.InitMask = Conf.lookup_string_array(conf_key_init_mask);

   // Conf: TCStatJob::ValidMask
   Filter.ValidMask = Conf.lookup_string_array(conf_key_valid_mask);

   // Conf: TCStatJob::LineType
   Filter.LineType = Conf.lookup_string_array(conf_key_line_type);

   // Conf: TCStatJob::TrackWatchWarn
   Filter.TrackWatchWarn = Conf.lookup_string_array(conf_key_track_watch_warn);

   // Conf: TCStatJob::ColumnThreshName, TCStatJob::ColumnThreshVal
   sa     = Conf.lookup_string_array(conf_key_column_thresh_name);
   ta_val = Conf.lookup_thresh_array(conf_key_column_thresh_val);

   // Check that they are the same length
   if(sa.n_elements() != ta_val.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"column_thresh_name\" and \"column_thresh_val\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Add entries to the map
   for(i=0; i<sa.n_elements(); i++) {
      if(Filter.ColumnThreshMap.count(sa[i]) > 0) {
         Filter.ColumnThreshMap[sa[i]].add(ta_val[i]);
      }
      else {
         ta_new.clear();
         ta_new.add(ta_val[i]);
         Filter.ColumnThreshMap.insert(pair<ConcatString,ThreshArray>(sa[i], ta_new));
      }
   } // end for i

   // Conf: TCStatJob::ColumnStrName, TCStatJob::ColumnStrVal
   sa     = Conf.lookup_string_array(conf_key_column_str_name);
   sa_val = Conf.lookup_string_array(conf_key_column_str_val);

   // Check that they are the same length
   if(sa.n_elements() != sa_val.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"column_str_name\" and \"column_str_val\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Add entries to the map
   for(i=0; i<sa.n_elements(); i++) {
      if(Filter.ColumnStrMap.count(sa[i]) > 0) {
         Filter.ColumnStrMap[sa[i]].add(sa_val[i]);
      }
      else {
         sa_new.clear();
         sa_new.set_ignore_case(1);
         sa_new.add(sa_val[i]);
         Filter.ColumnStrMap.insert(pair<ConcatString,StringArray>(sa[i], sa_new));
      }
   } // end for i

   // Conf: TCStatJob::InitThreshName, TCStatJob::InitThreshVal
   sa     = Conf.lookup_string_array(conf_key_init_thresh_name);
   ta_val = Conf.lookup_thresh_array(conf_key_init_thresh_val);

   // Check that they are the same length
   if(sa.n_elements() != ta_val.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"init_thresh_name\" and \"init_thresh_val\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Add entries to the map
   for(i=0; i<sa.n_elements(); i++) {
      if(Filter.InitThreshMap.count(sa[i]) > 0) {
         Filter.InitThreshMap[sa[i]].add(ta_val[i]);
      }
      else {
         ta_new.clear();
         ta_new.add(ta_val[i]);
         Filter.InitThreshMap.insert(pair<ConcatString,ThreshArray>(sa[i], ta_new));
      }
   } // end for i

   // Conf: TCStatJob::InitStrName, TCStatJob::InitStrVal
   sa     = Conf.lookup_string_array(conf_key_init_str_name);
   sa_val = Conf.lookup_string_array(conf_key_init_str_val);

   // Check that they are the same length
   if(sa.n_elements() != sa_val.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"init_str_name\" and \"init_str_val\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Add entries to the map
   for(i=0; i<sa.n_elements(); i++) {
      if(Filter.InitStrMap.count(sa[i]) > 0) {
         Filter.InitStrMap[sa[i]].add(sa_val[i]);
      }
      else {
         sa_new.clear();
         sa_new.set_ignore_case(1);
         sa_new.add(sa_val[i]);
         Filter.InitStrMap.insert(pair<ConcatString,StringArray>(sa[i], sa_new));
      }
   } // end for i

   // Conf: TCStatJob::WaterOnly
   Filter.WaterOnly = Conf.lookup_bool(conf_key_water_only);

   // Conf: TCStatJob::RIRWTrack,
   //       TCStatJob::RIRWTimeADeck, TCStatJob::RIRWTimeBDeck
   //       TCStatJob::RIRWExactADeck, TCStatJob::RIRWExactBDeck,
   //       TCStatJob::RIRWThreshADeck, TCStatJob::RIRWThreshBDeck
   Filter.RIRWTrack       = int_to_tracktype(Conf.lookup_int(conf_key_rirw_track));
   Filter.RIRWTimeADeck   = timestring_to_sec(Conf.lookup_string(conf_key_rirw_time_adeck).c_str());
   Filter.RIRWTimeBDeck   = timestring_to_sec(Conf.lookup_string(conf_key_rirw_time_bdeck).c_str());
   Filter.RIRWExactADeck  = Conf.lookup_bool(conf_key_rirw_exact_adeck);
   Filter.RIRWExactBDeck  = Conf.lookup_bool(conf_key_rirw_exact_bdeck);
   Filter.RIRWThreshADeck = Conf.lookup_thresh(conf_key_rirw_thresh_adeck);
   Filter.RIRWThreshBDeck = Conf.lookup_thresh(conf_key_rirw_thresh_bdeck);

   // Conf: TCStatJob::Landfall, TCStatJob::LandfallBeg, TCStatJob::LandfallEnd
   Filter.Landfall    = Conf.lookup_bool(conf_key_landfall);
   Filter.LandfallBeg = Conf.lookup_seconds(conf_key_landfall_beg);
   Filter.LandfallEnd = Conf.lookup_seconds(conf_key_landfall_end);

   // Conf: TCStatJob::MatchPoints
   Filter.MatchPoints = Conf.lookup_bool(conf_key_match_points);

   // Conf: TCStatJob::EventEqual
   Filter.EventEqual = Conf.lookup_bool(conf_key_event_equal);

   // Conf: TCStatJob::OutInitMask
   poly_file = Conf.lookup_string(conf_key_out_init_mask);
   if(poly_file.nonempty()) Filter.set_out_init_mask(poly_file.c_str());

   // Conf: TCStatJob::OutValidMask
   poly_file = Conf.lookup_string(conf_key_out_valid_mask);
   if(poly_file.nonempty()) Filter.set_out_valid_mask(poly_file.c_str());

   // Conf: Jobs
   Jobs = Conf.lookup_string_array(conf_key_jobs);
   if(Jobs.n_elements() == 0) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "must specify at least one entry in \"jobs\".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
