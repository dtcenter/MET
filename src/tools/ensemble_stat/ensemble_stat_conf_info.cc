// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ensemble_stat_conf_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class EnsembleStatConfInfo
//
////////////////////////////////////////////////////////////////////////

EnsembleStatConfInfo::EnsembleStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

EnsembleStatConfInfo::~EnsembleStatConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::init_from_scratch() {

   // Initialize pointers
   ens_gci     = (GCInfo *)        0;
   ens_ta      = (ThreshArray *)   0;
   gc_pd       = (GCEnsPairData *) 0;
   msg_typ     = (char **)         0;
   interp_mthd = (InterpMthd *)    0;
   interp_wdth = (int *)           0;
   mask_wd     = (WrfData *)       0;
   mask_name   = (char **)         0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::clear() {
   int i;

   // Set counts to zero
   n_ens_var    = 0;
   max_n_thresh = 0;
   n_vx         = 0;
   n_msg_typ    = 0;
   n_interp     = 0;
   n_mask       = 0;
   n_mask_area  = 0;
   n_mask_sid   = 0;

   // Deallocate memory
   if(ens_gci)     { delete [] ens_gci;     ens_gci     = (GCInfo *)        0; }
   if(ens_ta)      { delete [] ens_ta;      ens_ta      = (ThreshArray *)   0; }
   if(gc_pd)       { delete [] gc_pd;       gc_pd       = (GCEnsPairData *) 0; }
   if(msg_typ)     { delete [] msg_typ;     msg_typ     = (char **)         0; }
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)    0; }
   if(interp_wdth) { delete [] interp_wdth; interp_wdth = (int *)           0; }
   if(mask_wd)     { delete [] mask_wd;     mask_wd     = (WrfData *)       0; }

   for(i=0; i<n_mask; i++) {
      if(mask_name[i]) {
         delete [] mask_name[i];
         mask_name[i] = (char *) 0;
      }
   }
   if(mask_name) { delete [] mask_name; mask_name = (char **) 0; }

   mask_sid.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::read_config(const char *default_file_name,
                                       const char *user_file_name,
                                       FileType ftype, FileType otype) {

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   // Process the configuration file
   process_config(ftype, otype);

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_config(FileType ftype, FileType otype) {
   int i, j, n, n_mthd, n_wdth, a, b;
   GCInfo gci;
   InterpMthd im;

   //
   // Conf: version
   //

   if(strncasecmp(conf.version().sval(), met_version,
      strlen(conf.version().sval())) != 0) {

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
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

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "The model name (\"" << conf.model().sval() 
           << "\") must be non-empty and contain no embedded "
           << "whitespace.\n\n" << flush;
      exit(1);
   }

   //
   // Conf: ens_field
   //

   // Parse out the GRIB codes to be verified
   n_ens_var = conf.n_ens_field_elements();

   if(n_ens_var == 0) {

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "At least one value must be provided "
           << "for ens_field.\n\n" << flush;
      exit(1);
   }

   // Allocate space to store the GRIB code information
   ens_gci = new GCInfo [n_ens_var];

   // Parse the ensemble field information
   for(i=0; i<n_ens_var; i++) {
      ens_gci[i].set_gcinfo(conf.ens_field(i).sval(),
         conf.grib_ptv().ival(), ftype);
   }

   //
   // Conf: ens_thresh
   //

   // If the ensemble relative frequency is requested, check that the
   // number of forecast threshold levels matches n_ens_var
   if(conf.output_flag(i_nc_freq).ival() != 0 &&
      conf.n_ens_thresh_elements()       != n_ens_var) {

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "When computing ensemble relative frequencies, "
           << "the number of ens_thresh levels provided must match "
           << "the number of fields provided in fcst_field.\n\n"
           << flush;
      exit(1);
   }

   // Allocate space to store the threshold information
   ens_ta = new ThreshArray [n_ens_var];

   // Parse the ensemble threshold information
   for(i=0,max_n_thresh=0; i<n_ens_var; i++) {
      ens_ta[i].parse_thresh_str(conf.ens_thresh(i).sval());

      // Keep track of the maximum number of thresholds
      if(ens_ta[i].n_elements() > max_n_thresh) {
         max_n_thresh = ens_ta[i].n_elements();
      }
   }

   //
   // Conf: vld_ens_thresh
   //

   // Check that the valid ensemble threshold is between 0 and 1.
   if(conf.vld_ens_thresh().dval() <= 0.0 ||
      conf.vld_ens_thresh().dval() >  1.0) {

         cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
              << "The valid ensemble threshold value must be set "
              << "greater than 0 and less than or equal to 1.\n\n"
              << flush;
         exit(1);
   }

   //
   // Conf: vld_data_thresh
   //

   // Check that the valid ensemble threshold is between 0 and 1.
   if(conf.vld_data_thresh().dval() <= 0.0 ||
      conf.vld_data_thresh().dval() >  1.0) {

         cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
              << "The valid data threshold value must be set "
              << "greater than 0 and less than or equal to 1.\n\n"
              << flush;
         exit(1);
   }

   //
   // Conf: fcst_field
   //

   // Parse out the GRIB codes to be verified
   n_vx = conf.n_fcst_field_elements();

   if(n_vx != 0) {

      // Allocate space to store the GRIB code information
      gc_pd = new GCEnsPairData [n_vx];

      // Parse the fcst field information
      for(i=0; i<n_vx; i++) {
         gci.set_gcinfo(conf.fcst_field(i).sval(),
            conf.grib_ptv().ival(), ftype);

         gc_pd[i].set_fcst_gci(gci);
      }
   }

   //
   // Conf: obs_field
   //

   // Check if the length of obs_field is non-zero and
   // not equal to n_vx
   if(conf.n_obs_field_elements() != 0 &&
      conf.n_obs_field_elements() != n_vx) {

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "The length of obs_field must be the same as the "
           << "length of fcst_field.\n\n" << flush;
      exit(1);
   }

   if(n_vx != 0) {

      // Check that at least one observation file has been specified
      if(otype == NoFileType) {

         cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
              << "Verification has been requested but no observation "
              << "files have been specified.\n\n"
              << flush;
         exit(1);
      }

      // Parse the obs field information
      for(i=0; i<n_vx; i++) {

         // If obs_field is empty, use fcst_field
         if(conf.n_obs_field_elements() == 0) {
            gci.set_gcinfo(conf.fcst_field(i).sval(),
               conf.grib_ptv().ival(), otype);
         }
         else {
            gci.set_gcinfo(conf.obs_field(i).sval(),
               conf.grib_ptv().ival(), otype);
         }

         gc_pd[i].set_obs_gci(gci);
      }
   }

   //
   // Conf: message_type
   //

   // Parse the number of message types to be used
   n_msg_typ = conf.n_message_type_elements();

   // Check that at least one PrepBufr message type is provided
   if(n_vx > 0 && n_msg_typ == 0) {
      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "At least one PrepBufr message type must be provided.\n\n"
           << flush;
      exit(1);
   }

   if(n_msg_typ > 0) {

      // Store the message types
      msg_typ = new char *[n_msg_typ];
      for(i=0; i<n_msg_typ; i++) {
         msg_typ[i] = new char [strlen(conf.message_type(i).sval())+1];
         strcpy(msg_typ[i], conf.message_type(i).sval());
      }

      // Check that each PrepBufr message type provided is valid
      for(i=0; i<n_msg_typ; i++) {

         if(strstr(vld_msg_typ_str, msg_typ[i]) == NULL) {
            cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
                 << "Invalid message type string provided ("
                 << conf.message_type(i).sval() << ").\n\n" << flush;
            exit(1);
         }
      }
   }

   //
   // Conf: interp_method
   //

   // Parse the number of interpolation methods to be used
   n_mthd = conf.n_interp_method_elements();

   // Check that at least one interpolation method is provided
   if(n_vx > 0 && n_mthd == 0) {

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "At least one interpolation method must be provided.\n\n"
           << flush;
      exit(1);
   }

   //
   // Conf: interp_width
   //

   // Parse the number of interpolation widths to be used
   n_wdth = conf.n_interp_width_elements();

   // Check that at least one interpolation width is provided
   if(n_vx > 0 && n_wdth == 0) {

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "At least one interpolation width must be provided.\n\n"
           << flush;
      exit(1);
   }

   if(n_wdth > 0 && n_mthd > 0) {

      // Compute the number of interpolation methods to be used
      n_interp = 0;

      // Check for nearest neighbor special case
      for(i=0, a=n_wdth; i<n_wdth; i++) {

         if(conf.interp_width(i).ival() == 1) {
            a--;
            n_interp++;
         }

         // Perform error checking on widths
         if(conf.interp_width(i).ival() < 1) {
            cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
                 << "The interpolation width values must be set "
                 << "greater than or equal to 1 ("
                 << conf.interp_width(i).ival() << ").\n\n" << flush;
            exit(1);
         }
      }

      // Check for bilinear interpolation special case
      for(i=0, b=n_mthd; i<n_mthd; i++) {

         im = string_to_interpmthd(conf.interp_method(i).sval());

         if(im == im_bilin) {
            b--;
            n_interp++;
         }
      }

      // Compute n_interp
      n_interp += a*b;

      // Allocate space for the interpolation methods and widths
      interp_mthd = new InterpMthd [n_interp];
      interp_wdth = new int        [n_interp];

      // Initialize the interpolation method count
      n = 0;

      // Check for the nearest neighbor special case
      for(i=0; i<n_wdth; i++) {
         if(conf.interp_width(i).ival() == 1) {
            interp_mthd[n] = im_uw_mean;
            interp_wdth[n] = 1;
            n++;
         }
      }

      // Check for the bilinear interpolation special case
      for(i=0; i<n_mthd; i++) {
         im = string_to_interpmthd(conf.interp_method(i).sval());
         if(im == im_bilin) {
            interp_mthd[n] = im_bilin;
            interp_wdth[n] = 2;
            n++;
         }
      }

      // Loop through the interpolation widths
      for(i=0; i<n_wdth; i++) {

         // Skip the nearest neighbor case
         if(conf.interp_width(i).ival() == 1) continue;

         // Loop through the interpolation methods
         for(j=0; j<n_mthd; j++) {
            im = string_to_interpmthd(conf.interp_method(j).sval());

            // Skip the bilinear interpolation case
            if(im == im_bilin) continue;

            // Store the interpolation method and width
            interp_mthd[n] = im;
            interp_wdth[n] = conf.interp_width(i).ival();
            n++;
         }
      }
   }

   //
   // Conf: interp_thresh
   //

   // Check that the interpolation threshold is set between 0 and 1
   if(conf.interp_thresh().dval() < 0.0 ||
      conf.interp_thresh().dval() > 1.0) {

         cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
              << "The interpolation threshold value must be set "
              << "between 0 and 1.\n\n" << flush;
         exit(1);
   }

   //
   // Conf: output_flag
   //

   // Make sure the output_flag is the expected length
   if(conf.n_output_flag_elements() != n_out) {

      cerr << "\n\nERROR: EnsembleStatConfInfo::process_config() -> "
           << "Found " << conf.n_output_flag_elements()
           << " elements in the output_flag but expected " << n_out
           << ".\n\n"
           << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::process_masks(const Grid &grid) {
   int i, n_mask_grid, n_mask_poly;
   StringArray mask_sid_sa;

   // Get the number of masking areas
   n_mask_grid = conf.n_mask_grid_elements();
   n_mask_poly = conf.n_mask_poly_elements();
   n_mask_area = n_mask_grid + n_mask_poly;

   // Get the number of masking station ID's
   parse_sid_mask(conf.mask_sid().sval(), mask_sid_sa);
   n_mask_sid = mask_sid_sa.n_elements();

   // Save the total number masks as a sum of the masking areas and
   // masking points
   n_mask = n_mask_area + n_mask_sid;

   // Check that at least one verification masking region is provided
   if(n_mask == 0) {
      cerr << "\n\nERROR: EnsemblesStatConfInfo::process_masks() -> "
           << "At least one grid, polyline, or station ID "
           << "masking region must be provided.\n\n" << flush;
      exit(1);
   }

   // Allocate space to store the masking information
   mask_wd   = new WrfData [n_mask_area];
   mask_name = new char *  [n_mask];

   // Parse out the masking grid areas
   for(i=0; i<n_mask_grid; i++) {
      parse_grid_mask(conf.mask_grid(i).sval(), grid,
                      mask_wd[i], mask_name[i]);
   }

   // Parse out the masking poly areas
   for(i=0; i<n_mask_poly; i++) {
      parse_poly_mask(conf.mask_poly(i).sval(), grid,
                      mask_wd[i+n_mask_grid], mask_name[i+n_mask_grid]);
   }

   // Store the masking station ID points
   for(i=0; i<n_mask_sid; i++) {
      mask_name[i+n_mask_area] = new char [strlen(mask_sid_sa[i])+1];
      strcpy(mask_name[i+n_mask_area], mask_sid_sa[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void EnsembleStatConfInfo::set_gc_pd() {
   int i, j;

   // EnsPairData is stored in the gc_pd objects in the following order:
   // [n_msg_typ][n_mask][n_interp]
   for(i=0; i<n_vx; i++) {

      // Set up the dimensions for the gc_pd object
      gc_pd[i].set_pd_size(n_msg_typ, n_mask, n_interp);

      // Add the verifying message type to the gc_pd objects
      for(j=0; j<n_msg_typ; j++)
         gc_pd[i].set_msg_typ(j, msg_typ[j]);

      // Add the masking information to the gc_pd objects
      for(j=0; j<n_mask; j++) {

         // If this is a masking area
         if(j<n_mask_area)
            gc_pd[i].set_mask_wd(j, mask_name[j], &mask_wd[j]);
         // Otherwise this is a masking StationID
         else
            gc_pd[i].set_mask_wd(j, mask_name[j], (WrfData *) 0);
      }

      // Add the interpolation methods to the gc_pd objects
      for(j=0; j<n_interp; j++)
         gc_pd[i].set_interp(j, interp_mthd[j], interp_wdth[j]);
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::n_txt_row(int i) {
   int n;

   // Switch on the index of the line type
   switch(i) {

      case(i_rhist):
         // Maximum number of Rank Histogram lines possible =
         //    Fields * Masks * Interpolations * Message Type [Point Obs]
         //    Fields * Masks * Interpolations [Grid Obs]
         n =   n_vx * n_mask * n_interp * n_msg_typ
             + n_vx * n_mask * n_interp;
         break;

      case(i_orank):
         // Compute the maximum number of matched pairs to be written
         // out by summing the number for each GCEnsPairData object
         for(i=0, n=0; i<n_vx; i++) {
            n += gc_pd[i].get_n_pair();
         }
         break;

      default:
         n = 0;
         break;
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int EnsembleStatConfInfo::n_stat_row() {
   int i, n;

   // Set the maximum number of STAT output lines by summing the counts
   // for the optional text files that have been requested
   for(i=0, n=0; i<n_txt; i++) {

      if(conf.output_flag(i).ival() > 0) n += n_txt_row(i);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
