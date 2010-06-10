// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "wavelet_stat_conf_info.h"

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
   wvlt_ptr      = (gsl_wavelet *) 0;
   wvlt_work_ptr = (gsl_wavelet_workspace *) 0;
   fcst_gci      = (GCInfo *) 0;
   obs_gci       = (GCInfo *) 0;
   fcst_ta       = (ThreshArray *) 0;
   obs_ta        = (ThreshArray *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::clear() {

   // Set counts to zero
   n_vx         = 0;
   max_n_thresh = 0;
   n_tile       = 0;
   tile_dim     = 0;
   n_scale      = 0;

   // Initialize the pad bounding box
   pad_bb.x_ll   = 0.0;
   pad_bb.y_ll   = 0.0;
   pad_bb.x_ur   = 0.0;
   pad_bb.y_ur   = 0.0;
   pad_bb.width  = 0.0;
   pad_bb.height = 0.0;

   tile_xll.clear();
   tile_yll.clear();

   // Deallocate memory
   if(wvlt_ptr)      { wavelet_free(wvlt_ptr);                }
   if(wvlt_work_ptr) { wavelet_workspace_free(wvlt_work_ptr); }

   if(fcst_gci) { delete [] fcst_gci; fcst_gci = (GCInfo *)      0; }
   if(obs_gci)  { delete [] obs_gci;  obs_gci  = (GCInfo *)      0; }
   if(fcst_ta)  { delete [] fcst_ta;  fcst_ta  = (ThreshArray *) 0; }
   if(obs_ta)   { delete [] obs_ta;   obs_ta   = (ThreshArray *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::read_config(const char *file_name) {

   // Call the config read routine
   conf.read(file_name);

   // Process the configuration file
   process_config();

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::process_config() {
   int i, k;
   gsl_wavelet_type wvlt_type;

   //
   // Conf: version
   //

   if(strcasecmp(conf.version().sval(), met_version) != 0) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The version number listed in the config file ("
           << conf.version().sval() << ") does not match the version "
           << "of the code (" << met_version << ").\n\n" << flush;
      exit(1);
   }

   //
   // Conf: model
   //

   if(strlen(conf.model().sval()) == 0 ||
      check_reg_exp(ws_reg_exp, conf.model().sval()) == true) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The model name (\"" << conf.model().sval()
           << "\") must be non-empty and contain no embedded "
           << "whitespace.\n\n" << flush;
      exit(1);
   }

   //
   // Conf: fcst_field
   //

   // Parse out the forecast fields to be verified
   n_vx = conf.n_fcst_field_elements();

   if(n_vx == 0) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "At least one value must be provided "
           << "for fcst_field.\n\n" << flush;
      exit(1);
   }

   // Allocate space to store the forecast field information
   fcst_gci = new GCInfo [n_vx];

   // Parse the fcst_field information
   for(i=0; i<n_vx; i++) {
      fcst_gci[i].set_gcinfo(conf.fcst_field(i).sval(),
         conf.grib_ptv().ival());

      // No support for WDIR
      if(fcst_gci[i].code == wdir_grib_code) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using wavelet_stat.\n\n"
              << flush;
         exit(1);
      }
   }

   //
   // Conf: obs_field
   //

   // Check if the length of obs_field is non-zero and
   // not equal to n_vx
   if(conf.n_obs_field_elements() != 0 &&
      conf.n_obs_field_elements() != n_vx) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The length of obs_field must be the same as the "
           << "length of fcst_field.\n\n" << flush;
      exit(1);
   }

   // Allocate space to store the observation field information
   obs_gci = new GCInfo [n_vx];

   // Parse the obs field information
   for(i=0; i<n_vx; i++) {

      // If obs_field is empty, use fcst_field
      if(conf.n_obs_field_elements() == 0) {
         obs_gci[i].set_gcinfo(conf.fcst_field(i).sval(),
            conf.grib_ptv().ival());
      }
      else {
         obs_gci[i].set_gcinfo(conf.obs_field(i).sval(),
            conf.grib_ptv().ival());
      }

      // No support for WDIR
      if(obs_gci[i].code == wdir_grib_code) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using grid_stat.\n\n"
              << flush;
         exit(1);
      }
   }

   //
   // Conf: fcst_thresh
   //

   // Check that the number of forecast threshold levels matches n_vx
   if(conf.n_fcst_thresh_elements() != n_vx) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The number of fcst_thresh levels provided must match the "
           << "number of fields provided in fcst_field.\n\n"
           << flush;
      exit(1);
   }

   // Allocate space to store the forecast threshold information
   fcst_ta = new ThreshArray [n_vx];

   // Parse the fcst threshold information
   for(i=0; i<n_vx; i++) {
      fcst_ta[i].parse_thresh_str(conf.fcst_thresh(i).sval());
   }

   //
   // Conf: obs_thresh
   //

   // Check if the length of obs_thresh is non-zero and
   // not equal to n_vx
   if(conf.n_obs_thresh_elements() != 0 &&
      conf.n_obs_thresh_elements() != n_vx) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The number obs_thresh levels provided must match the "
           << "number of fields provided in obs_field.\n\n"
           << flush;
      exit(1);
   }

   // Allocate space to store the threshold information
   obs_ta = new ThreshArray [n_vx];

   // Parse the obs threshold information
   for(i=0; i<n_vx; i++) {

      // If obs_thresh is empty, use fcst_thresh
      if(conf.n_obs_thresh_elements() == 0) {
         obs_ta[i].parse_thresh_str(conf.fcst_thresh(i).sval());
      }
      else {
         obs_ta[i].parse_thresh_str(conf.obs_thresh(i).sval());
      }
   }

   //
   // Compute max_n_thresh
   //
   for(i=0, max_n_thresh=0; i<n_vx; i++) {

      // Check for the same number of fcst and obs thresholds
      if(fcst_ta[i].n_elements() != obs_ta[i].n_elements()) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "The number of thresholds for each field in "
              << "fcst_thresh must match the number of thresholds "
              << "for each field in obs_thresh.\n\n"
              << flush;
         exit(1);
      }

      if(fcst_ta[i].n_elements() > max_n_thresh)
         max_n_thresh = fcst_ta[i].n_elements();
   }

   //
   // Conf: mask_missing_flag
   //

   i = conf.mask_missing_flag().ival();

   if(i < 0 || i > 3) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The mask_missing_flag must be set to an integer value "
           << "between 0 and 3!\n\n"
           << flush;
      exit(1);
   }

   //
   // Conf: grid_decomp_flag
   //

   i = conf.grid_decomp_flag().ival();

   if(i < 0 || i > 2) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The grid decomposition flag must be set "
           << "between 0 and 2.\n\n" << flush;
      exit(1);
   }

   //
   // Only check the tile dimension parameters if the
   // grid_decomp_flag = 1
   //
   if(conf.grid_decomp_flag().ival() == 1) {

      //
      // Conf: tile_xll and tile_yll
      //
      if(conf.n_tile_xll_elements() != conf.n_tile_yll_elements()) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "The number of entries in tile_xll must match "
              << "the number of entries in tile_yll.\n\n" << flush;
         exit(1);
      }

      //
      // Conf: tile_dim and n_scale
      //
      if((n_scale = get_pow2(conf.tile_dim().ival())) < 0) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "The tile_dim parameter must be set to an integer "
              << "power of 2 when requesting tiling.\n\n";
         exit(1);
      }
   }

   //
   // Conf: wavelet_flag
   //

   i = conf.wavelet_flag().ival();

   if(i < 0 || i > 6) {
      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "The wavelet_flag must be set to an integer value "
           << "between 0 and 6!\n\n"
           << flush;
      exit(1);
   }

   // Get the requested wavelet type
   switch(i) {

      case(0):
         wvlt_type = *(gsl_wavelet_haar);
         break;

      case(1):
         wvlt_type = *(gsl_wavelet_haar_centered);
         break;

      case(2):
         wvlt_type = *(gsl_wavelet_daubechies);
         break;

      case(3):
         wvlt_type = *(gsl_wavelet_daubechies_centered);
         break;

      case(4):
         wvlt_type = *(gsl_wavelet_bspline);
         break;

      case(5):
         wvlt_type = *(gsl_wavelet_bspline_centered);
         break;

      default:

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "Unexpected flag value of " << i << "!\n\n"
              << flush;
         exit(1);
         break;
   }

   //
   // Conf: wavelet_k
   //

   k = conf.wavelet_k().ival();

   // Haar Wavelets: Check for valid k
   if(i == 0 || i == 1) {
      if(k != 2) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "For Haar wavelets, wavelet_k must be set to 2.\n\n"
             << flush;
         exit(1);
      }
   }
   // Daubechies Wavelets: Check for valid k
   else if(i == 2 || i == 3) {
      if(k < 4 || k > 20 || k%2 == 1) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "For Daubechies wavelets, wavelet_k must be set to "
              << "an even integer between 4 and 20.\n\n"
              << flush;
         exit(1);
      }
   }
   // Bspline Wavelets: Check for valid k
   else if(i == 4 || i == 5) {
      if(k != 103 && k != 105 && k != 202 && k != 204 && k != 206 &&
         k != 208 && k != 301 && k != 303 && k != 305 && k != 307 &&
         k != 309) {

         cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
              << "For Daubechies wavelets, wavelet_k must be set to "
              << "one of: 103, 105, 202, 204, 206, 208, 301, 303, 305, "
              << "307, 309.\n\n"
              << flush;
         exit(1);
      }
   }

   // Set the wavelet pointer
   wvlt_ptr = wavelet_set(&wvlt_type, k);

   //
   // Conf: output_flag
   //

   // Make sure the output_flag is the expected length
   if(conf.n_output_flag_elements() != n_out) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "Unexpected number of elements in the output_flag "
           << "parameter.\n\n"
           << flush;
      exit(1);
   }

   // Check that at least one output STAT type is requested
   if(conf.output_flag(0).ival() <= flag_no_out) {

      cerr << "\n\nERROR: WaveletStatConfInfo::process_config() -> "
           << "At least one output STAT type must be requested.\n\n"
           << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void WaveletStatConfInfo::process_tiles(const Grid &grid, int v) {
   int i;

   // Handle the grid_decomp_flag by setting up the tiles to be used
   switch(conf.grid_decomp_flag().ival()) {

      // Tile the input data using tiles of dimension n by n where n
      // is the largest integer power of 2 less than the smallest
      // dimension of the input data and allowing no overlap.
      case(0):

         center_tiles(grid.nx(), grid.ny());

         if(v > 1) {
            cout << "\nTiling Method: Apply " << n_tile
                 << " tile(s) automatically computed "
                 << "with dimension = " << tile_dim
                 << " and lower-left (x, y) = ";

            for(i=0; i<n_tile; i++)
               cout << "(" << tile_xll[i] << ", " << tile_yll[i] << ") ";
            cout << "\n" << flush;
         }
         break;

      // Apply the tiles specified in the configuration file
      case(1):

         // Store the tile_xll and tile_yll values
         n_tile = conf.n_tile_xll_elements();
         for(i=0; i<n_tile; i++) {
            tile_xll.add(conf.tile_xll(i).ival());
            tile_yll.add(conf.tile_yll(i).ival());
         }

         // Store the tile dimension specified
         tile_dim = conf.tile_dim().ival();

         if(v > 1) {
            cout << "\nTiling Method: Apply " << n_tile
                 << " tile(s) specified in the configuration file "
                 << "with dimension = " << tile_dim
                 << " and lower-left (x, y) = ";

            for(i=0; i<n_tile; i++)
               cout << "(" << tile_xll[i] << ", " << tile_yll[i] << ") ";
            cout << "\n" << flush;
         }
         break;

      // Setup tiles for padding the input fields out to the nearest
      // integer power of two
      case(2):

         pad_tiles(grid.nx(), grid.ny());

         if(v > 1) {

            cout << "\nTiling Method: Apply " << n_tile
                 << " tile padded out "
                 << "to dimension = " << tile_dim
                 << " and lower-left (x, y) = ";

            for(i=0; i<n_tile; i++)
               cout << "(" << tile_xll[i] << ", " << tile_yll[i] << ") ";
            cout << "\n" << flush;
         }

         break;

      default:
         cerr << "\n\nERROR: WaveletStatConfInfo::process_tiles() -> "
              << "unexpected value for grid_decomp_flag: "
              << conf.grid_decomp_flag().ival() << "\n\n"
              << flush;
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
   pad_bb.x_ll   = x_offset;
   pad_bb.y_ll   = y_offset;
   pad_bb.x_ur   = pad_bb.x_ll + nx;
   pad_bb.y_ur   = pad_bb.y_ll + ny;
   pad_bb.width  = pad_bb.x_ur - pad_bb.x_ll;
   pad_bb.height = pad_bb.y_ur - pad_bb.y_ll;

   return;
}

////////////////////////////////////////////////////////////////////////

int WaveletStatConfInfo::n_isc_row() {
   int i, n;

   // Compute the number of output lines for each verification field
   for(i=0,n=0; i<n_vx; i++) {

      n += (n_scale + 2) * fcst_ta[i].n_elements() * n_tile;

      if(n_tile > 1) n += (n_scale + 2) * fcst_ta[i].n_elements();
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
   if(abs((long double) (nint(a) - a)) > 10E-5) p = -1;
   else                                         p = nint(a);

   return(p);
}

////////////////////////////////////////////////////////////////////////
