// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

// Functions for parsing config entries
static void parse_conf_thresh_map(MetConfig &,
               const char *, const char *,
               map<ConcatString,ThreshArray> &);
static void parse_conf_string_map(MetConfig &,
               const char *, const char *,
               map<ConcatString,StringArray> &);

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
   StringArray sa;
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
   for(i=0; i<sa.n(); i++)
      Filter.InitInc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::InitExc
   sa = Conf.lookup_string_array(conf_key_init_exc);
   for(i=0; i<sa.n(); i++)
      Filter.InitExc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::ValidBeg, TCStatJob::ValidEnd
   Filter.ValidBeg = Conf.lookup_unixtime(conf_key_valid_beg);
   Filter.ValidEnd = Conf.lookup_unixtime(conf_key_valid_end);

   // Conf: TCStatJob::ValidInc
   sa = Conf.lookup_string_array(conf_key_valid_inc);
   for(i=0; i<sa.n(); i++)
      Filter.ValidInc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::ValidExc
   sa = Conf.lookup_string_array(conf_key_valid_exc);
   for(i=0; i<sa.n(); i++)
      Filter.ValidExc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: TCStatJob::InitHour
   sa = Conf.lookup_string_array(conf_key_init_hour);
   for(i=0; i<sa.n(); i++)
      Filter.InitHour.add(timestring_to_sec(sa[i].c_str()));

   // Conf: TCStatJob::ValidHour
   sa = Conf.lookup_string_array(conf_key_valid_hour);
   for(i=0; i<sa.n(); i++)
      Filter.ValidHour.add(timestring_to_sec(sa[i].c_str()));

   // Conf: TCStatJob::Lead
   sa = Conf.lookup_string_array(conf_key_lead);
   for(i=0; i<sa.n(); i++)
      Filter.Lead.add(timestring_to_sec(sa[i].c_str()));

   // Conf: TCStatJob::LeadReq
   sa = Conf.lookup_string_array(conf_key_lead_req);
   for(i=0; i<sa.n(); i++)
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
   parse_conf_thresh_map(Conf,
                         conf_key_column_thresh_name, conf_key_column_thresh_val,
                         Filter.ColumnThreshMap);

   // Conf: TCStatJob::ColumnStrIncName, TCStatJob::ColumnStrIncVal
   parse_conf_string_map(Conf,
                         conf_key_column_str_name, conf_key_column_str_val,
                         Filter.ColumnStrIncMap);

   // Conf: TCStatJob::ColumnStrExcName, TCStatJob::ColumnStrExcVal
   parse_conf_string_map(Conf,
                         conf_key_column_str_exc_name, conf_key_column_str_exc_val,
                         Filter.ColumnStrExcMap);

   // Conf: TCStatJob::InitThreshName, TCStatJob::InitThreshVal
   parse_conf_thresh_map(Conf,
                         conf_key_init_thresh_name, conf_key_init_thresh_val,
                         Filter.InitThreshMap);

   // Conf: TCStatJob::InitStrIncName, TCStatJob::InitStrIncVal
   parse_conf_string_map(Conf,
                         conf_key_init_str_name, conf_key_init_str_val,
                         Filter.InitStrIncMap);

   // Conf: TCStatJob::InitStrExcName, TCStatJob::InitStrExcVal
   parse_conf_string_map(Conf,
                         conf_key_init_str_exc_name, conf_key_init_str_exc_val,
                         Filter.InitStrExcMap);

   // Conf: TCStatJob::DiagThreshName, TCStatJob::DiagThreshVal
   parse_conf_thresh_map(Conf,
                         conf_key_diag_thresh_name, conf_key_diag_thresh_val,
                         Filter.DiagThreshMap);

   // Conf: TCStatJob::InitDiagThreshName, TCStatJob::InitDiagThreshVal
   parse_conf_thresh_map(Conf,
                         conf_key_init_diag_thresh_name, conf_key_init_diag_thresh_val,
                         Filter.InitDiagThreshMap);

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
   if(Jobs.n() == 0) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "must specify at least one entry in \"jobs\".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_conf_thresh_map(MetConfig &conf,
                           const char *conf_key_name, const char *conf_key_val,
                           map<ConcatString,ThreshArray> &m) {
   StringArray sa;
   ThreshArray ta_val, ta_new;
   
   sa     = conf.lookup_string_array(conf_key_name);
   ta_val = conf.lookup_thresh_array(conf_key_val);

   // Check that they are the same length
   if(sa.n() != ta_val.n()) {
      mlog << Error
           << "\nTCStatConfInfo::parse_conf_thresh_map() -> "
           << "the \"" << conf_key_name << "\" and \"" << conf_key_val << "\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Add entries to the map
   for(int i=0; i<sa.n(); i++) {
      if(m.count(sa[i]) > 0) {
         m[sa[i]].add(ta_val[i]);
      }
      else {
         ta_new.clear();
         ta_new.add(ta_val[i]);
         m.insert(pair<ConcatString,ThreshArray>(sa[i], ta_new));
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_conf_string_map(MetConfig &conf,
                           const char *conf_key_name, const char *conf_key_val,
                           map<ConcatString,StringArray> &m) {
   StringArray sa, sa_val, sa_new;
   
   sa     = conf.lookup_string_array(conf_key_name);
   sa_val = conf.lookup_string_array(conf_key_val);

   // Check that they are the same length
   if(sa.n() != sa_val.n()) {
      mlog << Error
           << "\nTCStatConfInfo::parse_conf_string_map() -> "
           << "the \"" << conf_key_name << "\" and \"" << conf_key_val << "\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Add entries to the map
   for(int i=0; i<sa.n(); i++) {
      if(m.count(sa[i]) > 0) {
         m[sa[i]].add(sa_val[i]);
      }
      else {
         sa_new.clear();
         sa_new.set_ignore_case(1);
         sa_new.add(sa_val[i]);
         m.insert(pair<ConcatString,StringArray>(sa[i], sa_new));
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
