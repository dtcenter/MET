

////////////////////////////////////////////////////////////////////////


static const bool verbose = true;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <cstdio>
#include <cmath>

#include "vx_cal.h"
#include "vx_util.h"

#include "mode_2d_input_file.h"
#include "mtd_file.h"
#include "mtd_nc_defs.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static int    Radius    = -1;
static double Threshold = -1.0;

static ConcatString output_directory = ".";

static ConcatString field_name = data_field_name;

static ConcatString prefix;


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static bool radius_set    = false;
static bool prefix_set    = false;

static int max_count = 0;

static double min_raw_value =  1.0e30;
static double max_raw_value = -1.0e30;


////////////////////////////////////////////////////////////////////////


inline int three_to_one(int Nx, int Ny, int Nz, int x, int y, int z)

{

return ( x*Ny*Nz + y*Nz + z );

}


////////////////////////////////////////////////////////////////////////


inline void one_to_three(int n, int Nx, int Ny, int Nz, int & x, int & y, int & z)

{

int k;

z = n%Nz;

k = n/Nz;   //  so k = x*Ny + y

y = k%Ny;

x = k/Ny;

return;

}


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_radius    (const StringArray &);
static void set_outdir    (const StringArray &);
static void set_field     (const StringArray &);
static void set_prefix    (const StringArray &);

static void process(const char *);

static double calc_conv_value(const ModeTimeDomainFile &, const int x, const int y, const int t);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_radius,    "-radius",    1);
cline.add(set_outdir,    "-outdir",    1);
cline.add(set_field,     "-field",     1);
cline.add(set_prefix,    "-prefix",    1);

cline.parse();

if ( cline.n() == 0 )  usage();

if ( !radius_set || !prefix_set )  usage();

int j;


for (j=0; j<(cline.n()); ++j)  {

   process(cline[j]);

}



   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " -radius n -prefix text [ -field name ] [ -outdir path ] raw_3d_file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_radius(const StringArray & a)

{

Radius = atoi(a[0]);

if ( Radius <= 0 )  {

   cerr << "\n\n  " << program_name << ": bad radius ... " << Radius << "\n\n";

   exit ( 1 );

}

radius_set = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_outdir(const StringArray & a)

{

output_directory = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_field(const StringArray & a)

{

field_name = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_prefix(const StringArray & a)

{

prefix = a[0];

prefix_set = true;

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * input_filename)

{

int j, n;
int x, y, t;
int bytes;
double value, denom;
double m, b;
ModeTimeDomainFile in, out;
double min_conv_value, max_conv_value;
ConcatString output_filename;
int month, day, year, hour, minute, second;
char junk[256];
double * conv_data = (double *) 0;
Unixtime time_start, time_stop;


   //
   //  open the input file
   //

if ( ! in.read(input_filename) )  {

   cerr << "\n\n  " << program_name << ": process() -> unable to open input file \"" << input_filename << "\"\n\n";

   exit ( 1 );

}

const int nx = in.nx();
const int ny = in.ny();
const int nt = in.nt();

conv_data = new double [nx*ny*nt];

if ( !conv_data )  {

   cerr << "\n\n  " << program_name << ": process() -> memory allocation error\n\n";

   exit ( 1 );

}

   //
   //  make the output filename
   //

output_filename.erase();

if ( output_directory.nonempty() )  output_filename << output_directory << '/';

if ( prefix.nonempty() )  output_filename << prefix << '_';

unix_to_mdyhms(in.start_time(), month, day, year, hour, minute, second);

sprintf(junk, "%04d%02d%02d_%02d%02d%02d", year, month, day, hour, minute, second);

output_filename << junk << '.';

sprintf(junk, "%02dR", Radius);

output_filename << junk;

output_filename << mtd_file_suffix;


   //
   //  get the min/max convolved data values
   //

time_start = time(0);

min_conv_value = max_conv_value = calc_conv_value(in, 0, 0, 0);

for (x=0; x<nx; ++x)  {

   // if ( verbose && ((x%100) == 0) )  cout << "Pass 1: x = " << x << " of " << nx << "\n" << flush;

   for (y=0; y<ny; ++y)  {

      for (t=0; t<nt; ++t)  {

         value = calc_conv_value(in, x, y, t);

         n = three_to_one(nx, ny, nt, x, y, t);

         conv_data[n] = value;

         if ( value < min_conv_value )  min_conv_value = value;
         if ( value > max_conv_value )  max_conv_value = value;

      }

   }

}

time_stop = time(0);

if ( verbose )  {

   cout << "\n  Conv data range = " << min_conv_value << " to " << max_conv_value << "\n"
        << "\n  Raw  data range = " << min_raw_value  << " to " << max_raw_value  << "\n"
        << "\n  Max count       = " << max_count      << "\n"
        << "\n  Conv time       = " << (time_stop - time_start) << " seconds\n"
        << flush;

}

   //
   //  setup the output file
   //

bytes = (int) sizeof(MtdDataType);

denom = 1.0;

for (j=0; j<bytes; ++j)  denom *= 256.0;

denom -= 1.0;   //  max value is 2^n - 1, where n is the number of bits

m = (max_conv_value - min_conv_value)/denom;

b = min_conv_value;


out.set_size(nx, ny, nt);

out.set_grid(in.grid());

out.set_radius(Radius);

out.set_threshold(Threshold);

out.set_start_time(in.start_time());

out.set_delta_t(in.delta_t());

out.set_mb(m, b);


   //
   //  calculate the data values
   //


for (x=0; x<nx; ++x)  {

   // if ( verbose && ((x%100) == 0) )  cout << "Pass 2: x = " << x << " of " << nx << "\n" << flush;

   for (y=0; y<ny; ++y)  {

      for (t=0; t<nt; ++t)  {

         n = three_to_one(nx, ny, nt, x, y, t);

         // value = calc_conv_value(in, x, y, t);
         value = conv_data[n];

         out.put_dval(value, x, y, t);

      }   //  for t

   }   //  for y

}   //  for x


   //
   //  write the file
   //

if ( ! out.write(output_filename) )  {

   cerr << "\n\n  " << program_name << ": process() -> trouble writing output file \"" << output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

if ( conv_data )  { delete [] conv_data;  conv_data = (double *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


double calc_conv_value(const ModeTimeDomainFile & in, const int x, const int y, const int t)

{

int ix, iy, it;
int xx, yy, tt;
int count;
double dx, dy;
double value, sum, dist;


sum = 0.0;

count = 0;


for (ix=-Radius; ix<=Radius; ++ix)  {

   xx = x + ix;

   if ( (xx < 0) || (xx >= in.nx()) )  continue;

   dx = (double) ix;

   for (iy=-Radius; iy<=Radius; ++iy)  {

      yy = y + iy;

      if ( (yy < 0) || (yy >= in.ny()) )  continue;

      dy = (double) iy;

      dist = sqrt(dx*dx + dy*dy);

      if ( dist > Radius )  continue;

      for (it=-1; it<=1; ++it)  {

         tt = t + it;

         if ( (tt < 0) || (tt >= in.nt()) )  continue;

         value = in(xx, yy, tt);

         min_raw_value = min(min_raw_value, value);
         max_raw_value = max(max_raw_value, value);

         sum += value;

         ++count;

      }   //  for it

   }   //  for iy

}   //  for ix


max_count = max(max_count, count);

if ( count == 0 )  value = 0.0;
else               value = sum/count;

   //
   //  done
   //

return ( value );

}


////////////////////////////////////////////////////////////////////////




