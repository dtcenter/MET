

///////////////////////////////////////////////////////////////////////


// ** Copyright UCAR (c) 1992 - 2019
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
#include "nc_utils.h"
#include "vx_regrid.h"


///////////////////////////////////////////////////////////////////////


static const int unmatched_id = -1;

static const char * cts_str[n_cts] = {"RAW", "OBJECT"};

static const char program_name [] = "mode";

static const char * default_config_filename = "MET_BASE/config/MODEConfig_default";


///////////////////////////////////////////////////////////////////////


static void nc_add_string(NcFile *, const char * text, const char * var_name, const char * dim_name);


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

R_index = T_index = 0;


   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::init()

{

GrdFileType ftype, otype;
Met2dDataFileFactory mtddf_factory;

R_index = T_index = 0;

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
   engine.conf_info.read_config(default_config_file.c_str(), match_config_file.c_str());

   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(engine.conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(engine.conf_info.conf.lookup_dictionary(conf_key_obs));


   // Read observation file
   if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(obs_file.c_str(), otype))) {
      mlog << Error << "\nTrouble reading observation file \""
           << obs_file << "\"\n\n";
      exit(1);
   }


   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_file.c_str(), ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_file << "\"\n\n";
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

   engine.conf_info.nc_info.compress_level = engine.conf_info.get_compression_level();

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

      mlog << Error << "\nsetup_fcst_obs_data() -> "
           << "can't get forecast data \""
           << engine.conf_info.fcst_info->magic_str()
           << "\" from file \"" << fcst_file << "\"\n\n";
      exit(1);
   }

   // Read the gridded data from the input observation file

   if ( !(obs_mtddf->data_plane(*(engine.conf_info.obs_info), Obs_sd.data)) )  {

      mlog << Error << "\nsetup_fcst_obs_data() -> "
           << "can't get observation data \""
           << engine.conf_info.obs_info->magic_str()
           << "\" from file \"" << obs_file << "\"\n\n";
      exit(1);
   }

      // Determine the verification grid

   grid = parse_vx_grid(engine.conf_info.fcst_info->regrid(),
                        &(fcst_mtddf->grid()), &(obs_mtddf->grid()));

      // Store the grid

   engine.set_grid(&grid);

      // Regrid, if necessary

   if ( !(fcst_mtddf->grid() == grid) )  {
      mlog << Debug(1)
           << "Regridding forecast " << engine.conf_info.fcst_info->magic_str()
           << " to the verification grid.\n";
      Fcst_sd.data = met_regrid(Fcst_sd.data, fcst_mtddf->grid(), grid,
                                engine.conf_info.fcst_info->regrid());
   }

      // Regrid, if necessary

   if ( !(obs_mtddf->grid() == grid) )  {
      mlog << Debug(1)
           << "Regridding observation " << engine.conf_info.obs_info->magic_str()
           << " to the verification grid.\n";
      Obs_sd.data = met_regrid(Obs_sd.data, obs_mtddf->grid(), grid,
                               engine.conf_info.obs_info->regrid());
   }

      // Rescale probabilites from [0, 100] to [0, 1]

   if ( engine.conf_info.fcst_info->p_flag() ) rescale_probability(Fcst_sd.data);

      // Rescale probabilites from [0, 100] to [0, 1]

   if ( engine.conf_info.obs_info->p_flag() ) rescale_probability(Obs_sd.data);

      // Print a warning if the valid times do not match

   if(Fcst_sd.data.valid() != Obs_sd.data.valid()) {

      mlog << Warning << "\nsetup_fcst_obs_data() -> "
           << "Forecast and observation valid times do not match "
           << unix_to_yyyymmdd_hhmmss(Fcst_sd.data.valid()) << " != "
           << unix_to_yyyymmdd_hhmmss(Obs_sd.data.valid()) << " for "
           << engine.conf_info.fcst_info->magic_str() << " versus "
           << engine.conf_info.obs_info->magic_str() << ".\n\n";
   }

      // Print a warning if the accumulation intervals do not match

   if(engine.conf_info.fcst_info->level().type() == LevelType_Accum &&
      engine.conf_info.obs_info->level().type()  == LevelType_Accum &&
      Fcst_sd.data.accum()                       != Obs_sd.data.accum()) {

      mlog << Warning << "\nsetup_fcst_obs_data() -> "
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
   if     (!is_bad_data(fmin) && !is_bad_data(omin)) data_min = min(fmin, omin);
   else if(!is_bad_data(fmin) &&  is_bad_data(omin)) data_min = fmin;
   else if( is_bad_data(fmin) && !is_bad_data(omin)) data_min = omin;

   if     (!is_bad_data(fmax) && !is_bad_data(omax)) data_max = max(fmax, omax);
   else if(!is_bad_data(fmax) &&  is_bad_data(omax)) data_max = fmax;
   else if( is_bad_data(fmax) && !is_bad_data(omax)) data_max = omax;

      // Process percentile thresholds

   engine.conf_info.set_perc_thresh(Fcst_sd.data, Obs_sd.data);

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_conv_thresh(const int r_index, const int t_index)

{

ModeConfInfo & conf = engine.conf_info;

static int local_r_index = -1;

R_index = r_index;
T_index = t_index;

conf.set_conv_radius_by_index  (R_index);
conf.set_conv_thresh_by_index  (T_index);

if ( conf.need_fcst_merge_thresh () )  conf.set_fcst_merge_thresh_by_index (T_index);
if ( conf.need_obs_merge_thresh  () )  conf.set_obs_merge_thresh_by_index  (T_index);

   //
   //  Set up the engine with these raw fields
   //

mlog << Debug(2)
     << "Identifying objects in the forecast and observation fields...\n";

if ( r_index != local_r_index )  {   //  need to do convolution

   engine.set(Fcst_sd, Obs_sd);

} else {   //  don't need to do convolution

   engine.set_no_conv(Fcst_sd, Obs_sd);

}

   //
   //  Compute the contingency table statistics for the fields, if needed
   //

if ( conf.ct_stats_flag )  compute_ct_stats();


   //
   //  done
   //

local_r_index = r_index;

return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_match_merge()

{

   mlog << Debug(2)
        << "Identified: " << engine.n_fcst << " forecast objects "
        << "and " << engine.n_obs << " observation objects.\n";

   mlog << Debug(2)
        << "Performing merging ("
        << mergetype_to_string(engine.conf_info.fcst_merge_flag)
        << ") in the forecast field.\n";

   // Do the forecast merging

   engine.do_fcst_merging(default_config_file.c_str(), merge_config_file.c_str());

   mlog << Debug(2)
        << "Performing merging ("
        << mergetype_to_string(engine.conf_info.obs_merge_flag)
        << ") in the observation field.\n";

   // Do the observation merging

   engine.do_obs_merging(default_config_file.c_str(), merge_config_file.c_str());

   mlog << Debug(2)
        << "Remaining: " << engine.n_fcst << " forecast objects "
        << "and " << engine.n_obs << " observation objects.\n";

   mlog << Debug(2)
        << "Performing matching ("
        << matchtype_to_string(engine.conf_info.match_flag)
        << ") between the forecast and observation fields.\n";

   // Do the matching of objects between fields

   engine.do_matching();

return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_masks(ShapeData & fcst_sd, ShapeData & obs_sd)

{

   ShapeData grid_mask_sd, poly_mask_sd;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Parse the grid mask into a ShapeData object
   if(engine.conf_info.mask_grid_flag != FieldType_None) {
      mlog << Debug(3)
           << "Processing grid mask: "
           << engine.conf_info.mask_grid_name << "\n";
      parse_grid_mask(engine.conf_info.mask_grid_name, grid,
                      grid_mask_sd.data, name);
   }

   // Parse the poly mask into a ShapeData object
   if(engine.conf_info.mask_poly_flag != FieldType_None) {
      mlog << Debug(3)
           << "Processing poly mask: "
           << engine.conf_info.mask_poly_name << "\n";
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

write_obj_netcdf(engine.conf_info.nc_info);

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
      // Object fields
      else if(i == 1) {
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

plot.open(ps_file.c_str());

plot.make_plot();

return;

}

////////////////////////////////////////////////////////////////////////


void ModeExecutive::build_outfile_prefix(ConcatString &str)

{

   //
   // Create leading part of output file name
   //

   // Append the output directory and program name

str << cs_erase
    << out_dir << "/" << program_name;

   // Append the output prefix, if defined

if(engine.conf_info.output_prefix.nonempty())
   str << "_" << engine.conf_info.output_prefix;

   // Append the timing information

str << "_"
    << sec_to_hhmmss(engine.fcst_raw->data.lead())            << "L_"
    << unix_to_yyyymmdd_hhmmss(engine.fcst_raw->data.valid()) << "V_"
    << sec_to_hhmmss(engine.fcst_raw->data.accum())           << "A";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeExecutive::build_simple_outfile_name(const char *suffix, ConcatString &str)

{

build_outfile_prefix(str);

   // Append the suffix

str << suffix;

return;

}


//////////////////////////////////////////////////////////////////////


void ModeExecutive::build_outfile_name(const char *suffix, ConcatString &str)

{

if ( engine.conf_info.n_runs() == 1 )  {

   build_simple_outfile_name(suffix, str);

   return;

}


build_outfile_prefix(str);

   //
   //  append the radius and threshold indices
   //

char junk[256];

snprintf(junk, sizeof(junk), "R%d_T%d", R_index + 1, T_index + 1);

str << '_' << junk;

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
   out.open(stat_file.c_str());

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
      out.open(stat_file.c_str());

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
      // Create output stats file for observation merging
      //
      build_outfile_name("_obs_merge.txt", stat_file);
      out.open(stat_file.c_str());

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


void ModeExecutive::write_obj_netcdf(const ModeNcOutInfo & info)

{

if ( info.all_false() )  return;

   int n, x, y;
   ConcatString out_file;
   ConcatString s;
   const ConcatString fcst_thresh = engine.conf_info.fcst_conv_thresh.get_str(5);
   const ConcatString  obs_thresh = engine.conf_info.obs_conv_thresh.get_str(5);

   float *fcst_raw_data      = (float *) 0;
   float *fcst_obj_raw_data  = (float *) 0;
   int   *fcst_obj_data      = (int *)   0;
   int   *fcst_clus_data     = (int *)   0;

   float *obs_raw_data       = (float *) 0;
   float *obs_obj_raw_data   = (float *) 0;
   int   *obs_obj_data       = (int *)   0;
   int   *obs_clus_data      = (int *)   0;

   NcFile *f_out             = (NcFile *) 0;

   NcDim  lat_dim           ;
   NcDim  lon_dim           ;

   NcDim  fcst_thresh_dim   ;
   NcDim   obs_thresh_dim   ;

   NcVar  fcst_raw_var      ;
   NcVar  fcst_obj_raw_var  ;
   NcVar  fcst_obj_var      ;
   NcVar  fcst_clus_var     ;

   NcVar  obs_raw_var       ;
   NcVar  obs_obj_raw_var   ;
   NcVar  obs_obj_var       ;
   NcVar  obs_clus_var      ;

   NcVar  fcst_radius_var   ;
   NcVar   obs_radius_var   ;

   NcVar  fcst_thresh_var   ;
   NcVar   obs_thresh_var   ;

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
   f_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(f_out)) {
      mlog << Error << "\nwrite_obj_netcdf() -> trouble opening output file "
           << out_file << "\n\n";
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_file.text(), program_name,
                       engine.conf_info.model.c_str(),
                       engine.conf_info.obtype.c_str(),
                       engine.conf_info.desc.c_str());

   // Add the projection information
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = add_dim(f_out, "lat", (long) grid.ny());
   lon_dim = add_dim(f_out, "lon", (long) grid.nx());

   fcst_thresh_dim = add_dim(f_out, "fcst_thresh_length", fcst_thresh.length());
    obs_thresh_dim = add_dim(f_out,  "obs_thresh_length",  obs_thresh.length());

   // Add the lat/lon variables
   if ( info.do_latlon )  write_netcdf_latlon(f_out, &lat_dim, &lon_dim, grid);

   int deflate_level = info.compress_level;

   // Define Variables
   if ( info.do_raw ) {
      fcst_raw_var     = add_var(f_out, "fcst_raw",     ncFloat, lat_dim, lon_dim, deflate_level);
      add_att(&fcst_raw_var, "long_name", "Forecast Raw Values");
      add_att(&fcst_raw_var, "_FillValue", bad_data_float);
   }
   if ( info.do_object_raw ) {
      fcst_obj_raw_var = add_var(f_out, "fcst_obj_raw", ncFloat, lat_dim, lon_dim, deflate_level);
      add_att(&fcst_obj_raw_var, "long_name", "Forecast Object Raw Values");
      add_att(&fcst_obj_raw_var, "_FillValue", bad_data_float);
   }
   if ( info.do_object_id ) {
      fcst_obj_var     = add_var(f_out, "fcst_obj_id",  ncInt,   lat_dim, lon_dim, deflate_level);
      add_att(&fcst_obj_var, "long_name", "Forecast Object ID");
      add_att(&fcst_obj_var, "_FillValue", bad_data_int);
   }
   if ( info.do_cluster_id ) {
      fcst_clus_var    = add_var(f_out, "fcst_clus_id", ncInt,   lat_dim, lon_dim, deflate_level);
      add_att(&fcst_clus_var, "long_name", "Forecast Cluster Object ID");
      add_att(&fcst_clus_var, "_FillValue", bad_data_int);
   }

   if ( info.do_raw ) {
      obs_raw_var      = add_var(f_out, "obs_raw",     ncFloat, lat_dim, lon_dim, deflate_level);
      add_att(&obs_raw_var, "long_name", "Observation Raw Values");
      add_att(&obs_raw_var, "_FillValue", bad_data_float);
   }
   if ( info.do_object_raw ) {
      obs_obj_raw_var  = add_var(f_out, "obs_obj_raw", ncFloat, lat_dim, lon_dim, deflate_level);
      add_att(&obs_obj_raw_var, "long_name", "Observation Object Raw Values");
      add_att(&obs_obj_raw_var, "_FillValue", bad_data_float);
   }
   if ( info.do_object_id ) {
      obs_obj_var      = add_var(f_out, "obs_obj_id",  ncInt,   lat_dim, lon_dim, deflate_level);
      add_att(&obs_obj_var, "long_name", "Observation Object ID");
      add_att(&obs_obj_var, "_FillValue", bad_data_int);
   }
   if ( info.do_cluster_id ) {
      obs_clus_var     = add_var(f_out, "obs_clus_id", ncInt,   lat_dim, lon_dim, deflate_level);
      add_att(&obs_clus_var, "long_name", "Observation Cluster Object ID");
      add_att(&obs_clus_var, "_FillValue", bad_data_int);
   }

   fcst_radius_var = add_var(f_out, (string)"fcst_conv_radius", ncInt, deflate_level);
    obs_radius_var = add_var(f_out, (string) "obs_conv_radius", ncInt, deflate_level);

   fcst_thresh_var = add_var(f_out, "fcst_conv_threshold", ncChar, fcst_thresh_dim, deflate_level);
    obs_thresh_var = add_var(f_out,  "obs_conv_threshold", ncChar,  obs_thresh_dim, deflate_level);

   //
   //  write the radius and threshold values
   //

   if ( !put_nc_data(&fcst_radius_var, &engine.conf_info.fcst_conv_radius)
       || !put_nc_data(&obs_radius_var, &engine.conf_info.obs_conv_radius) )  {

         mlog << Error
              << "write_obj_netcdf() -> "
              << "error writing fcst/obs convolution radii\n\n";

         exit(1);

   }

   if (    ! put_nc_data(&fcst_thresh_var, fcst_thresh.c_str())
        || ! put_nc_data(& obs_thresh_var, obs_thresh.c_str()) )  {

         mlog << Error
              << "write_obj_netcdf() -> "
              << "error writing fcst/obs thresholds\n\n";

         exit(1);

   }

      //
      //  fcst and obs values for variable, level and units
      //

   nc_add_string(f_out, engine.conf_info.fcst_info->name().c_str(),       "fcst_variable", "fcst_variable_length");
   nc_add_string(f_out, engine.conf_info.obs_info->name().c_str(),         "obs_variable",  "obs_variable_length");

   nc_add_string(f_out, engine.conf_info.fcst_info->level_name().c_str(), "fcst_level",    "fcst_level_length");
   nc_add_string(f_out, engine.conf_info.obs_info->level_name().c_str(),   "obs_level",     "obs_level_length");

   nc_add_string(f_out, engine.conf_info.fcst_info->units().c_str(),      "fcst_units",    "fcst_units_length");
   nc_add_string(f_out, engine.conf_info.obs_info->units().c_str(),        "obs_units",     "obs_units_length");



   // Add forecast variable attributes

   if ( info.do_raw )  {
      write_netcdf_var_times(&fcst_raw_var, engine.fcst_raw->data);
   }

   if ( info.do_object_raw )  {
      write_netcdf_var_times(&fcst_obj_raw_var, engine.fcst_raw->data);
   }

   if ( info.do_object_id )  {
      write_netcdf_var_times(&fcst_obj_var, engine.fcst_raw->data);
   }

   if ( info.do_cluster_id )  {
      write_netcdf_var_times(&fcst_clus_var, engine.fcst_raw->data);
   }

   // Add observation variable attributes

   if ( info.do_raw )  {
      write_netcdf_var_times(&obs_raw_var, engine.obs_raw->data);
   }

   if ( info.do_object_raw )  {
      write_netcdf_var_times(&obs_obj_raw_var, engine.obs_raw->data);
   }

   if ( info.do_object_id )  {
      write_netcdf_var_times(&obs_obj_var, engine.obs_raw->data);
   }

   if ( info.do_cluster_id )  {
      write_netcdf_var_times(&obs_clus_var, engine.obs_raw->data);
   }

   //
   // Allocate memory for the raw values and object ID's for each grid box
   //

   if ( info.do_raw )  {

      fcst_raw_data      = new float [grid.nx()*grid.ny()];
      obs_raw_data       = new float [grid.nx()*grid.ny()];

   }

   if ( info.do_object_raw )  {

      fcst_obj_raw_data  = new float [grid.nx()*grid.ny()];
      obs_obj_raw_data   = new float [grid.nx()*grid.ny()];

   }

   if ( info.do_object_id )  {

      fcst_obj_data      = new int   [grid.nx()*grid.ny()];
      obs_obj_data       = new int   [grid.nx()*grid.ny()];

   }

   if ( info.do_cluster_id )  {

      fcst_clus_data     = new int   [grid.nx()*grid.ny()];
      obs_clus_data      = new int   [grid.nx()*grid.ny()];

   }

   for(x=0; x<grid.nx(); x++) {

      for(y=0; y<grid.ny(); y++) {

         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

            //
            // Get raw values and object ID's for each grid box
            // Extra NULL checks to satisfy Fortify

         if ( info.do_raw &&
	        fcst_raw_data != NULL && obs_raw_data != NULL &&
	        engine.fcst_raw != NULL && engine.obs_raw != NULL  )  {

             fcst_raw_data[n] = engine.fcst_raw->data (x, y);
             obs_raw_data[n] = engine.obs_raw->data  (x, y);

         }

         if(engine.fcst_split->is_nonzero(x, y) ) {
	   if ( info.do_object_raw && fcst_obj_raw_data != NULL && engine.fcst_raw != NULL ) {
	     fcst_obj_raw_data[n] = engine.fcst_raw->data(x, y);
	   }
           if ( info.do_object_id && fcst_obj_data != NULL && engine.fcst_split != NULL ) {
             fcst_obj_data[n] = nint(engine.fcst_split->data(x, y));
           }
         }
         else {
            if ( info.do_object_raw && fcst_obj_raw_data != NULL ) {
	      fcst_obj_raw_data[n] = bad_data_float;
	    }
            if ( info.do_object_id && fcst_obj_data != NULL ) {
	      fcst_obj_data[n] = bad_data_int;
	    }
         }

         if(engine.obs_split->is_nonzero(x, y) ) {
	   if ( info.do_object_raw && obs_obj_raw_data != NULL ) {
	     obs_obj_raw_data[n] = engine.obs_raw->data(x, y);
	   }
	   if ( info.do_object_id && obs_obj_data != NULL ) {
	     obs_obj_data[n] = nint(engine.obs_split->data(x, y));
	   }
         }
         else {
	   if ( info.do_object_raw && obs_obj_raw_data != NULL) {
	     obs_obj_raw_data[n] = bad_data_float;
	   }
           if ( info.do_object_id && obs_obj_data != NULL ) {
	     obs_obj_data[n] = bad_data_int;
	   }
         }

            //
            // Get cluster object ID's for each grid box
            //

         if ( info.do_cluster_id && fcst_clus_data != NULL && obs_clus_data != NULL)  {

               // Write the index of the cluster object
            if ( engine.fcst_clus_split->data(x, y) > 0 ) {
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

         }    //  if info.do_cluster_id

      }   //  for y

   }   //  for x

   //
   // Write the forecast and observation raw value variables
   //

   if ( info.do_raw )  {

      if( !put_nc_data_with_dims(&fcst_raw_var, &fcst_raw_data[0], grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_raw_var, &obs_raw_data[0], grid.ny(), grid.nx()) ) {

         mlog << Error << "\nwrite_obj_netcdf() -> "
              << "error with the fcst_raw_var->put or obs_raw_var->put\n\n";
         exit(1);
      }

   }

   if ( info.do_object_raw )  {

      if( !put_nc_data_with_dims(&fcst_obj_raw_var, &fcst_obj_raw_data[0], grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_obj_raw_var, &obs_obj_raw_data[0], grid.ny(), grid.nx()) ) {

         mlog << Error << "\nwrite_obj_netcdf() -> "
              << "error with the fcst_obj_raw_var->put or obs_obj_raw_var->put\n\n";
         exit(1);
      }

   }

      //
      // Write the forecast and observation object ID variables
      //

   if ( info.do_object_id )  {

      if( !put_nc_data_with_dims(&fcst_obj_var, &fcst_obj_data[0], grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_obj_var, &obs_obj_data[0], grid.ny(), grid.nx()) ) {

         mlog << Error << "\nwrite_obj_netcdf() -> "
              << "error with the fcst_obj_var->put or obs_obj_var->put\n\n";
         exit(1);
      }

   }

      //
      // Write the forecast and observation cluster object ID variables
      //

   if ( info.do_cluster_id )  {

      if( !put_nc_data_with_dims(&fcst_clus_var, &fcst_clus_data[0], grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_clus_var, &obs_clus_data[0], grid.ny(), grid.nx()) ) {

         mlog << Error << "\nwrite_obj_netcdf() -> "
              << "error with the fcst_clus_var->put or obs_clus_var->put\n\n";
         exit(1);
      }

   }

   //
   // Delete allocated memory
   //

   if (fcst_raw_data)      { delete [] fcst_raw_data;      fcst_raw_data     = (float *) 0; }
   if (fcst_obj_raw_data)  { delete [] fcst_obj_raw_data;  fcst_obj_raw_data = (float *) 0; }
   if (fcst_obj_data)      { delete [] fcst_obj_data;      fcst_obj_data     = (int *) 0; }
   if (fcst_clus_data)     { delete [] fcst_clus_data;     fcst_clus_data    = (int *) 0; }

   if (obs_raw_data)       { delete [] obs_raw_data;       obs_raw_data      = (float *) 0; }
   if (obs_obj_raw_data)   { delete [] obs_obj_raw_data;   obs_obj_raw_data  = (float *) 0; }
   if (obs_obj_data)       { delete [] obs_obj_data;       obs_obj_data      = (int *) 0; }
   if (obs_clus_data)      { delete [] obs_clus_data;      obs_clus_data     = (int *) 0; }

      //
      // Write out the values of the vertices of the polylines.
      //

   if ( info.do_polylines ) write_poly_netcdf(f_out);

   //
   // Close the NetCDF file
   //
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_poly_netcdf(NcFile * f_out)

{

   //
   // Write out the number of forecast, observation, and cluster objects
   //
   NcVar n_fcst_simp_var ;
   NcVar n_obs_simp_var  ;
   NcVar n_clus_var      ;

   // Define scalar variables
   n_fcst_simp_var = add_var(f_out, "n_fcst_simp", ncInt);
   n_obs_simp_var  = add_var(f_out, "n_obs_simp",  ncInt);
   n_clus_var      = add_var(f_out, "n_clus", ncInt);

   //
   // Write the number of forecast and observation objects
   //
   if( !put_nc_data(&n_fcst_simp_var, &engine.n_fcst) ||
       !put_nc_data(&n_obs_simp_var, &engine.n_obs)   ||
       !put_nc_data(&n_clus_var, &engine.n_clus) ) {

      mlog << Error << "\nwrite_obj_netcdf() -> "
           << "error with the n_fcst_simp_var->put, "
           << "n_obs_simp_var->put, or n_clus_var->put\n\n";
      exit(1);
   }

   //
   // Only write out the polyline points if there are objects
   // present.
   //
   if(engine.n_fcst > 0) {
      write_poly_netcdf(f_out, FcstSimpBdyPoly);
      write_poly_netcdf(f_out, FcstSimpHullPoly);
   }
   if(engine.n_obs > 0) {
      write_poly_netcdf(f_out, ObsSimpBdyPoly);
      write_poly_netcdf(f_out, ObsSimpHullPoly);
   }
   if(engine.n_clus > 0) {
      write_poly_netcdf(f_out, FcstClusHullPoly);
      write_poly_netcdf(f_out, ObsClusHullPoly);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_poly_netcdf(NcFile *f_out, ObjPolyType poly_type)

{

   int i, j, x, y, n_pts, n_poly;
   double lat, lon;

   Polyline **poly            = (Polyline **) 0;

   int   *poly_start          = (int       *) 0;
   int   *poly_npts           = (int       *) 0;
   float *poly_lat            = (float     *) 0;
   float *poly_lon            = (float     *) 0;
   int   *poly_x              = (int       *) 0;
   int   *poly_y              = (int       *) 0;

   // Dimensions and variables for each object
   NcDim  obj_dim            ;
   NcVar  obj_poly_start_var ;
   NcVar  obj_poly_npts_var  ;

   // Dimensions and variables for each boundary point
   NcDim  poly_dim           ;
   NcVar  poly_lat_var       ;
   NcVar  poly_lon_var       ;
   NcVar  poly_x_var         ;
   NcVar  poly_y_var         ;

   // Dimension names
   ConcatString obj_dim_name,   poly_dim_name;

   // Variable names
   ConcatString start_var_name, start_long_name;
   ConcatString npts_var_name,  npts_long_name;
   ConcatString lat_var_name,   lat_long_name;
   ConcatString lon_var_name,   lon_long_name;
   ConcatString x_var_name,     x_long_name;
   ConcatString y_var_name,     y_long_name;
   ConcatString field_name,     field_long;
   ConcatString poly_name,      poly_long;

   // Determine the number of polylines to be written
   // and set up strings
   switch(poly_type) {

      case FcstSimpBdyPoly:
         n_poly     = engine.n_fcst;
         field_name = "fcst";
         field_long = "Forecast";
         poly_name  = "simp_bdy";
         poly_long  = "Simple Boundary";
         break;

      case ObsSimpBdyPoly:
         n_poly     = engine.n_obs;
         field_name = "obs";
         field_long = "Observation";
         poly_name  = "simp_bdy";
         poly_long  = "Simple Boundary";
         break;

      case FcstSimpHullPoly:
         n_poly     = engine.n_fcst;
         field_name = "fcst";
         field_long = "Forecast";
         poly_name  = "simp_hull";
         poly_long  = "Simple Convex Hull";
         break;

      case ObsSimpHullPoly:
         n_poly     = engine.n_obs;
         field_name = "obs";
         field_long = "Observation";
         poly_name  = "simp_hull";
         poly_long  = "Simple Convex Hull";
         break;

      case FcstClusHullPoly:
         n_poly     = engine.n_clus;
         field_name = "fcst";
         field_long = "Forecast";
         poly_name  = "clus_hull";
         poly_long  = "Cluster Convex Hull";
         break;

      case ObsClusHullPoly:
         n_poly     = engine.n_clus;
         field_name = "obs";
         field_long = "Observation";
         poly_name  = "clus_hull";
         poly_long  = "Cluster Convex Hull";
         break;

      default:
         return;
   }

   // Setup dimension name strings
   if(poly_type == FcstClusHullPoly ||
      poly_type == ObsClusHullPoly) {
      obj_dim_name << cs_erase << field_name << "_clus";
   }
   else {
      obj_dim_name << cs_erase << field_name << "_simp";
   }
   poly_dim_name   << cs_erase << field_name << "_" << poly_name;

   // Setup variable name strings
   start_var_name  << cs_erase << field_name << "_" << poly_name << "_start";
   start_long_name << cs_erase << field_long << " " << poly_long << " Starting Index";
   npts_var_name   << cs_erase << field_name << "_" << poly_name << "_npts";
   npts_long_name  << cs_erase << "Number of " << field_long << " " << poly_long << " Points";
   lat_var_name    << cs_erase << field_name << "_" << poly_name << "_lat";
   lat_long_name   << cs_erase << field_long << " " << poly_long << " Point Latitude";
   lon_var_name    << cs_erase << field_name << "_" << poly_name << "_lon";
   lon_long_name   << cs_erase << field_long << " " << poly_long << " Point Longitude";
   x_var_name      << cs_erase << field_name << "_" << poly_name << "_x";
   x_long_name     << cs_erase << field_long << " " << poly_long << " Point X-Coordinate";
   y_var_name      << cs_erase << field_name << "_" << poly_name << "_y";
   y_long_name     << cs_erase << field_long << " " << poly_long << " Point Y-Coordinate";

   // Allocate pointers for the polylines to be written
   poly = new Polyline * [n_poly];

   // Point at the polyline to be written
   for(i=0; i<n_poly; i++) {

      switch(poly_type) {

         case FcstSimpBdyPoly:
            poly[i] = &engine.fcst_single[i].boundary[0];
            break;

         case ObsSimpBdyPoly:
            poly[i] = &engine.obs_single[i].boundary[0];
            break;

         case FcstSimpHullPoly:
            poly[i] = &engine.fcst_single[i].convex_hull;
            break;

         case ObsSimpHullPoly:
            poly[i] = &engine.obs_single[i].convex_hull;
            break;

         case FcstClusHullPoly:
            poly[i] = &engine.fcst_cluster[i].convex_hull;
            break;

         case ObsClusHullPoly:
            poly[i] = &engine.obs_cluster[i].convex_hull;
            break;

         default:
            break;
      }
   }

   // Get the number of polyline points
   for(i=0, n_pts=0; i<n_poly; i++) n_pts += poly[i]->n_points;

   // Define dimensions
   NcDim tmp_obj_dim = get_nc_dim(f_out, (string)obj_dim_name);
   if(IS_INVALID_NC(tmp_obj_dim)) {
      obj_dim = add_dim(f_out, (string)obj_dim_name, (long) n_poly);
   }
   else {
      obj_dim = tmp_obj_dim;
   }
   poly_dim = add_dim(f_out, (string)poly_dim_name, (long) n_pts);

   // Define variables
   obj_poly_start_var = add_var(f_out, (string)start_var_name, ncInt,   obj_dim);
   obj_poly_npts_var  = add_var(f_out, (string)npts_var_name,  ncInt,   obj_dim);
   poly_lat_var       = add_var(f_out, (string)lat_var_name,   ncFloat, poly_dim);
   poly_lon_var       = add_var(f_out, (string)lon_var_name,   ncFloat, poly_dim);
   poly_x_var         = add_var(f_out, (string)x_var_name,     ncInt,   poly_dim);
   poly_y_var         = add_var(f_out, (string)y_var_name,     ncInt,   poly_dim);

   // Add variable attributes
   add_att(&obj_poly_start_var, "long_name", (string)start_long_name);
   add_att(&obj_poly_npts_var, "long_name", (string)npts_long_name);
   add_att(&poly_lat_var, "long_name", (string)lat_long_name);
   add_att(&poly_lat_var, "units", "degrees_north");
   add_att(&poly_lon_var, "long_name", (string)lon_long_name);
   add_att(&poly_lon_var, "units", "degrees_east");
   add_att(&poly_x_var, "long_name", (string)x_long_name);
   add_att(&poly_y_var, "long_name", (string)y_long_name);

   //
   // Allocate memory for the polyline points
   //
   poly_start = new int   [n_poly];
   poly_npts  = new int   [n_poly];
   poly_lat   = new float [n_pts];
   poly_lon   = new float [n_pts];
   poly_x     = new int   [n_pts];
   poly_y     = new int   [n_pts];

   //
   // Store the points for each polyline
   //
   for(i=0, n_pts=0; i<n_poly; i++) {

      // Store the starting point for this object.
      poly_start[i] = n_pts;

      // Store the number of points in this polyline.
      poly_npts[i] = poly[i]->n_points;

      for(j=0; j<poly_npts[i]; j++, n_pts++) {

         // Get the boundary point (x,y) coordinates and store them
         x = nint(poly[i]->u[j]);
         y = nint(poly[i]->v[j]);
         poly_x[n_pts] = x;
         poly_y[n_pts] = y;

         // Convert to lat/lon and store them
         grid.xy_to_latlon(x, y, lat, lon);
         poly_lat[n_pts] = lat;
         poly_lon[n_pts] = -1.0*lon;
      }
   }

   //
   // Write the polyline information
   //
   if( !put_nc_data_with_dims(&obj_poly_start_var, &poly_start[0], n_poly) ||
       !put_nc_data_with_dims(&obj_poly_npts_var, &poly_npts[0], n_poly) ) {

      mlog << Error << "\nwrite_poly_netcdf() -> "
           << "error with " << start_var_name << "->put or "
           << npts_var_name << "->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary lat/lon points
   //
   if( !put_nc_data_with_dims(&poly_lat_var, &poly_lat[0], n_pts) ||
       !put_nc_data_with_dims(&poly_lon_var, &poly_lon[0], n_pts) ) {

      mlog << Error << "\nwrite_poly_netcdf() -> "
           << "error with " << lat_var_name << "->put or "
           << lon_var_name << "->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary (x,y) points
   //
   if( !put_nc_data_with_dims(&poly_x_var, &poly_x[0], n_pts) ||
       !put_nc_data_with_dims(&poly_y_var, &poly_y[0], n_pts) ) {

      mlog << Error << "\nwrite_poly_netcdf() -> "
           << "error with " << x_var_name << "->put or"
           << y_var_name << "->put\n\n";
      exit(1);
   }

   //
   // Delete allocated memory
   //
   if(poly)       { delete [] poly;       poly       = (Polyline **) 0; }
   if(poly_start) { delete [] poly_start; poly_start = (int       *) 0; }
   if(poly_npts)  { delete [] poly_npts;  poly_npts  = (int       *) 0; }
   if(poly_lat)   { delete [] poly_lat;   poly_lat   = (float     *) 0; }
   if(poly_lon)   { delete [] poly_lon;   poly_lon   = (float     *) 0; }
   if(poly_x)     { delete [] poly_x;     poly_x     = (int       *) 0; }
   if(poly_y)     { delete [] poly_y;     poly_y     = (int       *) 0; }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_ct_stats()

{

   AsciiTable cts_at;
   ofstream out;
   int i, c;
   double v;
   ConcatString stat_file;

   //
   // Create output contingency table stat file name
   //
   build_outfile_name("_cts.txt", stat_file);

   //
   // Open output stat file
   //
   out.open(stat_file.c_str());

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
   cts_at.set_size(3, i);                        // Set table size
   justify_mode_cols(cts_at);                    // Justify columns
   cts_at.set_precision(                         // Set the precision
      engine.conf_info.conf.output_precision());
   cts_at.set_bad_data_value(bad_data_double);   // Set the bad data value
   cts_at.set_bad_data_str(na_str);              // Set the bad data string
   cts_at.set_delete_trailing_blank_rows(1);     // No trailing blank rows

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
     cts_at.set_entry(0, i + n_mode_hdr_columns, (string)mode_cts_columns[i]);
   }

   //
   // Store the contingency table counts and statistics in the AsciiTable
   // object.
   //
   for(i=0; i<n_cts; i++) {

      // Write out the header columns
      write_header_columns(engine, grid, cts_at, i+1);

      c = n_mode_hdr_columns;

      // Field
      cts_at.set_entry(i+1, c++, cts[i].name());

      // Total
      cts_at.set_entry(i+1, c++, cts[i].n());

      // FY_OY
      cts_at.set_entry(i+1, c++, cts[i].fy_oy());

      // FY_ON
      cts_at.set_entry(i+1, c++, cts[i].fy_on());

      // FN_OY
      cts_at.set_entry(i+1, c++, cts[i].fn_oy());

      // FN_ON
      cts_at.set_entry(i+1, c++, cts[i].fn_on());

      // Base Rate
      v = cts[i].oy_tp();
      cts_at.set_entry(i+1, c++, v);

      // Forecast Mean
      v = cts[i].fy_tp();
      cts_at.set_entry(i+1, c++, v);

      // Accuracy
      v = cts[i].accuracy();
      cts_at.set_entry(i+1, c++, v);

      // Forecast Bias
      v = cts[i].fbias();
      cts_at.set_entry(i+1, c++, v);

      // PODY
      v = cts[i].pod_yes();
      cts_at.set_entry(i+1, c++, v);

      // PODN
      v = cts[i].pod_no();
      cts_at.set_entry(i+1, c++, v);

      // POFD
      v = cts[i].pofd();
      cts_at.set_entry(i+1, c++, v);

      // FAR
      v = cts[i].far();
      cts_at.set_entry(i+1, c++, v);

      // CSI
      v = cts[i].csi();
      cts_at.set_entry(i+1, c++, v);

      // GSS
      v = cts[i].gss();
      cts_at.set_entry(i+1, c++, v);

      // HK
      v = cts[i].hk();
      cts_at.set_entry(i+1, c++, v);

      // HSS
      v = cts[i].hss();
      cts_at.set_entry(i+1, c++, v);

      // ODDS
      v = cts[i].odds();
      cts_at.set_entry(i+1, c++, v);
   }

   //
   // Write the AsciiTable object to the output file
   //
   out << cts_at;

   out.close();

   return;
}

///////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


///////////////////////////////////////////////////////////////////////


void nc_add_string(NcFile * f, const char * text, const char * var_name, const char * dim_name)

{

   NcDim  dim;
   NcVar  var;

   const char * t = 0;

   if ( ! text )  t = "XXX";
   else           t = text;

   const int N = strlen(t);


   dim = add_dim(f, dim_name, N);

   var = add_var(f, var_name, ncChar, dim);

   if ( ! put_nc_data(&var, t) )  {

      mlog << Error
           << " nc_add_string() -> unable to add string variable \"" << t << "\"\n\n";

      exit ( 1 );

   }

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////
