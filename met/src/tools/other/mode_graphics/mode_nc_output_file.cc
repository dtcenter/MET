// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "mode_nc_output_file.h"
#include "nc_var_info.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModeNcOutputFile
   //


////////////////////////////////////////////////////////////////////////


ModeNcOutputFile::ModeNcOutputFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModeNcOutputFile::~ModeNcOutputFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


ModeNcOutputFile::ModeNcOutputFile(const ModeNcOutputFile &)

{

// init_from_scratch();
// 
// assign(m);

mlog << Error << "\n\n  ModeNcOutputFile::ModeNcOutputFile(const ModeNcOutputFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


ModeNcOutputFile & ModeNcOutputFile::operator=(const ModeNcOutputFile &)

{

// if ( this == &m )  return ( * this );
// 
// assign(m);

mlog << Error << "\n\n  ModeNcOutputFile::operator=(const ModeNcOutputFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutputFile::init_from_scratch()

{

f = (NcFile *) 0;

_Grid = (Grid *) 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutputFile::close()

{

if ( f )  { delete f;  f = (NcFile *) 0; }

if ( _Grid )  { delete _Grid;  _Grid = (Grid *) 0; }

FcstObjId  = (NcVar *) 0;
FcstClusId = (NcVar *) 0;

ObsObjId   = (NcVar *) 0;
ObsClusId  = (NcVar *) 0;

FcstRaw    = (NcVar *) 0;
ObsRaw     = (NcVar *) 0;

Nx = Ny = 0;

ValidTime = InitTime = 0;

AccumTime = 0;

fcst_data_range_calculated = false;
 obs_data_range_calculated = false;

FcstDataMin = FcstDataMax = 0.0;
 ObsDataMin =  ObsDataMax = 0.0;

// NFcstObjs = 0;
// NFcstClus = 0;
// 
// NObsObjs  = 0;
// NObsClus  = 0;

Filename.clear();

return;

}


////////////////////////////////////////////////////////////////////////


bool ModeNcOutputFile::open(const char * _filename)

{

// int x, y;
// int value;
//NcDim * dim = (NcDim *) 0;
NcDim dim;
NcAtt *att = (NcAtt *)0;
ConcatString s;


close();

Filename = _filename;

f = open_ncfile(_filename);

if ( IS_INVALID_NC_P(f) )  {

   close();

   return ( false );

}

   //
   //  get dimensions
   //


dim = get_nc_dim(f, "lon");

Nx = GET_NC_SIZE(dim);


dim = get_nc_dim(f, "lat");

Ny = GET_NC_SIZE(dim);


//dim = (NcDim *) 0;

   //
   //  variables
   //

_FcstRaw    = get_nc_var(f, "fcst_raw" );
_ObsRaw     = get_nc_var(f, "obs_raw" );

_FcstObjId  = get_nc_var(f, "fcst_obj_id" );
_FcstClusId = get_nc_var(f, "fcst_clus_id");

_ObsObjId   = get_nc_var(f, "obs_obj_id" );
_ObsClusId  = get_nc_var(f, "obs_clus_id");

FcstRaw    = &_FcstRaw    ; 
ObsRaw     = &_ObsRaw     ; 
              
FcstObjId  = &_FcstObjId  ; 
FcstClusId = &_FcstClusId ; 
              
ObsObjId   = &_ObsObjId   ; 
ObsClusId  = &_ObsClusId  ; 



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
      value = fcst_clus_id(x, y);
      if ( value > NFcstClus )  NFcstClus = value;
      value = obs_clus_id(x, y);
      if ( value > NObsClus )  NObsClus = value;
   }
}
*/

   //
   //  get grid
   //

   _Grid = new Grid;
   
   read_netcdf_grid(f, *_Grid);

   //
   //  get init time, valid time, lead time from FcstRaw variable attributes
   //

   att = get_nc_att(FcstRaw, (string)"init_time_ut");
   InitTime = get_att_value_unixtime(att);
   if (InitTime < 0) {
      mlog << Error
           << "ModeNcOutputFile::open(const char *) -> init time should be an integer or a string!\n\n";
      exit ( 1 );
   }
   if (att) delete att;
   
   att = get_nc_att(FcstRaw, (string)"valid_time_ut");
   ValidTime = get_att_value_unixtime(att);
   if (ValidTime < 0) {
      mlog << Error
           << "ModeNcOutputFile::open(const char *) -> valid time should be an integer or a string!\n\n";
      exit ( 1 );
   }
   if (att) delete att;
   
// att = FcstRaw->get_att("accum_time_sec");
// 
// AccumTime = att->as_nclong(0);


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutputFile::dump(ostream & out) const

{

out << "\n";

if ( f )  out << "File is open\n";
else      out << "file is closed\n";

out << "\n";

out << "Nx = " << Nx << "\n";
out << "Ny = " << Ny << "\n";

out << "\n";

if ( f )  {

   int month, day, year, hour, minute, second;
   char junk[512];

   out << "ValidTime = ";
   unix_to_mdyhms(ValidTime, month, day, year, hour, minute, second);
   snprintf(junk, sizeof(junk), "%s %d %d  %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);
   out << junk << '\n';

   out << "InitTime  = ";
   unix_to_mdyhms(InitTime, month, day, year, hour, minute, second);
   snprintf(junk, sizeof(junk), "%s %d %d  %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);
   out << junk << '\n';

   out << "AccumTime = ";
   snprintf(junk, sizeof(junk), "%02d:%02d:%02d", AccumTime/3600, (AccumTime%3600)/60, AccumTime%60);
   out << junk << '\n';

}

// out << "NFcstObjs  = " << NFcstObjs  << "\n";
// out << "NObsObjs   = " << NObsObjs   << "\n";
// 
// out << "\n";
// 
// out << "NFcstClus = " << NFcstClus << "\n";
// out << "NObsClus  = " << NObsClus  << "\n";

out << "\n";


out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::n_fcst_simple_objs() const

{

if ( !f )  {

   mlog << Error << "\n\n  ModeNcOutputFile::n_fcst_simple_objs() const -> no file open!\n\n";

   exit ( 1 );

}

int n;

n = count_objects(FcstObjId);

return ( n );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::n_obs_simple_objs() const

{

if ( !f )  {

   mlog << Error << "\n\n  ModeNcOutputFile::n_obs_simple_objs() const -> no file open!\n\n";

   exit ( 1 );

}

int n;

n = count_objects(ObsObjId);

return ( n );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::n_fcst_clus_objs() const

{

if ( !f )  {

   mlog << Error << "\n\n  ModeNcOutputFile::n_fcst_clus_objs() const -> no file open!\n\n";

   exit ( 1 );

}

int n;

n = count_objects(FcstClusId);

return ( n );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::n_obs_clus_objs() const

{

if ( !f )  {

   mlog << Error << "\n\n  ModeNcOutputFile::n_obs_clus_objs() const -> no file open!\n\n";

   exit ( 1 );

}

int n;

n = count_objects(ObsClusId);

return ( n );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::count_objects(NcVar * var) const

{

int x, y, n, k;

n = 0;
int v[Ny][Nx];
long offsets[2] = { 0,  0};   //  NOT (x, y)!
long lengths[2] = {Ny, Nx};
if (get_nc_data(var, (int *)&v, lengths, offsets)) {
   for (x=0; x<Nx; ++x)  {
   
      for (y=0; y<Ny; ++y)  {
   
         //k = get_int(var, x, y);
         k = v[y][x];
   
         if ( k > n )  n = k;
   
      }
   
   }
}
return ( n );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::get_int(NcVar * var, int x, int y) const

{

if ( (x < 0) || (x >= Nx) || (y < 0) || (y >= Ny) )  {

   mlog << Error << "\n\n  ModeNcOutputFile::get_int() -> range check error ... "
        << "(" << x << ", " << y << ")"
        << "\n\n";

   exit ( 1 );

}

int i[2];
int status;
long offsets[2] = {y, x};   //  NOT (x, y)!
long lengths[2] = {1,1};

//status = var->set_cur(y, x);
//
//if ( !status )  {
//
//   mlog << Error << "\n\n  ModeNcOutputFile::get_int() const -> can't set_cur for (" << x << ", " << y << ")!\n\n";
//
//   exit ( 1 );
//
//}
//
//status = var->get(i, 1, 1);
status = get_nc_data(var, (int *)&i, lengths, offsets);

if ( !status )  {

   mlog << Error << "\n\n  ModeNcOutputFile::get_int() const -> can't get value!\n\n";

   exit ( 1 );

}

return ( i[0] );

}


////////////////////////////////////////////////////////////////////////


double ModeNcOutputFile::get_float(NcVar * var, int x, int y) const

{

if ( (x < 0) || (x >= Nx) || (y < 0) || (y >= Ny) )  {

   mlog << Error << "\n\n  ModeNcOutputFile::get_float() -> range check error ... "
        << "(" << x << ", " << y << ")"
        << "\n\n";

   exit ( 1 );

}

float ff[2];
int status;
long offsets[2] = {y, x};   //  NOT (x, y)!
long lengths[2] = {1,1};

//status = var->set_cur(y, x);   //  NOT (x, y)!
//
//if ( !status )  {
//
//   mlog << Error << "\n\n  ModeNcOutputFile::get_float() const -> can't set_cur for (" << x << ", " << y << ")!\n\n";
//
//   exit ( 1 );
//
//}
//
//status = var->get(ff, 1, 1);
status = get_nc_data(var, (float *)&ff, lengths, offsets);

if ( !status )  {

   mlog << Error << "\n\n  ModeNcOutputFile::get_float() const -> can't get value!\n\n";

   exit ( 1 );

}

return ( (double) (ff[0]) );

}


////////////////////////////////////////////////////////////////////////


double ModeNcOutputFile::fcst_raw(int x, int y) const

{

double z;

z = get_float(FcstRaw, x, y);

return ( z );

}


////////////////////////////////////////////////////////////////////////


double ModeNcOutputFile::obs_raw(int x, int y) const

{

double z;

z = get_float(ObsRaw, x, y);

return ( z );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::fcst_obj_id(int x, int y) const

{

int k;

k = get_int(FcstObjId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::fcst_clus_id(int x, int y) const

{

int k;

k = get_int(FcstClusId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::obs_obj_id(int x, int y) const

{

int k;

k = get_int(ObsObjId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


int ModeNcOutputFile::obs_clus_id(int x, int y) const

{

int k;

k = get_int(ObsClusId, x, y);

return ( k );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcOutputFile::select_obj(ModeObjectField field, int n) const

{

if ( (n == 0) || (n < -1) )  {

   mlog << Error << "\n\n  ModeNcOutputFile::select_obj() const -> bad value\n\n";

   exit ( 1 );

}

if ( !f )  {

   mlog << Error << "\n\n  ModeNcOutputFile::select_obj() const -> no data!\n\n";

   exit ( 1 );

}

int x, y;
int value = 0;
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

         case mof_fcst_clus:
            value = fcst_clus_id(x, y);
            break;

         case mof_obs_obj:
            value = obs_obj_id(x, y);
            break;

         case mof_obs_clus:
            value = obs_clus_id(x, y);
            break;

         default:
            mlog << Error << "\n\n  ModeNcOutputFile::select_obj() const -> bad field\n\n";
            exit ( 1 );
            break;

      }   //  switch


      if ( value < 0 )  continue;

      if ( n < 0 )  { fdata.set(1.0, x, y);  ++count;  continue; }

      if ( n == value )  { fdata.set(1.0, x, y);  ++count; }

   } 

}

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcOutputFile::select_fcst_obj  (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_fcst_obj, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcOutputFile::select_fcst_clus (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_fcst_clus, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcOutputFile::select_obs_obj   (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_obs_obj, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


DataPlane ModeNcOutputFile::select_obs_clus  (int n) const

{

DataPlane fdata;

fdata = select_obj(mof_obs_clus, n);

return ( fdata );

}


////////////////////////////////////////////////////////////////////////


bool ModeNcOutputFile::x_line_valid (const int x) const

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


bool ModeNcOutputFile::y_line_valid (const int y) const

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


void ModeNcOutputFile::calc_data_range(NcVar *, double & min_value, double & max_value)

{

int x, y;
int count;
double value;

min_value =  1.0e30;
max_value = -1.0e30;

count = 0;

for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      value = (double) get_float(ObsRaw, x, y);

      if ( value < -9000.0 )  continue;

      ++count;

      if ( value < min_value )  min_value = value;
      if ( value > max_value )  max_value = value;

   }

}

if ( count == 0 )  {

   mlog << Error << "\n\n  ModeNcOutputFile::calc_data_range() -> no valid data points!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutputFile::get_fcst_raw_range(double & min_value, double & max_value)

{

if ( !f )  {

   mlog << Error << "\n\n  ModeNcOutputFile::get_fcst_raw_range() -> no data file open!\n\n";

   exit ( 1 );

}

if ( fcst_data_range_calculated )  {

   min_value = FcstDataMin;
   max_value = FcstDataMax;

   return;

}

calc_data_range(FcstRaw, min_value, max_value);

FcstDataMin = min_value;
FcstDataMax = max_value;

fcst_data_range_calculated = true;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutputFile::get_obs_raw_range(double & min_value, double & max_value)

{

if ( !f )  {

   mlog << Error << "\n\n  ModeNcOutputFile::get_obs_raw_range() -> no data file open!\n\n";

   exit ( 1 );

}

if ( obs_data_range_calculated )  {

   min_value = ObsDataMin;
   max_value = ObsDataMax;

   return;

}

calc_data_range(ObsRaw, min_value, max_value);

ObsDataMin = min_value;
ObsDataMax = max_value;

obs_data_range_calculated = true;

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString ModeNcOutputFile::short_filename() const

{

if ( Filename.empty() )  {

   mlog << Error << "\n\n  ModeNcOutputFile::short_filename() const -> no filename set!\n\n";

   exit ( 1 );

}

ConcatString s;

s = get_short_name(Filename.c_str());

return ( s );

}


////////////////////////////////////////////////////////////////////////




