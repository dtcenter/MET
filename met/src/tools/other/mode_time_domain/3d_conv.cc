

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

#include "mtd_file.h"
#include "mtd_nc_defs.h"


////////////////////////////////////////////////////////////////////////


static int conv_radius = -1;

static double * sum_plane_buf = 0;


////////////////////////////////////////////////////////////////////////


struct DataHandle {

   int nx, ny;

   double * data_plane_below;
   double * data_plane_this;
   double * data_plane_above;

   double * sum_plane_below;
   double * sum_plane_this;
   double * sum_plane_above;

   bool * ok_plane_below;
   bool * ok_plane_this;
   bool * ok_plane_above;

   void set_size(int _nx, int _ny);

   DataHandle() {

      nx = ny = 0;

      data_plane_below = data_plane_this = data_plane_above = 0;

       sum_plane_below =  sum_plane_this =  sum_plane_above = 0;

      ok_plane_below = ok_plane_this = ok_plane_above = 0;

   }

  ~DataHandle() { clear(); }

   void clear() {

      if ( data_plane_below ) { delete data_plane_below;  data_plane_below = 0; }
      if ( data_plane_this  ) { delete data_plane_this ;  data_plane_this  = 0; }
      if ( data_plane_above ) { delete data_plane_above;  data_plane_above = 0; }

      if (  sum_plane_below ) { delete  sum_plane_below;   sum_plane_below = 0; }
      if (  sum_plane_this  ) { delete  sum_plane_this ;   sum_plane_this  = 0; }
      if (  sum_plane_above ) { delete  sum_plane_above;   sum_plane_above = 0; }

      if ( ok_plane_below ) { delete ok_plane_below;  ok_plane_below = 0; }
      if ( ok_plane_this  ) { delete ok_plane_this ;  ok_plane_this  = 0; }
      if ( ok_plane_above ) { delete ok_plane_above;  ok_plane_above = 0; }

      return;

   }

};


////////////////////////////////////////////////////////////////////////


static void get_data_plane(const MtdFloatFile &, const int t, double * data_plane);

static void calc_sum_plane(const int nx, const int ny, const double * data_plane, double * sum_plane);

static void load_handle(DataHandle &, const MtdFloatFile & in, const int t);

// static const char * dump_range(const double *, const int);


////////////////////////////////////////////////////////////////////////


MtdFloatFile MtdFloatFile::convolve(const int R) const

{

int j, n;
int x, y, t;
double value;
MtdFloatFile out;
double min_conv_value, max_conv_value;
double * conv_data = (double *) 0;
DataHandle handle;
unixtime time_start, time_stop;
double * p = 0;
double * s_below = 0;
double * s_this  = 0;
double * s_above = 0;
const int trp1 = 2*R + 1;
const double scale = 1.0/(3*trp1*trp1);



conv_radius = R;


cerr << "\n\n"
     << "    MtdFloatFile::convolve(const int) const -> still doesn't allow for bad data!\n\n"
     << "\n\n";

min_conv_value = 0.0;
max_conv_value = 0.0;

const int Nxy = Nx*Ny;

sum_plane_buf = new double [Nxy];

handle.set_size(Nx, Ny);

conv_data = new double [Nx*Ny*Nt];

if ( !conv_data )  {

   cerr << "\n\n  MtdFloatFile::convolve(const int) const: process() -> memory allocation error\n\n";

   exit ( 1 );

}

   //
   //  get the min/max convolved data values
   //

min_conv_value =  1.0e100;
max_conv_value = -1.0e100;

time_start = time(0);

for (t=0; t<Nt; ++t)  {

   n = mtd_three_to_one(Nx, Ny, Nt, 0, 0, t);

   p = conv_data + n;

   load_handle(handle, *this, t);

   s_below = handle.sum_plane_below;
   s_this  = handle.sum_plane_this;
   s_above = handle.sum_plane_above;

   if ( verbose )  {

      // cout << "T = " << t << '\n';

      // cout << "s_below range is " << dump_range(s_below, Nxy) << '\n';
      // cout << "s_this  range is " << dump_range(s_this,  Nxy) << '\n';
      // cout << "s_above range is " << dump_range(s_above, Nxy) << '\n';

      // cout << '\n';

      // cout << "data_below range is " << dump_range(handle.data_plane_below, Nxy) << '\n';
      // cout << "data_this  range is " << dump_range(handle.data_plane_this,  Nxy) << '\n';
      // cout << "data_above range is " << dump_range(handle.data_plane_above, Nxy) << '\n';

      // cout << "\n\n" << flush;

   }

   for (j=0; j<Nxy; ++j)  {

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
for (x=0; x<Nx; ++x)  {

   // if ( verbose && ((x%100) == 0) )  cout << "Pass 1: x = " << x << " of " << Nx << "\n" << flush;

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         value = calc_conv_value(in, x, y, t);

         n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

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


out.set_size(Nx, Ny, Nt);

out.set_grid(*G);

out.set_start_time(StartTime);

out.set_delta_t(DeltaT);

out.set_data_minmax(min_conv_value, max_conv_value);

out.set_filetype(mtd_file_conv);

out.set_radius(R);


   //
   //  calculate the data values
   //


for (x=0; x<Nx; ++x)  {

   // if ( verbose && ((x%100) == 0) )  cout << "Pass 2: x = " << x << " of " << Nx << "\n" << flush;

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

         // value = calc_conv_value(in, x, y, t);
         value = conv_data[n];

         out.put((float) value, x, y, t);

      }   //  for t

   }   //  for y

}   //  for x


   //
   //  done
   //

if ( conv_data )  { delete [] conv_data;  conv_data = (double *) 0; }

if ( sum_plane_buf )  { delete sum_plane_buf;  sum_plane_buf = 0; }

return ( out );

}


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

ok_plane_below = new bool [nxy];
ok_plane_this  = new bool [nxy];
ok_plane_above = new bool [nxy];

for (j=0; j<nxy; ++j)  {

   data_plane_below[j] = 0.0;

   ok_plane_below[j] = true;

}

memcpy(data_plane_this,  data_plane_below, nxy*sizeof(double));
memcpy(data_plane_above, data_plane_below, nxy*sizeof(double));

memcpy(sum_plane_below, data_plane_below, nxy*sizeof(double));
memcpy(sum_plane_this,  data_plane_below, nxy*sizeof(double));
memcpy(sum_plane_above, data_plane_below, nxy*sizeof(double));

memcpy(ok_plane_this,  ok_plane_below, nxy*sizeof(bool));
memcpy(ok_plane_above, ok_plane_below, nxy*sizeof(bool));


return;

}


////////////////////////////////////////////////////////////////////////


void get_data_plane(const MtdFloatFile & mtd, const int t, double * data_plane)

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

      *d++ = (float) value;

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
static int count = 0;
char junk[256];
double moving_sum = 0.0;
const int nxy = nx*ny;
const int xmin = conv_radius;
const int ymin = conv_radius;
const int xmax = nx - 1 - conv_radius;
const int ymax = ny - 1 - conv_radius;
const double * front = 0;
const double * back  = 0;
const double * in    = 0;
      double * out   = 0;
      double * put   = 0;

// cout << "in calc_sum_plane\n" << flush;

sprintf(junk, "raw_%02d", count);

// data_pgm(data_plane, nx, ny, junk);

   //
   //  zero out the sum plane buffer
   //

memset(sum_plane_buf, 0, nxy*sizeof(double));

   //
   //  calculate sums in x-direction for each y
   //

in = data_plane;

out = sum_plane_buf;


for (y=0; y<ny; ++y)  {

   moving_sum = 0.0;

   n = mtd_three_to_one(nx, ny, 1, 0, y, 0);

   back = in + n;

   for (x=0; x<=(2*conv_radius); ++x)  moving_sum += back[x];

   front = back + (2*conv_radius + 1);

   put = out + (n + conv_radius);

   for (x=xmin; x<xmax; ++x)  {

      *put = moving_sum;

      moving_sum += *front;

      moving_sum -= *back;

      ++front;
      ++back;
      ++put;

   }

}


// cout << "buf plane range = " << dump_range(sum_plane_buf, nxy) << '\n' << flush;
// 
// sprintf(junk, "buf_%02d", count);
// 
// data_pgm(sum_plane_buf, nx, ny, junk);


   //
   //  calculate sums in y-direction for each x
   //

in = sum_plane_buf;

out = sum_plane;

for (x=0; x<nx; ++x)  {

   moving_sum = 0.0;

   n = mtd_three_to_one(nx, ny, 1, x, 0, 0);

   back = in + n;

   for (y=0; y<=(2*conv_radius); ++y)  moving_sum += back[y*nx];

   front = back + (2*conv_radius + 1)*nx;

   put = out + (n + conv_radius*nx);

   for (y=ymin; y<ymax; ++y)  {

      *put = moving_sum;

      moving_sum += *front;

      moving_sum -= *back;

/*
      if ( moving_sum < 0 )  {

         cout << "moving_sum = " << moving_sum << " ... (x, y) = (" << x << ", " << y << ")\n";

      }
*/

      front += nx;
      back  += nx;
      put   += nx;

   }

}   //  for x

// cout << "sum plane range = " << dump_range(out, nxy) << '\n' << flush;
// 
// sprintf(junk, "sum_%02d", count);
// 
// data_pgm(out, nx, ny, junk);

   //
   //  done
   //

++count;

return;

}


////////////////////////////////////////////////////////////////////////


void load_handle(DataHandle & handle, const MtdFloatFile & in, const int t)

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

   // get_data_plane(in, t, handle.data_plane_above);
   get_data_plane(in, t + 1, handle.data_plane_above);

   calc_sum_plane(in.nx(), in.ny(), handle.data_plane_above, handle.sum_plane_above);

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

/*
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
*/

////////////////////////////////////////////////////////////////////////





