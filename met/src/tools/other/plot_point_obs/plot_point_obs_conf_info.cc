// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vx_log.h"
#include "vx_util.h"
#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"

#include "plot_point_obs_conf_info.h"

////////////////////////////////////////////////////////////////////////

bool operator<(const LocationInfo&lhs, const LocationInfo& rhs) {
   return(!is_eq(lhs.lat, rhs.lat) ||
          !is_eq(lhs.lon, rhs.lon) ||
          !is_eq(lhs.val, rhs.val));
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PlotPointObsOpt
//
////////////////////////////////////////////////////////////////////////

PlotPointObsOpt::PlotPointObsOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PlotPointObsOpt::~PlotPointObsOpt() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::clear() {

   // Initialize tring filtering options
   msg_typ.clear();
   sid_inc.clear();
   sid_exc.clear();
   obs_var.clear();
   obs_qty.clear();

   // Initialize time filtering options
   valid_beg = valid_end = (unixtime) 0;

   // Initialize value filtering options
   lat_thresh.clear();
   lon_thresh.clear();
   elv_thresh.clear();
   hgt_thresh.clear();
   prs_thresh.clear();
   obs_thresh.clear();

   // Initialize data processing options
   convert_fx.clear();
   censor_thresh.clear();
   censor_val.clear();

   // Initialize plotting options
   dotsize_fx.clear();
   line_color.clear();
   line_width = 0.0;
   fill_color.clear();
   fill_plot_info.clear();

   // Initialize point location data
   n_obs = 0;
   obs_value_flag = false;
   locations.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::process_config(Dictionary &dict) {

   // Initialize
   clear();

   // Conf: message_type
   msg_typ = parse_conf_message_type(&dict);
    
   // Conf: sid_inc and sid_exc
   sid_inc = parse_conf_sid_list(&dict, conf_key_sid_inc);
   sid_exc = parse_conf_sid_list(&dict, conf_key_sid_exc);

   // Conf: obs_var
   obs_var = dict.lookup_string_array(conf_key_obs_var);

   // Conf: obs_quality
   obs_qty = dict.lookup_string_array(conf_key_obs_qty);

   // Conf: valid_beg and valid_end
   valid_beg = dict.lookup_unixtime(conf_key_valid_beg);
   valid_end = dict.lookup_unixtime(conf_key_valid_end);

   // Conf: lat_thresh and lon_thresh
   lat_thresh = dict.lookup_thresh(conf_key_lat_thresh);
   lon_thresh = dict.lookup_thresh(conf_key_lon_thresh);

   // Conf: elv_thresh and hgt_thresh
   elv_thresh = dict.lookup_thresh(conf_key_elv_thresh);
   hgt_thresh = dict.lookup_thresh(conf_key_hgt_thresh);
    
   // Conf: prs_thresh and obs_thresh
   prs_thresh = dict.lookup_thresh(conf_key_prs_thresh);
   obs_thresh = dict.lookup_thresh(conf_key_obs_thresh);

   // Conf: convert function
   convert_fx.set(dict.lookup(conf_key_convert));

   // Conf: censor_thresh and censor_val
   censor_thresh = dict.lookup_thresh_array(conf_key_censor_thresh, false);
   censor_val    = dict.lookup_num_array(conf_key_censor_val, false);

   // Check for equal number of censor thresholds and values
   if(censor_thresh.n() != censor_val.n()) {
      mlog << Error << "\nPlotPointObsOpt::process_config() -> "
           << "The number of censor thresholds in \""
           << conf_key_censor_thresh << "\" (" << censor_thresh.n()
           << ") must match the number of replacement values in \""
           << conf_key_censor_val << "\" (" << censor_val.n() << ").\n\n";
      exit(1);
   }

   // Conf: dotsize function
   dotsize_fx.set(dict.lookup(conf_key_dotsize));

   // Conf: line_color
   line_color = dict.lookup_num_array(conf_key_line_color);

   // Conf: line_width
   line_width = dict.lookup_double(conf_key_line_width);

   // Conf: fill_color
   fill_color = dict.lookup_num_array(conf_key_fill_color);

   // Check for correctly formatted colors
   if(line_color.n() != 3 || fill_color.n() != 3) {
      mlog << Error << "\nPlotPointObsOpt::process_config() -> "
           << "The \"" << conf_key_line_color << "\" and \"" << conf_key_fill_color
           << "\" entries must be specified as three RGB values.\n\n";
      exit(1);
   }

   // Conf: fill_plot_info
   fill_plot_info = parse_conf_plot_info(
                       dict.lookup_dictionary(conf_key_fill_plot_info));

   // Set obs_value_flag if a fill color table is enabled
   // or the dotsize function is not constant
   obs_value_flag = (fill_plot_info.flag || dotsize_fx(1) != dotsize_fx(2));

   return;
}

////////////////////////////////////////////////////////////////////////

bool PlotPointObsOpt::add(const Observation &obs) {

   // message type
   if(msg_typ.n() > 0 && !msg_typ.has(obs.getHeaderType())) return(false);

   // station id
   if((sid_inc.n() > 0 && !sid_inc.has(obs.getStationId())) ||
      (sid_exc.n() > 0 &&  sid_exc.has(obs.getStationId()))) return(false);

   // observation variable
   if(obs_var.n() > 0 && !obs_var.has(obs.getVarName())) return(false);

   // quality control string
   if(obs_qty.n() > 0 && !obs_qty.has(obs.getQualityFlag())) return(false);
    
   // valid time
   unixtime ut = obs.getValidTime();
   if((valid_beg > 0 && ut < valid_beg) ||
      (valid_end > 0 && ut > valid_end)) return(false);

   // lat, lon
   if(!lat_thresh.check(obs.getLatitude()) ||
      !lon_thresh.check(obs.getLongitude())) return(false);

   // elevation, height, pressure
   if(!elv_thresh.check(obs.getElevation()) ||
      !hgt_thresh.check(obs.getHeight()) ||
      !prs_thresh.check(obs.getPressureLevel())) return(false);

   // observation value
   if(!obs_thresh.check(obs.getValue())) return(false);

   // Store this matching point location
   LocationInfo loc;
   loc.lat = obs.getLatitude();
   loc.lon = obs.getLongitude();

   // Only store the value if the flag is set
   loc.val = (obs_value_flag ? obs.getValue() : bad_data_double);

   // Update the set of locations
   n_obs++;
   locations.insert(loc);

   return(true);
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PlotPointObsConfInfo
//
////////////////////////////////////////////////////////////////////////

PlotPointObsConfInfo::PlotPointObsConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PlotPointObsConfInfo::~PlotPointObsConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::init_from_scratch() {

   // Initialize pointers
   grid_data_info = (VarInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::clear() {

   // Initialize values
   grid_data_flag = false;
   grid_plot_info.clear();
   point_opts.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::read_config(const char *user_file_name) {

   // Read the config file constants and map data
   conf.read(replace_path(config_const_filename).c_str());
   conf.read(replace_path(config_map_data_filename).c_str());
   conf.read(replace_path(default_config_filename).c_str());

   // Read the user file name, if provided
   if(user_file_name) conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::process_config(GrdFileType ftype) {
   VarInfoFactory info_factory;
   Dictionary  *dict = (Dictionary *) 0;
   Dictionary *fdict = (Dictionary *) 0;
   PlotPointObsOpt opt;
   int i, n;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: grid_data
   dict = conf.lookup_dictionary(conf_key_grid_data);

   // Conf: field
   fdict = dict->lookup_array(conf_key_field);

   // Check length
   if((n = parse_conf_n_vx(fdict)) > 1) {
      mlog << Error << "\nPlotPointObsConfInfo::process_config() -> "
           << "the \"" << conf_key_grid_data << "." << conf_key_field
           << "\" array can only have length 0 or 1.\n\n";
      exit(1);
   }

   // Process gridded data
   if(n > 0) {

      grid_data_flag = true;

      // Allocate new VarInfo object
      grid_data_info = info_factory.new_var_info(ftype);
      grid_data_info->set_dict(*(fdict));

      // Conf: grid_plot_info
      grid_plot_info = parse_conf_plot_info(
                          dict->lookup_dictionary(conf_key_grid_plot_info));
   }

   // Conf: point_data
   dict = conf.lookup_array(conf_key_point_data);

   // Check length
   if((n = dict->n_entries()) == 0) {
      mlog << Error << "\nPlotPointObsConfInfo::process_config() -> "
           << "the \"" << conf_key_point_data
           << "\" array is empty!\n\n";
      exit(1);
   }

   // Parse each array entry
   for(i=0; i<n; i++) {
      opt.process_config(*((*dict)[i]->dict_value()));
      point_opts.push_back(opt);
   }
   
   return;
}
////////////////////////////////////////////////////////////////////////

bool PlotPointObsConfInfo::add(const Observation &obs) {
   bool match = false;

   for(vector<PlotPointObsOpt>::iterator it = point_opts.begin();
       it != point_opts.end(); it++) {
      if((match = it->add(obs))) break;
   }

   return(match);
}
    
////////////////////////////////////////////////////////////////////////
