

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "met_file.h"
#include "get_met_grid.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


static const char x_dim_name          [] = "lon";
static const char y_dim_name          [] = "lat";

static const char valid_time_att_name [] = "valid_time_ut";
static const char  init_time_att_name [] = "init_time_ut";
static const char accum_time_att_name [] = "accum_time_sec";

static const char name_att_name       [] = "name";
static const char long_name_att_name  [] = "long_name";
static const char level_att_name      [] = "level";
static const char units_att_name      [] = "units";

static const int  max_met_args           = 30;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetNcFile
   //


////////////////////////////////////////////////////////////////////////


MetNcFile::MetNcFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetNcFile::~MetNcFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void MetNcFile::init_from_scratch()

{

Nc = (NcFile *) 0;

Dim = (NcDim **) 0;

Var = (NcVarInfo *) 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void MetNcFile::close()

{

if ( Nc )  { delete Nc;  Nc = (NcFile *) 0; }

if ( Dim )  { delete [] Dim;  Dim = (NcDim **) 0; }

Ndims = 0;

DimNames.clear();

Xdim = Ydim = (NcDim *) 0;

Nvars = 0;

if ( Var )  { delete [] Var;  Var = (NcVarInfo *) 0; }

ValidTime = (unixtime) 0;
InitTime  = (unixtime) 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool MetNcFile::open(const char * filename)

{

int j, k;
const char * c = (const char *) 0;
long long ill, vll;
NcVar * v   = (NcVar *) 0;


close();

Nc = new NcFile (filename);

if ( !(Nc->is_valid()) )  { close();  return ( false ); }

   //
   //  grid
   //

// if ( ! get_met_grid(*Nc, grid) )  { close();  return ( false ); }

read_netcdf_grid(Nc, grid);

   //
   //  dimensions
   //

Ndims = Nc->num_dims();

Dim = new NcDim * [Ndims];

DimNames.extend(Ndims);

for (j=0; j<Ndims; ++j)  {

   Dim[j] = Nc->get_dim(j);

   DimNames.add(Dim[j]->name());

}

for (j=0; j<Ndims; ++j)  {

   c = Dim[j]->name();

   if ( strcmp(c, x_dim_name) == 0 )  Xdim = Dim[j];
   if ( strcmp(c, y_dim_name) == 0 )  Ydim = Dim[j];

}

   //
   //  variables
   //

Nvars = Nc->num_vars();

Var = new NcVarInfo [Nvars];


for (j=0; j<Nvars; ++j)  {

   v = Nc->get_var(j);

   Var[j].var = v;

   Var[j].name = v->name();

   Var[j].Ndims = v->num_dims();

   Var[j].Dims = new NcDim * [Var[j].Ndims];

   //
   //  parse the variable attributes
   //
   
   get_att_str( Var[j], name_att_name,       Var[j].name_att      );
   get_att_str( Var[j], long_name_att_name,  Var[j].long_name_att );
   get_att_str( Var[j], level_att_name,      Var[j].level_att     );
   get_att_str( Var[j], units_att_name,      Var[j].units_att     );
   get_att_int( Var[j], accum_time_att_name, Var[j].AccumTime     );

   get_att_unixtime( Var[j], init_time_att_name,  ill);
   get_att_unixtime( Var[j], valid_time_att_name, vll);

   if ( !is_bad_data(ill) )   InitTime = ill;
   if ( !is_bad_data(vll) )  ValidTime = vll;

   for (k=0; k<(Var[j].Ndims); ++k)  {

      Var[j].Dims[k] = v->get_dim(k);

      if ( Var[j].Dims[k] == Xdim )  Var[j].x_slot = k;
      if ( Var[j].Dims[k] == Ydim )  Var[j].y_slot = k;

   }   //  for k

}   //  for j

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void MetNcFile::dump(ostream & out, int depth) const

{

int j, k;
int month, day, year, hour, minute, second;
char junk[256];
Indent prefix(depth);
Indent p2(depth + 1);
Indent p3(depth + 2);


out << prefix << "Grid ...\n";

grid.dump(out, depth + 1);

out << prefix << "\n";

out << prefix << "Nc = " << (Nc ? "ok" : "(nul)") << "\n";

out << prefix << "\n";

out << prefix << "Ndims = " << Ndims << "\n";

for (j=0; j<Ndims; ++j)  {

   out << p2 << "Dim # " << j << " = " << DimNames[j] << "   (" << (Dim[j]->size()) << ")\n";

}   //  for j

out << prefix << "\n";

out << prefix << "Xdim = " << (Xdim ? Xdim->name() : "(nul)") << "\n";
out << prefix << "Ydim = " << (Ydim ? Ydim->name() : "(nul)") << "\n";

out << prefix << "\n";

out << prefix << "Init Time = ";

unix_to_mdyhms(InitTime, month, day, year, hour, minute, second);

sprintf(junk, "%s %d, %d   %2d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

out << junk << "\n";

out << prefix << "\n";

out << prefix << "Nvars = " << Nvars << "\n";

for (j=0; j<Nvars; ++j)  {

   out << p2 << "Var # " << j << " = " << (Var[j].name) << "  (";

   for (k=0; k<(Var[j].Ndims); ++k)  {

           if ( Var[j].Dims[k] == Xdim )  out << 'X';
      else if ( Var[j].Dims[k] == Ydim )  out << 'Y';
      else                                out << Var[j].Dims[k]->name();

      if ( k < Var[j].Ndims - 1)  out << ", ";

   }   //  for k

   out << ")\n";

   out << p3 << "Slots (X, Y, Z, T) = (";

   if ( Var[j].x_slot >= 0 ) out << Var[j].x_slot; else out << '_';  out << ", ";
   if ( Var[j].y_slot >= 0 ) out << Var[j].y_slot; else out << '_';

   out << ")\n";

   out << p2 << "\n";

}   //  for j



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int MetNcFile::lead_time() const

{

unixtime dt = ValidTime - InitTime;

return ( (int) dt );

}


////////////////////////////////////////////////////////////////////////


double MetNcFile::data(NcVar * var, const LongArray & a) const

{

if ( !args_ok(a) )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
        << "bad arguments:\n";

   a.dump(cerr);

   exit ( 1 );

}

if ( var->num_dims() != a.n_elements() )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
        << "needed " << (var->num_dims()) << " arguments for variable "
        << (var->name()) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if ( var->num_dims() >= max_met_args )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
        << " too may arguments for variable \"" << (var->name()) << "\"\n\n";

   exit ( 1 );

}

int j;
float f[2];
double d[2];
int i[2];
bool status;
long counts[max_met_args];

for (j=0; j<(a.n_elements()); ++j) counts[j] = 1;

if ( !(var->set_cur((long *) a)) )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
        << " can't set corner for variable \"" << (var->name()) << "\"\n\n";

   exit ( 1 );

}

switch ( var->type() )  {

   case ncInt:
      status = var->get(i, counts);
      d[0] = (double) (i[0]);
      break;
  
   case ncFloat:
      status = var->get(f, counts);
      d[0] = (double) (f[0]);
      break;

   case ncDouble:
      status = var->get(d, counts);
      break;
      
   default:
      mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
           << " bad type for variable \"" << (var->name()) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch


if ( !status )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
        << " bad status for var->get()\n\n";

   exit ( 1 );
}


   //
   //  done
   //

return ( d[0] );

}


////////////////////////////////////////////////////////////////////////


bool MetNcFile::data(NcVar * v, const LongArray & a, DataPlane & plane) const

{

if ( !args_ok(a) )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
        << "bad arguments:\n";

   a.dump(cerr);

   exit ( 1 );

}

if ( v->num_dims() != a.n_elements() )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &, DataPlane &) -> "
        << "needed " << (v->num_dims()) << " arguments for variable "
        << (v->name()) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if ( v->num_dims() >= max_met_args )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &, DataPlane &) -> "
        << " too may arguments for variable \"" << (v->name()) << "\"\n\n";

   exit ( 1 );

}


int j, count;
int x, y;
double value;
bool found = false;
NcVarInfo * var = (NcVarInfo *) 0;
const int Nx = grid.nx();
const int Ny = grid.ny();
LongArray b = a;


   //
   //  find varinfo's
   //

found = false;

for (j=0; j<Nvars; ++j)  {

   if ( Var[j].var == v )  { found = true;  var = Var + j;  break; }

}

if ( !found )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
        << "variable " << (v->name()) << " not found!\n\n";

   exit ( 1 );

}

   //
   //  check star positions and count
   //

count = 0;

for (j=0; j<(a.n_elements()); ++j)  {

   if ( a[j] == vx_data2d_star )  {

      ++count;

      if ( (j != var->x_slot) && (j != var->y_slot) )  {

         mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
              << " star found in bad slot\n\n";

         exit ( 1 );

      }

   }

}

if ( count != 2 )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
        << " bad star count ... " << count << "\n\n";

   exit ( 1 );

}

   //
   //  check slots
   //

const int x_slot = var->x_slot;
const int y_slot = var->y_slot;

if ( (x_slot < 0) || (y_slot < 0) )  {

   mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
        << " bad x|y|z slot\n\n";

   exit ( 1 );

}

   //
   //  get the bad data value
   //

double missing_value = get_var_missing_value(v);
double fill_value    = get_var_fill_value(v);

   //
   //  set up the DataPlane object
   //

plane.clear();
plane.set_size(Nx, Ny);

   //
   //  get the data
   //

for (x=0; x<Nx; ++x)  {

   b[x_slot] = x;

   for (y=0; y<Ny; ++y)  {

      b[y_slot] = y;

      value = data(v, b);

      if(is_eq(value, missing_value) || is_eq(value, fill_value)) {
         value = bad_data_double;
      }

      plane.set(value, x, y);

   }   //  for y

}   //  for x

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool MetNcFile::data(const char * var_name, const LongArray & a, DataPlane & plane,
                     NcVarInfo *& info) const

{

info = find_var_name(var_name);

bool found = ( NULL != info );

if ( !found )  return ( false );

found = data(info->var, a, plane);

   //
   //  store the times
   //

plane.set_init  ( ValidTime - lead_time() );
plane.set_valid ( ValidTime );
plane.set_lead  ( lead_time() );
plane.set_accum ( info->AccumTime );

   //
   //  done
   //

return ( found );

}

////////////////////////////////////////////////////////////////////////

NcVarInfo* MetNcFile::find_var_name(const char * var_name) const {

   for(int i=0; i < Nvars; i++) if( Var[i].name == var_name ) return &Var[i];

   return NULL;
}


////////////////////////////////////////////////////////////////////////

