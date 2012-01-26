// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   madis2nc.cc
//
//   Description:
//      Parse MADIS NetCDF files containing surface point observations
//      and reformat them for use by MET.  Initial release provides
//      support for METAR and RAOB MADIS types.  Support for additional
//      MADIS types should be added.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    07-21-11  Halley Gotway  Adapted from contributed code.
//   001    01-06-12  Holmes         Added use of command line class to
//                                   parse the command line arguments.
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

#include "madis2nc.h"

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_nc_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void initialize();
static void process_command_line(int, char **);
static void process_madis_file(const char *);
static void clean_up();

static void setup_netcdf_out(int nhdr);

static int    get_nc_dim(NcFile *&f_in, const char *);
static NcVar *get_nc_var(NcFile *&f_in, const char *);
static void   get_nc_var_att(NcVar *&, const char *, float &);

static void   get_nc_var_val(NcVar *&, const long *cur, int len, ConcatString &);
static void   get_nc_var_val(NcVar *&, const long *cur, const long *dim, double &);
static void   get_nc_var_val(NcVar *&, const long *cur, const long *dim, float &);
static void   get_nc_var_val(NcVar *&, const long *cur, const long *dim, char &);
static void   get_nc_var_val(NcVar *&, const long *cur, const long *dim, int &);

static void   put_nc_var_val(NcVar *&, int i, const ConcatString &);
static void   put_nc_var_arr(NcVar *&, int i, int len, const float *);

static int    get_num_lvl(NcVar *&, const char *dim_str,
                          const long *cur, const long *dim);
static float  get_nc_obs(NcFile *&f_in, const char *in_str,
                         const long *cur, const long *dim,
                         int &n_rej_fill, int &n_rej_qc);
static void   process_obs(NcFile *&f_in, const char *in_str,
                          const long *cur, const long *dim,
                          const int gc, const float conversion,
                          float *obs_arr, int &i_obs,
                          int &n_rej_fill, int &n_rej_qc);

static MadisType get_madis_type(NcFile *&f_in);
static void      parse_css(const char *, StringArray &);
static void      convert_wind_wdir_to_u_v(float wind, float wdir,
                                          float &u, float &v);

static void process_madis_metar(NcFile *&f_in);
static void process_madis_raob(NcFile *&f_in);

static void usage();
static void set_type(const StringArray &);
static void set_qc_dd(const StringArray &);
static void set_lvl_dim(const StringArray &);
static void set_rec_beg(const StringArray &);
static void set_rec_end(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Initialize static variables
   //
   initialize();

   //
   // Process the command line arguments
   //
   process_command_line(argc, argv);

   //
   // Process the MADIS file
   //
   process_madis_file(mdfile);

   //
   // Deallocate memory and clean up
   //
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void initialize() {

   mdfile.clear();
   ncfile.clear();
   qc_dd_sa.clear();
   lvl_dim_sa.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   int i;

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   //
   // Store command line arguments to be written to output file.
   //
   argv_str = argv[0];
   for(i=1; i<argc; i++) argv_str << " " << argv[i];

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_type, "-type", 1);
   cline.add(set_qc_dd, "-qc_dd", 1);
   cline.add(set_lvl_dim, "-lvl_dim", 1);
   cline.add(set_rec_beg, "-rec_beg", 1);
   cline.add(set_rec_end, "-rec_end", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left; the
   // madis input filename and the netCDF output filename.
   //
   if(cline.n() != 2) usage();

   //
   // Store the input MADIS file name and the output NetCDF file name
   //
   mdfile = cline[0];
   ncfile = cline[1];

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_file(const char *madis_file) {

   // Print out current file name
   mlog << Debug(1) << "Reading MADIS File:\t" << madis_file << "\n";

   // Open the input NetCDF file
   NcFile *f_in = new NcFile(madis_file);

   // Check for a valid file
   if(!f_in->is_valid()) {
      mlog << Error << "\nprocess_madis_file() -> "
           << "can't open input NetCDF file \"" << madis_file
           << "\" for reading.\n\n";
      f_in->close();
      delete f_in;
      f_in = (NcFile *) 0;

      exit(1);
   }

   // If the MADIS type is not already set, try to guess.
   if(mtype == madis_none) mtype = get_madis_type(f_in);

   // Switch on the MADIS type and process accordingly.
   switch(mtype) {
      case(madis_metar):
         process_madis_metar(f_in);
         break;
      case(madis_raob):
         process_madis_raob(f_in);
         break;

      case(madis_coop):
      case(madis_HDW):
      case(madis_HDW1h):
      case(madis_hydro):
      case(madis_POES):
      case(madis_acars):
      case(madis_acarsProfiles):
      case(madis_maritime):
      case(madis_mesonet):
      case(madis_profiler):
      case(madis_radiometer):
      case(madis_sao):
      case(madis_satrad):
      case(madis_snow):
      case(madis_none):
      default:
         mlog << Error << "\nprocess_madis_file() -> "
              << "MADIS type (" << mtype
              << ") not currently supported.\n\n";
         exit(1);
         break;
   }

   // Close the input NetCDF file
   if(f_in) {
      f_in->close();
      delete f_in;
      f_in = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   //
   // Close the output NetCDF file
   //
   if(f_out) {
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_netcdf_out(int nhdr) {

   //
   // Create the output netCDF file for writing
   //
   mlog << Debug(1) << "Writing MET File:\t" << ncfile << "\n";
   f_out = new NcFile(ncfile, NcFile::Replace);

   //
   // Check for a valid file
   //
   if(!f_out->is_valid()) {
      mlog << Error << "\nsetup_netcdf_out() -> "
           << "trouble opening output file: " << ncfile << "\n\n";
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;
      exit(1);
   }

   //
   // Define netCDF dimensions
   //
   strl_dim    = f_out->add_dim("mxstr", (long) strl_len);
   hdr_arr_dim = f_out->add_dim("hdr_arr_len", (long) hdr_arr_len);
   obs_arr_dim = f_out->add_dim("obs_arr_len", (long) obs_arr_len);
   obs_dim     = f_out->add_dim("nobs"); // unlimited dimension
   hdr_dim     = f_out->add_dim("nhdr", (long) nhdr);

   //
   // Define netCDF variables
   //
   hdr_typ_var = f_out->add_var("hdr_typ", ncChar,  hdr_dim, strl_dim);
   hdr_sid_var = f_out->add_var("hdr_sid", ncChar,  hdr_dim, strl_dim);
   hdr_vld_var = f_out->add_var("hdr_vld", ncChar,  hdr_dim, strl_dim);
   hdr_arr_var = f_out->add_var("hdr_arr", ncFloat, hdr_dim, hdr_arr_dim);
   obs_arr_var = f_out->add_var("obs_arr", ncFloat, obs_dim, obs_arr_dim);

   //
   // Add variable attributes
   //
   hdr_typ_var->add_att("long_name", "message type");
   hdr_sid_var->add_att("long_name", "station identification");
   hdr_vld_var->add_att("long_name", "valid time");
   hdr_vld_var->add_att("units", "YYYYMMDD_HHMMSS");

   hdr_arr_var->add_att("long_name",
                        "array of observation station header values");
   hdr_arr_var->add_att("_fill_value", fill_value);
   hdr_arr_var->add_att("columns", "lat lon elv");
   hdr_arr_var->add_att("lat_long_name", "latitude");
   hdr_arr_var->add_att("lat_units", "degrees_north");
   hdr_arr_var->add_att("lon_long_name", "longitude");
   hdr_arr_var->add_att("lon_units", "degrees_east");
   hdr_arr_var->add_att("elv_long_name", "elevation");
   hdr_arr_var->add_att("elv_units", "meters above sea level (msl)");

   obs_arr_var->add_att("long_name", "array of observation values");
   obs_arr_var->add_att("_fill_value", fill_value);
   obs_arr_var->add_att("columns", "hdr_id gc lvl hgt ob");
   obs_arr_var->add_att("hdr_id_long_name",
                        "index of matching header data");
   obs_arr_var->add_att("gc_long_name",
      "grib code corresponding to the observation type");
   obs_arr_var->add_att("lvl_long_name",
      "pressure level (hPa) or accumulation interval (sec)");
   obs_arr_var->add_att("hgt_long_name",
                        "height in meters above sea level (msl)");
   obs_arr_var->add_att("ob_long_name", "observation value");

   //
   // Add global attributes
   //
   write_netcdf_global(f_out, ncfile.text(), program_name);

   //
   // Add the command line arguments that were applied.
   //
   f_out->add_att("RunCommand", argv_str);

   return;
}

////////////////////////////////////////////////////////////////////////

int get_nc_dim(NcFile *&f_in, const char *dim_str) {
   NcDim *dim;

   //
   // Retrieve the dimension from the NetCDF file.
   //
   if(!(dim = f_in->get_dim(dim_str))) {
      mlog << Error << "\nget_nc_dim() -> "
           << "can't read \"" << dim_str << "\" dimension.\n\n";
      exit(1);
   }

   return(dim->size());
}

////////////////////////////////////////////////////////////////////////

NcVar *get_nc_var(NcFile *&f_in, const char *var_str) {
   NcVar *var;

   //
   // Retrieve the variable from the NetCDF file.
   //
   if(!(var = f_in->get_var(var_str))) {
      mlog << Error << "\nget_nc_var() -> "
           << "can't read \"" << var_str << "\" variable.\n\n";
      exit(1);
   }

   return(var);
}

////////////////////////////////////////////////////////////////////////

void get_nc_var_att(NcVar *&var, const char *att_str, float &d) {
   NcAtt *att;

   //
   // Retrieve the NetCDF variable attribute.
   //
   if(!(att = var->get_att(att_str)) || !att->is_valid()) {
      mlog << Error << "\nget_nc_var_att(float) -> "
           << "can't read attribute \"" << att_str
           << "\" from \"" << var->name() << "\" variable.\n\n";
      exit(1);
   }
   d = att->as_float(0);

   return;
}

////////////////////////////////////////////////////////////////////////

void get_nc_var_val(NcVar *&var, const long *cur,
                    int len, ConcatString &tmp_cs) {
   char tmp_str[max_str_len];

   //
   // Retrieve the character array value from the NetCDF variable.
   //
   if(!var->set_cur((long *) cur) || !var->get(&tmp_str[0], 1, len)) {
      mlog << Error << "\nget_nc_var_val(ConcatString) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   //
   // Store the character array as a ConcatString
   //
   tmp_cs = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void get_nc_var_val(NcVar *&var, const long *cur, const long *dim,
                    float &d) {

   //
   // Retrieve the float value from the NetCDF variable.
   //
   if(!var->set_cur((long *) cur) || !var->get(&d, (long *) dim)) {
      mlog << Error << "\nget_nc_var_val(float) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_nc_var_val(NcVar *&var, const long *cur, const long *dim,
                    double &d) {

   //
   // Retrieve the double value from the NetCDF variable.
   //
   if(!var->set_cur((long *) cur) || !var->get(&d, (long *) dim)) {
      mlog << Error << "\nget_nc_var_val(double) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_nc_var_val(NcVar *&var, const long *cur, const long *dim,
                    char &d) {

   //
   // Retrieve the character value from the NetCDF variable.
   //
   if(!var->set_cur((long *) cur) || !var->get(&d, (long *) dim)) {
      mlog << Error << "\nget_nc_var_val(char) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_nc_var_val(NcVar *&var, const long *cur, const long *dim,
                    int &d) {

   //
   // Retrieve the character value from the NetCDF variable.
   //
   if(!var->set_cur((long *) cur) || !var->get(&d, (long *) dim)) {
      mlog << Error << "\nget_nc_var_val(int) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void put_nc_var_val(NcVar *&var, int i, const ConcatString &str) {

   //
   // Store the character array in the NetCDF variable.
   //
   if(!var->set_cur(i, (long) 0) ||
      !var->put(str, (long) 1, (long) str.length())) {
      mlog << Error << "\nput_nc_var_val(ConcatString) -> "
           << "can't write data to \"" << var->name()
           << "\" variable for record number " << i << ".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void put_nc_var_arr(NcVar *&var, int i, int len, const float *arr) {

   //
   // Store the array of floats in the NetCDF variable.
   //
   if(!var->set_cur(i, (long) 0) ||
      !var->put(arr, (long) 1, (long) len)) {
      mlog << Error << "\nput_nc_var_arr(float) -> "
           << "can't write data to \"" << var->name()
           << "\" variable for record number " << i << ".\n\n";
      exit(1);
   }

   int j;
   ConcatString msg;
   msg << "    [WRITE]  " << var->name() << "[" << i << "]:";
   for(j=0; j<len; j++) msg << " " << arr[j];
   mlog << Debug(3) << msg << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

int get_num_lvl(NcVar *&var, const char *dim_str,
                const long *cur, const long *dim) {
   int d;

   //
   // If vertical level dimensions were specified on the command line
   // but did not include this one, return 0.
   //
   if(lvl_dim_sa.n_elements() == 0 || lvl_dim_sa.has(dim_str)) {
      get_nc_var_val(var, cur, dim, d);
   }
   else {
      d = 0;
   }

   return(d);
}

////////////////////////////////////////////////////////////////////////

float get_nc_obs(NcFile *&f_in, const char *in_str,
                 const long *cur, const long *dim,
                 int &n_rej_fill, int &n_rej_qc) {
   float v, in_fill_value;
   ConcatString in_dd_str, dd_str;
   char dd;

   //
   // Setup the QC search string.
   //
   in_dd_str = in_str;
   in_dd_str << "DD";

   //
   // Retrieve the input and DD variables
   //
   NcVar *in_var    = get_nc_var(f_in, in_str);
   NcVar *in_var_dd = get_nc_var(f_in, in_dd_str);

   //
   // Retrieve the fill value
   //
   get_nc_var_att(in_var, in_fillValue_str, in_fill_value);

   //
   // Retrieve the values
   //
   get_nc_var_val(in_var, cur, dim, v);
   get_nc_var_val(in_var_dd, cur, dim, dd);
   dd_str << dd;

   //
   // Check for missing data
   //
   if(is_eq(v, in_fill_value)) {
      v = bad_data_float;
      n_rej_fill++;
   }

   //
   // Check quality control flag
   //
   if(!is_bad_data(v)  &&
      qc_dd_sa.n_elements() > 0 &&
      !qc_dd_sa.has(dd_str)) {
      v = bad_data_float;
      n_rej_qc++;
   }

   mlog << Debug(3)  << "    [" << (is_bad_data(v) ? "REJECT" : "ACCEPT") << "] " << in_str
        << ": value = " << v
        << ", qc = " << dd_str
        << "\n";

   return(v);
}

////////////////////////////////////////////////////////////////////////

void process_obs(NcFile *&f_in, const char *in_str,
                 const long *cur, const long *dim,
                 const int in_gc, const float conversion,
                 float *obs_arr, int &i_obs,
                 int &n_rej_fill, int &n_rej_qc) {
   //
   // Store the GRIB code
   //
   obs_arr[1] = in_gc;

   //
   // Get the observation value and store it
   //
   obs_arr[4] = get_nc_obs(f_in, in_str, cur, dim, n_rej_fill, n_rej_qc);

   //
   // Check for bad data and apply conversion factor
   //
   if(!is_bad_data(obs_arr[4])) {
      obs_arr[4] *= conversion;
      put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
      i_obs++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

MadisType get_madis_type(NcFile *&f_in) {

   //
   // FUTURE WORK: Interrogate the MADIS file and determine it's type.
   //
   return(madis_none);
}

////////////////////////////////////////////////////////////////////////

void parse_css(const char *in_str, StringArray &sa) {
   int i, n;
   char tmp_str[max_str_len];

   //
   // FUTURE WORK: This function could become a member function of
   // StringArray.
   //

   strcpy(tmp_str, in_str);

   //
   // Replace comma's with spaces.
   //
   for(i=0, n=strlen(in_str); i<n; i++)
      if(tmp_str[i] == ',') tmp_str[i] = ' ';

   //
   // Parse white-space separated text.
   //
   sa.parse_wsss(tmp_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void convert_wind_wdir_to_u_v(float wind, float wdir,
                              float &u, float &v) {
   //
   // Convert wind direction and speed to U and V
   //
   if(is_bad_data(wind) || is_bad_data(wdir)) {
      u = v = bad_data_float;
   }
   else {
      u = (float) -1.0*wind*sind(wdir);
      v = (float) -1.0*wind*cosd(wdir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_metar(NcFile *&f_in) {
   int nhdr, i_obs, n_rej_fill, n_rej_qc;
   long i_hdr;
   int hdr_typ_len, hdr_sid_len;
   double tmp_dbl;
   char tmp_str[max_str_len];
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float wdir, wind, ugrd, vgrd;

   //
   // Input header variables
   //
   NcVar *in_hdr_typ_var = get_nc_var(f_in, "reportType");
   NcVar *in_hdr_sid_var = get_nc_var(f_in, "stationName");
   NcVar *in_hdr_vld_var = get_nc_var(f_in, "timeObs");
   NcVar *in_hdr_lat_var = get_nc_var(f_in, "latitude");
   NcVar *in_hdr_lon_var = get_nc_var(f_in, "longitude");
   NcVar *in_hdr_elv_var = get_nc_var(f_in, "elevation");

   //
   // Retrieve applicable dimensions
   //
   hdr_typ_len = get_nc_dim(f_in, "maxRepLen");
   hdr_sid_len = get_nc_dim(f_in, "maxStaNamLen");
   nhdr        = get_nc_dim(f_in, in_recNum_str);
   if(rec_end == 0) rec_end = nhdr;

   //
   // Setup the output NetCDF file
   //
   setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing METAR recs\t= " << rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;
   i_obs = n_rej_fill = n_rej_qc = 0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [2];
   cur[0] = cur[1] = 0;
   long *dim = new long [1];
   dim[0] = 1;

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {

      //
      // Mapping of NetCDF variable names from input to output:
      // Output                    = Input
      // hdr_typ                   = reportType(maxRepLen = 6)
      // hdr_sid                   = stationName(maxStaNamLen = 5)
      // hdr_vld (YYYYMMDD_HHMMSS) = timeObs (unixtime)
      // hdr_arr[0](Lat)           = latitude
      // hdr_arr[1](Lon)           = longitude
      // hdr_arr[2](Elv)           = elevation
      //

      mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

      //
      // Use cur to index into the NetCDF variables.
      //
      cur[0] = i_hdr;

      //
      // Process the header type.
      // For METAR or SPECI, encode as ADPSFC.
      // Otherwise, use value from file.
      //
      get_nc_var_val(in_hdr_typ_var, cur, hdr_typ_len, hdr_typ);
      if(hdr_typ == metar_str || hdr_typ == "SPECI") hdr_typ = "ADPSFC";
      put_nc_var_val(hdr_typ_var, i_hdr, hdr_typ);

      //
      // Process the station name.
      //
      get_nc_var_val(in_hdr_sid_var, cur, hdr_sid_len, hdr_sid);
      put_nc_var_val(hdr_sid_var, i_hdr, hdr_sid);

      //
      // Process the observation time.
      //
      get_nc_var_val(in_hdr_vld_var, cur, dim, tmp_dbl);
      unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
      hdr_vld = tmp_str;
      put_nc_var_val(hdr_vld_var, i_hdr, hdr_vld);

      //
      // Process the latitude, longitude, and elevation.
      //
      get_nc_var_val(in_hdr_lat_var, cur, dim, hdr_arr[0]);
      get_nc_var_val(in_hdr_lon_var, cur, dim, hdr_arr[1]);
      get_nc_var_val(in_hdr_elv_var, cur, dim, hdr_arr[2]);

      //
      // Write the header array to the output file.
      //
      put_nc_var_arr(hdr_arr_var, i_hdr, hdr_arr_len, hdr_arr);

      //
      // Initialize the observation array: hdr_id, gc, lvl, hgt, ob
      //
      obs_arr[0] = (float) i_hdr;   // Index into header array
      obs_arr[2] = bad_data_float;  // Level: accum(sec) or pressure
      obs_arr[3] = bad_data_float;  // Height

      // Sea Level Pressure
      process_obs(f_in, "seaLevelPress", cur, dim, 2, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Visibility
      process_obs(f_in, "visibility", cur, dim, 20, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Temperature
      process_obs(f_in, "temperature", cur, dim, 11, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Dewpoint
      process_obs(f_in, "dewpoint", cur, dim, 7, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Wind Direction
      process_obs(f_in, "windDir", cur, dim, 31, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);
      wdir = obs_arr[4];

      // Wind Speed
      process_obs(f_in, "windSpeed", cur, dim, 32, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);
      wind = obs_arr[4];

      // Convert the wind direction and speed into U and V components
      convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

      // Write U-component of wind
      obs_arr[1] = 33;
      obs_arr[4] = ugrd;
      if(!is_bad_data(ugrd)) {
         put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
         i_obs++;
      }

      // Write V-component of wind
      obs_arr[1] = 34;
      obs_arr[4] = vgrd;
      if(!is_bad_data(vgrd)) {
         put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
         i_obs++;
      }

      // Wind Gust
      process_obs(f_in, "windGust", cur, dim, 180, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Min Temperature - 24 Hour
      process_obs(f_in, "minTemp24Hour", cur, dim, 16, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Max Temperature - 24 Hour
      process_obs(f_in, "maxTemp24Hour", cur, dim, 15, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      conversion = 1000.0;
      // Precipitation - 1 Hour
      obs_arr[2] = 1.0*sec_per_hour;
      process_obs(f_in, "precip1Hour", cur, dim, 61, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Precipitation - 3 Hour
      obs_arr[2] = 3.0*sec_per_hour;
      process_obs(f_in, "precip3Hour", cur, dim, 61, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Precipitation - 6 Hour
      obs_arr[2] = 6.0*sec_per_hour;
      process_obs(f_in, "precip6Hour", cur, dim, 61, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      // Precipitation - 24 Hour
      obs_arr[2] = 24.0*sec_per_hour;
      process_obs(f_in, "precip24Hour", cur, dim, 61, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);

      conversion = 1.0;
      // Snow Cover
      obs_arr[2] = bad_data_float;
      process_obs(f_in, "snowCover", cur, dim, 66, conversion,
                  obs_arr, i_obs, n_rej_fill, n_rej_qc);
   } // end for i_hdr

   mlog << Debug(2) << "Rejected based on QC\t= " << n_rej_qc << "\n"
           << "Rejected based on fill\t= " << n_rej_fill << "\n"
           << "Retained or derived\t= " << i_obs << "\n";

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_raob(NcFile *&f_in) {
   int nhdr, nlvl, i_lvl, i_obs;
   int n_rej_fill, n_rej_qc, n;
   long i_hdr;
   int hdr_sid_len;
   double tmp_dbl;
   char tmp_str[max_str_len];
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float wdir, wind, ugrd, vgrd;

   //
   // Input header variables:
   // Note: hdr_typ is always set to ADPUPA
   //
   NcVar *in_hdr_sid_var = get_nc_var(f_in, "staName");
   NcVar *in_hdr_vld_var = get_nc_var(f_in, "synTime");
   NcVar *in_hdr_lat_var = get_nc_var(f_in, "staLat");
   NcVar *in_hdr_lon_var = get_nc_var(f_in, "staLon");
   NcVar *in_hdr_elv_var = get_nc_var(f_in, "staElev");

   //
   // Variables for vertical level sizes
   //
   NcVar *in_man_var    = get_nc_var(f_in, "numMand");
   NcVar *in_sigt_var   = get_nc_var(f_in, "numSigT");
   NcVar *in_sigw_var   = get_nc_var(f_in, "numSigW");
   NcVar *in_sigprw_var = get_nc_var(f_in, "numSigPresW");
   NcVar *in_trop_var   = get_nc_var(f_in, "numTrop");
   NcVar *in_maxw_var   = get_nc_var(f_in, "numMwnd");

   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len  = get_nc_dim(f_in, "staNameLen");
   nhdr         = get_nc_dim(f_in, in_recNum_str);
   if(rec_end == 0) rec_end = nhdr;

   //
   // Setup the output NetCDF file
   //
   setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing RAOB recs\t= " << rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;
   i_obs = n_rej_fill = n_rej_qc = 0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [3];
   cur[0] = cur[1] = cur[2] = 0;
   long *dim = new long [3];
   dim[0] = dim[1] = dim[2] = 1;

   //
   // Loop through each record and get the header data.
   //
   for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {

      //
      // Mapping of NetCDF variable names from input to output:
      // Output                    = Input
      // hdr_typ                   = NA - always set to ADPUPA
      // hdr_sid                   = staName (staNameLen = 50)
      // hdr_vld (YYYYMMDD_HHMMSS) = synTime (unixtime) - synoptic time
      // hdr_arr[0](Lat)           = staLat
      // hdr_arr[1](Lon)           = staLon
      // hdr_arr[2](Elv)           = staElev
      //

      mlog << Debug(3) << "Record Number: " << i_hdr << "\n";

      //
      // Use cur to index into the NetCDF variables.
      //
      cur[0] = i_hdr;
      cur[1] = 0;

      //
      // Process the header type.
      // For RAOB, store as ADPUPA.
      //
      hdr_typ = "ADPUPA";
      put_nc_var_val(hdr_typ_var, i_hdr, hdr_typ);

      //
      // Process the station name.
      //
      get_nc_var_val(in_hdr_sid_var, cur, hdr_sid_len, hdr_sid);
      put_nc_var_val(hdr_sid_var, i_hdr, hdr_sid);

      //
      // Process the observation time.
      //
      get_nc_var_val(in_hdr_vld_var, cur, dim, tmp_dbl);
      unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
      hdr_vld = tmp_str;
      put_nc_var_val(hdr_vld_var, i_hdr, hdr_vld);

      //
      // Process the latitude, longitude, and elevation.
      //
      get_nc_var_val(in_hdr_lat_var, cur, dim, hdr_arr[0]);
      get_nc_var_val(in_hdr_lon_var, cur, dim, hdr_arr[1]);
      get_nc_var_val(in_hdr_elv_var, cur, dim, hdr_arr[2]);

      //
      // Write the header array to the output file.
      //
      put_nc_var_arr(hdr_arr_var, i_hdr, hdr_arr_len, hdr_arr);

      //
      // Initialize the observation array: hdr_id
      //
      obs_arr[0] = (float) i_hdr;

      //
      // Loop through the mandatory levels
      //
      nlvl = get_num_lvl(in_man_var, "manLevel", cur, dim);
      for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

         mlog << Debug(3) << "  Mandatory Level: " << i_lvl << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[1] = i_lvl;

         //
         // Get the pressure and height for this level
         //
         obs_arr[2] = get_nc_obs(f_in, "prMan", cur, dim, n, n);
         obs_arr[3] = get_nc_obs(f_in, "htMan", cur, dim, n, n);

         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

         // Pressure
         process_obs(f_in, "prMan", cur, dim, 1, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

         // Height
         process_obs(f_in, "htMan", cur, dim, 7, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

         // Temperature
         process_obs(f_in, "tpMan", cur, dim, 11, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

         // Dewpoint
         process_obs(f_in, "tdMan", cur, dim, 17, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

         // Wind Direction
         process_obs(f_in, "wdMan", cur, dim, 31, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wdir = obs_arr[4];

         // Wind Speed
         process_obs(f_in, "wsMan", cur, dim, 32, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

         // Write V-component of wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

      } // end for i_lvl

      //
      // Loop through the significant levels wrt T
      //
      nlvl = get_num_lvl(in_sigt_var, "sigTLevel", cur, dim);
      for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

         mlog << Debug(3) << "  Significant T Level: " << i_lvl << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[1] = i_lvl;

         //
         // Get the pressure and height for this level
         //
         obs_arr[2] = get_nc_obs(f_in, "prSigT", cur, dim, n, n);
         obs_arr[3] = bad_data_float;

         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

         // Temperature
         process_obs(f_in, "tpSigT", cur, dim, 11, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

         // Dewpoint
         process_obs(f_in, "tdSigT", cur, dim, 17, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

      } // end for i_lvl

      //
      // Loop through the significant levels wrt W
      //
      nlvl = get_num_lvl(in_sigw_var, "sigWLevel", cur, dim);
      for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

         mlog << Debug(3) << "  Significant W Level: " << i_lvl << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[1] = i_lvl;

         //
         // Get the pressure and height for this level
         //
         obs_arr[2] = bad_data_float;
         obs_arr[3] = get_nc_obs(f_in, "htSigW", cur, dim, n, n);

         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

         // Wind Direction
         process_obs(f_in, "wdSigW", cur, dim, 31, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wdir = obs_arr[4];

         // Wind Speed
         process_obs(f_in, "wsSigW", cur, dim, 32, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

         // Write V-component of wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

      } // end for i_lvl

      //
      // Loop through the significant levels wrt W-by-P
      //
      nlvl = get_num_lvl(in_sigprw_var, "sigPresWLevel", cur, dim);
      for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

         mlog << Debug(3) << "  Significant W-by-P Level: " << i_lvl << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[1] = i_lvl;

         //
         // Get the pressure and height for this level
         //
         obs_arr[2] = get_nc_obs(f_in, "prSigW", cur, dim, n, n);
         obs_arr[3] = bad_data_float;

         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

         // Wind Direction
         process_obs(f_in, "wdSigPrW", cur, dim, 31, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wdir = obs_arr[4];

         // Wind Speed
         process_obs(f_in, "wsSigPrW", cur, dim, 32, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

         // Write V-component of wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

      } // end for i_lvl

      //
      // Loop through the tropopause levels
      //
      nlvl = get_num_lvl(in_trop_var, "mTropNum", cur, dim);
      for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

         mlog << Debug(3) << "  Tropopause Level: " << i_lvl << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[1] = i_lvl;

         //
         // Get the pressure and height for this level
         //
         obs_arr[2] = get_nc_obs(f_in, "prTrop", cur, dim, n, n);
         obs_arr[3] = bad_data_float;

         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

         // Temperature
         process_obs(f_in, "tpTrop", cur, dim, 11, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

         // Dewpoint
         process_obs(f_in, "tdTrop", cur, dim, 17, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);

         // Wind Direction
         process_obs(f_in, "wdTrop", cur, dim, 31, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wdir = obs_arr[4];

         // Wind Speed
         process_obs(f_in, "wsTrop", cur, dim, 32, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

         // Write V-component of wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

      } // end for i_lvl

      //
      // Loop through the maximum wind levels
      //
      nlvl = get_num_lvl(in_maxw_var, "mWndNum", cur, dim);
      for(i_lvl=0; i_lvl<nlvl; i_lvl++) {

         mlog << Debug(3) << "  Maximum Wind Level: " << i_lvl << "\n";

         //
         // Use cur to index into the NetCDF variables.
         //
         cur[1] = i_lvl;

         //
         // Get the pressure and height for this level
         //
         obs_arr[2] = get_nc_obs(f_in, "prMaxW", cur, dim, n, n);
         obs_arr[3] = bad_data_float;

         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;

         // Wind Direction
         process_obs(f_in, "wdMaxW", cur, dim, 31, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wdir = obs_arr[4];

         // Wind Speed
         process_obs(f_in, "wsMaxW", cur, dim, 32, conversion,
                     obs_arr, i_obs, n_rej_fill, n_rej_qc);
         wind = obs_arr[4];

         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);

         // Write U-component of wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

         // Write V-component of wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            i_obs++;
         }

      } // end for i_lvl

   } // end for i_hdr

   mlog << Debug(2) << "Rejected based on QC\t= " << n_rej_qc << "\n"
        << "Rejected based on fill\t= " << n_rej_fill << "\n"
        << "Retained or derived\t= " << i_obs << "\n";

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tmadis_file\n"
        << "\tout_file\n"
        << "\t[-type str]\n"
        << "\t[-qc_dd list]\n"
        << "\t[-lvl_dim list]\n"
        << "\t[-rec_beg n]\n"
        << "\t[-rec_end n]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"madis_file\" is the MADIS NetCDF point "
        << "observation file (required).\n"

        << "\t\t\"out_file\" is the output NetCDF file (required).\n"

        << "\t\t\"-type str\" specifies the type of MADIS observations "
        << "(metar or raob).\n"

        << "\t\t\"-qc_dd list\" specifies a comma-separated list of "
        << "QC flag values to be accepted (Z,C,S,V,X,Q,K,G,B) "
        << "(optional).\n"

        << "\t\t\"-lvl_dim list\" specifies a comma-separated list of "
        << "vertical level dimensions to be processed. (optional).\n"

        << "\t\t\"-rec_beg n\" specifies the index of the first "
        << "MADIS record to process, zero-based (optional).\n"

        << "\t\t\"-rec_end n\" specifies the index of the last "
        << "MADIS record to process, zero-based (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_type(const StringArray & a)
{
   //
   // Parse the MADIS type
   //
   if(strcasecmp(a[0], metar_str) == 0) {
      mtype = madis_metar;
   }
   else if(strcasecmp(a[0], raob_str) == 0) {
      mtype = madis_raob;
   }
   else {
      mlog << Error << "\nprocess_command_line() -> "
            << "MADIS type \"" << a[0]
            << "\" not currently supported.\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_qc_dd(const StringArray & a)
{
   //
   // Parse the list of QC flags to be used
   //
   parse_css(a[0], qc_dd_sa);
}

////////////////////////////////////////////////////////////////////////

void set_lvl_dim(const StringArray & a)
{
   //
   // Parse the list vertical level dimensions to be processed
   //
   parse_css(a[0], lvl_dim_sa);
}

////////////////////////////////////////////////////////////////////////

void set_rec_beg(const StringArray & a)
{
   rec_beg = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_rec_end(const StringArray & a)
{
   rec_end = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

