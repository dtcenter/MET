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
   Version = Conf.lookup_string("version");
   check_met_version(Version);

   // Conf: TCStatJob::AModel
   Filter.AModel = Conf.lookup_string_array("amodel");

   // Conf: TCStatJob::BModel
   Filter.BModel = Conf.lookup_string_array("bmodel");

   // Conf: TCStatJob::StormId
   Filter.StormId = Conf.lookup_string_array("storm_id");
   
   // Conf: TCStatJob::Basin
   Filter.Basin = Conf.lookup_string_array("basin");

   // Conf: TCStatJob::Cyclone
   Filter.Cyclone = Conf.lookup_string_array("cyclone");

   // Conf: TCStatJob::StormName
   Filter.StormName = Conf.lookup_string_array("storm_name");

   // Conf: TCStatJob::InitBeg, TCStatJob::InitEnd
   Filter.InitBeg = timestring_to_unix(Conf.lookup_string("init_beg"));
   Filter.InitEnd = timestring_to_unix(Conf.lookup_string("init_end"));

   // Conf: TCStatJob::InitExc
   sa = Conf.lookup_string_array("init_exc");
   for(i=0; i<sa.n_elements(); i++)
      Filter.InitExc.add(timestring_to_unix(sa[i]));

   // Conf: TCStatJob::ValidBeg, TCStatJob::ValidEnd
   Filter.ValidBeg = timestring_to_unix(Conf.lookup_string("valid_beg"));
   Filter.ValidEnd = timestring_to_unix(Conf.lookup_string("valid_end"));

   // Conf: TCStatJob::ValidExc
   sa = Conf.lookup_string_array("valid_exc");
   for(i=0; i<sa.n_elements(); i++)
      Filter.ValidExc.add(timestring_to_unix(sa[i]));

   // Conf: TCStatJob::InitHour
   sa = Conf.lookup_string_array("init_hour");
   for(i=0; i<sa.n_elements(); i++)
      Filter.InitHour.add(timestring_to_sec(sa[i]));

   // Conf: TCStatJob::ValidHour
   sa = Conf.lookup_string_array("valid_hour");
   for(i=0; i<sa.n_elements(); i++)
      Filter.ValidHour.add(timestring_to_sec(sa[i]));
   
   // Conf: TCStatJob::Lead
   sa = Conf.lookup_string_array("lead");
   for(i=0; i<sa.n_elements(); i++)
      Filter.Lead.add(timestring_to_sec(sa[i]));

   // Conf: TCStatJob::InitMask
   Filter.InitMask = Conf.lookup_string_array("init_mask");

   // Conf: TCStatJob::ValidMask
   Filter.ValidMask = Conf.lookup_string_array("valid_mask");

   // Conf: TCStatJob::LineType
   Filter.LineType = Conf.lookup_string_array("line_type");
   
   // Conf: TCStatJob::WaterOnly
   Filter.WaterOnly = Conf.lookup_bool("water_only");

   // Conf: TCStatJob::TrackWatchWarn
   Filter.TrackWatchWarn = Conf.lookup_string_array("track_watch_warn");
   
   // Conf: TCStatJob::ColumnThreshName, TCStatJob::ColumnThreshVal
   Filter.ColumnThreshName = Conf.lookup_string_array("column_thresh_name");
   Filter.ColumnThreshVal  = Conf.lookup_thresh_array("column_thresh_val");

   // Check that they are the same length
   if(Filter.ColumnThreshName.n_elements() !=
      Filter.ColumnThreshName.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"column_thresh_name\" and \"column_thresh_val\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Conf: TCStatJob::ColumnStrName, TCStatJob::ColumnStrVal
   Filter.ColumnStrName = Conf.lookup_string_array("column_str_name");
   Filter.ColumnStrVal  = Conf.lookup_string_array("column_str_val");

   // Check that they are the same length
   if(Filter.ColumnStrName.n_elements() !=
      Filter.ColumnStrVal.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"column_str_name\" and \"column_str_val\" entries "
           << "must have the same length.\n\n";
      exit(1);
   }

   // Conf: TCStatJob::InitThreshName, TCStatJob::InitThreshVal
   Filter.InitThreshName = Conf.lookup_string_array("init_thresh_name");
   Filter.InitThreshVal  = Conf.lookup_thresh_array("init_thresh_val");

   // Check that they are the same length
   if(Filter.InitThreshName.n_elements() !=
      Filter.InitThreshName.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"init_thresh_name\" and \"init_thresh_val\" "
           << "entries must have the same length.\n\n";
      exit(1);
   }

   // Conf: TCStatJob::InitStrName, TCStatJob::InitStrVal
   Filter.InitStrName = Conf.lookup_string_array("init_str_name");
   Filter.InitStrVal  = Conf.lookup_string_array("init_str_val");

   // Check that they are the same length
   if(Filter.InitStrName.n_elements() !=
      Filter.InitStrVal.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"init_str_name\" and \"init_str_val\" entries "
           << "must have the same length.\n\n";
      exit(1);
   }

   // Conf: TCStatJob::RapidInten, TCStatJob::RapidIntenThresh
   Filter.RapidInten       = Conf.lookup_bool("rapid_inten");
   Filter.RapidIntenThresh = Conf.lookup_thresh("rapid_inten_thresh");

   // Conf: TCStatJob::Landfall, TCStatJob::LandfallBeg, TCStatJob::LandfallEnd
   Filter.Landfall    = Conf.lookup_bool("landfall");
   Filter.LandfallBeg = Conf.lookup_int("landfall_beg");
   Filter.LandfallEnd = Conf.lookup_int("landfall_end");

   // Conf: TCStatJob::MatchPoints
   Filter.MatchPoints = Conf.lookup_bool("match_points");

   // Conf: TCStatJob::EventEqual
   Filter.EventEqual = Conf.lookup_bool("event_equal");
   
   // Conf: TCStatJob::OutInitMask
   poly_file = Conf.lookup_string("out_init_mask");
   if(poly_file.nonempty()) Filter.set_mask(Filter.OutInitMask, poly_file);
   
   // Conf: TCStatJob::OutValidMask
   poly_file = Conf.lookup_string("out_valid_mask");
   if(poly_file.nonempty()) Filter.set_mask(Filter.OutValidMask, poly_file);
   
   // Conf: Jobs
   Jobs = Conf.lookup_string_array("jobs");
   
   return;
}

////////////////////////////////////////////////////////////////////////
