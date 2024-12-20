// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>
#include <time.h>

#include <netcdf>

#include "file_exists.h"

#include "string_fxns.h"
#include "vx_log.h"
#include "nc_utils.h"
#include "ioda.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

static bool has_postfix(const std::string &, std::string const &);

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


iodaFile::iodaFile() {
   initialize();
}


////////////////////////////////////////////////////////////////////////

/*
vector<point_pair_t> iodaFile::point_pairs(
      const char *var_name1, const char *var_name2,
      const char *group_name1, const char *group_name2, int channel) {
   static const char *method_name = "iodaFile::point_pairs -> ";
   NcVar obs_var1 = group_name1 ? get_var(f_in, var_name1, group_name1) : get_var(f_in, var_name1);
   NcVar obs_var2 = group_name2 ? get_var(f_in, var_name2, group_name2) : get_var(f_in, var_name2);

   if (IS_INVALID_NC(obs_var1)) {
      mlog << Error << "\n" << method_name 
           << "Fail to get data for " << var_name1
           << " (" << (group_name1 ? group_name1 : " ") << ")\n\n";
      clear();
      exit(-1);
   }

   if (IS_INVALID_NC(obs_var2)) {
           << "Fail to get data for " << var_name2
           << " (" << (group_name2 ? group_name2 : " ") << ")\n\n";
      clear();
      exit(-1);
   }


   ConcatString log_group_name1;
   ConcatString log_group_name2;
   StringArray dimNames1;
   StringArray dimNames2;
   get_dim_names(&obs_var1, dimNames1);
   get_dim_names(&obs_var2, dimNames2);

   if (group_name1) {
      log_group_name1.add("(");
      log_group_name1.add(group_name1);
      log_group_name1.add(")");
   }
   if (group_name2) {
      log_group_name2.add("(");
      log_group_name2.add(group_name2);
      log_group_name2.add(")");
   }
   if (is_eq(channel, bad_data_int)) {
      if (1 < dimNames1.n()) {
         mlog << Error << "\n" << method_name 
              << "Channel is not given for " << var_name1
              << " " << log_group_name1 << "\n\n";
         clear();
         exit(-1);
      }
      if (1 < dimNames2.n()) {
         mlog << Error << "\n" << method_name 
              << "Channel is not given for " << var_name2
              << " " << log_group_name2 << "\n\n";
         clear();
         exit(-1);
      }
   }
   else {
      if (channel >= dimNames1.n()) {
         mlog << Error << "\n" << method_name 
              << "Channel (" << channel << ") is out of bound (" << dimNames1.n()
              << ") for " << var_name1 << " " << log_group_name1 << "\n\n";
         clear();
         exit(-1);
      }
      if (channel >= dimNames2.n()) {
         mlog << Error << "\n" << method_name 
              << "Channel (" << channel << ") is out of bound (" << dimNames2.n()
              << ") for " << var_name2 << " " << log_group_name2 << "\n\n";
         clear();
         exit(-1);
      }
   }

   vector<float> obs_val1(nlocs, bad_data_float);
   vector<float> obs_val2(nlocs, bad_data_float);
   get_dim_names(&obs_var1, dimNames1);
   get_dim_names(&obs_var2, dimNames2);

   if(!get_nc_data(&obs_var1, obs_val1.data(), nlocs)) {
      mlog << Error << "\n" << method_name
           << "trouble getting latitude\n\n";
      exit(1);
   }
   if(!get_nc_data(&in_hdr_lon_var, lon_arr.data(), nlocs)) {
      mlog << Error << "\n" << method_name
           << "trouble getting longitude\n\n";
      exit(1);
   }


}
*/

////////////////////////////////////////////////////////////////////////

bool iodaFile::check_missing_thresh(float value) const {
   bool check = false;
   for(int idx=0; idx<conf_info->missing_thresh.n(); idx++) {
      if(conf_info->missing_thresh[idx].check(value)) {
          check = true;
          break;
      }
   }
   return check;
}

////////////////////////////////////////////////////////////////////////

void iodaFile::clear() {
   nrecs = 0;
   nlocs = 0;
   nvars = bad_data_int ;
   nstring = string_data_len;
   ndatetime = 1;

   dim_names.clear();
   metadata_vars.clear();
   obs_value_vars.clear();


   nlocs_name.clear();
   datetime_name.clear();
   lon_name.clear();
   lat_name.clear();
   msg_type_name.clear();
   station_id_name.clear();

   lat_arr.clear();
   lon_arr.clear();
   elv_arr.clear();
   vld_arr.clear();   // valid times`

   msg_types.clear();
   station_ids.clear();

   point_pairs.clear();
}

////////////////////////////////////////////////////////////////////////

e_ioda_format iodaFile::find_ioda_format(NcFile *_f_in) {
   ioda_format = e_ioda_format::v2;

   if (! has_nc_group(_f_in, obs_group_name)) ioda_format = e_ioda_format::v1;
   return ioda_format;
}

////////////////////////////////////////////////////////////////////////

ConcatString iodaFile::find_meta_name(const string &meta_key, const StringArray &available_names) {
   ConcatString metadata_name;

   if (available_names.has(meta_key)) metadata_name = meta_key;
   else {
      StringArray alt_names = conf_info->metadata_map[meta_key];
      for (int idx =0; idx<alt_names.n(); idx++) {
         if (available_names.has(alt_names[idx])) {
            metadata_name = alt_names[idx];
            break;
         }
      }
   }
   return metadata_name;
}

////////////////////////////////////////////////////////////////////////

bool iodaFile::get_meta_data_chars(NcVar &var, char *metadata_buf) const {
   bool status = false;
   static const char *method_name = "get_meta_data_chars() -> ";

   if(IS_VALID_NC(var)) {
      status = get_nc_data(&var, metadata_buf);
      if(!status) {
         mlog << Error << "\n" << method_name << " -> "
              << "trouble getting " << GET_NC_NAME(var) << "\n\n";
         exit(1);
      }
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

bool iodaFile::get_meta_data_double(const char *metadata_key, double *metadata_buf) {
   bool status = false;
   static const char *method_name = "iodaFile::get_meta_data_double() -> ";

   ConcatString metadata_name = find_meta_name(metadata_key, metadata_vars);
   if(metadata_name.nonempty()) {
      NcVar meta_var = get_var(f_in, metadata_name.c_str(), metadata_group_name);
      if(IS_VALID_NC(meta_var)) {
         status = get_nc_data(&meta_var, metadata_buf, nlocs);
         if (!status) mlog << Debug(3) << method_name
                           << "trouble getting " << metadata_name << "\n";
      }
   }
   else mlog << Debug(4) << method_name
             << "Metadata for " << metadata_key << " does not exist!\n";
   if(status) {
      for(int idx=0; idx<nlocs; idx++) {
         if(check_missing_thresh(metadata_buf[idx])) metadata_buf[idx] = bad_data_float;
      }
   }
   else {
      for(int idx=0; idx<nlocs; idx++)
         metadata_buf[idx] = bad_data_float;
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

void iodaFile::get_obs_metadata_names_v1() {
   static const char *method_name = "iodaFile::get_obs_metadata_names_v1() -> ";
   bool error_out = true;

   StringArray var_names;
   get_var_names(f_in, &var_names);
   for(int idx=0; idx<var_names.n(); idx++) {
      if(has_postfix(var_names[idx], "@MetaData")) {
         metadata_vars.add(var_names[idx].substr(0, var_names[idx].find('@')));
      }
      if(has_postfix(var_names[idx], "@ObsValue")
         || has_postfix(var_names[idx], "@DerivedObsValue")) {
         obs_value_vars.add(var_names[idx].substr(0, var_names[idx].find('@')));
      }
   }
   if(mlog.verbosity_level() >= 8) {
      for(int idx=0; idx<var_names.n(); idx++)
         mlog << Debug(8) << method_name << "var_name: " << var_names[idx] << "\n";
   }

   ConcatString nvars_ = find_meta_name("nvars", dim_names);
   if(has_dim(f_in, nvars_.c_str())) {
      nvars = get_dim_value(f_in, nvars_.c_str(), error_out); // number of variables
      ConcatString nstring_ = find_meta_name("nstring", dim_names);
      if(dim_names.has(nstring_)) nstring = get_dim_value(f_in, nstring_.c_str(), error_out);

      ConcatString nrecs_ = find_meta_name("nrecs", dim_names);
      if(dim_names.has(nrecs_)) nrecs = get_dim_value(f_in, nrecs_.c_str(), false);
      else {
         nrecs = nvars * nlocs;
         mlog << Debug(3) << "\n" << method_name
              << "nrecs dimension does not exist, so computed\n";
      }
   }
}

////////////////////////////////////////////////////////////////////////

void iodaFile::get_obs_metadata_names_v2() {
   StringArray group_names;
   group_names.add(metadata_group_name);
   get_var_names(f_in, &metadata_vars, group_names);
   group_names.clear();
   group_names.add("ObsValue");
   group_names.add("DerivedObsValue");
   get_var_names(f_in, &obs_value_vars, group_names);
}

////////////////////////////////////////////////////////////////////////

vector<point_pair_t> &iodaFile::get_point_pairs(
      const char *var_name_f, const char *var_name_o,
      const char *group_name_f, const char *group_name_o, const int channel) {
   ConcatString log_var_name_f = var_name_f;
   ConcatString log_var_name_o = var_name_o;
   static const char *method_name = "iodaFile::get_point_pairs -> ";

   vector<double> obs_val_f(nlocs, bad_data_float);
   vector<double> obs_val_o(nlocs, bad_data_float);

   if (!read_obs_data(obs_val_f.data(), var_name_f, group_name_f, channel)) {
      clear();
      exit(-1);
   }
   if (!read_obs_data(obs_val_o.data(), var_name_o, group_name_o, channel)) {
      clear();
      exit(-1);
   }

   point_pairs.clear();
   point_pairs.resize(nlocs);
   for (int i=0; i<nlocs; i++) {
//      point_pairs.push_back(point_pair_t(msg_types[i], station_ids[i], lat_arr[i],
//                                         lon_arr[i], vld_arr[i], obs_pres_arr[i],
//                                         obs_hght_arr[i],obs_val_f[i], obs_val_o[i]));
      point_pairs.push_back(point_pair_t {msg_types[i], station_ids[i], lat_arr[i],
                                          lon_arr[i], vld_arr[i], obs_pres_arr[i],
      obs_hght_arr[i],obs_val_f[i], obs_val_o[i]} );
   }

   return point_pairs;
}

////////////////////////////////////////////////////////////////////////

void iodaFile::initialize() {
}

////////////////////////////////////////////////////////////////////////

bool iodaFile::is_in_metadata_map(const string &metadata_key,
                                  const StringArray &available_list) const {
   bool found = available_list.has(metadata_key);

   if (!found) {
      StringArray alt_names = conf_info->metadata_map[metadata_key];
      if (alt_names.n() > 0) {
         for (int idx=0; idx<alt_names.n(); idx++) {
            found = available_list.has(alt_names[idx]);
            if (found) break;
         }
      }
   }
   return found;
}

////////////////////////////////////////////////////////////////////////

bool iodaFile::read_data_to_vector(const char *var_name, vector<string> &hdr_data, int nstring) {
   bool status = false;
   static const char *method_name = "iodaFile::read_data_to_vector() -> ";
   static const char *method_name_s = "iodaFile::read_data_to_vector() ";

   //   std::vector<char> station_ids;
   //msg_types.resize(nlocs*(nstring+1), 0);
   //station_ids.resize(nlocs*(nstring+1), 0);

   char hdr_val[512];
   hdr_data.clear();
   NcVar hdr_var = get_var(f_in, var_name, metadata_group_name);
cout << "?   metadata_group_name = " << metadata_group_name << ", var_name = " << var_name  << "\n";
   if (IS_INVALID_NC(hdr_var)) return status;

cout << "?   GET_NC_TYPE_ID(hdr_var) = " << GET_NC_TYPE_ID(hdr_var) << ", NC_STRING = " << NC_STRING << "\n";
   if (NC_STRING == GET_NC_TYPE_ID(hdr_var)) {
      vector<char *> hdr_data2(nlocs, nullptr);
      for (int i=0; i<nlocs; i++ ) hdr_data2.push_back(new char[nstring+1]);
      if ((status = get_nc_data(&hdr_var, hdr_data2.data()))) {
cout << " if(XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
         for (int i=0; i<nlocs; i++ ) {
            m_strncpy(hdr_val, hdr_data2[i], nstring, method_name_s, "ioda_header");
            m_rstrip(hdr_val, nstring);
cout << "?   hdr_val = [" << hdr_val << "]\n";
            hdr_data.push_back(hdr_val);
            delete hdr_data2[i];
         }
         hdr_data2.clear();
      }
      else cout << " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
   }
   else {
      vector<char> hdr_data2(nlocs*(nstring+1),0);
      if ((status = get_meta_data_chars(hdr_var, hdr_data2.data()))) {
         for (int i=0; i<nlocs; i++ ) {
            m_strncpy(hdr_val, hdr_data2.data()+(i*nstring), nstring, method_name_s, "ioda_header");
            m_rstrip(hdr_val, nstring);
cout << "?   hdr_val = [" << hdr_val << "]\n";
            hdr_data.push_back(hdr_val);
         }
      }
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

void iodaFile::read_header() {
   bool status = false;
   bool error_out = true;
   static const char *method_name = "iodaFile::read_header() -> ";
   static const char *method_name_s = "iodaFile::read_header() ";

   if (!read_time()) {
      clear();
      exit(-1);
   }

   NcVar in_hdr_lat_var = get_var(f_in, lat_name.c_str(), metadata_group_name);
   NcVar in_hdr_lon_var = get_var(f_in, lon_name.c_str(), metadata_group_name);
   lat_arr.resize(nlocs);
   lon_arr.resize(nlocs);
   elv_arr.resize(nlocs);
   if(!get_nc_data(&in_hdr_lat_var, lat_arr.data(), nlocs)) {
      mlog << Error << "\n" << method_name
           << "trouble getting latitude\n\n";
      exit(1);
   }
   if(!get_nc_data(&in_hdr_lon_var, lon_arr.data(), nlocs)) {
      mlog << Error << "\n" << method_name
           << "trouble getting longitude\n\n";
      exit(1);
   }
   get_meta_data_double("elevation", elv_arr.data());

   obs_pres_arr.resize(nlocs);
   obs_hght_arr.resize(nlocs);
   get_meta_data_double("pressure", obs_pres_arr.data());
   get_meta_data_double("height",   obs_hght_arr.data());

   if(msg_type_name.nonempty()) {
      status = read_data_to_vector(msg_type_name.c_str(), msg_types, nstring);
   }
   else {
      mlog << Debug(1) << method_name
           << "The metadata variable for message type does not exist!\n";
   }

   if(station_id_name.nonempty()) {
      status = read_data_to_vector(station_id_name.c_str(), station_ids, nstring);
   }
   else {
      mlog << Debug(1) << method_name
           << "The metadata variable for station ID does not exist!\n";
   }

}

////////////////////////////////////////////////////////////////////////

bool iodaFile::read_time() {
   bool status = false;
   bool error_out = true;
   static const char *method_name = "iodaFile::read_time() -> ";
   static const char *method_name_s = "iodaFile::read_time() ";

   NcVar in_hdr_vld_var = get_var(f_in, datetime_name.c_str(), metadata_group_name);
   if (IS_INVALID_NC(in_hdr_vld_var)) {
      clear();
      mlog << Error << "\n" << method_name << "Fail to get datetime variable ("
           << datetime_name << ")\n\n";
      exit(-1);
   }

   bool is_time_offset = false;
   bool is_time_string = false;
   if (NC_STRING == GET_NC_TYPE_ID(in_hdr_vld_var)) {
      is_time_string = true;
   }
   else if (NC_CHAR == GET_NC_TYPE_ID(in_hdr_vld_var)) {
      if(dim_names.has("ndatetime")) ndatetime = get_dim_value(f_in, "ndatetime", error_out);
      else {
         NcDim datetime_dim = get_nc_dim(&in_hdr_vld_var, 1);
         ndatetime = IS_VALID_NC(datetime_dim) ? get_dim_size(&datetime_dim) : nstring;
         mlog << Debug(3) << "\n" << method_name
              << "ndatetime dimension does not exist!\n";
      }
      mlog << Debug(5) << method_name << "dimensions: nvars=" << nvars << ", nlocs=" << nlocs
           << ", nrecs=" << nrecs << ", nstring=" << nstring << ", ndatetime=" << ndatetime << "\n";
   }
   else is_time_offset = true;

   vld_arr.resize(nlocs);
   if (is_time_offset) {
      unixtime base_ut;
      int sec_per_unit;
      ConcatString units;
      vector<float> hdr_ut_arr(nlocs);
      bool no_leap_year = get_att_no_leap_year(&in_hdr_vld_var);
      status = get_nc_data(&in_hdr_vld_var, hdr_ut_arr.data());
      if (get_var_units(&in_hdr_vld_var, units) && units.nonempty()) {
         parse_cf_time_string(units.c_str(), base_ut, sec_per_unit);
      }
      else {
         mlog << Error << "\n" << method_name << "Fail to get time units from "
              << GET_NC_NAME(in_hdr_vld_var) << "\n\n";
         return status;
      }
      for (int i=0; i< nlocs; i++) {
         vld_arr[i] = add_to_unixtime(base_ut, sec_per_unit,
                                       hdr_ut_arr[i], no_leap_year);
      }
   }
   else if (is_time_string) {
      char valid_time[ndatetime+1];
      vector<char> hdr_vld_block(nlocs*(ndatetime+1), 0);
      vector<char *> hdr_vld_block2(nlocs, nullptr);
      for (int i=0; i<nlocs; i++) {
         hdr_vld_block2.push_back(hdr_vld_block.data() + i*ndatetime);
      }
      status = get_nc_data(&in_hdr_vld_var, hdr_vld_block2.data());

      for (int i=0; i<nlocs; i++ ) {
         m_strncpy(valid_time, (const char *)hdr_vld_block2[i],
                   ndatetime, method_name_s, "valid_time", true);
         valid_time[ndatetime] = 0;
         vld_arr[i] = yyyymmddThhmmss_to_unix(valid_time);
      }
      hdr_vld_block.clear();
      hdr_vld_block2.clear();
   }
   else {
      char valid_time[ndatetime+1];
      vector<char> hdr_vld_block(nlocs*(ndatetime+1), 0);
      status = get_nc_data(&in_hdr_vld_var, hdr_vld_block.data());

      for (int i=0; i<nlocs; i++ ) {
         m_strncpy(valid_time, (const char *)(hdr_vld_block.data() + (i * ndatetime)),
                   ndatetime, method_name_s, "valid_time", true);
         valid_time[ndatetime] = 0;
         vld_arr[i] = yyyymmddThhmmss_to_unix(valid_time);
      }
   }
   if(!status) {
      mlog << Error << "\n" << method_name
           << "trouble getting datetime\n\n";
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

void iodaFile::read_ioda(netCDF::NcFile *_f_in) {
   static const char *method_name = "iodaFile::read_ioda() -> ";
   bool error_out = true;

   f_in = _f_in;

   clear();

   get_dim_names(f_in, &dim_names);
   nlocs_name = find_meta_name("nlocs", dim_names);
   if(nlocs_name.empty()) {
      mlog << Error << "\n" << method_name
           << "trouble getting the dimension name for nlocs\n\n";
      exit(1);
   }
   else {
      nlocs = get_dim_value(f_in, nlocs_name.c_str(), error_out); // number of locations
      if(nlocs == 0) {
         mlog << Error << "\n" << method_name
              << "No ioda record (the dimension size of "
              << nlocs_name << " is " << nlocs << ")\n\n";
         exit(1);
      }
   }

   if ( find_ioda_format(_f_in) == e_ioda_format::v1 ) {
      get_obs_metadata_names_v1();
   }
   else {
      get_obs_metadata_names_v2();
   }
   if(mlog.verbosity_level() >= 6) {
      for(int idx=0; idx<dim_names.n(); idx++)
         mlog << Debug(8) << method_name << "dim_name: " << dim_names[idx] << "\n";
      for(int idx=0; idx<metadata_vars.n(); idx++)
         mlog << Debug(8) << method_name << "metadata: " << metadata_vars[idx] << "\n";
      for(int idx=0; idx<obs_value_vars.n(); idx++)
         mlog << Debug(8) << method_name << "ObsValue or Derived: " << obs_value_vars[idx] << "\n";
   }

   read_metadata_names();

   read_header();

}

////////////////////////////////////////////////////////////////////////

void iodaFile::read_metadata_names() {
   datetime_name = find_meta_name(conf_key_datetime, metadata_vars);
   lon_name = find_meta_name("longitude", metadata_vars);
   lat_name = find_meta_name("latitude", metadata_vars);
   msg_type_name = find_meta_name(conf_key_message_type, metadata_vars);
   station_id_name = find_meta_name(conf_key_station_id, metadata_vars);
}

////////////////////////////////////////////////////////////////////////

bool iodaFile::read_obs_data(double *data_buf, const char *var_name,
                             const char *group_name, const int channel) {
   bool status = false;
   ConcatString log_var_name = var_name;
   static const char *method_name = "iodaFile::read_obs_data -> ";

   if (group_name) {
      log_var_name.add("(");
      log_var_name.add(group_name);
      log_var_name.add(")");
   }

   NcVar obs_var = group_name ? get_var(f_in, var_name, group_name) : get_var(f_in, var_name);
   if (IS_INVALID_NC(obs_var)) {
      mlog << Error << "\n" << method_name 
           << "Fail to get data for " << log_var_name << "\n\n";
      return status;
   }

   StringArray dim_names;
   get_dim_names(&obs_var, &dim_names);
   if (is_eq(channel, bad_data_int)) {
      if (1 < dim_names.n()) {
         mlog << Error << "\n" << method_name 
              << "Channel is not given for " << log_var_name << "\n\n";
         return status;
      }
      status = get_nc_data(&obs_var, data_buf, nlocs);
   }
   else {
      if (channel >= dim_names.n()) {
         mlog << Error << "\n" << method_name 
              << "Channel (" << channel << ") is out of bound (" << dim_names.n()
              << ") for " << log_var_name << "\n\n";
         return status;
      }
      if (1 == dim_names.n()) {
         mlog << Debug(1) << method_name 
              << "Channel (" << channel << ") is configured to the variable "
              << log_var_name << " which does not have the chenell dimension\n";
         status = get_nc_data(&obs_var, data_buf, nlocs);
      }
      else {
         LongArray lengths;
         LongArray offsets;
         if (nlocs_name == dim_names[0]) {
            lengths.add(nlocs);
            lengths.add(1);
            offsets.add(0);
            offsets.add(channel);
         }
         else {
            lengths.add(1);
            lengths.add(nlocs);
            offsets.add(0);
            offsets.add(channel);
         }
         status = get_nc_data(&obs_var, data_buf, lengths, offsets);
      }
   }

   if (!status) {
      mlog << Error << "\n" << method_name
           << "trouble getting " << log_var_name<< "\n\n";
   }

   return status;

}

////////////////////////////////////////////////////////////////////////

bool has_postfix(const std::string &str_buf, std::string const &postfix) {
   auto buf_len = str_buf.length();
   auto postfix_len = postfix.length();
   if(buf_len >= postfix_len) {
      return (0 == str_buf.compare(buf_len - postfix_len, postfix_len, postfix));
   } else {
      return false;
   }
}


////////////////////////////////////////////////////////////////////////

