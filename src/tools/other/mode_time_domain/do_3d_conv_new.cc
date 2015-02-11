

////////////////////////////////////////////////////////////////////////


static const bool verbose = true;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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


struct DataHandle {

   int nx, ny;

   double * data_plane_below;
   double * data_plane_this;
   double * data_plane_above;

   double * sum_plane_below;
   double * sum_plane_this;
   double * sum_plane_above;

   // bool * ok_plane_below;
   // bool * ok_plane_this;
   // bool * ok_plane_above;

   void set_size(int _nx, int _ny);

   DataHandle() {

      nx = ny = 0;

      data_plane_below = data_plane_this = data_plane_above = 0;

       sum_plane_below =  sum_plane_this =  sum_plane_above = 0;

      // ok_plane_below = ok_plane_this = ok_plane_above = 0;

   }

  ~DataHandle() { clear(); }

   void clear() {

      if ( data_plane_below ) { delete data_plane_below;  data_plane_below = 0; }
      if ( data_plane_this  ) { delete data_plane_this ;  data_plane_this  = 0; }
      if ( data_plane_above ) { delete data_plane_above;  data_plane_above = 0; }

      if (  sum_plane_below ) { delete  sum_plane_below;   sum_plane_below = 0; }
      if (  sum_plane_this  ) { delete  sum_plane_this ;   sum_plane_this  = 0; }
      if (  sum_plane_above ) { delete  sum_plane_above;   sum_plane_above = 0; }

      // if ( ok_plane_below ) { delete ok_plane_below;  ok_plane_below = 0; }
      // if ( ok_plane_this  ) { delete ok_plane_this ;  ok_plane_this  = 0; }
      // if ( ok_plane_above ) { delete ok_plane_above;  ok_plane_above = 0; }

      return;

   }

};


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static bool radius_set    = false;
static bool prefix_set    = false;

// static int max_count = 0;
// 
// static double min_raw_value =  1.0e30;
// static double max_raw_value = -1.0e30;

static int rm1o2;

static double * sum_plane_buf = 0;


////////////////////////////////////////////////////////////////////////


inline int three_to_one(int Nx, int Ny, int Nt, int x, int y, int t)

{

// return ( x*Ny*Nt + y*Nt + t );

return ( (t*Ny + y)*Nx + x );

}


////////////////////////////////////////////////////////////////////////


// inline void one_to_three(int n, int Nx, int Ny, int Nz, int & x, int & y, int & z)
// 
// {
// 
// int k;
// 
// z = n%Nz;
// 
// k = n/Nz;   //  so k = x*Ny + y
// 
// y = k%Ny;
// 
// x = k/Ny;
// 
// return;
// 
// }


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_radius    (const StringArray &);
static void set_outdir    (const StringArray &);
static void set_field     (const StringArray &);
static void set_prefix    (const StringArray &);

static void process(const char *);

static void   get_data_plane(const ModeTimeDomainFile &, const int t, double * data_plane);

// static double calc_conv_value(const ModeTimeDomainFile &, const int x, const int y, const int t);

static void   calc_sum_plane(const int nx, const int ny, const double * data_plane, double * sum_plane);

static void load_handle(DataHandle &, const ModeTimeDomainFile & in, const int t);

static const char * dump_range(const double *, const int);


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

if ( Radius < 3 )  {

   cerr << "\n\n  " << program_name << ": bad radius ... " << Radius << "\n\n";

   exit ( 1 );

}


if ( (Radius%2) == 0 )  {

   cerr << "\n\n  " << program_name << ": radius must be an odd number\n\n";

   exit ( 1 );

}

rm1o2 = (Radius - 1)/2;

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
DataHandle handle;
Unixtime time_start, time_stop;
double * p = 0;
double * s_below = 0;
double * s_this  = 0;
double * s_above = 0;
const double scale = 1.0/(Radius*Radius*3);


min_conv_value = 0.0;
max_conv_value = 0.0;

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

const int nxy = nx*ny;

sum_plane_buf = new double [nxy];

handle.set_size(nx, ny);

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

min_conv_value =  1.0e100;
max_conv_value = -1.0e100;

time_start = time(0);

for (t=0; t<nt; ++t)  {

   n = three_to_one(nx, ny, nt, 0, 0, t);

   p = conv_data + n;

   load_handle(handle, in, t);

   s_below = handle.sum_plane_below;
   s_this  = handle.sum_plane_this;
   s_above = handle.sum_plane_above;

   cout << "T = " << t << '\n';

   cout << "s_below range is " << dump_range(s_below, nxy) << '\n';
   cout << "s_this  range is " << dump_range(s_this,  nxy) << '\n';
   cout << "s_above range is " << dump_range(s_above, nxy) << '\n';

   cout << '\n';

   cout << "data_below range is " << dump_range(handle.data_plane_below, nxy) << '\n';
   cout << "data_this  range is " << dump_range(handle.data_plane_this,  nxy) << '\n';
   cout << "data_above range is " << dump_range(handle.data_plane_above, nxy) << '\n';

   cout << "\n\n" << flush;

   for (j=0; j<nxy; ++j)  {

      value = (*s_below) + (*s_this) + (*s_above);

      value *= scale;

      if ( value < min_conv_value )  min_conv_value = value;
      if ( value > max_conv_value )  max_conv_value = value;

      *p++ = value;

      ++s_below;
      ++s_this;
      ++s_above;

   }

}

time_stop = time(0);

cout << "Conv data range is " << min_conv_value << " to " << max_conv_value << "\n\n" << flush;



// min_conv_value = max_conv_value = calc_conv_value(in, 0, 0, 0);
/*
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
*/

if ( verbose )  {

   cout << "\n  Conv time       = " << (time_stop - time_start) << " seconds\n"
        << flush;

}

   //
   //  setup the output file
   //

bytes = (int) sizeof(MtdDataType);

denom = 1.0;

for (j=0; j<bytes; ++j)  denom *= 256.0;

denom -= 1.0;   //  max value is 2^n - 1, where n is the number of bits

// m = (max_conv_value - min_conv_value)/denom;
// 
// b = min_conv_value;

m = in.m();
b = in.b();


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

if ( sum_plane_buf )  { delete sum_plane_buf;  sum_plane_buf = 0; }

return;

}


////////////////////////////////////////////////////////////////////////

/*
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
*/

////////////////////////////////////////////////////////////////////////


void DataHandle::set_size(int _nx, int _ny)

{

if ( (_nx <= 0) || (_ny <= 0) )  {

   cerr << "\n\n  DataHandle::set_size() -> bad size\n\n";

   exit ( 1 );

}

clear();

nx = _nx;
ny = _ny;

int j;
const int nxy = nx*ny;

data_plane_below = new double [nxy];
data_plane_this  = new double [nxy];
data_plane_above = new double [nxy];

sum_plane_below  = new double [nxy];
sum_plane_this   = new double [nxy];
sum_plane_above  = new double [nxy];

// ok_plane_below = new bool [nxy];
// ok_plane_this  = new bool [nxy];
// ok_plane_above = new bool [nxy];

for (j=0; j<nxy; ++j)  {

   data_plane_below[j] = 0.0;

   // ok_plane_below[j] = true;

}

memcpy(data_plane_this,  data_plane_below, nxy*sizeof(double));
memcpy(data_plane_above, data_plane_below, nxy*sizeof(double));

memcpy(sum_plane_below, data_plane_below, nxy*sizeof(double));
memcpy(sum_plane_this,  data_plane_below, nxy*sizeof(double));
memcpy(sum_plane_above, data_plane_below, nxy*sizeof(double));

// memcpy(ok_plane_this,  ok_plane_below, nxy*sizeof(bool));
// memcpy(ok_plane_above, ok_plane_below, nxy*sizeof(bool));


return;

}


////////////////////////////////////////////////////////////////////////


void get_data_plane(const ModeTimeDomainFile & mtd, const int t, double * data_plane)

{

int x, y;
double * d = data_plane;
double value;
const int nx = mtd.nx();
const int ny = mtd.ny();

// cout << "In get_data_plane\n" << flush;

for (y=0; y<ny; ++y)  {

   for (x=0; x<nx; ++x)  {

      value = mtd(x, y, t);

      *d++ = value;

   }

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void calc_sum_plane(const int nx, const int ny, const double * data_plane, double * sum_plane)

{

int x, y, n;
double moving_sum = 0.0;
const int nxy = nx*ny;
const int xmin = rm1o2;
const int ymin = rm1o2;
const int xmax = nx - rm1o2;
const int ymax = ny - rm1o2;
const double * front = 0;
const double * back  = 0;
double * put = 0;

// cout << "in calc_sum_plane\n" << flush;

   //
   //  zero out the sum plane buffer
   //

memset(sum_plane_buf, 0, nxy*sizeof(double));

   //
   //  calculate sums in x-direction for each y
   //

for (y=0; y<ny; ++y)  {

   n = three_to_one(nx, ny, 0, 0, y, 0);

   back = data_plane + n;

   front = back + (xmin + 1);

   put = sum_plane_buf + (n + xmin);

   moving_sum = 0.0;

   for (x=0; x<=xmin; ++x)  moving_sum += back[x];

   for (x=xmin; x<xmax; ++x)  {

      moving_sum += *front++;

      *put++ = moving_sum;

      moving_sum -= *back++;

   }

}

// cout << "buf plane range = " << dump_range(sum_plane_buf, nxy) << '\n' << flush;

   //
   //  calculate sums in y-direction for each x
   //

for (x=0; x<nx; ++x)  {

   n = three_to_one(nx, ny, 0, x, 0, 0);

   back = sum_plane_buf + n;

   front = back + (ymin*nx);

   put = sum_plane + (n + rm1o2*nx);

   moving_sum = 0.0;

   for (y=0; y<=ymin; ++y)  moving_sum += back[y + y*nx];

   for (y=ymin; y<ymax; ++y)  {

      moving_sum += *front;

      front += nx;

      *put = moving_sum;

      // if ( moving_sum > 3 )  cout << moving_sum << '\n' << flush;

      put += nx;

      moving_sum -= *back;

      back += nx;

   }

}

// cout << "sum plane range = " << dump_range(sum_plane, nxy) << '\n' << flush;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void load_handle(DataHandle & handle, const ModeTimeDomainFile & in, const int t)

{

const bool first = (t == 0);
const bool last  = (t == (in.nt() - 1));
const int bytes = (in.nx())*(in.ny())*sizeof(double);
// const int nxy = (in.nx())*(in.ny());


if ( first )  {

   get_data_plane(in, 0, handle.data_plane_this);
   get_data_plane(in, 1, handle.data_plane_above);

   calc_sum_plane(in.nx(), in.ny(), handle.data_plane_this,  handle.sum_plane_this);
   calc_sum_plane(in.nx(), in.ny(), handle.data_plane_above, handle.sum_plane_above);

   return;

}

memcpy(handle.data_plane_below, handle.data_plane_this,  bytes);
memcpy(handle.data_plane_this,  handle.data_plane_above, bytes);

memcpy(handle.sum_plane_below, handle.sum_plane_this,  bytes);
memcpy(handle.sum_plane_this,  handle.sum_plane_above, bytes);

if ( last )  {

   memset(handle.data_plane_above, 0, bytes);

   memset(handle.sum_plane_above,  0, bytes);

} else {

   get_data_plane(in, t, handle.data_plane_above);

   calc_sum_plane(in.nx(), in.ny(), handle.data_plane_above, handle.sum_plane_above);

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


const char * dump_range(const double * p, const int n)

{

int j;
double value, min_value, max_value;
static char junk[256];

max_value = min_value = *p;

for (j=1; j<n; ++j)  {

   value = *p++;

   if ( value < min_value )  min_value = value;
   if ( value > max_value )  max_value = value;

}

sprintf(junk, "%.3f to %.3f", min_value, max_value);

return ( junk );

}


////////////////////////////////////////////////////////////////////////





