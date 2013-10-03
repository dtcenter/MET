

///////////////////////////////////////////////////////////////////////


// ** Copyright UCAR (c) 1992 - 2013
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////


using namespace std;


///////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mode_exec.h"


///////////////////////////////////////////////////////////////////////


static const int unmatched_id = -1;

static const char * cts_str[n_cts] = {"RAW", "FILTER", "OBJECT"};

static const char program_name [] = "mode";

static const char * default_config_filename = "MET_BASE/data/config/MODEConfig_default";


///////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeExecutive
   //


///////////////////////////////////////////////////////////////////////


ModeExecutive::ModeExecutive()

{

init_from_scratch();

}


///////////////////////////////////////////////////////////////////////


ModeExecutive::~ModeExecutive()

{

clear();

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::init_from_scratch()

{

fcst_mtddf = (Met2dDataFile *) 0;
 obs_mtddf = (Met2dDataFile *) 0;

clear();

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::clear()

{

default_config_file.clear();
match_config_file.clear();
merge_config_file.clear();

fcst_file.clear();
obs_file.clear();

if ( fcst_mtddf )  { delete fcst_mtddf;  fcst_mtddf = (Met2dDataFile *) 0; }
if (  obs_mtddf )  { delete  obs_mtddf;   obs_mtddf = (Met2dDataFile *) 0; }

for (int j=0; j<n_cts; ++j)  cts[j].zero_out();

engine.clear_features();
engine.clear_colors();

grid.clear();

xy_bb.clear();

out_dir.clear();

data_min = data_max = 0.0;

   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::init()

{

GrdFileType ftype, otype;
Met2dDataFileFactory mtddf_factory;


   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // If the merge config file was not set, use the match config file
   if(merge_config_file.length() == 0)
        merge_config_file = match_config_file;
   
   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "Match Config File: "   << match_config_file   << "\n"
        << "Merge Config File: "   << merge_config_file   << "\n";

   // Read the config files
   engine.conf_info.read_config(default_config_file, match_config_file);


   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(engine.conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(engine.conf_info.conf.lookup_dictionary(conf_key_obs));

   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_file, ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_file << "\"\n\n";
      exit(1);
   }


   // Read observation file
   if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(obs_file, otype))) {
      mlog << Error << "\nTrouble reading observation file \""
           << obs_file << "\"\n\n";
      exit(1);
   }


   // Store the input data file types
   ftype = fcst_mtddf->file_type();
   otype = obs_mtddf->file_type();

   // Process the configuration
   engine.conf_info.process_config(ftype, otype);

   const int shift = engine.conf_info.shift_right;

   fcst_mtddf->set_shift_right(shift);
    obs_mtddf->set_shift_right(shift);

   // List the input files
   mlog << Debug(1)
        << "Forecast File: "    << fcst_file << "\n"
        << "Observation File: " << obs_file  << "\n";

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::setup_fcst_obs_data()

{

   // ShapeData fcst_sd, obs_sd;
   double fmin, omin, fmax, omax;

   Fcst_sd.clear();
    Obs_sd.clear();

      // Read the gridded data from the input forecast file

   if ( !(fcst_mtddf->data_plane(*(engine.conf_info.fcst_info), Fcst_sd.data)) )  {

      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "can't get data from file \"" << fcst_file << "\"\n\n";
      exit(1);
   }

      // For probability fields, rescale from [0, 100] to [0, 1]

   if ( engine.conf_info.fcst_info->p_flag() ) rescale_probability(Fcst_sd.data);

      // Read the gridded data from the input observation file

   if ( !(obs_mtddf->data_plane(*(engine.conf_info.obs_info), Obs_sd.data)) )  {

      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "can't get data from file \"" << obs_file << "\"\n\n";
      exit(1);
   }

      // For probability fields, rescale from [0, 100] to [0, 1]

   if ( engine.conf_info.obs_info->p_flag() ) rescale_probability(Obs_sd.data);

      // Check that the grids match

   if ( !(fcst_mtddf->grid() == obs_mtddf->grid()) )  {

      mlog << Error << "\nprocess_fcst_obs_files() -> "
           << "The forecast and observation grids do not match: "
           << fcst_mtddf->grid().serialize() << " != "
           << obs_mtddf->grid().serialize() << "\n\n";
      exit(1);
   }

   grid = fcst_mtddf->grid();

   // grid.dump(cout);

      // Print a warning if the valid times do not match

   if(Fcst_sd.data.valid() != Obs_sd.data.valid()) {

      mlog << Warning << "\nprocess_fcst_obs_files() -> "
           << "Forecast and observation valid times do not match "
           << unix_to_yyyymmdd_hhmmss(Fcst_sd.data.valid()) << " != "
           << unix_to_yyyymmdd_hhmmss(Obs_sd.data.valid()) << " for "
           << engine.conf_info.fcst_info->magic_str() << " versus "
           << engine.conf_info.obs_info->magic_str() << ".\n\n";
   }

      // Print a warning if the accumulation intervals do not match

   if(engine.conf_info.fcst_info->level().type() == LevelType_Accum &&
      engine.conf_info.obs_info->level().type()  == LevelType_Accum &&
      Fcst_sd.data.accum()                != Obs_sd.data.accum()) {

      mlog << Warning << "\nprocess_fcst_obs_files() -> "
           << "Forecast and observation accumulation times do not match "
           << sec_to_hhmmss(Fcst_sd.data.valid()) << " != "
           << sec_to_hhmmss(Obs_sd.data.valid()) << " for "
           << engine.conf_info.fcst_info->magic_str() << " versus "
           << engine.conf_info.obs_info->magic_str() << ".\n\n";
   }

   mlog << Debug(1)
        << "Forecast Field: "
        << engine.conf_info.fcst_info->name() << " at "
        << engine.conf_info.fcst_info->level_name()
        << "\n"
        << "Observation Field: "
        << engine.conf_info.obs_info->name() << " at "
        << engine.conf_info.obs_info->level_name()
        << "\n";

      // Mask out the missing data between fields

   if(engine.conf_info.mask_missing_flag == FieldType_Fcst ||
      engine.conf_info.mask_missing_flag == FieldType_Both)
      mask_bad_data(Fcst_sd.data, Obs_sd.data);

      // Mask out the missing data between fields   

   if(engine.conf_info.mask_missing_flag == FieldType_Obs ||
      engine.conf_info.mask_missing_flag == FieldType_Both)
      mask_bad_data(Obs_sd.data, Fcst_sd.data);

      // Parse the grid and/or polyline masks from the configuration

   process_masks(Fcst_sd, Obs_sd);

      // Compute the min and max data values across both raw fields

   Fcst_sd.data.data_range(fmin, fmax);
   Obs_sd.data.data_range(omin, omax);
   data_min = min(fmin, omin);
   data_max = max(fmax, omax);

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_conv_thresh()

{

      // Set up the engine with these raw fields

   mlog << Debug(2)
        << "Identifying objects in the forecast and observation fields...\n";
   engine.set(Fcst_sd, Obs_sd);

      // Compute the contingency table statistics for the fields

   if ( engine.conf_info.ct_stats_flag )  compute_ct_stats();

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_match_merge()

{

      // Do matching and merging

   if ( engine.conf_info.ps_plot_flag || engine.conf_info.nc_pairs_flag )  {

      mlog << Debug(2)
           << "Identified: " << engine.n_fcst << " forecast objects "
           << "and " << engine.n_obs << " observation objects.\n";

      mlog << Debug(2)
           << "Performing merging ("
           << mergetype_to_string(engine.conf_info.fcst_merge_flag)
           << ") in the forecast field.\n";

         // Do the forecast merging

      engine.do_fcst_merging(default_config_file, merge_config_file);

      mlog << Debug(2)
           << "Performing merging ("
           << mergetype_to_string(engine.conf_info.obs_merge_flag)
           << ") in the observation field.\n";

         // Do the observation merging

      engine.do_obs_merging(default_config_file, merge_config_file);

      mlog << Debug(2)
           << "Remaining: " << engine.n_fcst << " forecast objects "
           << "and " << engine.n_obs << " observation objects.\n";

      mlog << Debug(2)
           << "Performing matching ("
           << matchtype_to_string(engine.conf_info.match_flag)
           << ") between the forecast and observation fields.\n";

         // Do the matching of objects between fields

      engine.do_matching();

   }   //  if engine.conf_info.ps_plot_flag || engine.conf_info.nc_pairs_flag

return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_masks(ShapeData & fcst_sd, ShapeData & obs_sd)

{

   ShapeData grid_mask_sd, poly_mask_sd;
   ConcatString name;

   // Parse the grid mask into a ShapeData object
   if(engine.conf_info.mask_grid_flag != FieldType_None) {
      parse_grid_mask(engine.conf_info.mask_grid_name, grid,
                      grid_mask_sd.data, name);
   }

   // Parse the poly mask into a ShapeData object
   if(engine.conf_info.mask_poly_flag != FieldType_None) {
      parse_poly_mask(engine.conf_info.mask_poly_name, grid,
                      poly_mask_sd.data, name);
   }

   // Apply the grid mask to the forecast field if requested
   if(engine.conf_info.mask_grid_flag == FieldType_Fcst ||
      engine.conf_info.mask_grid_flag == FieldType_Both) {
      apply_mask(fcst_sd, grid_mask_sd);
   }

   // Apply the grid mask to the observation field if requested
   if(engine.conf_info.mask_grid_flag == FieldType_Obs ||
      engine.conf_info.mask_grid_flag == FieldType_Both) {
      apply_mask(obs_sd, grid_mask_sd);
   }

   // Apply the polyline mask to the forecast field if requested
   if(engine.conf_info.mask_poly_flag == FieldType_Fcst ||
      engine.conf_info.mask_poly_flag == FieldType_Both) {
      apply_mask(fcst_sd, poly_mask_sd);
   }

   // Apply the polyline mask to the observation field if requested
   if(engine.conf_info.mask_poly_flag == FieldType_Obs ||
      engine.conf_info.mask_poly_flag == FieldType_Both) {
      apply_mask(obs_sd, poly_mask_sd);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_output()

{

   // Create output stats files and plots

write_obj_stats();   

if ( engine.conf_info.ct_stats_flag )  write_ct_stats();

if ( engine.conf_info.nc_pairs_flag )  write_obj_netcdf();

if ( engine.conf_info.ps_plot_flag )   plot_engine();

return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::compute_ct_stats()

{

   int i, x, y;
   ShapeData fcst_mask, obs_mask;

   mlog << Debug(2)
        << "Computing contingency table statistics...\n";

   for(i=0; i<n_cts; i++) {

      cts[i].set_name(cts_str[i]);

      // Raw fields
      if(i == 0) {
         fcst_mask = *engine.fcst_raw;
         obs_mask  = *engine.obs_raw;

         // Apply the thresholds specified in the config file
         fcst_mask.threshold(engine.conf_info.fcst_conv_thresh);
         obs_mask.threshold(engine.conf_info.obs_conv_thresh);
      }
      // Filtered fields
      else if(i == 1) {
         fcst_mask = *engine.fcst_filter;
         obs_mask  = *engine.obs_filter;

         // Apply the thresholds specified in the config file
         fcst_mask.threshold(engine.conf_info.fcst_conv_thresh);
         obs_mask.threshold(engine.conf_info.obs_conv_thresh);
      }
      // Object fields
      else if(i == 2) {
         fcst_mask = *engine.fcst_mask;
         obs_mask  = *engine.obs_mask;
      }

      // Compute contingency table counts
      for(x=0; x<fcst_mask.data.nx(); x++) {
         for(y=0; y<fcst_mask.data.ny(); y++) {

            // Key off of the bad data values in the raw field
            if(engine.fcst_raw->is_bad_data(x, y) ||
               engine.obs_raw->is_bad_data(x, y)) continue;

            else if( fcst_mask.s_is_on(x, y) &&  obs_mask.s_is_on(x, y)) cts[i].inc_fy_oy();
            else if( fcst_mask.s_is_on(x, y) && !obs_mask.s_is_on(x, y)) cts[i].inc_fy_on();
            else if(!fcst_mask.s_is_on(x, y) &&  obs_mask.s_is_on(x, y)) cts[i].inc_fn_oy();
            else if(!fcst_mask.s_is_on(x, y) && !obs_mask.s_is_on(x, y)) cts[i].inc_fn_on();
         }
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::plot_engine()

{

ModePsFile plot;
ConcatString ps_file;

plot.set(engine, grid, data_min, data_max);

build_outfile_name(".ps", ps_file);

//
// List the image file as it is being created
//
mlog << Debug(1) << "Creating postscript file: " << ps_file << "\n";

plot.open(ps_file);

plot.make_plot();

return;

}

////////////////////////////////////////////////////////////////////////

void ModeExecutive::build_outfile_name(const char *suffix, ConcatString &str)

{

   int mon, day, yr, hr, min, sec;
   int l_hr, l_min, l_sec;
   int a_hr, a_min, a_sec;
   char tmp_str[max_str_len];

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(engine.conf_info.output_prefix.nonempty())
      str << "_" << engine.conf_info.output_prefix;

   // Append the timing information
   sec_to_hms(engine.fcst_raw->data.lead(), l_hr, l_min, l_sec);
   unix_to_mdyhms(engine.fcst_raw->data.valid(),
                  mon, day, yr, hr, min, sec);
   sec_to_hms(engine.fcst_raw->data.accum(), a_hr, a_min, a_sec);
   snprintf(tmp_str, sizeof(tmp_str), 
           "%.2i%.2i%.2iL_%.4i%.2i%.2i_%.2i%.2i%.2iV_%.2i%.2i%.2iA",
           l_hr, l_min, l_sec,
           yr, mon, day, hr, min, sec,
           a_hr, a_min, a_sec);
   str << "_" << tmp_str;

   // Append the suffix
   str << suffix;

   return;
}

//////////////////////////////////////////////////////////////////////

void ModeExecutive::write_obj_stats()

{

   AsciiTable obj_at, fcst_merge_at, obs_merge_at;
   ofstream out;
   ConcatString stat_file;

   //
   // Create output MODE object stats files
   //
   build_outfile_name("_obj.txt", stat_file);

   //
   // Open output stat file
   //
   out.open(stat_file);

   if(!out) {
      mlog << Error << "\nwrite_obj_stats() -> "
           << "unable to open stats output file \""
           << stat_file << "\"\n\n";
      exit(1);
   }
   out.setf(ios::fixed);

   //
   // List stat file as it is being created
   //
   mlog << Debug(1) << "Creating Fcst-Obs Object Statistics file: " << stat_file << "\n";

   //
   // Write the output statistics to an AsciiTable object
   //
   write_engine_stats(engine, grid, obj_at);

   //
   // Write the AsciiTable object to the output file
   //
   out << obj_at;

   out.close();

   if(engine.conf_info.fcst_merge_flag == MergeType_Both ||
      engine.conf_info.fcst_merge_flag == MergeType_Engine) {

      //
      // Create output stats file for forecast merging
      //
      build_outfile_name("_fcst_merge.txt", stat_file);
      out.open(stat_file);

      if(!out) {
         mlog << Error << "\nwrite_obj_stats() -> "
              << "unable to open stats output file \""
              << stat_file << "\"\n\n";
         exit(1);
      }
      out.setf(ios::fixed);

      mlog << Debug(1) << "Creating Fcst-Fcst Object Statistics file: " << stat_file << "\n";

      //
      // Write the output statistics to an AsciiTable object
      //
      write_engine_stats(*engine.fcst_engine, grid, fcst_merge_at);

      //
      // Write the AsciiTable object to the output file
      //
      out << fcst_merge_at;

      out.close();
   }

   if(engine.conf_info.obs_merge_flag == MergeType_Both ||
      engine.conf_info.obs_merge_flag == MergeType_Engine) {

      //
      // Create output stats file for obseravation merging
      //
      build_outfile_name("_obs_merge.txt", stat_file);
      out.open(stat_file);

      if(!out) {
         mlog << Error << "\nwrite_obj_stats() -> "
              << "unable to open stats output file \""
              << stat_file << "\"\n\n";
         exit(1);
      }
      out.setf(ios::fixed);

      mlog << Debug(1) << "Creating Obs-Obs Object Statistics file: " << stat_file << "\n";

      //
      // Write the output statistics to an AsciiTable object
      //
      write_engine_stats(*engine.obs_engine, grid, obs_merge_at);

      //
      // Write the AsciiTable object to the output file
      //
      out << obs_merge_at;

      out.close();
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_obj_netcdf()

{

   int n, x, y;
   ConcatString out_file;

   float *fcst_raw_data      = (float *) 0;
   float *fcst_obj_raw_data  = (float *) 0;
   int   *fcst_obj_data      = (int *)   0;
   int   *fcst_clus_data     = (int *)   0;

   float *obs_raw_data       = (float *) 0;
   float *obs_obj_raw_data   = (float *) 0;
   int   *obs_obj_data       = (int *)   0;
   int   *obs_clus_data      = (int *)   0;

   NcFile *f_out             = (NcFile *) 0;

   NcDim  *lat_dim           = (NcDim *)  0;
   NcDim  *lon_dim           = (NcDim *)  0;

   NcVar  *fcst_raw_var      = (NcVar *)  0;
   NcVar  *fcst_obj_raw_var  = (NcVar *)  0;
   NcVar  *fcst_obj_var      = (NcVar *)  0;
   NcVar  *fcst_clus_var     = (NcVar *)  0;

   NcVar  *obs_raw_var       = (NcVar *)  0;
   NcVar  *obs_obj_raw_var   = (NcVar *)  0;
   NcVar  *obs_obj_var       = (NcVar *)  0;
   NcVar  *obs_clus_var      = (NcVar *)  0;

   //
   // Create output NetCDF file name
   //
   build_outfile_name("_obj.nc", out_file);

   mlog << Debug(1) << "Creating Object NetCDF file: " << out_file << "\n";

   //
   // Create a new NetCDF file and open it
   // NOTE: must multiply longitudes throughout by -1 to convert from
   // degrees_west to degree_east
   //
   f_out = new NcFile(out_file, NcFile::Replace);

   if(!f_out->is_valid()) {
      mlog << Error << "\nwrite_obj_netcdf() -> trouble opening output file "
           << out_file << "\n\n";
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_file.text(), program_name);

   // Add the projection information
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);

   // Define Variables
   fcst_raw_var     = f_out->add_var("fcst_raw",     ncFloat, lat_dim, lon_dim);
   fcst_obj_raw_var = f_out->add_var("fcst_obj_raw", ncFloat, lat_dim, lon_dim);
   fcst_obj_var     = f_out->add_var("fcst_obj_id",  ncInt,   lat_dim, lon_dim);
   fcst_clus_var    = f_out->add_var("fcst_clus_id", ncInt,   lat_dim, lon_dim);

   obs_raw_var      = f_out->add_var("obs_raw",     ncFloat, lat_dim, lon_dim);
   obs_obj_raw_var  = f_out->add_var("obs_obj_raw", ncFloat, lat_dim, lon_dim);
   obs_obj_var      = f_out->add_var("obs_obj_id",  ncInt,   lat_dim, lon_dim);
   obs_clus_var     = f_out->add_var("obs_clus_id", ncInt,   lat_dim, lon_dim);
   
   // Add forecast variable attributes

   fcst_raw_var->add_att("long_name", "Forecast Raw Values");
   write_netcdf_var_times(fcst_raw_var, engine.fcst_raw->data);
   fcst_raw_var->add_att("_FillValue", bad_data_float);

   fcst_obj_raw_var->add_att("long_name", "Forecast Object Raw Values");
   write_netcdf_var_times(fcst_obj_raw_var, engine.fcst_raw->data);
   fcst_obj_raw_var->add_att("_FillValue", bad_data_float);

   fcst_obj_var->add_att("long_name", "Forecast Object ID");
   write_netcdf_var_times(fcst_obj_var, engine.fcst_raw->data);
   fcst_obj_var->add_att("_FillValue", bad_data_int);

   fcst_clus_var->add_att("long_name", "Forecast Cluster Object ID");
   write_netcdf_var_times(fcst_clus_var, engine.fcst_raw->data);
   fcst_clus_var->add_att("_FillValue", bad_data_int);

   // Add observation variable attributes

   obs_raw_var->add_att("long_name", "Observation Raw Values");
   write_netcdf_var_times(obs_raw_var, engine.obs_raw->data);
   obs_raw_var->add_att("_FillValue", bad_data_float);

   obs_obj_raw_var->add_att("long_name", "Observation Object Raw Values");
   write_netcdf_var_times(obs_obj_raw_var, engine.obs_raw->data);
   obs_obj_raw_var->add_att("_FillValue", bad_data_float);

   obs_obj_var->add_att("long_name", "Observation Object ID");
   write_netcdf_var_times(obs_obj_var, engine.obs_raw->data);
   obs_obj_var->add_att("_FillValue", bad_data_int);

   obs_clus_var->add_att("long_name", "Observation Cluster Object ID");
   write_netcdf_var_times(obs_clus_var, engine.obs_raw->data);
   obs_clus_var->add_att("_FillValue", bad_data_int);

   //
   // Allocate memory for the raw values and object ID's for each grid box
   //

   fcst_raw_data      = new float [grid.nx()*grid.ny()];
   fcst_obj_raw_data  = new float [grid.nx()*grid.ny()];
   fcst_obj_data      = new int   [grid.nx()*grid.ny()];
   fcst_clus_data     = new int   [grid.nx()*grid.ny()];

   obs_raw_data       = new float [grid.nx()*grid.ny()];
   obs_obj_raw_data   = new float [grid.nx()*grid.ny()];
   obs_obj_data       = new int   [grid.nx()*grid.ny()];
   obs_clus_data      = new int   [grid.nx()*grid.ny()];

   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

         //
         // Get raw values and object ID's for each grid box
         //

         fcst_raw_data[n] = engine.fcst_raw->data (x, y);
          obs_raw_data[n] = engine.obs_raw->data  (x, y);

         if(engine.fcst_split->is_nonzero(x, y) ) {
            fcst_obj_raw_data[n] = engine.fcst_raw->data(x, y);
            fcst_obj_data[n] = nint(engine.fcst_split->data(x, y));
         }
         else {
            fcst_obj_raw_data[n] = bad_data_float;
            fcst_obj_data[n] = bad_data_int;
         }

         if(engine.obs_split->is_nonzero(x, y) ) {
            obs_obj_raw_data[n] = engine.obs_raw->data(x, y);
            obs_obj_data[n] = nint(engine.obs_split->data(x, y));
         }
         else {
            obs_obj_raw_data[n] = bad_data_float;
            obs_obj_data[n] = bad_data_int;
         }

         //
         // Get cluster object ID's for each grid box
         //

         // Write the index of the cluster object
         if(engine.fcst_clus_split->data(x, y) > 0) {
            fcst_clus_data[n] = nint(engine.fcst_clus_split->data(x, y));
         }
         // Write a value of 0 for unmatched simple objects
         else if(engine.fcst_split->data(x, y) > 0) {
            fcst_clus_data[n] = unmatched_id;
         }
         // Otherwise, write bad data
         else {
            fcst_clus_data[n] = bad_data_int;
         }

         // Write the index of the cluster object
         if(engine.obs_clus_split->data(x, y) > 0) {
            obs_clus_data[n] = nint(engine.obs_clus_split->data(x, y));
         }
         // Write a value of 0 for unmatched simple objects
         else if(engine.obs_split->data(x, y) > 0) {
            obs_clus_data[n] = unmatched_id;
         }
         // Otherwise, write bad data
         else {
            obs_clus_data[n] = bad_data_int;
         }
      }
   }

   //
   // Write the forecast and observation raw value variables
   //

   if( !fcst_raw_var->put(&fcst_raw_data[0], grid.ny(), grid.nx()) ||
       !obs_raw_var->put(&obs_raw_data[0], grid.ny(), grid.nx()) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the fcst_raw_var->put or obs_raw_var->put\n\n";
      exit(1);
   }

   if( !fcst_obj_raw_var->put(&fcst_obj_raw_data[0], grid.ny(), grid.nx()) ||
       !obs_obj_raw_var->put(&obs_obj_raw_data[0], grid.ny(), grid.nx()) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the fcst_obj_raw_var->put or obs_obj_raw_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast and observation object ID variables
   //
   if( !fcst_obj_var->put(&fcst_obj_data[0], grid.ny(), grid.nx()) ||
       !obs_obj_var->put(&obs_obj_data[0], grid.ny(), grid.nx()) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the fcst_obj_var->put or obs_obj_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast and observation cluster object ID variables
   //
   if( !fcst_clus_var->put(&fcst_clus_data[0], grid.ny(), grid.nx()) ||
       !obs_clus_var->put(&obs_clus_data[0], grid.ny(), grid.nx()) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the fcst_clus_var->put or obs_clus_var->put\n\n";
      exit(1);
   }

   //
   // Delete allocated memory
   //

   if (fcst_raw_data)      { delete fcst_raw_data;      fcst_raw_data     = (float *) 0; }
   if (fcst_obj_raw_data)  { delete fcst_obj_raw_data;  fcst_obj_raw_data = (float *) 0; }
   if (fcst_obj_data)      { delete fcst_obj_data;      fcst_obj_data     = (int *) 0; }
   if (fcst_clus_data)     { delete fcst_clus_data;     fcst_clus_data    = (int *) 0; }

   if (obs_raw_data)       { delete obs_raw_data;       obs_raw_data      = (float *) 0; }
   if (obs_obj_raw_data)   { delete obs_obj_raw_data;   obs_obj_raw_data  = (float *) 0; }
   if (obs_obj_data)       { delete obs_obj_data;       obs_obj_data      = (int *) 0; }
   if (obs_clus_data)      { delete obs_clus_data;      obs_clus_data     = (int *) 0; }

   //
   // Write out the values of the vertices of the boundary polylines for
   // the simple objects.
   //
   write_bdy_netcdf(f_out);

   //
   // Close the NetCDF file
   //
   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}


///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_bdy_netcdf(NcFile * f_out)

{

   //
   // Write out the number of forecast and observation objects
   //
   NcVar *n_fcst_obj_var = (NcVar *)  0;
   NcVar *n_obs_obj_var  = (NcVar *)  0;

   // Define scalar variables
   n_fcst_obj_var = f_out->add_var("n_fcst_obj", ncInt);
   n_obs_obj_var  = f_out->add_var("n_obs_obj",  ncInt);

   //
   // Write the number of forecast and observation objects
   //
   if( !n_fcst_obj_var->put(&engine.n_fcst) ||
       !n_obs_obj_var->put(&engine.n_obs) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the n_fcst_obj_var-> or "
           << "n_obs_obj_var->put\n\n";
      exit(1);
   }

   //
   // Only write out the boundary polyline points if there are objects
   // present.
   //
   if(engine.n_fcst > 0) write_fcst_bdy_netcdf(f_out);
   if(engine.n_obs  > 0) write_obs_bdy_netcdf(f_out);

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_fcst_bdy_netcdf(NcFile *f_out)

{

   int i, j, x, y, n_pts;
   double lat, lon;

   int   *bdy_start          = (int   *) 0;
   int   *bdy_npts           = (int   *) 0;
   float *bdy_lat            = (float *) 0;
   float *bdy_lon            = (float *) 0;
   int   *bdy_x              = (int   *) 0;
   int   *bdy_y              = (int   *) 0;

   // Dimensions and variables for each object
   NcDim  *obj_dim           = (NcDim *)  0;
   NcVar  *obj_bdy_start_var = (NcVar *)  0;
   NcVar  *obj_bdy_npts_var  = (NcVar *)  0;

   // Dimensions and variables for each boundary point
   NcDim  *bdy_dim           = (NcDim *)  0;
   NcVar  *bdy_lat_var       = (NcVar *)  0;
   NcVar  *bdy_lon_var       = (NcVar *)  0;
   NcVar  *bdy_x_var         = (NcVar *)  0;
   NcVar  *bdy_y_var         = (NcVar *)  0;

   // Get the number of forecast boundary polyline points
   for(i=0, n_pts=0; i<engine.n_fcst; i++)
      n_pts += engine.fcst_single[i].boundary[0].n_points;

   // Define dimensions
   obj_dim = f_out->add_dim("fcst_obj", (long) engine.n_fcst);
   bdy_dim = f_out->add_dim("fcst_bdy", (long) n_pts);

   // Define variables
   obj_bdy_start_var = f_out->add_var("fcst_obj_bdy_start", ncInt, obj_dim);
   obj_bdy_npts_var  = f_out->add_var("fcst_obj_bdy_npts", ncInt, obj_dim);
   bdy_lat_var       = f_out->add_var("fcst_bdy_lat", ncFloat, bdy_dim);
   bdy_lon_var       = f_out->add_var("fcst_bdy_lon", ncFloat, bdy_dim);
   bdy_x_var         = f_out->add_var("fcst_bdy_x", ncInt, bdy_dim);
   bdy_y_var         = f_out->add_var("fcst_bdy_y", ncInt, bdy_dim);

   // Add variable attributes
   obj_bdy_start_var->add_att("long_name", "Forecast Boundary Starting Index");
   obj_bdy_npts_var->add_att("long_name", "Number of Forecast Boundary Points");
   bdy_lat_var->add_att("long_name", "Forecast Boundary Point Latitude");
   bdy_lat_var->add_att("units", "degrees_north");
   bdy_lon_var->add_att("long_name", "Forecast Boundary Point Longitude");
   bdy_lon_var->add_att("units", "degrees_east");
   bdy_x_var->add_att("long_name", "Forecast Boundary Point X-Coordinate");
   bdy_y_var->add_att("long_name", "Forecast Boundary Point Y-Coordinate");

   //
   // Allocate memory for the forecast boundary points
   //
   bdy_start = new int   [engine.n_fcst];
   bdy_npts  = new int   [engine.n_fcst];
   bdy_lat   = new float [n_pts];
   bdy_lon   = new float [n_pts];
   bdy_x     = new int   [n_pts];
   bdy_y     = new int   [n_pts];

   //
   // Process each forecast object and store the boundary
   // polyline points.
   //
   for(i=0, n_pts=0; i<engine.n_fcst; i++) {

      // Store the starting point for this object.
      bdy_start[i] = n_pts;

      // Store the number of points in this polyline.
      bdy_npts[i] = engine.fcst_single[i].boundary[0].n_points;

      for(j=0; j<bdy_npts[i]; j++, n_pts++) {

         // Get the boundary point (x,y) coordinates and store them
         x = nint(engine.fcst_single[i].boundary[0].u[j]);
         y = nint(engine.fcst_single[i].boundary[0].v[j]);
         bdy_x[n_pts] = x;
         bdy_y[n_pts] = y;

         // Convert to lat/lon and store them
         grid.xy_to_latlon(x, y, lat, lon);
         bdy_lat[n_pts] = lat;
         bdy_lon[n_pts] = -1.0*lon;
      }
   }

   //
   // Write the forecast boundary polyline information
   //
   if( !obj_bdy_start_var->put(&bdy_start[0], engine.n_fcst) ||
       !obj_bdy_npts_var->put(&bdy_npts[0], engine.n_fcst) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the obj_bdy_start_var->put or "
           << "obj_bdy_npts_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary lat/lon points
   //
   if( !bdy_lat_var->put(&bdy_lat[0], n_pts) ||
       !bdy_lon_var->put(&bdy_lon[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_lat_var->put "
           << "or bdy_lon_var->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary (x,y) points
   //
   if( !bdy_x_var->put(&bdy_x[0], n_pts) ||
       !bdy_y_var->put(&bdy_y[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_x_var->put "
           << "or bdy_y_var->put\n\n";
      exit(1);
   }

   //
   // Delete allocated memory
   //
   if(bdy_start) { delete bdy_start; bdy_start = (int   *) 0; }
   if(bdy_npts)  { delete bdy_npts;  bdy_npts  = (int   *) 0; }
   if(bdy_lat)   { delete bdy_lat;   bdy_lat   = (float *) 0; }
   if(bdy_lon)   { delete bdy_lon;   bdy_lon   = (float *) 0; }
   if(bdy_x)     { delete bdy_x;     bdy_x     = (int   *) 0; }
   if(bdy_y)     { delete bdy_y;     bdy_y     = (int   *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_obs_bdy_netcdf(NcFile *f_out)

{

   int i, j, x, y, n_pts;
   double lat, lon;

   int   *bdy_start          = (int   *) 0;
   int   *bdy_npts           = (int   *) 0;
   float *bdy_lat            = (float *) 0;
   float *bdy_lon            = (float *) 0;
   int   *bdy_x              = (int   *) 0;
   int   *bdy_y              = (int   *) 0;

   // Dimensions and variables for each object
   NcDim  *obj_dim           = (NcDim *)  0;
   NcVar  *obj_bdy_start_var = (NcVar *)  0;
   NcVar  *obj_bdy_npts_var  = (NcVar *)  0;

   // Dimensions and variables for each boundary point
   NcDim  *bdy_dim           = (NcDim *)  0;
   NcVar  *bdy_lat_var       = (NcVar *)  0;
   NcVar  *bdy_lon_var       = (NcVar *)  0;
   NcVar  *bdy_x_var         = (NcVar *)  0;
   NcVar  *bdy_y_var         = (NcVar *)  0;

   // Get the number of observation boundary polyline points
   for(i=0, n_pts=0; i<engine.n_obs; i++)
      n_pts += engine.obs_single[i].boundary[0].n_points;

   // Define dimensions
   obj_dim = f_out->add_dim("obs_obj", (long) engine.n_obs);
   bdy_dim = f_out->add_dim("obs_bdy", (long) n_pts);

   // Define variables
   obj_bdy_start_var = f_out->add_var("obs_obj_bdy_start", ncInt, obj_dim);
   obj_bdy_npts_var  = f_out->add_var("obs_obj_bdy_npts", ncInt, obj_dim);
   bdy_lat_var       = f_out->add_var("obs_bdy_lat", ncFloat, bdy_dim);
   bdy_lon_var       = f_out->add_var("obs_bdy_lon", ncFloat, bdy_dim);
   bdy_x_var         = f_out->add_var("obs_bdy_x", ncInt, bdy_dim);
   bdy_y_var         = f_out->add_var("obs_bdy_y", ncInt, bdy_dim);

   // Add variable attributes
   obj_bdy_start_var->add_att("long_name", "Observation Boundary Starting Index");
   obj_bdy_npts_var->add_att("long_name", "Number of Observation Boundary Points");
   bdy_lat_var->add_att("long_name", "Observation Boundary Point Latitude");
   bdy_lat_var->add_att("units", "degrees_north");
   bdy_lon_var->add_att("long_name", "Observation Boundary Point Longitude");
   bdy_lon_var->add_att("units", "degrees_east");
   bdy_x_var->add_att("long_name", "Observation Boundary Point X-Coordinate");
   bdy_y_var->add_att("long_name", "Observation Boundary Point Y-Coordinate");

   //
   // Allocate memory for the observation boundary points
   //
   bdy_start = new int   [engine.n_obs];
   bdy_npts  = new int   [engine.n_obs];
   bdy_lat   = new float [n_pts];
   bdy_lon   = new float [n_pts];
   bdy_x     = new int   [n_pts];
   bdy_y     = new int   [n_pts];

   //
   // Process each observation object and store the boundary
   // polyline points.
   //
   for(i=0, n_pts=0; i<engine.n_obs; i++) {

      // Store the starting point for this object.
      bdy_start[i] = n_pts;

      // Store the number of points in this polyline.
      bdy_npts[i] = engine.obs_single[i].boundary[0].n_points;

      for(j=0; j<bdy_npts[i]; j++, n_pts++) {

         // Get the boundary point (x,y) coordinates and store them
         x = nint(engine.obs_single[i].boundary[0].u[j]);
         y = nint(engine.obs_single[i].boundary[0].v[j]);
         bdy_x[n_pts] = x;
         bdy_y[n_pts] = y;

         // Convert to lat/lon and store them
         grid.xy_to_latlon(x, y, lat, lon);
         bdy_lat[n_pts] = lat;
         bdy_lon[n_pts] = -1.0*lon;
      }
   }

   //
   // Write the observation boundary polyline information
   //
   if( !obj_bdy_start_var->put(&bdy_start[0], engine.n_obs) ||
       !obj_bdy_npts_var->put(&bdy_npts[0], engine.n_obs) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the obj_bdy_start_var->put or "
           << "obj_bdy_npts_var->put\n\n";
      exit(1);
   }

   //
   // Write the observation boundary lat/lon points
   //
   if( !bdy_lat_var->put(&bdy_lat[0], n_pts) ||
       !bdy_lon_var->put(&bdy_lon[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_lat_var->put "
           << "or bdy_lon_var->put\n\n";
      exit(1);
   }

   //
   // Write the observation boundary (x,y) points
   //
   if( !bdy_x_var->put(&bdy_x[0], n_pts) ||
       !bdy_y_var->put(&bdy_y[0], n_pts) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with bdy_x_var->put "
           << "or bdy_y_var->put\n\n";
      exit(1);
   }

   //
   // Delete allocated memory
   //
   if(bdy_start) { delete bdy_start; bdy_start = (int   *) 0; }
   if(bdy_npts)  { delete bdy_npts;  bdy_npts  = (int   *) 0; }
   if(bdy_lat)   { delete bdy_lat;   bdy_lat   = (float *) 0; }
   if(bdy_lon)   { delete bdy_lon;   bdy_lon   = (float *) 0; }
   if(bdy_x)     { delete bdy_x;     bdy_x     = (int   *) 0; }
   if(bdy_y)     { delete bdy_y;     bdy_y     = (int   *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_ct_stats()

{

   AsciiTable cts_at;
   ofstream out;
   int i;
   double v;
   ConcatString stat_file;

   //
   // Create output contingency table stat file name
   //
   build_outfile_name("_cts.txt", stat_file);

   //
   // Open output stat file
   //
   out.open(stat_file);

   if(!out) {
      mlog << Error << "\nwrite_ct_stats() -> "
           << "unable to open stats output file \""
           << stat_file << "\"\n\n";
      exit(1);
   }
   out.setf(ios::fixed);

   //
   // List stat file as it is being created
   //
   mlog << Debug(1) << "Creating Contingency Table Statistics file: " << stat_file << "\n";

   //
   // Setup the AsciiTable to be used
   //
   cts_at.clear();
   i = n_mode_hdr_columns + n_mode_cts_columns;
   cts_at.set_size(4, i);                      // Set table size
   cts_at.set_table_just(LeftJust);            // Left-justify columns
   cts_at.set_precision(default_precision);    // Set the precision
   cts_at.set_bad_data_value(bad_data_double); // Set the bad data value
   cts_at.set_bad_data_str(na_str);            // Set the bad data string
   cts_at.set_delete_trailing_blank_rows(1);   // No trailing blank rows

   //
   // Write out the MODE header columns
   //
   for(i=0; i<n_mode_hdr_columns; i++) {
      cts_at.set_entry(0, i, mode_hdr_columns[i]);
   }

   //
   // Write out the MODE contingecy table header columns
   //
   for(i=0; i<n_mode_cts_columns; i++) {
      cts_at.set_entry(0, i + n_mode_hdr_columns, mode_cts_columns[i]);
   }

   //
   // Store the contingency table counts and statistics in the AsciiTable
   // object.
   //
   for(i=0; i<n_cts; i++) {

      // Write out the header columns
      write_header_columns(engine, cts_at, i+1);

      // Field
      cts_at.set_entry(i+1, mode_field_offset, cts[i].name());

      // Total
      cts_at.set_entry(i+1, mode_total_offset, cts[i].n());

      // FY_OY
      cts_at.set_entry(i+1, mode_fy_oy_offset, cts[i].fy_oy());

      // FY_ON
      cts_at.set_entry(i+1, mode_fy_on_offset, cts[i].fy_on());

      // FN_OY
      cts_at.set_entry(i+1, mode_fn_oy_offset, cts[i].fn_oy());

      // FN_ON
      cts_at.set_entry(i+1, mode_fn_on_offset, cts[i].fn_on());

      // Base Rate
      v = cts[i].oy_tp();
      cts_at.set_entry(i+1, mode_baser_offset, v);

      // Forecast Mean
      v = cts[i].fy_tp();
      cts_at.set_entry(i+1, mode_fmean_offset, v);

      // Accuracy
      v = cts[i].accuracy();
      cts_at.set_entry(i+1, mode_acc_offset, v);

      // Forecast Bias
      v = cts[i].fbias();
      cts_at.set_entry(i+1, mode_fbias_offset, v);

      // PODY
      v = cts[i].pod_yes();
      cts_at.set_entry(i+1, mode_pody_offset, v);

      // PODN
      v = cts[i].pod_no();
      cts_at.set_entry(i+1, mode_podn_offset, v);

      // POFD
      v = cts[i].pofd();
      cts_at.set_entry(i+1, mode_pofd_offset, v);

      // FAR
      v = cts[i].far();
      cts_at.set_entry(i+1, mode_far_offset, v);

      // CSI
      v = cts[i].csi();
      cts_at.set_entry(i+1, mode_csi_offset, v);

      // GSS
      v = cts[i].gss();
      cts_at.set_entry(i+1, mode_gss_offset, v);

      // HK
      v = cts[i].hk();
      cts_at.set_entry(i+1, mode_hk_offset, v);

      // HSS
      v = cts[i].hss();
      cts_at.set_entry(i+1, mode_hss_offset, v);

      // ODDS
      v = cts[i].odds();
      cts_at.set_entry(i+1, mode_odds_offset, v);
   }

   //
   // Write the AsciiTable object to the output file
   //
   out << cts_at;

   out.close();

   return;
}

///////////////////////////////////////////////////////////////////////


