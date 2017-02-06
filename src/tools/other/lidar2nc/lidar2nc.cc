// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


static const char hdf_lat_name        [] = "Latitude";
static const char hdf_lon_name        [] = "Longitude";
static const char hdf_time_name       [] = "Profile_UTC_Time";
static const char hdf_data_name       [] = "Layer_Base_Altitude";

static const char nc_dim_name         [] = "track";

static const char nc_lat_name         [] = "lat";
static const char nc_lon_name         [] = "lon";
static const char nc_time_name        [] = "time_utc";
static const char nc_data_name        [] = "layer_base_altitude_km";

static const char na_string           [] = "NA";

static const char mxstr_dim_name      [] = "mxstr";
static const int  mxstr_dim_size         = 40;

static const char hdr_arr_len_dim_name [] = "hdf_arr_len";
static const int  hdr_arr_len_dim_size    = 2;

static const char obs_arr_len_dim_name [] = "obs_arr_len";
static const int  obs_arr_len_dim_size    = 5;

static const char nhdr_dim_name        [] = "nhdr";
static const char nobs_dim_name        [] = "nobs";

static const char hdr_type_string      [] = "calypso";

static const char hdr_type_var_name    [] = "hdr_type";
static const char hdr_sid_var_name     [] = "hdr_sid";
static const char hdr_vld_var_name     [] = "hdr_vld";
static const char hdr_arr_var_name     [] = "hdr_arr";
static const char obs_qty_var_name     [] = "obs_qty";
static const char obs_arr_var_name     [] = "obs_arr";


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


////////////////////////////////////////////////////////////////////////

// Constants

// static const char *DEFAULT_CONFIG_FILENAME = "MET_BASE/config/Ascii2NcConfig_default";

static ConcatString program_name;

static CommandLine cline;


////////////////////////////////////////////////////////////////////////

struct HdfVarInfo {

   int hdf_index;

   int hdf_id;

   int hdf_rank;

   int hdf_type;

   int hdf_atts;

   int hdf_dimsizes[MAX_VAR_DIMS];
};


////////////////////////////////////////////////////////////////////////

   //
   //   Command-line switches
   //

static ConcatString output_filename;

// static ConcatString config_filename(replace_path(DEFAULT_CONFIG_FILENAME));

// static Grid grid_mask;      // Grid masking region from the named Grid
// static MaskPoly poly_mask;
// StringArray mask_sid;

static int compress_level = -1;



////////////////////////////////////////////////////////////////////////


static void usage();

static void set_logfile   (const StringArray &);
static void set_outfile   (const StringArray &);
static void set_verbosity (const StringArray &);

static int sizeof_hdf_type(const int type);


static void process_calypso_file (NcFile &, const char * filename);

static int get_lat_size(const int hdf_sd_id, const char * hdf_lat_name);

static void get_hdf_var_info(const int hdf_sd_id, const char * hdf_name, HdfVarInfo &);

static void nc_populate(NcFile &, int hdf_sd_id, 
                        const char * nc_name, const char * hdf_name);


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

cline.add(set_logfile,   "-log", 1);
cline.add(set_outfile,   "-out", 1);
cline.add(set_verbosity, "-v",   1);

cline.parse();


if ( cline.n() < 1 )  usage();

if ( output_filename.empty() )  usage();

   //
   //  open the output file
   //

static NcFile ncf(output_filename.text(), NcFile::replace, NcFile::nc4);



   //
   //  process the lidar files
   //

int j;

for (j=0; j<(cline.n()); ++j)  {

   mlog << "Processing \"" << cline[j] << "\"\n";

   if ( (j%5) == 4 )  mlog << "\n";

   process_calypso_file(ncf, cline[j]);

}   //  for j



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


int sizeof_hdf_type(const int type)

{

int k = 0;

switch ( type )  {

   case DFNT_CHAR8:    k =  8;  break;   // 8-bit character type
   case DFNT_UCHAR8:   k =  8;  break;   // 8-bit unsigned character type
   case DFNT_INT8:     k =  8;  break;   // 8-bit integer type
   case DFNT_UINT8:    k =  8;  break;   // 8-bit unsigned integer type

   case DFNT_INT16:    k = 16;  break;   // 16-bit integer type
   case DFNT_UINT16:   k = 16;  break;   // 16-bit unsigned integer type

   case DFNT_INT32:    k = 32;  break;   // 32-bit integer type
   case DFNT_UINT32:   k = 32;  break;   // 32-bit unsigned integer type
   case DFNT_FLOAT32:  k = 32;  break;   // 32-bit floating-point type

   case DFNT_FLOAT64:  k = 64;  break;   // 64-bit floating-point type 

   default:
      mlog << Error
           << program_name << ": sizeof_hdf_type() -> unrecognized hdf data type\n\n";
      exit ( 1 );
      break;

}   //  switch



return ( k );

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


void process_calypso_file(NcFile & out, const char * filename)

{

int j, n;
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

   cerr << "\n\n  " << program_name << ": failed to open calypso file \"" << filename << "\"\n\n";

   exit ( 1 );

}


   //
   //  get number of data points 
   //
   //    we'll assume this is the number of latutide points
   //

n_data = get_lat_size(hdf_sd_id, hdf_lat_name);

const int nhdr_dim_size = n_data;
const int nobs_dim_size = n_data;

   //
   //  see how big a buffer we'll need
   //

const int hdr_type_bytes = nhdr_dim_size*mxstr_dim_size;
const int hdr_sid_bytes  = nhdr_dim_size*mxstr_dim_size;
const int hdr_vld_bytes  = nhdr_dim_size*mxstr_dim_size;
const int hdr_arr_bytes  = nhdr_dim_size*hdr_arr_len_dim_size*sizeof(float);
const int obs_qty_bytes  = nobs_dim_size*mxstr_dim_size;
const int obs_arr_bytes  = nobs_dim_size*obs_arr_len_dim_size*sizeof(float);

int buf_size = hdr_type_bytes;

buf_size = max<int>(buf_size, hdr_sid_bytes);
buf_size = max<int>(buf_size, hdr_vld_bytes);
buf_size = max<int>(buf_size, hdr_arr_bytes);
buf_size = max<int>(buf_size, obs_qty_bytes);
buf_size = max<int>(buf_size, obs_arr_bytes);



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
out.addDim(obs_arr_len_dim_name, obs_arr_len_dim_size);
out.addDim(nhdr_dim_name,        n_data);
out.addDim(nobs_dim_name,        n_data);

mxstr_dim       = out.getDim(mxstr_dim_name);
hdr_arr_len_dim = out.getDim(hdr_arr_len_dim_name);
obs_arr_len_dim = out.getDim(obs_arr_len_dim_name);
nhdr_dim        = out.getDim(nhdr_dim_name);
nobs_dim        = out.getDim(nobs_dim_name);

   //
   //  add the variables to the netcdf file
   //

vector<NcDim> dims;
NcVar hdr_type_var;
NcVar hdr_sid_var;
NcVar hdr_vld_var;
NcVar hdr_arr_var;
NcVar obs_qty_var;
NcVar obs_arr_var;


dims.resize(2);

    /////////////////////////////////////

dims.at(0) = nhdr_dim;
dims.at(1) = mxstr_dim;

out.addVar(hdr_type_var_name, NcType::nc_CHAR, dims);

hdr_type_var = out.getVar(hdr_type_var_name);

    /////////////////////////////////////

out.addVar(hdr_sid_var_name, NcType::nc_CHAR, dims);

hdr_sid_var = out.getVar(hdr_sid_var_name);

    /////////////////////////////////////

out.addVar(hdr_vld_var_name, NcType::nc_CHAR, dims);

hdr_vld_var = out.getVar(hdr_vld_var_name);

    /////////////////////////////////////

dims.at(0) = nhdr_dim;
dims.at(1) = obs_arr_len_dim;

out.addVar(hdr_arr_var_name, NcType::nc_FLOAT, dims);

hdr_arr_var = out.getVar(hdr_arr_var_name);

    /////////////////////////////////////

dims.at(0) = nobs_dim;
dims.at(1) = mxstr_dim;

out.addVar(obs_qty_var_name, NcType::nc_CHAR, dims);

obs_qty_var = out.getVar(obs_qty_var_name);

    /////////////////////////////////////

dims.at(0) = nobs_dim;
dims.at(1) = obs_arr_len_dim;

out.addVar(obs_arr_var_name, NcType::nc_FLOAT, dims);

obs_arr_var = out.getVar(obs_arr_var_name);

   //
   //  allocate the buffer
   //

unsigned char * buf = 0;

buf = new unsigned char [buf_size];

char  * const cbuf = (char *) buf;
float * const fbuf = (float *) buf;


   //
   //  populate the hdr_typ variable
   //



memset(cbuf, 0, n);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*mxstr_dim_size, hdr_type_string);

}

hdr_type_var.putVar(cbuf);

   //
   //  populate the hdr_sid variable
   //

memset(cbuf, 0, n);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*mxstr_dim_size, na_string);

}

hdr_sid_var.putVar(cbuf);

   //
   //  populate the obs_qty variable
   //

memset(cbuf, 0, n);

for (j=0; j<n_data; ++j)  {

   strcpy(cbuf + j*mxstr_dim_size, na_string);

}

obs_qty_var.putVar(cbuf);

   //
   //  populate the hdr_arr variable
   //

HdfVarInfo lat_info, lon_info;
float ff[2];

get_hdf_var_info(hdf_sd_id, hdf_lat_name, lat_info);
get_hdf_var_info(hdf_sd_id, hdf_lon_name, lon_info);

for (j=0; j<n_data; ++j)  {

   hdf_start[0] = j;
   hdf_start[1] = 0;

   hdf_stride[0] = 1;
   hdf_stride[1] = 1;

   hdf_edge[0] = 1;
   hdf_edge[1] = 1;

   if ( SDreaddata(lat_info.hdf_id, hdf_start, hdf_stride, hdf_edge, ff) < 0 )  {

      cerr << "\n\n  " << program_name << ": SDreaddata failed\n\n";

      exit ( 1 );

   }

   fbuf[2*j] = ff[0];

   if ( SDreaddata(lon_info.hdf_id, hdf_start, hdf_stride, hdf_edge, ff) < 0 )  {

      cerr << "\n\n  " << program_name << ": SDreaddata failed\n\n";

      exit ( 1 );

   }

   fbuf[2*j + 1] = ff[0];

}   //  for j

hdr_arr_var.putVar(fbuf);


























// nc_populate(out, hdf_sd_id, nc_lat_name,   hdf_lat_name);
// nc_populate(out, hdf_sd_id, nc_lon_name,   hdf_lon_name);
// nc_populate(out, hdf_sd_id, nc_time_name,  hdf_time_name);
// nc_populate(out, hdf_sd_id, nc_data_name,  hdf_data_name);

   //
   //  close hdf file
   //

if ( SDend(hdf_sd_id) < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to close file\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////


int get_lat_size(const int hdf_sd_id, const char * hdf_lat_name)

{

int n_data;
int hdf_index, hdf_id;
int hdf_dimsizes[MAX_VAR_DIMS];
int hdf_rank, hdf_type, hdf_atts;


if ( (hdf_index = SDnametoindex(hdf_sd_id, hdf_lat_name)) < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to get index for \""
        << hdf_lat_name << "\"\n\n";

   exit ( 1 );

}

if ( (hdf_id = SDselect(hdf_sd_id, hdf_index)) < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to get id for \""
        << hdf_lat_name << "\"\n\n";

   exit ( 1 );

}

if ( SDgetinfo(hdf_id, 0, &hdf_rank, hdf_dimsizes, &hdf_type, &hdf_atts) < 0 )  {

   cerr << "\n\n  " << program_name << ": SDgetinfo failed\n\n";

   exit ( 1 );

}


n_data = max<int>(hdf_dimsizes[0], hdf_dimsizes[1]);

   //
   //  done
   //

return ( n_data );

}


////////////////////////////////////////////////////////////////////////


void get_hdf_var_info(const int hdf_sd_id, const char * hdf_name, HdfVarInfo & info)

{

if ( (info.hdf_index = SDnametoindex(hdf_sd_id, hdf_name)) < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to get index for \""
        << hdf_name << "\"\n\n";

   exit ( 1 );

}

if ( (info.hdf_id = SDselect(hdf_sd_id, info.hdf_index)) < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to get id for \""
        << hdf_name << "\"\n\n";

   exit ( 1 );

}

if ( SDgetinfo(info.hdf_id, 0, &(info.hdf_rank), info.hdf_dimsizes, &(info.hdf_type), &(info.hdf_atts)) < 0 )  {

   cerr << "\n\n  " << program_name << ": SDgetinfo failed\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void nc_populate(NcFile & out, int hdf_sd_id, const char * nc_name, const char * hdf_name)

{

int j;
int hdf_index, hdf_id;
int hdf_rank;
int hdf_type, hdf_atts;
int hdf_start[MAX_VAR_DIMS], hdf_stride[MAX_VAR_DIMS], hdf_edge[MAX_VAR_DIMS];
int hdf_dimsizes[MAX_VAR_DIMS];
int n_bytes, n_data;
NcVar nc_var;
vector<NcDim> dims;
unsigned char * buf = 0;
ConcatString dim_name;
NcType::ncType nc_type;


   //
   //  get hdf index and id for this variable
   //

if ( (hdf_index = SDnametoindex(hdf_sd_id, hdf_name)) < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to get index for \""
        << hdf_name << "\"\n\n";

   exit ( 1 );

}

if ( (hdf_id = SDselect(hdf_sd_id, hdf_index)) < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to get id for \""
        << hdf_name << "\"\n\n";

   exit ( 1 );

}

   //
   //  get hdf info on this variable
   //

if ( SDgetinfo(hdf_id, 0, &hdf_rank, hdf_dimsizes, &hdf_type, &hdf_atts) < 0 )  {

   cerr << "\n\n  " << program_name << ": SDgetinfo failed\n\n";

   exit ( 1 );

}

nc_type = hdf_type_to_nc_type(hdf_type);

   //
   //  how many data points altogether?
   //

n_data = 1;

for (j=0; j<hdf_rank; ++j)  n_data *= hdf_dimsizes[j];

   //
   //  allocate the buffer
   //

n_bytes = n_data*sizeof_hdf_type(hdf_type);

buf = new unsigned char [n_bytes];

   //
   //  add dims to output file
   //

dims.resize(hdf_rank);

for (j=0; j<hdf_rank; ++j)  {

   dim_name << cs_erase << nc_name << "_dim_" << (j + 1);

   dims.at(j) = out.addDim(dim_name.text(), hdf_dimsizes[j]);

}

   //
   //  add variable to output file
   //

nc_var = out.addVar(string(nc_name), nc_type, dims);

  //
  //  select hdf data cube
  //

for (j=0; j<MAX_VAR_DIMS; ++j)  hdf_start[j] = hdf_stride[j] = hdf_edge[j] = 0;

for (j=0; j<hdf_rank; ++j)  {

   hdf_start[j] = 0;

   hdf_stride[j] = 1;

   hdf_edge[j] = hdf_dimsizes[j];

}

   //
   //  read the data from the hdf file into the buffer
   //

if ( SDreaddata(hdf_id, hdf_start, hdf_stride, hdf_edge, buf) < 0 )  {

   cerr << "\n\n  " << program_name << ": SDreaddata failed\n\n";

   exit ( 1 );

}

   //
   //  write the data to the netcdf file
   //
   //    the NcVar::putVar function is overloaded, so we have to 
   //     use a cast to make sure the right function is called
   //

switch ( nc_type )  {

   case NcType::nc_FLOAT:   nc_var.putVar((const float *)  buf);  break;
   case NcType::nc_DOUBLE:  nc_var.putVar((const double *) buf);  break;

   case NcType::nc_INT:     nc_var.putVar((const int *)    buf);  break;

   case NcType::nc_CHAR:    nc_var.putVar((const char *)   buf);  break;

   default:
      mlog << Error
           << program_name << ": nc_populate() -> bad netcdf data type ... " << nc_type << "\n\n";
      exit ( 1 );
      break;

}   //  switch

   //
   //  done
   //

if ( buf )  { delete [] buf;  buf = 0; }

return;

}


////////////////////////////////////////////////////////////////////////



