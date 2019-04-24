// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


static const bool verbose = false; 

static const bool do_ppms = false;


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
#include "vx_pxm.h"

#include "mtd_file.h"
#include "mtd_nc_defs.h"


////////////////////////////////////////////////////////////////////////


static int conv_radius = -1;

static double *     sum_plane_buf = 0;
static bool   *  ok_sum_plane_buf = 0;


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

   bool * ok_sum_plane_below;
   bool * ok_sum_plane_this;
   bool * ok_sum_plane_above;

   void set_size(int _nx, int _ny);

   DataHandle() {

      nx = ny = 0;

      data_plane_below = data_plane_this = data_plane_above = 0;

       sum_plane_below =  sum_plane_this =  sum_plane_above = 0;

        ok_plane_below =   ok_plane_this =   ok_plane_above = 0;

        ok_sum_plane_below =   ok_sum_plane_this =   ok_sum_plane_above = 0;

   }

  ~DataHandle() { clear(); }

   void clear() {

      if ( data_plane_below ) { delete data_plane_below;  data_plane_below = 0; }
      if ( data_plane_this  ) { delete data_plane_this ;  data_plane_this  = 0; }
      if ( data_plane_above ) { delete data_plane_above;  data_plane_above = 0; }

      if (  sum_plane_below ) { delete  sum_plane_below;   sum_plane_below = 0; }
      if (  sum_plane_this  ) { delete  sum_plane_this ;   sum_plane_this  = 0; }
      if (  sum_plane_above ) { delete  sum_plane_above;   sum_plane_above = 0; }

      if (   ok_plane_below ) { delete   ok_plane_below;    ok_plane_below = 0; }
      if (   ok_plane_this  ) { delete   ok_plane_this ;    ok_plane_this  = 0; }
      if (   ok_plane_above ) { delete   ok_plane_above;    ok_plane_above = 0; }

      if (   ok_sum_plane_below ) { delete   ok_sum_plane_below;    ok_sum_plane_below = 0; }
      if (   ok_sum_plane_this  ) { delete   ok_sum_plane_this ;    ok_sum_plane_this  = 0; }
      if (   ok_sum_plane_above ) { delete   ok_sum_plane_above;    ok_sum_plane_above = 0; }

      return;

   }

};


////////////////////////////////////////////////////////////////////////


static void get_data_plane(const MtdFloatFile &, const int t, double * data_plane, bool * ok_plane);

static void calc_sum_plane(const int nx, const int ny, 
                           const double * data_plane, const bool * ok_plane, 
                                 double * sum_plane,        bool * ok_sum_plane);

static void load_handle(DataHandle &, const MtdFloatFile & in, const int t);

static void data_handle_ppm(const double * data_plane, const int nx, const int ny, const char * filename);
static void   ok_handle_ppm(const bool * ok_plane, const int nx, const int ny, const char * filename);


////////////////////////////////////////////////////////////////////////


MtdFloatFile MtdFloatFile::convolve(const int R) const

{

int j, n;
int x, y, t;
double value;
bool ok = false;
MtdFloatFile out;
double min_conv_value, max_conv_value;
double * conv_data = (double *) 0;
double scale;
DataHandle handle;
unixtime time_start, time_stop;
double * p = 0;
const double * s_below = 0;
const double * s_this  = 0;
const double * s_above = 0;

const bool * ok_below = 0;
const bool * ok_this  = 0;
const bool * ok_above = 0;

const int trp1 = 2*R + 1;
const double mid_scale = 1.0/(3*trp1*trp1);
const double be_scale  = 1.0/(2*trp1*trp1);



conv_radius = R;


// mlog << Error << "\n\n"
//      << "    MtdFloatFile::convolve(const int) const -> still doesn't allow for bad data!\n\n"
//      << "\n\n";

min_conv_value = 0.0;
max_conv_value = 0.0;

const int Nxy = Nx*Ny;

   sum_plane_buf = new double [Nxy];
ok_sum_plane_buf = new bool   [Nxy];

handle.set_size(Nx, Ny);

conv_data = new double [Nx*Ny*Nt];

if ( !conv_data )  {

   mlog << Error << "\n\n  MtdFloatFile::convolve(const int) const: process() -> memory allocation error\n\n";

   exit ( 1 );

}

   //
   //  get the min/max convolved data values
   //

min_conv_value =  1.0e100;
max_conv_value = -1.0e100;

time_start = time(0);

for (t=0; t<Nt; ++t)  {

   if ( (t == 0) || (t == (Nt - 1)) )  scale = be_scale;
   else                                scale = mid_scale;

   n = mtd_three_to_one(Nx, Ny, Nt, 0, 0, t);

   p = conv_data + n;

   load_handle(handle, *this, t);

   s_below = handle.sum_plane_below;
   s_this  = handle.sum_plane_this;
   s_above = handle.sum_plane_above;

   ok_below = handle.ok_sum_plane_below;
   ok_this  = handle.ok_sum_plane_this;
   ok_above = handle.ok_sum_plane_above;

   // if ( t == 0 )  data_handle_ppm(handle.sum_plane_this, handle.nx, handle.ny, "sum_00.ppm");

      // 
      //   the order of loops is important, here
      // 

/*
   for (y=0; y<Ny; ++y)  {

      for (x=0; x<Nx; ++x)  {

      }   //  for x

   }   //  for y
*/

   for (j=0; j<Nxy; ++j)  {

      ok = (*ok_below) || (*ok_this) || (*ok_above);

      if ( !ok )  value = bad_data_double;
      else {
/*
         if ( j == 250000 )  {

            cout << "Hello\n";

            cout << "ok_below = " << (*ok_below) << '\n';
            cout << "ok_this  = " << (*ok_this)  << '\n';
            cout << "ok_above = " << (*ok_above) << '\n';

            cout << '\n';

            cout << "s_below  = " << (*s_below) << '\n';
            cout << "s_this   = " << (*s_this)  << '\n';
            cout << "s_above  = " << (*s_above) << '\n';

            cout << '\n';

         }
*/
         value = 0.0;

         if ( *ok_below )  value += (*s_below);
         if ( *ok_this  )  value += (*s_this);
         if ( *ok_above )  value += (*s_above);

         value *= scale;

         if ( value < min_conv_value )  min_conv_value = value;
         if ( value > max_conv_value )  max_conv_value = value;

      }

      *p++ = value;

      ++s_below;
      ++s_this;
      ++s_above;

      ++ok_below;
      ++ok_this;
      ++ok_above;

   }

}

time_stop = time(0);


mlog << Debug(5) << "Conv data range is " << min_conv_value << " to " << max_conv_value << "\n\n";


if ( verbose )  {

   mlog << Debug(5) << "\n  Conv time       = " << (time_stop - time_start) << " seconds\n";

}

   //
   //  setup the output file
   //


out.set_size(Nx, Ny, Nt);

out.set_grid(*G);

out.set_start_valid_time(StartValidTime);

out.set_delta_t(DeltaT);

out.set_data_minmax(min_conv_value, max_conv_value);

out.set_filetype(mtd_file_conv);

out.set_radius(R);

for (j=0; j<Nt; ++j)  {

   out.set_lead_time(j, lead_time(j));

}


   //
   //  calculate the data values
   //


for (x=0; x<Nx; ++x)  {

   // if ( verbose && ((x%100) == 0) )  mlog << Debug(5) << "Pass 2: x = " << x << " of " << Nx << "\n";

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

if (    sum_plane_buf )  { delete []    sum_plane_buf;     sum_plane_buf = 0; }
if ( ok_sum_plane_buf )  { delete [] ok_sum_plane_buf;  ok_sum_plane_buf = 0; }

return ( out );

}


////////////////////////////////////////////////////////////////////////


void DataHandle::set_size(int _nx, int _ny)

{

if ( (_nx <= 0) || (_ny <= 0) )  {

   mlog << Error << "\n\n  DataHandle::set_size() -> bad size\n\n";

   exit ( 1 );

}

clear();

nx = _nx;
ny = _ny;

int j;
const int nxy = nx*ny;

data_plane_below   = new double [nxy];
data_plane_this    = new double [nxy];
data_plane_above   = new double [nxy];

sum_plane_below    = new double [nxy];
sum_plane_this     = new double [nxy];
sum_plane_above    = new double [nxy];

ok_plane_below     = new bool [nxy];
ok_plane_this      = new bool [nxy];
ok_plane_above     = new bool [nxy];

ok_sum_plane_below = new bool [nxy];
ok_sum_plane_this  = new bool [nxy];
ok_sum_plane_above = new bool [nxy];

for (j=0; j<nxy; ++j)  {

   data_plane_below[j] = 0.0;

     ok_plane_below[j] = false;

}

memcpy(data_plane_this,  data_plane_below, nxy*sizeof(double));
memcpy(data_plane_above, data_plane_below, nxy*sizeof(double));

memcpy(sum_plane_below, data_plane_below, nxy*sizeof(double));
memcpy(sum_plane_this,  data_plane_below, nxy*sizeof(double));
memcpy(sum_plane_above, data_plane_below, nxy*sizeof(double));

memcpy(ok_plane_this,  ok_plane_below, nxy*sizeof(bool));
memcpy(ok_plane_above, ok_plane_below, nxy*sizeof(bool));

memcpy(ok_sum_plane_this,  ok_plane_below, nxy*sizeof(bool));
memcpy(ok_sum_plane_above, ok_plane_below, nxy*sizeof(bool));


return;

}


////////////////////////////////////////////////////////////////////////


void get_data_plane(const MtdFloatFile & mtd, const int t, double * data_plane, bool * ok_plane)

{

int x, y;
double * d = data_plane;
bool * ok = ok_plane;
double value;
bool status = false;
const int nx = mtd.nx();
const int ny = mtd.ny();


// mlog << Debug(5) << "In get_data_plane\n";

for (y=0; y<ny; ++y)  {

   for (x=0; x<nx; ++x)  {

      value = mtd(x, y, t);

      status = is_bad_data(value);

      *d++ = (float) value;

      *ok++ = ! status;

   }

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void calc_sum_plane(const int nx, const int ny, 
                    const double * data_plane, const bool * ok_plane, 
                          double * sum_plane,        bool * ok_sum_plane)

{

if ( conv_radius == 0 )  {

   memcpy(   sum_plane, data_plane, nx*ny*sizeof(double));
   memcpy(ok_sum_plane,   ok_plane, nx*ny*sizeof(bool));

   return;

}

int j, x, y, n;
int bad_count;
static int t_count = 0;
char junk[256];
double value, value_front, value_back;
bool      ok, ok_front, ok_back;
double moving_sum      = 0.0;
bool * b               = 0;
const bool * ok_back_p   = 0;
const bool * ok_front_p  = 0;
const bool * ok_in_p     = 0;
      bool * ok_out_p    = 0;
      bool * ok_put_p    = 0;
const int two_r        = 2*conv_radius;
const int two_r_plus_1 = 2*conv_radius + 1;
const int nxy          = nx*ny;
const int x_min        = conv_radius;
const int y_min        = conv_radius;
const int x_max        = nx - 1 - conv_radius;
const int y_max        = ny - 1 - conv_radius;
const double * data_front_p   = 0;
const double * data_back_p    = 0;
const double * data_in_p      = 0;
      double * data_out_p     = 0;
      double * data_put_p     = 0;


// mlog << Debug(5) << "in calc_sum_plane\n";

// snprintf(junk, sizeof(junk), "raw_%02d", t_count);

// data_pgm(data_plane, nx, ny, junk);

   //
   //  zero out the sum plane buffer
   //

memset(sum_plane_buf, 0, nxy*sizeof(double));

b = ok_sum_plane_buf;

for (j=0; j<nxy; ++j)  *b++ = true;

   //
   //  calculate sums in x-direction for each y
   //

data_in_p  = data_plane;
  ok_in_p  = ok_plane;

data_out_p =    sum_plane_buf;
  ok_out_p = ok_sum_plane_buf;


for (y=0; y<ny; ++y)  {

   moving_sum = 0.0;

   bad_count = 0;

   n = mtd_three_to_one(nx, ny, 1, 0, y, 0);

   data_back_p = data_in_p + n;
     ok_back_p =   ok_in_p + n;

   for (x=0; x<=two_r; ++x)  {

      value = data_back_p[x];
         ok =   ok_back_p[x];

      if ( ok )  moving_sum += value;
      else       ++bad_count;

   }

   data_front_p = data_back_p + two_r_plus_1;
     ok_front_p =   ok_back_p + two_r_plus_1;

     data_put_p =  data_out_p + (n + conv_radius);
       ok_put_p =    ok_out_p + (n + conv_radius);

   for (x=x_min; x<x_max; ++x)  {

      value_front = *data_front_p;
      value_back  = *data_back_p;

         ok_front = *ok_front_p;
          ok_back = *ok_back_p;

      if ( ! ok_front )  ++bad_count;

      if ( bad_count > 0 )  {

         *data_put_p = bad_data_double;
           *ok_put_p = false;

      } else {

         *data_put_p = moving_sum;
           *ok_put_p = true;

      }

      if ( ok_front )  moving_sum += value_front;

      if ( ok_back )  moving_sum -= value_back;
      else            --bad_count;

      ++data_front_p;
      ++data_back_p;
      ++data_put_p;

      ++ok_front_p;
      ++ok_back_p;
      ++ok_put_p;

   }   //  for x

}   //  for y

// memcpy(   sum_plane,    sum_plane_buf, nxy*sizeof(double));
// memcpy(ok_sum_plane, ok_sum_plane_buf, nxy*sizeof(bool));

if ( do_ppms )  {

   snprintf(junk, sizeof(junk), "sum_a_%02d.ppm", t_count);

   data_handle_ppm(data_out_p, nx, ny, junk);

   snprintf(junk, sizeof(junk), "ok_a_%02d.ppm", t_count);

   ok_handle_ppm(ok_out_p, nx, ny, junk);

}

   //
   //  calculate sums in y-direction for each x
   //

data_in_p =    sum_plane_buf;
  ok_in_p = ok_sum_plane_buf;

data_out_p =    sum_plane;
  ok_out_p = ok_sum_plane;

for (x=0; x<nx; ++x)  {

   moving_sum = 0.0;

   bad_count = 0;

   n = mtd_three_to_one(nx, ny, 1, x, 0, 0);

   data_back_p = data_in_p + n;
     ok_back_p =   ok_in_p + n;

   for (y=0; y<=two_r; ++y)  {

      value = data_back_p[y*nx];
         ok =   ok_back_p[y*nx];

      if ( ok )  moving_sum += data_back_p[y*nx];
      else       ++bad_count;

   }

   data_front_p = data_back_p + two_r_plus_1*nx;
     ok_front_p =   ok_back_p + two_r_plus_1*nx;

   data_put_p = data_out_p + (n + conv_radius*nx);
     ok_put_p =   ok_out_p + (n + conv_radius*nx);

   for (y=y_min; y<y_max; ++y)  {

      value_front = *data_front_p;
      value_back  = *data_back_p;

         ok_front = *ok_front_p;
         ok_back  = *ok_back_p;

      if ( ! ok_front )  ++bad_count;

      if ( bad_count > 0 )  {

         *data_put_p = bad_data_double;
           *ok_put_p = false;

      } else {

         *data_put_p = moving_sum;
           *ok_put_p = true;

      }

      if ( ok_front )  moving_sum += value_front;

      if ( ok_back )  moving_sum -= value_back;
      else            --bad_count;

      data_front_p += nx;
      data_back_p  += nx;
      data_put_p   += nx;

        ok_front_p += nx;
        ok_back_p  += nx;
        ok_put_p   += nx;

   }   //  for y

}   //  for x

if ( do_ppms )  {

   snprintf(junk, sizeof(junk), "sum_b_%02d.ppm", t_count);

   data_handle_ppm(data_out_p, nx, ny, junk);

   snprintf(junk, sizeof(junk), "ok_b_%02d.ppm", t_count);

   ok_handle_ppm(ok_out_p, nx, ny, junk);

}

   //
   //  done
   //

++t_count;

return;

}


////////////////////////////////////////////////////////////////////////


void load_handle(DataHandle & handle, const MtdFloatFile & in, const int t)

{

int j;
const bool first = (t == 0);
const bool last  = (t == (in.nt() - 1));
const int data_bytes = (in.nx())*(in.ny())*sizeof(double);
const int   tf_bytes = (in.nx())*(in.ny())*sizeof(bool);
const int nxy = (in.nx())*(in.ny());


if ( first )  {

   get_data_plane(in, 0, handle.data_plane_this,  handle.ok_plane_this);
   get_data_plane(in, 1, handle.data_plane_above, handle.ok_plane_above);

   calc_sum_plane(in.nx(), in.ny(), handle.data_plane_this,  handle.ok_plane_this, 
                                    handle.sum_plane_this,   handle.ok_sum_plane_this);

   calc_sum_plane(in.nx(), in.ny(), handle.data_plane_above, handle.ok_plane_above, 
                                    handle.sum_plane_above,  handle.ok_sum_plane_above);

   for (j=0; j<nxy; ++j)  handle.ok_plane_below[j] = handle.ok_sum_plane_below[j] = false;

   return;

}

memcpy(handle.data_plane_below, handle.data_plane_this,  data_bytes);
memcpy(handle.data_plane_this,  handle.data_plane_above, data_bytes);

memcpy(handle.ok_plane_below, handle.ok_plane_this,  tf_bytes);
memcpy(handle.ok_plane_this,  handle.ok_plane_above, tf_bytes);

memcpy(handle.sum_plane_below, handle.sum_plane_this,  data_bytes);
memcpy(handle.sum_plane_this,  handle.sum_plane_above, data_bytes);

memcpy(handle.ok_sum_plane_below, handle.ok_sum_plane_this,  tf_bytes);
memcpy(handle.ok_sum_plane_this,  handle.ok_sum_plane_above, tf_bytes);

if ( last )  {

   memset(handle.data_plane_above, 0, data_bytes);

   memset(handle.sum_plane_above,  0, data_bytes);

   for (j=0; j<nxy; ++j)  handle.ok_plane_above[j] = handle.ok_sum_plane_above[j] = false;

} else {

   get_data_plane(in, t + 1, handle.data_plane_above, handle.ok_plane_above);

   calc_sum_plane(in.nx(), in.ny(), handle.data_plane_above, handle.ok_plane_above,
                                    handle.sum_plane_above,  handle.ok_sum_plane_above);

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void data_handle_ppm(const double * data_plane, const int nx, const int ny, const char * filename)

{

int x, y, n, k;
Ppm image;
double value, t;
double min_value, max_value;
Color c;


image.set_size_xy(nx, ny);

   //
   //  get data range
   //

min_value =  1.0e30;
max_value = -1.0e30;

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      n = y*nx + x;

      value = data_plane[n];

      if ( value == bad_data_double )  continue;

      if ( value < min_value )  min_value = value;
      if ( value > max_value )  max_value = value;

   }

}

   //
   //  make image
   //

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      n = y*nx + x;

      value = data_plane[n];

      if ( value == bad_data_double )  c.set_rgb(255, 0, 0);
      else {

         t = (value - min_value)/(max_value - min_value);

         k = nint(255.0*t);

         k = 255 - k;

         c.set_rgb(k, k, k);

      }

      image.putxy(c, x, y);

   }

}

// mlog << Debug(5) << "writing image file \"" << filename << "\"\n";

if ( ! image.write(filename) )  {

   mlog << Error << "\n\n  unable to write image file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ok_handle_ppm(const bool * ok_plane, const int nx, const int ny, const char * filename)

{

int x, y, n;
Ppm image;
bool tf = false;
Color c;
const Color white (255, 255, 255);
const Color black (  0,   0,   0);


image.set_size_xy(nx, ny);

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      n = y*nx + x;

      tf = ok_plane[n];

      if ( tf )  c = white;
      else       c = black;

      image.putxy(c, x, y);

   }

}

// mlog << Debug(5) << "writing image file \"" << filename << "\"\n";

if ( ! image.write(filename) )  {

   mlog << Error << "\n\n  unable to write image file \"" << filename << "\"\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////






