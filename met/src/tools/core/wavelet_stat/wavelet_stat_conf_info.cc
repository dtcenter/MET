// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "wavelet_stat_conf_info.h"
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class WaveletStatConfInfo
//
////////////////////////////////////////////////////////////////////////

WaveletStatConfInfo::WaveletStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

WaveletStatConfInfo::~WaveletStatConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::init_from_scratch() {

   // Initialize pointers
   fcst_info     = (VarInfo **)    0;
   obs_info      = (VarInfo **)    0;
   fcat_ta       = (ThreshArray *) 0;
   ocat_ta       = (ThreshArray *) 0;
   wvlt_ptr      = (gsl_wavelet *) 0;
   wvlt_work_ptr = (gsl_wavelet_workspace *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::clear() {
   int i;

   // Set counts to zero
   max_n_thresh = 0;
   n_tile       = 0;
   n_scale      = 0;

   model.clear();
   desc.clear();
   obtype.clear();
   mask_missing_flag = FieldType_None;
   grid_decomp_flag = GridDecompType_None;
   tile_dim = 0;
   tile_xll.clear();
   tile_yll.clear();
   pad_bb.set_llwh(0.0, 0.0, 0.0, 0.0);
   wvlt_type = WaveletType_None;
   // nc_pairs_flag = false;
   nc_info.set_all_true();
   ps_plot_flag = false;
   met_data_dir.clear();
   output_prefix.clear();
   version.clear();

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   // Deallocate memory
   if(wvlt_ptr)      { wavelet_free(wvlt_ptr);                }
   if(wvlt_work_ptr) { wavelet_workspace_free(wvlt_work_ptr); }
   if(fcat_ta)       { delete [] fcat_ta;   fcat_ta   = (ThreshArray *) 0; }
   if(ocat_ta)       { delete [] ocat_ta;   ocat_ta   = (ThreshArray *) 0; }

   // Clear fcst_info
   if(fcst_info) {
      for(i=0; i<n_vx; i++)
         if(fcst_info[i]) { delete fcst_info[i]; fcst_info[i] = (VarInfo *) 0; }
      delete fcst_info; fcst_info = (VarInfo **) 0;
   }

   // Clear obs_info
   if(obs_info) {
      for(i=0; i<n_vx; i++)
         if(obs_info[i]) { delete obs_info[i]; obs_info[i] = (VarInfo *) 0; }
      delete obs_info; obs_info = (VarInfo **) 0;
   }

   // Reset count
   n_vx = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::read_config(const char *default_file_name,
                                      const char *user_file_name) {
   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());
   conf.read(replace_path(config_map_data_filename).c_str());

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::process_config(GrdFileType ftype,
                                         GrdFileType otype) {
   int i, n;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *fcst_dict = (Dictionary *) 0;
   Dictionary *obs_dict  = (Dictionary *) 0;
   Dictionary *dict      = (Dictionary *) 0;
   Dictionary i_fdict, i_odict;
   gsl_wavelet_type type;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model);

   // Conf: obtype
   obtype = parse_conf_string(&conf, conf_key_obtype);

   // Conf: output_flag
   output_map = parse_conf_output_flag(&conf, txt_file_type, n_txt);

   // Populate the output_flag array with map values
   for(i=0,n=0; i<n_txt; i++) {
      output_flag[i] = output_map[txt_file_type[i]];
      if(output_flag[i] != STATOutputType_None) n++;
   }

   // Check for at least one output line type
   if(n == 0) {
      mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
           << "At least one output STAT type must be requested.\n\n";
      exit(1);
   }

   // Conf: fcst.field and obs.field
   fcst_dict = conf.lookup_array(conf_key_fcst_field);
   obs_dict  = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_vx = parse_conf_n_vx(fcst_dict);

   // Check for a valid number of verification tasks
   if(n_vx == 0 || parse_conf_n_vx(obs_dict) != n_vx) {
      mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_field
           << "\" must be non-zero and match the number in \""
           << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   // Allocate space based on the number of verification tasks
   fcst_info = new VarInfo *   [n_vx];
   obs_info  = new VarInfo *   [n_vx];
   fcat_ta   = new ThreshArray [n_vx];
   ocat_ta   = new ThreshArray [n_vx];

   // Initialize pointers
   for(i=0; i<n_vx; i++) fcst_info[i] = obs_info[i] = (VarInfo *) 0;

   // Parse the fcst and obs field information
   max_n_thresh = 0;
   for(i=0; i<n_vx; i++) {

      // Allocate new VarInfo objects
      fcst_info[i] = info_factory.new_var_info(ftype);
      obs_info[i]  = info_factory.new_var_info(otype);

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fcst_dict, i);
      i_odict = parse_conf_i_vx_dict(obs_dict, i);

      // Set the current dictionaries
      fcst_info[i]->set_dict(i_fdict);
      obs_info[i]->set_dict(i_odict);

      // Conf: desc
      desc.add(parse_conf_string(&i_odict, conf_key_desc));

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed forecast field number " << i+1 << ":\n";
         fcst_info[i]->dump(cout);
         mlog << Debug(5)
              << "Parsed observation field number " << i+1 << ":\n";
         obs_info[i]->dump(cout);
      }

      // No support for wind direction
      if(fcst_info[i]->is_wind_direction() ||
         obs_info[i]->is_wind_direction()) {
         mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using wavelet_stat.\n\n";
         exit(1);
      }

      // Conf: cat_thresh
      fcat_ta[i] = i_fdict.lookup_thresh_array(conf_key_cat_thresh);
      ocat_ta[i] = i_odict.lookup_thresh_array(conf_key_cat_thresh);

      // Dump the contents of the current thresholds
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed thresholds for field number "  << i+1 << "...\n"
              << "Forecast categorical thresholds: "    << fcat_ta[i].get_str() << "\n"
              << "Observed categorical thresholds: "    << ocat_ta[i].get_str() << "\n";
      }

      // Check for the same number of fcst and obs thresholds
      if(fcat_ta[i].n_elements() != ocat_ta[i].n_elements()) {

         mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
              << "The number of thresholds for each field in \"fcst."
              << conf_key_cat_thresh
              << "\" must match the number of thresholds for each "
              << "field in \"obs." << conf_key_cat_thresh << "\".\n\n";
         exit(1);
      }

      // Keep track of the maximum number of thresholds
      if(fcat_ta[i].n_elements() > max_n_thresh) max_n_thresh = fcat_ta[i].n_elements();

   } // end for i

   // Conf: mask_missing_flag
   mask_missing_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_missing_flag));

   // Conf: grid_decomp_flag
   grid_decomp_flag = parse_conf_grid_decomp_flag(&conf);

   // Only check tile definitions for GridDecompType_Tile
   if(grid_decomp_flag == GridDecompType_Tile) {

      // Conf: tile.width
      tile_dim = conf.lookup_int(conf_key_tile_width);

      // Compute the number of scales
      if((n_scale = get_pow2(tile_dim)) < 0) {
         mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
              << "The \"" << conf_key_tile_width
              << "\" parameter must be set to an integer power of 2.\n\n";
         exit(1);
      }

      // Conf: tile.location
      dict = conf.lookup_array(conf_key_tile_location);

      // Loop over the array entries
      for(i=0; i<dict->n_entries(); i++) {

         // Retrieve the x_ll and y_ll tile locations
         tile_xll.add((*dict)[i]->dict_value()->lookup_int(conf_key_x_ll));
         tile_yll.add((*dict)[i]->dict_value()->lookup_int(conf_key_y_ll));
      }
   }

   // Conf: wavelet.type
   wvlt_type = parse_conf_wavelet_type(&conf);

   // Process the wavelet type
   switch(wvlt_type) {
      case(WaveletType_Haar):         type = *(gsl_wavelet_haar); break;
      case(WaveletType_Haar_Cntr):    type = *(gsl_wavelet_haar_centered); break;
      case(WaveletType_Daub):         type = *(gsl_wavelet_daubechies); break;
      case(WaveletType_Daub_Cntr):    type = *(gsl_wavelet_daubechies_centered); break;
      case(WaveletType_BSpline):      type = *(gsl_wavelet_bspline); break;
      case(WaveletType_BSpline_Cntr): type = *(gsl_wavelet_bspline_centered); break;
      case(WaveletType_None):
      default:
         mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
              << "Unsupported wavelet type value of " << wvlt_type << ".\n\n";
         exit(1);
         break;
   }

   // Conf: wavelet.member
   wvlt_member = conf.lookup_int(conf_key_wavelet_member);

   // Check for valid member number
   switch(wvlt_type) {
      case(WaveletType_Haar):
      case(WaveletType_Haar_Cntr):
         if(wvlt_member != 2) {
            mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
                 << "For Haar wavelets, \"" << conf_key_wavelet_member
                 << "\" must be set to 2.\n\n";
            exit(1);
         }
         break;

      case(WaveletType_Daub):
      case(WaveletType_Daub_Cntr):
         if(wvlt_member < 4 || wvlt_member > 20 || wvlt_member%2 == 1) {
            mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
                 << "For Daubechies wavelets, \"" << conf_key_wavelet_member
                 << "\" must be set to an even integer between 4 and 20.\n\n";
            exit(1);
         }
         break;

      case(WaveletType_BSpline):
      case(WaveletType_BSpline_Cntr):
         if(wvlt_member != 103 && wvlt_member != 105 && wvlt_member != 202 &&
            wvlt_member != 204 && wvlt_member != 206 && wvlt_member != 208 &&
            wvlt_member != 301 && wvlt_member != 303 && wvlt_member != 305 &&
            wvlt_member != 307 && wvlt_member != 309) {
            mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
                 << "For BSpline wavelets, \"" << conf_key_wavelet_member
                 << "\" must be set to one of: 103, 105, 202, 204, 206, "
                 << "208, 301, 303, 305, 307, 309.\n\n";
            exit(1);
         }
         break;

      case(WaveletType_None):
      default:
         mlog << Error << "\nWaveletStatConfInfo::process_config() -> "
              << "Unsupported wavelet type value of " << wvlt_type << ".\n\n";
         exit(1);
         break;
   }

   // Initialize the requested wavelet
   wvlt_ptr = wavelet_set(&type, wvlt_member);

   // Conf: nc_pairs_flag
   parse_nc_info();

   // Conf: ps_plot_flag
   ps_plot_flag = conf.lookup_bool(conf_key_ps_plot_flag);

   // Conf: met_data_dir
   met_data_dir = replace_path(conf.lookup_string(conf_key_met_data_dir));

   // Conf: fcst_raw_plot
   fcst_raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_fcst_raw_plot));

   // Conf: obs_raw_plot
   obs_raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_obs_raw_plot));

   // Conf: wvlt_plot
   wvlt_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_wvlt_plot));

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::set_perc_thresh(const DataPlane &f_dp,
                                          const DataPlane &o_dp) {

   //
   // Compute percentiles for forecast and observation thresholds,
   // but not for wind speed or climatology CDF thresholds.
   //
   if(!fcat_ta->need_perc() && !ocat_ta->need_perc()) return;

   //
   // Sort the input arrays
   //
   NumArray fsort, osort;
   int nxy = f_dp.nx() * f_dp.ny();

   fsort.extend(nxy);
   osort.extend(nxy);

   for(int i=0; i<nxy; i++) {
      if(!is_bad_data(f_dp.data()[i])) fsort.add(f_dp.data()[i]);
      if(!is_bad_data(o_dp.data()[i])) osort.add(o_dp.data()[i]);
   }

   fsort.sort_array();
   osort.sort_array();

   //
   // Compute percentiles
   //
   fcat_ta->set_perc(&fsort, &osort, (NumArray *) 0, fcat_ta, ocat_ta);
   ocat_ta->set_perc(&fsort, &osort, (NumArray *) 0, fcat_ta, ocat_ta);

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::parse_nc_info() {
   const DictionaryEntry * e = (const DictionaryEntry *) 0;

   e = conf.lookup(conf_key_nc_pairs_flag);

   if(!e) {
      mlog << Error << "\nWaveletStatConfInfo::parse_nc_info() -> "
           << "lookup failed for key \"" << conf_key_nc_pairs_flag
           << "\"\n\n";
      exit(1);
   }

   const ConfigObjectType type = e->type();

   if(type == BooleanType) {
      bool value = e->b_value();
      if(!value) nc_info.set_all_false();
      return;
   }

   //
   //  it should be a dictionary
   //
   if(type != DictionaryType) {
      mlog << Error << "\nWaveletStatConfInfo::parse_nc_info() -> "
           << "bad type (" << configobjecttype_to_string(type)
           << ") for key \"" << conf_key_nc_pairs_flag << "\"\n\n";
      exit(1);
   }

   //
   //  parse the various entries
   //

   Dictionary * d = e->dict_value();

   nc_info.do_raw    = d->lookup_bool(conf_key_raw_flag);
   nc_info.do_diff   = d->lookup_bool(conf_key_diff_flag);

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::process_tiles(const Grid &grid) {
   int i;
   ConcatString msg;

   // Handle the grid_decomp_flag by setting up the tiles to be used
   switch(grid_decomp_flag) {

      // Tile the input data using tiles of dimension n by n where n
      // is the largest integer power of 2 less than the smallest
      // dimension of the input data and allowing no overlap.
      case(GridDecompType_Auto):

         center_tiles(grid.nx(), grid.ny());

         msg  << "\nTiling Method: Apply " << n_tile
              << " tile(s) automatically computed "
              << "with dimension = " << tile_dim
              << " and lower-left (x, y) = ";

         for(i=0; i<n_tile; i++)
            msg << "(" << tile_xll[i] << ", " << tile_yll[i] << ") ";
         msg << "\n";
         mlog << Debug(2) << msg;

         break;

      // Apply the tiles specified in the configuration file
      case(GridDecompType_Tile):

         // Number of tiles based on the user-specified locations
         n_tile = tile_xll.n_elements();

         msg  << "\nTiling Method: Apply " << n_tile
              << " tile(s) specified in the configuration file "
              << "with dimension = " << tile_dim
              << " and lower-left (x, y) = ";

         for(i=0; i<n_tile; i++)
            msg << "(" << tile_xll[i] << ", " << tile_yll[i] << ") ";
         msg << "\n";
         mlog << Debug(2) << msg;

         break;

      // Setup tiles for padding the input fields out to the nearest
      // integer power of two
      case(GridDecompType_Pad):

         pad_tiles(grid.nx(), grid.ny());

         msg  << "\nTiling Method: Apply " << n_tile
              << " tile padded out to dimension = "
              << tile_dim << " and lower-left (x, y) = ";

         for(i=0; i<n_tile; i++)
            msg << "(" << tile_xll[i] << ", " << tile_yll[i] << ") ";
         msg << "\n";
         mlog << Debug(2) << msg;

         break;

      case(GridDecompType_None):
      default:
         mlog << Error << "\nWaveletStatConfInfo::process_tiles() -> "
              << "Unsupported grid decomposition type of "
              << grid_decomp_flag << ".\n\n";
         exit(1);
         break;
   } // end switch

   // Compute n_scale based on tile_dim
   n_scale  = get_pow2(tile_dim);

   // Allocate space for the wavelet workspace
   wvlt_work_ptr = wavelet_workspace_set(tile_dim);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the tile dimension to be the largest integer power of 2 less
// than the smallest dimension of the input data.  Center as many tiles
// as possible without allowing overlap.
//
////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::center_tiles(int nx, int ny) {
   int m, i;
   int n_tile_x, n_tile_y, x_start, y_start;

   // Get the smallest dimension
   m = min(nx, ny);

   i = 0;
   while(pow(2.0, i) <= m) i++;

   // Set the tile dimension to be the largest power of 2 <= min(nx, ny)
   tile_dim = nint(pow(2.0, i-1));

   // Compute the number of tiles that will fit in the domain without
   // allowing overlap
   n_tile_x = nint(floor((double) nx/tile_dim));
   n_tile_y = nint(floor((double) ny/tile_dim));
   n_tile   = max(n_tile_x, n_tile_y);

   // Compute the lower-left coordinates for the tiles to be used
   x_start = nint((nx - n_tile_x*tile_dim)/2.0);
   y_start = nint((ny - n_tile_y*tile_dim)/2.0);

   // Tile in the x-direction
   if(nx >= ny) {
      for(i=0; i<n_tile; i++) {
         tile_xll.add(x_start + i*tile_dim);
         tile_yll.add(y_start);
      }
   }
   // Tile in the y-direction
   else {
      for(i=0; i<n_tile; i++) {
         tile_xll.add(x_start);
         tile_yll.add(y_start + i*tile_dim);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::pad_tiles(int nx, int ny) {
   int i, m, x_offset, y_offset;

   // Setup the tile
   n_tile = 1;
   tile_xll.add(0);
   tile_yll.add(0);

   // Get the larger setup_first_passgrid dimension
   m = max(nx, ny);

   // Find smallest power of two >= max_dim
   i = 0;
   while(pow(2.0, i) < m) i++;
   tile_dim = nint(pow(2.0, i));

   // Pad the field so that the original data is in the center or
   // the domain.  Choose (x_ll, y_ll) and (x_ur, y_ur) corner
   // points.
   x_offset = nint((tile_dim - nx)/2.0);
   y_offset = nint((tile_dim - ny)/2.0);

   // Setup the pad bounding box
   pad_bb.set_llwh(x_offset, y_offset, nx, ny);

   return;
}

////////////////////////////////////////////////////////////////////////

int WaveletStatConfInfo::n_isc_row() {
   int i, n;

   // Compute the number of output lines for each verification field
   for(i=0,n=0; i<n_vx; i++) {

      n += (n_scale + 2) * fcat_ta[i].n_elements() * n_tile;

      if(n_tile > 1) n += (n_scale + 2) * fcat_ta[i].n_elements();
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int WaveletStatConfInfo::n_stat_row() {

   return(n_isc_row());
}

////////////////////////////////////////////////////////////////////////

int get_pow2(double n) {
   double a;
   int p;

   // Take the log base 2
   a = log(n)/log(2.0);

   // Check whether or not the log base 2 is an integer
   if(fabs(nint(a) - a) > 10E-5) p = -1;
   else                          p = nint(a);

   return(p);
}

////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct WaveletStatNcInfo
   //


////////////////////////////////////////////////////////////////////////


WaveletStatNcOutInfo::WaveletStatNcOutInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void WaveletStatNcOutInfo::clear()

{

set_all_true();

return;

}


////////////////////////////////////////////////////////////////////////


bool WaveletStatNcOutInfo::all_false() const

{

bool status = do_raw || do_diff;

return ( !status );

}


////////////////////////////////////////////////////////////////////////


void WaveletStatNcOutInfo::set_all_false()

{

do_raw    = false;
do_diff   = false;

return;

}


////////////////////////////////////////////////////////////////////////


void WaveletStatNcOutInfo::set_all_true()

{

do_raw    = true;
do_diff   = true;

return;

}


////////////////////////////////////////////////////////////////////////



