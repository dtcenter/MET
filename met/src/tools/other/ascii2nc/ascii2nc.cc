// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   ascii2nc.cc
//
//   Description:
//      Parse ASCII observations and convert them to NetCDF.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-22-08  Halley Gotway  New
//   001    09-16-08  Halley Gotway  Keep track of the header values and
//                    only write out a header record when they change.
//   002    07-15-10  Halley Gotway  Store accumulation intervals in
//                    seconds rather than hours.
//   003    01-06-12  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//   005    08-01-12  Oldenburg      Added support for obs quality flag.
//   006    09-13-12  Halley Gotway  Added support for Little_r and
//                    factored out common code.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <regex.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "netcdf.hh"

#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name = "ascii2nc";
static const float fill_value   = -9999.f;
static const int   strl_len     = 16; // Length of "YYYYMMDD_HHMMSS"
static const int   hdr_arr_len  = 3;  // Observation header length
static const int   obs_arr_len  = 5;  // Observation values length

////////////////////////////////////////////////////////////////////////

// Supported input ASCII formats
enum ASCIIFormat {
   ASCIIFormat_None,
   ASCIIFormat_MET,
   ASCIIFormat_Little_R
};
static ASCIIFormat ascii_format = ASCIIFormat_None;

////////////////////////////////////////////////////////////////////////

// MET point observation format info
static const char *met_fmt_str   = "met_point";
static const int   n_met_col     = 10;
static const int   n_met_col_qty = 11;
static       int   n_file_col    = 0;

////////////////////////////////////////////////////////////////////////

// Little-R format info
static const char  *lr_fmt_str       = "little_r";
static const double lr_end_value     = -777777.0;
static const double lr_missing_value = -888888.0;

// Little-R fixed widths for the report lines
static const int lr_rpt_wdth[] = {
   20, 20, 40, 40, 40, 40, 20,
   10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 20,
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7,
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7,
   13,  7, 13,  7, 13,  7
};
static int n_lr_rpt_wdth = sizeof(lr_rpt_wdth)/sizeof(*lr_rpt_wdth);

// Little-R fixed widths for the data lines
static const int lr_meas_wdth[] = {
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7,
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7
};   
static int n_lr_meas_wdth = sizeof(lr_meas_wdth)/sizeof(*lr_meas_wdth);

// Little-R fixed widths for the end of report lines
static const int lr_end_wdth[] = {
    7,  7,  7
};
static int n_lr_end_wdth = sizeof(lr_end_wdth)/sizeof(*lr_end_wdth);

// GRIB codes for entries in the Little-R data lines
static const int lr_grib_codes[] = {
   1, 7, 11, 17, 32, 31, 33, 34, 52, bad_data_int
};

// Little-R regular expression used to determine file type
static const char *lr_rpt_reg_exp = "FM-[0-9]";
static const char *lr_rpt_sfc_str = "FM-12";
static const char *lr_rpt_upa_str = "FM-35";

////////////////////////////////////////////////////////////////////////

// Variables for command line arguments
static ConcatString asfile;
static ConcatString ncfile;
static int nhdr = 0;

////////////////////////////////////////////////////////////////////////

// Variables for writing output NetCDF file
NcFile *f_out       = (NcFile *) 0;
NcVar  *hdr_typ_var = (NcVar *)  0;
NcVar  *hdr_sid_var = (NcVar *)  0;
NcVar  *hdr_vld_var = (NcVar *)  0;
NcVar  *hdr_arr_var = (NcVar *)  0;
NcVar  *obs_qty_var = (NcVar *)  0;
NcVar  *obs_arr_var = (NcVar *)  0;

////////////////////////////////////////////////////////////////////////

static ASCIIFormat determine_ascii_format(LineDataFile &);

static void process_met_hdr(LineDataFile &);
static void process_met_obs(LineDataFile &);

static void process_little_r_hdr(LineDataFile &);
static void process_little_r_obs(LineDataFile &);

static void open_netcdf();
static void write_hdr_info(int &, const ConcatString &,
                           const ConcatString &, const ConcatString &,
                           double, double, double);
static void write_obs_info(int &, int, int, float, float, float,
                           const ConcatString &);
static void close_netcdf();

static void usage();
static void set_format(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   CommandLine cline;
   LineDataFile f_in;

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Check for zero arguments
   //
   if(argc == 1) usage();

   //
   // Parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // Set the usage function
   //
   cline.set_usage(usage);

   //
   // Add the options function calls
   //
   cline.add(set_format, "-format", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);

   //
   // Parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left:
   // ASCII input filename and the NetCDF output filename
   //
   if(cline.n() != 2) usage();

   //
   // Store the input ASCII file name and the output NetCDF file name
   //
   asfile = cline[0];
   ncfile = cline[1];

   //
   // Open the input ASCII observation file
   //
   if(!f_in.open(asfile)) {
      mlog << Error << "\nmain() -> "
           << "can't open input ASCII file \"" << asfile
           << "\" for reading\n\n";
      exit(1);
   }
   
   //
   // If the format was not set on the command line, try to guess it
   //
   if(ascii_format == ASCIIFormat_None) {
      ascii_format = determine_ascii_format(f_in);
   }
   
   //
   // Handle the different ASCII formats
   //
   if(ascii_format == ASCIIFormat_MET) {
      process_met_hdr(f_in);
      process_met_obs(f_in);
   }
   else if(ascii_format == ASCIIFormat_Little_R) {
      process_little_r_hdr(f_in);
      process_little_r_obs(f_in);
   }
   else {
      mlog << Error << "\nmain() -> "
           << "unsupported ASCII file format \"" << asfile
           << "\"\n\n";
      exit(1);
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////

ASCIIFormat determine_ascii_format(LineDataFile &f_in) {
   DataLine dl;
   ASCIIFormat format = ASCIIFormat_None;
   int i;

   //
   // Read the first line from the file
   //
   f_in >> dl;

   //
   // Check for a Little_R regular expression
   //
   for(i=0; i<dl.n_items(); i++) {
      if(check_reg_exp(lr_rpt_reg_exp, dl[i])) {
         format = ASCIIFormat_Little_R;
         break;
      }
   }

   //
   // Check for expected number of MET columns
   //
   if(format == ASCIIFormat_None &&
      (dl.n_items() == n_met_col || dl.n_items() == n_met_col_qty))
      format = ASCIIFormat_MET;

   return(format);
}

////////////////////////////////////////////////////////////////////////

void process_met_hdr(LineDataFile &f_in) {
   DataLine dl;
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   double hdr_lat, hdr_lon, hdr_elv;

   mlog << Debug(1) << "Reading MET ASCII Observation file: "
        << asfile << "\n";

   //
   // Rewind to the beginning of the file
   //
   f_in.rewind();
        
   //
   // Initialize the number of headers
   //
   nhdr = 0;

   //
   // Process each line of the file
   //
   while(f_in >> dl) {

      //
      // Store/check the number of columns in the first line
      //
      if(dl.line_number() == 1) {

         //
         // Make sure the number of columns is an acceptible format
         //
         if(dl.n_items() != n_met_col && dl.n_items() != n_met_col_qty) {
            mlog << Error << "\nprocess_met_hdr() -> "
                 << "line number " << dl.line_number()
                 << " does not contain the expected number of columns ("
                 << n_met_col << " or " << n_met_col_qty << ").\n\n";
            exit(1);
         }

         //
         // Store the column format
         //
         n_file_col = dl.n_items();
         if(n_file_col == n_met_col) {
            mlog << Debug(1) << "Found 10 column input file format deprecated "
                 << "in METv4.1 - consider adding quality flag value\n";
         }

         //
         // Increment the header count and store the first line
         //
         nhdr++;
         hdr_typ =      dl[0];
         hdr_sid =      dl[1];
         hdr_vld =      dl[2];
         hdr_lat = atof(dl[3]);
         hdr_lon = atof(dl[4]);
         hdr_elv = atof(dl[5]);

      }
      //
      // Check all following lines
      //
      else {

         //
         // Verify format consistency
         //
         if(dl.n_items() != n_file_col) {
            mlog << Error << "\nprocess_met_hdr() -> "
                 << "line number " << dl.line_number()
                 << " does not have the same number of columns as the "
                 << "first line (" << n_file_col << ").\n\n";
            exit(1);
         }

         //
         // Check to see if the header info has changed
         //
         if(hdr_typ           != dl[0]   ||
            hdr_sid           != dl[1]   ||
            hdr_vld           != dl[2]   ||
            !is_eq(hdr_lat, atof(dl[3])) ||
            !is_eq(hdr_lon, atof(dl[4])) ||
            !is_eq(hdr_elv, atof(dl[5]))) {

            //
            // Increment the header count and store the current values
            //
            nhdr++;
            hdr_typ =      dl[0];
            hdr_sid =      dl[1];
            hdr_vld =      dl[2];
            hdr_lat = atof(dl[3]);
            hdr_lon = atof(dl[4]);
            hdr_elv = atof(dl[5]);
         }
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void process_met_obs(LineDataFile &f_in) {
   DataLine dl;
   int i_obs, i_hdr, obs_idx;
   ConcatString hdr_typ, hdr_sid, hdr_vld, obs_qty;
   double hdr_lat, hdr_lon, hdr_elv, obs_prs, obs_hgt, obs_val;

   //
   // Open the NetCDF observation file for writing
   //
   open_netcdf();
   
   mlog << Debug(2) << "Processing observations for " << nhdr
        << " headers.\n";

   //
   // Rewind to the beginning of the file
   //
   f_in.rewind();
        
   //
   // Initialize counters to -1 to be incremented before writing
   //
   i_hdr = i_obs = -1;
        
   //
   // Process each line of the file
   //
   while(f_in >> dl) {

      //
      // Check for the first line of the file or the header changing
      //
      if(dl.line_number()  == 1       ||
         hdr_typ           != dl[0]   ||
         hdr_sid           != dl[1]   ||
         hdr_vld           != dl[2]   ||
         !is_eq(hdr_lat, atof(dl[3])) ||
         !is_eq(hdr_lon, atof(dl[4])) ||
         !is_eq(hdr_elv, atof(dl[5]))) {

         //
         // Store the header info
         //
         hdr_typ =      dl[0];
         hdr_sid =      dl[1];
         hdr_vld =      dl[2];
         hdr_lat = atof(dl[3]);
         hdr_lon = atof(dl[4]);
         hdr_elv = atof(dl[5]);

         //
         // Write the header info
         //
         write_hdr_info(i_hdr, hdr_typ, hdr_sid, hdr_vld,
                        hdr_lat, hdr_lon, hdr_elv);
      }

      //
      // Pressure level (hPa) or precip accumulation interval (sec)
      //
      obs_prs = (is_precip_grib_code(atoi(dl[6])) ?
                 timestring_to_sec(dl[7]) : atof(dl[7]));

      //
      // Observation height (meters above sea level)
      //
      obs_hgt = atof(dl[8]);

      //
      // Observation quality
      //
      obs_qty = (n_file_col == n_met_col ? "NA" : dl[9]);

      //
      // Observation value
      //
      obs_idx = (n_file_col == n_met_col ? 9 : 10);
      obs_val = atof(dl[obs_idx]);
             
      //
      // Write the observation info
      //
      write_obs_info(i_obs, i_hdr, atoi(dl[6]),
                     obs_prs, obs_hgt, obs_val, obs_qty);
                        
   } // end while

   mlog << Debug(2) << "Finished processing " << i_obs+1
        << " observations for " << i_hdr+1 << " headers.\n";

   //
   // Close the NetCDF file
   //
   close_netcdf();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_little_r_hdr(LineDataFile &f_in) {
   DataLine dl;

   mlog << Debug(1) << "Reading little_r ASCII Observation file: "
       << asfile << "\n";

   //
   // Rewind to the beginning of the file
   //
   f_in.rewind();
       
   //
   // Initialize the number of headers
   //
   nhdr = 0;

   //
   // Read the fixed-width lines:
   //   - one header report line
   //   - variable number of data lines
   //   - one end of report line
   //
   while(f_in.read_fwf_line(dl, lr_rpt_wdth, n_lr_rpt_wdth)) {

      //
      // Check for expected header line
      //
      if(!check_reg_exp(lr_rpt_reg_exp, dl[4])) {
         mlog << Error << "\nprocess_little_r_hdr() -> "
              << "the fifth entry of the little_r report on line "
              << dl.line_number() << " does not match \""
              << lr_rpt_reg_exp << "\":\n\"" << dl[4] << "\"\n\n";
         exit(1);
      }

      //
      // Increment the header count
      //
      nhdr++;

      //
      // Read the data lines
      //
      while(f_in.read_fwf_line(dl, lr_meas_wdth, n_lr_meas_wdth)) {

         //
         // Check for the end of report
         //
         if(is_eq(atof(dl[0]), lr_end_value) &&
            is_eq(atof(dl[2]), lr_end_value)) break;
      }

      //
      // Read the end of report line
      //
      f_in.read_fwf_line(dl, lr_end_wdth, n_lr_end_wdth);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_little_r_obs(LineDataFile &f_in) {
   DataLine dl;
   int i, i_obs, i_hdr, i_data, n_data_hdr;
   ConcatString cs, hdr_typ, hdr_sid, hdr_vld, obs_qty;
   double hdr_elv, obs_prs, obs_hgt, obs_val;

   //
   // Open the NetCDF observation file for writing
   //
   open_netcdf();

   mlog << Debug(2) << "Processing " << nhdr << " Little_r reports.\n";

   //
   // Rewind to the beginning of the file
   //
   f_in.rewind();
   
   //
   // Initialize counters to -1 to be incremented before writing
   //
   i_hdr = i_obs = -1;
   
   //
   // Read the fixed-width lines:
   //   - one header report line
   //   - variable number of data lines
   //   - one end of report line
   //
   while(f_in.read_fwf_line(dl, lr_rpt_wdth, n_lr_rpt_wdth)) {

      //
      // Check for expected header line
      //
      if(!check_reg_exp(lr_rpt_reg_exp, dl[4])) {
         mlog << Error << "\nprocess_little_r_obs() -> "
              << "the fifth entry of the little_r report on line "
              << dl.line_number() << " does not match \""
              << lr_rpt_reg_exp << "\":\n\"" << dl[4] << "\"\n\n";
         exit(1);
      }

      //
      // Store the message type, checking for special strings
      //
      if(strstr(dl[4], lr_rpt_sfc_str) != NULL) {
         hdr_typ = "ADPSFC";
      }
      else if(strstr(dl[4], lr_rpt_upa_str) != NULL) {
         hdr_typ = "ADPUPA";
      }
      else {
         hdr_typ = dl[4];
         hdr_typ.ws_strip();
         hdr_typ.replace(" ", "_");
      }

      //
      // Store the station id
      //
      hdr_sid = dl[2];
      hdr_sid.ws_strip();
      hdr_sid.replace(" ", "_");

      //
      // Store the valid time in YYYYMMDD_HHMMSS format
      //
      cs = dl[17];      
      cs.ws_strip();
      hdr_vld << cs_erase;
      hdr_vld.format("%.8s_%.6s", cs.text(), cs.text()+8);

      //
      // Store the elevation
      //
      hdr_elv = atof(dl[6]);

      //
      // Write the header info
      //
      write_hdr_info(i_hdr, hdr_typ, hdr_sid, hdr_vld,
                     atof(dl[0]), atof(dl[1]), hdr_elv);
      
      //
      // Store the number of data lines specified in the header
      //
      n_data_hdr = atoi(dl[7]);

      //
      // Observation of sea level pressure.
      // Convert from hectopascals to pascals.
      //
      if(!is_eq(atof(dl[18]), lr_missing_value)) {
         obs_qty = (is_eq(atof(dl[19]), lr_missing_value) ?
                    na_str : dl[19]);
         obs_qty.ws_strip();
         write_obs_info(i_obs, i_hdr, 2, bad_data_float, hdr_elv,
                        atof(dl[18])*10, obs_qty);
      }

      //
      // Read the data lines
      //
      i_data = 0;
      while(f_in.read_fwf_line(dl, lr_meas_wdth, n_lr_meas_wdth)) {

         //
         // Check for the end of report
         //
         if(is_eq(atof(dl[0]), lr_end_value) &&
            is_eq(atof(dl[2]), lr_end_value)) break;

         //
         // Retrieve pressure and height
         //
         obs_prs = (is_eq(atof(dl[0]), lr_missing_value) ?
                    bad_data_float : atof(dl[0]));
         obs_hgt = (is_eq(atof(dl[2]), lr_missing_value) ?
                    bad_data_float : atof(dl[2]));

         //
         // Convert pressure from hectopascals to millibars
         //
         if(!is_bad_data(obs_prs)) obs_prs /= 100;

         //
         // Store observations of:
         //    pressure, height, temperature, dew point,
         //    wind speed, wind direction, u-wind, v-wind,
         //    relative humidity
         //
         for(i=0; i<dl.n_items(); i++) {

            //
            // Only store valid observation values with a
            // valid corresponding GRIB code
            //
            if(!is_eq(atof(dl[i]), lr_missing_value) &&
               !is_bad_data(lr_grib_codes[i/2])) {

               //
               // Observation quality
               //
               obs_qty = (is_eq(atof(dl[i+1]), lr_missing_value) ?
                          na_str : dl[i+1]);
               obs_qty.ws_strip();

               //
               // Observation value
               //
               obs_val = atof(dl[i]);

               //
               // Convert pressure from hectopascals to pascals
               //
               if(lr_grib_codes[i/2] == 1) obs_val *= 10;
            
               //
               // Write the observation info
               //
               write_obs_info(i_obs, i_hdr, lr_grib_codes[i/2],
                              obs_prs, obs_hgt, obs_val, obs_qty);
            }

            //
            // Increment i to skip over the QC entry
            //
            i++;
         }

         //
         // Increment the data line count
         //
         i_data++;
      }

      //
      // Check that the number of data lines specified in the header
      // matches the number found
      //
      if(n_data_hdr != i_data) {
         mlog << Warning << "\nprocess_little_r_obs() -> "
              << "the number of data lines specified in the header ("
              << n_data_hdr
              << ") does not match the number found in the data ("
              << i_data << ") on line number " << dl.line_number() << ".\n\n";
      }

      //
      // Read the end of report line
      //
      f_in.read_fwf_line(dl, lr_end_wdth, n_lr_end_wdth);

   } // end while

   mlog << Debug(2) << "Finished processing " << i_obs+1
        << " observations from " << i_hdr+1 << " Little_r reports.\n";

   //
   // Close the NetCDF file
   //
   close_netcdf();

   return;
}

////////////////////////////////////////////////////////////////////////

void open_netcdf() {

   mlog << Debug(1) << "Creating NetCDF Observation file: "
        << ncfile << "\n";

   //
   // Create the output NetCDF file for writing
   //
   f_out = new NcFile(ncfile, NcFile::Replace);
   
   if(!f_out->is_valid()) {
      mlog << Error << "\nopen_netcdf() -> "
           << "can't open output NetCDF file \"" << ncfile
           << "\" for writing\n\n";
      close_netcdf();

      exit(1);
   }

   //
   // Define the NetCDF dimensions
   //
   NcDim *strl_dim    = f_out->add_dim("mxstr", (long) strl_len);
   NcDim *hdr_arr_dim = f_out->add_dim("hdr_arr_len", (long) hdr_arr_len);
   NcDim *obs_arr_dim = f_out->add_dim("obs_arr_len", (long) obs_arr_len);
   NcDim *hdr_dim     = f_out->add_dim("nhdr", (long) nhdr);
   NcDim *obs_dim     = f_out->add_dim("nobs"); // unlimited dimension

   //
   // Add variables to NetCDF file
   //
   hdr_typ_var = f_out->add_var("hdr_typ", ncChar, hdr_dim, strl_dim);
   hdr_sid_var = f_out->add_var("hdr_sid", ncChar, hdr_dim, strl_dim);
   hdr_vld_var = f_out->add_var("hdr_vld", ncChar, hdr_dim, strl_dim);
   hdr_arr_var = f_out->add_var("hdr_arr", ncFloat, hdr_dim, hdr_arr_dim);
   obs_qty_var = f_out->add_var("obs_qty", ncChar, obs_dim, strl_dim);
   obs_arr_var = f_out->add_var("obs_arr", ncFloat, obs_dim, obs_arr_dim);

   //
   // Add attributes to the NetCDF variables
   //
   hdr_typ_var->add_att("long_name", "message type");
   hdr_sid_var->add_att("long_name", "station identification");
   hdr_vld_var->add_att("long_name", "valid time");
   hdr_vld_var->add_att("units", "YYYYMMDD_HHMMSS UTC");

   hdr_arr_var->add_att("long_name", "array of observation station header values");
   hdr_arr_var->add_att("_fill_value", fill_value);
   hdr_arr_var->add_att("columns", "lat lon elv");
   hdr_arr_var->add_att("lat_long_name", "latitude");
   hdr_arr_var->add_att("lat_units", "degrees_north");
   hdr_arr_var->add_att("lon_long_name", "longitude");
   hdr_arr_var->add_att("lon_units", "degrees_east");
   hdr_arr_var->add_att("elv_long_name", "elevation ");
   hdr_arr_var->add_att("elv_units", "meters above sea level (msl)");

   obs_qty_var->add_att("long_name", "quality flag");

   obs_arr_var->add_att("long_name", "array of observation values");
   obs_arr_var->add_att("_fill_value", fill_value);
   obs_arr_var->add_att("columns", "hdr_id gc lvl hgt ob");
   obs_arr_var->add_att("hdr_id_long_name", "index of matching header data");
   obs_arr_var->add_att("gc_long_name", "grib code corresponding to the observation type");
   obs_arr_var->add_att("lvl_long_name", "pressure level (hPa) or accumulation interval (sec)");
   obs_arr_var->add_att("hgt_long_name", "height in meters above sea level or ground level (msl or agl)");
   obs_arr_var->add_att("ob_long_name", "observation value");

   //
   // Add global attributes
   //
   write_netcdf_global(f_out, ncfile.text(), program_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_hdr_info(int &i_hdr,
                    const ConcatString &hdr_typ,
                    const ConcatString &hdr_sid,
                    const ConcatString &hdr_vld,
                    double lat, double lon, double elv) {
   float hdr_arr[hdr_arr_len];

   //
   // Increment header count before writing
   //
   i_hdr++;
   
   //
   // Build the header array
   //
   hdr_arr[0] = lat;
   hdr_arr[1] = lon;
   hdr_arr[2] = elv;
   
   //
   // Store the message type
   //
   if(!hdr_typ_var->set_cur(i_hdr, (long) 0) ||
      !hdr_typ_var->put(hdr_typ, (long) 1, (long) hdr_typ.length())) {
      mlog << Error << "\nwrite_hdr_info() -> "
           << "error writing the message type to the NetCDF file\n\n";
      exit(1);
   }

   //
   // Store the station id
   //
   if(!hdr_sid_var->set_cur(i_hdr, (long) 0) ||
      !hdr_sid_var->put(hdr_sid, (long) 1, (long) hdr_sid.length())) {
      mlog << Error << "\nwrite_hdr_info() -> "
           << "error writing the station id to the NetCDF file\n\n";
      exit(1);
   }

   //
   // Store the valid time and check that it's is in the expected
   // time format: YYYYMMDD_HHMMSS
   //
   if(check_reg_exp(yyyymmdd_hhmmss_reg_exp, hdr_vld) != true) {
      mlog << Error << "\nwrite_hdr_info() -> "
           << "valid time is not in the expected YYYYMMDD_HHMMSS format: "
           << hdr_vld << "\n\n";
      exit(1);
   }

   if(!hdr_vld_var->set_cur(i_hdr, (long) 0) ||
      !hdr_vld_var->put(hdr_vld, (long) 1, (long) hdr_vld.length())) {
      mlog << Error << "\nwrite_hdr_info() -> "
           << "error writing the valid time to the NetCDF file\n\n";
      exit(1);
   }

   //
   // Store the header array
   //
   if(!hdr_arr_var->set_cur(i_hdr, (long) 0) ||
      !hdr_arr_var->put(hdr_arr, (long) 1, (long) hdr_arr_len) ) {
      mlog << Error << "\nwrite_hdr_info() -> "
           << "error writing the header array to the NetCDF file\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_obs_info(int &i_obs, int i_hdr,
                    int gc, float prs, float hgt, float obs,
                    const ConcatString &qty) {
   float obs_arr[obs_arr_len];
   
   //
   // Increment observation count before writing
   //
   i_obs++;
   
   //
   // Build the observation array
   //
   obs_arr[0] = i_hdr; // Index of header
   obs_arr[1] = gc;    // GRIB code
   obs_arr[2] = prs;   // Pressure level
   obs_arr[3] = hgt;   // Height level = elevation of station
   obs_arr[4] = obs;   // Observation value

   //
   // Write the observation array
   //
   if(!obs_arr_var->set_cur(i_obs, (long) 0) ||
      !obs_arr_var->put(obs_arr, (long) 1, (long) obs_arr_len) ) {
      mlog << Error << "\nwrite_obs_info() -> "
           << "error writing the observation array to the NetCDF file\n\n";
      exit(1);
   }

   //
   // Write the observation flag value
   //
   if(!obs_qty_var->set_cur(i_obs, (long) 0) ||
      !obs_qty_var->put(qty, (long) 1, (long) qty.length()) ) {
      mlog << Error << "\nnwrite_obs_info() -> "
           << "error writing the quality flag to the NetCDF file\n\n";
      exit(1);
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////

void close_netcdf() {
   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tascii_file\n"
        << "\tnetcdf_file\n"
        << "\t[-format ASCII_format]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"ascii_file\" is the formatted ASCII "
        << "observation file to be converted to NetCDF format "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output "
        << "NetCDF file to be written (required).\n"

        << "\t\t\"-format ASCII_format\" may be set to \"" << met_fmt_str
        << "\" or \"" << lr_fmt_str << "\" (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"

        << "\tThe \"" << met_fmt_str
        << "\" ASCII format consists of 11 columns:\n"
        << "\t\tMessage_Type Station_ID Valid_Time(YYYYMMDD_HHMMSS)\n"
        << "\t\tLat(Deg North) Lon(Deg East) Elevation(msl)\n"
        << "\t\tGrib_Code Level Height(msl or agl) QC_String Observation_Value\n\n"

        << "\t\twhere\t\"Level\" is the pressure level (hPa) or "
        << "accumulation interval (HH[MMSS]).\n"
        << "\t\t\t\"Height\" is meters above sea level or above ground level for the "
        << "observation (msl or agl).\n\n"

        << "\t\t\tUse a value of \"" << bad_data_int
        << "\" to indicate missing data.\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_format(const StringArray & a) {
  
   if(strcmp(met_fmt_str, a[0]) == 0) {
      ascii_format = ASCIIFormat_MET;
   }
   else if(strcmp(lr_fmt_str, a[0]) == 0) {
      ascii_format = ASCIIFormat_Little_R;
   }
   else {
      mlog << Error << "\nset_format() -> "
           << "unsupported ASCII observation format \""
           << a[0] << "\".\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
