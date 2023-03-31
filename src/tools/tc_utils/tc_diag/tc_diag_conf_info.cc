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

#include "tc_diag_conf_info.h"

#include "vx_log.h"

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

   data_opt.clear();
   domain_info_map.clear();

   compute_tangential_and_radial_winds = false;
   u_wind_field_name.clear();
   v_wind_field_name.clear();
   tangential_velocity_field_name.clear();
   radial_velocity_field_name.clear();
   tangential_velocity_long_field_name.clear();
   radial_velocity_long_field_name.clear();

   nc_diag_info.clear();
   cira_diag_flag = false;

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

void TCDiagConfInfo::process_config(GrdFileType file_type) {
   int i;
   StringArray sa;
   VarInfoFactory info_factory;
   Dictionary *dict = (Dictionary *) 0;

// JHG note that "regrid" appears in the TC-Diag and TC-RMW config files
// I think it currently only applies to the raw data, meaning we can regrid
// the raw data PRIOR TO the cyl coordinate transformation. Wondering if we
// need options to support the cyl coordinate transformation?

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

   // Conf: data.field
   dict = conf.lookup_array(conf_key_data_field);

   // Determine number of fields (name/level)
   int n_data = parse_conf_n_vx(dict);

   mlog << Debug(2) << "Found " << n_data << " variable/level fields "
        << "requested in the configuration file.\n";

   // Check for empty data settings
   if(n_data == 0) {
       mlog << Error << "\nTCDiagConfInfo::process_config() -> "
            << "data may not be empty.\n\n";
       exit(1);
   }

   // Process each field
   for(i=0; i<n_data; i++) {

      // Get current dictionary
      Dictionary i_dict = parse_conf_i_vx_dict(dict, i);

      // Parse and store config options
      TCDiagDataOpt vx_opt;
      vx_opt.process_config(file_type, i_dict);

      // Store the options
      data_opt.push_back(vx_opt);
   }

   // Parse the cyclindrical coordinate grids
   parse_domain_info_map();

   // Conf: compute_tangential_and_radial_winds
   compute_tangential_and_radial_winds =
      conf.lookup_bool(conf_key_compute_tangential_and_radial_winds);

// JHG, replace this with is_u_wind/is_v_wind?

   // Conf: u_wind_field_name
   u_wind_field_name =
      conf.lookup_string(conf_key_u_wind_field_name);

   // Conf: v_wind_field_name
   v_wind_field_name =
      conf.lookup_string(conf_key_v_wind_field_name);

   // Conf: tangential_velocity_field_name
   tangential_velocity_field_name =
      conf.lookup_string(conf_key_tangential_velocity_field_name);

   // Conf: radial_velocity_field_name
   radial_velocity_field_name =
      conf.lookup_string(conf_key_radial_velocity_field_name);

   // Conf: tangential_velocity_long_field_name
   tangential_velocity_long_field_name =
      conf.lookup_string(conf_key_tangential_velocity_long_field_name);

   // Conf: radial_velocity_long_field_name
   radial_velocity_long_field_name =
      conf.lookup_string(conf_key_radial_velocity_long_field_name);

   // Conf: nc_diag_flag
   parse_nc_diag_info();

   // Set the write_nc flag, if needed
   if(nc_diag_info.do_cyl_latlon || nc_diag_info.do_cyl_raw) {
      map<string,TCDiagDomainInfo>::iterator it;
      for(it = domain_info_map.begin(); it != domain_info_map.end(); it++) {
         it->second.write_nc = true;
      }
   }

   // Conf: cira_diag_flag
   cira_diag_flag = conf.lookup_bool(conf_key_cira_diag_flag);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::parse_domain_info_map() {

   const DictionaryEntry * e = (const DictionaryEntry *) 0;

   e = conf.lookup(conf_key_domain_info);

   if(!e) {
      mlog << Error << "\nTCDiagConfInfo::parse_domain_info_map() -> "
           << "lookup failed for key \"" << conf_key_domain_info
           << "\"\n\n";
      exit(1);
   }

   const ConfigObjectType type = e->type();

   // It should be an array of dictionaries
   if(type != ArrayType) {
      mlog << Error << "\nTCDiagConfInfo::parse_domain_info_map() -> "
           << "bad type (" << configobjecttype_to_string(type)
           << ") for key \"" << conf_key_domain_info << "\"\n\n";
      exit(1);
   }

   // Parse each grid info object
   for(int i=0; i<e->array_value()->n_entries(); i++) {
      ConcatString domain;
      TCDiagDomainInfo di;

      parse_domain_info(e->array_value()[i],
                        domain, di);

      // Check for duplicate entries
      if(domain_info_map.count(domain) > 0) {
         mlog << Error << "\nTCDiagConfInfo::parse_domain_info_map() -> "
              << "multiple \"" << conf_key_domain_info
              << "\" entries found for domain \"" << domain << "\"!\n\n";
         exit(1);
      }
      // Store a new entry
      else {
         domain_info_map[domain] = di;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::parse_domain_info(Dictionary &dict,
                                       ConcatString &domain,
                                       TCDiagDomainInfo &di) {

   // Conf: domain
   domain = dict.lookup_string(conf_key_domain);

   // Initialize
   di.clear();

   // Hard-code the name for now
   di.data.name = "TCRMW";

   // Conf: n_range
   di.data.range_n = dict.lookup_int(conf_key_n_range);

   // Conf: azimuth_n
   di.data.azimuth_n = dict.lookup_int(conf_key_n_azimuth);

   // Conf: max_range
   di.data.range_max_km = dict.lookup_double(conf_key_max_range);

   // Conf: delta_range
   di.delta_range_km = dict.lookup_double(conf_key_delta_range);

   // Conf: diag_script
   di.diag_script = dict.lookup_string_array(conf_key_diag_script);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagConfInfo::parse_nc_diag_info() {
   const DictionaryEntry * e = (const DictionaryEntry *) 0;

   e = conf.lookup(conf_key_nc_diag_flag);

   if(!e) {
      mlog << Error << "\nTCDiagConfInfo::parse_nc_diag_info() -> "
           << "lookup failed for key \"" << conf_key_nc_diag_flag
           << "\"\n\n";
      exit(1);
   }

   const ConfigObjectType type = e->type();

   if(type == BooleanType) {
      bool value = e->b_value();

      if(!value) nc_diag_info.set_all_false();

      return;
   }

   // It should be a dictionary
   if(type != DictionaryType) {
      mlog << Error << "\nTCDiagConfInfo::parse_nc_diag_info() -> "
           << "bad type (" << configobjecttype_to_string(type)
           << ") for key \"" << conf_key_nc_diag_flag << "\"\n\n";
      exit(1);
   }

   // Parse the various entries
   Dictionary * d = e->dict_value();

   nc_diag_info.do_track       = d->lookup_bool(conf_key_track_flag);
   nc_diag_info.do_diag        = d->lookup_bool(conf_key_diag_flag);
   nc_diag_info.do_grid_latlon = d->lookup_bool(conf_key_grid_latlon_flag);
   nc_diag_info.do_grid_raw    = d->lookup_bool(conf_key_grid_raw_flag);
   nc_diag_info.do_cyl_latlon  = d->lookup_bool(conf_key_cyl_latlon_flag);
   nc_diag_info.do_cyl_raw     = d->lookup_bool(conf_key_cyl_raw_flag);

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCDiagDataOpt
//
////////////////////////////////////////////////////////////////////////

TCDiagDataOpt::TCDiagDataOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCDiagDataOpt::~TCDiagDataOpt() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCDiagDataOpt::init_from_scratch() {

   // Initialize pointer
   var_info = (VarInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagDataOpt::clear() {
   int i;

   domain_sa.clear();

   // Deallocate memory
   if(var_info) { delete var_info; var_info = (VarInfo *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagDataOpt::process_config(GrdFileType file_type, Dictionary &dict) {
   int i, n;
   VarInfoFactory info_factory;

   // Initialize
   clear();

   // Conf: field.domain
   domain_sa = dict.lookup_string_array(conf_key_domain);

   // Conf: field.name and field.level
   var_info = info_factory.new_var_info(file_type);
   var_info->set_dict(dict);

   // Dump the contents of the current VarInfo
   mlog << Debug(3) << "Parsed field \"" << var_info->magic_str()
        << "\" for domains \"" << write_css(domain_sa) << "\".\n";
   if(mlog.verbosity_level() >= 5) var_info->dump(cout);

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for struct TCDiagNcOutInfo
//
////////////////////////////////////////////////////////////////////////

TCDiagNcOutInfo::TCDiagNcOutInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

TCDiagNcOutInfo & TCDiagNcOutInfo::operator+=(const TCDiagNcOutInfo &t) {

   if(t.do_track)       do_track       = true;
   if(t.do_diag)        do_diag        = true;
   if(t.do_grid_latlon) do_grid_latlon = true;
   if(t.do_grid_raw)    do_grid_raw    = true;
   if(t.do_cyl_latlon)  do_cyl_latlon  = true;
   if(t.do_cyl_raw)     do_cyl_raw     = true;

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCDiagNcOutInfo::clear() {

   set_all_false();

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCDiagNcOutInfo::all_false() const {

   bool status = do_track       || do_diag     ||
                 do_grid_latlon || do_grid_raw ||
                 do_cyl_latlon  || do_cyl_raw;

   return(!status);
}

////////////////////////////////////////////////////////////////////////

void TCDiagNcOutInfo::set_all_false() {

   do_track       = false;
   do_diag        = false;
   do_grid_latlon = false;
   do_grid_raw    = false;
   do_cyl_latlon  = false;
   do_cyl_raw     = false;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagNcOutInfo::set_all_true() {

   do_track       = true;
   do_diag        = true;
   do_grid_latlon = true;
   do_grid_raw    = true;
   do_cyl_latlon  = true;
   do_cyl_raw     = true;

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for struct TCDiagDomainInfo
//
////////////////////////////////////////////////////////////////////////

TCDiagDomainInfo::TCDiagDomainInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

bool TCDiagDomainInfo::operator==(const TCDiagDomainInfo &t) {

   return(data.range_n ==          t.data.range_n &&
          data.azimuth_n ==        t.data.azimuth_n &&
          is_eq(data.range_max_km, t.data.range_max_km) &&
          is_eq(delta_range_km,    t.delta_range_km));
}

////////////////////////////////////////////////////////////////////////

void TCDiagDomainInfo::clear() {

   data.range_n      = bad_data_int;
   data.azimuth_n    = bad_data_int;
   data.range_max_km = bad_data_double;
   data.lat_center   = bad_data_double;
   data.lon_center   = bad_data_double;

   delta_range_km = bad_data_double;

   diag_script.clear();

   write_nc = false;

   return;
}

////////////////////////////////////////////////////////////////////////
