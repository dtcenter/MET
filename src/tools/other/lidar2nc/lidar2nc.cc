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
NcDim nc_dim;
NcVar nc_lat_var, nc_lon_var, nc_data_var, nc_time_var;


   //
   //  open hdf file
   //

hdf_sd_id = SDstart(filename, DFACC_READ);

if ( hdf_sd_id < 0 )  {

   cerr << "\n\n  " << program_name << ": failed to open calypso file \"" << filename << "\"\n\n";

   exit ( 1 );

}


   //
   //  fill in the data
   //

nc_populate(out, hdf_sd_id, nc_lat_name,   hdf_lat_name);
nc_populate(out, hdf_sd_id, nc_lon_name,   hdf_lon_name);
nc_populate(out, hdf_sd_id, nc_time_name,  hdf_time_name);
nc_populate(out, hdf_sd_id, nc_data_name, hdf_data_name);

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
   //  get hdf info on this data
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



