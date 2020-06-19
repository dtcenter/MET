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
#include <time.h>

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "met_file.h"
#include "get_met_grid.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


static const char x_dim_name []          = "lon";
static const char y_dim_name []          = "lat";

static const string valid_time_att_name  = "valid_time_ut";
static const string  init_time_att_name  = "init_time_ut";
static const string accum_time_att_name  = "accum_time_sec";

static const string name_att_name        = "name";
static const string long_name_att_name   = "long_name";
static const string level_att_name       = "level";
static const string units_att_name       = "units";

static const int  max_met_args           = 30;

////////////////////////////////////////////////////////////////////////

template <typename T>

void copy_nc_data_as_double(double *to_array, const T *from_array,
                            const int x_slot, const int y_slot,
                            const int nx, const int ny,
                            double missing_value, double fill_value,
                            float add_offset, float scale_factor) {
   double value;
   int x, y, offset, start_offset;

   offset = 0;
   if (add_offset != 0.0 || scale_factor != 1.0) {
      if (x_slot > y_slot) {
         for (y=0; y<ny; ++y) {
            start_offset = y * nx;
            for (x=0; x<nx; ++x) {
               value = (double)from_array[x + start_offset];
               if(is_eq(value, missing_value) || is_eq(value, fill_value))
                  value = bad_data_double;
               else value = value * scale_factor + add_offset;
               to_array[offset++] = value;
            }
         }
      }
      else {
         for (x=0; x<nx; ++x) {
            start_offset = x * ny;
            for (y=0; y<ny; ++y) {
               value = (double)from_array[y + start_offset];
               if(is_eq(value, missing_value) || is_eq(value, fill_value))
                  value = bad_data_double;
               else value = value * scale_factor + add_offset;
               to_array[offset++] = value;
            }
         }
      }
   }
   else {
      if (x_slot > y_slot) {
         for (y=0; y<ny; ++y) {
            start_offset = y * nx;
            for (x=0; x<nx; ++x) {
               value = (double)from_array[x + start_offset];
               if(is_eq(value, missing_value) || is_eq(value, fill_value))
                  value = bad_data_double;
               to_array[offset++] = value;
            }
         }
      }
      else {
         for (x=0; x<nx; ++x) {
            start_offset = x * ny;
            for (y=0; y<ny; ++y) {
               value = (double)from_array[y + start_offset];
               if(is_eq(value, missing_value) || is_eq(value, fill_value))
                  value = bad_data_double;
               to_array[offset++] = value;
            }
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////

void check_nc_data_2d(const double *from_array, const int nx, const int ny,
                      const double missing_value) {
   int count_zero, count_missing, count_valid;
   count_zero = count_missing = count_valid = 0;
   for (int y=0; y<ny; ++y) {
      int start_offset = y * nx;
      for (int x=0; x<nx; ++x) {
         double value = from_array[x + start_offset];
         if (is_eq(missing_value, value)) count_missing++;
         else if (is_eq(missing_value, 0)) count_zero++;
         else count_valid++;
      }
   }
   mlog << Debug(7) << "check_nc_data_2d() count: valid=" << count_valid << ", zero="
        << count_zero << ", missing=" << count_missing << ", " << (nx*ny)
        << " == " << (count_zero+count_missing+count_valid) << "\n";
}

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
string c;
long long ill, vll;
NcVar v;


close();

Nc = open_ncfile(filename);

if ( IS_INVALID_NC_P(Nc) )  { close();  return ( false ); }

   //
   //  grid
   //

read_netcdf_grid(Nc, grid);

   //
   //  dimensions
   //

StringArray gDimNames;
get_dim_names(Nc, &gDimNames);

Ndims = gDimNames.n_elements();

for (j=0; j<Ndims; ++j)  {
   c = to_lower(gDimNames[j]);
   NcDim dim = get_nc_dim(Nc, gDimNames[j]);

   if ( c.compare(x_dim_name) == 0 ) {
      Xdim = &dim;
   }
   if ( c.compare(y_dim_name) == 0 ) {
      Ydim = &dim;
   }

}

   //
   //  variables
   //

   StringArray varNames;
   Nvars = get_var_names(Nc, &varNames);

   Var = new NcVarInfo [Nvars];


   for (j=0; j<Nvars; ++j)  {

      v = get_var(Nc, varNames[j].c_str());

      Var[j].var = new NcVar(v);

      Var[j].name = GET_NC_NAME(v).c_str();

      int dim_count = GET_NC_DIM_COUNT(v);
      Var[j].Ndims = dim_count;

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

      StringArray dimNames;
      get_dim_names(&v, &dimNames);

      for (k=0; k<(dim_count); ++k)  {
         c = to_lower(dimNames[k]);
         NcDim dim = get_nc_dim(&v, dimNames[k]);
         Var[j].Dims[k] = &dim;

         if ( c.compare(x_dim_name) == 0 ) Var[j].x_slot = k;
         if ( c.compare(y_dim_name) == 0 ) Var[j].y_slot = k;

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

   out << p2 << "Dim # " << j << " = " << DimNames[j] << "   (" << (GET_NC_SIZE_P(Dim[j])) << ")\n";

}   //  for j

out << prefix << "\n";

out << prefix << "Xdim = " << (Xdim ? GET_NC_NAME_P(Xdim) : "(nul)") << "\n";
out << prefix << "Ydim = " << (Ydim ? GET_NC_NAME_P(Ydim) : "(nul)") << "\n";

out << prefix << "\n";

out << prefix << "Init Time = ";

unix_to_mdyhms(InitTime, month, day, year, hour, minute, second);

snprintf(junk, sizeof(junk), "%s %d, %d   %2d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

out << junk << "\n";

out << prefix << "\n";

out << prefix << "Nvars = " << Nvars << "\n";

for (j=0; j<Nvars; ++j)  {

   out << p2 << "Var # " << j << " = " << (Var[j].name) << "  (";

   for (k=0; k<(Var[j].Ndims); ++k)  {

           if ( Var[j].Dims[k] == Xdim )  out << 'X';
      else if ( Var[j].Dims[k] == Ydim )  out << 'Y';
      else                                out << GET_NC_NAME_P(Var[j].Dims[k]);

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

const char *method_name = "MetNcFile::data(NcVar *, const LongArray &) const -> ";

if ( !args_ok(a) )  {

   mlog << Error << "\n" << method_name << "bad arguments:\n";

   a.dump(cerr);

   exit ( 1 );

}

int dimCount = GET_NC_DIM_COUNT_P(var);
if ( dimCount != a.n_elements() )  {

   mlog << Error << "\n" << method_name
        << "needed " << (dimCount) << " arguments for variable "
        << (var->getName()) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if ( dimCount >= max_met_args )  {

   mlog << Error << "\n" << method_name
        << " too may arguments for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";

   exit ( 1 );

}

int i;
short s;
float f;
double d = bad_data_double;
bool status;
float add_offset   = 0.f;
float scale_factor = 1.f;
double missing_value = get_var_missing_value(var);
double fill_value    = get_var_fill_value(var);
NcVarAtt *att_add_offset   = get_nc_att(var, (string)"add_offset");
NcVarAtt *att_scale_factor = get_nc_att(var, (string)"scale_factor");
if (!IS_INVALID_NC_P(att_add_offset) && !IS_INVALID_NC_P(att_scale_factor)) {
   add_offset = get_att_value_float(att_add_offset);
   scale_factor = get_att_value_float(att_scale_factor);
}
if (att_add_offset) delete att_add_offset;
if (att_scale_factor) delete att_scale_factor;

status = false;
switch ( GET_NC_TYPE_ID_P(var) )  {

   case NcType::nc_INT:
      get_nc_data(var, &i, (long *)a);
      d = (double) (i);
      status = true;
      break;

   case NcType::nc_SHORT:
      get_nc_data(var, &s, (long *)a);
      d = (double) (s);
      status = true;
      break;

   case NcType::nc_FLOAT:
      get_nc_data(var, &f, (long *)a);
      d = (double) (f);
      status = true;
      break;

   case NcType::nc_DOUBLE:
      get_nc_data(var, &d, (long *)a);
      status = true;
      break;

   default:
      mlog << Error << "\n" << method_name << " bad type (" << GET_NC_TYPE_ID_P(var)
           << ") for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch
if ((add_offset != 0.0 || scale_factor != 1.0) && !is_eq(d, missing_value) && !is_eq(d, fill_value)) {
   d = d * scale_factor + add_offset;
}


if ( !status )  {

   mlog << Error << "\n" << method_name << " bad status for var->get()\n\n";

   exit ( 1 );
}


   //
   //  done
   //

return ( d );

}


////////////////////////////////////////////////////////////////////////


bool MetNcFile::data(NcVar * v, const LongArray & a, DataPlane & plane) const

{

const char *method_name_short = "MetNcFile::data() const -> ";
const char *method_name = "MetNcFile::data(NcVar *, const LongArray &, DataPlane &) const -> ";

if ( !args_ok(a) )  {

   mlog << Error << "\n" << method_name << "bad arguments:\n";

   a.dump(cerr);

   exit ( 1 );

}

int dimCount = GET_NC_DIM_COUNT_P(v);

if ( dimCount != a.n_elements() )  {

   mlog << Error << "\n" << method_name
        << "needed " << (dimCount) << " arguments for variable "
        << (GET_NC_NAME_P(v)) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if ( dimCount >= max_met_args )  {

   mlog << Error << "\n" << method_name
        << " too may arguments for variable \"" << (GET_NC_NAME_P(v)) << "\"\n\n";

   exit ( 1 );

}


int j, count;
bool found = false;
NcVarInfo * var = (NcVarInfo *) 0;
const int Nx = grid.nx();
const int Ny = grid.ny();
LongArray b = a;
const int cell_count = Nx * Ny;


   //
   //  find varinfo's
   //

found = false;

for (j=0; j<Nvars; ++j)  {

   if ( Var[j].var == v )  { found = true;  var = Var + j;  break; }

}

if ( !found )  {

   mlog << Error << "\n" << method_name
        << "variable " << (GET_NC_NAME_P(v)) << " not found!\n\n";

   exit ( 1 );

}

   //
   //  check star positions and count
   //

count = 0;

for (j=0; j<(a.n_elements()); ++j)  {

   if ( a[j] == vx_data2d_star )  {

      ++count;

      if ( (var == NULL) || ( (j != var->x_slot) && (j != var->y_slot) ) )  {

         mlog << Error << "\n" << method_name << " star found in bad slot\n\n";

         exit ( 1 );

      }

   }

}

if ( count != 2 )  {

   mlog << Error << "\n" << method_name
        << " bad star count ... " << count << "\n\n";

   exit ( 1 );

}

   //
   //  check slots - additional logic to satisfy Fortify Null Dereference
   //
   
 int x_slot_tmp = 0;
 int y_slot_tmp = 0;
 if ( var == NULL || (var->x_slot < 0) || (var->y_slot < 0)  )  {

   mlog << Error << "\n" << method_name << " bad x|y|z slot\n\n";

   exit ( 1 );

}
else {
  x_slot_tmp = var->x_slot;
  y_slot_tmp = var->y_slot;
}

const int x_slot = x_slot_tmp;
const int y_slot = y_slot_tmp;
 
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
   clock_t clock_time;
   double nc_time;
   float add_offset   = 0.f;
   float scale_factor = 1.f;
   NcVarAtt *att_add_offset   = get_nc_att(v, (string)"add_offset");
   NcVarAtt *att_scale_factor = get_nc_att(v, (string)"scale_factor");
   if (!IS_INVALID_NC_P(att_add_offset) && !IS_INVALID_NC_P(att_scale_factor)) {
      add_offset = get_att_value_float(att_add_offset);
      scale_factor = get_att_value_float(att_scale_factor);
   }
   if (att_add_offset) delete att_add_offset;
   if (att_scale_factor) delete att_scale_factor;

   int type_id = GET_NC_TYPE_ID_P(v);
   long dim[dimCount], cur[dimCount];
   for (int index=0; index<dimCount; index++) {
      dim[index] = 1;
      cur[index] = (b[index] == vx_data2d_star) ? 0 : b[index];
   }
   dim[x_slot] = Nx;
   dim[y_slot] = Ny;

   int *int_array = (int *)0;
   short *short_array = (short *)0;
   float *float_array = (float *)0;
   double *double_array = (double *)0;

   double *data_array = new double[cell_count];
   clock_time = clock();
   
   switch ( type_id ) {
      case NcType::nc_INT:
         int_array = new int[cell_count];
         get_nc_data(v, int_array, dim, cur);
         copy_nc_data_as_double(data_array, int_array, x_slot, y_slot, Nx, Ny,
                                missing_value, fill_value, add_offset, scale_factor);
         if (int_array) delete[] int_array;
         break;

      case NcType::nc_SHORT:
         short_array = new short[cell_count];
         get_nc_data(v, short_array, dim, cur);
         copy_nc_data_as_double(data_array, short_array, x_slot, y_slot, Nx, Ny,
                                missing_value, fill_value, add_offset, scale_factor);
         if (short_array) delete[] short_array;
         break;

      case NcType::nc_FLOAT:
         float_array = new float[cell_count];
         get_nc_data(v, float_array, dim, cur);
         copy_nc_data_as_double(data_array, float_array, x_slot, y_slot, Nx, Ny,
                                missing_value, fill_value, add_offset, scale_factor);
         if (float_array) delete[] float_array;
         break;

      case NcType::nc_DOUBLE:
         double_array = new double[cell_count];
         get_nc_data(v, double_array, dim, cur);
         copy_nc_data_as_double(data_array, double_array, x_slot, y_slot, Nx, Ny,
                                missing_value, fill_value, add_offset, scale_factor);
         if (double_array) delete[] double_array;
         break;

      default:
         mlog << Error << "\n" << method_name << " bad type (" << GET_NC_TYPE_NAME_P(v)
              << ") for variable \"" << (GET_NC_NAME_P(v)) << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

   nc_time = clock();
   if (mlog.verbosity_level() >= 7) {
      double duration_sec = (double)(nc_time - clock_time)/CLOCKS_PER_SEC;
      check_nc_data_2d(data_array, Nx, Ny, missing_value);
      mlog << Debug(7) << method_name_short << "took " << duration_sec
           << " seconds to read NetCDF data\n";
   }
   
   plane.set_block(data_array, Nx, Ny);
   
   if (mlog.verbosity_level() >= 7) {
      double duration_sec = (double)(clock() - nc_time)/CLOCKS_PER_SEC;
      mlog << Debug(7) << method_name_short << "took " << duration_sec
           << " seconds to fill data plane\n";
   }
   
   if (data_array) delete[] data_array;
   
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

