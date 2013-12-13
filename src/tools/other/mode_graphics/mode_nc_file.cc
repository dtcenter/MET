

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "mode_nc_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeNcFile
   //


////////////////////////////////////////////////////////////////////////


ModeNcFile::ModeNcFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModeNcFile::~ModeNcFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


ModeNcFile::ModeNcFile(const ModeNcFile &)

{

// init_from_scratch();
// 
// assign(m);

cerr << "\n\n  ModeNcFile::ModeNcFile(const ModeNcFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


ModeNcFile & ModeNcFile::operator=(const ModeNcFile &)

{

// if ( this == &m )  return ( * this );
// 
// assign(m);

cerr << "\n\n  ModeNcFile::operator=(const ModeNcFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ModeNcFile::init_from_scratch()

{

f = (NcFile *) 0;

_Grid = (Grid *) 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeNcFile::close()

{

if ( f )  { delete f;  f = (NcFile *) 0; }

if ( _Grid )  { delete _Grid;  _Grid = (Grid *) 0; }

FcstObjId  = (NcVar *) 0;
FcstCompId = (NcVar *) 0;

ObsObjId   = (NcVar *) 0;
ObsCompId  = (NcVar *) 0;

FcstRaw    = (NcVar *) 0;
ObsRaw     = (NcVar *) 0;

Nx = Ny = 0;

// NFcstObjs  = 0;
// NFcstComps = 0;
// 
// NObsObjs   = 0;
// NObsComps  = 0;

return;

}


////////////////////////////////////////////////////////////////////////


bool ModeNcFile::open(const char * filename)

{

// int x, y;
// int value;
NcDim * dim = (NcDim *) 0;


close();

f = new NcFile(filename);

if ( !(f->is_valid()) )  {

   close();

   return ( false );

}

   //
   //  get dimensions
   //


dim = f->get_dim("lon");

Nx = dim->size();


dim = f->get_dim("lat");

Ny = dim->size();


dim = (NcDim *) 0;

   //
   //  variables
   //

FcstRaw    = f->get_var("fcst_raw" );
ObsRaw     = f->get_var("obs_raw" );

FcstObjId  = f->get_var("fcst_obj_id" );
// FcstCompId = f->get_var("fcst_comp_id");
FcstCompId = f->get_var("fcst_clus_id");

ObsObjId   = f->get_var("obs_obj_id" );
// ObsCompId  = f->get_var("obs_comp_id");
ObsCompId  = f->get_var("obs_clus_id");

   //
   //  count objects
   //
/*
for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      value = fcst_obj_id(x, y);

      if ( value > NFcstObjs )  NFcstObjs = value;

      value = obs_obj_id(x, y);

      if ( value > NObsObjs )  NObsObjs = value;

      value = fcst_comp_id(x, y);

      if ( value > NFcstComps )  NFcstComps = value;

      value = obs_comp_id(x, y);

      if ( value > NObsComps )  NObsComps = value;

   }

}
*/

   //
   //  get grid
   //

_Grid = new Grid;

read_netcdf_grid(f, *_Grid);


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void ModeNcFile::dump(ostream & out) const

{

out << "\n";

if ( f )  out << "File is open\n";
else      out << "file is closed\n";

out << "\n";

out << "Nx = " << Nx << "\n";
out << "Ny = " << Ny << "\n";

out << "\n";

// out << "NFcstObjs  = " << NFcstObjs  << "\n";
// out << "NObsObjs   = " << NObsObjs   << "\n";
// 
// out << "\n";
// 
// out << "NFcstComps = " << NFcstComps << "\n";
// out << "NObsComps  = " << NObsComps  << "\n";

out << "\n";


out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int ModeNcFile::get_int(NcVar * var, int x, int y) const

{

if ( (x < 0) || (x >= Nx) || (y < 0) || (y >= Ny) )  {

   cerr << "\n\n  ModeNcFile::get_int() -> range check error ... "
        << "(" << x << ", " << y << ")"
        << "\n\n";

   exit ( 1 );

}

int i[2];
int status;

status = var->set_cur(y, x);   //  NOT (x, y)!

if ( !status )  {

   cerr << "\n\n  ModeNcFile::get_int() const -> can't set_cur for (" << x << ", " << y << ")!\n\n";

   exit ( 1 );

}

status = var->get(i, 1, 1);

if ( !status )  {

   cerr << "\n\n  ModeNcFile::get_int() const -> can't get value!\n\n";

   exit ( 1 );

}

return ( i[0] );

}


////////////////////////////////////////////////////////////////////////


double ModeNcFile::get_float(NcVar * var, int x, int y) const

{

if ( (x < 0) || (x >= Nx) || (y < 0) || (y >= Ny) )  {

   cerr << "\n\n  ModeNcFile::get_float() -> range check error ... "
        << "(" << x << ", " << y << ")"
        << "\n\n";

   exit ( 1 );

}

float ff[2];
int status;

status = var->set_cur(y, x);   //  NOT (x, y)!

if ( !status )  {

   cerr << "\n\n  ModeNcFile::get_float() const -> can't set_cur for (" << x << ", " << y << ")!\n\n";

   exit ( 1 );

}

status = var->get(ff, 1, 1);

if ( !status )  {

   cerr << "\n\n  ModeNcFile::get_float() const -> can't get value!\n\n";

   exit ( 1 );

}

return ( (double) (ff[0]) );

}


////////////////////////////////////////////////////////////////////////


double ModeNcFile::fcst_raw(int x, int y) const

{

double z;

z = get_float(FcstRaw, x, y);

return ( z );

}


////////////////////////////////////////////////////////////////////////


double ModeNcFile::obs_raw(int x, int y) const

{

double z;

z = get_float(ObsRaw, x, y);

return ( z );

}


////////////////////////////////////////////////////////////////////////


int ModeNcFile::fcst_obj_id(int x, int y) const

{

int k;

k = get_int(FcstObjId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeNcFile::fcst_comp_id(int x, int y) const

{

int k;

k = get_int(FcstCompId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeNcFile::obs_obj_id(int x, int y) const

{

int k;

k = get_int(ObsObjId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeNcFile::obs_comp_id(int x, int y) const

{

int k;

k = get_int(ObsCompId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcFile::select_obj(ModeObjectField field, int n) const

{

if ( (n == 0) || (n < -1) )  {

   cerr << "\n\n  ModeNcFile::select_obj() const -> bad value\n\n";

   exit ( 1 );

}

if ( !f )  {

   cerr << "\n\n  ModeNcFile::select_obj() const -> no data!\n\n";

   exit ( 1 );

}

int x, y;
int value;
int count;
DataPlane fdata;

fdata.set_size(Nx, Ny);

count = 0;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      fdata.set(0.0, x, y);


      switch ( field )  {

         case mof_fcst_obj:
            value = fcst_obj_id(x, y);
            break;

         case mof_fcst_comp:
            value = fcst_comp_id(x, y);
            break;

         case mof_obs_obj:
            value = obs_obj_id(x, y);
            break;

         case mof_obs_comp:
            value = obs_comp_id(x, y);
            break;

         default:
            cerr << "\n\n  ModeNcFile::select_obj() const -> bad field\n\n";
            exit ( 1 );
            break;

      }   //  switch


      if ( value < 0 )  continue;

      if ( n < 0 )  { fdata.set(1.0, x, y);  ++count;  continue; }

      if ( n == value )  { fdata.set(1.0, x, y);  ++count; }

   } 

}

// cout << "ModeNcFile::select_obj() -> count = " << count << "\n" << flush;

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcFile::select_fcst_obj  (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_fcst_obj, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcFile::select_fcst_comp (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_fcst_comp, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcFile::select_obs_obj   (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_obs_obj, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcFile::select_obs_comp  (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_obs_comp, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


bool ModeNcFile::x_line_valid (const int x) const

{

if ( (x < 0) || (x >= Nx) )  return ( false );

int y;
double v;
const double bad = -9999.0;
const double tol = 0.01;

for (y=0; y<Ny; ++y)  {

   v = obs_raw(x, y);

   if ( fabs(v - bad) > tol )  return ( true );

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool ModeNcFile::y_line_valid (const int y) const

{

if ( (y < 0) || (y >= Ny) )  return ( false );

int x;
double v;
const double bad = -9999.0;
const double tol = 0.01;

for (x=0; x<Nx; ++x)  {

   v = obs_raw(x, y);

   if ( fabs(v - bad) > tol )  return ( true );

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


void ModeNcFile::get_fcst_raw_range(double & min_value, double & max_value) const

{

if ( !f )  {

   cerr << "\n\n  ModeNcFile::fcst_raw_range() -> no data file open!\n\n";

   exit ( 1 );

}

int x, y;
int count;
double value;

min_value =  1.0e30;
max_value = -1.0e30;

count = 0;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      value = (double) get_float(FcstRaw, x, y);

      if ( value < -9000.0 )  continue;

      ++count;

      if ( value < min_value )  min_value = value;
      if ( value > max_value )  max_value = value;

   }

}

if ( count == 0 )  {

   cerr << "\n\n  ModeNcFile::get_fcst_raw_range() -> no valid data points!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////




