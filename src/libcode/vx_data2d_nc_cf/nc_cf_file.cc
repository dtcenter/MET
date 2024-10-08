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
#include <string>
#include <time.h>

#include <netcdf>

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "nc_cf_file.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////


static const int  max_met_args              = 30;
static const double DELTA_TOLERANCE_PERCENT = 0.05;
static const double EARTH_MAJOR_AXIS_km = 6371.20;

const double NcCfFile::DELTA_TOLERANCE = 15.0;

static const char grid_mapping_name_geostationary[] = "geostationary";
static const char x_dim_key_name[] = "projection_x_coordinate";
static const char y_dim_key_name[] = "projection_y_coordinate";
static ConcatString t_dim_name = (string)"Time";

static const char dim_lat_nt[] = "nlat";
static const char dim_lon_nt[] = "nlon";
static const char var_lat_nt[] = "tlat";
static const char var_lon_nt[] = "tlon";

static ConcatString x_dim_var_name;
static ConcatString y_dim_var_name;

static double get_nc_var_att_double(const NcVar *nc_var, const char *att_name,
                                    bool is_required=true);

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class NcCfFile
   //


////////////////////////////////////////////////////////////////////////


NcCfFile::NcCfFile()
{
  init_from_scratch();
}


////////////////////////////////////////////////////////////////////////


NcCfFile::~NcCfFile()
{
  close();
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::init_from_scratch()

{
  // Initialize the pointers

  _ncFile = (NcFile *) nullptr;
  _dims = (NcDim **) nullptr;
  Var = (NcVarInfo *) nullptr;
  _time_var_info = (NcVarInfo *)nullptr;

  _xDim = (NcDim *)nullptr;
  _yDim = (NcDim *)nullptr;
  _tDim = (NcDim *)nullptr;
  _latVar = (NcVar *)nullptr;
  _lonVar = (NcVar *)nullptr;
  _xCoordVar = (NcVar *)nullptr;
  _yCoordVar = (NcVar *)nullptr;

  // Close any existing file

  close();

  return;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::close()
{

  // Reclaim the file pointer

  if (_ncFile)
  {
    delete _ncFile;
    _ncFile = (NcFile *)nullptr;
  }

  // Reclaim the dimension pointers

  if (_dims)
  {
    delete [] _dims;
    _dims = (NcDim **)nullptr;
  }

  grid_ready = false;
  has_attr_grid = false;

  _numDims = 0;

  _dimNames.clear();

  _xDim = _yDim = _tDim = (NcDim *) nullptr;

  // Reclaim the variable pointers

  if (Var)
  {
    delete [] Var;
    Var = (NcVarInfo *)nullptr;
  }

  Nvars = 0;

  // Reset the time values

  ValidTime.clear();
  raw_times.clear();
  vlevels.clear();
  InitTime = (unixtime)0;
  AccumTime = (unixtime)0;

  //  done

  return;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::open(const char * filepath)
{
  unixtime ut;
  int sec_per_unit;
  const char *method_name = "NcCfFile::open() -> ";

  // Close any open files and clear out the associated members

  close();

  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program. In the case of this example, we just exit with
  // an NC_ERR error code.

  // FIXME: Commented out with NetCDF4 enabling
  // NcError err(NcError::silent_nonfatal);

  // Open the file
  _ncFile = open_ncfile(filepath);

  if (IS_INVALID_NC_P(_ncFile))
  {
    close();
    return false;
  }

  // Pull out the dimensions
  _numDims = get_dim_count(_ncFile);
  _dims = new NcDim*[_numDims];

  StringArray gDimNames;
  get_dim_names(_ncFile, &gDimNames);

  for (int j=0; j<_numDims; ++j)  {
     NcDim dim = get_nc_dim(_ncFile, gDimNames[j]);
     _dims[j] = new NcDim(dim);
  }

  // Pull out the variables

  int max_dim_count = 0;
  auto z_var = (NcVar *)nullptr;
  auto valid_time_var = (NcVar *)nullptr;
  ConcatString att_value;

  StringArray varNames;
  Nvars = get_var_names(_ncFile, &varNames);
  Var = new NcVarInfo [Nvars];

  for (int j=0; j<Nvars; ++j)  {
    NcVar v = get_var(_ncFile, varNames[j].c_str());

    Var[j].var = new NcVar(v);

    Var[j].name = GET_NC_NAME(v).c_str();

    int dim_count = GET_NC_DIM_COUNT(v);
    Var[j].Ndims = dim_count;
    if (dim_count > max_dim_count) max_dim_count = dim_count;

    Var[j].Dims = new NcDim * [dim_count];

    //  parse the variable attributes
    get_att_str( Var[j], long_name_att_name, Var[j].long_name_att );
    get_att_str( Var[j], units_att_name,     Var[j].units_att     );

    if (get_var_axis(Var[j].var, att_value)) {
      if ( "T" == att_value ||  "time" == att_value ) {
        valid_time_var = Var[j].var;
        _time_var_info = &Var[j];
      }
      else if ( "Z" == att_value ||  "z" == att_value ) {
        z_var = Var[j].var;
      }
    }

    if (get_var_standard_name(Var[j].var, att_value)) {
      if ( "time" == att_value ) {
        valid_time_var = Var[j].var;
        _time_var_info = &Var[j];
      }
      else if( "latitude" == att_value ) _latVar = Var[j].var;
      else if( "longitude" == att_value ) _lonVar = Var[j].var;
      else if( ("air_pressure" == att_value || "height" == att_value)
               && (nullptr==z_var && 1==get_dim_count(Var[j].var))) z_var = Var[j].var;
    }
    if ( Var[j].name == "time" && (valid_time_var == nullptr)) {
      valid_time_var = Var[j].var;
      _time_var_info = &Var[j];
    }
  }   //  for j

  if (nullptr == _time_var_info) {
    for (int j=0; j<Nvars; ++j)  {
      if (is_nc_unit_time(Var[j].units_att.c_str())) {
        valid_time_var = Var[j].var;
        _time_var_info = &Var[j];
        break;
      }
    }
  }   //  for j


  // Pull out the valid and init times
  ConcatString units;
  if (IS_INVALID_NC_P(valid_time_var))
  {

    mlog << Debug(4) << method_name
         << "could not extract valid time from the "
         << "\"time\" variable.\n";

    // Time not in file, get from file name
    if ((ut = get_valid_time_from_file_path(filepath)) == 0)
    {
       mlog << Debug(4) << method_name
            << "could not extract valid time from file name.\n";
    }
    if( ut == 0 ) {
       mlog << Warning << "\n" << method_name
            << "could not determine the valid time, using 0.\n\n";
    }
    ValidTime.add(ut);
  }
  else
  {
    // Store the dimension for the time variable as the time dimension
    int time_dim_count = get_dim_count(valid_time_var);
    if (time_dim_count == 1) {
       NcDim tDim = get_nc_dim(valid_time_var, 0);
       if (IS_VALID_NC(tDim)) {
         _tDim = new NcDim(tDim);
         t_dim_name = GET_NC_NAME(tDim).c_str();
       }
    }

    // Parse the units for the time variable.
    ut = sec_per_unit = 0;
    if (get_var_units(valid_time_var, units)) {
      if (units.empty()) {
         mlog << Warning << "\n" << method_name
              << "the \"time\" variable must contain a \"units\" attribute. "
              << "Using valid time of 0\n\n";
      }
      else {
         mlog << Debug(4) << method_name
              << "parsing units for the time variable \"" << units << "\"\n";
         parse_cf_time_string(units.c_str(), ut, sec_per_unit);
      }
    }

    NcVar bounds_time_var;
    auto nc_time_var = (NcVar *)nullptr;
    bool use_bounds_var = false;
    ConcatString bounds_var_name;
    nc_time_var = valid_time_var;
    NcVarAtt *bounds_att = get_nc_att(valid_time_var, bounds_att_name, false);
    if (get_att_value_chars(bounds_att, bounds_var_name)) {
      bounds_time_var = get_nc_var(_ncFile, bounds_var_name.c_str());
      use_bounds_var = IS_VALID_NC(bounds_time_var);
      if (use_bounds_var) {
        nc_time_var = &bounds_time_var;
        mlog << Debug(3) << method_name
             << "read time from the bounds variable \"" << bounds_var_name << "\"\n";
      }
    }
    if (bounds_att) delete bounds_att;

    // Determine the number of times present.
    int n_times = get_data_size(valid_time_var);
    int tim_buf_size = n_times;
    if (use_bounds_var) tim_buf_size *= 2;
    vector<double> time_values(tim_buf_size);

    if( get_nc_data(nc_time_var, time_values) ) {
      bool no_leap_year = get_att_no_leap_year(valid_time_var);
      if( time_dim_count > 1 ) {
        double latest_time = bad_data_double;
        for(int i=0; i<n_times; i++) {
          if( latest_time < time_values[i] ) latest_time = time_values[i];
        }
        ValidTime.add(add_to_unixtime(ut, sec_per_unit, latest_time, no_leap_year));
        raw_times.add(latest_time);
      }
      else {
        if (use_bounds_var) {
          double bounds_diff;
          for(int i=0; i<n_times; i++) {
            ValidTime.add(add_to_unixtime(ut, sec_per_unit, time_values[i*2+1], no_leap_year));
            raw_times.add(time_values[i*2+1]);
            bounds_diff = time_values[i*2+1] - time_values[i*2];
            if (abs(bounds_diff - nint(bounds_diff)) < TIME_EPSILON) {
              AccumTime = (unixtime)(sec_per_unit * nint(bounds_diff));
            }
            else {
              AccumTime = (unixtime)(sec_per_unit * bounds_diff);
            }
          }
        }
        else {
          for(int i=0; i<n_times; i++) {
            ValidTime.add(add_to_unixtime(ut, sec_per_unit, time_values[i], no_leap_year));
            raw_times.add(time_values[i]);
          }
        }
      }
    }
    else ValidTime.add(0);  //Initialize
  }

  NcVar init_time_var = get_var(_ncFile, "forecast_reference_time");
  if (IS_INVALID_NC(init_time_var))
  {

    mlog << Debug(4) << method_name
         << "could not extract init time from the "
         << "\"forecast_reference_time\" variable.\n";

    // Time not in file, get from file name
    if ((InitTime = get_init_time_from_file_path(filepath)) == 0)
    {
      mlog << Debug(4) << method_name
           << "could not extract init time from file name.\n";
    }
  }
  else
  {

    // Parse the units for the time variable.
    if (get_var_units(&init_time_var, units)) {
      if (units.empty()) {
         mlog << Warning << "\n" << method_name
              << "the \"forecast_reference_time\" variable must contain a \"units\" attribute.\n\n";
         ut = sec_per_unit = 0;
      }
      else {
         mlog << Debug(4) << method_name
              << "parsing units for the forecast_reference_time variable \"" << units << "\"\n";
         parse_cf_time_string(units.c_str(), ut, sec_per_unit);
      }
    }
    else {
      ut = sec_per_unit = 0;
    }

    double time_value = get_nc_time(&init_time_var,0);
    InitTime = ut + (unixtime)(sec_per_unit * time_value);
  }

  // Pull out the grid.  This must be done after pulling out the dimension
  // and variable information since this information is used to pull out the
  // grid.  This call sets the _xDim and _yDim pointers.

  read_netcdf_grid();

  // Now go back through the variables and use _xDim, _yDim, and _tDim
  // to set the slots.
  // Should be called after read_netcdf_grid() is called

  string z_dim_name;
  StringArray z_dims;
  StringArray t_dims;
  StringArray dimNames;
  string var_x_dim_name;
  string var_y_dim_name;
  if (IS_VALID_NC_P(_xDim)) var_x_dim_name = GET_NC_NAME_P(_xDim);
  if (IS_VALID_NC_P(_yDim)) var_y_dim_name = GET_NC_NAME_P(_yDim);
  for (int j=0; j<Nvars; ++j) {

    int dim_count = Var[j].Ndims;
    NcVar *v = Var[j].var;

    dimNames.clear();
    get_dim_names(v, &dimNames);

    for (int k=0; k<dim_count; ++k)  {
      NcDim *dim = Var[j].Dims[k];
      const ConcatString dim_name = dimNames[k];
      if      ((dim && dim == _xDim) || dim_name == var_x_dim_name || dim_name == x_dim_var_name) {
         Var[j].x_slot = k;
      }
      else if ((dim && dim == _yDim) || dim_name == var_y_dim_name || dim_name == y_dim_var_name) {
         Var[j].y_slot = k;
      }
      else if ((dim && (dim == _tDim)) || dim_name == t_dim_name || t_dims.has(dim_name)) {
         Var[j].t_slot = k;
      }
      else if (z_dims.has(dim_name)) {
         Var[j].z_slot = k;
      }
      else if (dim_count == max_dim_count) {
         NcVarInfo *info = find_var_by_dim_name(dim_name.c_str());
         if (info) {
            if (is_nc_unit_time(info->units_att.c_str())) {
               Var[j].t_slot = k;
               t_dims.add(dim_name);
            }
            else if (is_nc_unit_latitude(info->units_att.c_str())) {
               Var[j].y_slot = k;
            }
            else if (is_nc_unit_longitude(info->units_att.c_str())) {
               Var[j].x_slot = k;
            }
            else {
               Var[j].z_slot = k;
               z_dims.add(dim_name);
               if (0 == z_dim_name.length()) z_dim_name = dim_name;
            }
         }
      }
    }
  }   //  for j

  // Find the vertical level variable from dimension name if not found
  if (IS_INVALID_NC_P(z_var) && (0 < z_dim_name.length())) {
    NcVarInfo *info = find_var_by_dim_name(z_dim_name.c_str());
    if (info) z_var = info->var;
  }

  mlog << Debug(5) << method_name << "coordinate variables:"
       << " x=" << (IS_VALID_NC_P(_xCoordVar) ? GET_NC_NAME_P(_xCoordVar) : "N/A")
       << ", y=" << (IS_VALID_NC_P(_yCoordVar) ? GET_NC_NAME_P(_yCoordVar) : "N/A")
       << ", z=" << (IS_VALID_NC_P(z_var) ? GET_NC_NAME_P(z_var) : "N/A")
       << ", t=" << (IS_VALID_NC_P(valid_time_var) ? GET_NC_NAME_P(valid_time_var) : "N/A")
       << "\n";

  // Pull out the vertical levels
  if (IS_VALID_NC_P(z_var)) {

    int z_count = get_data_size(z_var);
    vector<double> z_values(z_count);

    if( get_nc_data(z_var, z_values) ) {
      for(int i=0; i<z_count; i++) {
        vlevels.add(z_values[i]);
      }
    }
  }

  //  done

  return true;
}


////////////////////////////////////////////////////////////////////////


unixtime NcCfFile::get_valid_time_from_file_path(const string &filepath) const
{
  // Extract the file name from the path

  string filename;
  size_t slash_pos = filepath.rfind('/');

  if (slash_pos == string::npos)
    filename = filepath;
  else
    filename = filepath.substr(slash_pos+1);

  // See if this is a TRMM 3hourly precip file

  unixtime file_time;

  if ((file_time = get_time_from_TRMM_3B42_3hourly_filename(filename)) != 0)
    return file_time + (int)(1.5 * 3600.0);

  // See if this is a TRMM daily precip file

  if ((file_time = get_time_from_TRMM_3B42_daily_filename(filename)) != 0)
    return file_time + (int)(22.5 * 3600.0);

  // If we get here, we couldn't get the time from the filename

  return 0;
}


////////////////////////////////////////////////////////////////////////


unixtime NcCfFile::get_init_time_from_file_path(const string &filepath) const
{
  // Extract the file name from the path

  string filename;
  size_t slash_pos = filepath.rfind('/');

  if (slash_pos == string::npos)
    filename = filepath;
  else
    filename = filepath.substr(slash_pos+1);

  // See if this is a TRMM 3hourly precip file

  unixtime file_time;

  if ((file_time = get_time_from_TRMM_3B42_3hourly_filename(filename)) != 0)
    return file_time - (int)(1.5 * 3600.0);

  // See if this is a TRMM daily precip file

  if ((file_time = get_time_from_TRMM_3B42_daily_filename(filename)) != 0)
    return file_time - (int)(1.5 * 3600.0);

  // If we get here, we couldn't get the time from the filename

  return 0;
}


////////////////////////////////////////////////////////////////////////


unixtime NcCfFile::get_time_from_TRMM_3B42_3hourly_filename(const string &filename) const
{
  // The format of the TRMM 3hourly files is:
  //      3B42.<yyyymmdd>.<hh>.7.G3.nc
  // See if the tokens in this filename seem to match

  ConcatString fn_string(filename.c_str());
  StringArray tokens = fn_string.split(".");

  if (tokens.n_elements() != 6)
    return 0;

  // 3B42

  if ( tokens[0] != "3B42" )
    return 0;

  // <yyyymmdd>

  if ( tokens[1].length() != 8)
    return 0;

  for (int i = 0; i < 8; ++i)
  {
    if (!isdigit(tokens[1][i]))
      return 0;
  }

  // <hh>

  if (tokens[2].length() != 2)
    return 0;

  for (int i = 0; i < 2; ++i)
  {
    if (!isdigit(tokens[2][i]))
      return 0;
  }

  // 7

  if ( tokens[3] != "7" )
    return 0;

  // G3

  if ( tokens[4] != "G3" )
    return 0;

  // nc

  if ( tokens[5] != "nc" )
    return 0;

  // If we get here, this is a TRMM 3B42 3hourly file.  Extract the file time.

  string date_string = tokens[1];
  string hour_string = tokens[2];

  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));

  time_struct.tm_year = atoi(date_string.substr(0, 4).c_str()) - 1900;
  time_struct.tm_mon = atoi(date_string.substr(4, 2).c_str()) - 1;
  time_struct.tm_mday = atoi(date_string.substr(6, 2).c_str());
  time_struct.tm_hour = atoi(hour_string.c_str());

  return (unixtime)timegm(&time_struct);
}


////////////////////////////////////////////////////////////////////////


unixtime NcCfFile::get_time_from_TRMM_3B42_daily_filename(const string &filename) const
{
  // The format of the TRMM 3hourly files is:
  //      3B42_daily.<yyyy>.<mm>.<dd>.7.G3.nc
  // See if the tokens in this filename seem to match

  ConcatString fn_string(filename.c_str());
  StringArray tokens = fn_string.split(".");

  if (tokens.n_elements() != 7)
    return 0;

  // 3B42_daily

  if ( tokens[0] != "3B42_daily" )
    return 0;

  // <yyyy>

  if ( tokens[1].length() != 4)
    return 0;

  for (int i = 0; i < 4; ++i)
  {
    if (!isdigit(tokens[1][i]))
      return 0;
  }

  // <mm>

  if (tokens[2].length() != 2)
    return 0;

  for (int i = 0; i < 2; ++i)
  {
    if (!isdigit(tokens[2][i]))
      return 0;
  }

  // <dd>

  if ( tokens[3].length() != 2)
    return 0;

  for (int i = 0; i < 2; ++i)
  {
    if (!isdigit(tokens[3][i]))
      return 0;
  }

  // 7

  if ( tokens[4] != "7" )
    return 0;

  // G3

  if ( tokens[5] != "G3" )
    return 0;

  // nc

  if ( tokens[6] != "nc" )
    return 0;

  // If we get here, this is a TRMM 3B42 daily file.  Extract the file time.

  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));

  time_struct.tm_year = atoi(tokens[1].c_str()) - 1900;
  time_struct.tm_mon = atoi(tokens[2].c_str()) - 1;
  time_struct.tm_mday = atoi(tokens[3].c_str());

  return (unixtime)timegm(&time_struct);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::dump(ostream & out, int depth) const
{
  char junk[256];

  Indent prefix(depth);
  Indent p2(depth + 1);
  Indent p3(depth + 2);

  out << prefix << "Grid ...\n";

  grid.dump(out, depth + 1);

  out << prefix << "\n";

  out << prefix << "Nc = " << (_ncFile ? "ok" : "(nul)") << "\n";

  out << prefix << "\n";

  out << prefix << "Ndims = " << _numDims << "\n";

  for (int j = 0; j < _numDims; ++j)
  {
    out << p2 << "Dim # " << j << " = " << _dimNames[j] << "   ("
        << (GET_NC_SIZE_P(_dims[j])) << ")\n";
  }   //  for j

  out << prefix << "\n";

  out << prefix << "Xdim = " << (_xDim ? GET_NC_NAME_P(_xDim) : "(nul)") << "\n";
  out << prefix << "Ydim = " << (_yDim ? GET_NC_NAME_P(_yDim) : "(nul)") << "\n";
  out << prefix << "Tdim = " << (_tDim ? GET_NC_NAME_P(_tDim) : "(nul)") << "\n";

  out << prefix << "\n";

  out << prefix << "Init Time = ";

  int month, day, year, hour, minute, second;

  unix_to_mdyhms(InitTime, month, day, year, hour, minute, second);

  snprintf(junk, sizeof(junk), "%s %d, %d   %2d:%02d:%02d",
          short_month_name[month], day, year, hour, minute, second);

  out << junk << "\n";

  out << prefix << "\n";

  if (AccumTime > 0) {
    unix_to_mdyhms(AccumTime, month, day, year, hour, minute, second);
    snprintf(junk, sizeof(junk), "%2d:%02d:%02d (%d seconds)",
             hour, minute, second, (int)AccumTime);
    out << prefix << "Accum Time = ";
    out << junk << "\n";
    out << prefix << "\n";
  }

  out << prefix << "\n";

  out << prefix << "Nvars = " << Nvars << "\n";

  for (int j = 0; j < Nvars; ++j)
  {
    out << p2 << "Var # " << j << " = " << (Var[j].name) << "  (";

    for (int k = 0; k < Var[j].Ndims; ++k)
    {
      if (Var[j].Dims[k] == _xDim)
        out << 'X';
      else if (Var[j].Dims[k] == _yDim)
        out << 'Y';
      else if (Var[j].Dims[k] == _tDim)
        out << 'T';
      else
        out << GET_NC_NAME_P(Var[j].Dims[k]);

      if (k < Var[j].Ndims - 1)
        out << ", ";
    }   //  for k

    out << ")\n";

    out << p3 << "Slots (X, Y, Z, T) = (";

    if (Var[j].x_slot >= 0)
      out << Var[j].x_slot;
    else
      out << '_';  out << ", ";

    if (Var[j].y_slot >= 0)
      out << Var[j].y_slot;
    else
      out << '_';

    out << ")\n";

    out << p2 << "\n";

  }   //  for j

  //  done

  out.flush();

  return;
}


////////////////////////////////////////////////////////////////////////


int NcCfFile::lead_time() const
{
  unixtime dt = ValidTime[0] - InitTime;

  return (int) dt;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::update_grid(const Grid &attr_grid)
{
  static const string method_name = "NcCfFile::update_grid(Grid &) -> ";
  has_attr_grid = attr_grid.is_set();
  if (has_attr_grid) {
    grid = attr_grid;
    grid_ready = true;
    mlog << Debug(3) << method_name << "Override grid from set_attr_grid\n";
  }

  if (!grid_ready)
  {
    mlog << Error << "\n" << method_name << "Grid is not ready\n\n";
    exit(1);
  }
  return grid_ready;
}


////////////////////////////////////////////////////////////////////////


double NcCfFile::getData(NcVar * var, const LongArray & a) const
{
  clock_t start_clock = clock();
  static const string method_name
      = "NcCfFile::getData(NcVar *, const LongArray &) -> ";
  if (!args_ok(a))
  {
    mlog << Error << "\n" << method_name
         << "bad arguments:\n";
    a.dump(cerr);
    exit(1);
  }

  int dim_count = get_dim_count(var);
  if (dim_count != a.n_elements())
  {
    mlog << Error << "\n" << method_name
         << "needed " << dim_count << " arguments for variable "
         << (GET_NC_NAME_P(var)) << ", got " << (a.n_elements()) << "\n\n";
    exit(1);
  }

  if (dim_count >= max_met_args)
  {
    mlog << Error << "\n" << method_name
         << "too may arguments for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";
    exit(1);
  }

  for (int k=0; k<dim_count; k++) {
    int dim_size = var->getDim(k).getSize();
    if (dim_size < a[k]) {
      mlog << Error << "\n" << method_name
           << "offset (" << a[k] << ") at " << k
           << "th dimension (" << long(dim_size) << ") is too big for variable \""
           << GET_NC_NAME_P(var) << "\"\n\n";
      exit ( 1 );
    }
  }

  bool status = false;
  double d = bad_data_double;

  double fill_value;
  //double missing_value = get_var_missing_value(var);
  get_var_fill_value(var, fill_value);

  status = get_nc_data_ptr(var, &d, a);

  if (!status)
  {
    mlog << Error << "\n" << method_name
         << "bad status for var->get()\n\n";
    exit(1);
  }

  //  done

  mlog << Debug(6) << method_name << "took "
       << get_exe_duration(start_clock) << " seconds\n";

  return d;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::getData(NcVar * v, const LongArray & a, DataPlane & plane) const
{
  clock_t start_clock = clock();
  static const string method_name_short
      = "NcCfFile::getData(NcVar*, LongArray&, DataPlane&) ";
  static const string method_name
      = "NcCfFile::getData(NcVar *, const LongArray &, DataPlane &) const -> ";

  if (!args_ok(a))
  {
    mlog << Error << "\n" << method_name
         << "bad arguments:\n";
    a.dump(cerr);
    exit(1);
  }

  int dim_count = get_dim_count(v);
  if (dim_count != a.n_elements())
  {
    mlog << Error << "\n" << method_name
         << "needed " << dim_count << " arguments for variable "
         << (GET_NC_NAME_P(v)) << ", got " << (a.n_elements()) << "\n\n";
    exit(1);
  }

  if (dim_count >= max_met_args)
  {
    mlog << Error << "\n" << method_name
         << "too may arguments for variable \"" << (GET_NC_NAME_P(v)) << "\"\n\n";
    exit(1);
  }

  //  find varinfo's

  bool found = false;
  auto var = (NcVarInfo *)nullptr;

  for (int j = 0; j < Nvars; ++j)
  {
    if (Var[j].var == v)
    {
      found = true;
      var = Var + j;
      break;
    }
  }

  if (!found)
  {
    mlog << Error << "\n" << method_name
         << "variable " << GET_NC_NAME_P(v) << " not found!\n\n";
    exit(1);
  }

  //  check star positions and count

  int count = 0;

  for (int j = 0; j < a.n_elements(); ++j)
  {
    if (a[j] != vx_data2d_star) continue;

    ++count;
    if ( var == nullptr || ((j != var->x_slot) && (j != var->y_slot)) )
    {
      if (has_attr_grid) {
        mlog << Debug(3) << "\n" << method_name
             << "star found in unknown slot (" << j << ") for " << GET_NC_NAME_P(v) << "\n\n";
      }
      else {
        mlog << Error << "\n" << method_name
             << "star found in bad slot (" << j << ") for " << GET_NC_NAME_P(v) << "\n\n";

        exit(1);
      }
    }
  }

  if (count != 2)
  {
    mlog << Error << "\n" << method_name
         << "bad star count ... " << count << "\n\n";
    exit(1);
  }

  //  check slots - additional logic to satisfy Fortify Null Dereference
  int x_slot_tmp = 0;
  int y_slot_tmp = 0;
  if (var == nullptr || var->x_slot < 0 || var->y_slot < 0)
  {
    if (has_attr_grid) {
      mlog << Warning << "\n" << method_name
           << "bad x|y|z slot (" << var->x_slot << "|" << var->y_slot
           << "|" << var->z_slot << "|" << var->t_slot <<")\n\n";
      x_slot_tmp = dim_count - 1;
      y_slot_tmp = dim_count - 2;
    }
    else {
      mlog << Error << "\n" << method_name
           << "bad x|y|z slot\n\n";
      exit(1);
    }
  }
  else {
    x_slot_tmp = var->x_slot;
    y_slot_tmp = var->y_slot;
  }

  const int x_slot = x_slot_tmp;
  const int y_slot = y_slot_tmp;

  //
  //  get the bad data values
  //

  double fill_value;
  double missing_value = get_var_missing_value(v);
  get_var_fill_value(v, fill_value);

  //  set up the DataPlane object

  const int nx = grid.nx();
  const int ny = grid.ny();

  size_t data_size = 1;
  for (int k=0; k<dim_count; k++) {
    if (a[k] == vx_data2d_star) data_size *= v->getDim(k).getSize();
  }
  if (data_size == 1) data_size = v->getDim(x_slot).getSize() * v->getDim(y_slot).getSize();
  if (!is_eq(data_size, (size_t)nx*ny)) {
    mlog << Error << "\n" << method_name
         << "Allocated DataPlane from Grid (" << nx*ny << ") does not match with the variable size ("
         << data_size << "). Please check set_attr_grid settings (nx and ny) if applied.\n\n";
    exit(1);
  }

  plane.clear();
  plane.set_size(nx, ny);

  int y_offset;
  bool swap_to_north = grid.get_swap_to_north();
  if (swap_to_north) {
    mlog << Debug(2) << "\n" << method_name << "data was flipped to north.\n";
  }

  //  get the data
  const int plane_size = nx * ny;
  vector<double> d(plane_size);

  size_t dim_size;
  LongArray offsets;
  LongArray lengths;
  for (int k=0; k<dim_count; k++) {
    offsets.add((a[k] == vx_data2d_star) ? 0 : a[k]);
    lengths.add(1);
    dim_size = v->getDim(k).getSize();
    if (dim_size < offsets[k]) {
      mlog << Error << "\n" << method_name
           << "offset (" << offsets[k] << ") at " << k
           << "th dimension (" << long(dim_size) << ") is too big for variable \""
           << GET_NC_NAME_P(v) << "\"\n\n";
      exit ( 1 );
    }
  }

  offsets[x_slot] = 0;
  lengths[x_slot] = nx;
  offsets[y_slot] = 0;
  lengths[y_slot] = ny;

  get_nc_data(v, d, lengths, offsets);

  int offset = 0;
  if( x_slot > y_slot ) {
    for (int y=0; y<ny; ++y) {
      y_offset = y;
      if (swap_to_north) y_offset = ny - 1 - y;

      for (int x = 0; x< nx; ++x) {
        double value = d[offset++];

        if( is_eq(value, missing_value) || is_eq(value, fill_value) ) {
           value = bad_data_double;
        }

        plane.set(value, x, y_offset);

      } /*  for y */
    }   /*  for x */
  }
  else {
    for (int x = 0; x< nx; ++x) {
      for (int y=0; y<ny; ++y) {
        y_offset = y;
        if (swap_to_north) y_offset = ny - 1 - y;

        double value = d[offset++];

        if( is_eq(value, missing_value) || is_eq(value, fill_value) ) {
           value = bad_data_double;
        }

        plane.set(value, x, y_offset);

      } /*  for y */
    }   /*  for x */
  }

  //  done
  mlog << Debug(6) << method_name << "took "
       << get_exe_duration(start_clock) << " seconds\n";

  return true;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::getData(const char *var_name,
                       const LongArray &a, DataPlane &plane,
                       NcVarInfo *&info) const
{
  info = find_var_name(var_name);
  if (info == nullptr)
    return false;

  bool found = getData(info->var, a, plane);

  //  store the times
  unixtime valid_ut;
  if(info->t_slot >= 0) valid_ut = ValidTime[a[info->t_slot]];
  else                  valid_ut = ValidTime[0];

  //  if unset, set the init time to the valid time
  unixtime init_ut = InitTime;
  if(init_ut == 0 && valid_ut != 0) {
     mlog << Debug(4) << "NcCfFile::getData() -> "
          << "setting the unset init time to the valid time of "
          << unix_to_yyyymmdd_hhmmss(valid_ut) << ".\n";
     init_ut = valid_ut;
  }

  unixtime accum_time = info->AccumTime;
  if ((0 == accum_time) && (AccumTime>0)) accum_time = AccumTime;

  plane.set_init(init_ut);
  plane.set_valid(valid_ut);
  plane.set_lead(valid_ut - init_ut);
  plane.set_accum(accum_time);

  //  done

  return found;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::find_xy_vars(const string &caller_name) {

  // Look for the x/y dimensions

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // Get the standard name for the coordinate variable

    const NcVar coord_var = get_var(_ncFile, _dims[dim_num]->getName().c_str());
    if (IS_INVALID_NC(coord_var)) continue;

    ConcatString dim_std_name;
    if (!get_var_standard_name(&coord_var, dim_std_name)) {
      continue;
    }

    // See if this is an X or Y dimension

    if (dim_std_name == x_dim_key_name)
    {
      _xDim = _dims[dim_num];

      x_dim_var_name = GET_NC_NAME_P(_xDim);
      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
        if (Var[var_num].name == x_dim_var_name)
        {
          _xCoordVar = Var[var_num].var;
          break;
        }
      }
    }

    if (dim_std_name == y_dim_key_name)
    {
      _yDim = _dims[dim_num];

      y_dim_var_name = GET_NC_NAME_P(_yDim);
      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
        if ( Var[var_num].name == y_dim_var_name)
        {
          _yCoordVar = Var[var_num].var;
          break;
        }
      }
    }

  }

  if (_xDim == nullptr || _xCoordVar == nullptr)
  {
    if (_xCoordVar == nullptr) _xCoordVar = find_var_by_standard_name(x_dim_key_name);
    if (_xCoordVar != nullptr && _xDim == nullptr) {
      NcDim dim = get_nc_dim(_xCoordVar, (get_dim_count(_xCoordVar)-1));
      for (int dim_num = 0; dim_num < _numDims; ++dim_num) {
        if(GET_NC_NAME(dim) == GET_NC_NAME_P(_dims[dim_num])) {
          _xDim = _dims[dim_num];
          break;
        }
      }
    }
  }

  if (_yDim == nullptr || _yCoordVar == nullptr)
  {
    if (_yCoordVar == nullptr) _yCoordVar = find_var_by_standard_name(y_dim_key_name);
    if (_yCoordVar != nullptr && _yDim == nullptr) {
      int dim_offset = get_dim_count(_xCoordVar) - 2;
      if (dim_offset < 0) dim_offset = 0;
      NcDim dim = get_nc_dim(_yCoordVar, dim_offset);
      for (int dim_num = 0; dim_num < _numDims; ++dim_num) {
        if(GET_NC_NAME(dim) == GET_NC_NAME_P(_dims[dim_num])) {
          _yDim = _dims[dim_num];
          break;
        }
      }
    }
  }

  if (_xDim == nullptr)
  {
    mlog << Error << "\n" << caller_name
         << "Didn't find X dimension (projection_x_coordinate) in netCDF file.\n\n";
    exit(1);
  }

  if (_yDim == nullptr)
  {
    mlog << Error << "\n" << caller_name
         << "Didn't find Y dimension (projection_y_coordinate) in netCDF file.\n\n";
    exit(1);
  }

  if (_xCoordVar == nullptr)
  {
    mlog << Error << "\n" << caller_name
         << "Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_yCoordVar == nullptr)
  {
    mlog << Error << "\n" << caller_name
         << "Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }
}


////////////////////////////////////////////////////////////////////////


NcVarInfo* NcCfFile::find_var_name(const char * var_name) const
{
  for (int i = 0; i < Nvars; i++)
    if (Var[i].name == var_name)
      return &Var[i];

  return nullptr;
}


////////////////////////////////////////////////////////////////////////


NcVarInfo* NcCfFile::find_var_by_dim_name(const char *dim_name) const
{
  NcVarInfo *var = find_var_name(dim_name);
  if (!var) {
    //StringArray dimNames;
    for (int i=0; i<Nvars; i++) {
      if (1 == Var[i].Ndims) {
        //dimNames.clear();
        //get_dim_names(Var[j].var, &dimNames);
        //if (dimNames[i] == dim_name) {
        NcDim dim = get_nc_dim(Var[i].var, 0);
        if (GET_NC_NAME(dim) == dim_name) {
          var = &Var[i];
          break;
        }
      }
    }
  }

  return var;
}


////////////////////////////////////////////////////////////////////////


NcVar *NcCfFile::find_var_by_standard_name(const char *standard_name) const
{
  NcVar *var = nullptr;
  ConcatString att_value;
  for (int i=0; i<Nvars; i++) {
    if (get_var_standard_name(Var[i].var, att_value)) {
      if (att_value == standard_name) {
        var = Var[i].var;
        break;
      }
    }
    //if (get_var_long_name(Var[i].var, att_value) {
    //  if (att_value == standard_name) {
    //    var = &Var[i];
    //    break;
    //  }
    //}
  }

  return var;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::read_netcdf_grid()
{
  // Loop through the variables looking for the first gridded variable.  We
  // will use this variable to pull out the grid information.  The CF
  // description allows for different fields in the same file to have different
  // grids, but with how the gridded information is used in MET, I'm making the
  // assumption that all fields are on the same grid.

  bool ignore;
  int max_dim = 0;
  NcVar *data_var = nullptr;
  NcVar *tmp_data_var = nullptr;
  IntArray var_index_list;

  for (int i = 0; i < Nvars; ++i)
  {
    // Get a pointer to the variable

    NcVar *var = Var[i].var;

    // A gridded data variable can have anywhere from 2 to 4 dimensions (the
    // fourth being time).  Any other variables can be ignored

    int num_dims = get_dim_count(var);

    if (num_dims < 2 || num_dims > 4)
      continue;

    // Skip the latitude and longitude variables, if they are present

    ConcatString std_name;
    bool has_standard_name = get_var_standard_name(var, std_name);

    ignore = false;
    if (has_standard_name) {
      if (std_name == "" || std_name == "latitude"
          || std_name == "longitude" || std_name == "time") {
        ignore = true;
      }
    }
    if (ignore) continue;

    if (max_dim < num_dims) max_dim = num_dims;

    if (has_att(var, coordinates_att_name)) {
      data_var = var;
      break;

    }

    // If we get here, this should be a gridded data variable
    if (tmp_data_var == nullptr) tmp_data_var = var;
    if (!has_standard_name) continue;   // Skip if no standard name
    var_index_list.add(i);

  } /* endfor - i */

  if (data_var == nullptr)
  {
    for (int i = 0; i < var_index_list.n(); ++i)
    {
      // Get a pointer to the variable
      NcVar *var = Var[i].var;

      // Exclude with less dimensions
      if (get_dim_count(var) < max_dim) continue;

      // If we get here, this should be a gridded data variable
      data_var = var;
      break;

    } /* endfor - i */
  }

  if (data_var == nullptr) data_var = tmp_data_var;

  // Pull the grid projection from the variable information.  First, look for
  // a grid_mapping attribute.

  NcVarAtt *grid_mapping_att = get_nc_att(data_var, grid_mapping_att_name);

  if (IS_VALID_NC_P(grid_mapping_att))
  {
    get_grid_from_grid_mapping(grid_mapping_att);
    if (grid_mapping_att) delete grid_mapping_att;
    return;
  }

  if (grid_mapping_att) delete grid_mapping_att;

  // If the grid mapping isn't provided, see if we can intuit a projection
  // from the given dimensions

  bool status = get_grid_from_dimensions();

  if (!status) status = get_grid_from_coordinates(data_var);

  // As a sanity check, make sure the x/y dimensions are both set or
  // the x_dim_var_name and y_dim_var_name strings are both set.

  if (nullptr == _xDim && nullptr == _yDim && _latVar && _lonVar) {
    int dim_offset = get_dim_count(_lonVar) - 1;
    NcDim lon_dim = get_nc_dim(_lonVar, dim_offset);
    NcDim lat_dim = get_nc_dim(_latVar, 0);
    _xDim = &lon_dim;
    _yDim = &lat_dim;
    x_dim_var_name = GET_NC_NAME(lon_dim);
    y_dim_var_name = GET_NC_NAME(lat_dim);
    get_grid_from_lat_lon_vars(_latVar, _lonVar, _yDim->getSize(), _xDim->getSize());
    status = true;
  }

  if (!status ||
      !((_xDim && _yDim) ||
        (x_dim_var_name.nonempty() && y_dim_var_name.nonempty())))
  {
     mlog << Warning << "\nNcCfFile::read_netcdf_grid() -> "
          << "Couldn't figure out projection from information in netCDF file.\n\n";
     return;
  }

  return;

}


////////////////////////////////////////////////////////////////////////


Grid NcCfFile::build_grid_from_lat_lon_vars(NcVar *lat_var, NcVar *lon_var,
                                            const long lat_counts, const long lon_counts) {
  Grid grid_ll;
  bool swap_to_north = false;
  LatLonData data = get_data_from_lat_lon_vars(lat_var, lon_var,
                                               lat_counts, lon_counts,
                                               swap_to_north);

  data.dump();

  grid_ll.set(data);   // resets swap_to_north to false
  if (swap_to_north) grid_ll.set_swap_to_north(true);
  return grid_ll;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_from_grid_mapping(const NcVarAtt *grid_mapping_att)
{
  static const string method_name = "NcCfFile::get_grid_from_grid_mapping()";

  // The grid_mapping attribute gives the name of the variable that
  // contains the grid mapping information.  Find that variable.

  //char *mapping_name = grid_mapping_att->getValues(att->as_string(0);
  ConcatString mapping_name;
  bool status = get_att_value_chars(grid_mapping_att, mapping_name);
  if (!status)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot extract grid mapping name from netCDF file.\n\n";
    return;
  }

  NcVar *grid_mapping_var = nullptr;

  for (int i = 0; i < Nvars; ++i)
  {
    if ( Var[i].name == mapping_name )
    {
      grid_mapping_var = Var[i].var;
      break;
    }
  } /* endfor - i */

  if ((nullptr == grid_mapping_var) || (IS_INVALID_NC_P(grid_mapping_var)))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot extract grid mapping variable (" << mapping_name
         << ") from netCDF file.\n\n";
    return;
  }

  // Get the name of the grid mapping

  NcVarAtt *grid_mapping_name_att = get_nc_att(grid_mapping_var, grid_mapping_name_att_name);

  if (IS_INVALID_NC_P(grid_mapping_name_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot get coordinate system name from netCDF file.\n\n";
    if (grid_mapping_name_att) delete grid_mapping_name_att;
    return;
  }

  //string grid_mapping_name = grid_mapping_name_att->getValues(att->as_string(0);
  ConcatString grid_mapping_name;
  get_att_value_chars(grid_mapping_name_att, grid_mapping_name);
  if (grid_mapping_name_att) delete grid_mapping_name_att;

  // Handle each mapping type defined in the standard

  if (grid_mapping_name == "albers_conical_equal_area")
  {
    get_grid_mapping_albers_conical_equal_area(grid_mapping_var);
  }
  else if (grid_mapping_name == "azimuthal_equidistant")
  {
    get_grid_mapping_azimuthal_equidistant(grid_mapping_var);
  }
  else if (grid_mapping_name == "lambert_azimuthal_equal_area")
  {
    get_grid_mapping_lambert_azimuthal_equal_area(grid_mapping_var);
  }
  else if (grid_mapping_name == "lambert_conformal_conic")
  {
    get_grid_mapping_lambert_conformal_conic(grid_mapping_var);
  }
  else if (grid_mapping_name == "lambert_cylindrical_equal_area")
  {
    get_grid_mapping_lambert_cylindrical_equal_area(grid_mapping_var);
  }
  else if (grid_mapping_name == "latitude_longitude")
  {
    get_grid_mapping_latitude_longitude(grid_mapping_var);
  }
  else if (grid_mapping_name == "mercator")
  {
    get_grid_mapping_mercator(grid_mapping_var);
  }
  else if (grid_mapping_name == "orthographic")
  {
    get_grid_mapping_orthographic(grid_mapping_var);
  }
  else if (grid_mapping_name == "polar_stereographic")
  {
    get_grid_mapping_polar_stereographic(grid_mapping_var);
  }
  else if (grid_mapping_name == "rotated_latitude_longitude")
  {
    get_grid_mapping_rotated_latitude_longitude(grid_mapping_var);
  }
  else if (grid_mapping_name == "stereographic")
  {
    get_grid_mapping_stereographic(grid_mapping_var);
  }
  else if (grid_mapping_name == "transverse_mercator")
  {
    get_grid_mapping_transverse_mercator(grid_mapping_var);
  }
  else if (grid_mapping_name == "vertical_perspective")
  {
    get_grid_mapping_vertical_perspective(grid_mapping_var);
  }
  else if (grid_mapping_name == grid_mapping_name_geostationary)
  {
    get_grid_mapping_geostationary(grid_mapping_var);
  }
  else
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Unknown grid mapping name (" << grid_mapping_name
         << ") found in netCDF file.\n\n";
    return;
  }

}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_albers_conical_equal_area(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_albers_conical_equal_area()";

  mlog << Error << "\n" << method_name << " -> "
       << "Albers conical equal area grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_azimuthal_equidistant(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_azimuthal_equidistant()";

  mlog << Error << "\n" << method_name << " -> "
       << "Azimuthal equidistant grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_lambert_azimuthal_equal_area(const NcVar *grid_mapping_var)
{
  static string method_name = "NcCfFile::get_grid_mapping_lambert_azimuthal_equal_area()";
  double x_coord_to_m_cf = 1.0;
  double y_coord_to_m_cf = 1.0;

  // Look for the x/y dimensions and x/y coordinate variables
  find_xy_vars(method_name);

  // Handle coordinate variable units

  ConcatString x_coord_units_name;
  if (!get_var_units(_xCoordVar, x_coord_units_name)) {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for X coordinate variable -- "
         << "assuming meters.\n\n";
  }
  else {
    if (0 == x_coord_units_name.length()) {
      mlog << Warning << "\n" << method_name << " -> "
           << "Cannot extract X coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if ( x_coord_units_name == "m" ||
                x_coord_units_name == "meter" ||
                x_coord_units_name == "meters") x_coord_to_m_cf = 1.0;
      else if (x_coord_units_name == "km") x_coord_to_m_cf = 1000.0;
      else {
        mlog << Warning << "\n" << method_name << " -> "
             << "The X coordinates must be in meters or kilometers for MET.\n\n";
        return;
      }
    }
  }

  ConcatString y_coord_units_name;
  if (!get_var_units(_yCoordVar, y_coord_units_name)) {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for Y coordinate variable -- "
         << "assuming meters.\n\n";
  }
  else {
    if (0 == y_coord_units_name.length()) {
      mlog << Warning << "\n" << method_name << " -> "
           << "Cannot extract Y coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if ( y_coord_units_name == "m" ||
                y_coord_units_name == "meter" ||
                y_coord_units_name == "meters" ) y_coord_to_m_cf = 1.0;
      else if (y_coord_units_name == "km") y_coord_to_m_cf = 1000.0;
      else {
        mlog << Warning << "\n" << method_name << " -> "
             << "The Y coordinates must be in meters or kilometers for MET.\n\n";
        return;
      }
    }
  }

  // Figure out the dx/dy  and x/y pin values from the dimension variables

  long x_counts = GET_NC_SIZE_P(_xDim);
  vector<double> x_values(x_counts, bad_data_double);

  get_nc_data(_xCoordVar, x_values);

  long y_counts = GET_NC_SIZE_P(_yDim);
  vector<double> y_values(y_counts, bad_data_double);

  get_nc_data(_yCoordVar, y_values);

  // Unit conversion

  for (int i = 0; i<x_counts; ++i) x_values[i] *= x_coord_to_m_cf;
  for (int i = 0; i<y_counts; ++i) y_values[i] *= y_coord_to_m_cf;

  // Calculate dx and dy

  double dx_m = (x_values[x_counts-1] - x_values[0]) / (x_counts - 1);
  double dy_m = (y_values[y_counts-1] - y_values[0]) / (y_counts - 1);
  double dx_m_a = fabs(dx_m);
  double dy_m_a = fabs(dy_m);

  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < (int)x_counts; ++i)
  {
    double curr_delta = fabs(x_values[i] - x_values[i-1]);
    if (fabs(curr_delta - dx_m_a) > DELTA_TOLERANCE)
    {
      mlog << Warning << "\n" << method_name << " -> "
           << "MET can only process Lambert Azimuthal Equal Area files "
           << "where the delta along the x-axis is constant ("
           << curr_delta << " != " << dx_m_a << ")\n\n";
      return;
    }
  }

  for (int i = 1; i < (int)y_counts; ++i)
  {
    double curr_delta = fabs(y_values[i] - y_values[i-1]);
    if (fabs(curr_delta - dy_m_a) > DELTA_TOLERANCE)
    {
      mlog << Warning << "\n" << method_name << " -> "
           << "MET can only process Lambert Azimuthal Equal Area files "
           << "where the delta along the y-axis is constant ("
           << curr_delta << " != " << dy_m_a << ")\n\n";
      return;
    }
  }

  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  LaeaNetcdfData data;
  data.name = laea_proj_type;

  // longitude_of_projection_origin (convert degrees east to west)

  data.prime_meridian_lon = get_nc_var_att_double(
    grid_mapping_var, "longitude_of_prime_meridian", false);

  if(is_bad_data(data.prime_meridian_lon)) data.prime_meridian_lon =   0.0;
  else                                     data.prime_meridian_lon *= -1.0;

  // semi_major_axis (convert m to km)

  data.semi_major_axis_km = get_nc_var_att_double(
    grid_mapping_var, "semi_major_axis", false);

  if(is_bad_data(data.semi_major_axis_km)) data.semi_major_axis_km = EARTH_MAJOR_AXIS_km;
  else                                     data.semi_major_axis_km /= m_per_km;

  // semi_minor_axis (convert m to km)

  data.semi_minor_axis_km = get_nc_var_att_double(
    grid_mapping_var, "semi_minor_axis", false);

  if(is_bad_data(data.semi_minor_axis_km)) data.semi_minor_axis_km = EARTH_MAJOR_AXIS_km;
  else                                     data.semi_minor_axis_km /= m_per_km;

  // latitude_of_projection_origin

  data.proj_origin_lat = get_nc_var_att_double(
    grid_mapping_var, "latitude_of_projection_origin");

  // longitude_of_projection_origin (convert degrees east to west)

  data.proj_origin_lon = -1.0 * get_nc_var_att_double(
    grid_mapping_var, "longitude_of_projection_origin");

  // false_easting

  double false_easting = get_nc_var_att_double(
    grid_mapping_var, "false_easting", false);

  if(!is_bad_data(false_easting) && !is_eq(false_easting, 0.0))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "MET cannot process Lambert Azimuthal Equal Area files "
         << "with non-zero false_easting (" << false_easting
         << ").\n\n";
    return;
  }

  // false_northing

  double false_northing = get_nc_var_att_double(
    grid_mapping_var, "false_northing", false);

  if(!is_bad_data(false_northing) && !is_eq(false_northing, 0.0))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "MET cannot process Lambert Azimuthal Equal Area files "
         << "with non-zero false_northing (" << false_northing
         << ").\n\n";
    return;
  }

  // Calculate the pin indices.  The pin will be located at the grid's reference
  // location since that's the only lat/lon location we know about.

  data.x_pin = -(x_values[0] / dx_m);
  data.y_pin = -(y_values[0] / dy_m);

  data.dx_km = dx_m / m_per_km;
  data.dy_km = dy_m / m_per_km;
  data.nx = _xDim->getSize();
  data.ny = _yDim->getSize();

  data.dump();

  // Instantiate grid

  grid.set(data);
  if (dy_m < 0) grid.set_swap_to_north(true);

  grid_ready = true;

}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_lambert_conformal_conic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_lambert_conformal_conic()";
  double x_coord_to_m_cf = 1.0;
  double y_coord_to_m_cf = 1.0;

  // standard_parallel -- there can be 1 or 2 of these

  NcVarAtt *std_parallel_att = get_nc_att(
    grid_mapping_var, (string)"standard_parallel");
  if (IS_INVALID_NC_P(std_parallel_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot get standard_parallel attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // longitude_of_central_meridian

  NcVarAtt *central_lon_att = get_nc_att(
    grid_mapping_var, (string)"longitude_of_central_meridian");
  if (IS_INVALID_NC_P(central_lon_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot get longitude_of_central_meridian attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // latitude_of_projection_origin

  NcVarAtt *proj_origin_lat_att = get_nc_att(
    grid_mapping_var, (string)"latitude_of_projection_origin");
  if (IS_INVALID_NC_P(proj_origin_lat_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot get latitude_of_projection_origin attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // Look for the x/y dimensions and x/y coordinate variables
  find_xy_vars(method_name);

  if (get_data_size(_xCoordVar) != (int) GET_NC_SIZE_P(_xDim) ||
      get_data_size(_yCoordVar) != (int) GET_NC_SIZE_P(_yDim))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    return;
  }

  // Make sure that the coordinate variables are given in meters.  If we get
  // files that are in other units, we'll have to update the code to do the
  // units conversions.

  ConcatString x_coord_units_name;
  if (!get_var_units(_xCoordVar, x_coord_units_name)) {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for X coordinate variable -- assuming meters.\n\n";
  }
  else {
    if (0 == x_coord_units_name.length()) {
      mlog << Warning << "\n" << method_name << " -> "
           << "Cannot extract X coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if ( x_coord_units_name == "m" ||
                x_coord_units_name == "meter" ||
                x_coord_units_name == "meters") x_coord_to_m_cf = 1.0;
      else if (x_coord_units_name == "km") x_coord_to_m_cf = 1000.0;
      else {
        mlog << Warning << "\n" << method_name << " -> "
             << "The X coordinates must be in meters or kilometers for MET.\n\n";
        return;
      }
    }
  }

  ConcatString y_coord_units_name;
  if (!get_var_units(_yCoordVar, y_coord_units_name)) {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for Y coordinate variable -- assuming meters.\n\n";
  }
  else {
    if (0 == y_coord_units_name.length()) {
      mlog << Warning << "\n" << method_name << " -> "
           << "Cannot extract Y coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if ( y_coord_units_name == "m" ||
                y_coord_units_name == "meter" ||
                y_coord_units_name == "meters" ) y_coord_to_m_cf = 1.0;
      else if (y_coord_units_name == "km") y_coord_to_m_cf = 1000.0;
      else {
        mlog << Warning << "\n" << method_name << " -> "
             << "The Y coordinates must be in meters or kilometers for MET.\n\n";
        return;
      }
    }
  }

  // Figure out the dx/dy  and x/y pin values from the dimension variables

  long x_counts = GET_NC_SIZE_P(_xDim);
  vector<double> x_values(x_counts, bad_data_double);
  get_nc_data(_xCoordVar, x_values);

  long y_counts = GET_NC_SIZE_P(_yDim);
  vector<double> y_values(y_counts, bad_data_double);
  get_nc_data(_yCoordVar, y_values);

  // Unit conversion

  for (int i = 0; i<x_counts; ++i) x_values[i] *= x_coord_to_m_cf;
  for (int i = 0; i<y_counts; ++i) y_values[i] *= y_coord_to_m_cf;

  // Calculate dx and dy assuming they are constant.  MET requires that dx be
  // equal to dy

  double dx_m = (x_values[x_counts-1] - x_values[0]) / (x_counts - 1);
  double dy_m = (y_values[y_counts-1] - y_values[0]) / (y_counts - 1);
  double dx_m_a = fabs(dx_m);
  double dy_m_a = fabs(dy_m);

  if (fabs(dx_m_a - dy_m_a) > DELTA_TOLERANCE)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "MET can only process Lambert Conformal files where the x-axis and y-axis deltas are the same\n\n";
    return;
  }

  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < (int)x_counts; ++i)
  {
    double curr_delta = fabs(x_values[i] - x_values[i-1]);
    if (fabs(curr_delta - dx_m_a) > DELTA_TOLERANCE)
    {
      mlog << Warning << "\n" << method_name << " -> "
           << "MET can only process Lambert Conformal files where the delta along the x-axis is constant\n\n";
      return;
    }
  }

  for (int i = 1; i < (int)y_counts; ++i)
  {
    double curr_delta = fabs(y_values[i] - y_values[i-1]);
    if (fabs(curr_delta - dy_m_a) > DELTA_TOLERANCE)
    {
      mlog << Warning << "\n" << method_name << " -> "
           << "MET can only process Lambert Conformal files where the delta along the y-axis is constant\n\n";
      return;
    }
  }

  // Calculate the pin indices.  The pin will be located at the grid's reference
  // location since that's the only lat/lon location we know about.

  double x_pin = -(x_values[0] / dx_m);
  double y_pin = -(y_values[0] / dy_m);

  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  LambertData data;
  double double_data;
  NumArray double_datas;
  data.name = lambert_proj_type;
  get_att_value_doubles(std_parallel_att, double_datas);
  data.scale_lat_1 = double_datas[0];
  if (std_parallel_att->getAttLength() == 1)
    data.scale_lat_2 = data.scale_lat_1;
  else
    data.scale_lat_2 = double_datas[1];
  double_data = get_att_value_double(proj_origin_lat_att);
  data.lat_pin = double_data;
  get_att_value_doubles(central_lon_att, double_datas);
  data.lon_pin = -double_datas[0];
  data.hemisphere = (data.lat_pin > 0 ? 'N' : 'S');
  data.x_pin = x_pin;
  data.y_pin = y_pin;
  data.lon_orient = -double_datas[0];
  data.d_km = dx_m / 1000.0;
  data.r_km = EARTH_MAJOR_AXIS_km;
  data.nx = _xDim->getSize();
  data.ny = _yDim->getSize();
  data.so2_angle = 0.0;

  data.dump();

  grid.set(data);
  if (dy_m < 0) grid.set_swap_to_north(true);
  grid_ready = true;

  if(std_parallel_att) delete std_parallel_att;
  if(central_lon_att) delete central_lon_att;
  if(proj_origin_lat_att) delete proj_origin_lat_att;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_lambert_cylindrical_equal_area(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_lambert_cynlindrical_equal_area()";

  mlog << Error << "\n" << method_name << " -> "
       << "Lambert cylindrical equal area grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_latitude_longitude(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_latitude_longitude()";

  // Look for the lat/lon dimensions

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // The lat/lon dimensions are identified by their units

    const NcVar coord_var = get_var(_ncFile, _dims[dim_num]->getName().c_str());
    if (IS_INVALID_NC(coord_var)) continue;

    ConcatString dim_units;
    if (!get_var_units(&coord_var, dim_units)) continue;

    // See if this is a lat or lon dimension

    if (is_nc_unit_latitude(dim_units.c_str()))
    {
      if (_yDim == nullptr)
      {
        _yDim = _dims[dim_num];

        y_dim_var_name = GET_NC_NAME_P(_yDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if ( Var[var_num].name == GET_NC_NAME_P(_yDim))
          {
            _yCoordVar = Var[var_num].var;
            break;
          }
        }
      }
      else
      {
        mlog << Warning << "\n" << method_name << " -> "
             << "Found multiple variables for latitude, using \""
             << GET_NC_NAME_P(_yCoordVar) << "\".\n\n";
      }
    }

    if (is_nc_unit_longitude(dim_units.c_str()))
    {
      if (_xDim == nullptr)
      {
        _xDim = _dims[dim_num];

        x_dim_var_name = GET_NC_NAME_P(_xDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if ( Var[var_num].name == GET_NC_NAME_P(_xDim))
          {
            _xCoordVar = Var[var_num].var;
            break;
          }
        }
      }
      else
      {
        mlog << Warning << "\n" << method_name << " -> "
             << "Found multiple variables for longitude, using \""
             << GET_NC_NAME_P(_xCoordVar) << "\".\n\n";
      }
    }

  }

  if (_xDim == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find X dimension (degrees_east) in netCDF file.\n\n";
    return;
  }

  if (_yDim == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find Y dimension (degrees_north) in netCDF file.\n\n";
    return;
  }

  if (_xCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    return;
  }

  if (_yCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    return;
  }

  long lon_counts = _xDim->getSize();
  long lat_counts = _yDim->getSize();
  if (get_data_size(_xCoordVar) != lon_counts ||
      get_data_size(_yCoordVar) != lat_counts)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    return;
  }

  get_grid_from_lat_lon_vars(_yCoordVar, _xCoordVar, lat_counts, lon_counts);

}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_mercator(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_mercator()";

  mlog << Error << "\n" << method_name << " -> "
       << "Mercator grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_orthographic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_orthographic()";

  mlog << Error << "\n" << method_name << " -> "
       << "Orthographic grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


double get_nc_var_att_double(const NcVar *nc_var, const char *att_name, bool is_required)
{
   NcVarAtt *nc_att = get_nc_att(nc_var, (string)att_name);

   if(IS_INVALID_NC_P(nc_att))
   {
      if (is_required) {
         mlog << Error << "\nget_nc_var_att_double() -> "
              << "Cannot get \"" << att_name << "\" from "
              << GET_NC_NAME_P(nc_var) << " variable.\n\n";
         exit(1);
      }
      else return bad_data_double;
   }
   double att_val = get_att_value_double(nc_att);
   if (nc_att) delete nc_att;

   return att_val;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_polar_stereographic(const NcVar *grid_mapping_var)
{
  double x_coord_to_m_cf = 1.0;
  double y_coord_to_m_cf = 1.0;
  static const string method_name = "NcCfFile::get_grid_mapping_polar_stereographic() -> ";

  // Get projection attributes
  // proj_origin_lat: either 90.0 or -90.0, to decide the northern/southern hemisphere
  bool is_spherical_earch = true;
  double proj_origin_lat = get_nc_var_att_double(
                grid_mapping_var, "latitude_of_projection_origin");
  double proj_vertical_lon = get_nc_var_att_double(
                grid_mapping_var, "straight_vertical_longitude_from_pole");
  double proj_origin_lon = get_nc_var_att_double(
                grid_mapping_var, "longitude_of_projection_origin", false);
  double proj_standard_parallel = get_nc_var_att_double(
                grid_mapping_var, "standard_parallel", false);
  double proj_origin_scale_factor = get_nc_var_att_double(
                grid_mapping_var, "scale_factor_at_projection_origin", false);
  double semi_major_axis = get_nc_var_att_double(
                grid_mapping_var, "semi_major_axis", false);
  double semi_minor_axis = get_nc_var_att_double(
                grid_mapping_var, "semi_minor_axis", false);
  double inverse_flattening = get_nc_var_att_double(
                grid_mapping_var, "inverse_flattening", false);
  bool has_scale_factor = !is_eq(proj_origin_scale_factor, bad_data_double);
  bool has_standard_parallel = !is_eq(proj_standard_parallel, bad_data_double);

  // Check that the scale factor at the origin is 1.

  if (!is_eq(inverse_flattening, bad_data_double) ||
    (!is_eq(semi_minor_axis, bad_data_double) && !is_eq(semi_minor_axis,semi_major_axis))) {
    is_spherical_earch = false;
    mlog << Debug(2) << "\n" << method_name
         << "This is an ellipsoidal earth.\n\n";
  }
  else if(!has_scale_factor && !has_standard_parallel) {
    mlog << Warning << "\n" << method_name
         << "The attribute \"scale_factor_at_projection_origin\" and \"standard_parallel\" of the "
         << GET_NC_NAME_P(grid_mapping_var) << " variable do not exist.\n\n";
    return;
  }
  else if(has_scale_factor && !is_eq(proj_origin_scale_factor, 1.0)) {
    mlog << Warning << "\n" << method_name
         << "Unexpected attribute value of " << proj_origin_scale_factor
         << " for the scale_factor_at_projection_origin attribute of the "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // Look for the x/y dimensions and x/y coordinate variables
  find_xy_vars(method_name);

  if (get_data_size(_xCoordVar) != (int) GET_NC_SIZE_P(_xDim) ||
      get_data_size(_yCoordVar) != (int) GET_NC_SIZE_P(_yDim))
  {
    mlog << Warning << "\n" << method_name
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    return;
  }

  // Make sure that the coordinate variables are given in meters.  If we get
  // files that are in other units, we'll have to update the code to do the
  // units conversions.

  ConcatString x_coord_units_name;
  if (!get_var_units(_xCoordVar, x_coord_units_name)) {
    mlog << Warning << "\n" << method_name
         << "Units not given for X coordinate variable -- assuming meters.\n\n";
  }
  else {
    if (0 == x_coord_units_name.length()) {
      mlog << Warning << "\n" << method_name
           << "Cannot extract X coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if ( x_coord_units_name == "m" ||
                x_coord_units_name == "meter" ||
                x_coord_units_name == "meters") x_coord_to_m_cf = 1.0;
      else if ( x_coord_units_name == "km") x_coord_to_m_cf = 1000.0;
      else {
        mlog << Warning << "\n" << method_name
             << "The X coordinates (" << x_coord_units_name
             << ") must be in meters or kilometers for MET.\n\n";
        return;
      }
    }
  }

  ConcatString y_coord_units_name;
  if (!get_var_units(_yCoordVar, y_coord_units_name)) {
    mlog << Warning << "\n" << method_name
         << "Units not given for Y coordinate variable -- assuming meters.\n\n";
  }
  else {
    if (0 == y_coord_units_name.length()) {
      mlog << Warning << "\n" << method_name
           << "Cannot extract Y coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if ( y_coord_units_name == "m" ||
                y_coord_units_name == "meter" ||
                y_coord_units_name == "meters" ) y_coord_to_m_cf = 1.0;
      else if ( y_coord_units_name == "km") y_coord_to_m_cf = 1000.0;
      else {
        mlog << Warning << "\n" << method_name
             << "The Y coordinates must be in meters or kilometers for MET.\n\n";
        return;
      }
    }
  }

  // Figure out the dx/dy  and x/y pin values from the dimension variables

  long x_counts = GET_NC_SIZE_P(_xDim);
  vector<double> x_values(x_counts, bad_data_double);
  get_nc_data(_xCoordVar, x_values);

  long y_counts = GET_NC_SIZE_P(_yDim);
  vector<double> y_values(y_counts, bad_data_double);
  get_nc_data(_yCoordVar, y_values);

  // Unit conversion

  for (int i = 0; i<x_counts; ++i) x_values[i] *= x_coord_to_m_cf;
  for (int i = 0; i<y_counts; ++i) y_values[i] *= y_coord_to_m_cf;

  // Calculate dx and dy assuming they are constant.  MET requires that dx be
  // equal to dy

  double dx_m = (x_values[x_counts-1] - x_values[0]) / (x_counts - 1);
  double dy_m = (y_values[y_counts-1] - y_values[0]) / (y_counts - 1);
  double dx_m_a = fabs(dx_m);
  double dy_m_a = fabs(dy_m);

  if (fabs(dx_m_a - dy_m_a) > DELTA_TOLERANCE)
  {
    mlog << Warning << "\n" << method_name
         << "MET can only process Polar Stereographic files where the x-axis and y-axis deltas are the same.\n\n";
    return;
  }

  if (is_eq(semi_major_axis, bad_data_double))
    semi_major_axis = EARTH_MAJOR_AXIS_km * m_per_km;
  else if (semi_major_axis < 10000.0) semi_major_axis *= m_per_km; // meters

  // Calculate the pin indices.  The pin will be located at the grid's reference
  // location since that's the only lat/lon location we know about.

  double x_pin = -(x_values[0] / dx_m);
  double y_pin = -(y_values[0] / dy_m);

  // Fill in the data structure.  Remember to negate the longitude values.

  StereographicData data;
  data.name = stereographic_proj_type;
  data.lat_pin = proj_origin_lat;
  data.lon_pin = -1.0 * (is_eq(proj_origin_lon, bad_data_double) ? 0 : proj_origin_lon);
  data.hemisphere = (proj_origin_lat > 0 ? 'N' : 'S');
  data.x_pin = x_pin;
  data.y_pin = y_pin;
  data.scale_lat = proj_origin_lat;
  data.lon_orient = -1.0 * proj_vertical_lon;
  data.d_km = dx_m / m_per_km;
  data.dy_km = dy_m / m_per_km;
  data.r_km = semi_major_axis / m_per_km;
  data.nx = _xDim->getSize();
  data.ny = _yDim->getSize();

  bool is_north_hemisphere = proj_origin_lat > 0;
  double eccentricity, false_east,false_north, scale_factor;
  scale_factor = proj_origin_scale_factor;
  eccentricity = false_east = false_north = 0.;
  if(!has_scale_factor && has_standard_parallel) {
    double lat, lon;
    double x, y, x2, y2;

    false_east = get_nc_var_att_double(grid_mapping_var, "false_east", false);
    false_north = get_nc_var_att_double(grid_mapping_var, "false_north", false);

    if (is_eq(false_east, bad_data_double)) false_east = 0.;
    if (is_eq(false_north, bad_data_double)) false_north = 0.;
    if(!is_spherical_earch) eccentricity = st_eccentricity_func(semi_major_axis, semi_minor_axis,
                                                                inverse_flattening);

    x = x_values[0];
    y = y_values[0];
    scale_factor = st_sf_func(proj_standard_parallel, eccentricity, is_north_hemisphere);
    st_xy_to_latlon_func(x, y, lat, lon, scale_factor, semi_major_axis,
                         proj_vertical_lon, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    data.lat_pin = lat;
    data.lon_pin = -lon;

    if(!has_scale_factor && has_standard_parallel) {
      if(is_eq(eccentricity, 0.0)) {
        data.x_pin = data.y_pin = 0.;
      }
      else {
        data.x_pin = x / dx_m_a;
        data.y_pin = y / dy_m_a;
      }
    }

    st_latlon_to_xy_func(lat, lon, x2, y2, scale_factor, proj_vertical_lon,
                         semi_major_axis, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    mlog << Debug(6) << method_name
         << " x: between " << x_values[0] <<" and " << x_values[x_counts-1]
         << ", y: between " << y_values[0] <<" and " << y_values[y_counts-1] << "\n";
    mlog << Debug(6) << method_name
         << "pin: (x,y) -> (lat,lon) -> (x2,y2): (" << x << "," << y << ") -> ("
         << lat << "," << lon << ") -> (" << x2 << "," << y2
         << ") Diff (m): x=" << (x-x2) <<", y=" << (y-y2) << "\n";

  }

  // ellipsoidal earth
  data.scale_factor = scale_factor;
  data.eccentricity = eccentricity;
  data.false_east = false_east;
  data.false_north = false_north;

  data.dump();

  grid.set(data);
  grid_ready = true;

  //Note: do not set grid.set_swap_to_north()

  if(mlog.verbosity_level() >= 10) {
    double lat1, lon1;
    double x1, y1, x2, y2;

    mlog << Debug(15) << method_name
         << "dx_m=" << dx_m << ", dy_m=" << dy_m << "\n";

    x1 = (dx_m > 0) ? x_values[0] : x_values[x_counts-1];
    y1 = (dy_m > 0) ? y_values[0] : y_values[y_counts-1];
    st_xy_to_latlon_func(x1, y1, lat1, lon1, scale_factor, semi_major_axis,
                         proj_vertical_lon, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    st_latlon_to_xy_func(lat1, lon1, x2, y2, scale_factor, proj_vertical_lon,
                         semi_major_axis, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    mlog << Debug(10) << method_name
         << " bottom left: (x,y) -> (lat,lon) -> (x2,y2): ("
         << x1 << "," << y1 << ") -> ("
         << lat1 << "," << lon1 << ") -> (" << x2 << "," << y2
         << ") Diff (m): x=" << (x1-x2) <<", y=" << (y1-y2)
         << "\n";

    x1 = (dx_m > 0) ? x_values[0] : x_values[x_counts-1];
    y1 = (dy_m > 0) ? y_values[y_counts-1] : y_values[0];
    st_xy_to_latlon_func(x1, y1, lat1, lon1, scale_factor, semi_major_axis,
                         proj_vertical_lon, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    st_latlon_to_xy_func(lat1, lon1, x2, y2, scale_factor, proj_vertical_lon,
                         semi_major_axis, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    mlog << Debug(10) << method_name
         << "    top left: (x,y) -> (lat,lon) -> (x2,y2): ("
         << x1 << "," << y1 << ") -> ("
         << lat1 << "," << lon1 << ") -> (" << x2 << "," << y2
         << ") Diff (m): x=" << (x1-x2) <<", y=" << (y1-y2)
         << "\n";

    x1 = x_values[int(x_counts/2)];
    y1 = y_values[int(y_counts/2)];
    st_xy_to_latlon_func(x1, y1, lat1, lon1, scale_factor, semi_major_axis,
                         proj_vertical_lon, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    st_latlon_to_xy_func(lat1, lon1, x2, y2, scale_factor, proj_vertical_lon,
                         semi_major_axis, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    mlog << Debug(10) << method_name
         << "      center: (x,y) -> (lat,lon) -> (x2,y2): (" << x1
         << "," << y1 << ") -> ("
         << lat1 << "," << lon1 << ") -> (" << x2 << "," << y2
         << ") Diff (m): x=" << (x1-x2) <<", y=" << (y1-y2)
         << "\n";

    x1 = (dx_m > 0) ? x_values[x_counts-1] : x_values[0];
    y1 = (dy_m > 0) ? y_values[y_counts-1] : y_values[0];
    st_xy_to_latlon_func(x1, y1, lat1, lon1, scale_factor, semi_major_axis,
                         proj_vertical_lon, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    st_latlon_to_xy_func(lat1, lon1, x2, y2, scale_factor, proj_vertical_lon,
                         semi_major_axis, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    mlog << Debug(10) << method_name
         << "   top right: (x,y) -> (lat,lon) -> (x2,y2): (" << x1
         << "," << y1 << ") -> ("
         << lat1 << "," << lon1 << ") -> (" << x2 << "," << y2
         << ") Diff (m): x=" << (x1-x2) <<", y=" << (y1-y2)
         << "\n";

    x1 = (dx_m > 0) ? x_values[x_counts-1] : x_values[0];
    y1 = (dy_m > 0) ? y_values[0] : y_values[y_counts-1];
    st_xy_to_latlon_func(x1, y1, lat1, lon1, scale_factor, semi_major_axis,
                         proj_vertical_lon, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    st_latlon_to_xy_func(lat1, lon1, x2, y2, scale_factor, proj_vertical_lon,
                         semi_major_axis, false_east, false_north,
                         eccentricity, is_north_hemisphere);
    mlog << Debug(10) << method_name
         << "bottom right: (x,y) -> (lat,lon) -> (x2,y2): (" << x1
         << "," << y1 << ") -> ("
         << lat1 << "," << lon1 << ") -> (" << x2 << "," << y2
         << ") Diff (m): x=" << (x1-x2) <<", y=" << (y1-y2)
         << "\n";

    if(mlog.verbosity_level() >= 15) {
      mlog << Debug(15) << method_name
           << "data.scale_factor=" << data.scale_factor
           <<", -data.lon_orient=" << -1.0*data.lon_orient << ", data.r_km=" << data.r_km
           << ", data.false_east=" << data.false_east << ", data.false_north=" << data.false_north
           << ", data.eccentricity=" << data.eccentricity
           << ", is_north_hemisphere=" << is_north_hemisphere << "\n";
      for (int ix=0; ix<data.nx; ix++) {
        for (int iy=0; iy<data.ny; iy++) {
          double x_diff, y_diff;
          x1 = x_values[ix];
          y1 = y_values[iy];

          st_xy_to_latlon_func(x1, y1, lat1, lon1, data.scale_factor, data.r_km*m_per_km,
                               -data.lon_orient, data.false_east, data.false_north,
                               data.eccentricity, is_north_hemisphere);
          st_latlon_to_xy_func(lat1, lon1, x2, y2, data.scale_factor, -data.lon_orient,
                               data.r_km*m_per_km, data.false_east, data.false_north,
                               data.eccentricity, is_north_hemisphere);
          x_diff = (x1-x2);
          y_diff = (y1-y2);
          mlog << Debug(15)
               << "        index=("<< ix << "," << iy << "): (x,y) -> (lat,lon) -> (x2,y2): (" << x1
               << "," << y1 << ") -> ("
               << lat1 << "," << lon1 << ") -> (" << x2 << "," << y2
               << ") Diff (m): x=" << x_diff <<", y=" << y_diff
               << ((abs(x_diff/dx_m) > 0.5) ? " [x_delta > dx/2] " : " ")
               << ((abs(y_diff/dy_m) > 0.5) ? " [y_delta > dy/2] " : " ")
               << ((abs(x_diff) > dx_m_a || abs(y_diff) > dy_m_a) ? "  *** check dx/dy ***" : " ")
               << "\n";
        }
      }
    }
  }

}


////////////////////////////////////////////////////////////////////////
//
// Reference:
//   https://cfconventions.org/Data/cf-conventions/cf-conventions-1.9/cf-conventions.html#_rotated_pole
//
////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_rotated_latitude_longitude(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_rotated_latitude_longitude()";

  // grid_north_pole_latitude

  NcVarAtt *grid_np_lat_att = get_nc_att(
    grid_mapping_var, (string)"grid_north_pole_latitude");
  if (IS_INVALID_NC_P(grid_np_lat_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot get grid_north_pole_latitude attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // grid_north_pole_longitude

  NcVarAtt *grid_np_lon_att = get_nc_att(
    grid_mapping_var, (string)"grid_north_pole_longitude");
  if (IS_INVALID_NC_P(grid_np_lon_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Cannot get grid_north_pole_longitude attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // Look for the grid_latitude and grid_longitude dimensions

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // These dimensions are identified by the standard_name attribute

    const NcVar coord_var = get_var(_ncFile, _dims[dim_num]->getName().c_str());
    if (IS_INVALID_NC(coord_var))
      continue;

    ConcatString dim_standard_name;
    if (!get_var_standard_name(&coord_var, dim_standard_name)) {
      continue;
    }

    if (0 == dim_standard_name.length()) {
      continue;
    }

    // See if this is a grid_latitude or grid_longitude dimension

    if (dim_standard_name == "grid_latitude")
    {
      if (_yDim == nullptr)
      {
        _yDim = _dims[dim_num];

        y_dim_var_name = GET_NC_NAME_P(_yDim).c_str();

        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if ( Var[var_num].name == GET_NC_NAME_P(_yDim))
          {
            _yCoordVar = Var[var_num].var;
            break;
          }
        }
      }
      else
      {
        mlog << Warning << "\n" << method_name << " -> "
             << "Found multiple variables for grid_latitude, using \""
             << GET_NC_NAME_P(_yCoordVar) << "\".\n\n";
      }
    }

    if (dim_standard_name == "grid_longitude")
    {
      if (_xDim == nullptr)
      {
        _xDim = _dims[dim_num];

        x_dim_var_name = GET_NC_NAME_P(_xDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if ( Var[var_num].name == GET_NC_NAME_P(_xDim))
          {
            _xCoordVar = Var[var_num].var;
            break;
          }
        }
      }
      else
      {
        mlog << Warning << "\n" << method_name << " -> "
             << "Found multiple variables for grid_longitude, using \""
             << GET_NC_NAME_P(_xCoordVar) << "\".\n\n";
      }
    }

  }

  if (_xDim == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find X dimension (degrees_east) in netCDF file.\n\n";
    return;
  }

  if (_yDim == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find Y dimension (degrees_north) in netCDF file.\n\n";
    return;
  }

  if (_xCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    return;
  }

  if (_yCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    return;
  }

  long lon_counts = _xDim->getSize();
  long lat_counts = _yDim->getSize();
  if (get_data_size(_xCoordVar) != lon_counts ||
      get_data_size(_yCoordVar) != lat_counts)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    return;
  }

  // Store spacing in LatLon data structure
  bool swap_to_north;
  LatLonData ll_data = get_data_from_lat_lon_vars(_yCoordVar, _xCoordVar,
                                                  lat_counts, lon_counts,
                                                  swap_to_north);

  // Fill in the Rotated LatLon data structure
  RotatedLatLonData data;

  data.name = rotated_latlon_proj_type;

  // Derive south pole location from the north pole:
  // - Reverse the sign of the latitude
  // - Add 180 to the longitude and switch from degrees east to west
  data.true_lat_south_pole = -1.0 * get_att_value_double(grid_np_lat_att);
  double np_lon = rescale_lon(get_att_value_double(grid_np_lon_att));
  data.true_lon_south_pole = rescale_lon(-1.0 * (np_lon + 180.0));

  // Copied from the LatLon data structure
  data.rot_lat_ll = ll_data.lat_ll;
  data.rot_lon_ll = ll_data.lon_ll;
  data.delta_rot_lat = ll_data.delta_lat;
  data.delta_rot_lon = ll_data.delta_lon;

  // Grid dimension
  data.Nlon = _xDim->getSize();
  data.Nlat = _yDim->getSize();

  data.aux_rotation = 0;

  data.dump();

  grid.set(data);
  grid.set_swap_to_north(swap_to_north);

  if(grid_np_lat_att) delete grid_np_lat_att;
  if(grid_np_lon_att) delete grid_np_lon_att;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_stereographic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_stereographic()";

  mlog << Error << "\n" << method_name << " -> "
       << "Stereographic grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_transverse_mercator(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_transverse_mercator()";

  mlog << Error << "\n" << method_name << " -> "
       << "Transverse mercator grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_vertical_perspective(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_vertical_perspective()";

  mlog << Error << "\n" << method_name << " -> "
       << "Vertical perspective grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_geostationary(
    const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_geostationary() ";

  // perspective_point_height
  NcVarAtt *perspective_point_height_att = get_nc_att(
    grid_mapping_var, (string)"perspective_point_height");
  if (IS_INVALID_NC_P(perspective_point_height_att))
  {
    mlog << Warning << "\n" << method_name
         << "-> Cannot get perspective_point_height attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // semi_major_axis
  NcVarAtt *semi_major_axis_att = get_nc_att(
    grid_mapping_var, (string)"semi_major_axis");
  if (IS_INVALID_NC_P(semi_major_axis_att))
  {
    mlog << Warning << "\n" << method_name
         << "-> Cannot get semi_major_axis attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // semi_minor_axis
  NcVarAtt *semi_minor_axis_att = get_nc_att(
    grid_mapping_var, (string)"semi_minor_axis");
  if (IS_INVALID_NC_P(semi_minor_axis_att))
  {
    mlog << Warning << "\n" << method_name
         << "-> Cannot get semi_minor_axis attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // inverse_flattening
  NcVarAtt *inverse_flattening_att = get_nc_att(
    grid_mapping_var, (string)"inverse_flattening");
  if (IS_INVALID_NC_P(inverse_flattening_att))
  {
    mlog << Warning << "\n" << method_name
         << "-> Cannot get inverse_flattening attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // latitude_of_projection_origin
  NcVarAtt *proj_origin_lat_att = get_nc_att(
    grid_mapping_var, (string)"latitude_of_projection_origin");
  if (IS_INVALID_NC_P(proj_origin_lat_att))
  {
    mlog << Warning << "\n" << method_name
         << "-> Cannot get latitude_of_projection_origin attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // longitude_of_projection_origin
  NcVarAtt *proj_origin_lon_att = get_nc_att(
    grid_mapping_var, (string)"longitude_of_projection_origin");
  if (IS_INVALID_NC_P(proj_origin_lon_att))
  {
    mlog << Warning << "\n" << method_name
         << "-> Cannot get longitude_of_projection_origin attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // sweep_angle_axis
  NcVarAtt *sweep_angle_axis_att = get_nc_att(
    grid_mapping_var, (string)"sweep_angle_axis");
  if (IS_INVALID_NC_P(sweep_angle_axis_att))
  {
    mlog << Warning << "\n" << method_name
         << "-> Cannot get sweep_angle_axis attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    return;
  }

  // Look for the x/y dimensions and x/y coordinate variables
  find_xy_vars(method_name);

  bool do_exit = false;
  if (_xDim == nullptr)
  {
    mlog << Warning << "\n" << method_name
         << "-> Didn't find X dimension (projection_x_coordinate) in netCDF file.\n\n";
    do_exit = true;
  }

  if (_yDim == nullptr)
  {
    mlog << Warning << "\n" << method_name
         << "-> Didn't find Y dimension (projection_y_coordinate) in netCDF file.\n\n";
    do_exit = true;
  }

  if (_xCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name
         << "-> Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    do_exit = true;
  }

  if (_yCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name
         << "-> Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    do_exit = true;
  }

  if (get_data_size(_xCoordVar) != (int) GET_NC_SIZE_P(_xDim) ||
      get_data_size(_yCoordVar) != (int) GET_NC_SIZE_P(_yDim))
  {
    mlog << Warning << "\n" << method_name
         << "-> Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    do_exit = true;
  }

  if (do_exit) return;

  // Figure out the dx/dy  and x/y pin values from the dimension variables

  long x_counts = GET_NC_SIZE_P(_xDim);
  vector<double> x_values(x_counts);

  get_nc_data(_xCoordVar, x_values);

  long y_counts = GET_NC_SIZE_P(_yDim);
  vector<double> y_values(y_counts);

  get_nc_data(_yCoordVar, y_values);

  // Unit conversion

  // Calculate dx and dy assuming they are constant.  MET requires that dx be
  // equal to dy

  int bound_count = 0;
  for (int j=0; j<_numDims; ++j)  {
     if (0 == strcmp(_dims[j]->getName().c_str(), "number_of_image_bounds")) {
        bound_count = _dims[j]->getSize();
        break;
     }
  }
  NcVar *var_x_bound = (NcVar *)nullptr;
  NcVar *var_y_bound = (NcVar *)nullptr;
  for (int j=0; j<Nvars; ++j)  {
    if ( Var[j].name == "x_image_bounds" ) var_x_bound = Var[j].var;
    else if ( Var[j].name == "y_image_bounds" ) var_y_bound = Var[j].var;
  }


  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  GoesImagerData data;
  NumArray double_datas;
  data.reset();

  //data.name = grid_mapping_var->getName().c_str();
  data.name = grid_mapping_name_geostationary;
  data.perspective_point_height = get_att_value_double(perspective_point_height_att);
  data.semi_major_axis = get_att_value_double(semi_major_axis_att);
  data.semi_minor_axis = get_att_value_double(semi_minor_axis_att);
  data.inverse_flattening = get_att_value_double(inverse_flattening_att);
  data.lat_of_projection_origin = get_att_value_double(proj_origin_lat_att);
  data.lon_of_projection_origin = get_att_value_double(proj_origin_lon_att);
  data.nx = x_counts;
  data.ny = y_counts;
  data.dx_rad = (x_values[x_counts-1] - x_values[0]) / (x_counts - 1);
  data.dy_rad = (y_values[y_counts-1] - y_values[0]) / (y_counts - 1);
  if (bound_count > 0) {
    data.x_image_bounds.resize(bound_count, bad_data_double);
    data.y_image_bounds.resize(bound_count, bad_data_double);
    if (nullptr != var_x_bound) get_nc_data(var_x_bound, data.x_image_bounds);
    if (nullptr != var_y_bound) get_nc_data(var_y_bound, data.y_image_bounds);
  }

  double flatten = 1.0/data.inverse_flattening;
  data.ecc = sqrt(2.0*flatten - flatten*flatten);
  data.radius_ratio2 = pow((data.semi_major_axis/data.semi_minor_axis), 2.0);
  data.inv_radius_ratio2 = 1.0/data.radius_ratio2;
  data.H = data.perspective_point_height + data.semi_major_axis;

  data.x_values = x_values;
  data.y_values = y_values;

  // Get scene_id: "Full Disk", "CONUS", or "Mesoscale"
  ConcatString scene_id;
  if (get_global_att(_ncFile, (string)"scene_id", scene_id)) {
    data.scene_id = scene_id;
  }

  data.dump();

  // Note: Computing lat/lon was deferred because it took 1 minutes

  grid.set(data);
  grid_ready = true;

  if (perspective_point_height_att) delete perspective_point_height_att;
  if (semi_major_axis_att)          delete semi_major_axis_att;
  if (semi_minor_axis_att)          delete semi_minor_axis_att;
  if (inverse_flattening_att)       delete inverse_flattening_att;
  if (proj_origin_lat_att)          delete proj_origin_lat_att;
  if (proj_origin_lon_att)          delete proj_origin_lon_att;
  if (sweep_angle_axis_att)         delete sweep_angle_axis_att;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::get_grid_from_coordinates(const NcVar *data_var) {
  static const string method_name = "NcCfFile::get_grid_from_coordinates()";

  // Find the a lat/lon grid from the coordinates attribute.
  // Get the dimensions from the coordinate variables.

  mlog << Debug(6) << "\n" << method_name << " -> "
       << "collect GRID info from \"" << GET_NC_NAME_P(data_var) << "\".\n\n";

  NcVarAtt *coordinates_att = get_nc_att(data_var, coordinates_att_name);

  if (IS_VALID_NC_P(coordinates_att)) {
    ConcatString coordinates_value, units_value, axis_value;
    get_att_value_chars(coordinates_att, coordinates_value);
    StringArray sa = coordinates_value.split(" ");
    int count = sa.n_elements();
    if (count >= 2) {
      x_dim_var_name = sa[count-2];
      y_dim_var_name = sa[count-1];
    }

    bool is_x_dim_var, is_y_dim_var;
    float lat_missing_value = bad_data_double;
    float lon_missing_value = bad_data_double;
    for (int var_num = 0; var_num < Nvars; ++var_num) {
      is_x_dim_var = is_y_dim_var = false;
      for (int cIdx = 0; cIdx<count; cIdx++) {
        if ( Var[var_num].name == sa[cIdx]) {
          if (get_var_units(Var[var_num].var, units_value)) {
            if (is_nc_unit_latitude(units_value.c_str())) {
              y_dim_var_name = sa[cIdx];
              is_y_dim_var = true;
              mlog << Debug(4) << method_name << " -> "
                   << "found the latitude variable \"" << Var[var_num].name << "\"\n";
            }
            else if (is_nc_unit_longitude(units_value.c_str())) {
              x_dim_var_name = sa[cIdx];
              is_x_dim_var = true;
              mlog << Debug(4) << method_name << " -> "
                   << "found the longitude variable \"" << Var[var_num].name << "\"\n";
            }
            else if (!is_nc_unit_time(units_value.c_str())) {
              if (!get_var_axis(Var[var_num].var, axis_value)
                  || (axis_value != "Z" && axis_value != "z")) {
                mlog << Debug(4) << "\n" << method_name << " -> "
                     << "unknown units [" << units_value << "] for the coordinate variable ["
                     << Var[var_num].name << "]\n\n";
              }
            }
          }
          break;
        }
      }
      if (is_y_dim_var || Var[var_num].name == y_dim_var_name) {
        _yCoordVar = Var[var_num].var;
        get_var_fill_value(_yCoordVar, lat_missing_value);
      }
      else if (is_x_dim_var || Var[var_num].name == x_dim_var_name) {
        _xCoordVar = Var[var_num].var;
        get_var_fill_value(_xCoordVar, lon_missing_value);
      }
    }

    if (_xCoordVar == nullptr) {
      mlog << Warning << "\n" << method_name << " -> "
           << "Didn't find X coord variable (" << x_dim_var_name
           << ") in netCDF file.\n\n";
      if (coordinates_att) delete coordinates_att;
      return false;
    }

    if (_yCoordVar == nullptr) {
      mlog << Warning << "\n" << method_name << " -> "
           << "Didn't find Y coord variable (" << y_dim_var_name
           << ") in netCDF file.\n\n";
      if (coordinates_att) delete coordinates_att;
      return false;
    }

    StringArray dimNames;
    get_dim_names(_xCoordVar, &dimNames);
    NcDim cur_xDim, cur_yDim;
    int dim_count = dimNames.n_elements();
    if (dim_count == 2) {
      x_dim_var_name = dimNames[dim_count-1];
      y_dim_var_name = dimNames[dim_count-2];
      cur_xDim = get_nc_dim(_xCoordVar, string(x_dim_var_name));
      cur_yDim = get_nc_dim(_xCoordVar, string(y_dim_var_name));
    }
    else {
      x_dim_var_name = dimNames[0];
      if (dimNames.n_elements() == 1) cur_xDim = get_nc_dim(_xCoordVar, string(x_dim_var_name));
      get_dim_names(_yCoordVar, &dimNames);
      y_dim_var_name = dimNames[0];
      if (dimNames.n_elements() == 1) cur_yDim = get_nc_dim(_yCoordVar, string(y_dim_var_name));
    }

    long lat_counts = GET_NC_SIZE(cur_yDim);
    long lon_counts = GET_NC_SIZE(cur_xDim);
    long x_size = get_data_size(_xCoordVar);
    long y_size = get_data_size(_yCoordVar);
    long latlon_counts = lon_counts*lat_counts;
    if ((x_size != lon_counts && x_size != latlon_counts) ||
        (y_size != lat_counts && x_size != latlon_counts))
    {
      mlog << Warning << "\n" << method_name << " -> "
           << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
      if (coordinates_att) delete coordinates_att;
      return false;
    }

    if (coordinates_att) {
      delete coordinates_att;
      coordinates_att = (NcVarAtt *)nullptr;
    }
    get_grid_from_lat_lon_vars(_yCoordVar, _xCoordVar, lat_counts, lon_counts);
  }

  if (coordinates_att) delete coordinates_att;
  return true;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::get_grid_from_dimensions()
{
  static const string method_name = "NcCfFile::get_grid_from_dimensions()";

  // Currently, we can only intuit a lat/lon grid from the dimensions.
  // Start by looking for the lat/lon dimensions in the file

  NcVar coord_var;
  const NcVarAtt units_att;
  ConcatString dim_units;
  ConcatString dim_name;
  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // The lat/lon dimensions are identified by their units

    dim_name = _dims[dim_num]->getName().c_str();
    if (!has_var(_ncFile, dim_name.c_str())) {
      if ( dim_lat_nt == dim_name) {
        dim_name = var_lat_nt;
      }
      else if (dim_lon_nt == dim_name) {
        dim_name = var_lon_nt;
      }

      if (!has_var(_ncFile, dim_name.c_str())) {
        mlog << Debug(6) << method_name << " -> " << "The coordinate variable \""
             << _dims[dim_num]->getName() << "\" does not exist.\n";
        continue;
      }
    }

    coord_var = get_nc_var(_ncFile, dim_name.c_str());
    if (IS_INVALID_NC(coord_var))
      continue;

    if (!get_att_value_string(&coord_var, (string)"units", dim_units))
      continue;

    // See if this is a lat or lon dimension

    if (is_nc_unit_latitude(dim_units.c_str()))
    {
      if (_yDim == nullptr)
      {
        _yDim = _dims[dim_num];

        y_dim_var_name = GET_NC_NAME_P(_yDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if ( Var[var_num].name == y_dim_var_name)
          {
            _yCoordVar = Var[var_num].var;
            break;
          }
          if (( Var[var_num].name == var_lat_nt)
              && ( y_dim_var_name == dim_lat_nt)) {
            y_dim_var_name = dim_lat_nt;
            _yCoordVar = Var[var_num].var;
            break;
          }
        }
      }
      else
      {
        mlog << Warning << "\n" << method_name << " -> "
             << "Found multiple variables for latitude, using \""
             << GET_NC_NAME_P(_yCoordVar) << "\".\n\n";
      }
    }
    else if (is_nc_unit_longitude(dim_units.c_str()))
    {
      if (_xDim == nullptr)
      {
        _xDim = _dims[dim_num];

        x_dim_var_name = GET_NC_NAME_P(_xDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if ( Var[var_num].name == x_dim_var_name)
          {
            _xCoordVar = Var[var_num].var;
            break;
          }
          if (( Var[var_num].name == var_lon_nt)
              && (x_dim_var_name == dim_lon_nt)) {
            x_dim_var_name = dim_lon_nt;
            _xCoordVar = Var[var_num].var;
            break;
          }
        }
      }
      else
      {
        mlog << Warning << "\n" << method_name << " -> "
             << "Found multiple variables for longitude, using \""
             << GET_NC_NAME_P(_xCoordVar) << "\".\n\n";
      }
    }

  }

  if (_xDim == nullptr || _yDim == nullptr)
    return false;

  if (_xCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    return false;
  }

  if (_yCoordVar == nullptr)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    return false;
  }

  long lat_counts = GET_NC_SIZE_P(_yDim);
  long lon_counts = GET_NC_SIZE_P(_xDim);
  get_grid_from_lat_lon_vars(_yCoordVar, _xCoordVar, lat_counts, lon_counts);

  return true;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_from_lat_lon_vars(NcVar *lat_var, NcVar *lon_var,
                                          const long lat_counts, const long lon_counts) {
  static const string method_name = "NcCfFile::get_grid_from_lat_lon_vars()";

  bool swap_to_north;
  LatLonData data = get_data_from_lat_lon_vars(lat_var, lon_var,
                                               lat_counts, lon_counts,
                                               swap_to_north);

  data.dump();

  grid.set(data);   // resets swap_to_north to false
  if (swap_to_north) grid.set_swap_to_north(true);
}


////////////////////////////////////////////////////////////////////////


LatLonData NcCfFile::get_data_from_lat_lon_vars(NcVar *lat_var, NcVar *lon_var,
                                                const long lat_counts, const long lon_counts,
                                                bool &swap_to_north) {
  static const string method_name = "NcCfFile::get_data_from_lat_lon_vars()";

  // Figure out the dlat/dlon values from the dimension variables

  LatLonData data;
  data.name = latlon_proj_type;
  data.Nlat = (int)lat_counts;
  data.Nlon = (int)lon_counts;

  long x_size = get_data_size(lon_var);
  long y_size = get_data_size(lat_var);
  long latlon_counts = lon_counts*lat_counts;
  bool two_dim_coord = (x_size == latlon_counts) && (y_size == latlon_counts );

  if( !two_dim_coord && (x_size != lon_counts || y_size != lat_counts))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    return data;
  }

  vector<double> lat_values(lat_counts, bad_data_double);
  vector<double> lon_values(lon_counts, bad_data_double);
  bool lat_first = false;
  if (two_dim_coord) {
    lat_first = (lat_counts == get_dim_size(lat_var, 0));
    LongArray cur, length;  // {0,0}, {1,1}
    cur.add(0);
    cur.add(0);
    length.add(1);
    length.add(1);
    if (lat_first) length[0] = lat_counts;
    else length[1] = lat_counts;
    get_nc_data(lat_var,lat_values, length, cur);

    length[0] = length[1] = 1;
    if (lat_first) length[1] = lon_counts;
    else length[0] = lon_counts;
    get_nc_data(lon_var,lon_values, length, cur);
  }
  else {
    get_nc_data(lat_var,lat_values);
    get_nc_data(lon_var,lon_values);
  }
  data.lat_ll = lat_values[0];
  data.lon_ll = rescale_lon(-lon_values[0]);

  // Calculate dlat and dlon assuming they are constant.

  double dlat = lat_values[1] - lat_values[0];
  double dlon = rescale_lon(lon_values[1] - lon_values[0]);
  mlog << Debug(7) << method_name << " -> lat[0]=" << lat_values[0]
       << " lat[" << (lat_counts-1) << "]=" << lat_values[lat_counts-1]
       << " dlat=" << dlat << " lon[0]=" << lon_values[0]
       << " lon[" << (lon_counts-1) << "]=" << lon_values[lon_counts-1]
       << " dlon=" << dlon << "\n";

  data.delta_lat = dlat;
  data.delta_lon = dlon;

  ConcatString point_nccf;
  bool skip_sanity_check = get_att_value_string(_ncFile, nc_att_met_point_nccf, point_nccf);
  if (!skip_sanity_check) {
    get_env(nc_att_met_point_nccf, point_nccf);
    skip_sanity_check = (point_nccf == "yes");
  }

  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  if (!skip_sanity_check) {
    double degree_tolerance;
    float lat_missing_value = bad_data_double;
    float lon_missing_value = bad_data_double;
    bool sanity_check_failed = false;
    get_var_att_float(lat_var, fill_value_att_name, lat_missing_value);
    get_var_att_float(lon_var, fill_value_att_name, lon_missing_value);

    degree_tolerance = fabs(dlat * DELTA_TOLERANCE_PERCENT);
    for (int i = 1; i < (int)lat_counts; ++i)
    {
      if ((fabs(lat_missing_value - lat_values[i]) < degree_tolerance) ||
          (fabs(lat_missing_value - lat_values[i-1]) < degree_tolerance)) continue;
      double curr_delta = lat_values[i] - lat_values[i-1];
      if (fabs(curr_delta - dlat) > degree_tolerance)
      {
        mlog << Debug(4) << method_name << " -> lat["
             << i-1 << "]=" << lat_values[i-1] << " lat["
             << i << "]=" << lat_values[i] << "  "
             << fabs(curr_delta - dlat) << " > " << degree_tolerance << "\n";
        mlog << Warning << "\n" << method_name << " -> "
             << "MET can only process Latitude/Longitude files where the latitudes are evenly spaced (dlat="
             << dlat <<", delta[" << i << "]=" << curr_delta << ")\n\n";
        sanity_check_failed = true;
        break;
      }
    }

    degree_tolerance = fabs(dlon * DELTA_TOLERANCE_PERCENT);
    for (int i = 1; i < (int)lon_counts; ++i)
    {
      if ((fabs(lon_missing_value - lon_values[i]) < degree_tolerance) ||
          (fabs(lon_missing_value - lon_values[i-1]) < degree_tolerance)) continue;
      double curr_delta = rescale_lon(lon_values[i] - lon_values[i-1]);
      if (fabs(curr_delta - dlon) > degree_tolerance)
      {
        mlog << Debug(4) << method_name << " -> lon["
             << i-1 << "]=" << lon_values[i-1] << " lon["
             << i << "]=" << lon_values[i] << "  "
             << fabs(curr_delta - dlon) << " > " << degree_tolerance << "\n";
        mlog << Warning << "\n" << method_name << " -> "
             << "MET can only process Latitude/Longitude files where the longitudes are evenly spaced (dlon="
             << dlon <<", delta[" << i << "]=" << curr_delta << ")\n\n";
        sanity_check_failed = true;
        break;
      }
    }

    if (sanity_check_failed) {
      mlog << Warning << "\n" << method_name << " -> "
           << "Please check the input data is the lat/lon projection\n\n";
      return data;
    }
  }

  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  // Note that I am assuming that the data is ordered from the lower-left
  // corner.  I think this will generally be the case, but it is not
  // guaranteed anywhere that I see.  But if this is not the case, then we
  // will probably also need to reorder the data itself.

  if (dlat < 0) {
    swap_to_north = true;
    data.delta_lat = -dlat;
    data.lat_ll = lat_values[lat_counts-1];
  }
  else {
    swap_to_north = false;
  }
  grid_ready = true;

  return data;

}


////////////////////////////////////////////////////////////////////////
