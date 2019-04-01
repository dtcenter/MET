// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const char default_units [] = "none";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>
#include <ctype.h>

#include "modis_file.h"

#include "vx_util.h"
#include "vx_log.h"
#include "vx_math.h"
#include "vx_grid.h"
#include "vx_data2d_factory.h"

#include "data_plane_to_netcdf.h"

#include "data_averager.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static Grid grid;

static ConcatString grid_data_file;

static ConcatString output_filename;

static ConcatString modis_filename;

static ConcatString modis_field;

static ConcatString units = (string)default_units;

static double data_fill_value = -9999.0;

static double data_scale      = 1.0;
static double data_offset     = 0.0;

static int compress_level = -1;

////////////////////////////////////////////////////////////////////////


static void set_out_file  (const StringArray &);
static void set_grid_file (const StringArray &);
static void set_field     (const StringArray &);
static void set_units     (const StringArray &);
static void set_scale     (const StringArray &);
static void set_offset    (const StringArray &);
static void set_fillvalue (const StringArray &);
static void set_compress  (const StringArray &);
static void set_verbosity(const StringArray & a);

static void usage();

static void get_grid();

static void process(const char * input_filename);

int  get_compress();

////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

CommandLine cline;

cline.set(argc, argv);

cline.set_usage(usage);

cline.allow_numbers();

cline.add(set_out_file,  "-out",       1);
cline.add(set_grid_file, "-data_file", 1);
cline.add(set_field,     "-field",     1);
cline.add(set_units,     "-units",     1);
cline.add(set_scale,     "-scale",     1);
cline.add(set_offset,    "-offset",    1);
cline.add(set_fillvalue, "-fill",      1);
cline.add(set_verbosity, "-v", 1);
cline.add(set_compress,  "-compress",  1);

cline.parse();

if ( cline.n() != 1 )  usage();

if ( grid_data_file.empty() )  {

   mlog << Error
        << program_name << ": must specify a data file\n";

   exit ( 1 );

}

if ( output_filename.empty() )  {

   mlog << Error
        << program_name << ": must specify an output filename\n";

   exit ( 1 );

}

if ( modis_field.empty() )  {

   mlog << Error
        << program_name << ": must specify a modis data field\n";

   exit ( 1 );

}

modis_filename = cline[0];

get_grid();

mlog << Debug(1) << "Processing " << modis_filename << "\n";

process(modis_filename.c_str());




   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void set_out_file(const StringArray & a)

{

output_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_grid_file(const StringArray & a)

{

grid_data_file = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_field(const StringArray & a)

{

modis_field = a[0];

return;

}


////////////////////////////////////////////////////////////////////////

void set_units(const StringArray & a)

{

units = a[0];

return;

}



////////////////////////////////////////////////////////////////////////


void usage()

{

ConcatString tab;

tab.set_repeat(' ', 13);

cout << "\n"
     << "Usage: " << program_name << '\n'
     << tab << "-data_file path\n"
     << tab << "-field name\n"
     << tab << "-out path\n"
     << tab << "-scale value\n"
     << tab << "-offset value\n"
     << tab << "-fill value\n"
     << tab << "[-units text]\n"
     << tab << "[-compress level]\n"
     << tab << "modis_file\n\n"

     << "  where  \"-data_file path\" specifies the data files used to get the grid information.\n"
     << "         \"-field name\" specifies the name of the field to use in the modis data file.\n"
     << "         \"-out path\" specifies the name of the output netcdf file.\n"
     << "         \"-scale value\" specifies the scale factor to be used on the raw modis values.\n"
     << "         \"-offset value\" specifies the offset value to be used on the raw modis values.\n"
     << "         \"-fill value\" specifies the bad data value in the modis data.\n"
     << "         \"-units text\" specifies the units string in the global attributes section of the output file (optional).\n"
     << "         \"-compress level\" specifies the compression level of output NetCDF variable (optional).\n"
     << "         \"modis_file\" is the name of the modis input file.\n"

     << "\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void get_grid()

{

   //
   //  stole this code from plot_data_plane.cc
   //

Met2dDataFile * met_ptr = (Met2dDataFile * ) 0;
Met2dDataFileFactory m_factory;

mlog << Debug(1)  << "Opening data file: " << grid_data_file << "\n";
met_ptr = m_factory.new_met_2d_data_file(grid_data_file.c_str());

if ( !met_ptr ) {

      mlog << Error << "\n" << program_name << " -> file \""
           << grid_data_file << "\" not a valid data file\n\n";
      exit (1);
}

grid = met_ptr->grid();

   //
   //  done
   //

delete met_ptr;   met_ptr = (Met2dDataFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void set_scale(const StringArray & a)

{

data_scale = atof(a[0].c_str());

return;

}


////////////////////////////////////////////////////////////////////////


void set_offset(const StringArray & a)

{

data_offset = atof(a[0].c_str());

return;

}


////////////////////////////////////////////////////////////////////////


void set_fillvalue(const StringArray & a)

{

data_fill_value = atof(a[0].c_str());

return;

}


////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

///////////////////////////////////////////////////////////////////////////////////////////////

int get_compress() {
   return ((compress_level < 0) ? 0 : compress_level);
}

/////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

/////////////////////////////////////////////////


void process(const char * input_filename)

{

int x, y;
int n0, n1;
int nonzero_count;
double v;
double lat, lon;
double lat_min, lat_max, lon_min, lon_max;
double dx, dy;
bool status = false;
ModisFile in;
DataPlane plane;
DataAverager a;
double data_min, data_max;


if ( ! in.open(input_filename) )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

in.select_data_field(modis_field.c_str());

in.set_data_scale(data_scale);
in.set_data_offset(data_offset);
in.set_data_fill_value(data_fill_value);

in.latlon_range(lat_min, lat_max, lon_min, lon_max);

mlog << Debug(2) << "Lat range is " << lat_min << " to " << lat_max << "\n";
mlog << Debug(2) << "Lon range is " << lon_min << " to " << lon_max << "\n";

plane.set_size(grid.nx(), grid.ny());

plane.set_valid(in.scan_start_time());

plane.set_init(in.scan_start_time());

plane.set_lead  (0);
plane.set_accum (0);

const int nx = grid.nx();
const int ny = grid.ny();

a.set(grid);

for (n0=0; n0<(in.dim0()); ++n0)  {

   for (n1=0; n1<(in.dim1()); ++n1)  {

      lat = in.lat(n0, n1);
      lon = in.lon(n0, n1);

      grid.latlon_to_xy(lat, lon, dx, dy);

      x = nint(dx);
      y = nint(dy);

      if ( (x < 0) || (x >= nx) || (y < 0) || (y >= ny) )  continue;

      status = in.data(n0, n1, v);

      if ( ! status )  v = bad_data_float;

      a.put(v, x, y);

   }

}

data_min =  1.0e30;
data_max = -1.0e30;

nonzero_count = 0;

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      v = a.ave(x, y);

      if ( ! is_bad_data(v) )  {

         if ( v < data_min )  data_min = v;
         if ( v > data_max )  data_max = v;

         if ( v > 0.0 )  ++nonzero_count;

      }

      plane.set(v, x, y);

   }

}

mlog << Debug(2) << "Data range is " << data_min << " to " << data_max << "\n";

mlog << Debug(2) << "Nonzero count is " << nonzero_count << "\n";

ConcatString output_field_name = modis_field;

write_grid_to_netcdf(plane, grid, output_filename.c_str(), output_field_name.c_str(), modis_field.c_str(), units.c_str());





   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


