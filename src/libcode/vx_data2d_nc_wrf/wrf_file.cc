// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

#include <netcdf>
using namespace netCDF;

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"
#include "vx_nc_util.h"

#include "wrf_file.h"
#include "get_wrf_grid.h"


////////////////////////////////////////////////////////////////////////


static const char x_dim_name           [] = "west_east";
static const char x_dim_stag_name      [] = "west_east_stag";
static const char x_dim_subgrid_name   [] = "west_east_subgrid";
static const char y_dim_name           [] = "south_north";
static const char y_dim_stag_name      [] = "south_north_stag";
static const char y_dim_subgrid_name   [] = "south_north_subgrid";
static const char t_dim_name           [] = "time";
static const char z_dim_p_interp_name  [] = "num_metgrid_levels";
static const char z_dim_wrf_interp_name[] = "vlevs";
static const char z_dim_wrf_stag_name  [] = "bottom_top_stag";
static const char z_dim_wrf_name       [] = "bottom_top";
static const char z_dim_wrf_pres_name  [] = "num_press_levels_stag";
static const char z_dim_wrf_z_name     [] = "num_z_levels_stag";
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
static const char pressure_var_wrf_name        [] = "P_PL";

static const char pa_units_str         [] = "Pa";
static const char hpa_units_str        [] = "hPa";

static const string start_time_att_name   = "START_DATE";

static const int max_wrf_args         = 30;

static const double wrf_missing       = 1.0e35;

static const char *accum_var_names     [] = { "ACGRDFLX", "CUPPT",
                                              "RAINC",    "RAINNC",
                                              "SNOWNC",   "GRAUPELNC",
                                              "ACHFX",    "ACLHF" };
static const int n_accum_var_names        = sizeof(accum_var_names)/sizeof(*accum_var_names);

////////////////////////////////////////////////////////////////////////


static unixtime parse_init_time(const char *);

static bool is_bad_data_wrf(double);

static bool is_accumulation(const char *);

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class WrfFile
   //


////////////////////////////////////////////////////////////////////////


WrfFile::WrfFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


WrfFile::~WrfFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void WrfFile::init_from_scratch()

{

Nc = (NcFile *) 0;

Dim = (NcDim **) 0;

Var = (NcVarInfo *) 0;

Time = (unixtime *) 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void WrfFile::close()

{

if ( Nc )  { delete Nc;  Nc = (NcFile *) 0; }

if ( Dim )  { delete [] Dim;  Dim = (NcDim **) 0; }

if ( Time )  { delete [] Time;  Time = (unixtime *) 0; }

Ndims = 0;

DimNames.clear();

Tdim = (NcDim *) 0;

Nvars = 0;

if ( Var )  { delete [] Var;  Var = (NcVarInfo *) 0; }

InitTime = (unixtime) 0;

Ntimes = 0;

PressureIndex = -1;

TimeInPressure = false;

hPaCF = 1.0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool WrfFile::open(const char * filename)

{

int j, k;
int month, day, year, hour, minute, second, str_len;
char time_str[max_str_len];
string c;
NcVar v;
const char *method_name = "WrfFile::open() -> ";

close();

Nc = open_ncfile(filename);
mlog << Debug(5) << "\n" << method_name
     << "open \"" << filename << "\".\n\n";

if ( IS_INVALID_NC_P(Nc) )  { close();  return ( false ); }

   //
   //  grid
   //

if ( !get_wrf_grid(*Nc, grid) )  { close();  return ( false ); }

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
      m_strncpy( time_str, tmp_time_str.c_str(), str_len, method_name, "time_str", true);
      time_str[str_len] = '\0';

      // Check for leading blank
      if(time_str[0] == ' ') {
         Time[j] = 0;
      }
      else {

         if(sscanf(time_str, "%4d-%2d-%2d_%2d:%2d:%2d",
                   &year, &month, &day, &hour, &minute, &second) != 6) {
            mlog << Error << "\n" << method_name
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
get_global_att(Nc, start_time_att_name, att_value);

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
           strcasecmp(GET_NC_NAME(v).c_str(), pressure_var_wrf_interp_name) == 0 ||
           strcasecmp(GET_NC_NAME(v).c_str(), pressure_var_wrf_name) == 0 ) {
         PressureIndex = j;
         if( strcasecmp(GET_NC_NAME(v).c_str(), pressure_var_wrf_name) == 0 ) {
            TimeInPressure = true;
         }
              if ( strcasecmp(Var[j].units_att.c_str(), pa_units_str ) == 0 ) hPaCF = 0.01;
         else if ( strcasecmp(Var[j].units_att.c_str(), hpa_units_str) == 0 ) hPaCF = 1.0;
      }


      dimNames.clear();
      get_dim_names(&v, &dimNames);
      string c;
      for (k=0; k<(dim_count); ++k)  {
        c = to_lower(dimNames[k]);
        NcDim dim = get_nc_dim(&v, dimNames[k]);
        Var[j].Dims[k] = &dim;

        if ( c.compare(x_dim_name) == 0 ) {
           Var[j].x_slot = k;
        }
        else if ( c.compare(x_dim_stag_name) == 0 ) {
           Var[j].x_slot = k;
           Var[j].x_stag = true;
        }
        else if ( c.compare(x_dim_subgrid_name) == 0 ) {
            mlog << Error << "\n" << method_name
                 << "X Dimension \"" << x_dim_subgrid_name << "\" is not supported.\n\n";
            return ( false );
        }
        else if ( c.compare(y_dim_name) == 0 ) {
           Var[j].y_slot = k;
        }
        else if ( c.compare(y_dim_stag_name) == 0 ) {
           Var[j].y_slot = k;
           Var[j].y_stag = true;
        }
        else if ( c.compare(y_dim_subgrid_name) == 0 ) {
            mlog << Error << "\n" << method_name
                 << "Y Dimension \"" << y_dim_subgrid_name << "\" is not supported.\n\n";
            return ( false );
        }
        else if ( c.compare(z_dim_p_interp_name  ) == 0 ||
                  c.compare(z_dim_wrf_interp_name) == 0 ||
                  c.compare(z_dim_wrf_name  ) == 0) {
           Var[j].z_slot = k;
           if ( c.compare(z_dim_wrf_name) != 0 ) {
              Var[j].is_pressure = true;
           }
        }
        else if ( c.compare(z_dim_wrf_stag_name) == 0 ||
                  c.compare(z_dim_wrf_pres_name) == 0 ||
                  c.compare(z_dim_wrf_z_name ) == 0) {
           Var[j].z_slot = k;
           Var[j].z_stag = true;
           if ( c.compare(z_dim_wrf_pres_name) == 0 ) {
              Var[j].is_pressure = true;
           }
        }
        else if ( c.compare(t_dim_name) == 0 ) {
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


void WrfFile::dump(ostream & out, int depth) const

{

int j, k;
int month, day, year, hour, minute, second;
char junk[256];
string c;
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

out << prefix << "Tdim = " << (Tdim ? GET_NC_NAME_P(Tdim) : "(nul)") << "\n";

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
      c = to_lower(DimNames[k]);
           if ( c.compare(x_dim_name) == 0 )           out << "X";
      else if ( c.compare(x_dim_stag_name) == 0 )      out << "X (staggered)";
      else if ( c.compare(y_dim_name) == 0 )           out << "Y";
      else if ( c.compare(y_dim_stag_name) == 0 )      out << "Y (staggered)";
      else if ( c.compare(z_dim_p_interp_name) == 0 ||
                c.compare(z_dim_wrf_interp_name) == 0 ||
                c.compare(z_dim_wrf_name) == 0)        out << "Z";
      else if ( c.compare(z_dim_wrf_stag_name) == 0 ||
                c.compare(z_dim_wrf_pres_name) == 0 ||
                c.compare(z_dim_wrf_z_name) == 0)      out << "Z (staggered)";
      else if ( Var[j].Dims[k] == Tdim )                  out << 'T';
      else                                                out << GET_NC_NAME_P(Var[j].Dims[k]);

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


unixtime WrfFile::valid_time(int n) const

{

if ( (n < 0) || (n >= Ntimes) )  {

   mlog << Error << "\nWrfFile::valid_time(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}


return ( Time [n] );

}


////////////////////////////////////////////////////////////////////////


int WrfFile::lead_time(int n) const

{

if ( (n < 0) || (n >= Ntimes) )  {

   mlog << Error << "\nWrfFile::lead_time(int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

unixtime dt = Time[n] - InitTime;

return ( (int) dt );

}


////////////////////////////////////////////////////////////////////////


double WrfFile::data(NcVar * var, const LongArray & a) const

{
const char *method_name = "WrfFile::data(NcVar *, const LongArray &) const -> ";
if ( !args_ok(a) )  {

   mlog << Error << "\n" << method_name << "bad arguments:\n";

   a.dump(cerr);

   exit ( 1 );

}

int dim_count = var->getDimCount();
if ( dim_count != a.n_elements() )  {

   mlog << Error << "\n" << method_name
        << "needed " << (dim_count) << " arguments for variable "
        << (GET_NC_NAME_P(var)) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if (dim_count >= max_wrf_args )  {

   mlog << Error << "\n" << method_name
        << " too may arguments for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";

   exit ( 1 );

}

bool status = false;
double fill_value;
double d = bad_data_double;
double missing_value = get_var_missing_value(var);
get_var_fill_value(var, fill_value);

status = get_nc_data(var, &d, a);

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


bool WrfFile::data(NcVar * v, const LongArray & a, DataPlane & plane, double & pressure) const

{
const char *method_name = "WrfFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> ";
if ( !args_ok(a) )  {

   mlog << Warning << "\n" << method_name << "bad arguments:\n";

   a.dump(cerr);

   return ( false );

}

string var_name = GET_NC_NAME_P(v);
int dim_count = v->getDimCount();
if ( dim_count != a.n_elements() )  {

   mlog << Warning << "\n" << method_name
        << "needed " << dim_count << " arguments for variable "
        << (var_name) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if (dim_count >= max_wrf_args )  {

   mlog << Warning << "\n" << method_name
        << " too may arguments for variable \"" << (var_name) << "\"\n\n";

   return ( false );

}


int j, count;
int x, y;
double value;
bool found = false;
NcVarInfo * var = (NcVarInfo *) 0;
NcVarInfo * P   = (NcVarInfo *) 0;
LongArray b = a;

pressure = bad_data_double;

   //
   //  find varinfo's
   //

found = false;

for (j=0; j<Nvars; ++j)  {

   if ( Var[j].var == v )  { found = true;  var = Var + j;  break; }

}

if ( !found )  {

   mlog << Warning << "\n" << method_name
        << "variable " << (var_name) << " not found!\n\n";

   return ( false );

}

   //
   // get pressure only if var is on pressure levels and pressure is in file
   //
   if ( var->is_pressure && PressureIndex >= 0 )  P = Var + PressureIndex;

   //
   // set nx and ny based on staggering of dimensions of the variable to read
   //
   const int Nx = var->x_stag ? grid.nx() + 1 : grid.nx();
   const int Ny = var->y_stag ? grid.ny() + 1 : grid.ny();

   //
   //  check x_slot and y_slot
   //

if ( var == nullptr || (var->x_slot < 0) || (var->y_slot < 0) )  {

   mlog << Error << "\n" << method_name
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

         mlog << Warning << "\n" << method_name << " star found in bad slot\n\n";

         return ( false );

      }

   }

}

if ( count != 2 )  {

   mlog << Warning << "\n" << method_name << " bad star count ... " << count << "\n\n";

   return ( false );

}

   //
   //  check slots
   //

const int x_slot = var->x_slot;
const int y_slot = var->y_slot;
const int z_slot = var->z_slot;

if ( (x_slot < 0) || (y_slot < 0) )  {

   mlog << Warning << "\n" << method_name << " bad x|y slot\n\n";

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

LongArray offsets;
LongArray lengths;

for (int k=0; k<dim_count; k++) {
  offsets.add((a[k] == vx_data2d_star) ? 0 : a[k]);
  lengths.add(1);
}
lengths[y_slot] = Ny;

int type_id = GET_NC_TYPE_ID_P(v);
for (x=0; x<Nx; ++x)  {
   offsets[x_slot] = x;
   get_nc_data(v, (double *)&d, lengths, offsets);

   b[x_slot] = x;

   for (y=0; y<Ny; ++y)  {
      value = d[y];

      if (is_bad_data_wrf(value) ) {
         value = bad_data_double;
      }

      plane.set(value, x, y);

   }   //  for y

}   //  for x

// de-stagger the DataPlane if necessary
plane.destagger(var->x_stag, var->y_stag);

   //
   //  get the pressure
   //

if ( P && z_slot > 0 )  {

   LongArray c;

   if(TimeInPressure) c.add(a[var->t_slot]);
   
   c.add(a[z_slot]);

   pressure = data(P->var, c) * hPaCF;

}

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool WrfFile::data(const char * var_name, const LongArray & a, DataPlane & plane,
                   double & pressure, NcVarInfo *&info) const {

   int time_index;
   bool found = false;

   if (nullptr != info) found = true;
   else found = get_nc_var_info(var_name, info);

   if ( !found )  return ( false );

   found = data(info->var, a, plane, pressure);

   //
   //  store the times
   //

   time_index = a[info->t_slot];

   plane.set_init  ( InitTime );
   plane.set_valid ( valid_time(time_index) );
   plane.set_lead  ( lead_time(time_index) );

   //
   //  since Pinterp files only contain WRF-ARW output, it is always
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

bool WrfFile::get_nc_var_info(const char *var_name, NcVarInfo *&info) const {
   bool found = false;

   if (nullptr == info) {
      for (int j=0; j<Nvars; ++j)  {

         if ( Var[j].name == var_name )  {
            found = true;
            info = &Var[j];
            break;
         }

      }
   }

   return found;
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


bool is_bad_data_wrf(double v)

{

if (v < wrf_missing )  return ( false );
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
