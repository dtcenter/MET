

////////////////////////////////////////////////////////////////////////


static const char default_units [] = "none";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

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

static ConcatString units = default_units;


////////////////////////////////////////////////////////////////////////


static void set_out_file  (const StringArray &);
static void set_grid_file (const StringArray &);
static void set_field     (const StringArray &);
static void set_units     (const StringArray &);

static void usage();

static void get_grid();

static void process(const char * input_filename);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

CommandLine cline;

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_out_file,  "-out",       1);
cline.add(set_grid_file, "-data_file", 1);
cline.add(set_field,     "-field",     1);
cline.add(set_units,     "-units",     1);

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

cout << "Processing " << modis_filename << "\n" << flush;

process(modis_filename);




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

mlog << Error
     << "\n\n   usage:  " << program_name << " -data_file path -units text -field name -out path modis_file\n\n";

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
met_ptr = m_factory.new_met_2d_data_file(grid_data_file);

if ( !met_ptr ) {

      mlog << Error << "\n" << program_name << " -> file \""
           << grid_data_file << "\" not a valid data file\n\n";
      exit (1);
}

grid = met_ptr->grid();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * input_filename)

{

int x, y;
int n0, n1;
int nonzero_count, percent;
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

// in.dump(cout);

in.select_data_field(modis_field);

mlog << Warning
     << "Need to fix scale/offset values!\n\n";

in.set_data_scale  (0.01);
in.set_data_offset (-15000.0);
in.set_data_fill   (-32768.0);

in.latlon_range(lat_min, lat_max, lon_min, lon_max);

cout << "\n  Lat range is " << lat_min << " to " << lat_max << "\n";
cout << "\n  Lon range is " << lon_min << " to " << lon_max << "\n";

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

cout << "Data range is " << data_min << " to " << data_max << "\n" << flush;

cout << "Nonzero count is " << nonzero_count << "\n" << flush;

percent = nint( (100.0*nonzero_count)/(nx*ny) );

ConcatString output_field_name = modis_field;

write_grid_to_netcdf(plane, grid, output_filename, output_field_name, modis_field, units);





   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


