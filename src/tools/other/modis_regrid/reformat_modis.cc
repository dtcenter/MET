

////////////////////////////////////////////////////////////////////////


static const char output_field_name [] = "MODIS";

static const char var_long_name     [] = "MODIS Cloud Fraction";
static const char units             [] = "none";

static const char algorithm_name    [] = "modis";


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

#include "data_plane_to_netcdf.h"

#include "data_averager.h"


////////////////////////////////////////////////////////////////////////


static const LambertData rapice_grid_data = { 

    "G236", 25.0, 25.0, 16.281,  126.138, 
     0.0, 0.0,  95.0, 40.635,   
     ncep_earth_radius_km,  151,  113 };

static ConcatString program_name;

static const Grid grid (rapice_grid_data);

static ConcatString output_directory = ".";


////////////////////////////////////////////////////////////////////////


static void set_outdir(const StringArray &);

static void usage();

static ConcatString make_output_filename(const ModisFile &, int percent);

static void process(const char * input_filename);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

CommandLine cline;

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outdir, "-outdir", 1);

cline.parse();

if ( cline.n() == 0 )  usage();

int j;

cout << "\n\n";

for (j=0; j<(cline.n()); ++j)  {

   if ( (j%5) == 0 )  cout << '\n';

   cout << "Processing " << cline[j] << " ... " << (j + 1) << " of " << cline.n() << "\n" << flush;

   process(cline[j]);

}

cout << "\n\n";




   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void set_outdir(const StringArray & a)

{

output_directory = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " modis_file\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString make_output_filename(const ModisFile & in, int percent)

{

ConcatString s;
char junk[256];
int month, day, year, hour, minute, second;

unix_to_mdyhms(in.scan_start_time(), month, day, year, hour, minute, second);

sprintf(junk, "%04d%02d%02d_%02d%02d%02d",
               year, month, day, hour, minute, second);

s << cs_erase
  << output_directory << '/'
  << algorithm_name << '_'
  << junk << 'V';

sprintf(junk, "%02d", percent);

s << '_' << junk << "P";

s << ".nc";

return ( s );

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
ConcatString output_filename;


if ( ! in.open(input_filename) )  {

   cerr << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}


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

      status = in.cloud_fraction(x, y, v);

      if ( ! status )  v = 0.0;

      a.put(v, x, y);

   }

}

data_min =  1.0e30;
data_max = -1.0e30;

nonzero_count = 0;

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      v = a.ave(x, y);

      if ( v < data_min )  data_min = v;
      if ( v > data_max )  data_max = v;

      if ( v > 0.0 )  ++nonzero_count;

      plane.set(v, x, y);

   }

}

cout << "Data range is " << data_min << " to " << data_max << "\n" << flush;

cout << "Nonzero count is " << nonzero_count << "\n" << flush;

percent = nint( (100.0*nonzero_count)/(nx*ny) );

output_filename = make_output_filename(in, percent);


write_grid_to_netcdf(plane, grid, output_filename, output_field_name, var_long_name, units);





   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


