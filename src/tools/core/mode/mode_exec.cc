// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>
#include <algorithm>
#include <netcdf>

#include "mode_exec.h"
#include "nc_utils.h"
#include "vx_regrid.h"

using namespace std;
using namespace netCDF;


///////////////////////////////////////////////////////////////////////


static const int unmatched_id = -1;

static const char * cts_str[n_cts] = {"RAW", "OBJECT"};

static const char program_name [] = "mode";

static const char * default_config_filename = "MET_BASE/config/MODEConfig_default";

// took this out of the do_conv_thresh() method 
static int local_r_index = -1;

static string fcst_magic_string = "";
static string obs_magic_string = "";

///////////////////////////////////////////////////////////////////////


static void nc_add_string(NcFile *, const char * text, const char * var_name, const char * dim_name);
static void replaceAll(std::string& str, const std::string& from, const std::string& to);


///////////////////////////////////////////////////////////////////////


//
//  Code for class ModeExecutive
//


///////////////////////////////////////////////////////////////////////

ModeExecutive::ModeExecutive()//Processing_t p)

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

   fcst_mtddf = (Met2dDataFile *) nullptr;
   obs_mtddf = (Met2dDataFile *) nullptr;

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

   if ( fcst_mtddf )  { delete fcst_mtddf;  fcst_mtddf = (Met2dDataFile *) nullptr; }
   if (  obs_mtddf )  { delete  obs_mtddf;   obs_mtddf = (Met2dDataFile *) nullptr; }

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


void ModeExecutive::init_traditional(int n_files)

{

   Met2dDataFileFactory mtddf_factory;

   R_index = T_index = 0;

   conf_read();

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
   engine.conf_info.process_config_traditional(ftype, otype);
   int nf = engine.conf_info.n_fields_f();  // same as obs for traditional mode
   if (nf != n_files) {
      mlog << Error << "\nNumber of input files " << n_files << " Not equal to config number of fields "
           << nf << "\n\n";
      exit(1);
   }

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

void ModeExecutive::init_multivar_simple(int j, int n_files, ModeDataType dtype,
                                         const ModeConfInfo &conf)

{
   R_index = T_index = 0;
   engine.conf_info = conf;

   // tell the engine which type of data it is
   engine.set_data_type(dtype);

   engine.conf_info.set_field_index(j);

   // engine.conf_info.nc_info.compress_level = engine.conf_info.get_compression_level();

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::init_multivar_intensities(const ModeConfInfo &conf)

{

   R_index = T_index = 0;

   engine.conf_info = conf;
   
   // tell the engine which type of data it is
   engine.set_data_type(ModeDataType::MvMode_Both);

   // check one again for multivar problems
   engine.conf_info.check_multivar_not_implemented();

   // engine.conf_info.nc_info.compress_level = engine.conf_info.get_compression_level();

   return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::setup_traditional_fcst_obs_data()

{

   // ShapeData fcst_sd, obs_sd;
   double fmin, omin, fmax, omax;

   Fcst_sd.clear();
   Obs_sd.clear();

   // Read the gridded data from the input forecast file

   if ( !(fcst_mtddf->data_plane(*(engine.conf_info.Fcst->var_info), Fcst_sd.data)) )  {

      mlog << Error << "\nModeExecutive::setup_traditionalfcst_obs_data() -> "
           << "can't get forecast data \""
           << engine.conf_info.Fcst->var_info->magic_str()
           << "\" from file \"" << fcst_file << "\"\n\n";
      exit(1);
   }

   // Read the gridded data from the input observation file

   if ( !(obs_mtddf->data_plane(*(engine.conf_info.Obs->var_info), Obs_sd.data)) )  {

      mlog << Error << "\nModeExecutive::setup_traditional_fcst_obs_data() -> "
           << "can't get observation data \""
           << engine.conf_info.Obs->var_info->magic_str()
           << "\" from file \"" << obs_file << "\"\n\n";
      exit(1);
   }

   // Determine the verification grid

   grid = parse_vx_grid(engine.conf_info.Fcst->var_info->regrid(),
                        &(fcst_mtddf->grid()), &(obs_mtddf->grid()));

   // Store the grid

   engine.set_grid(&grid);

   // Regrid, if necessary

   if ( !(fcst_mtddf->grid() == grid) )  {
      mlog << Debug(1)
           << "Regridding forecast " << engine.conf_info.Fcst->var_info->magic_str()
           << " to the verification grid using "
           << engine.conf_info.Fcst->var_info->regrid().get_str() << ".\n";
      Fcst_sd.data = met_regrid(Fcst_sd.data, fcst_mtddf->grid(), grid,
                                engine.conf_info.Fcst->var_info->regrid());
   }

   // Regrid, if necessary

   if ( !(obs_mtddf->grid() == grid) )  {
      mlog << Debug(1)
           << "Regridding observation " << engine.conf_info.Obs->var_info->magic_str()
           << " to the verification grid using "
           << engine.conf_info.Obs->var_info->regrid().get_str() << ".\n";
      Obs_sd.data = met_regrid(Obs_sd.data, obs_mtddf->grid(), grid,
                               engine.conf_info.Obs->var_info->regrid());
   }

   // Rescale probabilites from [0, 100] to [0, 1]

   if ( engine.conf_info.Fcst->var_info->p_flag() ) rescale_probability(Fcst_sd.data);

   // Rescale probabilites from [0, 100] to [0, 1]

   if ( engine.conf_info.Obs->var_info->p_flag() ) rescale_probability(Obs_sd.data);

   // Check that the valid times match

   if(Fcst_sd.data.valid() != Obs_sd.data.valid()) {

      ConcatString cs;
      cs << "Forecast and observation valid times do not match ("
         << unix_to_yyyymmdd_hhmmss(Fcst_sd.data.valid()) << " != "
         << unix_to_yyyymmdd_hhmmss(Obs_sd.data.valid()) << ") for "
         << engine.conf_info.Fcst->var_info->magic_str() << " versus "
         << engine.conf_info.Obs->var_info->magic_str() << ".";

      if(engine.conf_info.conf.time_offset_warning(
           (int) (Fcst_sd.data.valid() - Obs_sd.data.valid()))) {
         mlog << Warning << "\nModeExecutive::setup_fcst_obs_data_traditional() ->"
              << cs << "\n\n";
      }
      else {
         mlog << Debug(3) << cs << "\n";
      }
   }

   // Check that the accumulation intervals match

   if(engine.conf_info.Fcst->var_info->level().type() == LevelType_Accum &&
      engine.conf_info.Obs->var_info->level().type()  == LevelType_Accum &&
      Fcst_sd.data.accum()                            != Obs_sd.data.accum()) {

      mlog << Warning << "\nModeExecutive::setup_fcst_obs_data_traditional() -> "
           << "Forecast and observation accumulation times do not match ("
           << sec_to_hhmmss(Fcst_sd.data.accum()) << " != "
           << sec_to_hhmmss(Obs_sd.data.accum()) << ") for "
           << engine.conf_info.Fcst->var_info->magic_str() << " versus "
           << engine.conf_info.Obs->var_info->magic_str() << ".\n\n";
   }

   mlog << Debug(1)
        << "Forecast Field: "
        << engine.conf_info.Fcst->var_info->name_attr() << " at "
        << engine.conf_info.Fcst->var_info->level_attr()
        << "\n"
        << "Observation Field: "
        << engine.conf_info.Obs->var_info->name_attr() << " at "
        << engine.conf_info.Obs->var_info->level_attr()
        << "\n";

   // Mask out the missing data between fields

   if(engine.conf_info.mask_missing_flag == FieldType::Fcst ||
      engine.conf_info.mask_missing_flag == FieldType::Both)
      mask_bad_data(Fcst_sd.data, Obs_sd.data);

   // Mask out the missing data between fields

   if(engine.conf_info.mask_missing_flag == FieldType::Obs ||
      engine.conf_info.mask_missing_flag == FieldType::Both)
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

   // store the input data units
   funits = engine.conf_info.Fcst->var_info->units();
   ounits = engine.conf_info.Obs->var_info->units();

   // store the input data level
   flevel = engine.conf_info.Fcst->var_info->level_name();
   olevel = engine.conf_info.Obs->var_info->level_name();

   //
   //  done
   //

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::setup_verification_grid(const ModeInputData &fcst,
                                            const ModeInputData &obs,
                                            const ModeConfInfo &conf)
{
   R_index = T_index = 0;
   engine.conf_info = conf;
   engine.conf_info.check_multivar_not_implemented();

   Fcst_sd.clear();
   Obs_sd.clear();
   Fcst_sd.data = fcst._dataPlane;
   Obs_sd.data = obs._dataPlane;

   // set this local conf to point to forecast 0, so that that regrid info
   // can be accessed and it can be decided if we are to use fcst or obs
   // input
   engine.conf_info.set_data_type(ModeDataType::MvMode_Fcst);
   engine.conf_info.set_field_index(0);
   grid = parse_vx_grid(engine.conf_info.Fcst->var_info->regrid(),
                        &fcst._grid, &obs._grid);
   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::setup_multivar_fcst_data(const Grid &verification_grid,
                                             const ModeInputData &input)
{

   double fmin, fmax;

   Fcst_sd.clear();
   Fcst_sd.data = input._dataPlane;

   grid = verification_grid;

   // Store the grid

   engine.set_grid(&grid);

   // Regrid, if necessary
   if ( !(input._grid == grid) ) {
      mlog << Debug(1)
           << "Regridding forecast " << engine.conf_info.Fcst->var_info->magic_str()
           << " to the verification grid using "
           << engine.conf_info.Fcst->var_info->regrid().get_str() << ".\n";
      Fcst_sd.data = met_regrid(Fcst_sd.data, input._grid, grid, 
                                engine.conf_info.Fcst->var_info->regrid());
   }

   // Rescale probabilites from [0, 100] to [0, 1]

   if ( engine.conf_info.Fcst->var_info->p_flag() ) rescale_probability(Fcst_sd.data);

   mlog << Debug(1)
        << "Forecast Field: "
        << engine.conf_info.Fcst->var_info->name_attr() << " at "
        << engine.conf_info.Fcst->var_info->level_attr()
        << "\n";

   // Parse the grid and/or polyline masks from the configuration

   process_fcst_masks(Fcst_sd);

   // Compute the min and max data values across both raw fields

   Fcst_sd.data.data_range(fmin, fmax);
   data_min = fmin;
   data_max = fmax;

   // store the input data units
   funits = engine.conf_info.Fcst->var_info->units();
   ounits = na_str;

   // store the input data level
   flevel = engine.conf_info.Fcst->var_info->level_name();
   olevel = na_str;

   //
   //  done
   //

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::setup_multivar_obs_data(const Grid &verification_grid,
                                            const ModeInputData &input)
{

   // ShapeData fcst_sd, obs_sd;
   double omin, omax;

   Obs_sd.clear();

   // Read the gridded data from the input observation file
   Obs_sd.data = input._dataPlane;

   grid = verification_grid;

   // Store the grid

   engine.set_grid(&grid);

   // Regrid, if necessary

   if ( !(input._grid == grid) )  {
      mlog << Debug(1)
           << "Regridding observation " << engine.conf_info.Obs->var_info->magic_str()
           << " to the verification grid using "
           << engine.conf_info.Obs->var_info->regrid().get_str() << ".\n";
      Obs_sd.data = met_regrid(Obs_sd.data, input._grid, grid,
                               engine.conf_info.Obs->var_info->regrid());
   }

   // Rescale probabilites from [0, 100] to [0, 1]

   if ( engine.conf_info.Obs->var_info->p_flag() ) rescale_probability(Obs_sd.data);

   mlog << Debug(1)
        << "Observation Field: "
        << engine.conf_info.Obs->var_info->name_attr() << " at "
        << engine.conf_info.Obs->var_info->level_attr()
        << "\n";

   // Parse the grid and/or polyline masks from the configuration

   process_obs_masks(Obs_sd);

   // Compute the min and max data values across both raw fields

   Obs_sd.data.data_range(omin, omax);
   data_min = omin;
   data_max = omax;

   // store the input data units
   funits = na_str;
   ounits = engine.conf_info.Obs->var_info->units();

   // store the input data level
   flevel = na_str;
   olevel = engine.conf_info.Obs->var_info->level_name();

   //
   //  done
   //

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::setup_multivar_fcst_obs_data_intensities(const MultiVarData &mvdf,
                                                             const MultiVarData &mvdo)
{
   double fmin, fmax, omin, omax;

   bool simple = true;
   Fcst_sd = *(mvdf.shapedata_ptr(simple));
   Fcst_sd.debug_examine();
   Obs_sd = *(mvdo.shapedata_ptr(simple));
   Obs_sd.debug_examine();
   grid = *(mvdf._grid);

   // Store the grid

   engine.set_grid(&grid);

   // Rescale probabilites from [0, 100] to [0, 1] not needed in the second pass

   // Check that the valid times match

   if(Fcst_sd.data.valid() != Obs_sd.data.valid()) {

      ConcatString cs;
      cs << "Forecast and observation valid times do not match ("
         << unix_to_yyyymmdd_hhmmss(Fcst_sd.data.valid()) << " != "
         << unix_to_yyyymmdd_hhmmss(Obs_sd.data.valid()) << ") for "
         << engine.conf_info.Fcst->var_info->magic_str() << " versus "
         << engine.conf_info.Obs->var_info->magic_str() << ".";

      if(engine.conf_info.conf.time_offset_warning(
           (int) (Fcst_sd.data.valid() - Obs_sd.data.valid()))) {
         mlog << Warning << "\nModeExecutive::setup_fcst_obs_data_multivar_intensities() ->"
              << cs << "\n\n";
      }
      else {
         mlog << Debug(3) << cs << "\n";
      }
   }

   // Check that the accumulation intervals match

   if(engine.conf_info.Fcst->var_info->level().type() == LevelType_Accum &&
      engine.conf_info.Obs->var_info->level().type()  == LevelType_Accum &&
      Fcst_sd.data.accum()                            != Obs_sd.data.accum()) {

       mlog << Warning << "\nModeExecutive::setup_fcst_obs_data_multivar_intensities() -> "
           << "Forecast and observation accumulation times do not match ("
           << sec_to_hhmmss(Fcst_sd.data.accum()) << " != "
           << sec_to_hhmmss(Obs_sd.data.accum()) << ") for "
           << engine.conf_info.Fcst->var_info->magic_str() << " versus "
           << engine.conf_info.Obs->var_info->magic_str() << ".\n\n";
   }

   mlog << Debug(1)
        << "Forecast Field: "
        << engine.conf_info.Fcst->var_info->name_attr() << " at "
        << engine.conf_info.Fcst->var_info->level_attr()
        << "\n";
   Fcst_sd.data.data_range(fmin, fmax);
   funits = engine.conf_info.Fcst->var_info->units();
   flevel = engine.conf_info.Fcst->var_info->level_name();
   mlog << Debug(1)
        << "Observation Field: "
        << engine.conf_info.Obs->var_info->name_attr() << " at "
        << engine.conf_info.Obs->var_info->level_attr()
        << "\n";
   Obs_sd.data.data_range(omin, omax);
   ounits = engine.conf_info.Obs->var_info->units();
   olevel = engine.conf_info.Obs->var_info->level_name();
   
   // in case perc thresh was done, retrieve the values created in the
   // first pass, which have been stored in mvdo and mvdf
   // NOTE: this might be removable with new design
   engine.conf_info.Obs->conv_thresh_array = mvdo._merge->_convThreshArray;
   engine.conf_info.Obs->merge_thresh_array = mvdo._merge->_mergeThreshArray;
   engine.conf_info.Fcst->conv_thresh_array = mvdf._merge->_convThreshArray;
   engine.conf_info.Fcst->merge_thresh_array = mvdf._merge->_mergeThreshArray;

   // masking of inputs not needed, as it was done in the first pass
   // and stored to Fcst_sd and Obs_sd
   
   // Compute the min and max data values across raw fields
   // do this in the 2nd pass as data has now been masked to within superobjects

   if     (!is_bad_data(fmin) && !is_bad_data(omin)) data_min = min(fmin, omin);
   else if(!is_bad_data(fmin) &&  is_bad_data(omin)) data_min = fmin;
   else if( is_bad_data(fmin) && !is_bad_data(omin)) data_min = omin;

   if     (!is_bad_data(fmax) && !is_bad_data(omax)) data_max = max(fmax, omax);
   else if(!is_bad_data(fmax) &&  is_bad_data(omax)) data_max = fmax;
   else if( is_bad_data(fmax) && !is_bad_data(omax)) data_max = omax;

   //  done
   //

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::setup_multivar_fcst_obs_data_super(const ShapeData &f_super,
                                                       const ShapeData &o_super,
                                                       const Grid &igrid)
{
   double fmin, omin, fmax, omax;

   bool simple = true;
   Fcst_sd = f_super;
   Fcst_sd.debug_examine();
   Obs_sd = o_super;
   Obs_sd.debug_examine();
   grid = igrid;

   // do not need to read any data, it is stored in the mvd input
   // the verification grid was created in the first pass, so we have that as well

   // Store the grid

   engine.set_grid(&grid);

   // regridding of inputs is not needed in the second pass, as regridded data outputs
   // are stored to input mvd

   // Rescale probabilites from [0, 100] to [0, 1] also not neede in the second pass

   // Check that the valid times match

   if(Fcst_sd.data.valid() != Obs_sd.data.valid()) {

      ConcatString cs;
      cs << "Forecast and observation valid times do not match ("
         << unix_to_yyyymmdd_hhmmss(Fcst_sd.data.valid()) << " != "
         << unix_to_yyyymmdd_hhmmss(Obs_sd.data.valid()) << ") for "
         << engine.conf_info.Fcst->var_info->magic_str() << " versus "
         << engine.conf_info.Obs->var_info->magic_str() << ".";

      if(engine.conf_info.conf.time_offset_warning(
           (int) (Fcst_sd.data.valid() != Obs_sd.data.valid()))) {
         mlog << Warning << "\nModeExecutive::setup_fcst_obs_data_multivar_super() ->"
              << cs << "\n\n";
      }
      else {
         mlog << Debug(3) << cs << "\n";
      }
   }

   // Check that the accumulation intervals match

   if(engine.conf_info.Fcst->var_info->level().type() == LevelType_Accum &&
      engine.conf_info.Obs->var_info->level().type()  == LevelType_Accum &&
      Fcst_sd.data.accum()                            != Obs_sd.data.accum()) {

      mlog << Warning << "\nModeExecutive::setup_fcst_obs_data_multivar_super() -> "
           << "Forecast and observation accumulation times do not match ("
           << sec_to_hhmmss(Fcst_sd.data.accum()) << " != "
           << sec_to_hhmmss(Obs_sd.data.accum()) << ") for "
           << engine.conf_info.Fcst->var_info->magic_str() << " versus "
           << engine.conf_info.Obs->var_info->magic_str() << ".\n\n";
   }

   mlog << Debug(1)
        << "Forecast Field: "
        << engine.conf_info.Fcst->var_info->name_attr() << " at "
        << engine.conf_info.Fcst->var_info->level_attr()
        << "\n"
        << "Observation Field: "
        << engine.conf_info.Obs->var_info->name_attr() << " at "
        << engine.conf_info.Obs->var_info->level_attr()
        << "\n";

   // masking of inputs not needed, as it was done in the first pass
   // and stored to Fcst_sd and Obs_sd
   
   // Compute the min and max data values across both raw fields
   // do this in the 2nd pass as data has now been masked to within superobjects

   Fcst_sd.data.data_range(fmin, fmax);
   Obs_sd.data.data_range(omin, omax);
   if     (!is_bad_data(fmin) && !is_bad_data(omin)) data_min = min(fmin, omin);
   else if(!is_bad_data(fmin) &&  is_bad_data(omin)) data_min = fmin;
   else if( is_bad_data(fmin) && !is_bad_data(omin)) data_min = omin;

   if     (!is_bad_data(fmax) && !is_bad_data(omax)) data_max = max(fmax, omax);
   else if(!is_bad_data(fmax) &&  is_bad_data(omax)) data_max = fmax;
   else if( is_bad_data(fmax) && !is_bad_data(omax)) data_max = omax;

   //
   //  done
   //

   return;

}
  

///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_conv_thresh_traditional(const int r_index, const int t_index)
{

   ModeConfInfo & conf = engine.conf_info;

   // note that we are assuming the same r_index and t_index for both forecasts
   // and obs, which might not be true if mvmode were to allow >1 index
   // (currently it does not)

   R_index = r_index;
   T_index = t_index;

   conf.set_conv_radius_by_index(R_index);
   conf.set_conv_thresh_by_index(T_index);
   conf.set_merge_thresh_by_index(T_index);

   //
   //  Set up the engine with these raw fields
   //

   string what = "forecast and observation fields";
   mlog << Debug(2) << "Identifying objects in the " << what << "...\n";

   if ( r_index != local_r_index ) {
      //  need to do convolution
      engine.set(Fcst_sd, Obs_sd);
   } else {
      //  don't need to do convolution
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

void ModeExecutive::do_conv_thresh_multivar_super()
{

   ModeConfInfo & conf = engine.conf_info;

   R_index = 0;
   T_index = 0;

   SingleThresh s("ne-9999");
   conf.set_conv_thresh(s);
   conf.set_conv_radius(0.0);
   conf.set_merge_thresh_by_index(T_index);

   //
   //  Set up the engine with these raw fields
   //
   string what = "forecast and observation fields";
   mlog << Debug(2) << "Identifying objects in the " << what << "...\n";

   engine.set_only_split(Fcst_sd, Obs_sd);

   //
   //  Compute the contingency table statistics for the fields, if needed
   // (not needed for simple or merge, only one field)
   //
   if ( conf.ct_stats_flag )  compute_ct_stats();

   //
   //  done
   //

   local_r_index = 0;

   return;

}


///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_conv_thresh_multivar_intensity_compare()
{

   ModeConfInfo & conf = engine.conf_info;

   R_index = 0;
   T_index = 0;

   conf.set_conv_radius_by_index(R_index);
   conf.set_conv_thresh_by_index(T_index);
   conf.set_merge_thresh_by_index(T_index);

   //
   //  Set up the engine with these raw fields
   //

   string what = "forecast and observation fields";
   mlog << Debug(2) << "Identifying objects in the " << what << "...\n";

   engine.set_only_split(Fcst_sd, Obs_sd);

   //
   //  Compute the contingency table statistics for the fields, if needed
   //
   if ( conf.ct_stats_flag )  compute_ct_stats();

   //
   //  done
   //

   local_r_index = 0;

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::do_conv_thresh_multivar_simple(Processing_t p)
{

   ModeConfInfo & conf = engine.conf_info;

   R_index = 0;
   T_index = 0;

   if (p == MULTIVAR_SIMPLE_MERGE) {
      conf.set_conv_thresh_by_merge_index(T_index);
   } else if (p == MULTIVAR_SIMPLE) {
      conf.set_conv_radius_by_index(0);
      conf.set_conv_thresh_by_index(0);
   } else {
      mlog << Error << "\nModeExecutive::do_conv_thresh_multivar_simple() -> "
           << "Wrong processing type input " << stype(p) << "\"\n\n";
      exit(1);
   }         

   conf.set_merge_thresh_by_index(0);

   //
   //  Set up the engine with these raw fields
   //

   string what;
   if (conf.data_type == ModeDataType::MvMode_Obs) {
      what = "observation field";
   } else {
      what = "forecast field";
   }
   mlog << Debug(2) << "Identifying objects in the " << what << "...\n";

   engine.set(Fcst_sd, Obs_sd);

   // (ct_stats not needed for simple or merge, only one field)
   //
   //  done
   //

   local_r_index = 0;

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::clear_internal_r_index()
{
   local_r_index = -1;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::do_merging_traditional()
{
   mlog << Debug(2)
        << "Identified: " << engine.n_fcst << " forecast objects "
        << "and " << engine.n_obs << " observation objects.\n";

   mlog << Debug(2)
        << "Performing merging ("
        << mergetype_to_string(engine.conf_info.Fcst->merge_flag)
        << ") in the forecast field.\n";

   // Do the forecast merging

   engine.do_fcst_merging(default_config_file.c_str(), merge_config_file.c_str());

   mlog << Debug(2)
        << "Performing merging ("
        << mergetype_to_string(engine.conf_info.Obs->merge_flag)
        << ") in the observation field.\n";

   // Do the observation merging

   engine.do_obs_merging(default_config_file.c_str(), merge_config_file.c_str());

   mlog << Debug(2)
        << "Remaining: " << engine.n_fcst << " forecast objects "
        << "and " << engine.n_obs << " observation objects.\n";

}   

///////////////////////////////////////////////////////////////////////

void ModeExecutive::do_merging_multivar(const ShapeData &f_merge,
                                        const ShapeData &o_merge,
                                        Processing_t p)
{
   if (p == MULTIVAR_SUPER) {
      // set the merge flag and merge_thresh appropriately
      ModeConfInfo & conf = engine.conf_info;
      SingleThresh s("ne-9999");
      conf.set_fcst_merge_flag(MergeType::Thresh);
      conf.set_fcst_merge_thresh(s);
      conf.set_obs_merge_flag(MergeType::Thresh);
      conf.set_obs_merge_thresh(s);
   } else if (p != MULTIVAR_INTENSITY) {
      mlog << Error << "\nModeExecutive::do_merging(shapedata, shapedata, p) -> "
           << "wrong method for processing type " << stype(p) << "\n\n";
      exit(1);
   }

   mlog << Debug(2)
        << "Identified: " << engine.n_fcst << " forecast objects "
        << "and " << engine.n_obs << " observation objects.\n";

   mlog << Debug(2)
        << "Performing merging ("
        << mergetype_to_string(engine.conf_info.Fcst->merge_flag)
        << ") in the forecast field.\n";

   // Do the forecast merging

   engine.do_fcst_merging(f_merge);

   mlog << Debug(2)
        << "Performing merging ("
        << mergetype_to_string(engine.conf_info.Obs->merge_flag)
        << ") in the observation field.\n";

   // Do the observation merging

   engine.do_obs_merging(o_merge);

   mlog << Debug(2)
        << "Remaining: " << engine.n_fcst << " forecast objects "
        << "and " << engine.n_obs << " observation objects.\n";

}   

///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_match_merge_multivar(const ShapeData &f_merge,
                                            const ShapeData &o_merge,
                                            Processing_t p)

{
   do_merging_multivar(f_merge, o_merge, p);

   // Do the matching of objects between fields

   engine.do_matching();

   return;

}

///////////////////////////////////////////////////////////////////////


void ModeExecutive::do_match_merge_traditional()

{
   do_merging_traditional();

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
   if(engine.conf_info.mask_grid_flag != FieldType::None) {
      mlog << Debug(3)
           << "Processing grid mask: "
           << engine.conf_info.mask_grid_name << "\n";
      parse_grid_mask(engine.conf_info.mask_grid_name, grid,
                      grid_mask_sd.data, name);
   }

   // Parse the poly mask into a ShapeData object
   if(engine.conf_info.mask_poly_flag != FieldType::None) {
      mlog << Debug(3)
           << "Processing poly mask: "
           << engine.conf_info.mask_poly_name << "\n";
      parse_poly_mask(engine.conf_info.mask_poly_name, grid,
                      poly_mask_sd.data, name);
   }

   // Apply the grid mask to the forecast field if requested
   if(engine.conf_info.mask_grid_flag == FieldType::Fcst ||
      engine.conf_info.mask_grid_flag == FieldType::Both) {
      apply_mask(fcst_sd, grid_mask_sd);
   }

   // Apply the grid mask to the observation field if requested
   if(engine.conf_info.mask_grid_flag == FieldType::Obs ||
      engine.conf_info.mask_grid_flag == FieldType::Both) {
      apply_mask(obs_sd, grid_mask_sd);
   }

   // Apply the polyline mask to the forecast field if requested
   if(engine.conf_info.mask_poly_flag == FieldType::Fcst ||
      engine.conf_info.mask_poly_flag == FieldType::Both) {
      apply_mask(fcst_sd, poly_mask_sd);
   }

   // Apply the polyline mask to the observation field if requested
   if(engine.conf_info.mask_poly_flag == FieldType::Obs ||
      engine.conf_info.mask_poly_flag == FieldType::Both) {
      apply_mask(obs_sd, poly_mask_sd);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_fcst_masks(ShapeData & fcst_sd)

{

   ShapeData grid_mask_sd, poly_mask_sd;
   ConcatString name;

   mlog << Debug(3) << "Processing masking regions.\n";

   // Parse the grid mask into a ShapeData object
   if(engine.conf_info.mask_grid_flag != FieldType::None) {
      mlog << Debug(3)
           << "Processing grid mask: "
           << engine.conf_info.mask_grid_name << "\n";
      parse_grid_mask(engine.conf_info.mask_grid_name, grid,
                      grid_mask_sd.data, name);
   }

   // Parse the poly mask into a ShapeData object
   if(engine.conf_info.mask_poly_flag != FieldType::None) {
      mlog << Debug(3)
           << "Processing poly mask: "
           << engine.conf_info.mask_poly_name << "\n";
      parse_poly_mask(engine.conf_info.mask_poly_name, grid,
                      poly_mask_sd.data, name);
   }

   // Apply the grid mask to the forecast field if requested
   if(engine.conf_info.mask_grid_flag == FieldType::Fcst ||
      engine.conf_info.mask_grid_flag == FieldType::Both) {
      apply_mask(fcst_sd, grid_mask_sd);
   }

   // Apply the polyline mask to the forecast field if requested
   if(engine.conf_info.mask_poly_flag == FieldType::Fcst ||
      engine.conf_info.mask_poly_flag == FieldType::Both) {
      apply_mask(fcst_sd, poly_mask_sd);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_obs_masks(ShapeData & obs_sd)

{

   ShapeData grid_mask_sd, poly_mask_sd;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Parse the grid mask into a ShapeData object
   if(engine.conf_info.mask_grid_flag != FieldType::None) {
      mlog << Debug(3)
           << "Processing grid mask: "
           << engine.conf_info.mask_grid_name << "\n";
      parse_grid_mask(engine.conf_info.mask_grid_name, grid,
                      grid_mask_sd.data, name);
   }

   // Parse the poly mask into a ShapeData object
   if(engine.conf_info.mask_poly_flag != FieldType::None) {
      mlog << Debug(3)
           << "Processing poly mask: "
           << engine.conf_info.mask_poly_name << "\n";
      parse_poly_mask(engine.conf_info.mask_poly_name, grid,
                      poly_mask_sd.data, name);
   }

   // Apply the grid mask to the observation field if requested
   if(engine.conf_info.mask_grid_flag == FieldType::Obs ||
      engine.conf_info.mask_grid_flag == FieldType::Both) {
      apply_mask(obs_sd, grid_mask_sd);
   }

   // Apply the polyline mask to the observation field if requested
   if(engine.conf_info.mask_poly_flag == FieldType::Obs ||
      engine.conf_info.mask_poly_flag == FieldType::Both) {
      apply_mask(obs_sd, poly_mask_sd);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::set_raw_to_full(float *fcst_raw_data,
                                    float *obs_raw_data,
                                    int nx, int ny,
                                    double idata_min, double idata_max)
{
   engine.fcst_raw->data.set_all(fcst_raw_data, nx, ny);
   engine.obs_raw->data.set_all(obs_raw_data, nx, ny);
   data_min = idata_min;
   data_max = idata_max;
}
      
///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_output_multivar_super()
{
   isMultivarOutput = false;
   isMultivarSuperOutput = true;

   // use the configured multivar name and level
   fcst_magic_string = engine.conf_info.fcst_multivar_name.string() + "_" + engine.conf_info.fcst_multivar_level.string();
   obs_magic_string = engine.conf_info.obs_multivar_name.string() + "_" + engine.conf_info.obs_multivar_level.string();
   
   // Create output stats files and plots

   write_obj_stats();

   if ( engine.conf_info.ct_stats_flag )  write_ct_stats();
   
   write_obj_netcdf(engine.conf_info.nc_info);

   if ( engine.conf_info.ps_plot_flag )   plot_engine();

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_output_multivar_intensity_compare(const MultiVarData *mvdf,
                                                              const MultiVarData *mvdo)

{
   isMultivarOutput = true;
   isMultivarSuperOutput = false;

   // get the magic strings, which will be used in file naming
   fcst_magic_string = engine.conf_info.Fcst->var_info->magic_str().c_str();
   obs_magic_string = engine.conf_info.Obs->var_info->magic_str().c_str();

   // replace forward slashes with underscores to prevent new directories
   replace(fcst_magic_string.begin(), fcst_magic_string.end(), '/', '_');   
   replace(obs_magic_string.begin(), obs_magic_string.end(), '/', '_');   

   // replace (*,*) with '_all_all_' 
   replaceAll(fcst_magic_string, "*", "all");
   replaceAll(obs_magic_string, "*", "all");
   replaceAll(fcst_magic_string, ",", "_");
   replaceAll(obs_magic_string, ",", "_");
   replaceAll(fcst_magic_string, "(", "_");
   replaceAll(obs_magic_string, "(", "_");
   replaceAll(fcst_magic_string, ")", "");
   replaceAll(obs_magic_string, ")", "");
      
   // Create output stats files and plots

   write_obj_stats();

   if ( engine.conf_info.ct_stats_flag )  write_ct_stats();


   double fmin = mvdf->_data_min;
   double fmax = mvdf->_data_max;
   double omin = mvdo->_data_min;
   double omax = mvdo->_data_max;
   double data_min, data_max;
   if     (!is_bad_data(fmin) && !is_bad_data(omin)) data_min = min(fmin, omin);
   else if(!is_bad_data(fmin) &&  is_bad_data(omin)) data_min = fmin;
   else if( is_bad_data(fmin) && !is_bad_data(omin)) data_min = omin;

   if     (!is_bad_data(fmax) && !is_bad_data(omax)) data_max = max(fmax, omax);
   else if(!is_bad_data(fmax) &&  is_bad_data(omax)) data_max = fmax;
   else if( is_bad_data(fmax) && !is_bad_data(omax)) data_max = omax;
         
   set_raw_to_full(mvdf->_simple->_raw_data,mvdo->_simple->_raw_data,
                   mvdf->_nx, mvdf->_ny, data_min, data_max);

   write_obj_netcdf(engine.conf_info.nc_info);

   if ( engine.conf_info.ps_plot_flag )   plot_engine();

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::process_output_traditional()
{
   isMultivarOutput = false;
   isMultivarSuperOutput = false;

   // just in case make these empty
   fcst_magic_string = "";
   obs_magic_string = "";

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

      cts[i].zero_out();
      cts[i].set_name(cts_str[i]);

      // Raw fields
      if(i == 0) {
         fcst_mask = *engine.fcst_raw;
         obs_mask  = *engine.obs_raw;

         // Apply the thresholds specified in the config file
         fcst_mask.threshold(engine.conf_info.Fcst->conv_thresh);
         obs_mask.threshold(engine.conf_info.Obs->conv_thresh);
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

   plot.make_plot(isMultivarSuperOutput);

   return;

}

////////////////////////////////////////////////////////////////////////


void ModeExecutive::build_outfile_prefix(ConcatString &str)

{

   //
   // Create leading part of output file name including out_dir
   //


   if (isMultivarOutput || isMultivarSuperOutput) {

      // Append the program name and magic strings

      str << cs_erase << out_dir << "/" << program_name
          << "_Fcst_" << fcst_magic_string
          << "_Obs_" << obs_magic_string;
   } else {

      // Append the program name

      str << cs_erase << out_dir << "/" << program_name;
   }      
   
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
      mlog << Error << "\nModeExecutive::write_obj_stats() -> "
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
   write_engine_stats(engine, grid, obj_at, isMultivarSuperOutput,
                      isMultivarOutput); 

   //
   // Write the AsciiTable object to the output file
   //
   out << obj_at;

   out.close();

   if(engine.conf_info.Fcst->merge_flag == MergeType::Both ||
      engine.conf_info.Fcst->merge_flag == MergeType::Engine) {

      //
      // Create output stats file for forecast merging
      //
      build_outfile_name("_fcst_merge.txt", stat_file);
      out.open(stat_file.c_str());

      if(!out) {
         mlog << Error << "\nModeExecutive::write_obj_stats() -> "
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

   if(engine.conf_info.Obs->merge_flag == MergeType::Both ||
      engine.conf_info.Obs->merge_flag == MergeType::Engine) {

      //
      // Create output stats file for observation merging
      //
      build_outfile_name("_obs_merge.txt", stat_file);
      out.open(stat_file.c_str());

      if(!out) {
         mlog << Error << "\nModeExecutive::write_obj_stats() -> "
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

//////////////////////////////////////////////////////////////////////

MultiVarData *ModeExecutive::get_multivar_data(ModeDataType dtype)
{
   bool simple=true;
   MultiVarData *mvd = new MultiVarData();

   switch (dtype)
   {
   case ModeDataType::MvMode_Obs:
      obs_magic_string = engine.conf_info.Obs->var_info->magic_str().c_str();
      // replace forward slashes with underscores to prevent new directories
      replace(obs_magic_string.begin(), obs_magic_string.end(), '/', '_');   
      mvd->init(dtype, obs_magic_string, grid, 
                ounits, olevel, data_min, data_max);
      mvd->set_obj(engine.obs_split, simple);
      mvd->set_raw(engine.obs_raw, simple);
      mvd->set_shapedata(Obs_sd, simple);
      mvd->set_conv_thresh_array(engine.conf_info.Obs->conv_thresh_array, simple);
      mvd->set_merge_thresh_array(engine.conf_info.Obs->merge_thresh_array, simple);
      break;
   case ModeDataType::MvMode_Fcst:
      fcst_magic_string = engine.conf_info.Fcst->var_info->magic_str().c_str();
      // replace forward slashes with underscores to prevent new directories
      replace(fcst_magic_string.begin(), fcst_magic_string.end(), '/', '_');   
      mvd->init(dtype, fcst_magic_string, grid, 
                funits, flevel, data_min, data_max);
      mvd->set_obj(engine.fcst_split, simple);
      mvd->set_raw(engine.fcst_raw, simple);
      mvd->set_shapedata(Fcst_sd, simple);
      mvd->set_conv_thresh_array(engine.conf_info.Fcst->conv_thresh_array, simple);
      mvd->set_merge_thresh_array(engine.conf_info.Fcst->merge_thresh_array, simple);
      break;
   default:
      mlog << Error << "\nModeExecutive::get_multivar_data() -> wrong data type "
           << sprintModeDataType(dtype) << "\n\n";
      exit(1);
   }
   return mvd;
}

///////////////////////////////////////////////////////////////////////


void ModeExecutive::add_multivar_merge_data(MultiVarData *mvd, ModeDataType dtype)
{
   bool simple = false;
   switch (dtype)
   {
   case ModeDataType::MvMode_Obs:
      mvd->set_obj(engine.obs_split, simple);
      mvd->set_raw(engine.obs_raw, simple);
      mvd->set_shapedata(Obs_sd, simple);
      mvd->set_conv_thresh_array(engine.conf_info.Obs->conv_thresh_array, simple);
      mvd->set_merge_thresh_array(engine.conf_info.Obs->merge_thresh_array, simple);
      break;
   case ModeDataType::MvMode_Fcst:
      mvd->set_obj(engine.fcst_split, simple);
      mvd->set_raw(engine.fcst_raw, simple);
      mvd->set_shapedata(Fcst_sd, simple);
      mvd->set_conv_thresh_array(engine.conf_info.Fcst->conv_thresh_array, simple);
      mvd->set_merge_thresh_array(engine.conf_info.Fcst->merge_thresh_array, simple);
      break;
   default:
      mlog << Error << "\nModeExecutive::add_multivar_merge_data() -> wrong data type "
           << sprintModeDataType(dtype) << "\n\n";
      exit(1);
   }
}

///////////////////////////////////////////////////////////////////////


void ModeExecutive::write_obj_netcdf(const ModeNcOutInfo & info)

{

   if ( info.all_false() )  return;

   int n, x, y;
   ConcatString out_file;
   ConcatString s;
   const ConcatString fcst_thresh = engine.conf_info.Fcst->conv_thresh.get_str(5);
   const ConcatString  obs_thresh = engine.conf_info.Obs->conv_thresh.get_str(5);

   vector<float> fcst_raw_data    ;
   vector<float> fcst_obj_raw_data;
   vector<int  > fcst_obj_data    ;
   vector<int  > fcst_clus_data   ;

   vector<float> obs_raw_data    ;
   vector<float> obs_obj_raw_data;
   vector<int  > obs_obj_data    ;
   vector<int  > obs_clus_data   ;

   NcFile *f_out             = (NcFile *) nullptr;

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
      mlog << Error << "\nModeExecutive::write_obj_netcdf() -> trouble opening output file "
           << out_file << "\n\n";
      delete f_out;
      f_out = (NcFile *) nullptr;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_file.text(), program_name,
                       engine.conf_info.model.c_str(),
                       engine.conf_info.obtype.c_str(),
                       engine.conf_info.desc.c_str());

   // Add the projection information
   write_netcdf_proj(f_out, grid, lat_dim, lon_dim);

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

   if ( !put_nc_data(&fcst_radius_var, &engine.conf_info.Fcst->conv_radius)
        || !put_nc_data(&obs_radius_var, &engine.conf_info.Obs->conv_radius) )  {

      mlog << Error << "\nModeExecutive::write_obj_netcdf() -> "
           << "error writing fcst/obs convolution radii\n\n";

      exit(1);

   }

   if (    ! put_nc_data(&fcst_thresh_var, fcst_thresh.c_str())
           || ! put_nc_data(& obs_thresh_var, obs_thresh.c_str()) )  {

      mlog << Error << "\nModeExecutive::write_obj_netcdf() -> "
           << "error writing fcst/obs thresholds\n\n";

      exit(1);

   }

   //
   //  fcst and obs values for variable, level and units
   //

   if (isMultivarSuperOutput) {
      nc_add_string(f_out, engine.conf_info.fcst_multivar_name.c_str(),  "fcst_variable", "fcst_variable_length");
      nc_add_string(f_out, engine.conf_info.obs_multivar_name.c_str(),    "obs_variable",  "obs_variable_length");

      nc_add_string(f_out, engine.conf_info.fcst_multivar_level.c_str(), "fcst_level",    "fcst_level_length");
      nc_add_string(f_out, engine.conf_info.obs_multivar_level.c_str(),   "obs_level",     "obs_level_length");

      nc_add_string(f_out, "NA", "fcst_units",    "fcst_units_length");
      nc_add_string(f_out, "NA", "obs_units",     "obs_units_length");
   } else {
      nc_add_string(f_out, engine.conf_info.Fcst->var_info->name_attr().c_str(),  "fcst_variable", "fcst_variable_length");
      nc_add_string(f_out, engine.conf_info.Obs->var_info->name_attr().c_str(),    "obs_variable",  "obs_variable_length");

      nc_add_string(f_out, engine.conf_info.Fcst->var_info->level_attr().c_str(), "fcst_level",    "fcst_level_length");
      nc_add_string(f_out, engine.conf_info.Obs->var_info->level_attr().c_str(),   "obs_level",     "obs_level_length");

      nc_add_string(f_out, engine.conf_info.Fcst->var_info->units_attr().c_str(), "fcst_units",    "fcst_units_length");
      nc_add_string(f_out, engine.conf_info.Obs->var_info->units_attr().c_str(),   "obs_units",     "obs_units_length");
   }

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

      fcst_raw_data.resize(grid.nx()*grid.ny());
       obs_raw_data.resize(grid.nx()*grid.ny());

   }

   if ( info.do_object_raw )  {

      fcst_obj_raw_data.resize(grid.nx()*grid.ny());
       obs_obj_raw_data.resize(grid.nx()*grid.ny());

   }

   if ( info.do_object_id )  {

      fcst_obj_data.resize(grid.nx()*grid.ny());
       obs_obj_data.resize(grid.nx()*grid.ny());

   }

   if ( info.do_cluster_id )  {

      fcst_clus_data.resize(grid.nx()*grid.ny());
       obs_clus_data.resize(grid.nx()*grid.ny());

   }

   for(x=0; x<grid.nx(); x++) {

      for(y=0; y<grid.ny(); y++) {

         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);

         //
         // Get raw values and object ID's for each grid box
         // Extra nullptr checks to satisfy Fortify

         if ( info.do_raw &&
              !fcst_raw_data.empty() && !obs_raw_data.empty() &&
              engine.fcst_raw != nullptr && engine.obs_raw != nullptr  )  {

            fcst_raw_data[n] = engine.fcst_raw->data (x, y);
            obs_raw_data[n] = engine.obs_raw->data  (x, y);

         }

         if(engine.fcst_split->is_nonzero(x, y) ) {
            if ( info.do_object_raw && !fcst_obj_raw_data.empty() && engine.fcst_raw != nullptr ) {
               fcst_obj_raw_data[n] = engine.fcst_raw->data(x, y);
            }
            if ( info.do_object_id && !fcst_obj_data.empty() && engine.fcst_split != nullptr ) {
               fcst_obj_data[n] = nint(engine.fcst_split->data(x, y));
            }
         }
         else {
            if ( info.do_object_raw && !fcst_obj_raw_data.empty() ) {
               fcst_obj_raw_data[n] = bad_data_float;
            }
            if ( info.do_object_id && !fcst_obj_data.empty() ) {
               fcst_obj_data[n] = bad_data_int;
            }
         }

         if(engine.obs_split->is_nonzero(x, y) ) {
            if ( info.do_object_raw && !obs_obj_raw_data.empty() ) {
               obs_obj_raw_data[n] = engine.obs_raw->data(x, y);
            }
            if ( info.do_object_id && !obs_obj_data.empty() ) {
               obs_obj_data[n] = nint(engine.obs_split->data(x, y));
            }
         }
         else {
            if ( info.do_object_raw && !obs_obj_raw_data.empty()) {
               obs_obj_raw_data[n] = bad_data_float;
            }
            if ( info.do_object_id && !obs_obj_data.empty() ) {
               obs_obj_data[n] = bad_data_int;
            }
         }

         //
         // Get cluster object ID's for each grid box
         //

         if ( info.do_cluster_id && !fcst_clus_data.empty() && !obs_clus_data.empty())  {

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

      if( !put_nc_data_with_dims(&fcst_raw_var, fcst_raw_data.data(), grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_raw_var, obs_raw_data.data(), grid.ny(), grid.nx()) ) {

         mlog << Error << "\nModeExecutive::write_obj_netcdf() -> "
              << "error with the fcst_raw_var->put or obs_raw_var->put\n\n";
         exit(1);
      }

   }

   if ( info.do_object_raw )  {

      if( !put_nc_data_with_dims(&fcst_obj_raw_var, fcst_obj_raw_data.data(), grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_obj_raw_var, obs_obj_raw_data.data(), grid.ny(), grid.nx()) ) {

         mlog << Error << "\nModeExecutive::write_obj_netcdf() -> "
              << "error with the fcst_obj_raw_var->put or obs_obj_raw_var->put\n\n";
         exit(1);
      }

   }

   //
   // Write the forecast and observation object ID variables
   //

   if ( info.do_object_id )  {

      if( !put_nc_data_with_dims(&fcst_obj_var, fcst_obj_data.data(), grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_obj_var, obs_obj_data.data(), grid.ny(), grid.nx()) ) {

         mlog << Error << "\nModeExecutive::write_obj_netcdf() -> "
              << "error with the fcst_obj_var->put or obs_obj_var->put\n\n";
         exit(1);
      }

   }

   //
   // Write the forecast and observation cluster object ID variables
   //

   if ( info.do_cluster_id )  {

      if( !put_nc_data_with_dims(&fcst_clus_var, fcst_clus_data.data(), grid.ny(), grid.nx()) ||
          !put_nc_data_with_dims(&obs_clus_var, obs_clus_data.data(), grid.ny(), grid.nx()) ) {

         mlog << Error << "\nModeExecutive::write_obj_netcdf() -> "
              << "error with the fcst_clus_var->put or obs_clus_var->put\n\n";
         exit(1);
      }

   }

   //
   // Write out the values of the vertices of the polylines.
   //

   if ( info.do_polylines ) write_poly_netcdf(f_out);

   //
   // Close the NetCDF file
   //
   delete f_out;
   f_out = (NcFile *) nullptr;

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

      mlog << Error << "\nModeExecutive::write_poly_netcdf() -> "
           << "error with the n_fcst_simp_var->put, "
           << "n_obs_simp_var->put, or n_clus_var->put\n\n";
      exit(1);
   }

   //
   // Only write out the polyline points if there are objects
   // present.
   //
   if(engine.n_fcst > 0) {
      write_poly_netcdf(f_out, ObjPolyType::FcstSimpBdy);
      write_poly_netcdf(f_out, ObjPolyType::FcstSimpHull);
   }
   if(engine.n_obs > 0) {
      write_poly_netcdf(f_out, ObjPolyType::ObsSimpBdy);
      write_poly_netcdf(f_out, ObjPolyType::ObsSimpHull);
   }
   if(engine.n_clus > 0) {
      write_poly_netcdf(f_out, ObjPolyType::FcstClusHull);
      write_poly_netcdf(f_out, ObjPolyType::ObsClusHull);
   }

   return;
}

///////////////////////////////////////////////////////////////////////

void ModeExecutive::write_poly_netcdf(NcFile *f_out, ObjPolyType poly_type)

{

   int i, j, x, y, n_pts, n_poly;
   double lat, lon;

   Polyline **poly            = (Polyline **) nullptr;

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

   case ObjPolyType::FcstSimpBdy:
      n_poly     = engine.n_fcst;
      field_name = "fcst";
      field_long = "Forecast";
      poly_name  = "simp_bdy";
      poly_long  = "Simple Boundary";
      break;

   case ObjPolyType::ObsSimpBdy:
      n_poly     = engine.n_obs;
      field_name = "obs";
      field_long = "Observation";
      poly_name  = "simp_bdy";
      poly_long  = "Simple Boundary";
      break;

   case ObjPolyType::FcstSimpHull:
      n_poly     = engine.n_fcst;
      field_name = "fcst";
      field_long = "Forecast";
      poly_name  = "simp_hull";
      poly_long  = "Simple Convex Hull";
      break;

   case ObjPolyType::ObsSimpHull:
      n_poly     = engine.n_obs;
      field_name = "obs";
      field_long = "Observation";
      poly_name  = "simp_hull";
      poly_long  = "Simple Convex Hull";
      break;

   case ObjPolyType::FcstClusHull:
      n_poly     = engine.n_clus;
      field_name = "fcst";
      field_long = "Forecast";
      poly_name  = "clus_hull";
      poly_long  = "Cluster Convex Hull";
      break;

   case ObjPolyType::ObsClusHull:
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
   if(poly_type == ObjPolyType::FcstClusHull ||
      poly_type == ObjPolyType::ObsClusHull) {
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

      case ObjPolyType::FcstSimpBdy:
         poly[i] = &engine.fcst_single[i].boundary[0];
         break;

      case ObjPolyType::ObsSimpBdy:
         poly[i] = &engine.obs_single[i].boundary[0];
         break;

      case ObjPolyType::FcstSimpHull:
         poly[i] = &engine.fcst_single[i].convex_hull;
         break;

      case ObjPolyType::ObsSimpHull:
         poly[i] = &engine.obs_single[i].convex_hull;
         break;

      case ObjPolyType::FcstClusHull:
         poly[i] = &engine.fcst_cluster[i].convex_hull;
         break;

      case ObjPolyType::ObsClusHull:
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
   vector<int  > poly_start(n_poly);
   vector<int  > poly_npts (n_poly);
   vector<float> poly_lat  (n_pts);
   vector<float> poly_lon  (n_pts);
   vector<int  > poly_x    (n_pts);
   vector<int  > poly_y    (n_pts);

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
   if( !put_nc_data_with_dims(&obj_poly_start_var, poly_start.data(), n_poly) ||
       !put_nc_data_with_dims(&obj_poly_npts_var, poly_npts.data(), n_poly) ) {

      mlog << Error << "\nModeExecutive::write_poly_netcdf() -> "
           << "error with " << start_var_name << "->put or "
           << npts_var_name << "->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary lat/lon points
   //
   if( !put_nc_data_with_dims(&poly_lat_var, poly_lat.data(), n_pts) ||
       !put_nc_data_with_dims(&poly_lon_var, poly_lon.data(), n_pts) ) {

      mlog << Error << "\nModeExecutive::write_poly_netcdf() -> "
           << "error with " << lat_var_name << "->put or "
           << lon_var_name << "->put\n\n";
      exit(1);
   }

   //
   // Write the forecast boundary (x,y) points
   //
   if( !put_nc_data_with_dims(&poly_x_var, poly_x.data(), n_pts) ||
       !put_nc_data_with_dims(&poly_y_var, poly_y.data(), n_pts) ) {

      mlog << Error << "\nModeExecutive::write_poly_netcdf() -> "
           << "error with " << x_var_name << "->put or"
           << y_var_name << "->put\n\n";
      exit(1);
   }

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
      mlog << Error << "\nModeExecutive::write_ct_stats() -> "
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
      cts_at.set_entry(i+1, c++, cts[i].n_pairs());

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

      // LODDS
      v = cts[i].lodds();
      cts_at.set_entry(i+1, c++, v);

      // ORSS
      v = cts[i].orss();
      cts_at.set_entry(i+1, c++, v);

      // EDS
      v = cts[i].eds();
      cts_at.set_entry(i+1, c++, v);

      // SEDS
      v = cts[i].seds();
      cts_at.set_entry(i+1, c++, v);

      // EDI
      v = cts[i].edi();
      cts_at.set_entry(i+1, c++, v);

      // SEDI
      v = cts[i].sedi();
      cts_at.set_entry(i+1, c++, v);

      // BAGSS
      v = cts[i].bagss();
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

void ModeExecutive::conf_read()
{
   
   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // If the merge config file was not set, use the match config file
   if(merge_config_file.length() == 0)
      merge_config_file = match_config_file;

   // List the config files
   mlog << Debug(3)
        << "Default Config File: " << default_config_file << "\n"
        << "Match Config File: "   << match_config_file   << "\n"
        << "Merge Config File: "   << merge_config_file   << "\n";

   // Read the config files
   engine.conf_info.read_config(default_config_file.c_str(), match_config_file.c_str());
}

///////////////////////////////////////////////////////////////////////

string ModeExecutive::stype(Processing_t t)
{
   string s;
   switch (t) {
   case MULTIVAR_SIMPLE:
      s = "Multivar Simple Objects";
      break;
   case MULTIVAR_SIMPLE_MERGE:
      s = "Multivar Simple Object Merge";
      break;
   case MULTIVAR_INTENSITY:
      s = "Multivar Intensity";
      break;
   case MULTIVAR_SUPER:
      s = "Multivar Superobjects";
      break;
   case TRADITIONAL:
   default:
      s = "Traditional";
      break;
   }
   return s;
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

   const int N = m_strlen(t);


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

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
   if(from.empty())
      return;
   size_t start_pos = 0;
   while((start_pos = str.find(from, start_pos)) != std::string::npos) {
      str.replace(start_pos, from.length(), to);
      start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
   }
}


