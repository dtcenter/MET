

////////////////////////////////////////////////////////////////////////


   //
   //  Write functions for class ModeTimeDomainFile
   //


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "nc_grid.h"
#include "mtd_file.h"
#include "mtd_nc_defs.h"


////////////////////////////////////////////////////////////////////////


bool ModeTimeDomainFile::write(const char * _filename)

{

int j, x, y, t, n;
NcFile * ncfile = (NcFile *) 0;
int month, day, year, hour, minute, second;
float fval[2];
double value;
char junk[256];
NcDim  * nx_dim    = (NcDim *)  0;
NcDim  * ny_dim    = (NcDim *)  0;
NcDim  * nt_dim    = (NcDim *)  0;
NcDim  * nobj_dim  = (NcDim *)  0;
NcVar  * data_var  = (NcVar *)  0;
NcVar  * vol_var   = (NcVar *)  0;
NcVar  * int_var   = (NcVar *)  0;
const bool is_split = (Volumes != 0);


   //
   //  open the file
   //

ncfile = new NcFile (_filename, NcFile::Replace);

if ( ! ncfile->is_valid() )  {

   cerr << "\n\n  ModeTimeDomainFile::write() -> unable to open netcdf output file \"" << _filename << "\"\n\n";

   // exit ( 1 );

   return ( false );

}

   //
   //  dimensions
   //

ncfile->add_dim(nx_dim_name, Nx);
ncfile->add_dim(ny_dim_name, Ny);
ncfile->add_dim(nt_dim_name, Nt);

if ( is_split )  ncfile->add_dim(nobj_dim_name, NObjects);

nx_dim = ncfile->get_dim(nx_dim_name);
ny_dim = ncfile->get_dim(ny_dim_name);
nt_dim = ncfile->get_dim(nt_dim_name);

if ( is_split )  nobj_dim = ncfile->get_dim(nobj_dim_name);

   //
   //  variables
   //

// ncfile->add_var(data_field_name, ncFloat, nx_dim, ny_dim, nt_dim);
ncfile->add_var(data_field_name, ncFloat, nt_dim, ny_dim, nx_dim);

data_var = ncfile->get_var(data_field_name);

if ( is_split )  {

   ncfile->add_var(volumes_name, ncInt, nobj_dim);

   vol_var = ncfile->get_var(volumes_name);

   ncfile->add_var(intensity_name, ncFloat, nobj_dim);

   int_var = ncfile->get_var(intensity_name);

}

   //
   //  global attributes
   //

unix_to_mdyhms(StartTime, month, day, year, hour, minute, second);

sprintf(junk, "%04d%02d%02d_%02d%02d%02d", year, month, day, hour, minute, second);

ncfile->add_att(start_time_att_name, junk);


sprintf(junk, "%d", DeltaT);

ncfile->add_att(delta_t_att_name, junk);


sprintf(junk, "%d", Radius);

ncfile->add_att(radius_att_name, junk);


sprintf(junk, "%.4f", Threshold);

fix_float(junk);

ncfile->add_att(threshold_att_name, junk);


sprintf(junk, "%.4f", int_to_double(0));

fix_float(junk);

ncfile->add_att(min_value_att_name, junk);


sprintf(junk, "%.6f", int_to_double(mtd_max_ival));

fix_float(junk);

ncfile->add_att(max_value_att_name, junk);


ncfile->add_att(is_split_att_name, (is_split ? 1 : 0));



write_nc_grid(*ncfile, *G);


   //
   //  volumes, if needed
   //

if ( is_split )  {

   for (j=0; j<NObjects; ++j)  {

      if ( !(vol_var->set_cur(j)) )  {

         cerr << "\n\n  ModeTimeDomainFile::write() -> trouble setting corner on volume field\n\n";

         exit ( 1 );

      }

      if ( !(vol_var->put(Volumes + j, 1)) )  {

         cerr << "\n\n  ModeTimeDomainFile::write() -> trouble with put in volume data field\n\n";

         exit ( 1 );

      }

   }   //  for j

}

   //
   //  max convolved intensities, if needed
   //

if ( is_split )  {

   for (j=0; j<NObjects; ++j)  {

      if ( !(int_var->set_cur(j)) )  {

         cerr << "\n\n  ModeTimeDomainFile::write() -> trouble setting corner on max intensity var\n\n";

         exit ( 1 );

      }

      fval[0] = (float) (MaxConvIntensity[j]);

      if ( !(int_var->put(fval, 1)) )  {

         cerr << "\n\n  ModeTimeDomainFile::write() -> trouble with put in max intensity field\n\n";

         exit ( 1 );

      }

   }   //  for j

}


   //
   //  data
   //

const time_t t_start = time(0);

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      for (t=0; t<Nt; ++t)  {

         n = mtd_three_to_one(Nx, Ny, Nt, x, y, t);

         value = int_to_double(Data[n]);

         fval[0] = (float) value;

         // if ( !(data_var->set_cur(x, y, t)) )  {
         if ( !(data_var->set_cur(t, y, x)) )  {

            cerr << "\n\n  ModeTimeDomainFile::write() -> trouble setting corner on data field\n\n";

            exit ( 1 );

         }

         if ( !(data_var->put(fval, 1, 1, 1)) )  {

            cerr << "\n\n  ModeTimeDomainFile::write() -> trouble with put in data field\n\n";

            exit ( 1 );

         }

      }

   }

}

const time_t t_stop = time(0);

cout << "\n\n  Time to write data = " << (t_stop - t_start) << " seconds\n\n" << flush;


   //
   //  done
   //

if ( ncfile )  { delete ncfile;  ncfile = (NcFile *) 0; }

return ( true );

}


////////////////////////////////////////////////////////////////////////



