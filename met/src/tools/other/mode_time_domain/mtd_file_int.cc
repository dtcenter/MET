

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "mtd_file.h"
#include "mtd_nc_defs.h"
#include "nc_utils.h"

#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MtdIntFile
   //


////////////////////////////////////////////////////////////////////////


MtdIntFile::MtdIntFile()

{

int_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MtdIntFile::~MtdIntFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MtdIntFile::MtdIntFile(const MtdIntFile & f)

{

int_init_from_scratch();

int_assign(f);

}


////////////////////////////////////////////////////////////////////////


MtdIntFile & MtdIntFile::operator=(const MtdIntFile & f)

{

if ( this == &f )  return ( * this );

int_assign(f);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::int_init_from_scratch()

{

Data = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::clear()

{

MtdFileBase::clear();

if ( Data )  { delete [] Data;  Data = 0; }

DataMin = DataMax = 0;

Radius = -1;

Threshold = -1.0;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::int_assign(const MtdIntFile & f)

{

clear();

base_assign(f);

DataMin = f.DataMin;
DataMax = f.DataMax;

Radius = f.Radius;

Threshold = f.Threshold;

const int n = Nx*Ny*Nt;

if ( f.Data )  {

   Data = new int [n];

   memcpy(Data, f.Data, n*sizeof(int));

}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);

MtdFileBase::dump(out, depth);

out << prefix << "Radius = " << Radius << '\n';

out << prefix << "Threshold = " << Threshold << '\n';


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::set_size(int _nx, int _ny, int _nt)

{


if ( Data )  { delete [] Data;  Data = 0; }

int j;
const int n3 = _nx*_ny*_nt;

Data = new int [n3];

Nx = _nx;
Ny = _ny;
Nt = _nt;

int * d = Data;

for (j=0; j<n3; ++j)  *d++ = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::set_radius(int r)

{

if ( r < 0 )  {

   cerr << "\n\n  MtdIntFile::set_radius(int) -> bad value ... " << r << "\n\n";

   exit ( 1 );

}

Radius = r;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::set_threshold(double t)

{

// if ( t < 0.0 )  {
// 
//    cerr << "\n\n  MtdIntFile::set_threshold(double) -> bad value ... " << t << "\n\n";
// 
//    exit ( 1 );
// 
// }

Threshold = t;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::put(const int value, int _x, int _y, int _t)

{

const int n = mtd_three_to_one(Nx, Ny, Nt, _x, _y, _t);

Data[n] = value;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::set_data_minmax(double _data_min, double _data_max)

{

DataMin = _data_min;

DataMax = _data_max;

return;

}


////////////////////////////////////////////////////////////////////////


bool MtdIntFile::read(const char * _filename)

{

NcFile f(_filename);

if ( ! f.is_valid() )  return ( false );

Filename = _filename;

MtdIntFile::read(f);

return ( true );

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::read(NcFile & f)

{

NcVar * var = 0;



   //
   //  read the base class stuff
   //

MtdFileBase::read(f);

   //  DataMin, DataMax

DataMin = string_att_as_double (f, min_value_att_name);
DataMax = string_att_as_double (f, max_value_att_name);

   //  Data

set_size(Nx, Ny, Nt);

var = f.get_var(data_field_name);

if ( !(var->set_cur(0, 0, 0)) )  {

   cerr << "\n\n  MtdIntFile::read() -> trouble setting corner\n\n";

   exit ( 1 );

}

const time_t t_start = time(0);   //  for timing the data read operation

if ( ! (var->get(Data, Nt, Ny, Nx)) )  {

   cerr << "\n\n  MtdIntFile::read(const char *) -> trouble getting data\n\n";

   exit ( 1 );

}

const time_t t_stop = time(0);   //  for timing the data read operation

cout << "\n\n  MtdIntFile::read(): Time to read data = " << (t_stop - t_start) << " seconds\n\n" << flush;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::write(NcFile & f) const

{

NcDim * nx_dim   = 0;
NcDim * ny_dim   = 0;
NcDim * nt_dim   = 0;
NcVar * data_var = 0;
const char format [] = "%d";
char junk[256];


   //
   //  write stuff from parent class
   //

MtdFileBase::write(f);

   //
   //  get the dimensions of the data field
   //

nx_dim = f.get_dim(nx_dim_name);
ny_dim = f.get_dim(ny_dim_name);
nt_dim = f.get_dim(nt_dim_name);


   //  DataMin, DataMax

sprintf(junk, format, DataMin);

f.add_att(min_value_att_name, junk);

sprintf(junk, format, DataMax);

f.add_att(max_value_att_name, junk);

   //  Radius

f.add_att(radius_att_name, Radius);

   //  Threshold

f.add_att(threshold_att_name, Threshold);

   //  Data

f.add_var(data_field_name, ncInt, nt_dim, ny_dim, nx_dim);

data_var = f.get_var(data_field_name);

if ( !(data_var->set_cur(0, 0, 0)) )  {

   cerr << "\n\n  MtdIntFile::write() -> trouble setting corner on data field\n\n";

   exit ( 1 );

}

const time_t t_start = time(0);   //  for timing the data write operation

if ( !(data_var->put(Data, Nt, Ny, Nx)) )  {

   cerr << "\n\n  MtdIntFile::write() -> trouble with put in data field\n\n";

   exit ( 1 );

}


const time_t t_stop = time(0);   //  for timing the data write operation

cout << "\n\n  MtdIntFile::write(): Time to write data = " << (t_stop - t_start) << " seconds\n\n" << flush;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::write(const char * _filename) const

{

NcFile f(_filename, NcFile::Replace);

if ( ! f.is_valid() )  {

   cerr << "\n\n  MtdIntFile::write(const char *) -> unable to open netcdf output file \"" << _filename << "\"\n\n";

   // exit ( 1 );

   return;

}

MtdIntFile::write(f);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdIntFile::split()

{



return;

}


////////////////////////////////////////////////////////////////////////


MtdIntFile MtdIntFile::const_t_slice(int t) const

{

if ( (t < 0) || (t >= Nt) )  {

   cerr << "\n\n  MtdIntFile MtdIntFile::const_t_slice(int) const -> range check error\n\n";

   exit ( 1 );

}

int j, n;
const int nxy = Nx*Ny;
int bytes = nxy*sizeof(int);
int fmin, fmax;
MtdIntFile f;
int * d;
int value;

f.base_assign(*this);

f.Nt = 1;

f.DeltaT = 0;

f.StartTime = StartTime + t*DeltaT;

f.Radius = Radius;

f.Data = new int [Nx*Ny];

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

return ( f );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////






