// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


static const char na_string               [] = "NA";

static const char mxstr_dim_name          [] = "mxstr";
static const int  mxstr_dim_size             = 40;

static const char hdr_arr_len_dim_name    [] = "hdr_arr_len";
static const int  hdr_arr_len_dim_size       = 3;

static const char obs_arr_len_dim_name    [] = "obs_arr_len";

static const int  n_obs_types                = 5;  //  layer base, layer top, opacity, cad score, feature classification

static const char nhdr_dim_name           [] = "nhdr";
static const char nobs_dim_name           [] = "nobs";

static const char hdr_typ_string          [] = "calipso";

static const char hdr_typ_var_name        [] = "hdr_typ";
static const char hdr_sid_var_name        [] = "hdr_sid";
static const char hdr_vld_var_name        [] = "hdr_vld";
static const char hdr_arr_var_name        [] = "hdr_arr";
static const char obs_qty_var_name        [] = "obs_qty";
static const char obs_arr_var_name        [] = "obs_arr";


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

//#include "netcdf.hh"
#include <netcdf>
// #include <netcdfcpp.h>
// using namespace netCDF;

#include "hdf.h"
#include "mfhdf.h"

#include "data2d_factory.h"
#include "mask_poly.h"
#include "vx_grid.h"
#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"

#include "calipso_5km.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static const int na_len = strlen(na_string);


////////////////////////////////////////////////////////////////////////


   //
   //   Command-line switches
   //

static ConcatString output_filename;

static int compress_level = -1;

static int grib_code = 500;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_logfile   (const StringArray &);
static void set_outfile   (const StringArray &);
static void set_verbosity (const StringArray &);
static void set_grib_code (const StringArray &);

static void process_calipso_file (NcFile &, const char * filename);

static void write_nc_record(NcFile & out, NcVar & obs_qty_var, NcVar & obs_arr_var, const float * f);


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

cline.add(set_logfile,   "-log",       1);
cline.add(set_outfile,   "-out",       1);
cline.add(set_verbosity, "-v",         1);
cline.add(set_grib_code, "-grib_code", 1);

cline.parse();


if ( cline.n() != 1 )  usage();

if ( output_filename.empty() )  usage();

   //
   //  open the output file
   //

static NcFile ncf(output_filename.text(), NcFile::replace, NcFile::nc4);

   //
   //  process the lidar file
   //

mlog << "Processing \"" << cline[0] << "\"\n";

process_calipso_file(ncf, cline[0]);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

ConcatString tab;

tab.set_repeat(' ', 5 + program_name.length());

cout << "\n\n"

     << "   usage: " << program_name << "\n\n"

     << tab << "-out nc_filename\n\n"

     << tab << "[ -log filename ]\n\n"

     // << tab << "[ -grib_code value ]    (default: " << default_grib_code << ")\n\n"

     << tab << "   lidar_filename\n"

     << "\n\n";


exit (1);

return;

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
/*

void set_config(const StringArray & a)

{

config_filename = a[0];

return;

}

*/
////////////////////////////////////////////////////////////////////////


void set_verbosity(const StringArray & a)

{

mlog.set_verbosity_level(atoi(a[0]));

return;

}


////////////////////////////////////////////////////////////////////////


void set_grib_code(const StringArray & a)

{

grib_code = atoi(a[0]);

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


void process_calipso_file(NcFile & out, const char * filename)

{

int j;
int hdf_sd_id;
int hdf_lat_index, hdf_lat_id;
int hdf_lon_index, hdf_lon_id;
int hdf_time_index, hdf_time_id;
int hdf_data_index, hdf_data_id;
int hdf_rank, hdf_atts, hdf_type;
int n_data;
int hdf_start[MAX_VAR_DIMS], hdf_stride[MAX_VAR_DIMS], hdf_edge[MAX_VAR_DIMS];
NcDim nc_dim;
NcVar nc_lat_var, nc_lon_var, nc_data_var, nc_time_var;


for (j=0; j<MAX_VAR_DIMS; ++j)  hdf_start[j] = hdf_stride[j] = hdf_edge[j] = 0;

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

const int hdr_typ_bytes = nhdr_dim_size*mxstr_dim_size;
const int hdr_sid_bytes = nhdr_dim_size*mxstr_dim_size;
const int hdr_vld_bytes = nhdr_dim_size*mxstr_dim_size;
const int hdr_arr_bytes = nhdr_dim_size*hdr_arr_len_dim_size*sizeof(float);

int buf_size = hdr_typ_bytes;

buf_size = max<int>(buf_size, hdr_sid_bytes);
buf_size = max<int>(buf_size, hdr_vld_bytes);
buf_size = max<int>(buf_size, hdr_arr_bytes);



   //
   //  add some dimensions to the netcdf file
   //

NcDim mxstr_dim;
NcDim hdr_arr_len_dim;
NcDim obs_arr_len_dim;
NcDim nhdr_dim;
NcDim nobs_dim;


out.addDim(mxstr_dim_name,       mxstr_dim_size);
out.addDim(hdr_arr_len_dim_name, hdr_arr_len_dim_size);
out.addDim(obs_arr_len_dim_name, n_obs_types);
out.addDim(nhdr_dim_name,        n_data);

out.addDim(nobs_dim_name);                  //  unlimited

mxstr_dim       = out.getDim(mxstr_dim_name);
hdr_arr_len_dim = out.getDim(hdr_arr_len_dim_name);
obs_arr_len_dim = out.getDim(obs_arr_len_dim_name);
nhdr_dim        = out.getDim(nhdr_dim_name);
nobs_dim        = out.getDim(nobs_dim_name);

   //
   //  add the variables to the netcdf file
   //

vector<NcDim> dims;
NcVar hdr_typ_var;
NcVar hdr_sid_var;
NcVar hdr_vld_var;
NcVar hdr_arr_var;
NcVar obs_qty_var;
NcVar obs_arr_var;


dims.resize(2);


    /////////////////////////////////////

       //
       //  hdr type variable
       //

dims.at(0) = nhdr_dim;
dims.at(1) = mxstr_dim;

out.addVar(hdr_typ_var_name, NcType::nc_CHAR, dims);

hdr_typ_var = out.getVar(hdr_typ_var_name);

(void) hdr_typ_var.putAtt(string("long_name"), string("message type"));

    /////////////////////////////////////

       //
       //  hdr sid variable
       //

out.addVar(hdr_sid_var_name, NcType::nc_CHAR, dims);

hdr_sid_var = out.getVar(hdr_sid_var_name);

(void) hdr_sid_var.putAtt(string("long_name"), string("station identification"));   //  we pause now for ...

    /////////////////////////////////////

       //
       //  hdr vld variable
       //

out.addVar(hdr_vld_var_name, NcType::nc_CHAR, dims);

hdr_vld_var = out.getVar(hdr_vld_var_name);

(void) hdr_vld_var.putAtt(string("long_name"), string("valid time"));

(void) hdr_vld_var.putAtt(string("units"), string("YYYYMMDD_HHMMS UTC"));

    /////////////////////////////////////

       //
       //  hdr arr variable
       //

dims.at(0) = nhdr_dim;
dims.at(1) = hdr_arr_len_dim;

out.addVar(hdr_arr_var_name, NcType::nc_FLOAT, dims);

hdr_arr_var = out.getVar(hdr_arr_var_name);

(void) hdr_arr_var.putAtt(string("long_name"),      string("array of observation station header values"));
(void) hdr_arr_var.putAtt(string("missing_value"),  NcType::nc_FLOAT, FILL_VALUE);
(void) hdr_arr_var.putAtt(string("_FillValue"),     NcType::nc_FLOAT, FILL_VALUE);
(void) hdr_arr_var.putAtt(string("columns"),        string("lat lon elv"));
(void) hdr_arr_var.putAtt(string("lat_long_name"),  string("latitude"));
(void) hdr_arr_var.putAtt(string("lat_units"),      string("degrees_north"));
(void) hdr_arr_var.putAtt(string("lon_long_name"),  string("longitude"));
(void) hdr_arr_var.putAtt(string("lon_units"),      string("degrees_east"));
(void) hdr_arr_var.putAtt(string("elev_long_name"), string("elevation"));
(void) hdr_arr_var.putAtt(string("elev_units"),     string("meters above sea level (msl)"));

    /////////////////////////////////////

       //
       //  obs qty variable
       //

dims.at(0) = nobs_dim;
dims.at(1) = mxstr_dim;

out.addVar(obs_qty_var_name, NcType::nc_CHAR, dims);

obs_qty_var = out.getVar(obs_qty_var_name);

(void) obs_qty_var.putAtt(string("long_name"), string("quality flag"));

    /////////////////////////////////////

       //
       //  obs arr variable
       //

dims.at(0) = nobs_dim;
dims.at(1) = obs_arr_len_dim;

out.addVar(obs_arr_var_name, NcType::nc_FLOAT, dims);

obs_arr_var = out.getVar(obs_arr_var_name);

(void) obs_arr_var.putAtt(string("long_name"),        string("array of observation values"));
(void) obs_arr_var.putAtt(string("missing_value"),    NcType::nc_FLOAT, FILL_VALUE);
(void) obs_arr_var.putAtt(string("_FillValue"),       NcType::nc_FLOAT, FILL_VALUE);
(void) obs_arr_var.putAtt(string("columns"),          string("hdr_id gc lvl hgt ob"));
(void) obs_arr_var.putAtt(string("hdr_id_long_name"), string("index of matching header data"));
(void) obs_arr_var.putAtt(string("gc_long_name"),     string("grib code corresponding to the observation type"));
(void) obs_arr_var.putAtt(string("lvl_long_name"),    string("pressure level (hPa) or accumulation interval (sec)"));
(void) obs_arr_var.putAtt(string("hgt_long_name"),    string("height in meters above sea level or ground level (msl or agl)"));
(void) obs_arr_var.putAtt(string("ob_long_name"),     string("observation value"));

   //
   //  global attributes for netcdf output file
   //

const unixtime now = time(0);
int month, day, year, hour, minute, second;
ConcatString s;
char junk [1 + mxstr_dim_size];

unix_to_mdyhms(now, month, day, year, hour, minute, second);

snprintf(junk, sizeof(junk),
         "%04d%02d%02d_%02d%02d%02d",
         year, month, day, hour, minute, second);


s << "File " << output_filename << " generated " << junk << " UTC";

memset(junk, 0, sizeof(junk));

if ( gethostname(junk, sizeof(junk) - 1) < 0 )  s << " on unknown host";
else                                            s << " on host " << junk;

(void) out.putAtt(string("FileOrigins"), string( s.text() ));
// (void) out.putAtt(string("MET_version"), string( VERSION ));
(void) out.putAtt(string("MET_version"), string( met_version ));
(void) out.putAtt(string("MET_tool"),    string(program_name.text()));

   //
   //  allocate the buffer
   //

unsigned char * buf = 0;

buf = new unsigned char [buf_size];

char  * const cbuf = (char *)  buf;
float * const fbuf = (float *) buf;


   //
   //  populate the hdr_typ variable
   //

memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*mxstr_dim_size, hdr_typ_string);

}

hdr_typ_var.putVar(cbuf);

   //
   //  populate the hdr_sid variable
   //

memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*mxstr_dim_size, na_string);

}

hdr_sid_var.putVar(cbuf);

   //
   //  populate the obs_qty variable
   //
/* 
memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*mxstr_dim_size, na_string);

}

obs_qty_var.putVar(cbuf);
*/
   //
   //  populate the hdr_arr variable
   //

float ff[2];

memset(buf, 0, buf_size);

for (j=0; j<n_data; ++j)  {

   hdf_5km.get_latlon(j, ff[0], ff[1]);

   fbuf[hdr_arr_len_dim_size*j]     = ff[0];   //  latitude

   fbuf[hdr_arr_len_dim_size*j + 1] = ff[1];   //  longitude

   fbuf[hdr_arr_len_dim_size*j + 2] = FILL_VALUE;

}   //  for j

hdr_arr_var.putVar(fbuf);

   //
   //  populate the hdr_vld variable
   //
   //    the time in the hdf file is seconds from Jan 1, 1993 0h
   //

int k;
HdfVarInfo info;
int layer;
unixtime t;
double dd;



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

   strcpy(cbuf + j*mxstr_dim_size, junk);

}   //  for j

hdr_vld_var.putVar(cbuf);

   //
   //  populate the obs_arr variable
   //

Calipso_5km_Obs obs;

memset(buf, 0, buf_size);

float f[5];

for (j=0; j<n_data; ++j)  {

   hdf_5km.get_obs(j, obs);

   for (layer=0; layer<(obs.n_layers); ++layer)  {

      obs.get_layer_base_record         (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_layer_top_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_opacity_record            (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_cad_score_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_feature_type_record       (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_feature_type_qa_record    (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_ice_water_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_ice_water_qa_record       (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_subtype_record            (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_cloud_aerosol_record      (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

      obs.get_h_average_record          (j, layer, f);
         write_nc_record                (out, obs_qty_var, obs_arr_var, f);

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


void write_nc_record(NcFile & out, NcVar & obs_qty_var, NcVar & obs_arr_var, const float * f)

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

index.at(0) = pos;
index.at(1) = 0;

count.at(0) = 1;
count.at(1) = na_len;

obs_qty_var.putVar(index, count, na_string);

   //
   //  done
   //

++pos;

return;

}


////////////////////////////////////////////////////////////////////////



