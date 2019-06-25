// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gsid2mpr.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    06/09/15  Bullock         New
//   001    01/26/16  Halley Gotway   Add -no_check_dup option.
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
#include "gsid2mpr.h"

////////////////////////////////////////////////////////////////////////

static void process_conv(const char *conv_filename, const char *output_filename);
static void process_rad (const char *rad_filename, const char *output_filename);

static void write_mpr_row_conv(AsciiTable &at, int row, const ConvData &d);
static void write_mpr_row_rad (AsciiTable &at, int row, const RadData  &d);

static bool is_dup(const char *);

static void usage();
static void set_swap(const StringArray &);
static void set_no_check_dup(const StringArray &);
static void set_channel(const StringArray &);
static void set_hdr(const StringArray &);
static void set_suffix(const StringArray &);
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []) {
   CommandLine cline;
   ConcatString output_filename;

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage
   cline.set_usage(usage);

   // Add options
   cline.add(set_swap,         "-swap",         0);
   cline.add(set_no_check_dup, "-no_check_dup", 0);
   cline.add(set_channel,      "-channel",      1);
   cline.add(set_hdr,          "-set_hdr",      2);
   cline.add(set_suffix,       "-suffix",       1);
   cline.add(set_outdir,       "-outdir",       1);
   cline.add(set_logfile,      "-log",          1);
   cline.add(set_verbosity,    "-v",            1);

   // Parse the command line
   cline.parse();

   // Check for zero files to process
   if(cline.n() == 0) usage();

   // Process each remaining argument
   for(int i=0; i<(cline.n()); i++) {

      mlog << Debug(1)
           << "\nReading: " << cline[i] << " ... " << (i + 1)
           << " of " << cline.n() << "\n";

      // Construct output file name
      output_filename << cs_erase
                      << output_directory << '/'
                      << get_short_name(cline[i].c_str()) << suffix;

      // Initialize output StatHdrColumns
      setup_header(shc, hdr_name, hdr_value, "MPR");

      // Initialize the observation key
      obs_key.clear();
      obs_key_map.clear();

      // Process by file type
      if(is_conv(cline[i].c_str())) process_conv(cline[i].c_str(), output_filename.c_str());
      else                          process_rad (cline[i].c_str(), output_filename.c_str());
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////
//
// Process conventional GSI data.
//
////////////////////////////////////////////////////////////////////////

void process_conv(const char *conv_filename, const char *output_filename) {
   int i;
   int n_in, n_out;
   ofstream out;
   AsciiTable at;
   bool uv_flag;

   ConvFile f;
   ConvRecord r;
   ConvData d, d_v;

   // Open input file
   if(!(f.open(conv_filename, swap_endian))) {
      mlog << Error << "\nprocess_conv() -> "
           << "can't open input file \"" << conv_filename << "\"\n\n";
      exit(1);
   }

   // Setup output AsciiTable
   at.set_size(f.n_pair() + 1,
               n_header_columns + n_mpr_columns + n_conv_extra_cols);
   setup_table(at);

   // Write header row
   write_header_row(mpr_columns, n_mpr_columns, 1, at, 0, 0);
   write_header_row(conv_extra_columns, n_conv_extra_cols, 0, at, 0,
                    n_header_columns + n_mpr_columns);

   mlog << Debug(2) << "Processing " << f.n_rec() << " records from "
        << conv_filename << ".\n";

   // Process each record
   n_in  = 0;
   n_out = 1; // 1 for header line
   while(f >> r) {

      mlog << Debug(3) << "Processing record " << n_in+1
           << " of " << f.n_rec() << ".\n";

      uv_flag = (str_trim(r.variable) == "uv");

      for(i=0; i<(r.ii); i++)  {

         // Parse the current convetional data
         d = parse_conv_data(r, i);

         // Handle uv pairs
         if(uv_flag) {
            d_v       = d;
            d.var     = "u";
            d_v.var   = "v";
            d_v.guess = d_v.guess_v;
            d_v.obs   = d_v.obs_v;

            if(!check_dup || (check_dup && !is_dup(get_conv_key(d).c_str()))) {
               write_mpr_row_conv(at, n_out++, d);
            }
            if(!check_dup || (check_dup && !is_dup(get_conv_key(d_v).c_str()))) {
               write_mpr_row_conv(at, n_out++, d_v);
            }
         }
         // Handle other variable types
         else {
            if(!check_dup || (check_dup && !is_dup(get_conv_key(d).c_str()))) {
               write_mpr_row_conv(at, n_out++, d);
            }
         }
      }

      n_in++;
   } // end while

   mlog << Debug(2) << "Read " << n_in << " records and wrote "
        << n_out << " lines.\n";

   mlog << Debug(1)
        << "\nWriting: " << output_filename << "\n";

   // Open output file
   out.open(output_filename);
   if(!out) {
      mlog << Error << "\nprocess_conv() -> "
           << "can't open output file \"" << output_filename << "\"\n\n";
      exit(1);
   }

   // Format and write AsciiTable to output file
   setup_table(at);
   out << at;

   // Close files
   f.close();
   out.close();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Process GSI radiance data.
//
////////////////////////////////////////////////////////////////////////

void process_rad(const char *rad_filename, const char *output_filename) {
   int i;
   int n_in, n_out;
   ofstream out;
   AsciiTable at;
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

   // Setup output AsciiTable
   at.set_size(f.n_rec() * i_channel.n_elements() + 1,
               n_header_columns + n_mpr_columns + n_rad_extra_cols);
   setup_table(at);

   // Write header row
   write_header_row(mpr_columns, n_mpr_columns, 1, at, 0, 0);
   write_header_row(rad_extra_columns, n_rad_extra_cols, 0, at, 0,
                    n_header_columns + n_mpr_columns);

   // Update header columns for microwave
   if(is_micro(rad_filename)) {
       write_header_row(micro_extra_columns, n_micro_extra_cols, 0, at, 0,
                        n_header_columns + n_mpr_columns + micro_extra_begin);
   }

   // Update header columns for retrievals
   if(is_retr(rad_filename)) {
       write_header_row(retr_extra_columns, n_retr_extra_cols, 0, at, 0,
                        n_header_columns + n_mpr_columns + retr_extra_begin);
   }

   // Process each record
   n_in  = 0;
   n_out = 1; // 1 for header line
   while(f >> r)  {

      mlog << Debug(3) << "Processing record " << n_in+1
           << " of " << f.n_rec() << ".\n";

      for(i=0; i<i_channel.n_elements(); i++) {
         d = parse_rad_data(r, i_channel[i]-1,
                            f.channel_val(i_channel[i]-1),
                            f.use_channel(i_channel[i]-1));

         if(!check_dup || (check_dup && !is_dup(get_rad_key(d).c_str()))) {
            write_mpr_row_rad(at, n_out++, d);
         }
      }

      n_in++;
   } // end while

   mlog << Debug(2) << "Read " << n_in << " records and wrote "
        << n_out << " lines.\n";

   mlog << Debug(1)
        << "\nWriting: " << output_filename << "\n";

   // Open output file
   out.open(output_filename);
   if(!out) {
      mlog << Error << "\nprocess_rad() -> "
           << "can't open output file \"" << output_filename << "\"\n\n";
      exit(1);
   }

   // Format and write AsciiTable to output file
   setup_table(at);
   out << at;

   // Close files
   f.close();
   out.close();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Write MPR output line for conventional data.
//
////////////////////////////////////////////////////////////////////////

void write_mpr_row_conv(AsciiTable &at, int row, const ConvData &d) {
   int col;

   // Update header for current data
   if(not_has_FCST_VALID_BEG) shc.set_fcst_valid_beg(d.fcst_ut);
   if(not_has_FCST_VALID_END) shc.set_fcst_valid_end(d.fcst_ut);
   if(not_has_OBS_VALID_BEG)  shc.set_obs_valid_beg(d.obs_ut);
   if(not_has_OBS_VALID_END)  shc.set_obs_valid_end(d.obs_ut);
   if(not_has_FCST_VAR)       shc.set_fcst_var(d.var);
   if(not_has_OBS_VAR)        shc.set_obs_var(d.var);
   if(not_has_OBTYPE)         shc.set_obtype(d.obtype.c_str());

   // Write header columns
   write_header_cols(shc, at, row);
   col = n_header_columns;

   // Write MPR columns
   at.set_entry(row, col++, 1);           // TOTAL
   at.set_entry(row, col++, 0);           // INDEX

   at.set_entry(row, col++, d.sid);       // OBS_SID
   at.set_entry(row, col++, d.lat);       // OBS_LAT
   at.set_entry(row, col++, d.lon);       // OBS_LON
   at.set_entry(row, col++, d.hgt);       // OBS_LVL
   at.set_entry(row, col++, d.elv);       // OBS_ELV

   at.set_entry(row, col++, d.guess);     // FCST
   at.set_entry(row, col++, d.obs);       // OBS
   at.set_entry(row, col++, d.obs_qc[0]); // OBS_QC
   at.set_entry(row, col++, na_str);      // CLIMO_MEAN
   at.set_entry(row, col++, na_str);      // CLIMO_STDEV
   at.set_entry(row, col++, na_str);      // CLIMO_CDF

   // Write extra columns
   at.set_entry(row, col++, d.prs);       // OBS_PRS

   at.set_entry(row, col++, d.err_in);    // OBS_ERR_IN
   at.set_entry(row, col++, d.err_adj);   // OBS_ERR_ADJ
   at.set_entry(row, col++, d.err_fin);   // OBS_ERR_FIN

   at.set_entry(row, col++, d.prep_use);  // PREP_USE
   at.set_entry(row, col++, d.anly_use);  // ANLY_USE

   at.set_entry(row, col++, d.setup_qc);  // SETUP_QC
   at.set_entry(row, col++, d.qc_wght);   // QC_WGHT

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Write MPR output line for radiance data.
//
////////////////////////////////////////////////////////////////////////

void write_mpr_row_rad(AsciiTable &at, int row, const RadData & d) {
   int col;

   // Update header for current data
   if(not_has_FCST_VALID_BEG) shc.set_fcst_valid_beg(d.fcst_ut);
   if(not_has_FCST_VALID_END) shc.set_fcst_valid_end(d.fcst_ut);
   if(not_has_OBS_VALID_BEG)  shc.set_obs_valid_beg(d.obs_ut);
   if(not_has_OBS_VALID_END)  shc.set_obs_valid_end(d.obs_ut);
   if(not_has_FCST_VAR)       shc.set_fcst_var(d.var);
   if(not_has_OBS_VAR)        shc.set_obs_var(d.var);

   // Write header columns
   write_header_cols(shc, at, row);
   col = n_header_columns;

   // Write MPR columns
   at.set_entry(row, col++, 1);             // TOTAL
   at.set_entry(row, col++, 0);             // INDEX

   at.set_entry(row, col++, na_str);        // OBS_SID
   at.set_entry(row, col++, d.lat);         // OBS_LAT
   at.set_entry(row, col++, d.lon);         // OBS_LON
   at.set_entry(row, col++, na_str);        // OBS_LVL
   at.set_entry(row, col++, d.elv);         // OBS_ELV

   at.set_entry(row, col++, d.guess);       // FCST
   at.set_entry(row, col++, d.obs);         // OBS
   at.set_entry(row, col++, d.obs_qc[0]);   // OBS_QC
   at.set_entry(row, col++, na_str);        // CLIMO_MEAN
   at.set_entry(row, col++, na_str);        // CLIMO_STDEV
   at.set_entry(row, col++, na_str);        // CLIMO_CDF

   // Write extra columns
   at.set_entry(row, col++, d.use);         // CHAN_USE
   at.set_entry(row, col++, d.scan_pos);    // SCAN_POS
   at.set_entry(row, col++, d.sat_znth);    // SAT_ZNTH
   at.set_entry(row, col++, d.sat_azmth);   // SAT_AZMTH

   at.set_entry(row, col++, d.sun_znth);    // SUN_ZNTH
   at.set_entry(row, col++, d.sun_azmth);   // SUN_AZMTH
   at.set_entry(row, col++, d.sun_glnt);    // SUN_GLNT

   at.set_entry(row, col++, d.frac_wtr);    // FRAC_WTR
   at.set_entry(row, col++, d.frac_lnd);    // FRAC_LND
   at.set_entry(row, col++, d.frac_ice);    // FRAC_ICE
   at.set_entry(row, col++, d.frac_snw);    // FRAC_SNW

   at.set_entry(row, col++, d.sfc_twtr);    // SFC_TWTR
   at.set_entry(row, col++, d.sfc_tlnd);    // SFC_TLND
   at.set_entry(row, col++, d.sfc_tice);    // SFC_TICE
   at.set_entry(row, col++, d.sfc_tsnw);    // SFC_TSNW

   at.set_entry(row, col++, d.tsoil);       // TSOIL
   at.set_entry(row, col++, d.soilm);       // SOILM

   at.set_entry(row, col++, d.land_type);   // LAND_TYPE
   at.set_entry(row, col++, d.frac_veg);    // FRAC_VEG
   at.set_entry(row, col++, d.snw_dpth);    // SNW_DPTH
   at.set_entry(row, col++, d.sfc_wind);    // SFC_WIND

   at.set_entry(row, col++, d.frac_cld);    // FRAC_CLD
   at.set_entry(row, col++, d.ctop_prs);    // CTOP_PRS

   at.set_entry(row, col++, d.tfnd);        // TFND
   at.set_entry(row, col++, d.twarm);       // TWARM
   at.set_entry(row, col++, d.tcool);       // TCOOL
   at.set_entry(row, col++, d.tzfnd);       // TZFND

   at.set_entry(row, col++, d.obs_err);     // OBS_ERR
   at.set_entry(row, col++, d.fcst_nobc);   // FCST_NOBC
   at.set_entry(row, col++, d.sfc_emis);    // SFC_EMIS
   at.set_entry(row, col++, d.stability);   // STABILITY
   at.set_entry(row, col++, d.prs_max_wgt); // PRS_MAX_WGT

   return;
}

////////////////////////////////////////////////////////////////////////

bool is_dup(const char *key) {
   bool dup;
   dup = false;
   int int_key = key_to_integer(key);
   StringArray key_array = obs_key_map[int_key];
   if (key_array.n_elements() > 0) {
      if (key_array.has(key)) {
         mlog << Warning
              << "\nSkipping duplicate entry for case \"" << key << "\"\n\n";
         dup = true;
      }
      else {
         key_array.add(key);
         obs_key_map[int_key] = key_array;
      }
   }
   else {
      key_array.add(key);
      obs_key_map[int_key] = key_array;
   }
   return(dup);
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tgsi_file_1 [gsi_file_2 ... gsi_file_n]\n"
        << "\t[-swap]\n"
        << "\t[-no_check_dup]\n"
        << "\t[-channel n]\n"
        << "\t[-set_hdr col_name value]\n"
        << "\t[-suffix string]\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"gsi_file\" is a binary GSI diagnostic file "
        << "(conventional or radiance) to be reformatted (required).\n"
        << "\t\t\"-swap\" to switch the endianness when reading the "
        << "input binary files (optional).\n"
        << "\t\t\"-no_check_dup\" to skip the checking for duplicate "
        << "matched pairs (optional).\n"
        << "\t\t\"-channel n\" overrides the default processing of all "
        << "radiance channels with a comma-separated list (optional).\n"
        << "\t\t\"-set_hdr col_name value\" specifies what should be "
        << "written to the output header columns (optional).\n"
        << "\t\t\"-suffix string\" overrides the default output filename "
        << "suffix (" << suffix << ") (optional).\n"
        << "\t\t\"-outdir path\" overrides the default output directory ("
        << output_directory << ") (optional).\n"
        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_swap(const StringArray & a) {
   swap_endian = true;
}

////////////////////////////////////////////////////////////////////////

void set_no_check_dup(const StringArray & a) {
   check_dup = false;
}

////////////////////////////////////////////////////////////////////////

void set_channel(const StringArray & a) {
   channel.add_css(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_hdr(const StringArray & a) {
   hdr_name.add_css(to_upper(a[0]));
   hdr_value.add_css(a[1]);
}

////////////////////////////////////////////////////////////////////////

void set_suffix(const StringArray & a) {
   suffix = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a) {
   output_directory = a[0];
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
