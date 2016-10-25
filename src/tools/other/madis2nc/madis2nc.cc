// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
//      and reformat them for use by MET.  Current release provides
//      support for METAR, RAOB, PROFILER, MARITIME, MESONET, and
//      ACARSPROFILES MADIS types.
//      Support for additional MADIS types may be added.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    07-21-11  Halley Gotway  Adapted from contributed code.
//   001    01-06-12  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//   002    07-13-12  Oldenburg      Added support for profiler
//                                   observation type.
//   003    02-26-14  Halley Gotway  Added support for mesonet
//                                   observation type.
//   004    07-07-14  Halley Gotway  Added the mask_grid and mask_poly
//                                   options to filter spatially.
//   005    10-01-15  Chaudhuri      Added support for acarsProfiles
//                                   observation type.
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

#include "data2d_factory.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_nc_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE 32*1024

static void initialize();
static void process_command_line(int, char **);
static void process_madis_file(const char *);
static void clean_up();

static void setup_netcdf_out(int nhdr);

static int    get_nc_dim(NcFile *&f_in, const char *);
//static void   get_nc_var_att(NcVar *&, const char *, float &);

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
                         const long *cur, const long *dim);
static float  get_nc_obs(NcFile *&f_in, const char *in_str,
                         const long *cur, const long *dim,
                         char &qty);
static bool get_filtered_nc_data(NcVar *var, const long *cur, const long *dim, float *data);
static bool get_filtered_nc_data_2d(NcVar *var, const long *cur, const long *dim,
                                    int *data, bool count_bad=false);
static bool get_filtered_nc_data_2d(NcVar *var, const long *cur, const long *dim,
                                    float *data, bool count_bad=false);

static void   check_quality_control_flag(int &value, const char qty, const char* var_name);
static void   check_quality_control_flag(float &value, const char qty, const char* var_name);
                                    
static void   process_obs(const int gc, const float conversion,
                          float *obs_arr, char qty, const char* var_name='\0');
static void   process_obs(NcFile *&f_in, const char *in_str,
                          const long *cur, const long *dim,
                          const int gc, const float conversion,
                          float *obs_arr);
static void   process_obs(NcFile *&f_in, const char *in_str,
                          const long *cur, const long *dim,
                          const int gc, const float conversion,
                          float *obs_arr, char &qty);
static void   write_qty(char &qty);

static MadisType get_madis_type(NcFile *&f_in);
static void      parse_css(const char *, StringArray &);
static void      convert_wind_wdir_to_u_v(float wind, float wdir,
                                          float &u, float &v);
static bool      check_masks(double lat, double lon);

static void process_madis_metar(NcFile *&f_in);
static void process_madis_raob(NcFile *&f_in);
static void process_madis_profiler(NcFile *&f_in);
static void process_madis_maritime(NcFile *&f_in);
static void process_madis_mesonet(NcFile *&f_in);
static void process_madis_acarsProfiles(NcFile *&f_in);

static void usage();
static void set_type(const StringArray &);
static void set_qc_dd(const StringArray &);
static void set_lvl_dim(const StringArray &);
static void set_rec_beg(const StringArray &);
static void set_rec_end(const StringArray &);
static void set_logfile(const StringArray &);
static void set_mask_grid(const StringArray &);
static void set_mask_poly(const StringArray &);
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
   i_obs    = 0;
   rej_fill = 0;
   rej_qc   = 0;
   rej_grid = 0;
   rej_poly = 0;

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
   cline.add(set_mask_grid, "-mask_grid", 1);
   cline.add(set_mask_poly, "-mask_poly", 1);

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
   NcFile *f_in = open_ncfile(madis_file);

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
      case (madis_profiler):
         process_madis_profiler(f_in);
         break;
      case(madis_maritime):
         process_madis_maritime(f_in);
         break;

      case(madis_mesonet):
         process_madis_mesonet(f_in);
         break;

      case(madis_acarsProfiles):
         process_madis_acarsProfiles(f_in);
         break;

      case(madis_coop):
      case(madis_HDW):
      case(madis_HDW1h):
      case(madis_hydro):
      case(madis_POES):
      case(madis_acars):
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
   f_out = open_ncfile(ncfile, NcFile::Replace);

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
   obs_qty_var = f_out->add_var("obs_qty", ncChar,  obs_dim, strl_dim);
   obs_arr_var = f_out->add_var("obs_arr", ncFloat, obs_dim, obs_arr_dim);
   //obs_arr_var->SetCompression(false, true, 2)

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

   obs_qty_var->add_att("long_name", "quality flag");

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
   NcDim *dim = (NcDim *) 0;

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
// Moved to nc_utils.cc
//
//NcVar *get_nc_var(NcFile *&f_in, const char *var_str) {
//   NcVar *var = (NcVar *) 0;
//
//   //
//   // Retrieve the variable from the NetCDF file.
//   //
//   if(!(var = f_in->get_var(var_str))) {
//      mlog << Error << "\nget_nc_var() -> "
//           << "can't read \"" << var_str << "\" variable.\n\n";
//      exit(1);
//   }
//
//   return(var);
//}
//
////////////////////////////////////////////////////////////////////////
// Moved to nc_utils.cc and renamed to get_var_att
//
//void get_nc_var_att(NcVar *&var, const char *att_str, float &d) {
//   NcAtt *att = (NcAtt *) 0;
//
//   //
//   // Retrieve the NetCDF variable attribute.
//   //
//   if(!(att = var->get_att(att_str)) || !att->is_valid()) {
//      mlog << Error << "\nget_nc_var_att(float) -> "
//           << "can't read attribute \"" << att_str
//           << "\" from \"" << var->name() << "\" variable.\n\n";
//      exit(1);
//   }
//   d = att->as_float(0);
//
//   return;
//}

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
   double fill_value;

   //
   // Retrieve the float value from the NetCDF variable.
   //
   if(!var->set_cur((long *) cur) || !var->get(&d, (long *) dim)) {
      mlog << Error << "\nget_nc_var_val(float) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   //
   // Check fill value
   //
   if(get_var_att_double(var, in_fillValue_str, fill_value) &&
      is_eq(d, fill_value)) d = bad_data_float;

   return;
}

////////////////////////////////////////////////////////////////////////

void get_nc_var_val(NcVar *&var, const long *cur, const long *dim,
                    double &d) {
   double fill_value;

   //
   // Retrieve the double value from the NetCDF variable.
   //
   if(!var->set_cur((long *) cur) || !var->get(&d, (long *) dim)) {
      mlog << Error << "\nget_nc_var_val(double) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   //
   // Check fill value
   //
   if(get_var_att_double(var, in_fillValue_str, fill_value) &&
      is_eq(d, fill_value)) d = bad_data_double;

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
                 char &qty) {
   float v, in_fill_value;
   ConcatString in_dd_str, dd_str;

   //
   // Setup the QC search string.
   //
   in_dd_str = in_str;
   in_dd_str << "DD";

   //
   // Retrieve the input and DD variables
   //
   NcVar *in_var    = get_nc_var(f_in, in_str);
   NcVar *in_var_dd = (NcVar *) 0;
   in_var_dd = has_var(f_in, in_dd_str);

   //
   // Retrieve the fill value
   //
   get_var_att(in_var, in_fillValue_str, in_fill_value);

   //
   // Retrieve the observation value
   //
   get_nc_var_val(in_var, cur, dim, v);

   //
   // Retrieve the QC string, if present
   //
   if(in_var_dd) {
      get_nc_var_val(in_var_dd, cur, dim, qty);
      dd_str << cs_erase << qty;
   }
   else {
      qty = '\0';
      dd_str << cs_erase << na_str;
   }

   //
   // Check for missing data
   //
   if(is_eq(v, in_fill_value)) {
      v = bad_data_float;
      rej_fill++;
   }

   //
   // Check quality control flag
   //
   if(!is_bad_data(v)  &&
      qc_dd_sa.n_elements() > 0 &&
      !qc_dd_sa.has(dd_str)) {
      v = bad_data_float;
      rej_qc++;
   }

   mlog << Debug(3)  << "    [" << (is_bad_data(v) ? "REJECT" : "ACCEPT") << "] " << in_str
        << ": value = " << v
        << ", qc = " << dd_str
        << " (get_nc_obs)\n";

   return(v);
}

////////////////////////////////////////////////////////////////////////

float get_nc_obs(NcFile *&f_in, const char *in_str,
                 const long *cur, const long *dim) {
   char qty;
   return get_nc_obs(f_in, in_str, cur, dim, qty);
}

////////////////////////////////////////////////////////////////////////

static bool get_filtered_nc_data(NcVar *var, const long *cur, const long *dim, float *data) {

   //char qty;
   bool status;
   float in_fill_value;
   //ConcatString dd_str;
   
   status = get_nc_data(var, cur, dim, data);
   get_var_att(var, in_fillValue_str, in_fill_value);
   for (int idx=0; idx<dim[0]; idx++) {
      if(is_eq(data[idx], in_fill_value)) {
         data[idx] = bad_data_float;
         rej_fill++;
      }
   }
   return status;
}


static bool get_filtered_nc_data_2d(NcVar *var, const long *cur, const long *dim,
                                    int *data, bool count_bad) {

   bool status;
   int in_fill_value;
   
   float *data2D[dim[1]];
   
   status = get_nc_data(var, cur, dim, data);

   get_var_att(var, in_fillValue_str, in_fill_value);
   mlog << Debug(5)  << "    get_filtered_nc_data_2d(int): in_fill_value=" << in_fill_value <<"\n";
   int offset, offsetStart = 0;
   for (int idx=0; idx<dim[0]; idx++) {
      offsetStart = idx * dim[1];
      for (int vIdx=0; vIdx<dim[1]; vIdx++) {
         offset = offsetStart + vIdx;
      
         if(is_eq(data[offset], in_fill_value)) {
//cout << "    DEBUG HS: get_filtered_nc_data_2d: found fill_value at offset " << offset <<"\n";
            data[offset] = bad_data_int;
            if(count_bad) {
               rej_fill++;
            }
         }
      }
   }
   return status;
}

static bool get_filtered_nc_data_2d(NcVar *var, const long *cur, const long *dim,
                                    float *data, bool count_bad) {

   //char qty;
   bool status;
   float in_fill_value;
   
   status = get_nc_data(var, cur, dim, data);

   get_var_att(var, in_fillValue_str, in_fill_value);
   mlog << Debug(5)  << "    get_filtered_nc_data_2d: in_fill_value=" << in_fill_value <<"\n";
   int offset, offsetStart = 0;
   for (int idx=0; idx<dim[0]; idx++) {
      offsetStart = idx * dim[1];
      for (int vIdx=0; vIdx<dim[1]; vIdx++) {
         //for (int vIdx=0; vIdx<dim[1]; vIdx++) {
         //if vIdx >= vlevels[idx]
         offset = offsetStart + vIdx;
      
         if(is_eq(data[offset], in_fill_value)) {
//cout << "    DEBUG HS: get_filtered_nc_data_2d: found fill_value at offset " << offset <<"\n";
            data[offset] = bad_data_float;
            if(count_bad) {
               rej_fill++;
            }
         }
      }
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

void check_quality_control_flag(int &value, const char qty, const char* var_name) {
   ConcatString dd_str;

   if (qty) {
      dd_str << cs_erase << qty;
   }
   else {
      dd_str << cs_erase << na_str;
   }
   
   //
   // Check quality control flag
   //
   if(!is_bad_data(value)  &&
      qc_dd_sa.n_elements() > 0 &&
      !qc_dd_sa.has(dd_str)) {
      value = bad_data_int;
      rej_qc++;
   }
   
   mlog << Debug(3)  << "    [" << (is_bad_data(value) ? "REJECT" : "ACCEPT") << "] " << var_name
        << ": value = " << value
        << ", qc = " << dd_str
        << "\n";
}

void check_quality_control_flag(float &value, const char qty, const char* var_name) {
   ConcatString dd_str;

   if (qty) {
      dd_str << cs_erase << qty;
   }
   else {
      dd_str << cs_erase << na_str;
   }
   
   //
   // Check quality control flag
   //
   if(!is_bad_data(value)  &&
      qc_dd_sa.n_elements() > 0 &&
      !qc_dd_sa.has(dd_str)) {
      value = bad_data_float;
      rej_qc++;
   }
   
   mlog << Debug(3)  << "    [" << (is_bad_data(value) ? "REJECT" : "ACCEPT") << "] " << var_name
        << ": value = " << value
        << ", qc = " << dd_str
        << "\n";
}

////////////////////////////////////////////////////////////////////////

// the observation value was stored already.
void process_obs(const int in_gc, const float conversion,
                 float *obs_arr, char qty, const char* var_name) {
   check_quality_control_flag(obs_arr[4], qty, var_name);
   
   //
   // Store the GRIB code
   //
   obs_arr[1] = in_gc;

   //
   // Check for bad data and apply conversion factor
   //
   if(!is_bad_data(obs_arr[4])) {
      obs_arr[4] *= conversion;
      put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
      write_qty(qty);
      i_obs++;
   }

   return;
}


//void process_obs(const int in_gc, const float conversion,
//                 float *obs_arr) {
//   char qty;
//   process_obs(in_gc, conversion, obs_arr, qty);
//}

////////////////////////////////////////////////////////////////////////

void process_obs(NcFile *&f_in, const char *in_str,
                 const long *cur, const long *dim,
                 const int in_gc, const float conversion,
                 float *obs_arr, char &qty) {
   //
   // Store the GRIB code
   //
   obs_arr[1] = in_gc;

   //
   // Get the observation value and store it
   //
   obs_arr[4] = get_nc_obs(f_in, in_str, cur, dim, qty);

   //
   // Check for bad data and apply conversion factor
   //
   if(!is_bad_data(obs_arr[4])) {
      obs_arr[4] *= conversion;
      put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
      write_qty(qty);
      i_obs++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
void process_obs(NcFile *&f_in, const char *in_str,
                 const long *cur, const long *dim,
                 const int in_gc, const float conversion,
                 float *obs_arr) {
   char qty;
   
   process_obs(f_in, in_str, cur, dim, in_gc, conversion, obs_arr, qty);
   //
   // Store the GRIB code
   //
   obs_arr[1] = in_gc;

   //
   // Get the observation value and store it
   //
   obs_arr[4] = get_nc_obs(f_in, in_str, cur, dim, qty);

   //
   // Check for bad data and apply conversion factor
   //
   if(!is_bad_data(obs_arr[4])) {
      obs_arr[4] *= conversion;
      put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
      write_qty(qty);
      i_obs++;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_qty(char &qty) {
   ConcatString qty_str;
   if(qty == '\0') qty_str = na_str;
   else            qty_str << qty;
   qty_str.replace(" ", "_", false);
   put_nc_var_val(obs_qty_var, i_obs, qty_str);
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

bool check_masks(double lat, double lon) {
   double grid_x, grid_y;

   //
   // Check grid masking.
   //
   if(grid_mask.nx() > 0 || grid_mask.ny() > 0) {
      grid_mask.latlon_to_xy(lat, -1.0*lon, grid_x, grid_y);
      if(grid_x < 0 || grid_x >= grid_mask.nx() ||
         grid_y < 0 || grid_y >= grid_mask.ny()) {
         rej_grid++;
         return false;
     }
   }

   //
   // Check polyline masking.
   //
   if(poly_mask.n_points() > 0) {
      if(!poly_mask.latlon_is_inside_dege(lat, lon)) {
         rej_poly++;
         return false;
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

void print_rej_counts() {

   mlog << Debug(2)
        << "Rejected recs based on masking grid\t= " << rej_grid << "\n"
        << "Rejected recs based on masking poly\t= " << rej_poly << "\n"
        << "Rejected based on fill value\t\t= " << rej_fill << "\n"
        << "Rejected based on quality control\t= " << rej_qc << "\n"
        << "Total observations retained or derived\t= " << i_obs << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_metar(NcFile *&f_in) {
   int nhdr;
   long i_hdr, i_hdr_s;
   int hdr_typ_len, hdr_sid_len, i_idx;
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

   NcVar *seaLevelPress_var = get_nc_var(f_in, "seaLevelPress");
   NcVar *visibility_var = get_nc_var(f_in, "visibility");
   NcVar *temperature_var = get_nc_var(f_in, "temperature");
   NcVar *dewpoint_var = get_nc_var(f_in, "dewpoint");
   NcVar *windDir_var = get_nc_var(f_in, "windDir");
   NcVar *windSpeed_var = get_nc_var(f_in, "windSpeed");
   NcVar *windGust_var = get_nc_var(f_in, "windGust");
   NcVar *minTemp24Hour_var = get_nc_var(f_in, "minTemp24Hour");
   NcVar *maxTemp24Hour_var = get_nc_var(f_in, "maxTemp24Hour");
   NcVar *precip1Hour_var = get_nc_var(f_in, "precip1Hour");
   NcVar *precip3Hour_var = get_nc_var(f_in, "precip3Hour");
   NcVar *precip6Hour_var = get_nc_var(f_in, "precip6Hour");
   NcVar *precip24Hour_var = get_nc_var(f_in, "precip24Hour");
   NcVar *snowCover_var = get_nc_var(f_in, "snowCover");

   NcVar *seaLevelPressQty_var = has_var(f_in, "seaLevelPressDD");
   NcVar *visibilityQty_var = has_var(f_in, "visibilityDD");
   NcVar *temperatureQty_var = has_var(f_in, "temperatureDD");
   NcVar *dewpointQty_var = has_var(f_in, "dewpointDD");
   NcVar *windDirQty_var = has_var(f_in, "windDirDD");
   NcVar *windSpeedQty_var = has_var(f_in, "windSpeedDD");
   NcVar *windGustQty_var = has_var(f_in, "windGustDD");
   NcVar *minTemp24HourQty_var = has_var(f_in, "minTemp24HourDD");
   NcVar *maxTemp24HourQty_var = has_var(f_in, "maxTemp24HourDD");
   NcVar *precip1HourQty_var = has_var(f_in, "precip1HourDD");
   NcVar *precip3HourQty_var = has_var(f_in, "precip3HourDD");
   NcVar *precip6HourQty_var = has_var(f_in, "precip6HourDD");
   NcVar *precip24HourQty_var = has_var(f_in, "precip24HourDD");
   NcVar *snowCoverQty_var = has_var(f_in, "snowCoverDD");
   
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

   mlog << Debug(2) << "Processing METAR recs\t\t\t= " << rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

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
   //for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {
   for(i_hdr_s=rec_beg; i_hdr_s<rec_end; i_hdr_s+=BUFFER_SIZE) {
      long *dim2D = new long [2];
      int buf_size = ((rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (rec_end - i_hdr_s);
      dim[0] = buf_size;
      cur[0] = i_hdr_s;
      
      //ConcatString hdr_typ, hdr_sid, hdr_vld;
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];
      
      float seaLevelPress[buf_size];
      float visibility[buf_size];
      float temperature[buf_size];
      float dewpoint[buf_size];
      float windDir[buf_size];
      float windSpeed[buf_size];
      float windGust[buf_size];
      float minTemp24Hour[buf_size];
      float maxTemp24Hour[buf_size];
      float precip1Hour[buf_size];
      float precip3Hour[buf_size];
      float precip6Hour[buf_size];
      float precip24Hour[buf_size];
      float snowCover[buf_size];

      char seaLevelPressQty[buf_size];
      char visibilityQty[buf_size];
      char temperatureQty[buf_size];
      char dewpointQty[buf_size];
      char windDirQty[buf_size];
      char windSpeedQty[buf_size];
      char windGustQty[buf_size];
      char minTemp24HourQty[buf_size];
      char maxTemp24HourQty[buf_size];
      char precip1HourQty[buf_size];
      char precip3HourQty[buf_size];
      char precip6HourQty[buf_size];
      char precip24HourQty[buf_size];
      char snowCoverQty[buf_size];

      char hdr_typ_arr[buf_size][hdr_typ_len];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      get_nc_data(in_hdr_vld_var, cur, dim, tmp_dbl_arr);
      get_nc_data(in_hdr_lat_var, cur, dim, hdr_lat_arr);
      get_nc_data(in_hdr_lon_var, cur, dim, hdr_lon_arr);
      get_filtered_nc_data(in_hdr_elv_var, cur, dim, hdr_elv_arr);

      if (seaLevelPressQty_var) get_nc_data(seaLevelPressQty_var, cur, dim, seaLevelPressQty);
      if (visibilityQty_var) get_nc_data(visibilityQty_var, cur, dim, visibilityQty);
      if (temperatureQty_var) get_nc_data(temperatureQty_var, cur, dim, temperatureQty);
      if (dewpointQty_var) get_nc_data(dewpointQty_var, cur, dim, dewpointQty);
      if (windDirQty_var) get_nc_data(windDirQty_var, cur, dim, windDirQty);
      if (windSpeedQty_var) get_nc_data(windSpeedQty_var, cur, dim, windSpeedQty);
      if (windGustQty_var) get_nc_data(windGustQty_var, cur, dim, windGustQty);
      if (minTemp24HourQty_var) get_nc_data(minTemp24HourQty_var, cur, dim, minTemp24HourQty);
      if (maxTemp24HourQty_var) get_nc_data(maxTemp24HourQty_var, cur, dim, maxTemp24HourQty);
      if (precip1HourQty_var) get_nc_data(precip1HourQty_var, cur, dim, precip1HourQty);
      if (precip3HourQty_var) get_nc_data(precip3HourQty_var, cur, dim, precip3HourQty);
      if (precip6HourQty_var) get_nc_data(precip6HourQty_var, cur, dim, precip6HourQty);
      if (precip24HourQty_var) get_nc_data(precip24HourQty_var, cur, dim, precip24HourQty);
      if (snowCoverQty_var) get_nc_data(snowCoverQty_var, cur, dim, snowCoverQty);
      
      get_filtered_nc_data(seaLevelPress_var, cur, dim, seaLevelPress);
      get_filtered_nc_data(visibility_var, cur, dim, visibility);
      get_filtered_nc_data(temperature_var, cur, dim, temperature);
      get_filtered_nc_data(dewpoint_var, cur, dim, dewpoint);
      get_filtered_nc_data(windDir_var, cur, dim, windDir);
      get_filtered_nc_data(windSpeed_var, cur, dim, windSpeed);
      get_filtered_nc_data(windGust_var, cur, dim, windGust);
      get_filtered_nc_data(minTemp24Hour_var, cur, dim, minTemp24Hour);
      get_filtered_nc_data(maxTemp24Hour_var, cur, dim, maxTemp24Hour);
      get_filtered_nc_data(precip1Hour_var, cur, dim, precip1Hour);
      get_filtered_nc_data(precip3Hour_var, cur, dim, precip3Hour);
      get_filtered_nc_data(precip6Hour_var, cur, dim, precip6Hour);
      get_filtered_nc_data(precip24Hour_var, cur, dim, precip24Hour);
      get_filtered_nc_data(snowCover_var, cur, dim, snowCover);
      
      dim2D[0] = buf_size;
      dim2D[1] = hdr_typ_len;
      get_nc_data(in_hdr_typ_var, cur, dim2D, (char *)&hdr_typ_arr);
      dim2D[1] = hdr_sid_len;
      get_nc_data(in_hdr_sid_var, cur, dim2D, (char *)&hdr_sid_arr);
      
      dim[0] = 1;
      for (i_idx=0; i_idx<buf_size; i_idx++) {
      
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
   
         i_hdr = i_hdr_s + i_idx;
         cur[0] = i_hdr;
         
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";
   
         //
         // Use cur to index into the NetCDF variables.
         //
   
         //
         // Process the latitude, longitude, and elevation.
         //
         //get_nc_var_val(in_hdr_lat_var, cur, dim, hdr_arr[0]);
         //get_nc_var_val(in_hdr_lon_var, cur, dim, hdr_arr[1]);
         //get_nc_var_val(in_hdr_elv_var, cur, dim, hdr_arr[2]);
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];
   
         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1])) continue;
   
         //
         // Process the header type.
         // For METAR or SPECI, encode as ADPSFC.
         // Otherwise, use value from file.
         //
         //get_nc_var_val(in_hdr_typ_var, cur, hdr_typ_len, hdr_typ);
         hdr_typ = hdr_typ_arr[i_idx];
         if(hdr_typ == metar_str || hdr_typ == "SPECI") hdr_typ = "ADPSFC";
         put_nc_var_val(hdr_typ_var, i_hdr, hdr_typ);
   
         //
         // Process the station name.
         //
         //get_nc_var_val(in_hdr_sid_var, cur, hdr_sid_len, hdr_sid);
         hdr_sid = hdr_sid_arr[i_idx];
         put_nc_var_val(hdr_sid_var, i_hdr, hdr_sid);
   
         //
         // Process the observation time.
         //
         //get_nc_var_val(in_hdr_vld_var, cur, dim, tmp_dbl);
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;
         
         unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
         hdr_vld = tmp_str;
         put_nc_var_val(hdr_vld_var, i_hdr, hdr_vld);
   
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
         obs_arr[4] = seaLevelPress[i_idx];
         process_obs(2, conversion, obs_arr, seaLevelPressQty[i_idx], seaLevelPress_var->name());
   
         // Visibility
         obs_arr[4] = visibility[i_idx];
         process_obs(20, conversion, obs_arr, visibilityQty[i_idx], visibility_var->name());
   
         // Temperature
         obs_arr[4] = temperature[i_idx];
         process_obs(11, conversion, obs_arr, temperatureQty[i_idx], temperature_var->name());
   
         // Dewpoint
         obs_arr[4] = dewpoint[i_idx];
         process_obs(17, conversion, obs_arr, dewpointQty[i_idx], dewpoint_var->name());
   
         // Wind Direction
         obs_arr[4] = windDir[i_idx];
         process_obs(31, conversion, obs_arr, windDirQty[i_idx], windDir_var->name());
         wdir = obs_arr[4];
   
         // Wind Speed
         obs_arr[4] = windSpeed[i_idx];
         process_obs(32, conversion, obs_arr, windSpeedQty[i_idx], windSpeed_var->name());
         wind = obs_arr[4];
   
         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
   
         // Write U-component of wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            write_qty(windSpeedQty[i_idx]);
            i_obs++;
         }
   
         // Write V-component of wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            write_qty(windSpeedQty[i_idx]);
            i_obs++;
         }
   
         // Wind Gust
         obs_arr[4] = windGust[i_idx];
         process_obs(180, conversion, obs_arr, windGustQty[i_idx], windGust_var->name());
   
         // Min Temperature - 24 Hour
         obs_arr[4] = minTemp24Hour[i_idx];
         process_obs(16, conversion, obs_arr, minTemp24HourQty[i_idx], minTemp24Hour_var->name());
   
         // Max Temperature - 24 Hour
         obs_arr[4] = maxTemp24Hour[i_idx];
         process_obs(15, conversion, obs_arr, maxTemp24HourQty[i_idx], maxTemp24Hour_var->name());
   
         conversion = 1000.0;
         // Precipitation - 1 Hour
         obs_arr[2] = 1.0*sec_per_hour;
         obs_arr[4] = precip1Hour[i_idx];
         process_obs(61, conversion, obs_arr, precip1HourQty[i_idx], precip1Hour_var->name());
   
         // Precipitation - 3 Hour
         obs_arr[2] = 3.0*sec_per_hour;
         obs_arr[4] = precip3Hour[i_idx];
         process_obs(61, conversion, obs_arr, precip3HourQty[i_idx], precip3Hour_var->name());
   
         // Precipitation - 6 Hour
         obs_arr[2] = 6.0*sec_per_hour;
         obs_arr[4] = precip6Hour[i_idx];
         process_obs(61, conversion, obs_arr, precip6HourQty[i_idx], precip6Hour_var->name());
   
         // Precipitation - 24 Hour
         obs_arr[2] = 24.0*sec_per_hour;
         obs_arr[4] = precip24Hour[i_idx];
         process_obs(61, conversion, obs_arr, precip24HourQty[i_idx], precip24Hour_var->name());
   
         conversion = 1.0;
         // Snow Cover
         obs_arr[2] = bad_data_float;
         obs_arr[4] = snowCover[i_idx];
         process_obs(66, conversion, obs_arr, snowCoverQty[i_idx], snowCover_var->name());
         
      }
      
   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_raob(NcFile *&f_in) {
   int nhdr, nlvl, i_lvl;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   char tmp_str[max_str_len], qty;
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float wdir, wind, ugrd, vgrd;

   int maxlvl_manLevel;
   int maxlvl_sigTLevel;
   int maxlvl_sigWLevel;
   int maxlvl_sigPresWLevel;
   int maxlvl_mTropNum;
   int maxlvl_mWndNum ;
   
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

   NcVar *prMan_var = get_nc_var(f_in, "prMan");
   NcVar *htMan_var = get_nc_var(f_in, "htMan");
   NcVar *tpMan_var = get_nc_var(f_in, "tpMan");
   NcVar *tdMan_var = get_nc_var(f_in, "tdMan");
   NcVar *wdMan_var = get_nc_var(f_in, "wdMan");
   NcVar *wsMan_var = get_nc_var(f_in, "wsMan");
   NcVar *prSigT_var = get_nc_var(f_in, "prSigT");
   NcVar *tpSigT_var = get_nc_var(f_in, "tpSigT");
   NcVar *tdSigT_var = get_nc_var(f_in, "tdSigT");
   NcVar *htSigW_var = get_nc_var(f_in, "htSigW");
   NcVar *wdSigW_var = get_nc_var(f_in, "wdSigW");
   NcVar *wsSigW_var = get_nc_var(f_in, "wsSigW");
   NcVar *prSigW_var = get_nc_var(f_in, "prSigW");
   NcVar *wdSigPrW_var = get_nc_var(f_in, "wdSigPrW");
   NcVar *wsSigPrW_var = get_nc_var(f_in, "wsSigPrW");
   NcVar *prTrop_var = get_nc_var(f_in, "prTrop");
   NcVar *tpTrop_var = get_nc_var(f_in, "tpTrop");
   NcVar *tdTrop_var = get_nc_var(f_in, "tdTrop");
   NcVar *wdTrop_var = get_nc_var(f_in, "wdTrop");
   NcVar *wsTrop_var = get_nc_var(f_in, "wsTrop");
   NcVar *prMaxW_var = get_nc_var(f_in, "prMaxW");
   NcVar *wdMaxW_var = get_nc_var(f_in, "wdMaxW");
   NcVar *wsMaxW_var = get_nc_var(f_in, "wsMaxW");

   NcVar *prManQty_var = has_var(f_in, "prManDD");
   NcVar *htManQty_var = has_var(f_in, "htManDD");
   NcVar *tpManQty_var = has_var(f_in, "tpManDD");
   NcVar *tdManQty_var = has_var(f_in, "tdManDD");
   NcVar *wdManQty_var = has_var(f_in, "wdManDD");
   NcVar *wsManQty_var = has_var(f_in, "wsManDD");
   NcVar *prSigTQty_var = has_var(f_in, "prSigTDD");
   NcVar *tpSigTQty_var = has_var(f_in, "tpSigTDD");
   NcVar *tdSigTQty_var = has_var(f_in, "tdSigTDD");
   NcVar *htSigWQty_var = has_var(f_in, "htSigWDD");
   NcVar *wdSigWQty_var = has_var(f_in, "wdSigWDD");
   NcVar *wsSigWQty_var = has_var(f_in, "wsSigWDD");
   NcVar *prSigWQty_var = has_var(f_in, "prSigWDD");
   NcVar *wdSigPrWQty_var = has_var(f_in, "wdSigPrWDD");
   NcVar *wsSigPrWQty_var = has_var(f_in, "wsSigPrWDD");
   NcVar *prTropQty_var = has_var(f_in, "prTropDD");
   NcVar *tpTropQty_var = has_var(f_in, "tpTropDD");
   NcVar *tdTropQty_var = has_var(f_in, "tdTropDD");
   NcVar *wdTropQty_var = has_var(f_in, "wdTropDD");
   NcVar *wsTropQty_var = has_var(f_in, "wsTropDD");
   NcVar *prMaxWQty_var = has_var(f_in, "prMaxWDD");
   NcVar *wdMaxWQty_var = has_var(f_in, "wdMaxWDD");
   NcVar *wsMaxWQty_var = has_var(f_in, "wsMaxWDD");
   
   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len  = get_nc_dim(f_in, "staNameLen");
   nhdr         = get_nc_dim(f_in, in_recNum_str);
   if(rec_end == 0) rec_end = nhdr;

   get_dim(f_in, "manLevel", maxlvl_manLevel);
   get_dim(f_in, "sigTLevel", maxlvl_sigTLevel);
   get_dim(f_in, "sigWLevel", maxlvl_sigWLevel);
   get_dim(f_in, "sigPresWLevel", maxlvl_sigPresWLevel);
   get_dim(f_in, "mTropNum", maxlvl_mTropNum);
   get_dim(f_in, "mWndNum", maxlvl_mWndNum);
   
   //
   // Setup the output NetCDF file
   //
   setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing RAOB recs\t\t\t= " << rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

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
   //for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {
   for(i_hdr_s=rec_beg; i_hdr_s<rec_end; i_hdr_s+=BUFFER_SIZE) {
      long *dim2D = new long [2];
      long *dim3D = new long [3];
      int buf_size = ((rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (rec_end - i_hdr_s);

      int nlvl_manLevel[buf_size];
      int nlvl_sigTLevel[buf_size];
      int nlvl_sigWLevel[buf_size];
      int nlvl_sigPresWLevel[buf_size];
      int nlvl_mTropNum[buf_size];
      int nlvl_mWndNum[buf_size];
      
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];

      float prMan[buf_size][maxlvl_manLevel];
      float htMan[buf_size][maxlvl_manLevel];
      float tpMan[buf_size][maxlvl_manLevel];
      float tdMan[buf_size][maxlvl_manLevel];
      float wdMan[buf_size][maxlvl_manLevel];
      float wsMan[buf_size][maxlvl_manLevel];
      float prSigT[buf_size][maxlvl_sigTLevel];
      float tpSigT[buf_size][maxlvl_sigTLevel];
      float tdSigT[buf_size][maxlvl_sigTLevel];
      float htSigW[buf_size][maxlvl_sigWLevel];
      float wdSigW[buf_size][maxlvl_sigWLevel];
      float wsSigW[buf_size][maxlvl_sigWLevel];
      float prSigW[buf_size][maxlvl_sigPresWLevel];
      float wdSigPrW[buf_size][maxlvl_sigPresWLevel];
      float wsSigPrW[buf_size][maxlvl_sigPresWLevel];
      float prTrop[buf_size][maxlvl_mTropNum];
      float tpTrop[buf_size][maxlvl_mTropNum];
      float tdTrop[buf_size][maxlvl_mTropNum];
      float wdTrop[buf_size][maxlvl_mTropNum];
      float wsTrop[buf_size][maxlvl_mTropNum];
      float prMaxW[buf_size][maxlvl_mWndNum];
      float wdMaxW[buf_size][maxlvl_mWndNum];
      float wsMaxW[buf_size][maxlvl_mWndNum];

      char prManQty[buf_size][maxlvl_manLevel];
      char htManQty[buf_size][maxlvl_manLevel];
      char tpManQty[buf_size][maxlvl_manLevel];
      char tdManQty[buf_size][maxlvl_manLevel];
      char wdManQty[buf_size][maxlvl_manLevel];
      char wsManQty[buf_size][maxlvl_manLevel];
      char prSigTQty[buf_size][maxlvl_sigTLevel];
      char tpSigTQty[buf_size][maxlvl_sigTLevel];
      char tdSigTQty[buf_size][maxlvl_sigTLevel];
      char htSigWQty[buf_size][maxlvl_sigWLevel];
      char wdSigWQty[buf_size][maxlvl_sigWLevel];
      char wsSigWQty[buf_size][maxlvl_sigWLevel];
      char prSigWQty[buf_size][maxlvl_sigPresWLevel];
      char wdSigPrWQty[buf_size][maxlvl_sigPresWLevel];
      char wsSigPrWQty[buf_size][maxlvl_sigPresWLevel];
      char prTropQty[buf_size][maxlvl_mTropNum];
      char tpTropQty[buf_size][maxlvl_mTropNum];
      char tdTropQty[buf_size][maxlvl_mTropNum];
      char wdTropQty[buf_size][maxlvl_mTropNum];
      char wsTropQty[buf_size][maxlvl_mTropNum];
      char prMaxWQty[buf_size][maxlvl_mWndNum];
      char wdMaxWQty[buf_size][maxlvl_mWndNum];
      char wsMaxWQty[buf_size][maxlvl_mWndNum];

      //char hdr_typ_arr[buf_size][hdr_typ_len];
      char hdr_sid_arr[buf_size][hdr_sid_len];
      //char *hdr_typ_arr_ptr = &hdr_typ_arr[0];
      //char *hdr_sid_arr_ptr = &hdr_sid_arr[0];

      cur[0] = i_hdr_s;

      dim[0] = buf_size;
      get_nc_data(in_man_var, cur, dim, nlvl_manLevel);
      get_nc_data(in_sigt_var, cur, dim, nlvl_sigTLevel);
      get_nc_data(in_sigw_var, cur, dim, nlvl_sigWLevel);
      get_nc_data(in_sigprw_var, cur, dim, nlvl_sigPresWLevel);
      get_nc_data(in_trop_var, cur, dim, nlvl_mTropNum);
      get_nc_data(in_maxw_var, cur, dim, nlvl_mWndNum);

      get_nc_data(in_hdr_vld_var, cur, dim, tmp_dbl_arr);
      get_nc_data(in_hdr_lat_var, cur, dim, hdr_lat_arr);
      get_nc_data(in_hdr_lon_var, cur, dim, hdr_lon_arr);
      get_filtered_nc_data(in_hdr_elv_var, cur, dim, hdr_elv_arr);

      dim2D[0] = buf_size;
      //dim2D[1] = hdr_typ_len;
      //get_nc_data(in_hdr_typ_var, cur, dim2D, (char *)&hdr_typ_arr[0]);
      dim2D[1] = hdr_sid_len;
      get_nc_data(in_hdr_sid_var, cur, dim2D, (char *)&hdr_sid_arr);
      
      dim3D[0] = buf_size;
      dim3D[1] = maxlvl_manLevel;
      if (prManQty_var) get_nc_data(prManQty_var, cur, dim3D, (char *)&prManQty);
      if (htManQty_var) get_nc_data(htManQty_var, cur, dim3D, (char *)&htManQty);
      if (tpManQty_var) get_nc_data(tpManQty_var, cur, dim3D, (char *)&tpManQty);
      if (tdManQty_var) get_nc_data(tdManQty_var, cur, dim3D, (char *)&tdManQty);
      if (wdManQty_var) get_nc_data(wdManQty_var, cur, dim3D, (char *)&wdManQty);
      if (wsManQty_var) get_nc_data(wsManQty_var, cur, dim3D, (char *)&wsManQty);
      dim3D[1] = maxlvl_sigTLevel;
      if (prSigTQty_var) get_nc_data(prSigTQty_var, cur, dim3D, (char *)&prSigTQty);
      if (tpSigTQty_var) get_nc_data(tpSigTQty_var, cur, dim3D, (char *)&tpSigTQty);
      if (tdSigTQty_var) get_nc_data(tdSigTQty_var, cur, dim3D, (char *)&tdSigTQty);
      dim3D[1] = maxlvl_sigWLevel;
      if (htSigWQty_var) get_nc_data(htSigWQty_var, cur, dim3D, (char *)&htSigWQty);
      if (wdSigWQty_var) get_nc_data(wdSigWQty_var, cur, dim3D, (char *)&wdSigWQty);
      if (wsSigWQty_var) get_nc_data(wsSigWQty_var, cur, dim3D, (char *)&wsSigWQty);
      dim3D[1] = maxlvl_sigPresWLevel;
      if (prSigWQty_var  ) get_nc_data(prSigWQty_var,   cur, dim3D,   (char *)&prSigWQty);
      if (wdSigPrWQty_var) get_nc_data(wdSigPrWQty_var, cur, dim3D, (char *)&wdSigPrWQty);
      if (wsSigPrWQty_var) get_nc_data(wsSigPrWQty_var, cur, dim3D, (char *)&wsSigPrWQty);
      dim3D[1] = maxlvl_mTropNum;
      if (prTropQty_var) get_nc_data(prTropQty_var, cur, dim3D, (char *)&prTropQty);
      if (tpTropQty_var) get_nc_data(tpTropQty_var, cur, dim3D, (char *)&tpTropQty);
      if (tdTropQty_var) get_nc_data(tdTropQty_var, cur, dim3D, (char *)&tdTropQty);
      if (wdTropQty_var) get_nc_data(wdTropQty_var, cur, dim3D, (char *)&wdTropQty);
      if (wsTropQty_var) get_nc_data(wsTropQty_var, cur, dim3D, (char *)&wsTropQty);
      dim3D[1] = maxlvl_mWndNum;
      if (prMaxWQty_var) get_nc_data(prMaxWQty_var, cur, dim3D, (char *)&prMaxWQty);
      if (wdMaxWQty_var) get_nc_data(wdMaxWQty_var, cur, dim3D, (char *)&wdMaxWQty);
      if (wsMaxWQty_var) get_nc_data(wsMaxWQty_var, cur, dim3D, (char *)&wsMaxWQty);

      dim3D[1] = maxlvl_manLevel;
      get_filtered_nc_data_2d(prMan_var, cur, dim3D, (float *)&prMan);
      get_filtered_nc_data_2d(htMan_var, cur, dim3D, (float *)&htMan);
      get_filtered_nc_data_2d(tpMan_var, cur, dim3D, (float *)&tpMan);
      get_filtered_nc_data_2d(tdMan_var, cur, dim3D, (float *)&tdMan);
      get_filtered_nc_data_2d(wdMan_var, cur, dim3D, (float *)&wdMan);
      get_filtered_nc_data_2d(wsMan_var, cur, dim3D, (float *)&wsMan);
      dim3D[1] = maxlvl_sigTLevel;
      get_filtered_nc_data_2d(prSigT_var, cur, dim3D, (float *)&prSigT);
      get_filtered_nc_data_2d(tpSigT_var, cur, dim3D, (float *)&tpSigT);
      get_filtered_nc_data_2d(tdSigT_var, cur, dim3D, (float *)&tdSigT);
      dim3D[1] = maxlvl_sigWLevel;
      get_filtered_nc_data_2d(htSigW_var, cur, dim3D, (float *)&htSigW);
      get_filtered_nc_data_2d(wdSigW_var, cur, dim3D, (float *)&wdSigW);
      get_filtered_nc_data_2d(wsSigW_var, cur, dim3D, (float *)&wsSigW);
      dim3D[1] = maxlvl_sigPresWLevel;
      get_filtered_nc_data_2d(prSigW_var,   cur, dim3D,   (float *)&prSigW);
      get_filtered_nc_data_2d(wdSigPrW_var, cur, dim3D, (float *)&wdSigPrW);
      get_filtered_nc_data_2d(wsSigPrW_var, cur, dim3D, (float *)&wsSigPrW);
      dim3D[1] = maxlvl_mTropNum;
      get_filtered_nc_data_2d(prTrop_var, cur, dim3D, (float *)&prTrop);
      get_filtered_nc_data_2d(tpTrop_var, cur, dim3D, (float *)&tpTrop);
      get_filtered_nc_data_2d(tdTrop_var, cur, dim3D, (float *)&tdTrop);
      get_filtered_nc_data_2d(wdTrop_var, cur, dim3D, (float *)&wdTrop);
      get_filtered_nc_data_2d(wsTrop_var, cur, dim3D, (float *)&wsTrop);
      dim3D[1] = maxlvl_mWndNum;
      get_filtered_nc_data_2d(prMaxW_var, cur, dim3D, (float *)&prMaxW);
      get_filtered_nc_data_2d(wdMaxW_var, cur, dim3D, (float *)&wdMaxW);
      get_filtered_nc_data_2d(wsMaxW_var, cur, dim3D, (float *)&wsMaxW);

      dim[0] = 1;
      for (int i_idx=0; i_idx<buf_size; i_idx++) {
      
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
         
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";
         
         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;
         
         //
         // Process the latitude, longitude, and elevation.
         //
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];
         
         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1])) continue;
         
         //
         // Process the header type.
         // For RAOB, store as ADPUPA.
         //
         hdr_typ = "ADPUPA";
         put_nc_var_val(hdr_typ_var, i_hdr, hdr_typ);
         
         //
         // Process the station name.
         //
         hdr_sid = hdr_sid_arr[i_idx];
         put_nc_var_val(hdr_sid_var, i_hdr, hdr_sid);
         
         //
         // Process the observation time.
         //
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;
         unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
         hdr_vld = tmp_str;
         put_nc_var_val(hdr_vld_var, i_hdr, hdr_vld);
         
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
         //nlvl = get_num_lvl(in_man_var, "manLevel", cur, dim);
         nlvl = nlvl_manLevel[i_idx];
         //nlvl = maxlvl_manLevel;
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {
         
            mlog << Debug(3) << "  Mandatory Level: " << i_lvl << "\n";
         
            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
         
            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prMan[i_idx][i_lvl];
            obs_arr[3] = htMan[i_idx][i_lvl];
         
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
            // Pressure
            obs_arr[4] = prMan[i_idx][i_lvl];
            process_obs(1, conversion, obs_arr, prManQty[i_idx][i_lvl], prMan_var->name());
         
            // Height
            obs_arr[4] = htMan[i_idx][i_lvl];
            process_obs(7, conversion, obs_arr, htManQty[i_idx][i_lvl], htMan_var->name());
         
            // Temperature
            obs_arr[4] = tpMan[i_idx][i_lvl];
            process_obs(11, conversion, obs_arr, tpManQty[i_idx][i_lvl], tpMan_var->name());
         
            // Dewpoint
            obs_arr[4] = tdMan[i_idx][i_lvl];
            process_obs(17, conversion, obs_arr, tdManQty[i_idx][i_lvl], tdMan_var->name());
         
            // Wind Direction
            obs_arr[4] = wdMan[i_idx][i_lvl];
            process_obs(31, conversion, obs_arr, wdManQty[i_idx][i_lvl], wdMan_var->name());
            wdir = obs_arr[4];
         
            // Wind Speed
            qty = wsManQty[i_idx][i_lvl];
            obs_arr[4] = wsMan[i_idx][i_lvl];
            process_obs(32, conversion, obs_arr, qty, wsMan_var->name());
            wind = obs_arr[4];
         
            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
            // Write U-component of wind
            obs_arr[1] = 33;
            obs_arr[4] = ugrd;
            if(!is_bad_data(ugrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
            // Write V-component of wind
            obs_arr[1] = 34;
            obs_arr[4] = vgrd;
            if(!is_bad_data(vgrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
         } // end for i_lvl
         
         //
         // Loop through the significant levels wrt T
         //
         //nlvl = get_num_lvl(in_sigt_var, "sigTLevel", cur, dim);
         nlvl = nlvl_sigTLevel[i_idx];
         //nlvl = maxlvl_sigTLevel;
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {
         
            mlog << Debug(3) << "  Significant T Level: " << i_lvl << "\n";
         
            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
         
            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prSigT[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;
         
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
            // Temperature
            obs_arr[4] = tpSigT[i_idx][i_lvl];
            process_obs(11, conversion, obs_arr, tpSigTQty[i_idx][i_lvl], tpSigT_var->name());
         
            // Dewpoint
            obs_arr[4] = tdSigT[i_idx][i_lvl];
            process_obs(17, conversion, obs_arr, tdSigTQty[i_idx][i_lvl], tdSigT_var->name());
         
         } // end for i_lvl
         
         //
         // Loop through the significant levels wrt W
         //
         //nlvl = get_num_lvl(in_sigw_var, "sigWLevel", cur, dim);
         //nlvl = maxlvl_sigWLevel;
         nlvl = nlvl_sigWLevel[i_idx];
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
            obs_arr[3] = htSigW[i_idx][i_lvl];
         
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
            // Wind Direction
            obs_arr[4] = wdSigW[i_idx][i_lvl];
            process_obs(31, conversion, obs_arr, wdSigWQty[i_idx][i_lvl], wdSigW_var->name());
            wdir = obs_arr[4];
         
            // Wind Speed
            qty = wsSigWQty[i_idx][i_lvl];
            obs_arr[4] = wsSigW[i_idx][i_lvl];
            process_obs(32, conversion, obs_arr, qty, wsSigW_var->name());
            wind = obs_arr[4];
         
            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
            // Write U-component of wind
            obs_arr[1] = 33;
            obs_arr[4] = ugrd;
            if(!is_bad_data(ugrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
            // Write V-component of wind
            obs_arr[1] = 34;
            obs_arr[4] = vgrd;
            if(!is_bad_data(vgrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
         } // end for i_lvl
         
         //
         // Loop through the significant levels wrt W-by-P
         //
         //nlvl = get_num_lvl(in_sigprw_var, "sigPresWLevel", cur, dim);
         nlvl = nlvl_sigPresWLevel[i_idx];
         //nlvl = maxlvl_sigPresWLevel;
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {
         
            mlog << Debug(3) << "  Significant W-by-P Level: " << i_lvl << "\n";
         
            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
         
            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prSigW[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;
         
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
            // Wind Direction
            obs_arr[4] = wdSigPrW[i_idx][i_lvl];
            process_obs(31, conversion, obs_arr, wdSigPrWQty[i_idx][i_lvl], wdSigPrW_var->name());
            wdir = obs_arr[4];
         
            // Wind Speed
            qty = wsSigPrWQty[i_idx][i_lvl];
            obs_arr[4] = wsSigPrW[i_idx][i_lvl];
            process_obs(32, conversion, obs_arr, qty, wsSigPrW_var->name());
            wind = obs_arr[4];
         
            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
            // Write U-component of wind
            obs_arr[1] = 33;
            obs_arr[4] = ugrd;
            if(!is_bad_data(ugrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
            // Write V-component of wind
            obs_arr[1] = 34;
            obs_arr[4] = vgrd;
            if(!is_bad_data(vgrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
         } // end for i_lvl
         
         //
         // Loop through the tropopause levels
         //
         //nlvl = get_num_lvl(in_trop_var, "mTropNum", cur, dim);
         nlvl = nlvl_mTropNum[i_idx];
         //nlvl = maxlvl_mTropNum;
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {
         
            mlog << Debug(3) << "  Tropopause Level: " << i_lvl << "\n";
         
            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
         
            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prTrop[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;
         
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
            // Temperature
            obs_arr[4] = tpTrop[i_idx][i_lvl];
            process_obs(11, conversion, obs_arr, tpTropQty[i_idx][i_lvl], tpTrop_var->name());
         
            // Dewpoint
            obs_arr[4] = tdTrop[i_idx][i_lvl];
            process_obs(17, conversion, obs_arr, tdTropQty[i_idx][i_lvl], tdTrop_var->name());
         
            // Wind Direction
            obs_arr[4] = wdTrop[i_idx][i_lvl];
            process_obs(31, conversion, obs_arr, wdTropQty[i_idx][i_lvl], wdTrop_var->name());
            wdir = obs_arr[4];
         
            // Wind Speed
            qty = wsTropQty[i_idx][i_lvl];
            obs_arr[4] = wsTrop[i_idx][i_lvl];
            process_obs(32, conversion, obs_arr, qty, wsTrop_var->name());
            wind = obs_arr[4];
         
            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
            // Write U-component of wind
            obs_arr[1] = 33;
            obs_arr[4] = ugrd;
            if(!is_bad_data(ugrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
            // Write V-component of wind
            obs_arr[1] = 34;
            obs_arr[4] = vgrd;
            if(!is_bad_data(vgrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
         } // end for i_lvl
         
         //
         // Loop through the maximum wind levels
         //
         //nlvl = get_num_lvl(in_maxw_var, "mWndNum", cur, dim);
         nlvl = nlvl_mWndNum[i_idx];
         //nlvl = maxlvl_mWndNum;
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {
         
            mlog << Debug(3) << "  Maximum Wind Level: " << i_lvl << "\n";
         
            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
         
            //
            // Get the pressure and height for this level
            //
            obs_arr[2] = prMaxW[i_idx][i_lvl];
            obs_arr[3] = bad_data_float;
         
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
            // Wind Direction
            obs_arr[4] = wdMaxW[i_idx][i_lvl];
            process_obs(31, conversion, obs_arr, wdMaxWQty[i_idx][i_lvl], wdMaxW_var->name());
            wdir = obs_arr[4];
         
            // Wind Speed
            qty = wsMaxWQty[i_idx][i_lvl];
            obs_arr[4] = wsMaxW[i_idx][i_lvl];
            process_obs(32, conversion, obs_arr, qty, wsMaxW_var->name());
            wind = obs_arr[4];
         
            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
            // Write U-component of wind
            obs_arr[1] = 33;
            obs_arr[4] = ugrd;
            if(!is_bad_data(ugrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
            // Write V-component of wind
            obs_arr[1] = 34;
            obs_arr[4] = vgrd;
            if(!is_bad_data(vgrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
         } // end for i_lvl

      } // end for i_hdr
   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_profiler(NcFile *&f_in) {
   int nhdr, nlvl, i_lvl;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   char tmp_str[max_str_len];
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float pressure;

   //
   // Input header variables:
   // Note: hdr_typ is always set to ADPUPA
   //
   NcVar *in_hdr_sid_var = get_nc_var(f_in, "staName");
   NcVar *in_hdr_vld_var = get_nc_var(f_in, "timeObs");
   NcVar *in_hdr_lat_var = get_nc_var(f_in, "staLat");
   NcVar *in_hdr_lon_var = get_nc_var(f_in, "staLon");
   NcVar *in_hdr_elv_var = get_nc_var(f_in, "staElev");
   NcVar *in_uComponent_var = get_nc_var(f_in, "uComponent");
   NcVar *in_vComponent_var = get_nc_var(f_in, "vComponent");
   NcVar *in_uComponentQty_var = has_var(f_in, "uComponentDD");
   NcVar *in_vComponentQty_var = has_var(f_in, "vComponentDD");

   //
   // Variables for vertical level information
   //
   NcVar *in_pressure_var = get_nc_var(f_in, "pressure");
   NcVar* var_levels = get_nc_var(f_in, "levels");

   //
   // Retrieve applicable dimensions
   //
   nlvl         = get_nc_dim(f_in, "level");
   hdr_sid_len  = get_nc_dim(f_in, "staNamLen");
   nhdr         = get_nc_dim(f_in, in_recNum_str);
   if(rec_end == 0) rec_end = nhdr;

   //
   // Setup the output NetCDF file
   //
   setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing PROFILER recs\t\t= " << rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [3];
   cur[0] = cur[1] = cur[2] = 0;
   long *dim = new long [3];
   dim[0] = dim[1] = dim[2] = 1;

   //int[] hdr_lat_arr = new int[BUFFER_SIZE];
   
   //
   // Loop through each record and get the header data.
   //
   //for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {
   for(i_hdr_s=rec_beg; i_hdr_s<rec_end; i_hdr_s+=BUFFER_SIZE) {
      //long *dim3D = new long [3];
      int buf_size = ((rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (rec_end - i_hdr_s);
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      float pressure_arr[buf_size];
      float levels_arr[buf_size][nlvl];
      float uComponent_arr[buf_size][nlvl];
      float vComponent_arr[buf_size][nlvl];
      char uComponentQty_arr[buf_size][nlvl];
      char vComponentQty_arr[buf_size][nlvl];

      cur[0] = i_hdr_s;
      dim[0] = buf_size;
      get_nc_data(in_hdr_vld_var, cur, dim, tmp_dbl_arr);
      get_nc_data(in_hdr_lat_var, cur, dim, hdr_lat_arr);
      get_nc_data(in_hdr_lon_var, cur, dim, hdr_lon_arr);
      get_filtered_nc_data(in_hdr_elv_var, cur, dim, hdr_elv_arr);
      get_filtered_nc_data(in_pressure_var, cur, dim, pressure_arr);
      
      dim[1] = hdr_sid_len;
      get_nc_data(in_hdr_sid_var, cur, dim, (char *)hdr_sid_arr);
      
      dim[1] = nlvl;
      get_nc_data(var_levels, cur, dim, (float *)levels_arr);
      if (in_uComponentQty_var) get_nc_data(in_uComponentQty_var, cur, dim, (char *)uComponentQty_arr);
      if (in_vComponentQty_var) get_nc_data(in_vComponentQty_var, cur, dim, (char *)vComponentQty_arr);
      get_filtered_nc_data_2d(in_uComponent_var, cur, dim, (float *)uComponent_arr);
      get_filtered_nc_data_2d(in_vComponent_var, cur, dim, (float *)vComponent_arr);
      
      
      dim[0] = 1;
      dim[1] = 1;
      for (int i_idx=0; i_idx<buf_size; i_idx++) {
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
      
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";
      
         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;
      
         //
         // Process the latitude, longitude, and elevation.
         //
         //get_nc_var_val(in_hdr_lat_var, cur, dim, hdr_arr[0]);
         //get_nc_var_val(in_hdr_lon_var, cur, dim, hdr_arr[1]);
         //get_nc_var_val(in_hdr_elv_var, cur, dim, hdr_arr[2]);
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];
      
         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1])) continue;
      
         //
         // Process the header type.
         // For PROFILER, store as ADPUPA.
         //
         hdr_typ = "ADPUPA";
         put_nc_var_val(hdr_typ_var, i_hdr, hdr_typ);
      
         //
         // Process the station name.
         //
         //get_nc_var_val(in_hdr_sid_var, cur, hdr_sid_len, hdr_sid);
         hdr_sid = hdr_sid_arr[i_idx];
         put_nc_var_val(hdr_sid_var, i_hdr, hdr_sid);
      
         //
         // Process the observation time.
         //
         //get_nc_var_val(in_hdr_vld_var, cur, dim, tmp_dbl);
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;
         unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
         hdr_vld = tmp_str;
         put_nc_var_val(hdr_vld_var, i_hdr, hdr_vld);
      
         //
         // Write the header array to the output file.
         //
         put_nc_var_arr(hdr_arr_var, i_hdr, hdr_arr_len, hdr_arr);
      
         //
         // Initialize the observation array: hdr_id
         //
         obs_arr[0] = (float) i_hdr;
      
         //
         // Get the pressure for the current level
         //
         //get_nc_var_val(in_pressure_var, cur, dim, pressure);
         pressure = pressure_arr[i_idx];
      
         //
         // Loop through the mandatory levels
         //
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {
      
            mlog << Debug(3) << "  Level: " << i_lvl << "\n";
      
            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
      
            //
            // Set the pressure and height for this level
            //
            obs_arr[2] = pressure;
            //get_nc_var_val(var_levels, cur, dim, obs_arr[3]);
            obs_arr[3] = levels_arr[i_idx][i_lvl];
      
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
      
            // Wind U
            //process_obs(f_in, "uComponent", cur, dim, 33, conversion, obs_arr);
            obs_arr[4] = uComponent_arr[i_idx][i_lvl];
            process_obs(33, conversion, obs_arr, uComponentQty_arr[i_idx][i_lvl], in_uComponent_var->name());
      
            // Wind V
            //process_obs(f_in, "vComponent", cur, dim, 34, conversion, obs_arr);
            obs_arr[4] = vComponent_arr[i_idx][i_lvl];
            process_obs(34, conversion, obs_arr, vComponentQty_arr[i_idx][i_lvl], in_vComponent_var->name());
      
         } // end for i_lvl
      
      } // end for i_hdr
   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_maritime(NcFile *&f_in) {
   int nhdr;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   char tmp_str[max_str_len];
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float pressure;

   //
   // Input header variables:
   // Note: hdr_typ is always set to ADPUPA
   //
   NcVar *in_hdr_sid_var = get_nc_var(f_in, "stationName");
   NcVar *in_hdr_vld_var = get_nc_var(f_in, "timeObs");
   NcVar *in_hdr_lat_var = get_nc_var(f_in, "latitude");
   NcVar *in_hdr_lon_var = get_nc_var(f_in, "longitude");
   NcVar *in_hdr_elv_var = get_nc_var(f_in, "elevation");

   //
   // Variables for vertical level information
   //
   NcVar *in_pressure_var = get_nc_var(f_in, "stationPress");
   
   NcVar *in_windDir_var = get_nc_var(f_in, "windDir");
   NcVar *in_windSpeed_var = get_nc_var(f_in, "windSpeed");
   NcVar *in_temperature_var = get_nc_var(f_in, "temperature");
   NcVar *in_dewpoint_var = get_nc_var(f_in, "dewpoint");
   NcVar *in_seaLevelPress_var = get_nc_var(f_in, "seaLevelPress");
   NcVar *in_windGust_var = get_nc_var(f_in, "windGust");
   NcVar *in_precip1Hour_var = get_nc_var(f_in, "precip1Hour");
   NcVar *in_precip6Hour_var = get_nc_var(f_in, "precip6Hour");
   NcVar *in_precip12Hour_var = get_nc_var(f_in, "precip12Hour");
   NcVar *in_precip18Hour_var = get_nc_var(f_in, "precip18Hour");
   NcVar *in_precip24Hour_var = get_nc_var(f_in, "precip24Hour");
   
   NcVar *in_windDirQty_var = has_var(f_in, "windDirDD");
   NcVar *in_windSpeedQty_var = has_var(f_in, "windSpeedDD");
   NcVar *in_temperatureQty_var = has_var(f_in, "temperatureDD");
   NcVar *in_dewpointQty_var = has_var(f_in, "dewpointDD");
   NcVar *in_seaLevelPressQty_var = has_var(f_in, "seaLevelPressDD");
   NcVar *in_windGustQty_var = has_var(f_in, "windGustDD");
   NcVar *in_precip1HourQty_var = has_var(f_in, "precip1HourDD");
   NcVar *in_precip6HourQty_var = has_var(f_in, "precip6HourDD");
   NcVar *in_precip12HourQty_var = has_var(f_in, "precip12HourDD");
   NcVar *in_precip18HourQty_var = has_var(f_in, "precip18HourDD");
   NcVar *in_precip24HourQty_var = has_var(f_in, "precip24HourDD");

   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len  = get_nc_dim(f_in, "maxStaNamLen");
   nhdr         = get_nc_dim(f_in, in_recNum_str);
   if(rec_end == 0) rec_end = nhdr;

   //
   // Setup the output NetCDF file
   //
   setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing MARITIME recs\t\t= " << rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

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
   //for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {
   for(i_hdr_s=rec_beg; i_hdr_s<rec_end; i_hdr_s+=BUFFER_SIZE) {
      int buf_size = ((rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (rec_end - i_hdr_s);
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      float pressure_arr[buf_size];
      
      float windDir_arr[buf_size];
      float windSpeed_arr[buf_size];
      float temperature_arr[buf_size];
      float dewpoint_arr[buf_size];
      float seaLevelPress_arr[buf_size];
      float windGust_arr[buf_size];
      float precip1Hour_arr[buf_size];
      float precip6Hour_arr[buf_size];
      float precip12Hour_arr[buf_size];
      float precip18Hour_arr[buf_size];
      float precip24Hour_arr[buf_size];
      char windDirQty_arr[buf_size];
      char windSpeedQty_arr[buf_size];
      char temperatureQty_arr[buf_size];
      char dewpointQty_arr[buf_size];
      char seaLevelPressQty_arr[buf_size];
      char windGustQty_arr[buf_size];
      char precip1HourQty_arr[buf_size];
      char precip6HourQty_arr[buf_size];
      char precip12HourQty_arr[buf_size];
      char precip18HourQty_arr[buf_size];
      char precip24HourQty_arr[buf_size];

      cur[0] = i_hdr_s;
      dim[0] = buf_size;
      get_nc_data(in_hdr_vld_var, cur, dim, tmp_dbl_arr);
      get_nc_data(in_hdr_lat_var, cur, dim, hdr_lat_arr);
      get_nc_data(in_hdr_lon_var, cur, dim, hdr_lon_arr);
      get_filtered_nc_data(in_hdr_elv_var, cur, dim, hdr_elv_arr);
      //get_filtered_nc_data(in_pressure_var, cur, dim, (float *)pressure_arr);
      
      if (in_windDirQty_var) get_nc_data(in_windDirQty_var, cur, dim, windDirQty_arr);
      if (in_windSpeedQty_var) get_nc_data(in_windSpeedQty_var, cur, dim, windSpeedQty_arr);
      if (in_temperatureQty_var) get_nc_data(in_temperatureQty_var, cur, dim, temperatureQty_arr);
      if (in_dewpointQty_var) get_nc_data(in_dewpointQty_var, cur, dim, dewpointQty_arr);
      if (in_seaLevelPressQty_var) get_nc_data(in_seaLevelPressQty_var, cur, dim, seaLevelPressQty_arr);
      if (in_windGustQty_var) get_nc_data(in_windGustQty_var, cur, dim, windGustQty_arr);
      if (in_precip1HourQty_var) get_nc_data(in_precip1HourQty_var, cur, dim, precip1HourQty_arr);
      if (in_precip6HourQty_var) get_nc_data(in_precip6HourQty_var, cur, dim, precip6HourQty_arr);
      if (in_precip12HourQty_var) get_nc_data(in_precip12HourQty_var, cur, dim, precip12HourQty_arr);
      if (in_precip18HourQty_var) get_nc_data(in_precip18HourQty_var, cur, dim, precip18HourQty_arr);
      if (in_precip24HourQty_var) get_nc_data(in_precip24HourQty_var, cur, dim, precip24HourQty_arr);
      
      get_filtered_nc_data(in_pressure_var, cur, dim, pressure_arr);
      get_filtered_nc_data(in_windDir_var, cur, dim, windDir_arr);
      get_filtered_nc_data(in_windSpeed_var, cur, dim, windSpeed_arr);
      get_filtered_nc_data(in_temperature_var, cur, dim, temperature_arr);
      get_filtered_nc_data(in_dewpoint_var, cur, dim, dewpoint_arr);
      get_filtered_nc_data(in_seaLevelPress_var, cur, dim, seaLevelPress_arr);
      get_filtered_nc_data(in_windGust_var, cur, dim, windGust_arr);
      get_filtered_nc_data(in_precip1Hour_var, cur, dim, precip1Hour_arr);
      get_filtered_nc_data(in_precip6Hour_var, cur, dim, precip6Hour_arr);
      get_filtered_nc_data(in_precip12Hour_var, cur, dim, precip12Hour_arr);
      get_filtered_nc_data(in_precip18Hour_var, cur, dim, precip18Hour_arr);
      get_filtered_nc_data(in_precip24Hour_var, cur, dim, precip24Hour_arr);

      dim[1] = hdr_sid_len;
      get_nc_data(in_hdr_sid_var, cur, dim, (char *)hdr_sid_arr);
      
      dim[0] = 1;
      dim[1] = 1;

      for (int i_idx=0; i_idx<buf_size; i_idx++) {
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

      
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";
         
         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;
         
         //
         // Process the latitude, longitude, and elevation.
         //
         //get_nc_var_val(in_hdr_lat_var, cur, dim, hdr_arr[0]);
         //get_nc_var_val(in_hdr_lon_var, cur, dim, hdr_arr[1]);
         //get_nc_var_val(in_hdr_elv_var, cur, dim, hdr_arr[2]);
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];
         
         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1])) continue;
         
         //
         // Process the header type.
         // For maritime, store as SFCSHP.
         //
         hdr_typ = "SFCSHP";
         put_nc_var_val(hdr_typ_var, i_hdr, hdr_typ);
         
         //
         // Process the station name.
         //
         //get_nc_var_val(in_hdr_sid_var, cur, hdr_sid_len, hdr_sid);
         hdr_sid = hdr_sid_arr[i_idx];
         put_nc_var_val(hdr_sid_var, i_hdr, hdr_sid);
         
         //
         // Process the observation time.
         //
         //get_nc_var_val(in_hdr_vld_var, cur, dim, tmp_dbl);
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;
         unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
         hdr_vld = tmp_str;
         put_nc_var_val(hdr_vld_var, i_hdr, hdr_vld);
         
         //
         // Write the header array to the output file.
         //
         put_nc_var_arr(hdr_arr_var, i_hdr, hdr_arr_len, hdr_arr);
         
         //
         // Initialize the observation array: hdr_id
         //
         obs_arr[0] = (float) i_hdr;
         
         //
         // Get the pressure for the current level
         //
         //get_nc_var_val(in_pressure_var, cur, dim, pressure);
         pressure = pressure_arr[i_idx];
         
         //
         // Set the pressure and height for this level
         //
         obs_arr[2] = pressure;
         obs_arr[3] = hdr_arr[2];
         
         //
         // Check for bad data
         //
         if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
         // Wind Direction
         obs_arr[4] = windDir_arr[i_idx];
         process_obs(31, conversion, obs_arr, windDirQty_arr[i_idx], in_windDir_var->name());
         
         // Wind Speed
         obs_arr[4] = windSpeed_arr[i_idx];
         process_obs(32, conversion, obs_arr, windSpeedQty_arr[i_idx], in_windSpeed_var->name());
         
         // Temperature
         obs_arr[4] = temperature_arr[i_idx];
         process_obs(11, conversion, obs_arr, temperatureQty_arr[i_idx], in_temperature_var->name());
         
         // Dew Point temperature
         obs_arr[4] = dewpoint_arr[i_idx];
         process_obs(17, conversion, obs_arr, dewpointQty_arr[i_idx], in_dewpoint_var->name());
         
         // Pressure reduced to MSL
         obs_arr[4] = seaLevelPress_arr[i_idx];
         process_obs(2, conversion, obs_arr, seaLevelPressQty_arr[i_idx], in_seaLevelPress_var->name());
         
         // Surface wind gust
         obs_arr[4] = windGust_arr[i_idx];
         process_obs(180, conversion, obs_arr, windGustQty_arr[i_idx], in_windGust_var->name());
         
         // APCP_01
         obs_arr[2] = 3600;
         obs_arr[4] = precip1Hour_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip1HourQty_arr[i_idx], in_precip1Hour_var->name());
         
         // APCP_06
         obs_arr[2] = 21600;
         obs_arr[4] = precip6Hour_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip6HourQty_arr[i_idx], in_precip6Hour_var->name());
         
         // APCP_12
         obs_arr[2] = 43200;
         obs_arr[4] = precip12Hour_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip12HourQty_arr[i_idx], in_precip12Hour_var->name());
         
         // APCP_18
         obs_arr[2] = 64800;
         obs_arr[4] = precip18Hour_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip18HourQty_arr[i_idx], in_precip18Hour_var->name());
         
         // APCP_24
         obs_arr[2] = 86400;
         obs_arr[4] = precip24Hour_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip24HourQty_arr[i_idx], in_precip24Hour_var->name());
      }

   } // end for i_hdr

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}
////////////////////////////////////////////////////////////////////////

void process_madis_mesonet(NcFile *&f_in) {
   int nhdr;
   long i_hdr, i_hdr_s;
   int hdr_sid_len;
   double tmp_dbl;
   char tmp_str[max_str_len];
   ConcatString hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float wdir, wind, ugrd, vgrd;

   //
   // Input header variables
   //
   NcVar *in_hdr_sid_var = get_nc_var(f_in, "stationId");
   NcVar *in_hdr_vld_var = get_nc_var(f_in, "observationTime");
   NcVar *in_hdr_lat_var = get_nc_var(f_in, "latitude");
   NcVar *in_hdr_lon_var = get_nc_var(f_in, "longitude");
   NcVar *in_hdr_elv_var = get_nc_var(f_in, "elevation");

   NcVar *in_temperature_var = get_nc_var(f_in, "temperature");
   NcVar *in_dewpoint_var = get_nc_var(f_in, "dewpoint");
   NcVar *in_relHumidity_var = get_nc_var(f_in, "relHumidity");
   NcVar *in_stationPressure_var = get_nc_var(f_in, "stationPressure");
   NcVar *in_seaLevelPressure_var = get_nc_var(f_in, "seaLevelPressure");
   NcVar *in_windDir_var = get_nc_var(f_in, "windDir");
   NcVar *in_windSpeed_var = get_nc_var(f_in, "windSpeed");
   NcVar *in_windGust_var = get_nc_var(f_in, "windGust");
   NcVar *in_visibility_var = get_nc_var(f_in, "visibility");
   NcVar *in_precipRate_var = get_nc_var(f_in, "precipRate");
   NcVar *in_solarRadiation_var = get_nc_var(f_in, "solarRadiation");
   NcVar *in_seaSurfaceTemp_var = get_nc_var(f_in, "seaSurfaceTemp");
   NcVar *in_totalColumnPWV_var = get_nc_var(f_in, "totalColumnPWV");
   NcVar *in_soilTemperature_var = get_nc_var(f_in, "soilTemperature");
   NcVar *in_minTemp24Hour_var = get_nc_var(f_in, "minTemp24Hour");
   NcVar *in_maxTemp24Hour_var = get_nc_var(f_in, "maxTemp24Hour");
   NcVar *in_precip3hr_var = get_nc_var(f_in, "precip3hr");
   NcVar *in_precip6hr_var = get_nc_var(f_in, "precip6hr");
   NcVar *in_precip12hr_var = get_nc_var(f_in, "precip12hr");
   NcVar *in_precip10min_var = get_nc_var(f_in, "precip10min");
   NcVar *in_precip1min_var = get_nc_var(f_in, "precip1min");
   NcVar *in_windDir10_var = get_nc_var(f_in, "windDir10");
   NcVar *in_windSpeed10_var = get_nc_var(f_in, "windSpeed10");
         
   NcVar *in_temperatureQty_var = has_var(f_in, "temperatureDD");
   NcVar *in_dewpointQty_var = has_var(f_in, "dewpointDD");
   NcVar *in_relHumidityQty_var = has_var(f_in, "relHumidityDD");
   NcVar *in_stationPressureQty_var = has_var(f_in, "stationPressureDD");
   NcVar *in_seaLevelPressureQty_var = has_var(f_in, "seaLevelPressureDD");
   NcVar *in_windDirQty_var = has_var(f_in, "windDirDD");
   NcVar *in_windSpeedQty_var = has_var(f_in, "windSpeedDD");
   NcVar *in_windGustQty_var = has_var(f_in, "windGustDD");
   NcVar *in_visibilityQty_var = has_var(f_in, "visibilityDD");
   NcVar *in_precipRateQty_var = has_var(f_in, "precipRateDD");
   NcVar *in_solarRadiationQty_var = has_var(f_in, "solarRadiationDD");
   NcVar *in_seaSurfaceTempQty_var = has_var(f_in, "seaSurfaceTempDD");
   NcVar *in_totalColumnPWVQty_var = has_var(f_in, "totalColumnPWVDD");
   NcVar *in_soilTemperatureQty_var = has_var(f_in, "soilTemperatureDD");
   NcVar *in_minTemp24HourQty_var = has_var(f_in, "minTemp24HourDD");
   NcVar *in_maxTemp24HourQty_var = has_var(f_in, "maxTemp24HourDD");
   NcVar *in_precip3hrQty_var = has_var(f_in, "precip3hrDD");
   NcVar *in_precip6hrQty_var = has_var(f_in, "precip6hrDD");
   NcVar *in_precip12hrQty_var = has_var(f_in, "precip12hrDD");
   NcVar *in_precip10minQty_var = has_var(f_in, "precip10minDD");
   NcVar *in_precip1minQty_var = has_var(f_in, "precip1minDD");
   NcVar *in_windDir10Qty_var = has_var(f_in, "windDir10DD");
   NcVar *in_windSpeed10Qty_var = has_var(f_in, "windSpeed10DD");

   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len = get_nc_dim(f_in, "maxStaIdLen");
   nhdr        = get_nc_dim(f_in, in_recNum_str);
   if(rec_end == 0) rec_end = nhdr;

   //
   // Setup the output NetCDF file
   //
   setup_netcdf_out(nhdr);

   mlog << Debug(2) << "Processing Integrated Mesonet recs\t= " << rec_end - rec_beg << "\n";

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;

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
   //for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {
   for(i_hdr_s=rec_beg; i_hdr_s<rec_end; i_hdr_s+=BUFFER_SIZE) {
      int buf_size = ((rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (rec_end - i_hdr_s);
      float hdr_lat_arr[buf_size];
      float hdr_lon_arr[buf_size];
      float hdr_elv_arr[buf_size];
      double tmp_dbl_arr[buf_size];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      //float pressure_arr[buf_size];
      
      float temperature_arr[buf_size];
      float dewpoint_arr[buf_size];
      float relHumidity_arr[buf_size];
      float stationPressure_arr[buf_size];
      float seaLevelPressure_arr[buf_size];
      float windDir_arr[buf_size];
      float windSpeed_arr[buf_size];
      float windGust_arr[buf_size];
      float visibility_arr[buf_size];
      float precipRate_arr[buf_size];
      float solarRadiation_arr[buf_size];
      float seaSurfaceTemp_arr[buf_size];
      float totalColumnPWV_arr[buf_size];
      float soilTemperature_arr[buf_size];
      float minTemp24Hour_arr[buf_size];
      float maxTemp24Hour_arr[buf_size];
      float precip3hr_arr[buf_size];
      float precip6hr_arr[buf_size];
      float precip12hr_arr[buf_size];
      float precip10min_arr[buf_size];
      float precip1min_arr[buf_size];
      float windDir10_arr[buf_size];
      float windSpeed10_arr[buf_size];

      char temperatureQty_arr[buf_size];
      char dewpointQty_arr[buf_size];
      char relHumidityQty_arr[buf_size];
      char stationPressureQty_arr[buf_size];
      char seaLevelPressureQty_arr[buf_size];
      char windDirQty_arr[buf_size];
      char windSpeedQty_arr[buf_size];
      char windGustQty_arr[buf_size];
      char visibilityQty_arr[buf_size];
      char precipRateQty_arr[buf_size];
      char solarRadiationQty_arr[buf_size];
      char seaSurfaceTempQty_arr[buf_size];
      char totalColumnPWVQty_arr[buf_size];
      char soilTemperatureQty_arr[buf_size];
      char minTemp24HourQty_arr[buf_size];
      char maxTemp24HourQty_arr[buf_size];
      char precip3hrQty_arr[buf_size];
      char precip6hrQty_arr[buf_size];
      char precip12hrQty_arr[buf_size];
      char precip10minQty_arr[buf_size];
      char precip1minQty_arr[buf_size];
      char windDir10Qty_arr[buf_size];
      char windSpeed10Qty_arr[buf_size];
      
      cur[0] = i_hdr_s;
      dim[0] = buf_size;
      get_nc_data(in_hdr_vld_var, cur, dim, tmp_dbl_arr);
      get_nc_data(in_hdr_lat_var, cur, dim, hdr_lat_arr);
      get_nc_data(in_hdr_lon_var, cur, dim, hdr_lon_arr);
      get_filtered_nc_data(in_hdr_elv_var, cur, dim, hdr_elv_arr);
      //get_filtered_nc_data(in_pressure_var, cur, dim, (float *)pressure_arr);
      
      if (in_temperatureQty_var) get_nc_data(in_temperatureQty_var, cur, dim, temperatureQty_arr);
      if (in_dewpointQty_var) get_nc_data(in_dewpointQty_var, cur, dim, dewpointQty_arr);
      if (in_relHumidityQty_var) get_nc_data(in_relHumidityQty_var, cur, dim, relHumidityQty_arr);
      if (in_stationPressureQty_var) get_nc_data(in_stationPressureQty_var, cur, dim, stationPressureQty_arr);
      if (in_seaLevelPressureQty_var) get_nc_data(in_seaLevelPressureQty_var, cur, dim, seaLevelPressureQty_arr);
      if (in_windDirQty_var) get_nc_data(in_windDirQty_var, cur, dim, windDirQty_arr);
      if (in_windSpeedQty_var) get_nc_data(in_windSpeedQty_var, cur, dim, windSpeedQty_arr);
      if (in_windGustQty_var) get_nc_data(in_windGustQty_var, cur, dim, windGustQty_arr);
      if (in_visibilityQty_var) get_nc_data(in_visibilityQty_var, cur, dim, visibilityQty_arr);
      if (in_precipRateQty_var) get_nc_data(in_precipRateQty_var, cur, dim, precipRateQty_arr);
      if (in_solarRadiationQty_var) get_nc_data(in_solarRadiationQty_var, cur, dim, solarRadiationQty_arr);
      if (in_seaSurfaceTempQty_var) get_nc_data(in_seaSurfaceTempQty_var, cur, dim, seaSurfaceTempQty_arr);
      if (in_totalColumnPWVQty_var) get_nc_data(in_totalColumnPWVQty_var, cur, dim, totalColumnPWVQty_arr);
      if (in_soilTemperatureQty_var) get_nc_data(in_soilTemperatureQty_var, cur, dim, soilTemperatureQty_arr);
      if (in_minTemp24HourQty_var) get_nc_data(in_minTemp24HourQty_var, cur, dim, minTemp24HourQty_arr);
      if (in_maxTemp24HourQty_var) get_nc_data(in_maxTemp24HourQty_var, cur, dim, maxTemp24HourQty_arr);
      if (in_precip3hrQty_var) get_nc_data(in_precip3hrQty_var, cur, dim, precip3hrQty_arr);
      if (in_precip6hrQty_var) get_nc_data(in_precip6hrQty_var, cur, dim, precip6hrQty_arr);
      if (in_precip12hrQty_var) get_nc_data(in_precip12hrQty_var, cur, dim, precip12hrQty_arr);
      if (in_precip10minQty_var) get_nc_data(in_precip10minQty_var, cur, dim, precip10minQty_arr);
      if (in_precip1minQty_var) get_nc_data(in_precip1minQty_var, cur, dim, precip1minQty_arr);
      if (in_windDir10Qty_var) get_nc_data(in_windDir10Qty_var, cur, dim, windDir10Qty_arr);
      if (in_windSpeed10Qty_var) get_nc_data(in_windSpeed10Qty_var, cur, dim, windSpeed10Qty_arr);
      
      get_filtered_nc_data(in_temperature_var, cur, dim, temperature_arr);
      get_filtered_nc_data(in_dewpoint_var, cur, dim, dewpoint_arr);
      get_filtered_nc_data(in_relHumidity_var, cur, dim, relHumidity_arr);
      get_filtered_nc_data(in_stationPressure_var, cur, dim, stationPressure_arr);
      get_filtered_nc_data(in_seaLevelPressure_var, cur, dim, seaLevelPressure_arr);
      get_filtered_nc_data(in_windDir_var, cur, dim, windDir_arr);
      get_filtered_nc_data(in_windSpeed_var, cur, dim, windSpeed_arr);
      get_filtered_nc_data(in_windGust_var, cur, dim, windGust_arr);
      get_filtered_nc_data(in_visibility_var, cur, dim, visibility_arr);
      get_filtered_nc_data(in_precipRate_var, cur, dim, precipRate_arr);
      get_filtered_nc_data(in_solarRadiation_var, cur, dim, solarRadiation_arr);
      get_filtered_nc_data(in_seaSurfaceTemp_var, cur, dim, seaSurfaceTemp_arr);
      get_filtered_nc_data(in_totalColumnPWV_var, cur, dim, totalColumnPWV_arr);
      get_filtered_nc_data(in_soilTemperature_var, cur, dim, soilTemperature_arr);
      get_filtered_nc_data(in_minTemp24Hour_var, cur, dim, minTemp24Hour_arr);
      get_filtered_nc_data(in_maxTemp24Hour_var, cur, dim, maxTemp24Hour_arr);
      get_filtered_nc_data(in_precip3hr_var, cur, dim, precip3hr_arr);
      get_filtered_nc_data(in_precip6hr_var, cur, dim, precip6hr_arr);
      get_filtered_nc_data(in_precip12hr_var, cur, dim, precip12hr_arr);
      get_filtered_nc_data(in_precip10min_var, cur, dim, precip10min_arr);
      get_filtered_nc_data(in_precip1min_var, cur, dim, precip1min_arr);
      get_filtered_nc_data(in_windDir10_var, cur, dim, windDir10_arr);
      get_filtered_nc_data(in_windSpeed10_var, cur, dim, windSpeed10_arr);
      
      dim[1] = hdr_sid_len;
      get_nc_data(in_hdr_sid_var, cur, dim, (char *)hdr_sid_arr);
      
      dim[0] = 1;
      dim[1] = 1;

      for (int i_idx=0; i_idx<buf_size; i_idx++) {
         //
         // Mapping of NetCDF variable names from input to output:
         // Output                    = Input
         // hdr_typ                   = ADPSFC (MESONET observations are at the surface)
         // hdr_sid                   = stationId (maxStaIdLen = 6)
         // hdr_vld (YYYYMMDD_HHMMSS) = observationTime (unixtime)
         // hdr_arr[0](Lat)           = latitude
         // hdr_arr[1](Lon)           = longitude
         // hdr_arr[2](Elv)           = elevation
         //

      
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";
         
         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         
         //
         // Process the latitude, longitude, and elevation.
         //
         //get_nc_var_val(in_hdr_lat_var, cur, dim, hdr_arr[0]);
         //get_nc_var_val(in_hdr_lon_var, cur, dim, hdr_arr[1]);
         //get_nc_var_val(in_hdr_elv_var, cur, dim, hdr_arr[2]);
         hdr_arr[0] = hdr_lat_arr[i_idx];
         hdr_arr[1] = hdr_lon_arr[i_idx];
         hdr_arr[2] = hdr_elv_arr[i_idx];
         
         //
         // Check masking regions.
         //
         if(!check_masks(hdr_arr[0], hdr_arr[1])) continue;
         
         //
         // Encode the header type as ADPSFC for MESONET observations.
         //
         put_nc_var_val(hdr_typ_var, i_hdr, "ADPSFC");
         
         //
         // Process the station name.
         //
         //get_nc_var_val(in_hdr_sid_var, cur, hdr_sid_len, hdr_sid);
         hdr_sid = hdr_sid_arr[i_idx];
         put_nc_var_val(hdr_sid_var, i_hdr, hdr_sid);
         
         //
         // Process the observation time.
         //
         //get_nc_var_val(in_hdr_vld_var, cur, dim, tmp_dbl);
         tmp_dbl = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl)) continue;
         unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
         hdr_vld = tmp_str;
         put_nc_var_val(hdr_vld_var, i_hdr, hdr_vld);
         
         //
         // Write the header array to the output file.
         //
         put_nc_var_arr(hdr_arr_var, i_hdr, hdr_arr_len, hdr_arr);
         
         //
         // Initialize the observation array: hdr_id, gc, lvl, hgt, ob
         //
         obs_arr[0] = (float) i_hdr;   // Index into header array
         obs_arr[2] = bad_data_float;  // Level: accum(sec) or pressure
         obs_arr[3] = 0;               // Height for surface is 0 meters
         
         // Temperature
         obs_arr[4] = temperature_arr[i_idx];
         process_obs(11, conversion, obs_arr, temperatureQty_arr[i_idx], in_temperature_var->name());
         
         // Dewpoint
         obs_arr[4] = dewpoint_arr[i_idx];
         process_obs(17, conversion, obs_arr, dewpointQty_arr[i_idx], in_dewpoint_var->name());
         
         // Relative Humidity
         obs_arr[4] = relHumidity_arr[i_idx];
         process_obs(52, conversion, obs_arr, relHumidityQty_arr[i_idx], in_relHumidity_var->name());
         
         // Station Pressure
         obs_arr[4] = stationPressure_arr[i_idx];
         process_obs(1, conversion, obs_arr, stationPressureQty_arr[i_idx], in_stationPressure_var->name());
         
         // Sea Level Pressure
         obs_arr[4] = seaLevelPressure_arr[i_idx];
         process_obs(2, conversion, obs_arr, seaLevelPressureQty_arr[i_idx], in_seaLevelPressure_var->name());
         
         // Wind Direction
         obs_arr[4] = windDir_arr[i_idx];
         process_obs(31, conversion, obs_arr, windDirQty_arr[i_idx], in_windDir_var->name());
         wdir = obs_arr[4];
         
         // Wind Speed
         obs_arr[4] = windSpeed_arr[i_idx];
         char qty = windSpeedQty_arr[i_idx];
         process_obs(32, conversion, obs_arr, qty, in_windSpeed_var->name());
         wind = obs_arr[4];
         
         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
         // Write U-component of wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            write_qty(qty);
            i_obs++;
         }
         
         // Write V-component of wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            write_qty(qty);
            i_obs++;
         }
         
         // Wind Gust
         obs_arr[4] = windGust_arr[i_idx];
         process_obs(180, conversion, obs_arr, windGustQty_arr[i_idx], in_windGust_var->name());
         
         // Visibility
         obs_arr[4] = visibility_arr[i_idx];
         process_obs(20, conversion, obs_arr, visibilityQty_arr[i_idx], in_visibility_var->name());
         
         // Precipitation Rate
         // Convert input meters/second to output millimeters/second
         obs_arr[4] = precipRate_arr[i_idx];
         process_obs(59, 1000.0, obs_arr, precipRateQty_arr[i_idx], in_precipRate_var->name());
         
         // Solar Radiation
         obs_arr[4] = solarRadiation_arr[i_idx];
         process_obs(250, conversion, obs_arr, solarRadiationQty_arr[i_idx], in_solarRadiation_var->name());
         
         // Sea Surface Temperature
         obs_arr[4] = seaSurfaceTemp_arr[i_idx];
         process_obs(80, conversion, obs_arr, seaSurfaceTempQty_arr[i_idx], in_seaSurfaceTemp_var->name());
         
         // Precipitable Water
         // Convert input cm to output mm
         obs_arr[4] = totalColumnPWV_arr[i_idx];
         process_obs(54, 10.0, obs_arr, totalColumnPWVQty_arr[i_idx], in_totalColumnPWV_var->name());
         
         // Soil Temperature
         obs_arr[4] = soilTemperature_arr[i_idx];
         process_obs(85, conversion, obs_arr, soilTemperatureQty_arr[i_idx], in_soilTemperature_var->name());
         
         // Minimum Temperature
         obs_arr[4] = minTemp24Hour_arr[i_idx];
         process_obs(16, conversion, obs_arr, minTemp24HourQty_arr[i_idx], in_minTemp24Hour_var->name());
         
         // Maximum Temperature
         obs_arr[4] = maxTemp24Hour_arr[i_idx];
         process_obs(15, conversion, obs_arr, maxTemp24HourQty_arr[i_idx], in_maxTemp24Hour_var->name());
         
         // Precipitation - 3 Hour
         obs_arr[2] = 3.0*sec_per_hour;
         obs_arr[4] = precip3hr_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip3hrQty_arr[i_idx], in_precip3hr_var->name());
         
         // Precipitation - 6 Hour
         obs_arr[2] = 6.0*sec_per_hour;
         obs_arr[4] = precip6hr_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip6hrQty_arr[i_idx], in_precip6hr_var->name());
         
         // Precipitation - 12 Hour
         obs_arr[2] = 12.0*sec_per_hour;
         obs_arr[4] = precip12hr_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip12hrQty_arr[i_idx], in_precip12hr_var->name());
         
         // Precipitation - 10 minutes
         obs_arr[2] = 600;
         obs_arr[4] = precip10min_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip10minQty_arr[i_idx], in_precip10min_var->name());
         
         // Precipitation - 1 minutes
         obs_arr[2] = 60;
         obs_arr[4] = precip1min_arr[i_idx];
         process_obs(61, conversion, obs_arr, precip1minQty_arr[i_idx], in_precip1min_var->name());
         
         // Set the level to bad data and the height to 10 meters
         obs_arr[2] = bad_data_float;
         obs_arr[3] = 10;
         
         // 10m Wind Direction
         obs_arr[4] = windDir10_arr[i_idx];
         process_obs(31, conversion, obs_arr, windDir10Qty_arr[i_idx], in_windDir10_var->name());
         wdir = obs_arr[4];
         
         // 10m Wind Speed
         qty = windSpeed10Qty_arr[i_idx];
         obs_arr[4] = windSpeed10_arr[i_idx];
         process_obs(32, conversion, obs_arr, qty, in_windSpeed10_var->name());
         wind = obs_arr[4];
         
         // Convert the wind direction and speed into U and V components
         convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
         // Write U-component of 10m wind
         obs_arr[1] = 33;
         obs_arr[4] = ugrd;
         if(!is_bad_data(ugrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            write_qty(qty);
            i_obs++;
         }
         
         // Write V-component of 10m wind
         obs_arr[1] = 34;
         obs_arr[4] = vgrd;
         if(!is_bad_data(vgrd)) {
            put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
            write_qty(qty);
            i_obs++;
         }
      }

   } // end for i

   print_rej_counts();

   //
   // Cleanup
   //
   if(cur) { delete [] cur; cur = (long *) 0; }
   if(dim) { delete [] dim; dim = (long *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_madis_acarsProfiles(NcFile *&f_in) {
   int nhdr, nlvl,nlvl1, i_lvl, maxLevels;
   long i_hdr, i_cnt, i_hdr_s;
   int hdr_sid_len;
   int buf_size;
   double tmp_dbl, tmp_dbl2, tmp_dbl1;
   char tmp_str[max_str_len], qty;
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   float hdr_arr[hdr_arr_len], obs_arr[obs_arr_len], conversion;
   float pressure, v, wdir, wind, ugrd, vgrd;

   //
   // Input header variables:
   // Note: hdr_typ is always set to AIRCFT
   //
   NcVar *in_hdr_sid_var = get_nc_var(f_in, "profileAirport");
   NcVar *in_hdr_vld_var = get_nc_var(f_in, "profileTime");
   NcVar *in_hdr_lat_var = get_nc_var(f_in, "trackLat");
   NcVar *in_hdr_lon_var = get_nc_var(f_in, "trackLon");
   NcVar *in_hdr_elv_var = get_nc_var(f_in, "altitude");
   NcVar *in_hdr_tob_var = get_nc_var(f_in, "obsTimeOfDay");
   
   NcVar *in_temperature_var = get_nc_var(f_in, "temperature");
   NcVar *in_dewpoint_var = get_nc_var(f_in, "dewpoint");
   NcVar *in_windDir_var = get_nc_var(f_in, "windDir");
   NcVar *in_windSpeed_var = get_nc_var(f_in, "windSpeed");
   
   NcVar *in_temperatureQty_var = has_var(f_in, "temperatureDD");
   NcVar *in_dewpointQty_var = has_var(f_in, "dewpointDD");
   NcVar *in_windDirQty_var = has_var(f_in, "windDirDD");
   NcVar *in_windSpeedQty_var = has_var(f_in, "windSpeedDD");
   NcVar *in_nLevelsQty_var = has_var(f_in, "nLevelsDD");
   NcVar *in_altitudeQty_var = has_var(f_in, "altitudeDD");
   
   //
   // Retrieve applicable dimensions
   //
   hdr_sid_len  = get_nc_dim(f_in, "AirportIdLen");
   nhdr         = get_nc_dim(f_in, in_recNum_str);
   maxLevels    = get_nc_dim(f_in, "maxLevels");

   if(rec_end == 0) rec_end = nhdr;

   //
   // Initialize variables for processing observations
   //
   conversion = 1.0;
   nlvl1 = 0;

   //
   // Arrays of longs for indexing into NetCDF variables
   //
   long *cur = new long [3];
   cur[0] = cur[1] = cur[2] = 0;
   long *dim = new long [3];
   dim[0] = dim[1] = dim[2] = 1;

   //
   // Get the number of levels
   //
   NcVar *in_var = get_nc_var(f_in, "nLevels");

   //
   // Obtain the total number of levels
   //
   buf_size = rec_end - rec_beg;
   int levels[buf_size];
   char levelsQty[buf_size];
   cur[0] = rec_beg;
   dim[0] = buf_size;
   get_nc_data(in_var, cur, dim, levels);
   if (in_nLevelsQty_var) get_nc_data(in_nLevelsQty_var, cur, dim, (char *)&levelsQty);
   for(i_hdr=0; i_hdr<buf_size; i_hdr++) {
      nlvl1 += levels[i_hdr];
   }
   cur[0] = 0;
   dim[0] = 1;

   //
   // Setup the output NetCDF file
   //
   setup_netcdf_out(nlvl1);

   mlog << Debug(2) << "Processing ACARS Profiles recs\t\t= " << nlvl1 << "\n";

   //
   // Initialize
   //
   i_cnt = -1;

   //
   // Loop through each record and get the header data.
   //
   //for(i_hdr=rec_beg; i_hdr<rec_end; i_hdr++) {
   for(i_hdr_s=rec_beg; i_hdr_s<rec_end; i_hdr_s+=BUFFER_SIZE) {
      int buf_size = ((rec_end - i_hdr_s) > BUFFER_SIZE) ? BUFFER_SIZE: (rec_end - i_hdr_s);
      
      double tmp_dbl_arr[buf_size];
      float hdr_lat_arr[buf_size][maxLevels];
      float hdr_lon_arr[buf_size][maxLevels];
      float hdr_elv_arr[buf_size][maxLevels];
      char hdr_sid_arr[buf_size][hdr_sid_len];

      //float pressure_arr[buf_size];

      int obsTimeOfDay_arr[buf_size][maxLevels];
      float temperature_arr[buf_size][maxLevels];
      float dewpoint_arr[buf_size][maxLevels];
      float windDir_arr[buf_size][maxLevels];
      float windSpeed_arr[buf_size][maxLevels];

      char temperatureQty_arr[buf_size][maxLevels];
      char dewpointQty_arr[buf_size][maxLevels];
      char windDirQty_arr[buf_size][maxLevels];
      char windSpeedQty_arr[buf_size][maxLevels];
      char altitudeQty_arr[buf_size][maxLevels];
      
      cur[0] = i_hdr_s;
      dim[0] = buf_size;
      dim[1] = maxLevels;
      get_nc_data(in_hdr_vld_var, cur, dim, tmp_dbl_arr);
      get_nc_data(in_hdr_lat_var, cur, dim, (float *)hdr_lat_arr);
      get_nc_data(in_hdr_lon_var, cur, dim, (float *)hdr_lon_arr);
      get_filtered_nc_data(in_hdr_elv_var, cur, dim, (float *)hdr_elv_arr);
      //get_filtered_nc_data(in_pressure_var, cur, dim, (float *)pressure_arr);
      
      if (in_temperatureQty_var) get_nc_data(in_temperatureQty_var, cur, dim, (char *)&temperatureQty_arr);
      if (in_dewpointQty_var) get_nc_data(in_dewpointQty_var, cur, dim, (char *)&dewpointQty_arr);
      if (in_windDirQty_var) get_nc_data(in_windDirQty_var, cur, dim, (char *)&windDirQty_arr);
      if (in_windSpeedQty_var) get_nc_data(in_windSpeedQty_var, cur, dim, (char *)&windSpeedQty_arr);
      if (in_altitudeQty_var) get_nc_data(in_altitudeQty_var, cur, dim, (char *)&altitudeQty_arr);
      
      get_filtered_nc_data_2d(in_hdr_tob_var, cur, dim, (int *)&obsTimeOfDay_arr);
      get_filtered_nc_data_2d(in_temperature_var, cur, dim, (float *)&temperature_arr);
      get_filtered_nc_data_2d(in_dewpoint_var, cur, dim, (float *)&dewpoint_arr);
      get_filtered_nc_data_2d(in_windDir_var, cur, dim, (float *)&windDir_arr);
      get_filtered_nc_data_2d(in_windSpeed_var, cur, dim, (float *)&windSpeed_arr);
      
      dim[1] = hdr_sid_len;
      get_nc_data(in_hdr_sid_var, cur, dim, (char *)hdr_sid_arr);
      
      dim[0] = 1;
      dim[1] = 1;

      // Process the header type.
      // For ACARS, store as AIRCFT.
      //
      hdr_typ = "AIRCFT";
      
      
      for (int i_idx=0; i_idx<buf_size; i_idx++) {
         i_hdr = i_hdr_s + i_idx;
         mlog << Debug(3) << "Record Number: " << i_hdr << "\n";
         
         //
         // Use cur to index into the NetCDF variables.
         //
         cur[0] = i_hdr;
         cur[1] = 0;
         
         //
         // Process the station i.e. airport name.
         //
         //get_nc_var_val(in_hdr_sid_var, cur, hdr_sid_len, hdr_sid);
         hdr_sid = hdr_sid_arr[i_idx];
         
         //
         // Process the observation time.
         //
         //get_nc_var_val(in_hdr_vld_var, cur, dim, tmp_dbl1);
         tmp_dbl1 = tmp_dbl_arr[i_idx];
         if(is_bad_data(tmp_dbl1)) continue;
         
         //
         // Process the number of levels
         //
         //get_nc_var_val(in_var, cur, dim, nlvl);
         check_quality_control_flag(levels[i_idx], levelsQty[i_idx], in_var->name());
         nlvl = levels[i_idx];
         obs_arr[2] = levels[i_idx];
         
         //
         // Loop through each level of each track
         //
         for(i_lvl=0; i_lvl<nlvl; i_lvl++) {
         
            i_cnt++;
            mlog << Debug(3) << "  Mandatory Level: " << i_lvl << "\n";
         
            //
            // Write header type
            //
            put_nc_var_val(hdr_typ_var, i_cnt, hdr_typ);
         
            //
            // Write Airport ID
            //
            put_nc_var_val(hdr_sid_var, i_cnt, hdr_sid);
         
            //
            // Use cur to index into the NetCDF variables.
            //
            cur[1] = i_lvl;
            obs_arr[0] = (float) i_cnt;
         
            //
            // Process the latitude, longitude, and elevation.
            //
            //get_nc_var_val(in_hdr_lat_var, cur, dim, hdr_arr[0]);
            //get_nc_var_val(in_hdr_lon_var, cur, dim, hdr_arr[1]);
            //get_nc_var_val(in_hdr_elv_var, cur, dim, hdr_arr[2]);
            check_quality_control_flag(hdr_elv_arr[i_idx][i_lvl], altitudeQty_arr[i_idx][i_lvl], in_hdr_elv_var->name());
            hdr_arr[0] = hdr_lat_arr[i_idx][i_lvl];
            hdr_arr[1] = hdr_lon_arr[i_idx][i_lvl];
            hdr_arr[2] = hdr_elv_arr[i_idx][i_lvl];
         
            //
            // Check masked regions
            //
            if(!check_masks(hdr_arr[0], hdr_arr[1])) continue;
         
            //
            // Get the number of levels  and height for this level
            //
            //obs_arr[3] = get_nc_obs(f_in, "altitude", cur, dim);
            //obs_arr[2] = get_nc_obs(f_in, "nLevels", cur, dim);
            obs_arr[3] = hdr_elv_arr[i_idx][i_lvl];
            
         
            //
            // Check for bad data
            //
            if(is_bad_data(obs_arr[2]) && is_bad_data(obs_arr[3])) continue;
         
            //
            // Process the observation time.
            //
            //get_nc_var_val(in_hdr_tob_var, cur, dim, tmp_dbl2);
            tmp_dbl2 = obsTimeOfDay_arr[i_idx][i_lvl];
         
            //
            // Add to Profile Time
            // Observation Time is relative to time of day
            //
            tmp_dbl = tmp_dbl1+tmp_dbl2-fmod(tmp_dbl1, 86400);
            unix_to_yyyymmdd_hhmmss((unixtime) tmp_dbl, tmp_str);
            hdr_vld = tmp_str;
         
            //
            // Write observation time
            //
            put_nc_var_val(hdr_vld_var, i_cnt, hdr_vld);
         
            //
            // Write header array
            //
            put_nc_var_arr(hdr_arr_var, i_cnt, hdr_arr_len, hdr_arr);
         
            //
            // Compute the pressure (hPa) from altitude data
            // Equation obtained from http://www.srh.noaa.gov/
            //
            pressure = 1013.25*pow((1-2.25577e-5*obs_arr[3]),5.25588);
         
            //
            // Replace number of Levels to Pressure values in Observation Array
            //
            obs_arr[2] = pressure;
         
            // Temperature
            obs_arr[4] = temperature_arr[i_idx][i_lvl];
            process_obs(11, conversion, obs_arr, temperatureQty_arr[i_idx][i_lvl], in_temperature_var->name());
         
            // Dewpoint
            obs_arr[4] = dewpoint_arr[i_idx][i_lvl];
            process_obs(17, conversion, obs_arr, dewpointQty_arr[i_idx][i_lvl], in_dewpoint_var->name());
         
            // Wind Direction
            obs_arr[4] = windDir_arr[i_idx][i_lvl];
            process_obs(31, conversion, obs_arr, windDirQty_arr[i_idx][i_lvl], in_windDir_var->name());
            wdir = obs_arr[4];
         
            // Wind Speed
            obs_arr[4] = windSpeed_arr[i_idx][i_lvl];
            qty = windSpeedQty_arr[i_idx][i_lvl];
            process_obs(32, conversion, obs_arr, qty, in_windSpeed_var->name());
            wind = obs_arr[4];
         
            // Convert the wind direction and speed into U and V components
            convert_wind_wdir_to_u_v(wind, wdir, ugrd, vgrd);
         
            // Write U-component of wind
            obs_arr[1] = 33;
            obs_arr[4] = ugrd;
            if(!is_bad_data(ugrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         
            // Write V-component of wind
            obs_arr[1] = 34;
            obs_arr[4] = vgrd;
            if(!is_bad_data(vgrd)) {
               put_nc_var_arr(obs_arr_var, i_obs, obs_arr_len, obs_arr);
               write_qty(qty);
               i_obs++;
            }
         } // end for i_lvl
      }
   } // end for i_hdr

   print_rej_counts();

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
        << "\t-type str\n"
        << "\t[-qc_dd list]\n"
        << "\t[-lvl_dim list]\n"
        << "\t[-rec_beg n]\n"
        << "\t[-rec_end n]\n"
        << "\t[-mask_grid string]\n"
        << "\t[-mask_poly file]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"madis_file\" is the MADIS NetCDF point "
        << "observation file (required).\n"

        << "\t\t\"out_file\" is the output NetCDF file (required).\n"

        << "\t\t\"-type str\" specifies the type of MADIS observations "
        << "(metar, raob, profiler, maritime, mesonet, or acarsProfiles) "
        << "(required).\n"

        << "\t\t\"-qc_dd list\" specifies a comma-separated list of "
        << "QC flag values to be accepted (Z,C,S,V,X,Q,K,G,B) "
        << "(optional).\n"

        << "\t\t\"-lvl_dim list\" specifies a comma-separated list of "
        << "vertical level dimensions to be processed. (optional).\n"

        << "\t\t\"-rec_beg n\" specifies the index of the first "
        << "MADIS record to process, zero-based (optional).\n"

        << "\t\t\"-rec_end n\" specifies the index of the last "
        << "MADIS record to process, zero-based (optional).\n"

        << "\t\t\"-mask_grid string\" is a named grid or a gridded data "
        << "file for filtering the point observations spatially (optional).\n"

        << "\t\t\"-mask_poly file\" is a polyline masking file for filtering "
        << "the point observations spatially (optional).\n"

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
   else if(strcasecmp(a[0], profiler_str) == 0) {
      mtype = madis_profiler;
   }
   else if(strcasecmp(a[0], maritime_str) == 0) {
      mtype = madis_maritime;
   }
   else if(strcasecmp(a[0], mesonet_str) == 0) {
      mtype = madis_mesonet;
   }
   else if(strcasecmp(a[0], acarsProfiles_str) == 0) {
      mtype = madis_acarsProfiles;
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

void set_mask_grid(const StringArray & a) {
  Met2dDataFileFactory factory;
  Met2dDataFile * datafile = (Met2dDataFile *) 0;

  // List the grid masking file
  mlog << Debug(1)
       << "Grid Masking: " << a[0] << "\n";

  // First, try to find the grid by name.
  if(!find_grid_by_name(a[0], grid_mask)) {

    // If that doesn't work, try to open a data file.
    datafile = factory.new_met_2d_data_file(replace_path(a[0]));

    if(!datafile) {
      mlog << Error << "\nset_mask_grid() -> "
           << "can't open data file \"" << a[0] << "\"\n\n";
      exit(1);
    }

    // Store the data file's grid
    grid_mask = datafile->grid();

    delete datafile; datafile = (Met2dDataFile *) 0;
  }

  // List the grid mask
  mlog << Debug(2)
       << "Parsed Masking Grid: " << grid_mask.name() << " ("
       << grid_mask.nx() << " x " << grid_mask.ny() << ")\n";
}

////////////////////////////////////////////////////////////////////////

void set_mask_poly(const StringArray & a) {

  // List the polyline masking file
  mlog << Debug(1)
       << "Polyline Masking File: " << a[0] << "\n";

  // Parse the polyline file.
  poly_mask.load(replace_path(a[0]));

  // List the polyline mask
  mlog << Debug(2)
       << "Parsed Masking Polyline: " << poly_mask.name()
       << " containing " <<  poly_mask.n_points() << " points\n";
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
