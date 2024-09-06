// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cstdio>
#include <cmath>

#include <netcdf>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_pxm.h"

#include "mtd_file.h"
#include "mtd_nc_defs.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////


static const bool verbose = false;

static const bool do_ppms = false;

static int spatial_conv_radius = -1;

static double *     sum_plane_buf = nullptr;
static bool   *  ok_sum_plane_buf = nullptr;


////////////////////////////////////////////////////////////////////////


struct DataHandle {

   int nx, ny;

   int time_radius;

   int t;


      ///////////////

   int n_planes () const

   {

      int k;

      k = 0;

      for (int j=0; j<time_radius; ++j)  {

         if ( plane_loaded[j] )   ++k;

      }

      return k;

   }

      ///////////////


   double ** data_plane;
   double ** sum_plane;

   bool **   ok_plane;
   bool **   ok_sum_plane;


   bool * plane_loaded;

   int  * plane_time;


   void set_size(int _nx, int _ny, int _time_radius);

   DataHandle() {

      nx = ny = 0;

      time_radius = 0;

      t = -1;

          data_plane = nullptr;

           sum_plane = nullptr;

            ok_plane = nullptr;

        ok_sum_plane = nullptr;

        plane_loaded = nullptr;

        plane_time   = nullptr;

   }

  ~DataHandle() { clear(); }

   void clear() {

      nx = ny = 0;

      if ( ! data_plane )  return;

         //////////////////

      int j;

      for (j=0; j<time_radius; ++j)  {

         if (   data_plane[j] )  { delete []   data_plane[j];    data_plane[j] = nullptr; }
         if (    sum_plane[j] )  { delete []    sum_plane[j];     sum_plane[j] = nullptr; }
         if (     ok_plane[j] )  { delete []     ok_plane[j];      ok_plane[j] = nullptr; }
         if ( ok_sum_plane[j] )  { delete [] ok_sum_plane[j];  ok_sum_plane[j] = nullptr; }

      }

      delete []   data_plane;    data_plane = nullptr;
      delete []    sum_plane;     sum_plane = nullptr;
      delete []     ok_plane;      ok_plane = nullptr;
      delete [] ok_sum_plane;  ok_sum_plane = nullptr;

      delete [] plane_loaded;  plane_loaded = nullptr;

      delete [] plane_time;    plane_time   = nullptr;

      return;

   }


   void dump(ostream & _out) const

   {

      _out.put('\n');

      _out << "   DataHandle:\n";

      _out << "   nx = " << nx << '\n';
      _out << "   ny = " << ny << '\n';
      _out << "   time_radius = " << time_radius << '\n';
      _out << "   t = " << t << '\n';
      _out << "   n_planes = " << (n_planes()) << '\n';

      _out << "   plane_loaded = [";

      for (int j=0; j<time_radius; ++j)  _out << ' ' << bool_to_string(plane_loaded[j]);

      _out << " ]\n";

      _out << "   plane_time = [";

      for (int j=0; j<time_radius; ++j)  {

         _out << ' ';

         if ( plane_time[j] < 0 )  _out << '.';
         else                      _out << plane_time[j];

      }

      _out << " ]\n";

      _out.flush();

      return;

   }



};


////////////////////////////////////////////////////////////////////////


static void get_data_plane(const MtdFloatFile &, const int t, double * data_plane, bool * ok_plane);

static void calc_sum_plane(const int nx, const int ny,
                           const double * data_plane_in, const bool * ok_plane_in,
                           double * sum_plane_out,       bool * ok_sum_plane_out);

static void load_handle(DataHandle &, const MtdFloatFile & in, const int t, const int time_beg, const int time_end);


static void set_false_plane(bool *, const int n);

static void data_handle_ppm(const double * data_plane, const int nx, const int ny, const char * filename);
static void   ok_handle_ppm(const bool * ok_plane, const int nx, const int ny, const char * filename);


////////////////////////////////////////////////////////////////////////


MtdFloatFile MtdFloatFile::convolve(const int spatial_R, const int time_beg, const int time_end) const

{

int n;
int n_good;
double value;
MtdFloatFile out;
double min_conv_value;
double max_conv_value;
auto conv_data = (double *) nullptr;
DataHandle handle;

const int time_radius = time_end - time_beg + 1;

double * p = nullptr;
double * ss [time_radius];
bool   * ok [time_radius];

const int trp1 = 2*spatial_R + 1;
const double scale = 1.0/(trp1*trp1);


file_id = 1;   //  This is declared static in the netCDF library header file ncGroup.h,
               //  so we have to do **something** with this or the compiler complains
               //  about an unused variable

spatial_conv_radius = spatial_R;


// mlog << Error << "\n\n"
//      << "    MtdFloatFile::convolve(const int) const -> still doesn't allow for bad data!\n\n"
//      << "\n\n";


const int Nxy = Nx*Ny;
const int Nxyz = Nx*Ny*Nt;

   sum_plane_buf = new double [Nxy];
ok_sum_plane_buf = new bool   [Nxy];

handle.set_size(Nx, Ny, time_radius);

conv_data = new double [Nxyz];
for (int k=0; k<Nxyz; k++) conv_data[k] = bad_data_double;

if ( !conv_data )  {

   mlog << Error << "\n\n  MtdFloatFile::convolve(const int, const int, const int) const: process() -> memory allocation error\n\n";

   exit ( 1 );

}

   //
   //  get the min/max convolved data values
   //

min_conv_value =  1.0e100;
max_conv_value = -1.0e100;

unixtime time_start = time(nullptr);

// cout << "\n\n  n = " << mtd_three_to_one(Nx, Ny, Nt, 88, 397, 0) << "\n\n";

for (int t=0; t<Nt; ++t)  {

   n = mtd_three_to_one(Nx, Ny, Nt, 0, 0, t);

   p = conv_data + n;

   load_handle(handle, *this, t, time_beg, time_end);

   // handle.dump(cout);

   for (int k=0; k<time_radius; ++k)  {

      ss[k] = handle.sum_plane[k];

      ok[k] = handle.ok_sum_plane[k];

   }

      //
      //   the order of loops is important here
      //

   for (int j=0; j<Nxy; ++j)  {

      // if ( (t == 0) && (j == 243846) )  {
      //    cerr << "ok\n";
      // }

      bool has_good_data = false;

      for (int k=0; k<time_radius; ++k)  {

         if ( handle.plane_loaded[k] && *(ok[k]) ) {
            has_good_data = true;
            break;
         }

      }

      if ( !has_good_data ) value = bad_data_double;
      else {

         value = 0.0;

         n_good = 0;

         for (int k=0; k<time_radius; ++k)  {
            if ( handle.plane_loaded[k] && *(ok[k]) )  { value += (*ss[k]);  ++n_good; }
         }

         if (n_good >0) {
            value /= n_good;

            value *= scale;

            if ( value < min_conv_value )  min_conv_value = value;
            if ( value > max_conv_value )  max_conv_value = value;
         }

      }

      if ((n + j) < Nxyz) *p++ = value;

      for (int k=0; k<time_radius; ++k)  {

         ++(ss[k]);
         ++(ok[k]);

      }   //  for k

   }   //  for j

}   //  for t


mlog << Debug(5) << "Conv data range is " << min_conv_value << " to " << max_conv_value << "\n\n";


if ( verbose )  {
   unixtime time_stop = time(nullptr);

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

out.set_spatial_radius(spatial_R);

out.set_time_window(time_beg, time_end);

for (int j=0; j<Nt; ++j)  {

   out.set_lead_time(j, lead_time(j));

}


   //
   //  calculate the data values
   //


for (int x=0; x<Nx; ++x)  {

   // if ( verbose && ((x%100) == 0) )  mlog << Debug(5) << "Pass 2: x = " << x << " of " << Nx << "\n";

   for (int y=0; y<Ny; ++y)  {

      for (int t=0; t<Nt; ++t)  {

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

if ( conv_data )  { delete [] conv_data;  conv_data = (double *) nullptr; }

if (    sum_plane_buf )  { delete []    sum_plane_buf;     sum_plane_buf = nullptr; }
if ( ok_sum_plane_buf )  { delete [] ok_sum_plane_buf;  ok_sum_plane_buf = nullptr; }

return out;

}


////////////////////////////////////////////////////////////////////////


void DataHandle::set_size(int _nx, int _ny, int _time_radius)

{

if ( (_nx <= 0) || (_ny <= 0) )  {

   mlog << Error << "\n\n  DataHandle::set_size() -> bad size\n\n";

   exit ( 1 );

}

clear();

nx = _nx;
ny = _ny;

time_radius = _time_radius;

t = -1;

int j;
const int nxy = nx*ny;

  data_plane = new double * [time_radius];
   sum_plane = new double * [time_radius];

    ok_plane = new bool   * [time_radius];
ok_sum_plane = new bool   * [time_radius];

for (j=0; j<time_radius; ++j)  {

     data_plane[j] = new double [nxy];
      sum_plane[j] = new double [nxy];

       ok_plane[j] = new bool   [nxy];
   ok_sum_plane[j] = new bool   [nxy];

}   //  for j

plane_loaded = new bool [time_radius];

for (j=0; j<time_radius; ++j)  plane_loaded[j] = false;

plane_time = new int [time_radius];

for (j=0; j<time_radius; ++j)  plane_time[j] = -1;

   //
   //  initialize planes
   //

double * dd = data_plane[0];
bool   * bb = ok_plane[0];

for (j=0; j<nxy; ++j)  {

   dd[j] = 0.0;

   bb[j] = false;

}

for (j=1; j<time_radius; ++j)  {   //  j starts at one here, not zero

   memcpy(  data_plane[j], dd, nxy*sizeof(double));
   memcpy(   sum_plane[j], dd, nxy*sizeof(double));

   memcpy(    ok_plane[j], bb, nxy*sizeof(bool));
   memcpy(ok_sum_plane[j], bb, nxy*sizeof(bool));

}


return;

}


////////////////////////////////////////////////////////////////////////


void get_data_plane(const MtdFloatFile & mtd, const int t, double * data_plane, bool * ok_plane)

{

int x, y;
int offset = 0;
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

      if (offset < (nx * ny)) { // kludge for SonarQube findings
         *(d+offset) = (float) value;

         *(ok+offset) = ! status;
      }

      offset++;

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

if ( spatial_conv_radius == 0 )  {

   memcpy(   sum_plane, data_plane, nx*ny*sizeof(double));
   memcpy(ok_sum_plane,   ok_plane, nx*ny*sizeof(bool));

   return;

}

int j, x, y, n;
int bad_count;
static int t_count = 0;
char junk[256];
double value, value_front, value_back;
bool   ok, ok_front, ok_back;
double moving_sum;
bool * b                 = nullptr;
const bool * ok_back_p   = nullptr;
const bool * ok_front_p  = nullptr;
const bool * ok_in_p     = nullptr;
      bool * ok_out_p    = nullptr;
      bool * ok_put_p    = nullptr;
const int two_r        = 2*spatial_conv_radius;
const int two_r_plus_1 = 2*spatial_conv_radius + 1;
const int nxy          = nx*ny;
const int x_min        = spatial_conv_radius;
const int y_min        = spatial_conv_radius;
const int x_max        = nx - 1 - spatial_conv_radius;
const int y_max        = ny - 1 - spatial_conv_radius;
const double * data_front_p   = nullptr;
const double * data_back_p    = nullptr;
const double * data_in_p      = nullptr;
      double * data_out_p     = nullptr;
      double * data_put_p     = nullptr;


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

     data_put_p =  data_out_p + (n + spatial_conv_radius);
       ok_put_p =    ok_out_p + (n + spatial_conv_radius);

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

      if ( ok )  moving_sum += value;
      else       ++bad_count;

   }

   data_front_p = data_back_p + two_r_plus_1*nx;
     ok_front_p =   ok_back_p + two_r_plus_1*nx;

   data_put_p = data_out_p + (n + spatial_conv_radius*nx);
     ok_put_p =   ok_out_p + (n + spatial_conv_radius*nx);

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


void load_handle(DataHandle & handle, const MtdFloatFile & in, const int t, const int time_beg, const int time_end)

{

int j,  index, t_real;
const int t_first = 0;
const int t_last  = (in.nt() - 1);
const int data_bytes = (in.nxy())*sizeof(double);
const int   tf_bytes = (in.nxy())*sizeof(bool);
const int nxy = in.nxy();
const int time_radius = time_end - time_beg + 1;
bool new_loaded[time_radius];


handle.t = t;

for (index=0; index<time_radius; ++index)  new_loaded[index] = false;

for (index=0; index<time_radius; ++index)  handle.plane_time[index] = -1;


for (index=0; index<time_radius; ++index)  {

   t_real = index + t + time_beg;

   if ( t_real < t_first )  continue;
   if ( t_real > t_last  )  break;

   new_loaded[index] = true;

   handle.plane_time[index] = t_real;

      //

   if ( ((index + 1) < time_radius) && (handle.plane_loaded[index + 1]) )  {

      memcpy(handle.data_plane   [index], handle.data_plane   [index + 1], data_bytes);
      memcpy(handle.sum_plane    [index], handle.sum_plane    [index + 1], data_bytes);

      memcpy(handle.ok_plane     [index], handle.ok_plane     [index + 1], tf_bytes);
      memcpy(handle.ok_sum_plane [index], handle.ok_sum_plane [index + 1], tf_bytes);

      continue;

   }

      //   nope

   get_data_plane(in, t_real, handle.data_plane[index],  handle.ok_plane[index]);

   calc_sum_plane(in.nx(), in.ny(), handle.data_plane[index], handle.ok_plane[index], handle.sum_plane[index], handle.ok_sum_plane[index]);


}   //  for index


for (index=0; index<time_radius; ++index)  {

   if ( ! new_loaded[index] )  {

      set_false_plane(handle.ok_plane     [index], nxy);
      set_false_plane(handle.ok_sum_plane [index], nxy);

   }

}



for (j=0; j<time_radius; ++j)  handle.plane_loaded[j] = new_loaded[j];

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void set_false_plane(bool * b, const int n)

{

for (int j=0; j<n; ++j)  b[j] = false;


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

int n;
Ppm image;
bool tf = false;
Color c;
const Color white (255, 255, 255);
const Color black (  0,   0,   0);


image.set_size_xy(nx, ny);

for (int x=0; x<nx; ++x)  {

   for (int y=0; y<ny; ++y)  {

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






