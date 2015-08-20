

////////////////////////////////////////////////////////////////////////


   //
   //  Read functions for class ModeTimeDomainFile
   //


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cmath>

#include "vx_math.h"

#include "nc_grid.h"
#include "nc_utils.h"
#include "mtd_file.h"
#include "mtd_nc_defs.h"


////////////////////////////////////////////////////////////////////////


static Unixtime parse_start_time(const char *);


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::read(const char * _filename)

{

clear();

NcFile f(_filename);

if ( !(f.is_valid()) )  return ( false );


Filename = _filename;

int j, x, y, t, n, k;
double min_value, max_value;
double value;
bool is_split = false;
float fval[2];
NcVar * var = (NcVar *) 0;
NcDim * dim = (NcDim *) 0;
NcAtt * is_split_att = (NcAtt *) 0;
NcVar * v = (NcVar *) 0;
// NcAtt * att = (NcAtt *) 0;


var = f.get_var(data_field_name);

   //
   //  dimensions
   //

dim = f.get_dim(nx_dim_name);
Nx  = dim->size();

dim = f.get_dim(ny_dim_name);
Ny  = dim->size();

dim = f.get_dim(nt_dim_name);
Nt  = dim->size();


set_size(Nx, Ny, Nt);

dim = (NcDim *) 0;

   //
   //  global attributes (including grid info)
   //

StartTime = parse_start_time(string_att(f, start_time_att_name));

DeltaT    = string_att_as_int    (f, delta_t_att_name);
Radius    = string_att_as_int    (f, radius_att_name);
Threshold = string_att_as_double (f, threshold_att_name);
min_value = string_att_as_double (f, min_value_att_name);
max_value = string_att_as_double (f, max_value_att_name);

is_split_att = f.get_att(is_split_att_name);

k = is_split_att->as_int(0);

is_split = (k != 0);

B = min_value;

M = (max_value - min_value)/(mtd_max_ival - 1);


G = new Grid;

read_nc_grid(f, *G);

   //
   //  # of objects
   //

if ( is_split )  {

   NcDim * d = (NcDim *) 0;

   d = f.get_dim(nobj_dim_name);

   NObjects = d->size();

}


   //
   //  volumes
   //

if ( is_split )  {

   v = f.get_var(volumes_name);

   Volumes = new int [NObjects];

   for (j=0; j<NObjects; ++j)  {

      if ( !(v->set_cur(j)) )  {

         cerr << "\n\n  ModeTimeDomainFile::read(const char *) -> trouble setting volumes corner\n\n";

         exit ( 1 );

      }

      if ( ! (v->get(&k, 1)) )  {

         cerr << "\n\n  ModeTimeDomainFile::read(const char *) -> trouble getting volumes data\n\n";

         exit ( 1 );

      }

      Volumes[j] = k;

   }   //  for j

}

   //
   //  max intensities
   //

if ( is_split )  {

   v = f.get_var(intensity_name);

   MaxConvIntensity = new double [NObjects];

   for (j=0; j<NObjects; ++j)  {

      if ( !(v->set_cur(j)) )  {

         cerr << "\n\n  ModeTimeDomainFile::read(const char *) -> trouble setting max intensities corner\n\n";

         exit ( 1 );

      }

      if ( ! (v->get(fval, 1)) )  {

         cerr << "\n\n  ModeTimeDomainFile::read(const char *) -> trouble getting max intensities data\n\n";

         exit ( 1 );

      }

      MaxConvIntensity[j] = (double) (fval[0]);

   }   //  for j

}


   //
   //  data
   //

const time_t t_start = time(0);   //  for timing the data read operation

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

         // if ( !(var->set_cur(x, y, t)) )  {
         if ( !(var->set_cur(t, y, x)) )  {

            cerr << "\n\n  ModeTimeDomainFile::read(const char *) -> trouble setting corner\n\n";

            exit ( 1 );

         }

         if ( ! (var->get(fval, 1, 1, 1)) )  {

            cerr << "\n\n  ModeTimeDomainFile::read(const char *) -> trouble getting data\n\n";

            exit ( 1 );

         }

         value = (double) (fval[0]);

         k = nint( (value - B)/M );

         if ( (k < 0) || (k > mtd_max_ival) )  {

            cerr << "\n\n  ModeTimeDomainFile::read(const char *) -> bad data value ... "
                 << "value = " << value << " ... "
                 << "k = " << k << " ... "
                 << "(x, y, t) = (" << x << ", " << y << ", " << t << ")"
                 << "\n\n";

            exit ( 1 );

         }

         Data[n] = (MtdDataType) k;

      }

   }

}

const time_t t_stop = time(0);   //  for timing the data read operation

cout << "\n\n  Time to read data = " << (t_stop - t_start) << " seconds\n\n" << flush;

   //
   //  done
   //

f.close();

return ( true );

}


////////////////////////////////////////////////////////////////////////


Unixtime parse_start_time(const char * text)

{

int k;
int month, day, year, hour, minute, second;
Unixtime t;
const int n = strlen(text);

if ( n != 15 )  {

   cerr << "\n\n  parse_start_time() -> bad string ... \"" << text << "\"\n\n";

   exit ( 1 );

}

k = atoi(text);

year  = k/10000;
month = (k%10000)/100;
day   = k%100;

k = atoi(text + 9);

hour   = k/10000;
minute = (k%10000)/100;
second = k%100;

t = mdyhms_to_unix(month, day, year, hour, minute, second);


   //
   //  done
   //

return ( t );

}


////////////////////////////////////////////////////////////////////////


