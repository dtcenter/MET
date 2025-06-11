// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "rmw_analysis_conf_info.h"

#include "apply_mask.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
//  Code for class RMWAnalysisConfInfo
//
////////////////////////////////////////////////////////////////////////

RMWAnalysisConfInfo::RMWAnalysisConfInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

RMWAnalysisConfInfo::~RMWAnalysisConfInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::init_from_scratch() {

   // Initialize pointers
   data_info = (VarInfo**) nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::clear() {

   Version.clear();
   Model.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();

   InitBeg = InitEnd = (unixtime) 0;
   InitInc.clear();
   InitExc.clear();
   InitHour.clear();

   Lead.clear();

   ValidBeg = ValidEnd = (unixtime) 0;
   ValidInc.clear();
   ValidExc.clear();
   ValidHour.clear();

   InitMaskName.clear();
   InitPolyMask.clear();
   InitGridMask.clear();
   InitAreaMask.clear();

   ValidMaskName.clear();
   ValidPolyMask.clear();
   ValidGridMask.clear();
   ValidAreaMask.clear();

   Category.clear();
   ColumnThreshMap.clear();
   InitThreshMap.clear();

   // Clear data_info
   if(data_info) {
      for(int i = 0; i < n_data; i++) {
         if(data_info[i]) {
            data_info[i] = (VarInfo*) nullptr;
         }
      }
      delete data_info;
      data_info = (VarInfo**) nullptr;
   }

   // Reset field count
   n_data = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::read_config(const char* default_file_name,
                                   const char* user_file_name) {

   // Read config file constants
   Conf.read(replace_path(config_const_filename).c_str());

   // Read default config file
   Conf.read(default_file_name);

   // Read user-specified config file
   Conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void RMWAnalysisConfInfo::process_config() {
   VarInfoFactory info_factory;
   Dictionary *fdict = (Dictionary *) nullptr;
   ConcatString poly_file;
   GrdFileType ftype = FileType_NcCF;
   StringArray sa;

   // Conf: Version
   Version = Conf.lookup_string(conf_key_version);
   check_met_version(Version.c_str());

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
   InitBeg = Conf.lookup_unixtime(conf_key_init_beg);
   InitEnd = Conf.lookup_unixtime(conf_key_init_end);

   // Conf: InitInc
   sa = Conf.lookup_string_array(conf_key_init_inc);
   for(int i=0; i<sa.n(); i++) {
      InitInc.add(timestring_to_unix(sa[i].c_str()));
   }

   // Conf: InitExc
   sa = Conf.lookup_string_array(conf_key_init_exc);
   for(int i=0; i<sa.n(); i++) {
      InitExc.add(timestring_to_unix(sa[i].c_str()));
   }

   // Conf: InitHour
   sa = Conf.lookup_string_array(conf_key_init_hour);
   for(int i=0; i<sa.n(); i++) {
      InitHour.add(timestring_to_sec(sa[i].c_str()));
   }

   // Conf: Lead
   sa = Conf.lookup_string_array(conf_key_lead);
   for(int i=0; i<sa.n(); i++) {
      Lead.add(timestring_to_sec(sa[i].c_str()));
   }

   // Conf: ValidBeg, ValidEnd
   ValidBeg = Conf.lookup_unixtime(conf_key_valid_beg);
   ValidEnd = Conf.lookup_unixtime(conf_key_valid_end);

   // Conf: ValidInc
   sa = Conf.lookup_string_array(conf_key_valid_inc);
   for(int i=0; i<sa.n(); i++) {
      ValidInc.add(timestring_to_unix(sa[i].c_str()));
   }

   // Conf: ValidExc
   sa = Conf.lookup_string_array(conf_key_valid_exc);
   for(int i=0; i<sa.n(); i++) {
      ValidExc.add(timestring_to_unix(sa[i].c_str()));
   }

   // Conf: ValidHour
   sa = Conf.lookup_string_array(conf_key_valid_hour);
   for(int i=0; i<sa.n(); i++) {
      ValidHour.add(timestring_to_sec(sa[i].c_str()));
   }

// JHG, define a parse_conf_time_array and parse_conf_sec_array utility and call it from here and TC-Stat

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

   // Conf: Category
   Category = Conf.lookup_string_array(conf_key_category);

   // Conf: ColumnThreshName, ColumnThreshVal
   ColumnThreshMap = parse_conf_thresh_map(&Conf,
                        conf_key_column_thresh_name,
                        conf_key_column_thresh_val);

   // Conf: InitThreshName, InitThreshVal
   InitThreshMap = parse_conf_thresh_map(&Conf,
                      conf_key_init_thresh_name,
                      conf_key_init_thresh_val);

   // Conf: data.field
   fdict = Conf.lookup_array(conf_key_data_field);

   // Determine number of fields (name/level)
   n_data = parse_conf_n_vx(fdict);

   // Check for empty data settings
   if(n_data == 0) {
      mlog << Error << "\nRMWAnalysisConfInfo::process_config() -> "
          << "data may not be empty.\n\n";
      exit(1);
   }

   // Allocate space based on number of fields
   data_info = new VarInfo*[n_data];

   // Initialize pointers
   for(int i = 0; i < n_data; i++) {
      data_info[i] = (VarInfo*) nullptr;
   }

   // Parse data field information
   ConcatString field_cs;
   for(int i = 0; i < n_data; i++) {

      // Allocate new VarInfo objects
      data_info[i] = info_factory.new_var_info(ftype);

      // Get current dictionary
      Dictionary i_fdict = parse_conf_i_vx_dict(fdict, i);

      // Set current dictionary
      data_info[i]->set_dict(i_fdict);

      if(i > 0) field_cs << ", ";
      field_cs << data_info[i]->magic_str();
   }

   mlog << Debug(2) << "Requested " << n_data << " fields: "
        << field_cs << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////
