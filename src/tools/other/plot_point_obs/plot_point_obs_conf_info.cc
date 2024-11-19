// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include "vx_log.h"
#include "vx_util.h"
#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_regrid.h"
#include "vx_statistics.h"

#include "plot_point_obs_conf_info.h"

using namespace std;


////////////////////////////////////////////////////////////////////////

bool LocationInfo::operator==(const LocationInfo &x) {
   return(is_eq(lat, x.lat) && is_eq(lon, x.lon) && is_eq(val, x.val));
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
   fill_ctable.clear();

   // Initialize point location data
   n_obs = 0;
   locations.clear();

   store_obs_val = false;
   fill_point = false;
   outline_point = false;

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::process_config(Dictionary &dict) {
   NumArray tmp_na;

   // Initialize
   clear();

   // Conf: msg_typ
   msg_typ = dict.lookup_string_array(conf_key_msg_typ);
    
   // Conf: sid_inc and sid_exc
   sid_inc = parse_conf_sid_list(&dict, conf_key_sid_inc);
   sid_exc = parse_conf_sid_list(&dict, conf_key_sid_exc);

   // Conf: obs_var
   obs_var = dict.lookup_string_array(conf_key_obs_var);

   // Conf: obs_gc
   obs_gc = dict.lookup_int_array(conf_key_obs_gc);

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
   censor_thresh = dict.lookup_thresh_array(conf_key_censor_thresh,
                                            false);
   censor_val    = dict.lookup_num_array(conf_key_censor_val, false);

   // Check for equal number of censor thresholds and values
   if(censor_thresh.n() != censor_val.n()) {
      mlog << Error << "\nPlotPointObsOpt::process_config() -> "
           << "The number of censor thresholds in \""
           << conf_key_censor_thresh << "\" (" << censor_thresh.n()
           << ") must match the number of replacement values in \""
           << conf_key_censor_val << "\" (" << censor_val.n()
           << ").\n\n";
      exit(1);
   }

   // Conf: dotsize function
   dotsize_fx.set(dict.lookup(conf_key_dotsize));

   // Conf: line_color
   tmp_na = dict.lookup_num_array(conf_key_line_color);

   // Check for correctly formatted colors
   if(tmp_na.n() != 0 && tmp_na.n() != 3) {
      mlog << Error << "\nPlotPointObsOpt::process_config() -> "
           << "The \"" << conf_key_line_color << "\" entry is "
           << "specified as three RGB values (0 to 255) or an empty "
           << "list.\n\n";
      exit(1);
   }

   // Store the color
   if(tmp_na.n() == 3) {
      outline_point = true;
      line_color.set_rgb(tmp_na[0], tmp_na[1], tmp_na[2]);
   }

   // Conf: line_width
   line_width = dict.lookup_double(conf_key_line_width);

   // Conf: fill_color
   tmp_na = dict.lookup_num_array(conf_key_fill_color);

   // Check for correctly formatted colors
   if(tmp_na.n() != 0 && tmp_na.n() != 3) {
      mlog << Error << "\nPlotPointObsOpt::process_config() -> "
           << "The \"" << conf_key_fill_color << "\" entry is "
           << "specified as three RGB values (0 to 255) or an empty "
           << "list.\n\n";
      exit(1);
   }

   // Store the color
   if(tmp_na.n() == 3) {
      fill_point = true;
      fill_color.set_rgb(tmp_na[0], tmp_na[1], tmp_na[2]);
   }
    
   // Conf: fill_plot_info
   fill_plot_info = parse_conf_plot_info(
                       dict.lookup_dictionary(conf_key_fill_plot_info));

   // Load the color table
   if(fill_plot_info.flag) {
      fill_point = true;
      fill_ctable.read(
         replace_path(fill_plot_info.color_table).c_str());
      if(fill_plot_info.plot_min != 0.0 ||
         fill_plot_info.plot_max != 0.0) {
         fill_ctable.rescale(fill_plot_info.plot_min,
                             fill_plot_info.plot_max,
                             bad_data_double);
      }
   }

   // Set store_obs_val if a fill color table is enabled
   // or the dotsize function is not constant
   store_obs_val = (fill_plot_info.flag ||
                    dotsize_fx(1) != dotsize_fx(2));

   return;
}

////////////////////////////////////////////////////////////////////////

bool PlotPointObsOpt::add(const Observation &obs) {

   // message type
   if(msg_typ.n() > 0 && !msg_typ.has(obs.getHeaderType())) {
      return false;
   }

   // station id
   if((sid_inc.n() > 0 && !sid_inc.has(obs.getStationId())) ||
      (sid_exc.n() > 0 &&  sid_exc.has(obs.getStationId()))) {
      return false;
   }

   // observation variable
   if(obs_var.n() > 0 && !obs_var.has(obs.getVarName())) {
      return false;
   }

   // observation GRIB code
   if(obs_gc.n() > 0 && !obs_gc.has(obs.getGribCode())) {
      return false;
   }

   // quality control string
   if(obs_qty.n() > 0 && !obs_qty.has(obs.getQualityFlag())) {
      return false;
   }
    
   // valid time
   unixtime ut = obs.getValidTime();
   if((valid_beg > 0 && ut < valid_beg) ||
      (valid_end > 0 && ut > valid_end)) {
      return false;
   }

   // lat, lon
   if(!lat_thresh.check(obs.getLatitude()) ||
      !lon_thresh.check(obs.getLongitude())) {
      return false;
   }

   // elevation, height, pressure
   if(!elv_thresh.check(obs.getElevation()) ||
      !hgt_thresh.check(obs.getHeight()) ||
      !prs_thresh.check(obs.getPressureLevel())) {
      return false;
   }

   // store the current observation value
   double cur_val = obs.getValue();

   // apply conversion logic
   if(convert_fx.is_set()) cur_val = convert_fx(cur_val);

   // apply censor logic
   for(int i=0; i<censor_thresh.n(); i++) {

      // break out after the first match
      if(censor_thresh[i].check(cur_val)) {
         cur_val = censor_val[i];
         break;
      }
   }

   // observation value
   if(!obs_thresh.check(cur_val)) {
      return false;
   }

   // Store this matching point location
   LocationInfo cur_loc;
   cur_loc.lat = obs.getLatitude();
   cur_loc.lon = obs.getLongitude();

   // Only store the value if the flag is set
   cur_loc.val = (store_obs_val ? cur_val : bad_data_double);

   // Update the set of locations
   n_obs++;
   if(!has(cur_loc)) locations.push_back(cur_loc);

   return true;
}

////////////////////////////////////////////////////////////////////////

bool PlotPointObsOpt::has(const LocationInfo &loc) {
   bool match = false;
   vector<LocationInfo>::iterator it;

   for(it = locations.begin(); it != locations.end(); it++) {
      if(*it == loc) {
         match = true;
         break;
      }
   }

   return match;
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
   grid_data_info = (VarInfo *) nullptr;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::clear() {

   // Initialize values
   find_grid_by_name("G004", grid);
   grid_data.clear();
   grid_plot_info.clear();
   point_opts.clear();
   do_colorbar = false;

   version.clear();

   // Delete allocated memory
   if(grid_data_info) { delete grid_data_info; grid_data_info = 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::read_config(const char *user_file_name) {
   ConcatString default_file_name =
                   replace_path(default_config_filename);
    
   // List the default config file
   mlog << Debug(1) << "Default Config File: "
        << default_file_name << "\n";

   // Read the config file constants and map data
   conf.read(replace_path(config_const_filename).c_str());
   conf.read(replace_path(config_map_data_filename).c_str());
   conf.read(replace_path(default_config_filename).c_str());

   // Read the user file name, if provided
   if(m_strlen(user_file_name) > 0) {
      mlog << Debug(1) << "User Config File: "
           << user_file_name << "\n";
      conf.read(user_file_name);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::process_config(
                              const char *plot_grid_string) {
   Dictionary  *dict = (Dictionary *) nullptr;
   Dictionary *fdict = (Dictionary *) nullptr;
   Dictionary i_fdict;
   StringArray sa;
   Met2dDataFileFactory m_factory;
   Met2dDataFile *met_ptr = (Met2dDataFile *) nullptr;
   PlotPointObsOpt opt;
   int i, n_vx;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: grid_data
   dict = conf.lookup_dictionary(conf_key_grid_data);

   // Conf: grid_data.file_type, if present
   GrdFileType ftype = parse_conf_file_type(dict);

   // Conf: field
   fdict = dict->lookup_array(conf_key_field);

   // Check length
   if((n_vx = parse_conf_n_vx(fdict)) > 1) {
      mlog << Error << "\nPlotPointObsConfInfo::process_config() -> "
           << "the \"" << conf_key_grid_data << "." << conf_key_field
           << "\" array can only have length 0 or 1.\n\n";
      exit(1);
   }

   // Parse plot_grid_string, if set
   if(m_strlen(plot_grid_string) > 0) {
      if (!build_grid_by_grid_string(plot_grid_string, grid,
                                     "PlotPointObsConfInfo::process_config -> ", false)) {
         // Extract the grid from a gridded data file
         mlog << Debug(3) << "Use the grid defined by file \""
              << plot_grid_string << "\".\n";

         // Open the data file
         if(!(met_ptr = m_factory.new_met_2d_data_file(
                           plot_grid_string, ftype))) {
            mlog << Error
                 << "\nPlotPointObsConfInfo::process_config() -> "
                 << "can't open file \"" << plot_grid_string
                 << "\"\n\n";
            exit(1);
         }

         // Store the grid
         grid = met_ptr->grid();
      }

      // Process gridded data
      if(n_vx > 0 && met_ptr) {

         // Allocate and set the VarInfo object
         VarInfoFactory v_factory;
         grid_data_info = v_factory.new_var_info(met_ptr->file_type());
         i_fdict = parse_conf_i_vx_dict(fdict, 0);
         grid_data_info->set_dict(i_fdict);

         // Get the requested data
         if(!met_ptr->data_plane(*grid_data_info, grid_data)) {
            mlog << Error
                 << "\nPlotPointObsConfInfo::process_config() -> "
                 << "trouble getting field \""
                 << grid_data_info->magic_str() << "\" from file \""
                 << plot_grid_string << "\"\n\n";
            exit(1);
         }

         // Regrid, if requested
         if(grid_data_info->regrid().enable) {
            mlog << Debug(1) << "Regridding field "
                 << grid_data_info->magic_str() << " using "
                 << grid_data_info->regrid().get_str() << ".\n";
            Grid to_grid(parse_vx_grid(grid_data_info->regrid(),
                                       &grid, &grid));
            grid_data = met_regrid(grid_data, grid, to_grid,
                                   grid_data_info->regrid());

            // Store the new grid definition
            grid = to_grid;
         }

         // Conf: grid_plot_info
         grid_plot_info = parse_conf_plot_info(
                             dict->lookup_dictionary(
                                conf_key_grid_plot_info));

         // Check for a colorbar
         if(grid_plot_info.colorbar_flag) do_colorbar = true;
      }

      // Cleanup
      if(met_ptr) { delete met_ptr; met_ptr = 0; }

   } // end if plot_grid_string

   // Conf: point_data
   dict = conf.lookup_array(conf_key_point_data);

   // Check length
   if(dict->n_entries() == 0) {
      mlog << Error << "\nPlotPointObsConfInfo::process_config() -> "
           << "the \"" << conf_key_point_data
           << "\" array is empty!\n\n";
      exit(1);
   }

   // Parse each array entry
   for(i=0; i<dict->n_entries(); i++) {
      opt.process_config(*((*dict)[i]->dict_value()));
      point_opts.push_back(opt);

      // Check for a colorbar
      if(opt.fill_plot_info.flag && opt.fill_plot_info.colorbar_flag) {
         do_colorbar = true;
      }
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::set_msg_typ(const StringArray &sa) {
   for(vector<PlotPointObsOpt>::iterator it = point_opts.begin();
       it != point_opts.end(); it++) {
      it->msg_typ = sa;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::set_obs_var(const StringArray &sa) {
   for(vector<PlotPointObsOpt>::iterator it = point_opts.begin();
       it != point_opts.end(); it++) {
      it->obs_var = sa;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::set_obs_gc(const IntArray &ia) {
   for(vector<PlotPointObsOpt>::iterator it = point_opts.begin();
       it != point_opts.end(); it++) {
      it->obs_gc = ia;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::set_dotsize(double s) {
   ConcatString cs;
   MetConfig config;

   cs << conf_key_dotsize << "(x) = " << s << ";";
   config.read_string(cs.c_str());

   const_dotsize_fx.set(config.lookup(conf_key_dotsize));

   for(vector<PlotPointObsOpt>::iterator it = point_opts.begin();
       it != point_opts.end(); it++) {
      it->dotsize_fx = const_dotsize_fx;
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

   return match;
}
    
////////////////////////////////////////////////////////////////////////
