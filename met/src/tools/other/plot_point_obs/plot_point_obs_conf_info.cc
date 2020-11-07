// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "plot_point_obs_conf_info.h"
#include "vx_log.h"

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
   fill_ctable.clear();
   fill_ctable_flag = false;
   fill_colorbar_flag = false;

   // Initialize point location data
   n_obs = 0;
   locations.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::process_config(Dictionary &dict) {

   // Conf: JHG work here

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

   // Only store the value if a colortable is defined
   loc.val = (fill_ctable_flag ? obs.getValue() : bad_data_double);

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
   data_plane_info = (VarInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::clear() {

   // Initialize values
   data_plane_flag = false;
   data_ctable.clear();
   data_ctable_flag = false;
   data_colorbar_flag = false;
   conf.clear();
   plot_opts.clear();

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

void PlotPointObsConfInfo::process_config() {
   ConcatString s;
   StringArray sa;
   Dictionary *dict = (Dictionary *) 0;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: JHG work here

   return;
}
////////////////////////////////////////////////////////////////////////

bool PlotPointObsConfInfo::add(const Observation &obs) {
   bool match = false;

   for(vector<PlotPointObsOpt>::iterator it = plot_opts.begin();
       it != plot_opts.end(); it++) {
      if((match = it->add(obs))) break;
   }

   return(match);
}
    
////////////////////////////////////////////////////////////////////////
