

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "mode_2d_input_file.h"
#include "mtd_file.h"
#include "partition.h"

#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


static void adjust_obj_numbers(ModeTimeDomainFile & s, int delta);

static void find_overlap(const ModeTimeDomainFile & before, const ModeTimeDomainFile & after, Partition &);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeTimeDomainFile
   //


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile::ModeTimeDomainFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile::~ModeTimeDomainFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile::ModeTimeDomainFile(const ModeTimeDomainFile & f)

{

init_from_scratch();

assign(f);

}


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile & ModeTimeDomainFile::operator=(const ModeTimeDomainFile & f)

{

if ( this == &f )  return ( * this );

assign(f);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::init_from_scratch()

{

Data = (MtdDataType *) 0;

G = (Grid *) 0;

Volumes = (int *) 0;

MaxConvIntensity = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::clear()

{

if ( G )  { delete G;  G = (Grid *) 0; }

if ( Data )  { delete [] Data;  Data = (MtdDataType *) 0; }

if ( Volumes )  { delete [] Volumes;  Volumes = (int *) 0; }

if ( MaxConvIntensity )  { delete [] MaxConvIntensity;  MaxConvIntensity = (double *) 0; }


NObjects = 0;

Nx = Ny = Nt = 0;

M = B = 0.0;

StartTime = (Unixtime) 0;

DeltaT = 0;

Radius = 0;

Threshold = -1.0;

Filename.clear();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::assign(const ModeTimeDomainFile & f)

{

clear();

if ( !(f.Data) )  return;

Nx         = f.Nx;
Ny         = f.Ny;
Nt         = f.Nt;

M          = f.M;
B          = f.B;

StartTime  = f.StartTime;

DeltaT     = f.DeltaT;

Radius     = f.Radius;

Threshold  = f.Threshold;

Filename   = f.Filename;


if ( f.G )  set_grid ( *(f.G) );

set_size(f.Nx, f.Ny, f.Nt);

memcpy(Data, f.Data, (Nx*Ny*Nt)*sizeof(MtdDataType));

NObjects = f.NObjects;

if ( f.Volumes )  {

   Volumes = new int [NObjects];

   memcpy(Volumes, f.Volumes, NObjects*sizeof(int));

}


if ( f.MaxConvIntensity )  {

   MaxConvIntensity = new double [NObjects];

   memcpy(MaxConvIntensity, f.MaxConvIntensity, NObjects*sizeof(double));

}




   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);
int month, day, year, hour, minute, second;
double dmin, dmax;
char junk[256];

out << prefix << "Filename          = ";

if ( Filename.nonempty() )  out << '\"' << Filename << "\"\n";
else                        out << "(nul)\n";

unix_to_mdyhms(StartTime, month, day, year, hour, minute, second);

sprintf(junk, "%s %d, %d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);


out << prefix << "StartTime         = " << StartTime << " ... (" << junk << ")\n";
out << prefix << "DeltaT            = " << DeltaT    << '\n';

out << prefix << "Radius            = " << Radius    << '\n';
out << prefix << "Threshold         = " << Threshold << '\n';

out << prefix << "M                 = " << M << " ... (" << (M*mtd_max_ival) << '/' << mtd_max_ival << ")\n";

out << prefix << "B                 = " << B << '\n';

calc_data_minmax(dmin, dmax);

out << prefix << "Calc Data Min/Max = (" << dmin << ", " << dmax << ")\n";

out << prefix << "(Nx, Ny, Nt)      = (" << Nx << ", " << Ny << ", " << Nt << ")\n";

if ( G )  {

   out << prefix << "Grid ...\n";

   G->dump(out, depth + 1);

   // out << prefix << (G->xml_serialize()) << '\n';

} else {

   out << prefix << "Grid              = 0\n";

}




   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_size(int _nx, int _ny, int _nt)

{

Nx = _nx;
Ny = _ny;
Nt = _nt;

if ( (Nx <= 0) || (Ny <= 0) || (Nt <= 0) )  {

   cerr << "\n\n  ModeTimeDomainFile::set_size(int _nx, int _ny, int _nt) -> bad size\n\n";

   exit ( 1 );

}

if ( Data )  { delete Data;  Data = (MtdDataType*) 0; }

Data = new MtdDataType [Nx*Ny*Nt];

memset(Data, 0, (Nx*Ny*Nt)*sizeof(MtdDataType));

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_grid(const Grid & g)

{

if ( G )  { delete G;  G = (Grid *) 0; }

G = new Grid ( g );

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_mb(double _m, double _b)

{

if ( _m <= 0.0 )  {

   cerr << "\n\n  ModeTimeDomainFile::set_mb() -> bad value for M ... " << _m << "\n\n";

   exit ( 1 );

}

M = _m;
B = _b;


return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_radius(int r)

{

// if ( Radius <= 0 )  {
// 
//    cerr << "\n\n  void ModeTimeDomainFile::set_radius(double) -> bad value for radius ... " << r << "\n\n";
// 
//    exit ( 1 );
// 
// }

Radius = r;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_threshold(double t)

{

Threshold = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_start_time(Unixtime t)

{

StartTime = t;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_delta_t(int t)

{

DeltaT = t;

return;

}


////////////////////////////////////////////////////////////////////////

/*
int ModeTimeDomainFile::ival(int x, int y, int t) const

{

int n, k;

n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

k = (int) (Data[n]);

return ( k );

}
*/

////////////////////////////////////////////////////////////////////////


double ModeTimeDomainFile::dval(int x, int y, int t) const

{

int n;
double value;

n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

value = int_to_double(Data[n]);

return ( value );

}


////////////////////////////////////////////////////////////////////////

/*
int ModeTimeDomainFile::three_to_one(int x, int y, int t) const

{

if ( !Data )  {

   cerr << "\n\n  ModeTimeDomainFile::three_to_one(int x, int y, int t) const -> no data!\n\n";

   exit ( 1 );

}

if ( (x < 0) || (x >= Nx) || (y < 0) || (y >= Ny) || (t < 0) || (t >= Nt) )  {

   cerr << "\n\n  ModeTimeDomainFile::three_to_one(int x, int y, int t) const -> range check error!\n\n";

   exit ( 1 );

}

int n = 0;

n = (y*Nx + x)*Nt + t;

return ( n );

}
*/

////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::put_ival(int value, int x, int y, int t)

{

if ( !Data )  {

   cerr << "\n\n  ModeTimeDomainFile::put_ival() -> no data!\n\n";

   exit ( 1 );

}

if ( (value < 0) || (value >= mtd_max_ival) )  {

   cerr << "\n\n  ModeTimeDomainFile::put_ival() -> bad value ... " << value << "\n\n";

   exit ( 1 );

}

const int n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

Data[n] = (MtdDataType) value;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::put_dval(double value, int x, int y, int t)

{

if ( M <= 0.0 )  {

   cerr << "\n\n  ModeTimeDomainFile::put_dval(double value, int x, int y, int t) -> bad M value ... " << M << "\n\n";

   exit ( 1 );

}

int k = nint( (value - B)/M );

if ( (k < 0) || (k > mtd_max_ival) )  {

   cerr << "\n\n  ModeTimeDomainFile::put_dval() -> bad integer value ... " << k << "\n\n";

   exit ( 1 );

}

const int n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

Data[n] = (MtdDataType) k;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::do_threshold(double thresh)

{

if ( !Data )  {

   cerr << "\n\n  ModeTimeDomainFile::do_threshold(double value) -> no data!\n\n";

   exit ( 1 );

}


if ( M <= 0.0 )  {

   cerr << "\n\n  ModeTimeDomainFile::do_threshold(double value) -> bad M value ... " << M << "\n\n";

   exit ( 1 );

}

int j, k;
double value;
const int Nxyt = Nx*Ny*Nt;

for (j=0; j<Nxyt; ++j)  {

   value = int_to_double(Data[j]);

   if ( value >= thresh )  k = 1;
   else                    k = 0;

   Data[j] = (MtdDataType) k;

}

M = 1.0;
B = 0.0;

Threshold = thresh;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Grid ModeTimeDomainFile::grid() const

{

if ( !G )  {

   cerr << "\n\n  ModeTimeDomainFile::grid() const -> no grid!\n\n";

   exit ( 1 );

}


return ( *G );

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::calc_ival_minmax(int & imin, int & imax) const

{

if ( !Data )  {

   cerr << "\n\n  ModeTimeDomainFile::calc_data_minmax() const -> no data!\n\n";

   exit ( 1 );

}

int j, i;
const int n = Nx*Ny*Nt;

imin = imax = (int) (Data[0]);

for (j=0; j<n; ++j)  {

   i = (int) (Data[j]);

   imin = min(imin, i);
   imax = max(imax, i);

}

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::calc_data_minmax(double & dmin, double & dmax) const

{

int imin, imax;

calc_ival_minmax(imin, imax);


dmin = int_to_double(imin);
dmax = int_to_double(imax);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

if ( !G )  {

   cerr << "\n\n  ModeTimeDomainFile::latlon_to_xy() -> no grid!\n\n";

   exit ( 1 );

}

G->latlon_to_xy(lat, lon, x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

if ( !G )  {

   cerr << "\n\n  ModeTimeDomainFile::xy_to_latlon() -> no grid!\n\n";

   exit ( 1 );

}

G->xy_to_latlon(x, y, lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


int ModeTimeDomainFile::volume() const

{

if ( ! Volumes )  return ( 0 );

int j, vol;

vol = 0;

for (j=0; j<NObjects; ++j)  {

   vol += Volumes[j];

}

return ( vol );

}


////////////////////////////////////////////////////////////////////////


int ModeTimeDomainFile::volume(int n) const

{

if ( ! Volumes )  return ( 0 );

if ( (n < 0) || (n >= NObjects) )  {

   cerr << "\n\n  ModeTimeDomainFile::volume(int) -> range check error ... "
        << "NObjects = " << NObjects << " ... "
        << "n = " << n
        << "\n\n";

   exit ( 1 );

}

return ( Volumes[n] );

}


////////////////////////////////////////////////////////////////////////


double ModeTimeDomainFile::max_conv_intensity(int n) const

{

if ( ! MaxConvIntensity )  return ( 0.0 );

if ( (n < 0) || (n >= NObjects) )  {

   cerr << "\n\n  ModeTimeDomainFile::max_conv_intensity(int) -> range check error ... "
        << "NObjects = " << NObjects << " ... "
        << "n = " << n
        << "\n\n";

   exit ( 1 );

}

return ( MaxConvIntensity[n] );

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::centroid(double & xbar, double & ybar, double & tbar) const

{

int x, y, t, n;
int count;
double xsum, ysum, tsum;


xsum = ysum = tsum = 0.0;

count = 0;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

         if ( Data[n] )  {

            xsum += x;
            ysum += y;
            tsum += t;

            ++count;

         }

      }

   }

}

if ( count == 0 )  {

   cerr << "\n\n  ModeTimeDomainFile::centroid() -> field seems to be empty!\n\n";

   exit ( 1 );

}


xbar = xsum/count;
ybar = ysum/count;
tbar = tsum/count;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int ModeTimeDomainFile::area_at_t(const int t) const

{

int x, y, n;
int a;

a = 0;

for (x=0;  x<Nx; ++x)  {

   for (y=0;  y<Ny; ++y)  {

      n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

      if ( Data[n] )  ++a;

   }

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


double ModeTimeDomainFile::average_area() const

{

int j, n, a;
double sum;

sum = 0.0;

n = 0;

for (j=0; j<Nt; ++j)  {

   a = area_at_t(j);

   if ( a != 0 )  { sum += (double) a;  ++n; }

}

if ( n == 0 )  return ( 0.0 );

return ( sum/n );

}


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::xy_centroid(const int t, double & xbar, double & ybar) const

{

int x, y, n;
int count;
double xsum, ysum;


xsum = ysum = 0.0;

count = 0;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

      if ( Data[n] )  {

         xsum += x;
         ysum += y;

         ++count;

      }

   }

}

if ( count == 0 )  {

   // cerr << "\n\n  ModeTimeDomainFile::xy_centroid() -> field seems to be empty!\n\n";
   // 
   // exit ( 1 );

   return ( false );

}

xbar = xsum/count;
ybar = ysum/count;

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::centroid_velocity(const int t, double & xdot, double & ydot) const

{

if ( (t < 0) || (t >= Nt) )  {

   cerr << "\n\n  ModeTimeDomainFile::centroid_velocity() const -> range check error on t\n\n";

   exit ( 1 );

}

double x1, y1, x2, y2;
double dx, dy, dt;


if ( ! xy_centroid(t,     x1, y1) )  return ( false );
if ( ! xy_centroid(t + 1, x2, y2) )  return ( false );


dx = x2 - x1;
dy = y2 - y1;

dt = 1.0;

xdot = dx/dt;
ydot = dy/dt;

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::average_centroid_velocity(double & xdot, double & ydot) const

{

int j, n;
double dx, dy;
bool status = false;

xdot = ydot = 0.0;

n = 0;

for (j=0; j<(NObjects - 1); ++j)  {

   status = centroid_velocity(j, dx, dy);

   if ( ! status )  continue;

   xdot += dx;
   ydot += dy;

   ++n;

}   //  for j


if ( n == 0 )  return ( false );

xdot /= n;
ydot /= n;

return ( true );

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::bbox(int & xmin, int & xmax, int & ymin, int & ymax, int & tmin, int & tmax) const 

{

int x, y, t, n;

xmin = ymin = tmin =  1000000;
xmax = ymax = tmax = -1000000;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

         if ( Data[n] )  {

            if ( x < xmin )  xmin = x;
            if ( x > xmax )  xmax = x;

            if ( y < ymin )  ymin = y;
            if ( y > ymax )  ymax = y;

            if ( t < tmin )  tmin = t;
            if ( t > tmax )  tmax = t;

         }

      }

   }

}

// cout << "Ymin = " << ymin << "\n" << flush;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Unixtime ModeTimeDomainFile::valid_time(int t) const

{

if ( (t < 0) || ( t >= Nt) )  {

   cerr << "\n\n  ModeTimeDomainFile::valid_time(int t) -> range check error\n\n";

   exit ( 1 );

}


return ( StartTime + t*DeltaT );

}


////////////////////////////////////////////////////////////////////////


Moments ModeTimeDomainFile::moments_at_t(const int t) const

{

int x, y, n;
int area;
Moments moments;
double sx, sy, sxx, syy, sxy;


area = 0;

sx = sy = 0.0;

sxx = sxy = syy = 0.0;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

      if ( ! (Data[n]) )  continue;

      ++area;

      sx += x;
      sy += y;

      sxx += x*x;
      sxy += x*y;
      syy += y*y;

   }

}

moments.set_0(area);

moments.set_1(sx, sy);

moments.set_2(sxx, sxy, syy);


return ( moments );

}


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile ModeTimeDomainFile::const_time_slice(int t) const

{

if ( (t < 0) || (t >= Nt) )  {

   cerr << "\n\n  ModeTimeDomainFile::const_time_slice() -> range check error\n\n";

   exit ( 1 );

}

int x, y;
ModeTimeDomainFile f;

f.set_size(Nx, Ny, 1);

f.set_grid(*G);

f.set_mb(M, B);

f.set_start_time(valid_time(t));

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      f.put_ival(ival(x, y, t), x, y, 0);

   }

}



   //
   //  done
   //

return ( f );

}


////////////////////////////////////////////////////////////////////////


int ModeTimeDomainFile::double_to_int(double value) const

{

int k = nint((value - B)/M);

if ( (k < 0) || (k >= mtd_max_ival) )  {

   cerr << "\n\n  ModeTimeDomainFile::double_to_int(double) -> range check error!\n\n";

   exit ( 1 );

}

return ( k );

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_to_zeroes()

{

if ( !Data )  {

   cerr << "\n\n  ModeTimeDomainFile::set_to_zeroes() -> no data field!\n\n";

   exit ( 1 );

}

int j;
const int k = double_to_int(0.0);
const int n = Nx*Ny*Nt;

for (j=0; j<n; ++j)  {

   Data[j] = (MtdDataType) k;

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::zero_border(int n)

{

if ( !Data )  {

   cerr << "\n\n  ModeTimeDomainFile::zero_border(int) -> no data field!\n\n";

   exit ( 1 );

}

if ( 2*n >= min(Nx, Ny) )  {

   cerr << "\n\n  ModeTimeDomainFile::zero_border(int) -> border size too large!\n\n";

   exit ( 1 );

}

int x, y, t;
const int k = double_to_int(0.0);

for (t=0; t<Nt; ++t)  {

   for (x=0; x<Nx; ++x)  {

      for (y=0; y<n; ++y)  {

         put_ival(k, x, y,          t);
         put_ival(k, x, Ny - 1 - k, t);

      }

   }

   for (y=0; y<Ny; ++y)  {

      for (x=0; x<n; ++x)  {

         put_ival(k, x,          y, t);
         put_ival(k, Nx - 1 - x, y, t);

      }

   }

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::s_is_on(int x, int y, int t) const

{

int j;

j = ival(x, y, t);

return ( j > 0 );

}


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::f_is_on(int x, int y, int t) const

{

if ( s_is_on(x, y, t) )  return ( true );

if ( (x > 0) && s_is_on(x - 1, y, t) )  return ( true );

if ( (x > 0) && (y > 0) && s_is_on(x - 1, y - 1, t) )  return ( true );

if ( (y > 0) && s_is_on(x, y - 1, t) )  return ( true );


return ( false );

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::split()

{

int j, k;
MtdDataType * d = (MtdDataType *) 0;
ModeTimeDomainFile old;

old = ::split(*this, NObjects);

const int Nxyt = Nx*Ny*Nt;

if ( NObjects == 0 )  return;

old.Volumes = new int [NObjects];

old.MaxConvIntensity = new double [NObjects];

d = old.Data;

for (j=0; j<NObjects; ++j)  old.Volumes[j] = 0;

for (j=0; j<NObjects; ++j)  old.MaxConvIntensity[j] = 0.0;


for (j=0; j<Nxyt; ++j, ++d)  {

   k = (int) (*d);

   if ( k == 0 )  continue;

   old.Volumes[k - 1] += 1;

}



   //
   //  done
   //

M = 1.0;
B = 0.0;

assign(old);

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::toss_small_objects(int min_volume)

{

int j, n_new;
int * new_to_old = (int *) 0;

new_to_old = new int[NObjects];   //  probably too big, but that's ok

n_new = 0;

for (j=0; j<NObjects; ++j)  {

   if ( Volumes[j] >= min_volume )  new_to_old[n_new++] = j;

}


sift_objects(n_new, new_to_old);


   //
   //  done
   //

delete [] new_to_old;   new_to_old = (int *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::toss_small_intensities(double min_intensity)

{

int j, n_new;
int * new_to_old = (int *) 0;

new_to_old = new int[NObjects];   //  probably too big, but that's ok

n_new = 0;

for (j=0; j<NObjects; ++j)  {

   if ( MaxConvIntensity[j] >= min_intensity )  new_to_old[n_new++] = j;

}


sift_objects(n_new, new_to_old);


   //
   //  done
   //

delete [] new_to_old;   new_to_old = (int *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::sift_objects(const int n_new, const int * new_to_old)

{

if ( n_new == NObjects )  return;

int j, k;
const int n3 = Nx*Ny*Nt;
int * old_to_new = (int *) 0;
int * new_volumes = (int *) 0;
double * new_intensities = (double *) 0;
MtdDataType * d = Data;


if ( n_new == 0 )  {

   cerr << "\n\n  ModeTimeDomainFile::sift_objects() -> no objects left!\n\n";

   exit ( 1 );

}


old_to_new = new int [NObjects];


new_volumes = new int [n_new];

new_intensities = new double [n_new];


for (j=0; j<n_new; ++j)  {

   new_volumes[j] = Volumes[new_to_old[j]];

   new_intensities[j] = MaxConvIntensity[new_to_old[j]];

}

for (j=0; j<NObjects; ++j)  old_to_new[j] = -1;

for (j=0; j<n_new; ++j)  old_to_new[new_to_old[j]] = j;

// for (j=0; j<NObjects; ++j)  cout << j << " ... " << old_to_new[j] << '\n';

int replace_count = 0;
int zero_count = 0;

d = Data;

for (j=0; j<n3; ++j, ++d)  {

   k = (int) (*d);

   if ( k == 0 )  continue;

   k = old_to_new[k - 1];

   if ( k < 0 )    { *d = (MtdDataType) 0;        ++zero_count; }
   else            { *d = (MtdDataType) (1 + k);  ++replace_count; }

}

// cout << "replace count = " << replace_count << '\n' << flush;
// cout << "zero count    = " << zero_count    << '\n' << flush;

   //
   //  rewire
   //

NObjects = n_new;


delete [] Volumes;  Volumes = (int *) 0;

Volumes = new_volumes;


delete [] MaxConvIntensity;  MaxConvIntensity = (double *) 0;

MaxConvIntensity = new_intensities;

   //
   //  done
   //

delete [] old_to_new;   old_to_new = (int *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_volumes(int nobj, const int * vols)

{

if ( Volumes )  { delete [] Volumes;  Volumes = (int *) 0; }

int j;

Volumes = new int [nobj];

for (j=0; j<nobj; ++j)  Volumes[j] = vols[j];

NObjects = nobj;


return;

}


////////////////////////////////////////////////////////////////////////


void ModeTimeDomainFile::set_max_conv_intensities(const double * values)

{

int j;

for (j=0; j<NObjects; ++j)  MaxConvIntensity[j] = values[j];

return;

}


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::object_closest_to_xy(const int x_test, const int y_test, int & obj_num, double & dist) const

{

const int min_volume = 0;

bool status = object_closest_to_xy(min_volume, x_test, y_test, obj_num, dist);

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::object_closest_to_xy(const int min_volume, const int x_test, const int y_test, int & obj_num, double & dist) const

{

obj_num = 0;

dist = 1.0e30;

if ( ! Volumes )  return ( false );

int k, x, y, t;
double d, dx, dy;
bool ok = false;


for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         k = ival(x, y, t);

         if ( k == 0 )  continue;

         if ( Volumes[k - 1] < min_volume )  continue;

         dx = (double) (x - x_test);
         dy = (double) (y - y_test);

         d = sqrt(dx*dx + dy*dy);

         if ( d < dist )  { obj_num = k;  dist = d; }

         ok = true;

      }

   }

}

if ( !ok )  return ( false );


return ( true );

}


////////////////////////////////////////////////////////////////////////


double ModeTimeDomainFile::closest_approach(const int obj_num, const int x_test, const int y_test) const

{

if ( (!Volumes) || (NObjects == 0) )  {

   cerr << "\n\n  ModeTimeDomainFile::closest_approach() const -> no objects!\n\n";

   exit ( 1 );

}

if ( (obj_num < 1) || (obj_num > NObjects) )  {

   cerr << "\n\n  ModeTimeDomainFile::closest_approach() const -> bad object number ... "
        << "object number = " << obj_num << ", NObjects = " << NObjects << "\n\n";

   exit ( 1 );

}


int k, x, y, t;
double d, dx, dy;
double min_dist;


min_dist = 1.0e30;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         k = ival(x, y, t);

         if ( k != obj_num )  continue;

         dx = (double) (x - x_test);
         dy = (double) (y - y_test);

         d = sqrt(dx*dx + dy*dy);

         if ( d < min_dist )  min_dist = d;

      }

   }

}



return ( min_dist );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile split(const ModeTimeDomainFile & mask, int & n_shapes)

{

int j, k;
int x, y, t;
int n_so_far;
int n_before, n_after;
int v, nc;
ModeTimeDomainFile f2;
ModeTimeDomainFile before;
ModeTimeDomainFile after;
ModeTimeDomainFile rv;
Partition p;
const int zero_border_size = 2;
int imin, imax;



   //
   //  find the partition
   //

n_shapes = 0;

n_so_far = 0;

f2 = mask.const_time_slice(0);

f2.zero_border(zero_border_size);

before = split_const_t(f2, n_before);

n_so_far = n_before;

for (j=0; j<n_before; ++j)  p.add_no_repeat(j + 1);

for (j=1; j<(mask.nt()); ++j)  {

   f2 = mask.const_time_slice(j);

   f2.zero_border(zero_border_size);

   after = split_const_t(f2, n_after);

   adjust_obj_numbers(after, n_so_far);

   for (k=0; k<n_after; ++k)  p.add_no_repeat(n_so_far + k + 1);

   n_so_far += n_after;

   find_overlap(before, after, p);

   before   = after;
   n_before = n_after;

}

// p.dump(cout);

n_shapes = p.n_elements();

   //
   //  use the partition
   //

rv = mask;

rv.set_to_zeroes();

n_so_far = 0;

for (t=0; t<(mask.nt()); ++t)  {

   f2 = mask.const_time_slice(t);

   f2.zero_border(zero_border_size);

   before = split_const_t(f2, n_before);

   adjust_obj_numbers(before, n_so_far);

   n_so_far += n_before;

   for (x=0; x<(mask.nx()); ++x)  {

      for (y=0; y<(mask.ny()); ++y)  {

         v = before.ival(x, y, 0);

         if ( v == 0 ) continue;

         nc = p.which_class(v);

         if ( nc < 0 )  {

            cerr << "\n\n  split(const ModeTimeDomainFile &, int &) -> can't find cell!\n\n";

            exit ( 1 );

         }

         rv.put_ival(nc + 1, x, y, t);

      }

   }

}

rv.calc_ival_minmax(imin, imax);

// rv.set_mb(imax, 0.0);
rv.set_mb(1.0, 0.0);

   //
   //  done
   //

return ( rv );

}


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile split_const_t(const ModeTimeDomainFile & id, int & n_shapes)

{

if ( id.nt() != 1 )  {

   cerr << "\n\n  split_const_t(const ModeTimeDomainFile &, int &) -> not const-time slice!\n\n";

   exit ( 1 );

}

int k, i, ii;
int x, y, xx, yy;
int current_shape;
int value;
bool shape_assigned = false;
ModeTimeDomainFile d;
Partition p;
const int nx = id.nx();
const int ny = id.ny();



d.set_size(nx, ny, 1);

d.set_grid(id.grid());

d.set_mb(1.0, 0.0);

d.set_to_zeroes();

n_shapes = 0;

   //
   //  shape numbers start at ONE here!!
   //

current_shape = 0;

for (y=(ny - 2); y>=0; --y)  {

   for (x=(nx - 2); x>=0; --x)  {

      shape_assigned = false;

      if ( !(id.s_is_on(x, y, 0)) )  continue;

         //
         //  check above left
         //

      xx = x - 1;  yy = y + 1;

      if ( (xx >= 0) && (yy < ny) && id.s_is_on(xx, yy, 0) )  {

         i  = d.ival( x,  y, 0);
         ii = d.ival(xx, yy, 0);

         if ( shape_assigned )  p.merge_values(i, ii);
         else                   d.put_ival(ii, x, y, 0);

         shape_assigned = true;

      }

         //
         //  check above
         //

      xx = x;  yy = y + 1;

      if ( (yy < ny) && id.s_is_on(xx, yy, 0) )  {

         i  = d.ival( x,  y, 0);
         ii = d.ival(xx, yy, 0);

         if ( shape_assigned )  p.merge_values(i, ii);
         else                   d.put_ival(ii, x, y, 0);

         shape_assigned = true;

      }

         //
         //  check upper right
         //

      xx = x + 1;  yy = y + 1;

      if ( (xx < nx) && (yy < ny) && id.s_is_on(xx, yy, 0) )  {

         i  = d.ival( x,  y, 0);
         ii = d.ival(xx, yy, 0);

         if ( shape_assigned )  p.merge_values(i, ii);
         else                   d.put_ival(ii, x, y, 0);

         shape_assigned = true;

      }

         //
         //  check to the right
         //

      xx = x + 1;  yy = y;

      if ( (xx < nx) && id.s_is_on(xx, yy, 0) )  {

         i  = d.ival( x,  y, 0);
         ii = d.ival(xx, yy, 0);

         if ( shape_assigned )  p.merge_values(i, ii);
         else                   d.put_ival(ii, x, y, 0);

         shape_assigned = true;

      }

         //
         //  is it a new shape?
         //

      if ( !shape_assigned )  {

         ++current_shape;

         d.put_ival(current_shape, x, y, 0);

         p.add_no_repeat(current_shape);

      }

   }   //  for x

}   //  for y




ModeTimeDomainFile q;


q.set_size(nx, ny, 1);

q.set_grid(id.grid());

q.set_mb(1.0, 0.0);

q.set_to_zeroes();


for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      i = d.ival(x, y, 0);

      value = 0;

      for (k=0; k<(p.n_elements()); ++k)  {

         if ( p.has(k, i) )  { value = k + 1;  break; }

      }

      q.put_ival(value, x, y, 0);

   }

}


n_shapes = p.n_elements();


   //
   //  done
   //

return ( q );

}


////////////////////////////////////////////////////////////////////////


void adjust_obj_numbers(ModeTimeDomainFile & s, int delta)

{

if ( s.nt() != 1 )  {

   cerr << "\n\n  adjust_obj_numbers() -> not const-time slice!\n\n";

   exit ( 1 );

}

if ( delta == 0 )  return;

int x, y;
int v;
const int nx = s.nx();
const int ny = s.ny();

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      v = s.ival(x, y, 0);

      if ( v != 0 )  s.put_ival(v + delta, x, y, 0);

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


void find_overlap(const ModeTimeDomainFile & before, const ModeTimeDomainFile & after, Partition & p)

{

int x, y;
int a, b;
const int nx = before.nx();
const int ny = before.ny();


for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      a = after.ival(x, y, 0);

      if ( !a )  continue;

      b = before.ival(x, y, 0);

      if ( !b )  continue;

      // cout << "merging values " << a << " and " << b << "\n";

      p.merge_values(a, b);

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


ModeTimeDomainFile select(const ModeTimeDomainFile & mask, int n)

{

if ( (n < 1) || (n > mask.n_objects()) )  {

   cerr << "\n\n  select(const ModeTimeDomainFile & mask, int n) -> range check error on n ... "
        << "NObjects = " << mask.n_objects() << " ... "
        << "n = " << n << "\n\n" << flush;

   exit ( 1 );

}

int x, y, t, k;
// const int np1 = n + 1;
int v;
int V[2];
ModeTimeDomainFile s;
const int nx = mask.nx();
const int ny = mask.ny();
const int nt = mask.nt();


s = mask;

s.set_to_zeroes();

v = 0;

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      for (t=0; t<nt; ++t)  {

         k = mask.ival(x, y, t);

         if ( k == n )  { s.put_ival(1, x, y, t);   ++v; }

      }

   }

}

V[0] = v;

s.set_volumes(1, V);

return ( s );

}


////////////////////////////////////////////////////////////////////////


int intersection_volume(const ModeTimeDomainFile & a, const ModeTimeDomainFile & b)

{

if ( (a.nx() != b.nx()) || (a.ny() != b.ny()) || (a.nt() != b.nt()) )  {

   cerr << "\n\n  intersection_volume(const SpacetimeField &, const SpacetimeField &) -> fields not same size!\n\n";

   exit ( 1 );

}

int x, y, t;
int count;
const int nx = a.nx();
const int ny = a.ny();
const int nt = a.nt();


count = 0;

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      for (t=0; t<nt; ++t)  {

         if ( a(x, y, t) && b(x, y, t) )  ++count;

      }

   }

}


return ( count );

}


////////////////////////////////////////////////////////////////////////




