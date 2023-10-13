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

#include "tc_diag_conf_info.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for struct DataOptInfo
//
////////////////////////////////////////////////////////////////////////

void DataOptInfo::clear() {

   tech_ids.clear();
   data_files.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

DataOptInfo & DataOptInfo::operator+=(const DataOptInfo &info) {

   tech_ids.add(info.tech_ids);
   data_files.add(info.data_files);

   return(*this);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class DomainInfo
//
////////////////////////////////////////////////////////////////////////

DomainInfo::DomainInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

DomainInfo::~DomainInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void DomainInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void DomainInfo::clear() {

   tech_ids.clear();
   data_files.clear();
   domain.clear();

   data.name         = (const char *) 0;
   data.range_n      = bad_data_int;
   data.azimuth_n    = bad_data_int;
   data.range_max_km = bad_data_double;
   data.lat_center   = bad_data_double;
   data.lon_center   = bad_data_double;

   delta_range_km = bad_data_double;

   var_info_ptr.clear();
   diag_script.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void DomainInfo::parse_domain_info(Dictionary &dict) {

   // Initialize
   clear();

   // Note: tech_ids and data_files are specified on the
   //       command line rather than in the config file

   // Conf: domain
   domain = dict.lookup_string(conf_key_domain);

   // Hard-code the name
   data.name = "TCDIAG";

   // Conf: n_range
   data.range_n = dict.lookup_int(conf_key_n_range);

   // Conf: azimuth_n
   data.azimuth_n = dict.lookup_int(conf_key_n_azimuth);

   // Conf: delta_range
   delta_range_km = dict.lookup_double(conf_key_delta_range);

   // Conf: diag_script
   diag_script = dict.lookup_string_array(conf_key_diag_script);

   // Expand MET_BASE
   for(int i=0; i<diag_script.n(); i++) {
      diag_script.set(i, replace_path(diag_script[i].c_str()));
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCDiagConfInfo
//
////////////////////////////////////////////////////////////////////////

TCDiagConfInfo::TCDiagConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCDiagConfInfo::~TCDiagConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::clear() {

   model.clear();
   storm_id.clear();
   basin.clear();
   cyclone.clear();

   init_inc = (unixtime) 0;
   valid_beg = valid_end = (unixtime) 0;
   valid_inc.clear();
   valid_exc.clear();
   valid_hour.clear();
   lead_time.clear();

   // Deallocate memory
   for(int i=0; i<var_info.size(); i++) {
      if(var_info[i]) { delete var_info[i]; var_info[i] = (VarInfo *) 0; }
   }
   var_info.clear();

   pressure_levels.clear();

   domain_info.clear();

   vortex_removal_flag = false;
   one_time_per_file_flag = true;

   nc_rng_azi_flag = false;
   nc_diag_flag    = false;
   cira_diag_flag  = false;

   tmp_dir.clear();
   output_prefix.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::read_config(const char *default_file_name,
                                 const char *user_file_name) {

   // Read config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read default config file
   conf.read(default_file_name);

   // Read user-specified config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::process_config(GrdFileType file_type,
                                    map<string,DataOptInfo> dmap) {
   int i, j;
   StringArray sa;
   Dictionary *dict = (Dictionary *) 0;
   VarInfoFactory vi_factory;

   // Conf: version
   check_met_version(conf.lookup_string(conf_key_version).c_str());

   // Conf: model
   model = conf.lookup_string_array(conf_key_model);

   // Conf: storm_id
   storm_id = conf.lookup_string(conf_key_storm_id);

   // Conf: basin
   basin = conf.lookup_string(conf_key_basin);

   // Conf: cyclone
   cyclone = conf.lookup_string(conf_key_cyclone);

   // Conf: init_inc
   init_inc = conf.lookup_unixtime(conf_key_init_inc);

   // Conf: valid_beg, valid_end
   valid_beg = conf.lookup_unixtime(conf_key_valid_beg);
   valid_end = conf.lookup_unixtime(conf_key_valid_end);

   // Conf: valid_inc
   sa = conf.lookup_string_array(conf_key_valid_inc);
   for(i=0; i<sa.n(); i++) {
      valid_inc.add(timestring_to_unix(sa[i].c_str()));
   }

   // Conf: valid_exc
   sa = conf.lookup_string_array(conf_key_valid_exc);
   for(i=0; i<sa.n(); i++) {
      valid_exc.add(timestring_to_unix(sa[i].c_str()));
   }

   // Conf: valid_hour
   sa = conf.lookup_string_array(conf_key_valid_hour);
   for(i=0; i<sa.n(); i++) {
      valid_hour.add(timestring_to_sec(sa[i].c_str()));
   }

   // Conf: lead
   sa = conf.lookup_string_array(conf_key_lead);
   for(i=0; i<sa.n(); i++) {
      lead_time.add(timestring_to_sec(sa[i].c_str()));
   }

   // Conf: domain_info
   parse_domain_info(dmap);

   // Conf: data.field
   dict = conf.lookup_array(conf_key_data_field);

   // Determine number of fields (name/level)
   int n_data = parse_conf_n_vx(dict);

   mlog << Debug(2) << "Found " << n_data << " variable/level fields "
        << "requested in the configuration file.\n";

   // Check for empty data settings
   if(n_data == 0) {
       mlog << Error << "\nTCDiagConfInfo::process_config() -> "
            << "the \"" << conf_key_data_field
            << "\" config file entry cannot be empty!\n\n";
       exit(1);
   }

   // Process each field
   for(i=0; i<n_data; i++) {

      // Get current dictionary
      Dictionary i_dict = parse_conf_i_vx_dict(dict, i);

      // Conf: field.name and field.level
      VarInfo *vi = vi_factory.new_var_info(file_type);
      vi->set_dict(i_dict);
      var_info.push_back(vi);

      // Unique list of requested pressure levels
      if(vi->level().type() == LevelType_Pres) {
         if(vi->level().lower() != vi->level().upper()) {
            mlog << Error << "\nTCDiagConfInfo::process_config() -> "
                 << "only individual pressure levels are supported, "
                 << "not ranges (" << vi->level().req_name()
                 << ").\n\n";
            exit(1);
         }
         pressure_levels.insert(vi->level().lower());
      }

      // Conf: field.domain
      sa = i_dict.lookup_string_array(conf_key_domain);

      // Store domain-specific VarInfo pointers
      for(j=0; j<domain_info.size(); j++) {
         if(sa.n() == 0 || sa.has(domain_info[j].domain)) {
            domain_info[j].var_info_ptr.push_back(var_info.back());
         }
      }
   }

   // Conf: vortex_removal
   vortex_removal_flag =
      conf.lookup_bool(conf_key_vortex_removal);

   // Conf: one_time_per_file_flag
   one_time_per_file_flag =
      conf.lookup_bool(conf_key_one_time_per_file_flag);

   // Conf: nc_rng_azi_flag
   nc_rng_azi_flag = conf.lookup_bool(conf_key_nc_rng_azi_flag);

   // Conf: nc_diag_flag
   nc_diag_flag = conf.lookup_bool(conf_key_nc_diag_flag);

   // Conf: cira_diag_flag
   cira_diag_flag = conf.lookup_bool(conf_key_cira_diag_flag);

   // At least one output type must be requested
   if(!nc_rng_azi_flag && !nc_diag_flag && !cira_diag_flag) {
      mlog << Error << "\nTCDiagConfInfo::process_config() -> "
           << "the \"" << conf_key_nc_rng_azi_flag
           << "\", \"" << conf_key_nc_diag_flag
           << "\", and \"" << conf_key_cira_diag_flag
           << "\" config entries cannot all be false.\n\n";
      exit(1);
   }

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::parse_domain_info(map<string,DataOptInfo> dmap) {
   Dictionary *dict = (Dictionary *) 0;
   int i, j;
   bool found;

   // Conf: domain_info
   dict = conf.lookup_array(conf_key_domain_info);

   if(!dict) {
      mlog << Error << "\nTCDiagConfInfo::parse_domain_info() -> "
           << "array lookup failed for key \"" << conf_key_domain_info
           << "\"\n\n";
      exit(1);
   }

   // Parse each grid info object
   for(i=0; i<dict->n_entries(); i++) {
      DomainInfo di;

      // Parse the current domain info
      di.parse_domain_info(*((*dict)[i]->dict_value()));

      // Store the domain-specifc data files
      if(dmap.count(di.domain) > 0) {
         di.tech_ids   = dmap[di.domain].tech_ids;
         di.data_files = dmap[di.domain].data_files;
      }
      else {
         mlog << Error << "\nTCDiagConfInfo::parse_domain_info() -> "
              << "no \"-data " << di.domain << "\" command line option provided for the \""
              << conf_key_domain_info << "." << conf_key_domain << "\" = \"" << di.domain
              << "\" config file entry!\n\n";
         exit(1);
      }

      // Check for duplicate entries
      for(j=0; i<domain_info.size(); j++) {
         if(di.domain == domain_info[j].domain) {
            mlog << Error << "\nTCDiagConfInfo::parse_domain_info() -> "
                 << "multiple \"" << conf_key_domain_info
                 << "\" entries found for domain \"" << di.domain << "\"!\n\n";
            exit(1);
         }
      }

      // Store new entry
      domain_info.push_back(di);
   }

   // Make sure all -data domains appear in the config file
   map<string,DataOptInfo>::iterator it;
   for(it = dmap.begin(); it != dmap.end(); it++) {

      for(i=0, found=false; i<domain_info.size(); i++) {
         if(it->first == domain_info[i].domain) {
            found = true;
            break;
         }
      } // end for i

      if(!found) {
         mlog << Error << "\nTCDiagConfInfo::parse_domain_info() -> "
              << "no \"" << conf_key_domain_info << "." << conf_key_domain << "\" = \""
              << it->first << "\" config file entry provided for the \"-data "
              << it->first << "\" command line option!\n\n";
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////
