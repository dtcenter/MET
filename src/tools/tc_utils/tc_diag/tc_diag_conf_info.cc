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

   // Initialize pointer
   data_opt = (TCDiagDataOpt *) 0;

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

   compute_tangential_and_radial_winds = false;
   u_wind_field_name.clear();
   v_wind_field_name.clear();
   tangential_velocity_field_name.clear();
   radial_velocity_field_name.clear();
   tangential_velocity_long_field_name.clear();
   radial_velocity_long_field_name.clear();

   tmp_dir.clear();
   output_prefix.clear();

   nc_info.clear();

   // Deallocate memory
   for(int i=0; i<n_data; i++) data_opt[i].clear();
   if(data_opt) { delete [] data_opt; data_opt = (TCDiagDataOpt *) 0; }

   n_data = 0;

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

   // Conf: version
   check_met_version(conf.lookup_string(conf_key_version).c_str());

   // Conf: model
   model = conf.lookup_string(conf_key_model);

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

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   // Conf: data.field
   dict = conf.lookup_array(conf_key_data_field);

   // Determine number of fields (name/level)
   n_data = parse_conf_n_vx(dict);

   // Allocate memory for data field options
   data_opt = new TCDiagDataOpt [n_data];

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
      data_opt[i].process_config(file_type, i_dict);

      // Update top-level TCDiagNcOutInfo settings
      nc_info += data_opt[i].nc_info;

      // Parse field-specific grid info
      TCRMWGridInfo gi = parse_grid_info(i_dict);

      // Search for existing grid info
      int i_match = -1;
      for(int i_gi=0; i_gi<grid_info_list.size(); i_gi++) {
         if(grid_info_list[i_gi] == gi) {
            i_match = i_gi;
            break;
         }
      }

      // Add new grid info, if needed
      if(i_match < 0) {
         grid_info_list.push_back(gi);
         i_match = grid_info_list.size()-1;
      }

      // Store the grid info pointer
      data_opt[i].grid_info = &grid_info_list[i_match];

      // Update the write_nc flag, if needed
      if(data_opt[i].nc_info.do_cyl_latlon ||
         data_opt[i].nc_info.do_cyl_raw) {
         data_opt[i].grid_info->write_nc = true;
      }

   }

   return;
}

////////////////////////////////////////////////////////////////////////

TCRMWGridInfo TCDiagConfInfo::parse_grid_info(Dictionary &dict) {
   TCRMWGridInfo gi;

   // Constant name
   gi.data.name = "TCRMW";

   // Conf: n_range
   gi.data.range_n = dict.lookup_int(conf_key_n_range);

   // Conf: azimuth_n
   gi.data.azimuth_n = dict.lookup_int(conf_key_n_azimuth);

   // Conf: max_range
   gi.data.range_max_km = dict.lookup_double(conf_key_max_range);

   // Conf: delta_range
   gi.delta_range_km = dict.lookup_double(conf_key_delta_range);

   // Conf: rmw_scale
   gi.rmw_scale = dict.lookup_double(conf_key_rmw_scale);

   return(gi);
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

   nc_info.clear();

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

   // Set the VarInfo object
   var_info = info_factory.new_var_info(file_type);
   var_info->set_dict(dict);

   // Dump the contents of the current VarInfo
   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5) << "Parsed field:\n";
      var_info->dump(cout);
   }

   // Conf: nc_pairs_flag
   parse_nc_info(dict);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagDataOpt::parse_nc_info(Dictionary &dict) {
   const DictionaryEntry * e = (const DictionaryEntry *) 0;

   e = dict.lookup(conf_key_nc_out_flag);

   if(!e) {
      mlog << Error << "\nTCDiagDataOpt::parse_nc_info() -> "
           << "lookup failed for key \"" << conf_key_nc_out_flag
           << "\"\n\n";
      exit(1);
   }

   const ConfigObjectType type = e->type();

   if(type == BooleanType) {
      bool value = e->b_value();

      if(!value) nc_info.set_all_false();

      return;
   }

   // It should be a dictionary
   if(type != DictionaryType) {
      mlog << Error << "\nTCDiagDataOpt::parse_nc_info() -> "
           << "bad type (" << configobjecttype_to_string(type)
           << ") for key \"" << conf_key_nc_out_flag << "\"\n\n";
      exit(1);
   }

   // Parse the various entries
   Dictionary * d = e->dict_value();

   nc_info.do_track       = d->lookup_bool(conf_key_track_flag);
   nc_info.do_grid_latlon = d->lookup_bool(conf_key_grid_latlon_flag);
   nc_info.do_grid_raw    = d->lookup_bool(conf_key_grid_raw_flag);
   nc_info.do_cyl_latlon  = d->lookup_bool(conf_key_cyl_latlon_flag);
   nc_info.do_cyl_raw     = d->lookup_bool(conf_key_cyl_raw_flag);
   nc_info.do_diag        = d->lookup_bool(conf_key_diag_flag);

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
   if(t.do_grid_latlon) do_grid_latlon = true;
   if(t.do_grid_raw)    do_grid_raw    = true;
   if(t.do_cyl_latlon)  do_cyl_latlon  = true;
   if(t.do_cyl_raw)     do_cyl_raw     = true;
   if(t.do_diag)        do_diag        = true;

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCDiagNcOutInfo::clear() {

   set_all_false();

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCDiagNcOutInfo::all_false() const {

   bool status = do_track       ||
                 do_grid_latlon || do_grid_raw ||
                 do_cyl_latlon  || do_cyl_raw  ||
                 do_diag;

   return(!status);
}

////////////////////////////////////////////////////////////////////////

void TCDiagNcOutInfo::set_all_false() {

   do_track       = false;
   do_grid_latlon = false;
   do_grid_raw    = false;
   do_cyl_latlon  = false;
   do_cyl_raw     = false;
   do_diag        = false;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCDiagNcOutInfo::set_all_true() {

   do_track       = true;
   do_grid_latlon = true;
   do_grid_raw    = true;
   do_cyl_latlon  = true;
   do_cyl_raw     = true;
   do_diag        = true;
   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for struct TCRMWGridInfo
//
////////////////////////////////////////////////////////////////////////

TCRMWGridInfo::TCRMWGridInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

bool TCRMWGridInfo::operator==(const TCRMWGridInfo &t) {

   return(data.range_n ==          t.data.range_n &&
          data.azimuth_n ==        t.data.azimuth_n &&
          is_eq(data.range_max_km, t.data.range_max_km) &&
          is_eq(delta_range_km,    t.delta_range_km) &&
          is_eq(rmw_scale,         t.rmw_scale));
}

////////////////////////////////////////////////////////////////////////

void TCRMWGridInfo::clear() {

   data.range_n      = bad_data_int;
   data.azimuth_n    = bad_data_int;
   data.range_max_km = bad_data_double;
   data.lat_center   = bad_data_double;
   data.lon_center   = bad_data_double;

   delta_range_km = bad_data_double;
   rmw_scale      = bad_data_double;

   write_nc = false;

   return;
}

////////////////////////////////////////////////////////////////////////
