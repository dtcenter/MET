// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
//
////////////////////////////////////////////////////////////////////////
//
//   Filename:   lidar2nc.cc
//
//   Description:
//      Parse HDF Lidar files and reformat them into the NetCDF point
//      observation format used by the other MET tools.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    03-22-17  Bullock        New
//
////////////////////////////////////////////////////////////////////////


static const int  n_obs_types                = 5;  //  layer base, layer top, opacity, cad score, feature classification

static const char hdr_typ_string          [] = "calipso";


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
#include <vector>

#include <netcdf>

#include "hdf.h"
#include "mfhdf.h"

#include "data2d_factory.h"
#include "mask_poly.h"
#include "vx_grid.h"
#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "calipso_5km.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static const int na_len = strlen(na_str);

static NetcdfObsVars obsVars;

////////////////////////////////////////////////////////////////////////


   //
   //   Command-line switches
   //

static ConcatString output_filename;

static int compress_level = -1;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_logfile   (const StringArray &);
static void set_outfile   (const StringArray &);
static void set_verbosity (const StringArray &);
static void set_compress  (const StringArray &);

static void process_calipso_file (NcFile *, const char * filename);

static void write_nc_record(NcFile * out, NcVar & obs_qty_var, NcVar & obs_arr_var, const float * f, int qc_value = -1);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

   //
   // Set handler to be called for memory allocation error
   //

set_new_handler(oom);

   //
   //  parse command line
   //

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outfile,   "-out",       1);
cline.add(set_logfile,   "-log",       1);
cline.add(set_verbosity, "-v",         1);
cline.add(set_compress,  "-compress",  1);

cline.parse();


if ( cline.n() != 1 )  usage();

if ( output_filename.empty() )  usage();

   //
   //  open the output file
   //

static NcFile *ncf = open_ncfile(output_filename.text(), true);

   //
   //  process the lidar file
   //

mlog << Debug(1) << "Processing Lidar File: " << cline[0] << "\n";

process_calipso_file(ncf, cline[0]);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////

void usage() {

cout << "\nUsage: "
     << program_name << "\n"
     << "\tlidar_file\n"
     << "\t-out out_file\n"
     << "\t[-log file]\n"
     << "\t[-v level]\n"
     << "\t[-compress level]\n\n"

     << "\twhere\t\"lidar_file\" is the HDF lidar point observation "
     << "file (required).\n"

     << "\t\t\"-out out_file\" is the output NetCDF file (required).\n"

     << "\t\t\"-log file\" outputs log messages to the specified "
     << "file (optional).\n"

     << "\t\t\"-v level\" overrides the default level of logging ("
     << mlog.verbosity_level() << ") (optional).\n"

     << "\t\t\"-compress level\" specifies the compression level of "
     << "the output NetCDF variable (optional).\n\n"

     << flush;

exit (1);

}


////////////////////////////////////////////////////////////////////////


void set_logfile(const StringArray & a)

{

mlog.open_log_file(a[0]);

return;

}


////////////////////////////////////////////////////////////////////////


void set_outfile(const StringArray & a)

{

output_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_verbosity(const StringArray & a)

{

mlog.set_verbosity_level(atoi(a[0]));

return;

}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a)

{

compress_level = atoi(a[0]);

return;

}


////////////////////////////////////////////////////////////////////////


NcType::ncType hdf_type_to_nc_type(int hdf_type)

{

NcType::ncType t;

switch ( hdf_type )  {

   case DFNT_CHAR8:    t =  NcType::nc_CHAR;   break;   // 8-bit character type
   case DFNT_UCHAR8:   t =  NcType::nc_BYTE;   break;   // 8-bit unsigned character type

   // case DFNT_INT8:     t =  NcType::nc_SHORT;  break;   // 8-bit integer type
   // case DFNT_UINT8:    t =  NcType::nc_SHORT;  break;   // 8-bit unsigned integer type

   case DFNT_INT16:    t = NcType::nc_SHORT;   break;   // 16-bit integer type
   case DFNT_UINT16:   t = NcType::nc_USHORT;  break;   // 16-bit unsigned integer type

   case DFNT_INT32:    t = NcType::nc_INT;     break;   // 32-bit integer type
   case DFNT_UINT32:   t = NcType::nc_UINT;    break;   // 32-bit unsigned integer type

   case DFNT_FLOAT32:  t = NcType::nc_FLOAT;   break;   // 32-bit floating-point type

   case DFNT_FLOAT64:  t = NcType::nc_DOUBLE;  break;   // 64-bit floating-point type

   default:
      mlog << Error
           << program_name << ": hdf_type_to_nc_type() -> unrecognized hdf data type ... "
           << hdf_type << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( t );

}


////////////////////////////////////////////////////////////////////////


void process_calipso_file(NcFile * out, const char * filename)

{

int j;
int hdf_sd_id;
int n_data;
int hdf_stride[MAX_VAR_DIMS], hdf_edge[MAX_VAR_DIMS];
NcDim nc_dim;
NcVar nc_lat_var, nc_lon_var, nc_data_var, nc_time_var;


for (j=0; j<MAX_VAR_DIMS; ++j)  hdf_stride[j] = hdf_edge[j] = 0;

   //
   //  open hdf file
   //

hdf_sd_id = SDstart(filename, DFACC_READ);

if ( hdf_sd_id < 0 )  {

   mlog << Error
        << "\n\n  " << program_name << ": failed to open calipso file \"" << filename << "\"\n\n";

   exit ( 1 );

}


   //
   //  get number of data points
   //
   //    we'll assume this is the same as the number of points in the latitude array
   //

Calipso_5km_Vars hdf_5km;

hdf_5km.get_vars(hdf_sd_id);

n_data = max<int> (hdf_5km.lat.dimsizes[0], hdf_5km.lat.dimsizes[1]);

const int nhdr_dim_size = n_data;

   //
   //  see how big a buffer we'll need
   //

const int hdr_typ_bytes = nhdr_dim_size*HEADER_STR_LEN_L;
const int hdr_vld_bytes = nhdr_dim_size*HEADER_STR_LEN_L;
const int hdr_arr_bytes = nhdr_dim_size*HDR_ARRAY_LEN*sizeof(float);

int buf_size      = hdr_typ_bytes;

buf_size = max<int>(buf_size, hdr_vld_bytes);
buf_size = max<int>(buf_size, hdr_arr_bytes);


mlog << Debug(1) << "Writing MET File:\t" << output_filename << "\n";


   //
   //  add some dimensions to the netcdf file
   //

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;

   //
   //  add the variables to the netcdf file
   //

   bool use_var_id = false;
   init_nc_dims_vars (obsVars, use_var_id);
   obsVars.attr_agl = true;
   create_nc_hdr_vars(obsVars, out, n_data, deflate_level);
   create_nc_obs_vars(obsVars, out, deflate_level, use_var_id);

   int strl_len = get_dim_size(&obsVars.strl_dim);
   int typ_len = strl_len;
   int sid_len = strl_len;
   int vld_len = strl_len;
   if (!IS_INVALID_NC(obsVars.strll_dim)) {
      NcDim str_dim;
      string dim_name = GET_NC_NAME(obsVars.strll_dim);
      int strll_len = get_dim_size(&obsVars.strll_dim);
      str_dim = get_nc_dim(&obsVars.hdr_typ_var, dim_name);
      if (!IS_INVALID_NC(str_dim)) typ_len = strll_len;
      str_dim = get_nc_dim(&obsVars.hdr_sid_var, dim_name);
      if (!IS_INVALID_NC(str_dim)) sid_len = strll_len;
      str_dim = get_nc_dim(&obsVars.hdr_vld_var, dim_name);
      if (!IS_INVALID_NC(str_dim)) vld_len = strll_len;
   }
   
   //
   //  global attributes for netcdf output file
   //

const unixtime now = time(0);
int month, day, year, hour, minute, second;
ConcatString s;
char junk [1 + HEADER_STR_LEN];

unix_to_mdyhms(now, month, day, year, hour, minute, second);

snprintf(junk, sizeof(junk),
         "%04d%02d%02d_%02d%02d%02d",
         year, month, day, hour, minute, second);


s << "File " << output_filename << " generated " << junk << " UTC";

memset(junk, 0, sizeof(junk));

if ( gethostname(junk, sizeof(junk) - 1) < 0 )  s << " on unknown host";
else                                            s << " on host " << junk;

(void) out->putAtt(string("FileOrigins"), string( s.text() ));
(void) out->putAtt(string("MET_version"), string( met_version ));
(void) out->putAtt(string("MET_tool"),    string(program_name.text()));

   //
   //  allocate the buffer
   //

unsigned char * buf   = 0;

buf   = new unsigned char [buf_size];

char  * const cbuf = (char *)  buf;
float * const fbuf = (float *) buf;

mlog << Debug(2) << "Processing Lidar points\t= " << n_data << "\n";


   //
   //  populate the hdr_typ variable
   //

memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*typ_len, hdr_typ_string);

}

obsVars.hdr_typ_var.putVar(cbuf);

   //
   //  populate the hdr_sid variable
   //

memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*sid_len, na_str);

}

obsVars.hdr_sid_var.putVar(cbuf);

   //
   //  populate the obs_qty variable
   //
/*
memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*HEADER_STR_LEN, na_str);

}

obs_qty_var.putVar(cbuf);
*/
   //
   //  populate the hdr_arr variable
   //

float ff[2];
int hdr_offset = 0;

memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   hdf_5km.get_latlon(j, ff[0], ff[1]);

   //hdr_offset = HDR_ARRAY_LEN*j;
   
   fbuf[hdr_offset]     = ff[0];   //  latitude

   fbuf[hdr_offset + 1] = ff[1];   //  longitude

   fbuf[hdr_offset + 2] = FILL_VALUE;
   
   hdr_offset += HDR_ARRAY_LEN;

}   //  for j

obsVars.hdr_arr_var.putVar(fbuf);

   //
   //  populate the hdr_vld variable
   //
   //    the time in the hdf file is seconds from Jan 1, 1993 0h
   //

HdfVarInfo info;
int layer;
unixtime t;



get_hdf_var_info(hdf_sd_id, hdf_time_name, info);

hdf_stride[0] = 1;
hdf_stride[1] = 1;

hdf_edge[0] = 1;
hdf_edge[1] = 1;

memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   t = hdf_5km.get_time(j);

   unix_to_mdyhms(t, month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk),
            "%04d%02d%02d_%02d%02d%02d",
             year, month, day, hour, minute, second);

   strcpy(cbuf + j*vld_len, junk);

}   //  for j

obsVars.hdr_vld_var.putVar(cbuf);

   //
   //  populate the obs_arr variable
   //

Calipso_5km_Obs obs;

memset(buf, 0, buf_size);

float f[5];
int qc_value;

NcVar obs_arr_var = obsVars.obs_arr_var;
NcVar obs_qty_var = obsVars.obs_qty_var;

for (j=0; j<n_data; ++j)  {

   hdf_5km.get_obs(j, obs);

   //
   //  write the number of layers
   //

   obs.get_n_layers_record              (j, f);
      write_nc_record                   (out, obs_qty_var, obs_arr_var, f);

   if ( obs.n_layers == 0 )  continue;

   for (layer=0; layer<(obs.n_layers); ++layer)  {

      obs.get_layer_base_record         (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_layer_top_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_opacity_record            (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_cad_score_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_ice_water_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_subtype_record            (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_cloud_aerosol_qa_record   (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_h_average_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

           /////////////////////////////////


                 obs.get_feature_type_record   (j, layer, f);
      qc_value = obs.get_feature_type_qa_value (layer);
         write_nc_record                       (out, obs_qty_var, obs_arr_var, f, qc_value);

                 obs.get_ice_water_record      (j, layer, f);
      qc_value = obs.get_ice_water_qa_value    (layer);
         write_nc_record                       (out, obs_qty_var, obs_arr_var, f, qc_value);

   }   //  for layer

   obs.get_base_base_record  (j, f);
      write_nc_record        (out, obs_qty_var, obs_arr_var, f);

   obs.get_top_top_record    (j, f);
      write_nc_record        (out, obs_qty_var, obs_arr_var, f);

}   //  for j


   //
   //  close hdf file
   //

if ( SDend(hdf_sd_id) < 0 )  {

   mlog << Error
        << "\n\n  " << program_name << ": failed to close file\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////


void write_nc_record(NcFile * out, NcVar & obs_qty_var, NcVar & obs_arr_var, const float * f, int qc_value)

{

static int pos = 0;
vector<size_t> index;
vector<size_t> count;

index.resize(2);
count.resize(2);

   //
   //  write obs_arr record
   //

index.at(0) = pos;
index.at(1) = 0;

count.at(0) = 1;
count.at(1) = n_obs_types;

obs_arr_var.putVar(index, count, f);


   //
   //  write obs_qty record
   //

char junk[HEADER_STR_LEN];


index.at(0) = pos;
index.at(1) = 0;

count.at(0) = 1;

if ( qc_value < 0 )  {

   count.at(1) = na_len;

   obs_qty_var.putVar(index, count, na_str);

} else {

   snprintf(junk, sizeof(junk), "%d", qc_value);

   count.at(1) = strlen(junk);

   obs_qty_var.putVar(index, count, junk);
}

   //
   //  done
   //

++pos;

return;

}


////////////////////////////////////////////////////////////////////////



