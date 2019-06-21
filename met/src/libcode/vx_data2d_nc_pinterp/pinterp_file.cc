

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include "vx_nc_util.h"

#include "pinterp_file.h"
#include "get_pinterp_grid.h"


////////////////////////////////////////////////////////////////////////


static const char x_dim_name           [] = "west_east";
static const char y_dim_name           [] = "south_north";
static const char t_dim_name           [] = "time";
static const char z_dim_p_interp_name  [] = "num_metgrid_levels";
static const char z_dim_wrf_interp_name[] = "vlevs";
static const string strl_dim_name         = "DateStrLen";

static const char  times_var_name      [] = "Times";
static const char  month_var_name      [] = "month";
static const char    day_var_name      [] = "day";
static const char   year_var_name      [] = "year";
static const char   hour_var_name      [] = "hour";
static const char minute_var_name      [] = "minute";
static const char second_var_name      [] = "second";

static const char pressure_var_p_interp_name   [] = "pressure";
static const char pressure_var_wrf_interp_name [] = "LEV";

static const char pa_units_str         [] = "Pa";
static const char hpa_units_str        [] = "hPa";

static const string init_time_att_name   = "START_DATE";

static const string description_att_name  = "description";
static const string units_att_name        = "units";

static const int max_pinterp_args         = 30;

static const double pinterp_missing       = 1.0e35;

static const char *accum_var_names     [] = { "ACGRDFLX", "CUPPT",
                                              "RAINC",    "RAINNC",
                                              "SNOWNC",   "GRAUPELNC",
                                              "ACHFX",    "ACLHF" };
static const int n_accum_var_names        = sizeof(accum_var_names)/sizeof(*accum_var_names);

////////////////////////////////////////////////////////////////////////


static unixtime parse_init_time(const char *);

static bool is_bad_data_pinterp(double);

static bool is_accumulation(const char *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PinterpFile
   //


////////////////////////////////////////////////////////////////////////


PinterpFile::PinterpFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PinterpFile::~PinterpFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void PinterpFile::init_from_scratch()

{

Nc = (NcFile *) 0;

Dim = (NcDim **) 0;

Var = (NcVarInfo *) 0;

Time = (unixtime *) 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void PinterpFile::close()

{

if ( Nc )  { delete Nc;  Nc = (NcFile *) 0; }

if ( Dim )  { delete [] Dim;  Dim = (NcDim **) 0; }

if ( Time )  { delete [] Time;  Time = (unixtime *) 0; }

Ndims = 0;

DimNames.clear();

Xdim = Ydim = Zdim = Tdim = (NcDim *) 0;

Nvars = 0;

if ( Var )  { delete [] Var;  Var = (NcVarInfo *) 0; }

InitTime = (unixtime) 0;

Ntimes = 0;

PressureIndex = -1;

hPaCF = 1.0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool PinterpFile::open(const char * filename)

{

int j, k;
int month, day, year, hour, minute, second, str_len;
char time_str[max_str_len];
string c;
NcVar v;


close();

Nc = open_ncfile(filename);
mlog << Debug(5) << "\nPinterpFile::open() -> "
     << "opend  \"" << filename << "\".\n\n";

if ( IS_INVALID_NC_P(Nc) )  { close();  return ( false ); }

   //
   //  grid
   //

if ( ! get_pinterp_grid(*Nc, grid) )  { close();  return ( false ); }

   //
   //  dimensions
   //
Ndims = get_dim_count(Nc);
Dim = new NcDim*[Ndims];

StringArray gDimNames;
get_dim_names(Nc, &gDimNames);

for (j=0; j<Ndims; ++j)  {
   c = to_lower(gDimNames[j]);
   NcDim dim = get_nc_dim(Nc, gDimNames[j]);

   if ( c.compare(t_dim_name) == 0 )  Ntimes = GET_NC_SIZE(dim);

   if ( c.compare(x_dim_name) == 0 )            Xdim = &dim;
   if ( c.compare(y_dim_name) == 0 )            Ydim = &dim;
   if ( c.compare(z_dim_p_interp_name  ) == 0 ||
        c.compare(z_dim_wrf_interp_name) == 0)  Zdim = &dim;
   if ( c.compare(t_dim_name) == 0 )            Tdim = &dim;

}

   //
   //  times
   //

if ( Ntimes == 0 )  { close();  return ( false ); }

Time = new unixtime [Ntimes];

   //
   // attempt to parse variable "char Times(time, DateStrLen)"
   // format = YYYY-MM-DD_hh:mm:ss
   //


if ( has_var(Nc, times_var_name) ) {
   v = get_var(Nc, times_var_name);

   get_dim(Nc, strl_dim_name, str_len, true);

   for (j=0; j<Ntimes; ++j)  {
      ConcatString tmp_time_str;
      get_string_val(&v, j, str_len, tmp_time_str);
      strncpy ( time_str, tmp_time_str.c_str(), str_len );
      time_str[str_len] = '\0';

      // Check for leading blank
      if(time_str[0] == ' ') {
         Time[j] = 0;
      }
      else {

         if(sscanf(time_str, "%4d-%2d-%2d_%2d:%2d:%2d",
                   &year, &month, &day, &hour, &minute, &second) != 6) {
            mlog << Error << "\nPinterpFile::open() -> "
                 << "error parsing time string \"" << time_str << "\".\n\n";
            return ( false );
         }

         Time[j] = mdyhms_to_unix(month, day, year, hour, minute, second);
      }
   }   //  for j
}
   //
   // otherwise, parse variables month, day, year, hour, minute, second
   //
else {

   for (j=0; j<Ntimes; ++j)  {

      month  = get_int_var(Nc,  month_var_name, j);
      day    = get_int_var(Nc,    day_var_name, j);
      year   = get_int_var(Nc,   year_var_name, j);
      hour   = get_int_var(Nc,   hour_var_name, j);
      minute = get_int_var(Nc, minute_var_name, j);
      second = get_int_var(Nc, second_var_name, j);

      Time[j] = mdyhms_to_unix(month, day, year, hour, minute, second);

   }   //  for j
}

ConcatString att_value;
get_global_att(Nc, init_time_att_name, att_value);

InitTime = parse_init_time(att_value.c_str());

   //
   //  variables
   //


   StringArray varNames;
   StringArray dimNames;
   Nvars = get_var_names(Nc, &varNames);
   Var = new NcVarInfo [Nvars];

   for (j=0; j<Nvars; ++j)  {
      v = get_var(Nc, varNames[j].c_str());

      Var[j].var = new NcVar(v);

      Var[j].name = GET_NC_NAME(v).c_str();

      int dim_count = GET_NC_DIM_COUNT(v);
      Var[j].Ndims = dim_count;

      Var[j].Dims = new NcDim * [dim_count];

      //
      //  parse the variable attributes
      //
      get_att_str( Var[j], description_att_name, Var[j].long_name_att );
      get_att_str( Var[j], units_att_name,       Var[j].units_att     );

      //
      //  get the pressure variable and store the hPa conversion factor
      //

      if ( strcasecmp(GET_NC_NAME(v).c_str(), pressure_var_p_interp_name)   == 0 ||
           strcasecmp(GET_NC_NAME(v).c_str(), pressure_var_wrf_interp_name) == 0 ) {
         PressureIndex = j;
              if ( strcasecmp(Var[j].units_att.c_str(), pa_units_str ) == 0 ) hPaCF = 0.01;
         else if ( strcasecmp(Var[j].units_att.c_str(), hpa_units_str) == 0 ) hPaCF = 1.0;
      }


      dimNames.clear();
      get_dim_names(&v, &dimNames);

      for (k=0; k<(dim_count); ++k)  {
        c = to_lower(dimNames[k]);
        NcDim dim = get_nc_dim(&v, dimNames[k]);
        Var[j].Dims[k] = &dim;

        if ( Var[j].Dims[k] == Xdim )  Var[j].x_slot = k;
        if ( Var[j].Dims[k] == Ydim )  Var[j].y_slot = k;
        if ( Var[j].Dims[k] == Zdim )  Var[j].z_slot = k;
        if ( Var[j].Dims[k] == Tdim )  Var[j].t_slot = k;
        if ( c.compare(x_dim_name) == 0 ) {
           Var[j].x_slot = k;
        }
        if ( c.compare(y_dim_name) == 0 ) {
           Var[j].y_slot = k;
        }
        if ( c.compare(z_dim_p_interp_name  ) == 0 ||
             c.compare(z_dim_wrf_interp_name) == 0) {
           Var[j].z_slot = k;
        }
        if ( c.compare(t_dim_name) == 0 ) {
           Var[j].t_slot = k;
        }
      }   //  for k
   }   //  for j


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void PinterpFile::dump(ostream & out, int depth) const

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
out << prefix << "Zdim = " << (Zdim ? GET_NC_NAME_P(Zdim) : "(nul)") << "\n";
out << prefix << "Tdim = " << (Zdim ? GET_NC_NAME_P(Tdim) : "(nul)") << "\n";

out << prefix << "\n";

out << prefix << "Ntimes = " << Ntimes << "\n";

for (j=0; j<Ntimes; ++j)  {

   out << p2 << "Time # " << j << " = ";

   unix_to_mdyhms(Time[j], month, day, year, hour, minute, second);

   snprintf(junk, sizeof(junk), "%s %d, %d   %2d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << junk << "\n";

}

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
      else if ( Var[j].Dims[k] == Zdim )  out << 'Z';
      else if ( Var[j].Dims[k] == Tdim )  out << 'T';
      else                                out << GET_NC_NAME_P(Var[j].Dims[k]);

      if ( k < Var[j].Ndims - 1)  out << ", ";

   }   //  for k

   out << ")\n";

   out << p3 << "Slots (X, Y, Z, T) = (";

   if ( Var[j].x_slot >= 0 ) out << Var[j].x_slot; else out << '_';  out << ", ";
   if ( Var[j].y_slot >= 0 ) out << Var[j].y_slot; else out << '_';  out << ", ";
   if ( Var[j].z_slot >= 0 ) out << Var[j].z_slot; else out << '_';  out << ", ";
   if ( Var[j].t_slot >= 0 ) out << Var[j].t_slot; else out << '_';

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


unixtime PinterpFile::valid_time(int n) const

{

if ( (n < 0) || (n >= Ntimes) )  {

   mlog << Error << "\nPinterpFile::valid_time(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}


return ( Time [n] );

}


////////////////////////////////////////////////////////////////////////


int PinterpFile::lead_time(int n) const

{

if ( (n < 0) || (n >= Ntimes) )  {

   mlog << Error << "\nPinterpFile::lead_time(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

unixtime dt = Time[n] - InitTime;

return ( (int) dt );

}


////////////////////////////////////////////////////////////////////////


double PinterpFile::data(NcVar * var, const LongArray & a) const

{

if ( !args_ok(a) )  {

   mlog << Error << "\nPinterpFile::data(NcVar *, const LongArray &) const -> "
        << "bad arguments:\n";

   a.dump(cerr);

   exit ( 1 );

}

int dim_count = var->getDimCount();
if ( dim_count != a.n_elements() )  {

   mlog << Error << "\nPinterpFile::data(NcVar *, const LongArray &) const -> "
        << "needed " << (dim_count) << " arguments for variable "
        << (GET_NC_NAME_P(var)) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if ( dim_count >= max_pinterp_args )  {

   mlog << Error << "\nPinterpFile::data(NcVar *, const LongArray &) const -> "
        << " too may arguments for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";

   exit ( 1 );

}

bool status;
int i;
short s;
float f;
double d;
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

switch ( GET_NC_TYPE_ID_P(var) )  {

   case NcType::nc_INT:
      status = get_nc_data(var, &i, (long *)a);
      d = (double) (i);
      break;

   case NcType::nc_SHORT:
      status = get_nc_data(var, &s, (long *)a);
      d = (double) (s);
      break;

   case NcType::nc_FLOAT:
      status = get_nc_data(var, &f, (long *)a);
      d = (double) (f);
      break;

   case NcType::nc_DOUBLE:
      status = get_nc_data(var, &d, (long *)a);
      break;

   default:
      mlog << Error << "\nPinterpFile::data(NcVar *, const LongArray &) const -> "
           << " bad type for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch
if ((add_offset != 0.0 || scale_factor != 1.0) && !is_eq(d, missing_value) && !is_eq(d, fill_value)) {
   d = d * scale_factor + add_offset;
}


if ( !status )  {

   mlog << Error << "\nPinterpFile::data(NcVar *, const LongArray &) const -> "
        << " bad status for var->get()\n\n";

   exit ( 1 );
}


   //
   //  done
   //

return ( d );

}


////////////////////////////////////////////////////////////////////////


bool PinterpFile::data(NcVar * v, const LongArray & a, DataPlane & plane, double & pressure) const

{

if ( !args_ok(a) )  {

   mlog << Warning << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << "bad arguments:\n";

   a.dump(cerr);

   return ( false );

}

string var_name = GET_NC_NAME_P(v);
int dim_count = v->getDimCount();
if ( dim_count != a.n_elements() )  {

   mlog << Warning << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << "needed " << dim_count << " arguments for variable "
        << (var_name) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if ( dim_count >= max_pinterp_args )  {

   mlog << Warning << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << " too may arguments for variable \"" << (var_name) << "\"\n\n";

   return ( false );

}


int j, count;
int x, y;
double value;
bool found = false;
NcVarInfo * var = (NcVarInfo *) 0;
NcVarInfo * P   = (NcVarInfo *) 0;
const int Nx = grid.nx();
const int Ny = grid.ny();
LongArray b = a;

pressure = bad_data_double;

   //
   //  find varinfo's
   //

if ( PressureIndex >= 0 )  P = Var + PressureIndex;

found = false;

for (j=0; j<Nvars; ++j)  {

   if ( Var[j].var == v )  { found = true;  var = Var + j;  break; }

}

if ( !found )  {

   mlog << Warning << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << "variable " << (var_name) << " not found!\n\n";

   return ( false );

}

   //
   //  check x_slot and y_slot
   //

if ( var == NULL || (var->x_slot < 0) || (var->y_slot < 0) )  {

   mlog << Error
        << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << "can't find needed dimensions(s) for variable \""
        << var_name << "\" ... one of the dimensions may be staggered.\n\n";

   return ( false );

}

   //
   //  check star positions and count
   //

count = 0;

for (j=0; j<(a.n_elements()); ++j)  {

   if ( a[j] == vx_data2d_star )  {

      ++count;

      if ( (j != var->x_slot) && (j != var->y_slot) )  {

         mlog << Warning << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
              << " star found in bad slot\n\n";

         return ( false );

      }

   }

}

if ( count != 2 )  {

   mlog << Warning << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << " bad star count ... " << count << "\n\n";

   return ( false );

}

   //
   //  check slots
   //

const int x_slot = var->x_slot;
const int y_slot = var->y_slot;
const int z_slot = var->z_slot;

if ( (x_slot < 0) || (y_slot < 0) )  {

   mlog << Warning << "\nPinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << " bad x|y slot\n\n";

   return ( false );

}

   //
   //  set up the DataPlane object
   //

plane.clear();
plane.set_size(Nx, Ny);

   //
   //  get the data
   //
double d[Ny];
int    i[Ny];
short  s[Ny];
float  f[Ny];

long offsets[dim_count];
long lengths[dim_count];
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

for (int k=0; k<dim_count; k++) {
  offsets[k] = (a[k] == vx_data2d_star) ? 0 : a[k];
  lengths[k] = 1;
}
lengths[y_slot] = Ny;

int type_id = GET_NC_TYPE_ID_P(v);
for (x=0; x<Nx; ++x)  {
   offsets[x_slot] = x;
   switch ( type_id )  {

      case NcType::nc_INT:
         get_nc_data(v, (int *)&i, lengths, offsets);
         for (y=0; y<Ny; ++y)  {
            d[y] = (double)i[y];
         }
         break;

      case NcType::nc_SHORT:
         get_nc_data(v, (short *)&s, lengths, offsets);
         for (y=0; y<Ny; ++y)  {
            d[y] = (double)s[y];
         }
         break;

      case NcType::nc_FLOAT:
         get_nc_data(v, (float *)&f, lengths, offsets);
         for (y=0; y<Ny; ++y)  {
            d[y] = (double)f[y];
         }
         break;

      case NcType::nc_DOUBLE:
         get_nc_data(v, (double *)&d, lengths, offsets);
         break;

      default:
         mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
              << " bad type for variable \"" << (GET_NC_NAME_P(v)) << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch


   b[x_slot] = x;

   for (y=0; y<Ny; ++y)  {
      value = d[y];

      if ( is_bad_data_pinterp( value ) ) {
         value = bad_data_double;
      }
      else if (add_offset != 0.0 || scale_factor != 1.0) {
         value = value * scale_factor + add_offset;
      }

      plane.set(value, x, y);

   }   //  for y

}   //  for x

   //
   //  get the pressure
   //

if ( P && z_slot > 0 )  {

   LongArray c;

   c.add(a[z_slot]);

   pressure = data(P->var, c) * hPaCF;

}

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool PinterpFile::data(const char * var_name, const LongArray & a, DataPlane & plane,
                       double & pressure, NcVarInfo *&info) const

{

int j, time_index;
bool found = false;

for (j=0; j<Nvars; ++j)  {

   if ( Var[j].name == var_name )  {
      found = true;
      info = &Var[j];
      break;
   }

}

if ( !found )  return ( false );

found = data(Var[j].var, a, plane, pressure);

   //
   //  store the times
   //

time_index = a[Var[j].t_slot];

plane.set_init  ( InitTime );
plane.set_valid ( valid_time(time_index) );
plane.set_lead  ( lead_time(time_index) );

   //
   //  since Pinterp files only contain WRF-ARW output, it is always a
   //  a runtime accumulation
   //

if ( is_accumulation(var_name) )  {

   plane.set_accum ( lead_time(time_index) );

} else  {

   plane.set_accum ( 0 );

}

   //
   //  done
   //

return ( found );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


unixtime parse_init_time(const char * s)

{

int j;
unixtime t;
int month, day, year, hour, minute, second;


j = sscanf(s, "%4d-%2d-%2d_%2d:%2d:%2d",
               &year, &month, &day, &hour, &minute, &second);

if ( j != 6 )  {

   mlog << Error << "\nparse_init_time(const char *) -> "
        << "bad time string ... \"" << s << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

t = mdyhms_to_unix(month, day, year, hour, minute, second);

return ( t );

}


////////////////////////////////////////////////////////////////////////


bool is_bad_data_pinterp(double v)

{

if ( v < pinterp_missing )  return ( false );
else                        return ( true  );

}


////////////////////////////////////////////////////////////////////////


bool is_accumulation(const char * s)

{

int j;
bool found = false;

for ( j=0; j<n_accum_var_names; ++j )  {

   if ( strcmp(s, accum_var_names[j]) == 0 )  { found = true;  break; }

}

return ( found );

}


////////////////////////////////////////////////////////////////////////
