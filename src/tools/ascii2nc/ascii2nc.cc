// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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
//      Parse ASCII observations in the following format:
//         sid typ vld lat lon elv pres gc ob
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-22-08  Halley Gotway  New
//   001    09-16-08  Halley Gotway  Keep track of the header values and
//                    only write out a header record when they change.
//   002    07-15-10  Halley Gotway  Store accumulation intervals in
//                    seconds rather than hours.
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

#include "vx_analysis_util.h"
#include "vx_met_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name = "ascii2nc";
static const float fill_value   = -9999.f;
static const int   strl_len     = 16; // Length of "YYYYMMDD_HHMMSS"
static const int   hdr_arr_len  = 3;  // Observation header length
static const int   obs_arr_len  = 5;  // Observation values length

// Supported input ASCII formats
enum ASCIIFormat {
   met_point = 0
};
static ASCIIFormat ascii_format = met_point;

// MET point observation format info
static const char *met_fmt_str = "met_point";
static const int   n_met_col   = 10;

// Variables for command line arguments
static ConcatString asfile;
static ConcatString ncfile;
static int verbosity = 1;
static int nhdr      = 0;
static int nrow      = 0;

////////////////////////////////////////////////////////////////////////

static void open_netcdf(NcFile *&);
static void open_met_ascii(LineDataFile &);
static void write_met_obs(LineDataFile &, NcFile *&);
static void usage(int, char **);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int i;

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Input and output files
   //
   LineDataFile  f_in;
   NcFile       *f_out = (NcFile *) 0;

   //
   // Check for the correct number of arguments
   //
   if(argc < 3) {
      usage(argc, argv);
      exit(1);
   }

   //
   // Store the input ASCII file name and the output NetCDF file name
   //
   asfile = argv[1];
   ncfile = argv[2];

   //
   // Parse command line arguments
   //
   for(i=0; i<argc; i++) {

      if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
         i++;
      }

      if(strcmp(argv[i], "-format") == 0) {

         if(strcmp(met_fmt_str, argv[i+1]) == 0) {
            ascii_format = met_point;
         }
         else {
            cerr << "\n\nmain() -> "
                 << "unsupported ASCII observation format \""
                 << argv[i+1] << "\".\n\n" << flush;
            exit(1);
         }
         i++;
      }
      else if(argv[i][0] == '-') {
         cerr << "\n\nERROR: main() -> "
              << "unrecognized command line switch: "
              << argv[i] << "\n\n" << flush;
         exit(1);
      }
   } // end for i

   //
   // Open the NetCDF observation file for writing
   //
   open_netcdf(f_out);

   if(ascii_format == met_point) {

      //
      // Open the MET ASCII observation file for reading
      //
      open_met_ascii(f_in);

      //
      // Process the MET ASCII observations: read, reformat, and write
      //
      write_met_obs(f_in, f_out);
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////

void open_netcdf(NcFile *&f_out) {

   if(verbosity > 0) {
      cout << "Creating NetCDF Observation file: " << ncfile << "\n"
           << flush;
   }

   f_out = new NcFile(ncfile, NcFile::Replace);

   //
   // Create the output netCDF file for writing
   //
   if(!f_out->is_valid()) {
      cerr << "\n\nERROR: open_netcdf() -> "
           << "can't open output NetCDF file \"" << ncfile
           << "\" for writing\n\n" << flush;
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void open_met_ascii(LineDataFile &f_in) {
   DataLine dl;
   ConcatString hdr_typ, prev_hdr_typ;
   ConcatString hdr_sid, prev_hdr_sid;
   ConcatString hdr_vld, prev_hdr_vld;
   float hdr_arr[hdr_arr_len], prev_hdr_arr[hdr_arr_len];

   if(verbosity > 0) {
      cout << "Reading ASCII Observation file: " << asfile << "\n"
           << flush;
   }

   //
   // Open the input ASCII observation file
   //
   if(!f_in.open(asfile)) {
      cerr << "\n\nopen_met_ascii() -> "
           << "can't open input ASCII file \"" << asfile
           << "\" for reading\n\n" << flush;
      exit(1);
   }

   //
   // Initialize the nrow and nhdr counts
   //
   nrow = nhdr = 0;

   //
   // Figure out the number of observation lines and unique header
   // values in the file
   //
   while(f_in >> dl) {

      //
      // Check that the line contains the expected number of columns
      //
      if(dl.n_items() != n_met_col) {
         cerr << "\n\nopen_met_ascii() -> "
              << "line number " << nrow << " doesn't contain the "
              << "expected number of columns (" << n_met_col
              << "):\n" << flush;

              dl.dump(cout);

              cout << "\n\n" << flush;
         exit(1);
      }

      //
      // Store the PrepBufr message type, station id, valid time,
      // and header array
      //
      hdr_typ    = dl[0];
      hdr_sid    = dl[1];
      hdr_vld    = dl[2];
      hdr_arr[0] = atof(dl[3]); // Lat
      hdr_arr[1] = atof(dl[4]); // Lon
      hdr_arr[2] = atof(dl[5]); // Elevation

      //
      // If this is the first row, store the data
      //
      if(nrow == 0) {
         prev_hdr_typ    = hdr_typ;
         prev_hdr_sid    = hdr_sid;
         prev_hdr_vld    = hdr_vld;
         prev_hdr_arr[0] = hdr_arr[0]; // Lat
         prev_hdr_arr[1] = hdr_arr[1]; // Lon
         prev_hdr_arr[2] = hdr_arr[2]; // Elevation
         nhdr++;
      }
      //
      // Otherwise, check to see if the header data has changed, and
      // if so, increment the header count
      //
      else {
         if(hdr_typ         != prev_hdr_typ     ||
            hdr_sid         != prev_hdr_sid     ||
            hdr_vld         != prev_hdr_vld     ||
            !is_eq(prev_hdr_arr[0], hdr_arr[0]) ||
            !is_eq(prev_hdr_arr[1], hdr_arr[1]) ||
            !is_eq(prev_hdr_arr[2], hdr_arr[2])) {

            //
            // Increment the header count and store the previous values
            //
            nhdr++;
            prev_hdr_typ    = hdr_typ;
            prev_hdr_sid    = hdr_sid;
            prev_hdr_vld    = hdr_vld;
            prev_hdr_arr[0] = hdr_arr[0]; // Lat
            prev_hdr_arr[1] = hdr_arr[1]; // Lon
            prev_hdr_arr[2] = hdr_arr[2]; // Elevation
         }
      }

      //
      // Increment the row count
      //
      nrow++;
   }

   //
   // Clear the error status and rewind back to the beginning
   // of the ASCII observation file
   //
   f_in.in->clear();
   f_in.in->seekg(0, ios::beg);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_met_obs(LineDataFile &f_in, NcFile *&f_out) {
   int i_obs, i_hdr;
   DataLine dl;

   ConcatString hdr_typ, prev_hdr_typ;
   ConcatString hdr_sid, prev_hdr_sid;
   ConcatString hdr_vld, prev_hdr_vld;
   float hdr_arr[hdr_arr_len], prev_hdr_arr[hdr_arr_len];
   float obs_arr[obs_arr_len];

   //
   // Define the netCDF dimensions
   //
   NcDim *strl_dim    = f_out->add_dim("mxstr", (long) strl_len);
   NcDim *hdr_arr_dim = f_out->add_dim("hdr_arr_len", (long) hdr_arr_len);
   NcDim *obs_arr_dim = f_out->add_dim("obs_arr_len", (long) obs_arr_len);
   NcDim *hdr_dim     = f_out->add_dim("nhdr", (long) nhdr);
   NcDim *obs_dim     = f_out->add_dim("nobs"); // unlimited dimension

   //
   // Define the netCDF variables
   //
   NcVar *hdr_typ_var = f_out->add_var("hdr_typ", ncChar, hdr_dim, strl_dim);
   NcVar *hdr_sid_var = f_out->add_var("hdr_sid", ncChar, hdr_dim, strl_dim);
   NcVar *hdr_vld_var = f_out->add_var("hdr_vld", ncChar, hdr_dim, strl_dim);
   NcVar *hdr_arr_var = f_out->add_var("hdr_arr", ncFloat, hdr_dim, hdr_arr_dim);
   NcVar *obs_arr_var = f_out->add_var("obs_arr", ncFloat, obs_dim, obs_arr_dim);

   //
   // Add attributes to the netCDF variables
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

   if(verbosity > 1) {
      cout << "Processing " << nrow << " observations.\n" << flush;
   }

   //
   // Process the observations
   //
   i_hdr = i_obs = 0;
   while(f_in >> dl) {

      //
      // Check that the line contains the expected number of columns
      //
      if(dl.n_items() != n_met_col) {
         cerr << "\n\nwrite_met_obs() -> "
              << "line number " << i_obs+1 << " doesn't contain the "
              << "expected number of columns (" << n_met_col
              << "):\n" << flush;

              dl.dump(cout);

              cout << "\n\n" << flush;
         exit(1);
      }

      //
      // Store the PrepBufr message type, station id, valid time,
      // and header array
      //
      hdr_typ    = dl[0];
      hdr_sid    = dl[1];
      hdr_vld    = dl[2];
      hdr_arr[0] = atof(dl[3]); // Lat
      hdr_arr[1] = atof(dl[4]); // Lon
      hdr_arr[2] = atof(dl[5]); // Elevation

      //
      // If this is the first row, store the data
      //
      if(i_obs == 0) {
         prev_hdr_typ    = hdr_typ;
         prev_hdr_sid    = hdr_sid;
         prev_hdr_vld    = hdr_vld;
         prev_hdr_arr[0] = hdr_arr[0]; // Lat
         prev_hdr_arr[1] = hdr_arr[1]; // Lon
         prev_hdr_arr[2] = hdr_arr[2]; // Elevation
      }
      //
      // Otherwise, check to see if the header data has changed, and
      // if so, increment the header count
      //
      else {
         if(hdr_typ         != prev_hdr_typ     ||
            hdr_sid         != prev_hdr_sid     ||
            hdr_vld         != prev_hdr_vld     ||
            !is_eq(prev_hdr_arr[0], hdr_arr[0]) ||
            !is_eq(prev_hdr_arr[1], hdr_arr[1]) ||
            !is_eq(prev_hdr_arr[2], hdr_arr[2])) {

            //
            // Increment the header count and store the previous values
            //
            i_hdr++;
            prev_hdr_typ    = hdr_typ;
            prev_hdr_sid    = hdr_sid;
            prev_hdr_vld    = hdr_vld;
            prev_hdr_arr[0] = hdr_arr[0]; // Lat
            prev_hdr_arr[1] = hdr_arr[1]; // Lon
            prev_hdr_arr[2] = hdr_arr[2]; // Elevation
         }
      }

      //
      // Store the PrepBufr message type
      //
      if(!hdr_typ_var->set_cur(i_hdr, (long) 0) ||
         !hdr_typ_var->put(hdr_typ, (long) 1, (long) hdr_typ.length())) {
         cerr << "\n\nERROR: write_met_obs() -> "
              << "error writing the message type to the netCDF file\n\n"
              << flush;
         exit(1);
      }

      //
      // Store the station id
      //
      if(!hdr_sid_var->set_cur(i_hdr, (long) 0) ||
         !hdr_sid_var->put(hdr_sid, (long) 1, (long) hdr_sid.length())) {
         cerr << "\n\nERROR: write_met_obs() -> "
              << "error writing the station id to the netCDF file\n\n"
              << flush;
         exit(1);
      }

      //
      // Store the valid time and check that it's is in the expected
      // time format: YYYYMMDD_HHMMSS
      //
      if(check_reg_exp(yyyymmdd_hhmmss_reg_exp, hdr_vld) != true) {
         cerr << "\n\nERROR: write_met_obs() -> "
              << "valid time is not in the expected YYYYMMDD_HHMMSS format: "
              << hdr_vld << "/n/n" << flush;
         exit(1);
      }

      if(!hdr_vld_var->set_cur(i_hdr, (long) 0) ||
         !hdr_vld_var->put(hdr_vld, (long) 1, (long) hdr_vld.length())) {
         cerr << "\n\nERROR: write_met_obs() -> "
              << "error writing the valid time to the netCDF file\n\n"
              << flush;
         exit(1);
      }

      //
      // Store the header array
      //
      if(!hdr_arr_var->set_cur(i_hdr, (long) 0) ||
         !hdr_arr_var->put(hdr_arr, (long) 1, (long) hdr_arr_len) ) {
         cerr << "\n\nERROR: main() -> "
              << "error writing the header array to the netCDF file\n\n"
              << flush;
         exit(1);
      }

      //
      // Store the observation array
      //
      obs_arr[0] = i_hdr;       // Header ID
      obs_arr[1] = atof(dl[6]); // Grib Code

      // Level: pressure level (hPa) or accumulation interval (sec) for precip
      if(is_precip_code(obs_arr[1])) obs_arr[2] = timestring_to_sec(dl[7]);
      else                           obs_arr[2] = atof(dl[7]);

      obs_arr[3] = atof(dl[8]); // Meters above sea level
      obs_arr[4] = atof(dl[9]); // Observation Value

      if(!obs_arr_var->set_cur(i_obs, (long) 0) ||
         !obs_arr_var->put(obs_arr, (long) 1, (long) obs_arr_len) ) {
         cerr << "\n\nERROR: main() -> "
              << "error writing the observation array to the netCDF file\n\n"
              << flush;
         exit(1);
      }

      //
      // Increment the observation count
      //
      i_obs++;
   } // end while


   if(verbosity > 1) {
      cout << "Finished processing " << nrow << " observations.\n" << flush;
   }

   //
   // Close the NetCDF file
   //
   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void usage(int argc, char *argv[]) {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tascii_file\n"
        << "\tnetcdf_file\n"
        << "\t[-format ASCII_format]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"ascii_file\" is the formatted ASCII "
        << "observation file to be converted to netCDF format "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output "
        << "netCDF file to be written (required).\n"

        << "\t\t\"-format ASCII_format\" overrides the default format "
        << "for the input ASCII file (" << met_fmt_str
        << ") (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n\n"

        << "\tThe only currently supported ASCII format is \""
        << met_fmt_str << "\" in which each row of observation data "
        << "consists of 10 columns:\n"
        << "\t\tMessage_Type Station_ID Valid_Time(YYYYMMDD_HHMMSS)\n"
        << "\t\tLat(Deg North) Lon(Deg East) Elevation(msl)\n"
        << "\t\tGrib_Code Level Height(msl or agl) Observation_Value\n\n"

        << "\t\twhere\t\"Level\" is the pressure level (hPa) or "
        << "accumulation interval (HH[MMSS]).\n"
        << "\t\t\t\"Height\" is meters above sea level or above ground level for the "
        << "observation (msl or agl).\n\n"

        << "\t\t\tUse a value of \"" << bad_data_int
        << "\" to indicate missing data.\n\n"

        << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
