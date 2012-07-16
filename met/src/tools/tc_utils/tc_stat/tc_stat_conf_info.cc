// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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

   // Conf: TCStatJob::Basin
   Filter.Basin = Conf.lookup_string_array("basin");

   // Conf: TCStatJob::Cyclone
   Filter.Cyclone = Conf.lookup_string_array("cyclone");

   // Conf: TCStatJob::InitBeg, TCStatJob::InitEnd
   Filter.InitBeg = timestring_to_unix(Conf.lookup_string("init_beg"));
   Filter.InitEnd = timestring_to_unix(Conf.lookup_string("init_end"));

   // Conf: TCStatJob::ValidBeg, TCStatJob::ValidEnd
   Filter.ValidBeg = timestring_to_unix(Conf.lookup_string("valid_beg"));
   Filter.ValidEnd = timestring_to_unix(Conf.lookup_string("valid_end"));

   // Conf: TCStatJob::InitHH
   sa = Conf.lookup_string_array("init_hh");
   for(i=0; i<sa.n_elements(); i++)
      Filter.InitHH.add(timestring_to_sec(sa[i]));

   // Conf: TCStatJob::Lead
   sa = Conf.lookup_string_array("lead");
   for(i=0; i<sa.n_elements(); i++)
      Filter.Lead.add(timestring_to_sec(sa[i]));

   // Conf: TCStatJob::InitMask
   Filter.InitMask = Conf.lookup_string_array("init_mask");

   // Conf: TCStatJob::ValidMask
   Filter.ValidMask = Conf.lookup_string_array("valid_mask");

   // Conf: TCStatJob::ColNumName, TCStatJob::ColNumThresh
   Filter.ColNumName   = Conf.lookup_string_array("col_num_name");
   Filter.ColNumThresh = Conf.lookup_thresh_array("col_num_thresh");

   // Check that they are the same length
   if(Filter.ColNumName.n_elements() != Filter.ColNumName.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"col_num_name\" and \"col_num_thresh\" entries "
           << "must have the same length.\n\n";
      exit(1);
   }

   // Conf: TCStatJob::ColStrName, TCStatJob::ColStrValue
   Filter.ColStrName  = Conf.lookup_string_array("col_str_name");
   Filter.ColStrValue = Conf.lookup_string_array("col_str_value");

   // Check that they are the same length
   if(Filter.ColStrName.n_elements() != Filter.ColStrValue.n_elements()) {
      mlog << Error
           << "\nTCStatConfInfo::process_config() -> "
           << "the \"col_str_name\" and \"col_str_value\" entries "
           << "must have the same length.\n\n";
      exit(1);
   }
   
   // Conf: TCStatJob::MatchPoints
   Filter.MatchPoints = Conf.lookup_bool("match_points");
   
   // Conf: Jobs
   Jobs = Conf.lookup_string_array("jobs");
   
   return;
}

////////////////////////////////////////////////////////////////////////
