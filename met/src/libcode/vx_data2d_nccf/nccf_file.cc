

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

  NcError err(NcError::silent_nonfatal);

  // Open the file

  _ncFile = new NcFile(filepath);

  if (!(_ncFile->is_valid()))
  {
    close();
    return false;
  }

  // Pull out the dimensions

  _numDims = _ncFile->num_dims();

  _dims = new NcDim*[_numDims];

  _dimNames.extend(_numDims);

  for (int j = 0; j < _numDims; ++j)
  {
    _dims[j] = _ncFile->get_dim(j);

    _dimNames.add(_dims[j]->name());
  }

  // Pull out the valid and init times

  NcVar *valid_time_var = _ncFile->get_var("time");
  if (valid_time_var == 0)
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
    _tDim = valid_time_var->get_dim(0);

    // Parse the units for the time variable.
    NcAtt *units_att = valid_time_var->get_att("units");
    if (units_att != 0)
    {
      char *units = units_att->as_string(0);

      if (units == 0)
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
    int n_times = (int) valid_time_var->num_vals();

    for(int i=0; i<n_times; i++)
    {
      double time_value;
      valid_time_var->set_cur((long int) i);
      valid_time_var->get(&time_value, 1);
      ValidTime.add((unixtime)ut + sec_per_unit * time_value);
    }
  }

  NcVar *init_time_var = _ncFile->get_var("forecast_reference_time");
  if (init_time_var == 0)
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
    NcAtt *units_att = init_time_var->get_att("units");
    if (units_att != 0)
    {
      char *units = units_att->as_string(0);

      if (units == 0)
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

    double time_value;
    init_time_var->set_cur((long int)0);
    init_time_var->get(&time_value, 1);
    InitTime = (unixtime)ut + sec_per_unit * time_value;
  }

  // Pull out the variables

  Nvars = _ncFile->num_vars();

  Var = new NcVarInfo [Nvars];

  for (int j = 0; j < Nvars; ++j)
  {
    NcVar *v = _ncFile->get_var(j);

    Var[j].var = v;

    Var[j].name = v->name();

    Var[j].Ndims = v->num_dims();

    Var[j].Dims = new NcDim * [Var[j].Ndims];

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

  for (int j = 0; j < Nvars; ++j)
  {
    NcVar *v = _ncFile->get_var(j);

    for (int k = 0; k < Var[j].Ndims; ++k)
    {
      Var[j].Dims[k] = v->get_dim(k);

      if (Var[j].Dims[k] == _xDim)
         Var[j].x_slot = k;
      if (Var[j].Dims[k] == _yDim)
         Var[j].y_slot = k;
      if (Var[j].Dims[k] == _tDim)
         Var[j].t_slot = k;
    }   //  for k

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
        << (_dims[j]->size()) << ")\n";
  }   //  for j

  out << prefix << "\n";

  out << prefix << "Xdim = " << (_xDim ? _xDim->name() : "(nul)") << "\n";
  out << prefix << "Ydim = " << (_yDim ? _yDim->name() : "(nul)") << "\n";
  out << prefix << "Tdim = " << (_tDim ? _tDim->name() : "(nul)") << "\n";

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
        out << Var[j].Dims[k]->name();

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

  if (var->num_dims() != a.n_elements())
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "needed " << (var->num_dims()) << " arguments for variable "
         << (var->name()) << ", got " << (a.n_elements()) << "\n\n";
    exit(1);
  }

  if (var->num_dims() >= max_met_args)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "too may arguments for variable \"" << (var->name()) << "\"\n\n";
    exit(1);
  }

  long counts[max_met_args];

  for (int j = 0; j < (a.n_elements()); ++j)
    counts[j] = 1;

  if (!(var->set_cur((long *)a)))
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "can't set corner for variable \"" << (var->name()) << "\"\n\n";
    exit(1);
  }

  bool status;
  double d[2];

  switch (var->type())
  {
  case ncInt:
  {
    int i[2];

    status = var->get(i, counts);
    d[0] = (double) (i[0]);
    break;
  }

  case ncFloat:
  {
    float f[2];

    status = var->get(f, counts);
    d[0] = (double) (f[0]);
    break;
  }

  case ncDouble:
  {
    status = var->get(d, counts);
    break;
  }

  default:
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
         << "bad type for variable \"" << (var->name()) << "\"\n\n";
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

  return d[0];
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

  if (v->num_dims() != a.n_elements())
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) -> "
         << "needed " << (v->num_dims()) << " arguments for variable "
         << (v->name()) << ", got " << (a.n_elements()) << "\n\n";
    exit(1);
  }

  if (v->num_dims() >= max_met_args)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) -> "
         << "too may arguments for variable \"" << (v->name()) << "\"\n\n";
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
         << "variable " << (v->name()) << " not found!\n\n";
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

    int num_dims = var->num_dims();

    if (num_dims < 2 || num_dims > 4)
      continue;

    // Skip the latitude and longitude variables, if they are present

    NcAtt *std_name_att = var->get_att("standard_name");

    if (std_name_att != 0)
    {
      char *std_name = std_name_att->as_string(0);

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

  NcAtt *grid_mapping_att = data_var->get_att("grid_mapping");

  if (grid_mapping_att != 0)
  {
    get_grid_from_grid_mapping(grid_mapping_att);
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


void NcCfFile::get_grid_from_grid_mapping(const NcAtt *grid_mapping_att)
{
  static const string method_name = "NcCfFile::get_grid_from_grid_mapping()";

  // The grid_mapping attribute gives the name of the variable that
  // contains the grid mapping information.  Find that variable.

  char *mapping_name = grid_mapping_att->as_string(0);
  if (mapping_name == 0)
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

  if (grid_mapping_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot extract grid mapping variable (" << mapping_name
         << ") from netCDF file.\n\n";
    exit(1);
  }

  // Get the name of the grid mapping

  NcAtt *grid_mapping_name_att = grid_mapping_var->get_att("grid_mapping_name");

  if (grid_mapping_name_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get coordinate system name from netCDF file.\n\n";
    exit(1);
  }

  string grid_mapping_name = grid_mapping_name_att->as_string(0);

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

  NcAtt *std_parallel_att = grid_mapping_var->get_att("standard_parallel");
  if (std_parallel_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get standard_parallel attribute from "
         << grid_mapping_var->name() << " variable.\n\n";
    exit(1);
  }

  // longitude_of_central_meridian

  NcAtt *central_lon_att =
    grid_mapping_var->get_att("longitude_of_central_meridian");
  if (central_lon_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get longitude_of_central_meridian attribute from "
         << grid_mapping_var->name() << " variable.\n\n";
    exit(1);
  }

  // latitude_of_projection_origin

  NcAtt *proj_origin_lat_att =
    grid_mapping_var->get_att("latitude_of_projection_origin");
  if (proj_origin_lat_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Cannot get latitude_of_projection_origin attribute from "
         << grid_mapping_var->name() << " variable.\n\n";
    exit(1);
  }

  // Look for the x/y dimensions

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // Get the standard name for the coordinate variable

    const NcVar *coord_var = _ncFile->get_var(_dims[dim_num]->name());
    if (coord_var == 0)
      continue;

    const NcAtt *std_name_att = coord_var->get_att("standard_name");
    if (std_name_att == 0)
      continue;

    const char *dim_std_name = std_name_att->as_string(0);
    if (dim_std_name == 0)
      continue;
    // See if this is an X or Y dimension

    if (strcmp(dim_std_name, "projection_x_coordinate") == 0)
    {
      _xDim = _dims[dim_num];

      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
        if (strcmp(Var[var_num].name, _xDim->name()) == 0)
        {
          _xCoordVar = Var[var_num].var;
          break;
        }
      }
    }

    if (strcmp(dim_std_name, "projection_y_coordinate") == 0)
    {
      _yDim = _dims[dim_num];

      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
        if (strcmp(Var[var_num].name, _yDim->name()) == 0)
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
         << "Didn't find X coord variable (" << _xDim->name()
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << _yDim->name()
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_xCoordVar->num_vals() != _xDim->size() ||
      _yCoordVar->num_vals() != _yDim->size())
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }

  // Make sure that the coordinate variables are given in meters.  If we get
  // files that are in other units, we'll have to update the code to do the
  // units conversions.

  const NcAtt *x_coord_units_att = _xCoordVar->get_att("units");
  if (x_coord_units_att == 0)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for X coordinate variable -- assuming meters.\n\n";
  }
  else
  {
    const char *x_coord_units_name = x_coord_units_att->as_string(0);
    if (x_coord_units_name == 0)
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

  const NcAtt *y_coord_units_att = _yCoordVar->get_att("units");
  if (y_coord_units_att == 0)
  {
    mlog << Warning << "\n" << method_name << " -> "
         << "Units not given for Y coordinate variable -- assuming meters.\n\n";
  }
  else
  {
    const char *y_coord_units_name = y_coord_units_att->as_string(0);
    if (y_coord_units_name == 0)
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

  double x_values[_xDim->size()];
  long x_counts = _xDim->size();

  _xCoordVar->get(x_values, &x_counts);

  double y_values[_yDim->size()];
  long y_counts = _yDim->size();

  _yCoordVar->get(y_values, &y_counts);

  // Unit conversion

  for (int i = 0; i<x_counts; ++i) x_values[i] *= x_coord_to_m_cf;
  for (int i = 0; i<y_counts; ++i) y_values[i] *= y_coord_to_m_cf;

  // Calculate dx and dy assuming they are constant.  MET requires that dx be
  // equal to dy

  double dx_m = (x_values[_xDim->size()-1] - x_values[0]) / (_xDim->size() - 1);
  double dy_m = (y_values[_yDim->size()-1] - y_values[0]) / (_yDim->size() - 1);

  if (fabs(dx_m - dy_m) > DELTA_TOLERANCE)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "MET can only process Lambert Conformal files where the x-axis and y-axis deltas are the same\n\n";
    exit(1);
  }

  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < _xDim->size(); ++i)
  {
    double curr_delta = x_values[i] - x_values[i-1];
    if (fabs(curr_delta - dx_m) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Lambert Conformal files where the delta along the x-axis is constant\n\n";
      exit(1);
    }
  }

  for (int i = 1; i < _yDim->size(); ++i)
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
  data.name = lambert_proj_type;
  data.scale_lat_1 = std_parallel_att->as_double(0);
  if (std_parallel_att->num_vals() == 1)
    data.scale_lat_2 = data.scale_lat_1;
  else
    data.scale_lat_2 = std_parallel_att->as_double(1);
  data.lat_pin = proj_origin_lat_att->as_double(0);
  data.lon_pin = -central_lon_att->as_double(0);
  data.x_pin = x_pin;
  data.y_pin = y_pin;
  data.lon_orient = -central_lon_att->as_double(0);
  data.d_km = dx_m / 1000.0;
  data.r_km = 6371.20;
  data.nx = _xDim->size();
  data.ny = _yDim->size();

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

    const NcVar *coord_var = _ncFile->get_var(_dims[dim_num]->name());
    if (coord_var == 0)
      continue;

    const NcAtt *units_att = coord_var->get_att("units");
    if (units_att == 0)
      continue;

    const char *dim_units = units_att->as_string(0);
    if (dim_units == 0)
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

        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, _yDim->name()) == 0)
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
             << _yCoordVar->name() << "\".\n\n";
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

        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, _xDim->name()) == 0)
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
             << _xCoordVar->name() << "\".\n\n";
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
         << "Didn't find X coord variable (" << _xDim->name()
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << _yDim->name()
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_xCoordVar->num_vals() != _xDim->size() ||
      _yCoordVar->num_vals() != _yDim->size())
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }

  // Figure out the dlat/dlon values from the dimension variables

  double lat_values[_yDim->size()];
  long lat_counts = _yDim->size();

  _yCoordVar->get(lat_values, &lat_counts);

  double lon_values[_xDim->size()];
  long lon_counts = _xDim->size();

  _xCoordVar->get(lon_values, &lon_counts);

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

  for (int i = 1; i < _yDim->size(); ++i)
  {
    double curr_delta = lat_values[i] - lat_values[i-1];
    if (fabs(curr_delta - dlat) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Latitude/Longitude files where the lat delta is constant\n\n";
      exit(1);
    }
  }

  for (int i = 1; i < _xDim->size(); ++i)
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
  data.Nlat = _yDim->size();
  data.Nlon = _xDim->size();

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

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // The lat/lon dimensions are identified by their units

    const NcVar *coord_var = _ncFile->get_var(_dims[dim_num]->name());
    if (coord_var == 0)
      continue;

    const NcAtt *units_att = coord_var->get_att("units");
    if (units_att == 0)
      continue;

    const char *dim_units = units_att->as_string(0);
    if (dim_units == 0)
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

        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, _yDim->name()) == 0)
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
             << _yCoordVar->name() << "\".\n\n";
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

        for (int var_num = 0; var_num < Nvars; ++var_num)
        {
          if (strcmp(Var[var_num].name, _xDim->name()) == 0)
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
             << _xCoordVar->name() << "\".\n\n";
      }
    }

  }

  if (_xDim == 0 || _yDim == 0)
    return false;

  if (_xCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find X coord variable (" << _xDim->name()
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Didn't find Y coord variable (" << _yDim->name()
         << ") in netCDF file.\n\n";
    exit(1);
  }

  if (_xCoordVar->num_vals() != _xDim->size() ||
      _yCoordVar->num_vals() != _yDim->size())
  {
    mlog << Error << "\n" << method_name << " -> "
         << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }

  // Figure out the dlat/dlon values from the dimension variables

  double lat_values[_yDim->size()];
  long lat_counts = _yDim->size();

  _yCoordVar->get(lat_values, &lat_counts);

  double lon_values[_xDim->size()];
  long lon_counts = _xDim->size();

  _xCoordVar->get(lon_values, &lon_counts);

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

  for (int i = 1; i < _yDim->size(); ++i)
  {
    double curr_delta = lat_values[i] - lat_values[i-1];
    if (fabs(curr_delta - dlat) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "MET can only process Latitude/Longitude files where the lat delta is constant\n\n";
      exit(1);
    }
  }

  for (int i = 1; i < _xDim->size(); ++i)
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
  data.Nlat = _yDim->size();
  data.Nlon = _xDim->size();

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
      StringArray tok;
      tok.parse_wsss(str);
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
                              atoi(hms[0]), atoi(hms[1]), atoi(hms[2]));
   }

   mlog << Debug(4) << "parse_cf_time_string() -> "
        << "parsed NetCDF CF convention time unit string \"" << str
        << "\" as a reference time of " << unix_to_yyyymmdd_hhmmss(ref_ut)
        << " and " << sec_per_unit << " second(s) per time step.\n";

   return;
}


////////////////////////////////////////////////////////////////////////

