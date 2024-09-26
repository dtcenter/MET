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
#include <cstdio>
#include <cmath>
#include <ctime>

#include <netcdf>

#include "mtd_file.h"
#include "mtd_partition.h"
#include "mtd_nc_defs.h"

#include "vx_math.h"
#include "vx_nc_util.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MtdFloatFile
   //


////////////////////////////////////////////////////////////////////////


MtdFloatFile::MtdFloatFile()

{

float_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MtdFloatFile::~MtdFloatFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MtdFloatFile::MtdFloatFile(const MtdFloatFile & f)

{

float_init_from_scratch();

float_assign(f);

}


////////////////////////////////////////////////////////////////////////


MtdFloatFile & MtdFloatFile::operator=(const MtdFloatFile & f)

{

if ( this == &f )  return *this;

float_assign(f);

return *this;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::float_init_from_scratch()

{

Data = nullptr;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::clear()

{

MtdFileBase::clear();

if ( Data )  { delete [] Data;  Data = nullptr; }

DataMin = DataMax = 0;

Spatial_Radius = -1;

TimeBeg = TimeEnd = bad_data_int;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::float_assign(const MtdFloatFile & f)

{

clear();

base_assign(f);

DataMin = f.DataMin;
DataMax = f.DataMax;

Spatial_Radius = f.Spatial_Radius;

TimeBeg = f.TimeBeg;

TimeEnd = f.TimeEnd;

const int n = Nx*Ny*Nt;

if ( f.Data )  {

   Data = new float [n];

   memcpy(Data, f.Data, n*sizeof(float));

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);

MtdFileBase::dump(out, depth);

if ( Spatial_Radius >= 0 )  {

   out << prefix << "Spatial_Radius = " << Spatial_Radius << '\n';

}

out << prefix << "TimeBeg = " << TimeBeg << '\n';

out << prefix << "TimeEnd = " << TimeEnd << '\n';

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::set_size(int _nx, int _ny, int _nt)

{


if ( Data )  { delete [] Data;  Data = nullptr; }

int j;
const int n3 = _nx*_ny*_nt;

Data = new float [n3];

Nx = _nx;
Ny = _ny;
Nt = _nt;

float * d = Data;

for (j=0; j<n3; ++j)  *d++ = 0.0;

Lead_Times.extend(Nt);

for (j=0; j<Nt; ++j)  {

   Lead_Times.add(0);

}

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::set_spatial_radius(int spatial_r)

{

if ( spatial_r < 0 )  {

   mlog << Error << "\nMtdFloatFile::set_spatial_radius(int) -> "
        << "bad value ... " << spatial_r << "\n\n";

   exit ( 1 );

}

Spatial_Radius = spatial_r;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::set_time_window(int beg, int end)

{

if ( end < beg )  {

   mlog << Error << "\nMtdFloatFile::set_time_window(int) -> "
        << "bad values ... " << beg << " and " << end << "\n\n";

   exit ( 1 );

}

TimeBeg = beg;

TimeEnd = end;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::put(const float value, int _x, int _y, int _t)

{

const int n = mtd_three_to_one(Nx, Ny, Nt, _x, _y, _t);

Data[n] = value;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::put(const DataPlane & plane, const int t)

{

if ( (plane.nx() != Nx) || (plane.ny() != Ny) )  {

   mlog << Error << "\nMtdFloatFile::put(const DataPlane &, const int) -> "
        << "plane wrong size!\n\n";

   exit ( 1 );

}

if ( (t < 0) || (t >= Nt) )  {

   mlog << Error << "\nMtdFloatFile::put(const DataPlane &, const int) -> "
        << "bad time\n\n";

   exit ( 1 );

}

   //
   //  Unfortunately, we can't do a memcpy here, because the
   //     DataPlane stores data as doubles, while this class
   //     stores it's data as floats ... too bad
   //

int x, y, n;
int count;
double value;

count = 0;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      value = plane(x, y);

      if ( value == bad_data_double )  ++count;

      n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

      Data[n] = (float) value;

   }

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::set_data_minmax(double _data_min, double _data_max)

{

DataMin = _data_min;

DataMax = _data_max;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::calc_data_minmax()

{

int j;
const int N = Nx*Ny*Nt;
bool ok = false;
const float * f = Data;
float value;

DataMin = DataMax = bad_data_float;

for (j=0; j<N; ++j)  {

   value = *f++;

   if ( value == bad_data_float )  continue;

   if ( !ok )  {

      DataMin = DataMax = value;

      ok = true;

   } else {

      if ( value < DataMin )  DataMin = value;
      if ( value > DataMax )  DataMax = value;

   }

}   //  for j


return;

}


////////////////////////////////////////////////////////////////////////


MtdIntFile MtdFloatFile::threshold(double T) const

{

MtdIntFile out;

threshold(T, out);

return out;

}


////////////////////////////////////////////////////////////////////////


MtdIntFile MtdFloatFile::threshold(const SingleThresh & t) const

{

MtdIntFile out;

threshold(t, out);

return out;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::threshold(double T, MtdIntFile & out) const

{

if ( !Data )  {

   mlog << Error << "\nMtdFloatFile::threshold(double, MtdIntFile &) const -> "
        << "no data!\n\n";

   exit ( 1 );

}

int j;
bool got_some = false;
float fval;
int ival;
const int n3 = Nx*Ny*Nt;
const float FT = (float) T;

out.clear();

out.base_assign(*this);

out.set_size(Nx, Ny, Nt);

for (j=0; j<Nt; ++j)  {

   out.set_lead_time(j, lead_time(j));

}

float * d = Data;
int * i = out.Data;


for (j=0; j<n3; ++j)  {

   fval = *d++;

   ival = ( (fval >= FT) ? 1 : 0);

   *i++ = ival;

   if ( ival )  got_some = true;

}

ival = ( got_some ? 1 : 0 );

out.set_data_minmax(0, ival);

out.set_radius(Spatial_Radius);

out.set_time_window(TimeBeg, TimeEnd);

out.set_threshold(T);

out.set_filetype(mtd_file_mask);



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::threshold(const SingleThresh & t, MtdIntFile & out) const

{

if ( !Data )  {

   mlog << Error << "\nMtdFloatFile::threshold(double, MtdIntFile &) const -> "
        << "no data!\n\n";

   exit ( 1 );

}

int j;
bool got_some = false;
float fval;
int ival;
const int n3 = Nx*Ny*Nt;
bool status = false;

out.clear();

out.base_assign(*this);

out.set_size(Nx, Ny, Nt);

float * d = Data;
int * i = out.Data;


for (j=0; j<n3; ++j)  {

   fval = *d++;

   status = t.check((double) fval);

   ival = ( status ? 1 : 0);

   *i++ = ival;

   if ( ival )  got_some = true;

}

ival = ( got_some ? 1 : 0 );

out.set_data_minmax(0, ival);

out.set_radius(Spatial_Radius);

out.set_time_window(TimeBeg, TimeEnd);

out.set_threshold(-9999.0);

out.set_filetype(mtd_file_mask);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool MtdFloatFile::read(const char * _filename)

{

NcFile f(_filename, NcFile::read);

if ( IS_INVALID_NC(f) )  return false;

Filename = _filename;

MtdFloatFile::read(f);

return true;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::read(NcFile & f)

{

NcVar var;


   //
   //  read the base class stuff
   //

MtdFileBase::read(f);

   //  DataMin, DataMax

DataMin = get_att_value_double(&f, min_value_att_name);
DataMax = get_att_value_double(&f, max_value_att_name);

   //  Data

set_size(Nx, Ny, Nt);

var = get_nc_var(&f, data_field_name);

LongArray offsets;  // {0,0,0};
LongArray lengths;  // {Nt, Ny, Nx};

offsets.add(0);
offsets.add(0);
offsets.add(0);
lengths.add(Nt);
lengths.add(Ny);
lengths.add(Nx);

if ( ! get_nc_data(&var, Data, lengths, offsets) )  {

   mlog << Error << "\nMtdFloatFile::read(const char *) -> "
        << "trouble getting data\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::write(NcFile & f) const

{

NcDim nx_dim;
NcDim ny_dim;
NcDim nt_dim;
NcVar data_var ;
const char format [] = "%.3f";
char junk[256];


   //
   //  write stuff from parent class
   //

MtdFileBase::write(f);

   //
   //  get the dimensions of the data field
   //

nx_dim = get_nc_dim(&f, nx_dim_name);
ny_dim = get_nc_dim(&f, ny_dim_name);
nt_dim = get_nc_dim(&f, nt_dim_name);


   //  DataMin, DataMax

snprintf(junk, sizeof(junk), format, DataMin);

add_att(&f, min_value_att_name, junk);

snprintf(junk, sizeof(junk), format, DataMax);

add_att(&f, max_value_att_name, junk);

   //  Spatial_Radius

if ( Spatial_Radius >= 0 )  {

   add_att(&f, radius_att_name, Spatial_Radius);

}

add_att(&f, time_beg_att_name, TimeBeg);

add_att(&f, time_end_att_name, TimeEnd);


   //  Data

add_var(&f, data_field_name, ncFloat, nt_dim, ny_dim, nx_dim);

data_var = get_nc_var(&f, data_field_name);

LongArray offsets;  // {0,0,0};
LongArray lengths;  // {Nt, Ny, Nx};

offsets.add(0);
offsets.add(0);
offsets.add(0);
lengths.add(Nt);
lengths.add(Ny);
lengths.add(Nx);

if ( ! get_nc_data(&data_var, Data, lengths, offsets) )  {

   mlog << Error << "\nMtdFloatFile::read(const char *) -> "
        << "trouble getting data\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::write(const char * _filename) const

{

NcFile f(_filename, NcFile::replace);

if ( IS_INVALID_NC(f) )  {

   mlog << Error << "\nMtdFloatFile::write(const char *) -> "
        << "unable to open netcdf output file: " << _filename << "\n\n";

   exit ( 1 );

}

MtdFloatFile::write(f);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


MtdFloatFile MtdFloatFile::const_t_slice(int t) const

{

if ( (t < 0) || (t >= Nt) )  {

   mlog << Error << "\nMtdFloatFile MtdFloatFile::const_t_slice(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

int j, n;
const int nxy = Nx*Ny;
int bytes = nxy*sizeof(float);
float fmin, fmax;
MtdFloatFile f;
float * d;
float value;

f.base_assign(*this);

f.Nt = 1;

f.DeltaT = 0;

f.StartValidTime = StartValidTime + t*DeltaT;

f.Spatial_Radius = Spatial_Radius;

f.TimeBeg = TimeBeg;

f.TimeEnd = TimeEnd;

f.Data = new float [Nx*Ny];

n = mtd_three_to_one(Nx, Ny, Nt, 0, 0, t);

memcpy(f.Data, Data + n, bytes);

d = f.Data;

fmin = fmax = f.Data[0];

for (j=0; j<nxy; ++j)  {

   value = *d++;

   if ( value < fmin )  fmin = value;
   if ( value > fmax )  fmax = value;

}

f.DataMin = fmin;
f.DataMax = fmax;

   //
   //  done
   //

return f;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::get_data_plane(const int t, DataPlane & out)

{

if ( (t < 0) || (t >= Nt) )  {

   mlog << Error << "\nMtdFloatFile::get_data_plane() -> "
        << "range check error on t\n\n";

   exit ( 1 );

}

if ( (out.nx() != Nx) || (out.ny() != Ny) )  out.set_size(Nx, Ny);

out.set_valid(StartValidTime + t*DeltaT);

   //  we really don't need these anyway, so just set them to zero

out.set_init(0);
out.set_lead(0);
out.set_accum(0);

int x, y, n;
double value;
const int data_cnt = Nx*Ny*Nt;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);
      value = (n >= 0 && n <data_cnt) ? Data[n] : bad_data_double;

      if ( value == bad_data_double )  out.put(bad_data_float, x, y);
      else                             out.put((float) value,  x, y);

   }   //  for y

}   //  for x


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::put_data_plane(const int t, const DataPlane & d)

{
const char *method_name = "MtdFloatFile::put_data_plane() -> ";

if ( (t < 0) || (t >= Nt) )  {

   mlog << Error << "\n" << method_name
        << "range check error on t\n\n";

   exit ( 1 );

}

if ( (d.nx() != Nx) || (d.ny() != Ny) )  {

   mlog << Error << "\n" << method_name
        << "data plane is wrong size!\n\n";

   exit ( 1 );

}

int x, y, n;
double value;
const int data_cnt = Nx*Ny*Nt;


for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);
      if (n < 0 || n >=data_cnt) {
         mlog << Debug(4) << method_name
              << "offset " << n << " is out of range (from "
              << x << ", " << y <<", " << t <<")\n";
         continue;
      }

      value = d(x, y);

      if ( value == bad_data_double )  Data[n] = bad_data_float;
      else                             Data[n] = (float) value;

   }

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFloatFile::regrid(const Grid & to_grid, const RegridInfo & info)

{

if ( to_grid == (*G) )  return;

MtdFloatFile old = *this;
DataPlane from_plane, to_plane;

from_plane.set_size(old.nx(), old.ny());
  to_plane.set_size(old.nx(), old.ny());

delete [] Data;  Data = nullptr;

Nx = to_grid.nx();
Ny = to_grid.ny();

Data = new float [Nx*Ny*Nt];
for (int t=0; t<Nx*Ny*Nt; ++t) Data[t] = bad_data_float;

for (int t=0; t<old.nt(); ++t)  {

   old.get_data_plane(t, from_plane);

   to_plane = met_regrid (from_plane, *G, to_grid, info);

   put_data_plane(t, to_plane);

}   //  for t

   //  store the updated grid

*G = to_grid;

   //
   //  done
   //

calc_data_minmax();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


