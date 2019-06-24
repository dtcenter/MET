// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gsidens2orank.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    07/09/15  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"
#include "config_constants.h"
#include "vx_stat_out.h"
#include "vx_log.h"

#include "read_fortran_binary.h"

#include "conv_record.h"
#include "rad_record.h"
#include "gsi_util.h"
#include "gsidens2orank.h"

////////////////////////////////////////////////////////////////////////

static void process_conv(const char *conv_filename, int i_mem);
static void process_conv_data(ConvData &d, int i_mem);

static void process_rad(const char *rad_filename, int i_mem);
static void process_rad_data(RadData &d, int i_mem);

static void write_orank();
static void write_orank_row_conv(AsciiTable &at, int row, int i_obs);
static void write_orank_row_rad (AsciiTable &at, int row, int i_obs);

static void add_key(const ConcatString &key);
static bool has_key(const ConcatString &key);
static bool has_key(const ConcatString &key, int & index);

static void check_int(int i1, int i2, const char *col, const ConcatString &key);
static void check_dbl(double d1, double d2, const char *col, const ConcatString &key);

static void usage();
static void set_out(const StringArray &);
static void set_ens_mean(const StringArray &);
static void set_swap(const StringArray &);
static void set_channel(const StringArray &);
static void set_rng_name(const StringArray &);
static void set_rng_seed(const StringArray &);
static void set_hdr(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []) {
   CommandLine cline;
   StringArray ens_file_list;

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage
   cline.set_usage(usage);

   // Add options
   cline.add(set_out,       "-out",      1);
   cline.add(set_ens_mean,  "-ens_mean", 1);
   cline.add(set_swap,      "-swap",     0);
   cline.add(set_channel,   "-channel",  1);
   cline.add(set_hdr,       "-set_hdr",  2);
   cline.add(set_rng_name,  "-rng_name", 1);
   cline.add(set_rng_seed,  "-rng_seed", 1);
   cline.add(set_logfile,   "-log",      1);
   cline.add(set_verbosity, "-v",        1);

   // Parse the command line
   cline.parse();

   // Check for output filename
   if(output_filename.length() == 0) {
      mlog << Error
           << "\nThe \"-out\" option is required.\n\n";
      usage();
   }

   // Check for zero files to process
   if(cline.n() == 0) {
      usage();
   }
   // Process one remaining argument as a filename
   else if(cline.n() == 1) {
      ens_file_list = parse_ascii_file_list(cline[0].c_str());
   }
   // Process multiple remaining arguments as a list of filenames
   else {
      for(int i=0; i<(cline.n()); i++) {
         ens_file_list.add(cline[i].c_str());
      }
   }

   // Set the expected number of ensemble members
   n_ens = ens_file_list.n_elements();

   // Initialize ensemble pair data object
   ens_pd.set_ens_size(n_ens);

   // Initialize the random number generator
   if(rng_name.length() == 0) rng_name = default_rng_name;
   if(rng_seed.length() == 0) rng_seed = default_rng_seed;
   rng_set(rng_ptr, rng_name.c_str(), rng_seed.c_str());

   // Process each ensemble member
   for(int i=0; i<ens_file_list.n_elements(); i++) {

      mlog << Debug(1)
           << "\nReading Ensemble Member: " << cline[i] << " ... "
           << (i + 1) << " of " << cline.n() << "\n";

      // Setup first pass
      if(i==0) {
         setup_header(shc, hdr_name, hdr_value, "ORANK");
         conv_flag  = is_conv(cline[0].c_str());
         retr_flag  = (!conv_flag) && is_retr(cline[0].c_str());
      }

      // Check for consistent ensemble member file types
      if(is_conv(cline[i].c_str()) != conv_flag) {
         mlog << Error
              << "\nThe ensemble binary GSI diagnostic files must all "
              << "be of the same type (conventional or radiance).\n\n.";
         exit(1);
      }

      // Process by file type
      if(conv_flag) process_conv(cline[i].c_str(), i);
      else          process_rad (cline[i].c_str(), i);
   }

   // Process ensemble mean file
   if(ens_mean_filename.length() > 0) {

      mlog << Debug(1)
           << "\nReading Ensemble Mean: " << ens_mean_filename << "\n";

      // Check for consistent ensemble mean file type
      if(is_conv(ens_mean_filename.c_str()) != conv_flag) {
         mlog << Error
              << "\nThe ensemble mean binary GSI diagnostic file must "
              << "be of the same type as the members.\n\n.";
         exit(1);
      }
      // Process by file type
      if(conv_flag) process_conv(ens_mean_filename.c_str(), -1);
      else          process_rad (ens_mean_filename.c_str(), -1);
   }

   // Write the output
   write_orank();

   return(0);
}

////////////////////////////////////////////////////////////////////////
//
// Process conventional GSI data.
//
////////////////////////////////////////////////////////////////////////

void process_conv(const char *conv_filename, int i_mem) {
   int i, n_in;

   ConvFile f;
   ConvRecord r;
   ConvData d, d_v;

   // Open input file
   if(!(f.open(conv_filename, swap_endian))) {
      mlog << Error << "\nprocess_conv() -> "
           << "can't open input file \"" << conv_filename << "\"\n\n";
      exit(1);
   }

   mlog << Debug(2) << "Processing " << f.n_rec() << " records from "
        << conv_filename << ".\n";

   // Process each record
   n_in = 0;
   while(f >> r) {
      for(i=0; i<(r.ii); i++)  {

         mlog << Debug(3) << "Processing record " << n_in+1
              << " of " << f.n_rec() << ".\n";

         // Parse the current conventional data
         d = parse_conv_data(r, i);

         // Handle uv pairs
         if(d.var == "uv") {
            d_v       = d;
            d.var     = "u";
            d_v.var   = "v";
            d_v.guess = d_v.guess_v;
            d_v.obs   = d_v.obs_v;
            process_conv_data(d, i_mem);
            process_conv_data(d_v, i_mem);
         }
         // Handle other variable types
         else {
            process_conv_data(d, i_mem);
         }
      } // end for i

      n_in++;
   } // end while

   mlog << Debug(2) << "Read " << n_in << " records.\n";

   // Close files
   f.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_conv_data(ConvData &d, int i_mem) {
   int i, i_obs;
   ConcatString cs;
   bool mn = (i_mem < 0);

   // Build current key
   ConcatString key = get_conv_key(d);

   // Add entry for new observation
   if(!has_key(key)) {

      // Store the current key
      add_key(key);

      // Store the current pair data
      conv_data.push_back(d);

      // Store the current observation info
      // Store default weight value of 1
      ens_pd.add_obs(d.sid.c_str(), d.lat, d.lon,
                     bad_data_double, bad_data_double,
                     d.obs_ut, d.prs, d.elv, d.obs, na_str,
                     bad_data_double, bad_data_double);

      // Initialize ensemble members and mean to bad data
      for(i=0; i<n_ens; i++) ens_pd.add_ens(i, bad_data_double);
      ens_pd.mn_na.add(bad_data_double);

   } // end if

   // Get the current observation index
   if(!has_key(key, i_obs)) {
      mlog << Error << "\nprocess_conv_data() -> "
           << "can't find entry for case \"" << key << "\"\n\n";
      exit(1);
   }

   // Check for consistentcy
   check_int(d.prep_use, conv_data[i_obs].prep_use, "PREP_USE", key);
   check_int(d.setup_qc, conv_data[i_obs].setup_qc, "SETUP_QC", key);

   // Store current ensemble data
   if(mn) {
      ens_pd.mn_na.set(i_obs, d.guess);
   }
   else {

      // Check for duplicates
      if(!is_bad_data(ens_pd.e_na[i_mem][i_obs])) {
         mlog << Warning
              << "\nSkipping duplicate entry for ensemble member "
              << i_mem + 1 << " case \"" << key << "\"\n\n";
         return;
      }

      // Store the ensemble member value
      ens_pd.e_na[i_mem].set(i_obs, d.guess);

      // Track the ensemble partial sums
      ens_pd.add_ens_var_sums(i_obs, d.guess);

      // Keep track of ensemble members using this observation
      if(d.anly_use == 1) conv_data[i_obs].n_use++;
   }

   // Keep track of unique quality control values
   if(!conv_data[i_obs].obs_qc.has(d.obs_qc[0])) {
      conv_data[i_obs].obs_qc.add(d.obs_qc[0]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Process GSI radiance data.
//
////////////////////////////////////////////////////////////////////////

void process_rad(const char *rad_filename, int i_mem) {
   int i, n_in;
   NumArray i_channel;
   ConcatString cs;

   RadFile f;
   RadRecord r;
   RadData d;

   // Open input file
   if(!(f.open(rad_filename, swap_endian))) {
      mlog << Error << "\nprocess_rad() -> "
           << "can't open input file \"" << rad_filename << "\"\n\n";
      exit(1);
   }

   // Process all channels, if not otherwise specified
   if(channel.n_elements() == 0) {
      for(i=0; i<f.n_channels(); i++) i_channel.add(i+1);

      mlog << Debug(2)
           << "Processing all " << i_channel.n_elements()
           << " channels from " << f.n_rec() << " records from "
           << rad_filename << ".\n";
   }
   else {

      // Find index for each requested channel
      for(i=0; i<f.n_channels(); i++) {
         if(channel.has(f.channel_val(i))) {
            i_channel.add(i+1);
            if(cs.nonempty()) cs << ", ";
            cs << f.channel_val(i);
         }
      }

      // Check for at least one matching channel
      if(i_channel.n_elements() == 0) {
         mlog << Error << "\nprocess_rad() -> "
              << "none of the requested channel numbers found in \""
              << rad_filename << "\".\n\n";
         exit(1);
      }

      mlog << Debug(2)
           << "Processing " << i_channel.n_elements() << " of "
           << channel.n_elements() << " requested channels ("
           << cs << ") from " << f.n_rec() << " records from "
           << rad_filename << ".\n";
   }

   // Process each record
   n_in = 0;
   while(f >> r) {

      mlog << Debug(3) << "Processing record " << n_in+1
           << " of " << f.n_rec() << ".\n";

      for(i=0; i<i_channel.n_elements(); i++) {

         // Parse the current radiance data
         d = parse_rad_data(r, i_channel[i]-1,
                            f.channel_val(i_channel[i]-1),
                            f.use_channel(i_channel[i]-1));

         process_rad_data(d, i_mem);
      }

      n_in++;
   } // end while

   mlog << Debug(2) << "Read " << n_in << " records.\n";

   // Close files
   f.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_rad_data(RadData &d, int i_mem) {
   int i, i_obs;
   ConcatString cs;
   bool mn = (i_mem < 0);

   // Build current key
   ConcatString key = get_rad_key(d);

   // Add entry for new observation
   if(!has_key(key)) {

      // Store the current key
      add_key(key);

      // Store the current pair data
      rad_data.push_back(d);

      // Store the current observation info
      // Store default weight value of 1
      ens_pd.add_obs(na_str, d.lat, d.lon,
                     bad_data_double, bad_data_double,
                     d.obs_ut, bad_data_double, d.elv, d.obs, na_str,
                     bad_data_double, bad_data_double);

      // Initialize ensemble members and mean to bad data
      for(i=0; i<n_ens; i++) ens_pd.add_ens(i, bad_data_double);
      ens_pd.mn_na.add(bad_data_double);

   } // end if

   // Get the current observation index
   if(!has_key(key, i_obs)) {
      mlog << Error << "\nprocess_rad_data() -> "
           << "can't find entry for case \"" << key << "\"\n\n";
      exit(1);
   }

   // Check for consistentcy
   check_int(d.use,       rad_data[i_obs].use,       "CHAN_USE",  key);
   check_int(d.scan_pos,  rad_data[i_obs].scan_pos,  "SCAN_POS",  key);
   check_dbl(d.sat_znth,  rad_data[i_obs].sat_znth,  "SAT_ZNTH",  key);
   check_dbl(d.sat_azmth, rad_data[i_obs].sat_azmth, "SAT_AZMTH", key);
   check_dbl(d.sun_znth,  rad_data[i_obs].sun_znth,  "SUN_ZNTH",  key);
   check_dbl(d.sun_azmth, rad_data[i_obs].sun_azmth, "SUN_AZMTH", key);
   check_dbl(d.sun_glnt,  rad_data[i_obs].sun_glnt,  "SUN_GLNT",  key);
   check_dbl(d.frac_wtr,  rad_data[i_obs].frac_wtr,  "FRAC_WTR",  key);
   check_dbl(d.frac_lnd,  rad_data[i_obs].frac_lnd,  "FRAC_LND",  key);
   check_dbl(d.frac_ice,  rad_data[i_obs].frac_ice,  "FRAC_ICE",  key);
   check_dbl(d.frac_snw,  rad_data[i_obs].frac_snw,  "FRAC_SNW",  key);
   check_dbl(d.sfc_twtr,  rad_data[i_obs].sfc_twtr,  "SFC_TWTR",  key);
   check_dbl(d.sfc_tlnd,  rad_data[i_obs].sfc_tlnd,  "SFC_TLND",  key);
   check_dbl(d.sfc_tice,  rad_data[i_obs].sfc_tice,  "SFC_TICE",  key);
   check_dbl(d.sfc_tsnw,  rad_data[i_obs].sfc_tsnw,  "SFC_TSNW",  key);
   check_dbl(d.tsoil,     rad_data[i_obs].tsoil,     "TSOIL",     key);
   check_dbl(d.soilm,     rad_data[i_obs].soilm,     "SOILM",     key);
   check_int(d.land_type, rad_data[i_obs].land_type, "LAND_TYPE", key);
   check_dbl(d.frac_veg,  rad_data[i_obs].frac_veg,  "FRAC_VEG",  key);
   check_dbl(d.snw_dpth,  rad_data[i_obs].snw_dpth,  "SNW_DPTH",  key);
   check_dbl(d.tfnd,      rad_data[i_obs].tfnd,      "TFND",      key);
   check_dbl(d.twarm,     rad_data[i_obs].twarm,     "TWARM",     key);
   check_dbl(d.tcool,     rad_data[i_obs].tcool,     "TCOOL",     key);
   check_dbl(d.tzfnd,     rad_data[i_obs].tzfnd,     "TZFND",     key);

   // Store current ensemble data
   if(mn) {
      ens_pd.mn_na.set(i_obs, d.guess);
   }
   else {

      // Check for duplicates
      if(!is_bad_data(ens_pd.e_na[i_mem][i_obs])) {
         mlog << Warning
              << "\nSkipping duplicate entry for ensemble member "
              << i_mem + 1 << " case \"" << key << "\"\n\n";
         return;
      }

      // Store the ensemble member value
      ens_pd.e_na[i_mem].set(i_obs, d.guess);

      // Track the ensemble partial sums
      ens_pd.add_ens_var_sums(i_obs, d.guess);

      // Keep track of ensemble members using this observation
      if(d.obs_qc[0] == 0) rad_data[i_obs].n_use++;
   }

   // Keep track of unique quality control values
   if(!rad_data[i_obs].obs_qc.has(d.obs_qc[0])) {
      rad_data[i_obs].obs_qc.add(d.obs_qc[0]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Write ORANK output.
//
////////////////////////////////////////////////////////////////////////

void write_orank() {
   int i, j, n_out;
   ofstream out;
   AsciiTable at;
   NumArray ens;

   int n_orank_cols = get_n_orank_columns(n_ens);
   int n_extra_cols = (conv_flag ? n_conv_extra_cols : n_rad_extra_cols);

   // Setup output AsciiTable
   at.set_size(ens_pd.n_obs + 1,
               n_header_columns + n_orank_cols + n_extra_cols);
   setup_table(at);

   // Write header row
   write_orank_header_row(1, n_ens, at, 0, 0);
   if(conv_flag) write_header_row(conv_extra_columns, n_conv_extra_cols, 0, at, 0,
                                  n_header_columns + n_orank_cols);
   else          write_header_row(rad_extra_columns, n_rad_extra_cols, 0, at, 0,
                                  n_header_columns + n_orank_cols);

   // Update header columns for retrievals
   if(retr_flag) {
       write_header_row(retr_extra_columns, n_retr_extra_cols, 0, at, 0,
                        n_header_columns + n_orank_cols + retr_extra_begin);
   }

   mlog << Debug(1)
        << "\nWriting: " << output_filename << "\n";

   // Open output file
   out.open(output_filename.c_str());
   if(!out) {
      mlog << Error << "\nwrite_orank_conv() -> "
           << "can't open output file \"" << output_filename << "\"\n\n";
      exit(1);
   }

   // Compute statistics
   ens_pd.compute_pair_vals(rng_ptr);
   ens_pd.compute_stats();

   // Compute ensemble mean, if necessary
   if(ens_mean_filename.length() == 0) {
      for(i=0; i<ens_pd.n_obs; i++) {
         for(j=0, ens.clear(); j<ens_pd.n_ens; j++) {
            if(!is_bad_data(ens_pd.e_na[j][i])) ens.add(ens_pd.e_na[j][i]);
         }
         if(ens.n_elements() > 0) ens_pd.mn_na.set(i, ens.mean());
      } // end for i
   } // end if

   // Write output for each observation
   n_out = 1; // 1 for header
   for(i=0; i<ens_pd.n_obs; i++) {

      // Write current output line
      if(conv_flag) write_orank_row_conv(at, n_out++, i);
      else          write_orank_row_rad (at, n_out++, i);
   } // end for i

   mlog << Debug(2) << "Wrote " << n_out << " lines.\n";

   // Format and write AsciiTable to output file
   setup_table(at);
   out << at;

   // Close files
   out.close();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Write ORANK output line for conventional data.
//
////////////////////////////////////////////////////////////////////////

void write_orank_row_conv(AsciiTable &at, int row, int i_obs) {
   int i, col;
   ConcatString cs;
   ConvData *d = &conv_data[i_obs];

   // Update header for current data
   if(!hdr_name.has("FCST_VALID_BEG")) shc.set_fcst_valid_beg(d->fcst_ut);
   if(!hdr_name.has("FCST_VALID_END")) shc.set_fcst_valid_end(d->fcst_ut);
   if(!hdr_name.has("OBS_VALID_BEG"))  shc.set_obs_valid_beg(d->obs_ut);
   if(!hdr_name.has("OBS_VALID_END"))  shc.set_obs_valid_end(d->obs_ut);
   if(!hdr_name.has("FCST_VAR"))       shc.set_fcst_var(d->var);
   if(!hdr_name.has("OBS_VAR"))        shc.set_obs_var(d->var);
   if(!hdr_name.has("OBTYPE"))         shc.set_obtype(d->obtype.c_str());

   // Write header columns
   write_header_cols(shc, at, row);
   col = n_header_columns;

   // Write ORANK columns
   at.set_entry(row, col++, 1);                        // TOTAL
   at.set_entry(row, col++, 0);                        // INDEX

   at.set_entry(row, col++, ens_pd.sid_sa[i_obs]);     // OBS_SID
   at.set_entry(row, col++, ens_pd.lat_na[i_obs]);     // OBS_LAT
   at.set_entry(row, col++, ens_pd.lon_na[i_obs]);     // OBS_LON
   at.set_entry(row, col++, ens_pd.lvl_na[i_obs]);     // OBS_LVL
   at.set_entry(row, col++, ens_pd.elv_na[i_obs]);     // OBS_ELV

   at.set_entry(row, col++, ens_pd.o_na[i_obs]);       // OBS
   at.set_entry(row, col++, ens_pd.pit_na[i_obs]);     // PIT
   at.set_entry(row, col++, nint(ens_pd.r_na[i_obs])); // RANK
   at.set_entry(row, col++, nint(ens_pd.v_na[i_obs])); // N_ENS_VLD
   at.set_entry(row, col++, nint(ens_pd.n_ens));       // N_ENS
   for(i=0; i<n_ens; i++) {                            // ENS_i
      at.set_entry(row, col++, ens_pd.e_na[i][i_obs]);
   }

   // Get list of unique obs_qc values
   d->obs_qc.sort_increasing();
   for(i=0; i<d->obs_qc.n_elements(); i++) {
      if(i==0) cs << d->obs_qc[i];
      else     cs << "," << d->obs_qc[i];
   }
   at.set_entry(row, col++, cs);                       // OBS_QC
   at.set_entry(row, col++, ens_pd.mn_na[i_obs]);      // ENS_MEAN
   at.set_entry(row, col++, bad_data_double);          // CLIMO
   at.set_entry(row, col++, ens_pd.spread_na[i_obs]);  // ENS_SPREAD

   at.set_entry(row, col++, bad_data_double);          // ENS_MEAN_OERR
   at.set_entry(row, col++, bad_data_double);          // SPREAD_OERR
   at.set_entry(row, col++, bad_data_double);          // SPREAD_PLUS_OERR

   // Write extra columns
   at.set_entry(row, col++, d->n_use);                 // N_USE
   at.set_entry(row, col++, d->prep_use);              // PREP_USE
   at.set_entry(row, col++, d->setup_qc);              // SETUP_QC

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Write ORANK output for radiance data.
//
////////////////////////////////////////////////////////////////////////

void write_orank_row_rad(AsciiTable &at, int row, int i_obs) {
   int i, col;
   ConcatString cs;
   RadData *d = &rad_data[i_obs];

   // Update header for current data
   if(!hdr_name.has("FCST_VALID_BEG")) shc.set_fcst_valid_beg(d->fcst_ut);
   if(!hdr_name.has("FCST_VALID_END")) shc.set_fcst_valid_end(d->fcst_ut);
   if(!hdr_name.has("OBS_VALID_BEG"))  shc.set_obs_valid_beg(d->obs_ut);
   if(!hdr_name.has("OBS_VALID_END"))  shc.set_obs_valid_end(d->obs_ut);
   if(!hdr_name.has("FCST_VAR"))       shc.set_fcst_var(d->var);
   if(!hdr_name.has("OBS_VAR"))        shc.set_obs_var(d->var);

   // Write header columns
   write_header_cols(shc, at, row);
   col = n_header_columns;

   // Write ORANK columns
   at.set_entry(row, col++, 1);                        // TOTAL
   at.set_entry(row, col++, 0);                        // INDEX

   at.set_entry(row, col++, na_str);                   // OBS_SID
   at.set_entry(row, col++, ens_pd.lat_na[i_obs]);     // OBS_LAT
   at.set_entry(row, col++, ens_pd.lon_na[i_obs]);     // OBS_LON
   at.set_entry(row, col++, bad_data_double);          // OBS_LVL
   at.set_entry(row, col++, ens_pd.elv_na[i_obs]);     // OBS_ELV

   at.set_entry(row, col++, ens_pd.o_na[i_obs]);       // OBS
   at.set_entry(row, col++, ens_pd.pit_na[i_obs]);     // PIT
   at.set_entry(row, col++, nint(ens_pd.r_na[i_obs])); // RANK
   at.set_entry(row, col++, nint(ens_pd.v_na[i_obs])); // N_ENS_VLD
   at.set_entry(row, col++, nint(ens_pd.n_ens));       // N_ENS
   for(i=0; i<n_ens; i++) {                            // ENS_i
      at.set_entry(row, col++, ens_pd.e_na[i][i_obs]);
   }

   // Get list of unique obs_qc values
   d->obs_qc.sort_increasing();
   for(i=0; i<d->obs_qc.n_elements(); i++) {
      if(i==0) cs << d->obs_qc[i];
      else     cs << "," << d->obs_qc[i];
   }
   at.set_entry(row, col++, cs);                       // OBS_QC
   at.set_entry(row, col++, ens_pd.mn_na[i_obs]);      // ENS_MEAN
   at.set_entry(row, col++, bad_data_double);          // CLIMO
   at.set_entry(row, col++, ens_pd.spread_na[i_obs]);  // ENS_SPREAD

   at.set_entry(row, col++, bad_data_double);          // ENS_MEAN_OERR
   at.set_entry(row, col++, bad_data_double);          // SPREAD_OERR
   at.set_entry(row, col++, bad_data_double);          // SPREAD_PLUS_OERR

   // Write extra columns
   at.set_entry(row, col++, d->n_use);                 // N_USE
   at.set_entry(row, col++, d->use);                   // CHAN_USE
   at.set_entry(row, col++, d->scan_pos);              // SCAN_POS
   at.set_entry(row, col++, d->sat_znth);              // SAT_ZNTH
   at.set_entry(row, col++, d->sat_azmth);             // SAT_AZMTH

   at.set_entry(row, col++, d->sun_znth);              // SUN_ZNTH
   at.set_entry(row, col++, d->sun_azmth);             // SUN_AZMTH
   at.set_entry(row, col++, d->sun_glnt);              // SUN_GLNT

   at.set_entry(row, col++, d->frac_wtr);              // FRAC_WTR
   at.set_entry(row, col++, d->frac_lnd);              // FRAC_LND
   at.set_entry(row, col++, d->frac_ice);              // FRAC_ICE
   at.set_entry(row, col++, d->frac_snw);              // FRAC_SNW

   at.set_entry(row, col++, d->sfc_twtr);              // SFC_TWTR
   at.set_entry(row, col++, d->sfc_tlnd);              // SFC_TLND
   at.set_entry(row, col++, d->sfc_tice);              // SFC_TICE
   at.set_entry(row, col++, d->sfc_tsnw);              // SFC_TSNW

   at.set_entry(row, col++, d->tsoil);                 // TSOIL
   at.set_entry(row, col++, d->soilm);                 // SOILM

   at.set_entry(row, col++, d->land_type);             // LAND_TYPE
   at.set_entry(row, col++, d->frac_veg);              // FRAC_VEG
   at.set_entry(row, col++, d->snw_dpth);              // SNW_DPTH

   at.set_entry(row, col++, d->tfnd);                  // TFND
   at.set_entry(row, col++, d->twarm);                 // TWARM
   at.set_entry(row, col++, d->tcool);                 // TCOOL
   at.set_entry(row, col++, d->tzfnd);                 // TZFND

   return;
}


////////////////////////////////////////////////////////////////////////

void add_key(const ConcatString &key) {
   int int_key = key_to_integer(key.c_str());
   StringArray key_array = obs_key_map[int_key];
   if (key_array.n_elements()) {
      if (key_array.has(key)) {
         mlog << Warning
              << "\nExist the key already for case \"" << key << "\"\n\n";
      }
      else {
         key_array.add(key);
         obs_key_map[int_key] = key_array;
      }
   }
   else {
      key_array.add(key);
      obs_key_map[int_key] = key_array;
      obs_index_map[int_key] = obs_key.n_elements();
   }
   obs_key.add(key);
}

////////////////////////////////////////////////////////////////////////

bool has_key(const ConcatString &key) {
   bool found_key;
   found_key = false;
   int int_key = key_to_integer(key.c_str());
   StringArray key_array = obs_key_map[int_key];
   if (key_array.n_elements()) {
      if (key_array.has(key)) {
         found_key = true;
      }
   }
   return(found_key);
}

////////////////////////////////////////////////////////////////////////

bool has_key(const ConcatString &key, int & index) {
   bool found_key;
   found_key = false;
   int int_key = key_to_integer(key.c_str());
   StringArray key_array = obs_key_map[int_key];
   if (key_array.n_elements()) {
      if (key_array.has(key)) {
         found_key = true;
         if (1 == key_array.n_elements()) {
            index = obs_index_map[int_key];
         }
         else {
            obs_key.has(key, index);
         }
      }
   }
   return(found_key);
}

////////////////////////////////////////////////////////////////////////

void check_int(int i1, int i2, const char *col, const ConcatString &key) {
   if(i1 != i2) {
      mlog << Warning
           << "\nThe \"" << col << "\" value unexpectedly changed ("
           << i1 << " != " << i2 << ") for case \"" << key << "\"\n\n";
   }
   return;
}

////////////////////////////////////////////////////////////////////////

void check_dbl(double d1, double d2, const char *col, const ConcatString &key) {
   if(!is_eq(d1, d2)) {
      mlog << Warning
           << "\nThe \"" << col << "\" value unexpectedly changed ("
           << d1 << " != " << d2 << ") for case \"" << key << "\"\n\n";
   }
   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tens_file_1 ... ens_file_n | ens_file_list\n"
        << "\t-out path\n"
        << "\t[-ens_mean path]\n"
        << "\t[-swap]\n"
        << "\t[-channel n]\n"
        << "\t[-rng_name str]\n"
        << "\t[-rng_seed str]\n"
        << "\t[-set_hdr col_name value]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"ens_file_1 ... ens_file_n\" is a list of ensemble "
        << "binary GSI diagnostic files (conventional or radiance) to be "
        << "reformatted (required).\n"
        << "\t\t\"ens_file_list\" is an ASCII file containing a list of "
        << "ensemble binary GSI diagnostic files (required).\n"
        << "\t\t\"-out path\" specifies the name of the output "
        << "\".stat\" file (required).\n"
        << "\t\t\"-ens_mean path\" is the ensemble mean binary GSI "
        << "diagnostic file (optional).\n"
        << "\t\t\"-swap\" to switch the endianness when reading the "
        << "input binary files (optional).\n"
        << "\t\t\"-channel n\" overrides the default processing of all "
        << "radiance channels with a comma-separated list (optional).\n"
        << "\t\t\"-rng_name str\" overrides the default random number "
        << "generator name (" << default_rng_name << ") (optional).\n"
        << "\t\t\"-rng_seed str\" overrides the default random number "
        << "generator seed (" << default_rng_seed << ") (optional).\n"
        << "\t\t\"-set_hdr col_name value\" specifies what should be "
        << "written to the output header columns (optional).\n"
        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_out(const StringArray & a) {
   output_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_ens_mean(const StringArray & a) {
   ens_mean_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_swap(const StringArray & a) {
   swap_endian = true;
}

////////////////////////////////////////////////////////////////////////

void set_channel(const StringArray & a) {
   channel.add_css(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_rng_name(const StringArray & a) {
   rng_name = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_rng_seed(const StringArray & a) {
   rng_seed = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_hdr(const StringArray & a) {
   hdr_name.add_css(to_upper(a[0]));
   hdr_value.add_css(a[1]);
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////
