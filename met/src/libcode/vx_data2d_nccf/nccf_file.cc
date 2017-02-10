

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
#include <string>
#include <time.h>

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "nccf_file.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


static const int  max_met_args           = 30;

const double NcCfFile::DELTA_TOLERANCE = 15.0;

static const char x_dim_key_name[] = "projection_x_coordinate";
static const char y_dim_key_name[] = "projection_y_coordinate";
static ConcatString t_dim_name = "Time";

static ConcatString x_dim_var_name;
static ConcatString y_dim_var_name;
//const char * t_dim_var_name;

#define USE_BUFFER  1

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

  _ncFile = (NcFile *) 0;
  _dims = (NcDim **) 0;
  Var = (NcVarInfo *) 0;

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
    _ncFile = (NcFile *)0;
  }

  // Reclaim the dimension pointers

  if (_dims)
  {
    delete [] _dims;
    _dims = (NcDim **)0;
  }

  _numDims = 0;

  _dimNames.clear();

  _xDim = _yDim = _tDim = (NcDim *) 0;

  // Reclaim the variable pointers

  if (Var)
  {
    delete [] Var;
    Var = (NcVarInfo *)0;
  }

  Nvars = 0;

  // Reset the time values

  ValidTime.clear();
  InitTime = (unixtime)0;

  //  done

  return;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::open(const char * filepath)
{
  unixtime ut;
  int sec_per_unit;

  // Close any open files and clear out the associated members

  close();

  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program. In the case of this example, we just exit with
  // an NC_ERR error code.

  //FIXME: Commented out with NetcDf4 enabling
  //NcError err(NcError::silent_nonfatal);

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
  //get_global_dims(_ncFile, &_numDims, _dims, &_dimNames);
  //get_global_dims(Nc, &Ndims, Dim, &DimNames);

  StringArray gDimNames;
  get_dim_names(_ncFile, &gDimNames);

  for (int j=0; j<_numDims; ++j)  {
     NcDim dim = get_nc_dim(_ncFile, gDimNames[j]);
     _dims[j] = new NcDim(dim);
  }

  // Pull out the valid and init times

  ConcatString units;
  NcVar valid_time_var_T = get_var(_ncFile, "time");
  NcVar *valid_time_var = &valid_time_var_T;
  if (IS_INVALID_NC_P(valid_time_var))
  {

    mlog << Debug(4) << "NcCfFile::open() -> "
         << "could not extract valid time from the "
         << "\"time\" variable.\n";

    // Time not in file, get from file name
    if ((ut = get_valid_time_from_file_path(filepath)) == 0)
    {
       mlog << Debug(4) << "NcCfFile::open() -> "
            << "could not extract valid time from file name.\n";
    }
    if( ut == 0 ) {
       mlog << Warning << "\nNcCfFile::open() -> "
            << "could not determine the valid time, using 0.\n\n";
    }
    ValidTime.add(ut);
  }
  else
  {

    // Store the dimension for the time variable as the time dimension
    tDim = get_nc_dim(valid_time_var, 0);
    _tDim = &tDim;
    t_dim_name = GET_NC_NAME(tDim).c_str();

    // Parse the units for the time variable.
    NcVarAtt units_att = get_nc_att(valid_time_var, "units", false);
    if (!IS_INVALID_NC(units_att))
    {
      //char *units = units_att->getValues(att->as_string(0);
      get_att_value_chars(&units_att, units);
      //get_var_att(units_att, units);

      if (units.length() == 0)
      {
         mlog << Warning << "\nNcCfFile::open() -> "
              << "the \"time\" variable must contain a \"units\" attribute. "
              << "Using valid time of 0\n\n";
         ut = sec_per_unit = 0;
      }
      else
      {
         mlog << Debug(4) << "NcCfFile::open() -> "
              << "parsing units for the time variable \"" << units << "\"\n";
         parse_cf_time_string(units, ut, sec_per_unit);
      }
    }
    else
    {
      ut = sec_per_unit = 0;
    }

    // Determine the number of times present.
    int n_times = (int) get_data_size(valid_time_var);

    for(int i=0; i<n_times; i++)
    {
      double time_value = get_double_var(valid_time_var, i);
      ValidTime.add((unixtime)ut + sec_per_unit * time_value);
    }
  }

  NcVar init_time_var = get_var(_ncFile, "forecast_reference_time");
  if (IS_INVALID_NC(init_time_var))
  {

    mlog << Debug(4) << "NcCfFile::open() -> "
         << "could not extract init time from the "
         << "\"forecast_reference_time\" variable.\n";

    // Time not in file, get from file name
    if ((InitTime = get_init_time_from_file_path(filepath)) == 0)
    {
      mlog << Debug(4) << "NcCfFile::open() -> "
           << "could not extract init time from file name.\n";
    }
  }
  else
  {

    // Parse the units for the time variable.
    NcVarAtt units_att = get_nc_att(&init_time_var, "units");
    if (!IS_INVALID_NC(units_att))
    {
      get_att_value_chars(&units_att, units);

      if (units.length() == 0)
      {
         mlog << Warning << "\nNcCfFile::open() -> "
              << "the \"forecast_reference_time\" variable must contain a \"units\" attribute.\n\n";
         ut = sec_per_unit = 0;
      }
      else
      {
         mlog << Debug(4) << "NcCfFile::open() -> "
              << "parsing units for the forecast_reference_time variable \"" << units << "\"\n";
         parse_cf_time_string(units, ut, sec_per_unit);
      }
    }
    else
    {
      ut = sec_per_unit = 0;
    }

    double time_value = get_double_var(&init_time_var,(int)0);
    InitTime = (unixtime)ut + sec_per_unit * time_value;
  }

  // Pull out the variables


  StringArray varNames;
  Nvars = get_var_names(_ncFile, &varNames);
  Var = new NcVarInfo [Nvars];
  //get_vars_info(Nc, &Var);

  for (int j=0; j<Nvars; ++j)  {
    const char * c = (const char *) 0;
    NcVar v = get_var(_ncFile, varNames[j]);

    Var[j].var = new NcVar(v);

    Var[j].name = GET_NC_NAME(v).c_str();

    int dim_count = GET_NC_DIM_COUNT(v);
    Var[j].Ndims = dim_count;

    Var[j].Dims = new NcDim * [dim_count];

    //  parse the variable attributes
    get_att_str( Var[j], "long_name",  Var[j].long_name_att );
    get_att_str( Var[j], "units",      Var[j].units_att     );

  }   //  for j

  // Pull out the grid.  This must be done after pulling out the dimension
  // and variable information since this information is used to pull out the
  // grid.  This call sets the _xDim and _yDim pointers.

  read_netcdf_grid();

  // Now go back through the variables and use _xDim, _yDim, and _tDim
  // to set the slots.
  // Should be called after read_netcdf_grid() is called

  StringArray dimNames;
  for (int j=0; j<Nvars; ++j)  {

    int dim_count = GET_NC_DIM_COUNT_P(Var[j].var);
    NcVar *v = Var[j].var;

    dimNames.clear();
    get_dim_names(v, &dimNames);

    for (int k=0; k<(dim_count); ++k)  {
      const char * c = dimNames[k];
      NcDim *dim = Var[j].Dims[k];

      if ((dim == _xDim) || (strcmp(c,x_dim_var_name) == 0)) {
         Var[j].x_slot = k;
      }
      if ((dim == _yDim) || (strcmp(c, y_dim_var_name) == 0)) {
         Var[j].y_slot = k;
      }
      if ((dim == _tDim) || (strcmp(c, t_dim_name) == 0)) {
         Var[j].t_slot = k;
      }
    }
  }   //  for j

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

  if (strcmp(tokens[0], "3B42") != 0)
    return 0;

  // <yyyymmdd>

  if (strlen(tokens[1]) != 8)
    return 0;

  for (int i = 0; i < 8; ++i)
  {
    if (!isdigit(tokens[1][i]))
      return 0;
  }

  // <hh>

  if (strlen(tokens[2]) != 2)
    return 0;

  for (int i = 0; i < 2; ++i)
  {
    if (!isdigit(tokens[2][i]))
      return 0;
  }

  // 7

  if (strcmp(tokens[3], "7") != 0)
    return 0;

  // G3

  if (strcmp(tokens[4], "G3") != 0)
    return 0;

  // nc

  if (strcmp(tokens[5], "nc") != 0)
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

  if (strcmp(tokens[0], "3B42_daily") != 0)
    return 0;

  // <yyyy>

  if (strlen(tokens[1]) != 4)
    return 0;

  for (int i = 0; i < 4; ++i)
  {
    if (!isdigit(tokens[1][i]))
      return 0;
  }

  // <mm>

  if (strlen(tokens[2]) != 2)
    return 0;

  for (int i = 0; i < 2; ++i)
  {
    if (!isdigit(tokens[2][i]))
      return 0;
  }

  // <dd>

  if (strlen(tokens[3]) != 2)
    return 0;

  for (int i = 0; i < 2; ++i)
  {
    if (!isdigit(tokens[3][i]))
      return 0;
  }

  // 7

  if (strcmp(tokens[4], "7") != 0)
    return 0;

  // G3

  if (strcmp(tokens[5], "G3") != 0)
    return 0;

  // nc

  if (strcmp(tokens[6], "nc") != 0)
    return 0;

  // If we get here, this is a TRMM 3B42 daily file.  Extract the file time.

  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));

  time_struct.tm_year = atoi(tokens[1]) - 1900;
  time_struct.tm_mon = atoi(tokens[2]) - 1;
  time_struct.tm_mday = atoi(tokens[3]);

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

  sprintf(junk, "%s %d, %d   %2d:%02d:%02d",
          short_month_name[month], day, year, hour, minute, second);

  out << junk << "\n";

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


double NcCfFile::getData(NcVar * var, const LongArray & a) const
{
  if (!args_ok(a))
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "bad arguments:\n";
    a.dump(cerr);
    exit(1);
  }

  int dim_count = get_dim_count(var);
  if (dim_count != a.n_elements())
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "needed " << (dim_count) << " arguments for variable "
         << (GET_NC_NAME_P(var)) << ", got " << (a.n_elements()) << "\n\n";
    exit(1);
  }

  if (dim_count >= max_met_args)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "too may arguments for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";
    exit(1);
  }

  //long counts[max_met_args];
  long counts[dim_count];

  for (int j = 0; j < (dim_count); ++j)
    counts[j] = 1;

  //if (!(var->set_cur((long *)a)))
  //{
  //  mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
  //       << "can't set corner for variable \"" << (var->getName()) << "\"\n\n";
  //  exit(1);
  //}

  bool status;
  double d;

  switch (GET_NC_TYPE_ID_P(var))
  {
    //case ncInt:
    case NcType::nc_INT:
    {
      int i;

      status = get_nc_data(var, &i, (long *)a);
      d = (double) (i);
      break;
    }

    //case ncFloat:
    case NcType::nc_FLOAT:
    {
      float f;

      status = get_nc_data(var, &f, (long *)a);
      d = (double) (f);
      break;
    }

    //case ncDouble:
    case NcType::nc_DOUBLE:
    {
      status = get_nc_data(var, &d, (long *)a);
      break;
    }

    default:
    {
      mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
           << "bad type for variable \"" << (GET_NC_NAME_P(var)) << "\"\n\n";
      exit(1);
      break;
    }
  }   //  switch

  if (!status)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "bad status for var->get()\n\n";
    exit(1);
  }

  //  done

  return d;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::getData(NcVar * v, const LongArray & a, DataPlane & plane) const
{
  if (!args_ok(a))
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
         << "bad arguments:\n";
    a.dump(cerr);
    exit(1);
  }

  int dim_count = get_dim_count(v);
  if (dim_count != a.n_elements())
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) -> "
         << "needed " << (dim_count) << " arguments for variable "
         << (GET_NC_NAME_P(v)) << ", got " << (a.n_elements()) << "\n\n";
    exit(1);
  }

  if (dim_count >= max_met_args)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) -> "
         << "too may arguments for variable \"" << (GET_NC_NAME_P(v)) << "\"\n\n";
    exit(1);
  }

  //  find varinfo's

  bool found = false;
  NcVarInfo *var = (NcVarInfo *)0;

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
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
         << "variable " << (GET_NC_NAME_P(v)) << " not found!\n\n";
    exit(1);
  }

  //  check star positions and count

  int count = 0;

  for (int j = 0; j < a.n_elements(); ++j)
  {
    if (a[j] == vx_data2d_star)
    {
      ++count;

      if ((j != var->x_slot) && (j != var->y_slot))
      {
        mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
             << "star found in bad slot\n\n";
        exit(1);
      }
    }
  }

  if (count != 2)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
         << "bad star count ... " << count << "\n\n";
    exit(1);
  }

  //  check slots

  const int x_slot = var->x_slot;
  const int y_slot = var->y_slot;

  if (x_slot < 0 || y_slot < 0)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
         << "bad x|y|z slot\n\n";
    exit(1);
  }

  //
  //  get the bad data values
  //

  double missing_value = get_var_missing_value(v);
  double fill_value    = get_var_fill_value(v);

  //  set up the DataPlane object

  const int nx = grid.nx();
  const int ny = grid.ny();

  plane.clear();
  plane.set_size(nx, ny);

  //  get the data
#ifdef USE_BUFFER
  int    i[nx];
  float  f[nx];
  double d[nx];

  long offsets[dim_count];
  long lengths[dim_count];
  for (int k=0; k<dim_count; k++) {
    offsets[k] = (a[k] == vx_data2d_star) ? 0 : a[k];
    lengths[k] = 1;
  }

  //offsets[x_slot] = 0;
  //offsets[y_slot] = 0;
  //lengths[x_slot] = 1;
  //lengths[y_slot] = ny;
  offsets[x_slot] = 0;
  lengths[x_slot] = nx;

  //status = false;
  int type_id = GET_NC_TYPE_ID_P(v);
  for (int y=0; y<ny; ++y)  {
    offsets[y_slot] = y;
    switch ( type_id )  {

      //case ncInt:
      case NcType::nc_INT:
        get_nc_data(v, (int *)&i, lengths, offsets);
        for (int x=0; x<nx; ++x)  {
          d[x] = (double)i[x];
        }
        break;

      //case ncFloat:
      case NcType::nc_FLOAT:
        get_nc_data(v, (float *)&f, lengths, offsets);
        for (int x=0; x<nx; ++x)  {
          d[x] = (double)f[x];
        }
        break;

      //case ncDouble:
      case NcType::nc_DOUBLE:
        get_nc_data(v, (double *)&d, lengths, offsets);
        break;

      default:
        mlog << Error << "\nMetNcFile::data(NcVar *, const LongArray &) const -> "
             << " bad type for variable \"" << (GET_NC_NAME_P(v)) << "\"\n\n";
        exit ( 1 );
        break;

    }   //  switch

    LongArray b = a;

    for (int x = 0; x< nx; ++x)
    {
      //b[y_slot] = y;

      //double value = getData(v, b);
      double value = d[x];

      if(is_eq(value, missing_value) || is_eq(value, fill_value)) {
         value = bad_data_double;
      }

      plane.set(value, x, y);

    }   //  for y
  }   //  for x

#else
  LongArray b = a;

  for (int x = 0; x < nx; ++x)
  {
    b[x_slot] = x;

    for (int y = 0; y < ny; ++y)
    {
      b[y_slot] = y;

      double value = getData(v, b);

      if(is_eq(value, missing_value) || is_eq(value, fill_value)) {
         value = bad_data_double;
      }

      plane.set(value, x, y);

    }   //  for y
  }   //  for x
#endif

  //  done

  return true;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::getData(const char *var_name,
                       const LongArray &a, DataPlane &plane,
                       NcVarInfo *&info) const
{
  info = find_var_name(var_name);
  if (info == 0)
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

  plane.set_init(init_ut);
  plane.set_valid(valid_ut);
  plane.set_lead(valid_ut - init_ut);
  plane.set_accum(info->AccumTime);

  //  done

  return found;
}


////////////////////////////////////////////////////////////////////////


NcVarInfo* NcCfFile::find_var_name(const char * var_name) const
{
  for (int i = 0; i < Nvars; i++)
    if (Var[i].name == var_name)
      return &Var[i];

  return 0;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::read_netcdf_grid()
{
  // Loop through the variables looking for the first gridded variable.  We
  // will use this variable to pull out the grid information.  The CF
  // description allows for different fields in the same file to have different
  // grids, but with how the gridded information is used in MET, I'm making the
  // assumption that all fields are on the same grid.

  NcVar *data_var = 0;

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

    NcVarAtt std_name_att = get_nc_att(var, "standard_name");

    if (!IS_INVALID_NC(std_name_att))
    {
      //char *std_name;
      ConcatString std_name;
      get_att_value_chars(&std_name_att, std_name);

      if (std_name == 0 ||
          strcmp(std_name, "latitude") == 0 ||
          strcmp(std_name, "longitude") == 0) continue;
    }

    // If we get here, this should be a gridded data variable

    data_var = var;
    break;

  } /* endfor - i */

  // Pull the grid projection from the variable information.  First, look for
  // a grid_mapping attribute.

  NcVarAtt grid_mapping_att = get_nc_att(data_var, "grid_mapping");

  if (!IS_INVALID_NC(grid_mapping_att))
  {
    get_grid_from_grid_mapping(&grid_mapping_att);
    return;
  }

  // If the grid mapping isn't provided, see if we can intuit a projection
  // from the given dimensions

  if (get_grid_from_dimensions())
    return;

  // If we get here, we couldn't get the grid projection from the file

  mlog << Error << "\nNcCfFile::read_netcdf_grid() -> "
       << "Couldn't figure out projection from information in netCDF file.\n\n";
  exit(1);

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
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot extract grid mapping name from netCDF file.\n\n";
    exit(1);
  }

  NcVar *grid_mapping_var = 0;

  for (int i = 0; i < Nvars; ++i)
  {
    if (strcmp(Var[i].name, mapping_name) == 0)
    {
      grid_mapping_var = Var[i].var;
      break;
    }
  } /* endfor - i */

  if (grid_mapping_var == 0 || IS_INVALID_NC_P(grid_mapping_var))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot extract grid mapping variable (" << mapping_name
         << ") from netCDF file.\n\n";
    exit(1);
  }

  // Get the name of the grid mapping

  NcVarAtt grid_mapping_name_att = get_nc_att(grid_mapping_var, "grid_mapping_name");

  if (IS_INVALID_NC(grid_mapping_name_att))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get coordinate system name from netCDF file.\n\n";
    exit(1);
  }

  //string grid_mapping_name = grid_mapping_name_att->getValues(att->as_string(0);
  ConcatString grid_mapping_name;
  status = get_att_value_chars(&grid_mapping_name_att, grid_mapping_name);

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
  else
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Unknown grid mapping name (" << grid_mapping_name
         << ") found in netCDF file.\n\n";
    exit(1);
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
  static const string method_name = "NcCfFile::get_grid_mapping_lambert_azimuthal_equal_area()";

  mlog << Error << "\n" << method_name << " -> "
       << "Lambert azimuthal equal area grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_lambert_conformal_conic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_lambert_conformal_conic()";
  double x_coord_to_m_cf = 1.0;
  double y_coord_to_m_cf = 1.0;

  // standard_parallel -- there can be 1 or 2 of these

  NcVarAtt std_parallel_att = get_nc_att(
    grid_mapping_var, "standard_parallel");
  if (IS_INVALID_NC(std_parallel_att))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get standard_parallel attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    exit(1);
  }

  // longitude_of_central_meridian

  NcVarAtt central_lon_att = get_nc_att(
    grid_mapping_var, "longitude_of_central_meridian");
  if (IS_INVALID_NC(central_lon_att))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get longitude_of_central_meridian attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    exit(1);
  }

  // latitude_of_projection_origin

  NcVarAtt proj_origin_lat_att = get_nc_att(
    grid_mapping_var, "latitude_of_projection_origin");
  if (IS_INVALID_NC(proj_origin_lat_att))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get latitude_of_projection_origin attribute from "
         << GET_NC_NAME_P(grid_mapping_var) << " variable.\n\n";
    exit(1);
  }

  // Look for the x/y dimensions

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // Get the standard name for the coordinate variable

    const NcVar coord_var = get_var(_ncFile, _dims[dim_num]->getName().c_str());
    if (IS_INVALID_NC(coord_var))
      continue;

    const NcVarAtt std_name_att = get_nc_att(&coord_var, "standard_name");
    if (IS_INVALID_NC(std_name_att))
      continue;

    //const char *dim_std_name = std_name_att->getValues(att->as_string(0);
    ConcatString dim_std_name;
    if (!get_att_value_chars(&std_name_att, dim_std_name))
      continue;
    // See if this is an X or Y dimension

    if (strcmp(dim_std_name, x_dim_key_name) == 0)
    {
      _xDim = _dims[dim_num];

      x_dim_var_name = GET_NC_NAME_P(_xDim).c_str();
      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
        if (strcmp(Var[var_num].name, x_dim_var_name) == 0)
        {
          _xCoordVar = Var[var_num].var;
          break;
        }
      }
    }

    if (strcmp(dim_std_name, y_dim_key_name) == 0)
    {
      _yDim = _dims[dim_num];

      y_dim_var_name = GET_NC_NAME_P(_yDim).c_str();
      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
        if (strcmp(Var[var_num].name, y_dim_var_name) == 0)
        {
          _yCoordVar = Var[var_num].var;
          break;
        }
      }
    }

  }

  if (_xDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find X dimension (projection_x_coordinate) in netCDF file.\n\n";
    exit(1);
  }

  if (_yDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y dimension (projection_y_coordinate) in netCDF file.\n\n";
    exit(1);
  }

  if (_xCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (get_data_size(_xCoordVar) != GET_NC_SIZE_P(_xDim) ||
      get_data_size(_yCoordVar) != GET_NC_SIZE_P(_yDim))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }

  // Make sure that the coordinate variables are given in meters.  If we get
  // files that are in other units, we'll have to update the code to do the
  // units conversions.

  const NcVarAtt x_coord_units_att = get_nc_att(_xCoordVar, "units");
  if (IS_INVALID_NC(x_coord_units_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for X coordinate variable -- assuming meters.\n\n";
  }
  else
  {
    //const char *x_coord_units_name = x_coord_units_att->getValues(att->as_string(0);
    ConcatString x_coord_units_name;
    if (!get_att_value_chars(&x_coord_units_att, x_coord_units_name))
    {
      mlog << Warning << "\n" << method_name << " -> "
           << "Cannot extract X coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if (strcmp(x_coord_units_name, "m" ) == 0) x_coord_to_m_cf = 1.0;
      else if (strcmp(x_coord_units_name, "km") == 0) x_coord_to_m_cf = 1000.0;
      else {
        mlog << Error << "\n" << method_name << " -> "
             << "The X coordinates must be in meters or kilometers for MET.\n\n";
        exit(1);
      }
    }
  }

  const NcVarAtt y_coord_units_att = get_nc_att(_yCoordVar, "units");
  if (IS_INVALID_NC(y_coord_units_att))
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for Y coordinate variable -- assuming meters.\n\n";
  }
  else
  {
    //const char *y_coord_units_name = y_coord_units_att->getValues(att->as_string(0);
    ConcatString y_coord_units_name;
    if (!get_att_value_chars(&y_coord_units_att, y_coord_units_name))
    {
      mlog << Warning << "\n" << method_name << " -> "
           << "Cannot extract Y coordinate units from netCDF file -- "
           << "assuming meters.\n\n";
    }
    else {
           if (strcmp(y_coord_units_name, "m" ) == 0) y_coord_to_m_cf = 1.0;
      else if (strcmp(y_coord_units_name, "km") == 0) y_coord_to_m_cf = 1000.0;
      else {
        mlog << Error << "\n" << method_name << " -> "
             << "The X coordinates must be in meters or kilometers for MET.\n\n";
        exit(1);
      }
    }
  }

  // Figure out the dx/dy  and x/y pin values from the dimension variables

  long x_counts = GET_NC_SIZE_P(_xDim);
  double x_values[x_counts];

  //_xCoordVar->get(x_values, &x_counts);
  get_nc_data(_xCoordVar, x_values);


  long y_counts = GET_NC_SIZE_P(_yDim);
  double y_values[y_counts];

  //_yCoordVar->get(y_values, &y_counts);
  get_nc_data(_yCoordVar, y_values);

  // Unit conversion

  for (int i = 0; i<x_counts; ++i) x_values[i] *= x_coord_to_m_cf;
  for (int i = 0; i<y_counts; ++i) y_values[i] *= y_coord_to_m_cf;

  // Calculate dx and dy assuming they are constant.  MET requires that dx be
  // equal to dy

  double dx_m = (x_values[x_counts-1] - x_values[0]) / (x_counts - 1);
  double dy_m = (y_values[y_counts-1] - y_values[0]) / (y_counts - 1);

  if (fabs(dx_m - dy_m) > DELTA_TOLERANCE)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "MET can only process Lambert Conformal files where the x-axis and y-axis deltas are the same\n\n";
    exit(1);
  }

  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < (int)x_counts; ++i)
  {
    double curr_delta = x_values[i] - x_values[i-1];
    if (fabs(curr_delta - dx_m) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Lambert Conformal files where the delta along the x-axis is constant\n\n";
      exit(1);
    }
  }

  for (int i = 1; i < (int)y_counts; ++i)
  {
    double curr_delta = y_values[i] - y_values[i-1];
    if (fabs(curr_delta - dy_m) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Lambert Conformal files where the delta along the y-axis is constant\n\n";
      exit(1);
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
  //double *double_data = new double[2];
  double double_data;
  double *double_datas;
  data.name = lambert_proj_type;
  double_datas = get_att_value_doubles(&std_parallel_att);
  data.scale_lat_1 = double_datas[0];
  if (std_parallel_att.getAttLength() == 1)
    data.scale_lat_2 = data.scale_lat_1;
  else
    data.scale_lat_2 = double_datas[1];
  double_data = get_att_value_double(&proj_origin_lat_att);
  data.lat_pin = double_data;
  double_datas = get_att_value_doubles(&central_lon_att);
  data.lon_pin = -double_datas[0];
  data.x_pin = x_pin;
  data.y_pin = y_pin;
  data.lon_orient = -double_datas[0];
  data.d_km = dx_m / 1000.0;
  data.r_km = 6371.20;
  data.nx = _xDim->getSize();
  data.ny = _yDim->getSize();

  grid.set(data);
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
    if (IS_INVALID_NC(coord_var))
      continue;

    const NcVarAtt units_att = get_nc_att(&coord_var, "units");
    if (IS_INVALID_NC(units_att))
      continue;

    //const char *dim_units = units_att->getValues(att->as_string(0);
    ConcatString dim_units;
    if (!get_att_value_chars(&units_att, dim_units))
      continue;

    // See if this is a lat or lon dimension

    if (strcmp(dim_units, "degrees_north") == 0 ||
        strcmp(dim_units, "degree_north") == 0 ||
        strcmp(dim_units, "degree_N") == 0 ||
        strcmp(dim_units, "degrees_N") == 0 ||
        strcmp(dim_units, "degreeN") == 0 ||
        strcmp(dim_units, "degreesN") == 0)
    {
      if (_yDim == 0)
      {
        _yDim = _dims[dim_num];

        y_dim_var_name = GET_NC_NAME_P(_yDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, GET_NC_NAME_P(_yDim).c_str()) == 0)
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

    if (strcmp(dim_units, "degrees_east") == 0 ||
        strcmp(dim_units, "degree_east") == 0 ||
        strcmp(dim_units, "degree_E") == 0 ||
        strcmp(dim_units, "degrees_E") == 0 ||
        strcmp(dim_units, "degreeE") == 0 ||
        strcmp(dim_units, "degreesE") == 0)
    {
      if (_xDim == 0)
      {
        _xDim = _dims[dim_num];

        x_dim_var_name = GET_NC_NAME_P(_xDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, GET_NC_NAME_P(_xDim).c_str()) == 0)
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

  if (_xDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find X dimension (degrees_east) in netCDF file.\n\n";
    exit(1);
  }

  if (_yDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y dimension (degrees_north) in netCDF file.\n\n";
    exit(1);
  }

  if (_xCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }

  long lon_counts = _xDim->getSize();
  long lat_counts = _yDim->getSize();
  if (get_data_size(_xCoordVar) != lon_counts ||
      get_data_size(_yCoordVar) != lat_counts)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }

  // Figure out the dlat/dlon values from the dimension variables

  double lat_values[lat_counts];

  //_yCoordVar->get(lat_values, &lat_counts);
  get_nc_data(_yCoordVar, lat_values);


  double lon_values[lon_counts];

  //_xCoordVar->get(lon_values, &lon_counts);
  get_nc_data(_xCoordVar, lon_values);

  // Calculate dlat and dlon assuming they are constant.  MET requires that
  // dlat be equal to dlon

  double dlat = lat_values[1] - lat_values[0];
  double dlon = rescale_lon(lon_values[1] - lon_values[0]);

  if (fabs(dlat - dlon) > DELTA_TOLERANCE)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "MET can only process Latitude/Longitude files where the delta lat and delta lon are the same\n\n";
    exit(1);
  }

  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < lat_counts; ++i)
  {
    double curr_delta = lat_values[i] - lat_values[i-1];
    if (fabs(curr_delta - dlat) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Latitude/Longitude files where the lat delta is constant\n\n";
      exit(1);
    }
  }

  for (int i = 1; i < lon_counts; ++i)
  {
    double curr_delta = rescale_lon(lon_values[i] - lon_values[i-1]);
    if (fabs(curr_delta - dlon) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Latitude/Longitude files where the lon delta is constant\n\n";
      exit(1);
    }
  }

  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  // Note that I am assuming that the data is ordered from the lower-left
  // corner.  I think this will generally be the case, but it is not
  // guaranteed anywhere that I see.  But if this is not the case, then we
  // will probably also need to reorder the data itself.

  LatLonData data;

  data.name = latlon_proj_type;
  data.lat_ll = lat_values[0];
  data.lon_ll = -lon_values[0];
  data.delta_lat = dlat;
  data.delta_lon = dlon;
  data.Nlat = _yDim->getSize();
  data.Nlon = _xDim->getSize();

  grid.set(data);
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


void NcCfFile::get_grid_mapping_polar_stereographic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_polar_stereographic()";

  mlog << Error << "\n" << method_name << " -> "
       << "Polar stereographic grid not handled in MET.\n\n";
  exit(1);
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::get_grid_mapping_rotated_latitude_longitude(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_rotated_latitude_longitude()";

  mlog << Error << "\n" << method_name << " -> "
       << "Rotated latitude longitude grid not handled in MET.\n\n";
  exit(1);
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


bool NcCfFile::get_grid_from_dimensions()
{
  static const string method_name = "NcCfFile::get_grid_from_dimensions()";

  // Currently, we can only intuit a lat/lon grid from the dimensions.
  // Start by looking for the lat/lon dimensions in the file

  NcVar coord_var;
  const NcVarAtt units_att;
  //const char *dim_units;
  ConcatString dim_units;
  string dim_units_str;
  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // The lat/lon dimensions are identified by their units

    coord_var = get_nc_var(_ncFile, _dims[dim_num]->getName().c_str());
    if (IS_INVALID_NC(coord_var))
      continue;

    if (!get_att_value_string(&coord_var, "units", dim_units))
      continue;

    //dim_units = get_att_value_chars(units_att);
    ////dim_units = dim_units_str.c_str();
    //if (dim_units.length() == 0)
    //  continue;

    //dim_units = dim_units_str.c_str();
    // See if this is a lat or lon dimension

    if (strcmp(dim_units, "degrees_north") == 0 ||
        strcmp(dim_units, "degree_north") == 0 ||
        strcmp(dim_units, "degree_N") == 0 ||
        strcmp(dim_units, "degrees_N") == 0 ||
        strcmp(dim_units, "degreeN") == 0 ||
        strcmp(dim_units, "degreesN") == 0)
    {
      if (_yDim == 0)
      {
        _yDim = _dims[dim_num];

        y_dim_var_name = GET_NC_NAME_P(_yDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, GET_NC_NAME_P(_yDim).c_str()) == 0)
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

    if (strcmp(dim_units, "degrees_east") == 0 ||
        strcmp(dim_units, "degree_east") == 0 ||
        strcmp(dim_units, "degree_E") == 0 ||
        strcmp(dim_units, "degrees_E") == 0 ||
        strcmp(dim_units, "degreeE") == 0 ||
        strcmp(dim_units, "degreesE") == 0)
    {
      if (_xDim == 0)
      {
        _xDim = _dims[dim_num];

        x_dim_var_name = GET_NC_NAME_P(_xDim).c_str();
        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, GET_NC_NAME_P(_xDim).c_str()) == 0)
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

  if (_xDim == 0 || _yDim == 0)
    return false;

  if (_xCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find X coord variable (" << GET_NC_NAME_P(_xDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << GET_NC_NAME_P(_yDim)
         << ") in netCDF file.\n\n";
    exit(1);
  }

  long lat_counts = GET_NC_SIZE_P(_yDim);
  long lon_counts = GET_NC_SIZE_P(_xDim);
  if (get_data_size(_xCoordVar) != lon_counts ||
      get_data_size(_yCoordVar) != lat_counts)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }

  // Figure out the dlat/dlon values from the dimension variables

  double lat_values[lat_counts];

  //_yCoordVar->get(lat_values, &lat_counts);
  get_nc_data(_yCoordVar,lat_values);

  double lon_values[lon_counts];

  //_xCoordVar->get(lon_values, &lon_counts);
  get_nc_data(_xCoordVar,lon_values);

  // Calculate dlat and dlon assuming they are constant.  MET requires that
  // dlat be equal to dlon

  double dlat = lat_values[1] - lat_values[0];
  double dlon = rescale_lon(lon_values[1] - lon_values[0]);

  if (fabs(dlat - dlon) > DELTA_TOLERANCE)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "MET can only process Latitude/Longitude files where the delta lat and delta lon are the same\n\n";
    exit(1);
  }

  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < (int)lat_counts; ++i)
  {
    double curr_delta = lat_values[i] - lat_values[i-1];
    if (fabs(curr_delta - dlat) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Latitude/Longitude files where the lat delta is constant\n\n";
      exit(1);
    }
  }

  for (int i = 1; i < (int)lon_counts; ++i)
  {
    double curr_delta = rescale_lon(lon_values[i] - lon_values[i-1]);
    if (fabs(curr_delta - dlon) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Latitude/Longitude files where the lon delta is constant\n\n";
      exit(1);
    }
  }

  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  // Note that I am assuming that the data is ordered from the lower-left
  // corner.  I think this will generally be the case, but it is not
  // guaranteed anywhere that I see.  But if this is not the case, then we
  // will probably also need to reorder the data itself.

  LatLonData data;

  data.name = latlon_proj_type;
  data.lat_ll = lat_values[0];
  data.lon_ll = -lon_values[0];
  data.delta_lat = dlat;
  data.delta_lon = dlon;
  data.Nlat = _yDim->getSize();
  data.Nlon = _xDim->getSize();

  grid.set(data);

  return true;
}


////////////////////////////////////////////////////////////////////////


void parse_cf_time_string(const char *str, unixtime &ref_ut, int &sec_per_unit) {

   // Initialize
   ref_ut = sec_per_unit = 0;

   // Check for expected time string format:
   //   [seconds|minutes|hours|days] since YYYY-MM-DD HH:MM:SS
   if(!check_reg_exp("^[a-z|A-Z]* since [0-9]\\{4\\}" , str)) {
      mlog << Warning << "\nparse_cf_time_string() -> "
           << "unexpected NetCDF CF convention time unit \""
           << str << "\"\n\n";
      return;
   }
   else {
      // Tokenize the input string
      // Parse using spaces or 'T' for timestrings such as:
      //   minutes since 2016-01-28T12:00:00Z
      //   seconds since 1977-08-07 12:00:00Z
      StringArray tok;
      tok.parse_delim(str, " T");
      tok.set_ignore_case(true);

      // Determine the time step
           if(tok.has("seconds")) sec_per_unit = 1;
      else if(tok.has("minutes")) sec_per_unit = 60;
      else if(tok.has("hours"))   sec_per_unit = 3600;
      else if(tok.has("days"))    sec_per_unit = 86400;
      else {
         mlog << Warning << "\nparse_cf_time_string() -> "
              << "Unsupported time step in the CF convention time unit \""
              << str << "\"\n\n";
         return;
      }

      // Parse the reference time
      StringArray ymd, hms;
      ymd.parse_delim(tok[2], "-");
      if(tok.n_elements() > 3) hms.parse_delim(tok[3], ":");
      else                     hms.parse_delim("00:00:00", ":");
      ref_ut = mdyhms_to_unix(atoi(ymd[1]), atoi(ymd[2]), atoi(ymd[0]),
                              atoi(hms[0]),
                              hms.n_elements() > 1 ? atoi(hms[1]) : 0,
                              hms.n_elements() > 2 ? atoi(hms[2]) : 0);
   }

   mlog << Debug(4) << "parse_cf_time_string() -> "
        << "parsed NetCDF CF convention time unit string \"" << str
        << "\" as a reference time of " << unix_to_yyyymmdd_hhmmss(ref_ut)
        << " and " << sec_per_unit << " second(s) per time step.\n";

   return;
}


////////////////////////////////////////////////////////////////////////

