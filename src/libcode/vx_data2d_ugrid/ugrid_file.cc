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
#include "config_util.h"

#include "ugrid_file.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////

static const char *def_user_config = "UGridConfig_user";
static const char *def_config_prefix = "UGridConfig_";
static const char *def_config_prefix2 = "MET_BASE/config/UGridConfig_";

array<string, UG_DIM_COUNT> DIM_KEYS = {
      "dim_face", "dim_node", "dim_edge", "dim_time", "dim_vert"
};

array<string, UG_META_VAR_COUNT> COORD_VAR_KEYS = {
      "time", "lat_face", "lon_face", "vert_face", "lat_edge",
      "lon_edge", "lat_node", "lon_node", "cell_id"
};

static double get_nc_var_att_double(const NcVar *nc_var, const char *att_name,
                                    bool is_required=true);

#define USE_BUFFER  1

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class UGridFile
   //


////////////////////////////////////////////////////////////////////////


UGridFile::UGridFile()
{
  init_from_scratch();
}


////////////////////////////////////////////////////////////////////////

UGridFile::~UGridFile()
{
  close();
}


////////////////////////////////////////////////////////////////////////


void UGridFile::init_from_scratch()

{
  // Initialize the pointers

  _ncFile = (NcFile *) nullptr;
  _ncMetaFile = (NcFile *) nullptr;
  Var = (NcVarInfo *) nullptr;
  _time_var_info = (NcVarInfo *)nullptr;

  _faceDim = (NcDim *)nullptr;
  _edgeDim = (NcDim *)nullptr;
  _nodeDim = (NcDim *)nullptr;
  _virtDim = (NcDim *)nullptr;
  _tDim = (NcDim *)nullptr;
  _latVar = (NcVar *)nullptr;
  _lonVar = (NcVar *)nullptr;
  metadata_map.clear();
  max_distance_km = bad_data_double;

  // Close any existing file

  close();

  return;
}


////////////////////////////////////////////////////////////////////////


void UGridFile::close()
{

  // Reclaim the file pointer

  if (_ncFile) {
    delete _ncFile;
    _ncFile = (NcFile *)nullptr;
  }

  if (_ncMetaFile) {
    delete _ncMetaFile;
    _ncMetaFile = (NcFile *)nullptr;
  }

  // Reclaim the dimension pointers

  _numDims = 0;

  _dimNames.clear();
  metadata_map.clear();
  metadata_names.clear();

  _faceDim = _edgeDim = _tDim = (NcDim *)nullptr;

  // Reclaim the variable pointers

  if (Var) {
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

  face_count = 0;

  //  done

  return;
}


////////////////////////////////////////////////////////////////////////


bool UGridFile::open(const char * filepath)
{
  const char *method_name = "UGridFile::open() -> ";

  // Close any open files and clear out the associated members
  close();
  if (_ncFile) {
    delete _ncFile;
    _ncFile = (NcFile *) nullptr;
  }

  // Open the file
  _ncFile = open_ncfile(filepath);

  if (IS_INVALID_NC_P(_ncFile))
  {
    close();
    return false;
  }

  return get_var_info();
}


////////////////////////////////////////////////////////////////////////


bool UGridFile::open_metadata(const char * filepath)
{
  unixtime ut = 0;
  const char *method_name = "UGridFile::open_metadata() -> ";

  // Open the file
  _ncMetaFile = open_ncfile(filepath);

  if (IS_INVALID_NC_P(_ncMetaFile)) {
    close();
    return false;
  }

  NcDim dim;
  string meta_name;
  string time_dim_name;
  string vert_dim_name;
  StringArray dim_names;
  StringArray meta_names;

  get_dim_names(_ncMetaFile, &dim_names);

  // Face (cell) dimension
  meta_name = find_metadata_name(DIM_KEYS[0], dim_names);
  if (0 < meta_name.length()) {
    dim = get_nc_dim(_ncMetaFile, meta_name.c_str());
    face_count = get_dim_size(&dim);
    _faceDim = new NcDim(dim);
    NcDim face_dim = get_nc_dim(_ncFile, meta_name.c_str());
    int data_face_count = get_dim_size(&face_dim);
    if (face_count != data_face_count) {
      mlog << Error << "\n" << method_name
           << meta_name << " dimension is different: data file = "
           << data_face_count << ", coordinates file = " << face_count << "\n\n";
      exit(1);
    }
  }

  // Node (vertex) dimension
  meta_name = find_metadata_name(DIM_KEYS[1], dim_names);
  if (0 < meta_name.length()) {
    dim = get_nc_dim(_ncMetaFile, meta_name.c_str());
    _nodeDim = new NcDim(dim);
  }

  // Edge dimension
  meta_name = find_metadata_name(DIM_KEYS[2], dim_names);
  if (0 < meta_name.length()) {
    dim = get_nc_dim(_ncMetaFile, meta_name.c_str());
    _edgeDim = new NcDim(dim);
  }

  // Time dimension
  time_dim_name = find_metadata_name(DIM_KEYS[3], dim_names);
  if (0 < time_dim_name.length()) {
    dim = get_nc_dim(_ncMetaFile, time_dim_name.c_str());
    _tDim = new NcDim(dim);
  }

  // Vertical dimension
  vert_dim_name = find_metadata_name(DIM_KEYS[4], dim_names);
  if (0 < vert_dim_name.length()) {
    dim = get_nc_dim(_ncMetaFile, vert_dim_name.c_str());
    _virtDim = new NcDim(dim);
  }

  int max_dim_count = 0;
  StringArray var_names;
  ConcatString att_value;
  NcVar *z_var = (NcVar *)nullptr;
  NcVar *valid_time_var = (NcVar *)nullptr;

  StringArray time_names = get_metadata_names(COORD_VAR_KEYS[0]);
  StringArray lat_names = get_metadata_names(COORD_VAR_KEYS[1]);
  StringArray lon_names = get_metadata_names(COORD_VAR_KEYS[2]);
  StringArray z_names = get_metadata_names(COORD_VAR_KEYS[3]);
  for (int j=0; j<Nvars; ++j) {
    if (time_names.has(Var[j].name)) {
      valid_time_var = Var[j].var;
      _time_var_info = &Var[j];
    }
    else if (lat_names.has(Var[j].name)) _latVar = Var[j].var;
    else if (lon_names.has(Var[j].name)) _lonVar = Var[j].var;
    else if (z_names.has(Var[j].name)) z_var = Var[j].var;
  }

  get_var_names(_ncMetaFile, &var_names);
  for (int j=0; j<COORD_VAR_KEYS.size(); j++) {
    meta_name = find_metadata_name(COORD_VAR_KEYS[j], var_names);
    if (0 < meta_name.length()) {
      NcVar v = get_var(_ncMetaFile, meta_name.c_str());

      MetaVar[j].var = new NcVar(v);

      MetaVar[j].name = GET_NC_NAME(v).c_str();

      int dim_count = GET_NC_DIM_COUNT(v);
      MetaVar[j].Ndims = dim_count;
      MetaVar[j].Dims = new NcDim * [dim_count];
      if (dim_count > max_dim_count) max_dim_count = dim_count;

      //  parse the variable attributes
      get_att_str( MetaVar[j], long_name_att_name, MetaVar[j].long_name_att );
      get_att_str( MetaVar[j], units_att_name,     MetaVar[j].units_att     );

      if (0 == j && nullptr == _time_var_info) {
        valid_time_var = MetaVar[j].var;
        _time_var_info = &MetaVar[j];
      }
      else if (1 == j && nullptr == _latVar) _latVar = MetaVar[j].var;
      else if (2 == j && nullptr == _lonVar) _lonVar = MetaVar[j].var;
      else if (3 == j && nullptr == _latVar) z_var = MetaVar[j].var;
    }

  }   //  for j


  // Pull out the valid and init times
  if (IS_INVALID_NC_P(valid_time_var)) {
    mlog << Debug(4) << method_name
         << "could not extract valid time from the "
         << "time variable from " << filepath << "\n";

    ValidTime.add(ut);
  }
  else {
    int sec_per_unit;

    // Store the dimension for the time variable as the time dimension
    ConcatString units;
    bool use_bounds_var = false;
    int time_dim_count = get_dim_count(valid_time_var);
    if (time_dim_count == 1 || time_dim_count == 2) {
       NcDim tDim = get_nc_dim(valid_time_var, 0);
       if (IS_VALID_NC(tDim)) {
         _tDim = new NcDim(tDim);
       }
    }

    // Parse the units for the time variable.
    ut = sec_per_unit = 0;
    if (get_var_units(valid_time_var, units) && (time_dim_count < 2)) {
      if (units.length() == 0) {
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

    // Determine the number of times present.
    int n_times = IS_VALID_NC_P(_tDim) ? get_dim_size(_tDim)
                                       : (int) get_data_size(valid_time_var);
    int tim_buf_size = n_times;
    double *time_values = new double[tim_buf_size];
    if(2 == time_dim_count) {
      for(int i=0; i<n_times; i++) {
        time_values[i] = get_nc_time(valid_time_var, i);
        ValidTime.add(time_values[i]);
        raw_times.add(time_values[i]);
        mlog << Debug(7) << method_name
             << "get time " << time_values[i] << " ("
             << unix_to_yyyymmdd_hhmmss(time_values[i]) << ") from "
             << GET_NC_NAME_P(valid_time_var) << "\n";
      }
    }
    else if( get_nc_data(valid_time_var, time_values) ) {
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
    delete [] time_values;
  }

  // Pull out the grid.  This must be done after pulling out the dimension
  // and variable information since this information is used to pull out the
  // grid.  This call sets the _faceDim and _edgeDim pointers.

  read_netcdf_grid();

  // Now go back through the variables and use _faceDim, _edgeDim, and _tDim
  // to set the slots.
  // Should be called after read_netcdf_grid() is called

  StringArray dimNames;
  for (int j=0; j<Nvars; ++j) {

    int dim_count = Var[j].Ndims;
    NcVar *v = Var[j].var;

    dimNames.clear();
    get_dim_names(v, &dimNames);

    for (int k=0; k<dim_count; ++k)  {
      NcDim *dim_p = Var[j].Dims[k];
      const ConcatString dim_name = dimNames[k];
      if ((nullptr != dim_p && dim_p == _tDim) || dim_name == time_dim_name) {
         Var[j].t_slot = k;
      }
      else if (dim_name == vert_dim_name) {
         Var[j].z_slot = k;
      }
    }
  }   //  for j

  // Find the vertical level variable from dimension name if not found
  if (IS_INVALID_NC_P(z_var) && (0 < vert_dim_name.length())) {
    NcVarInfo *info = find_var_by_dim_name(vert_dim_name.c_str());
    if (info) z_var = info->var;
  }

  // Pull out the vertical levels
  if (IS_VALID_NC_P(z_var)) {

    int z_count = (int) get_data_size(z_var);
    double *z_values = new double[z_count];

    if( get_nc_data(z_var, z_values) ) {
      for(int i=0; i<z_count; i++) {
        vlevels.add(z_values[i]);
      }
    }
    delete [] z_values;
  }

  //  done
  return true;
}


////////////////////////////////////////////////////////////////////////


void UGridFile::dump(ostream & out, int depth) const
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

  out << prefix << "face_dim = " << (_faceDim ? GET_NC_NAME_P(_faceDim) : "(nul)") << "\n";
  out << prefix << "edge_dim = " << (_edgeDim ? GET_NC_NAME_P(_edgeDim) : "(nul)") << "\n";
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
      if (Var[j].Dims[k] == _faceDim)
        out << 'X';
      else if (Var[j].Dims[k] == _edgeDim)
        out << 'Y';
      else if (Var[j].Dims[k] == _tDim)
        out << 'T';
      else
        out << GET_NC_NAME_P(Var[j].Dims[k]);

      if (k < Var[j].Ndims - 1)
        out << ", ";
    }   //  for k

    out << ")\n";

    out << p2 << "\n";

  }   //  for j

  //  done

  out.flush();

  return;
}


////////////////////////////////////////////////////////////////////////

std::string UGridFile::find_metadata_name(std::string &key, StringArray &available_names) {
  string meta_name = "";
  StringArray meta_names = get_metadata_names(key);

  for (int idx=0; idx<meta_names.n(); idx++) {
    if (available_names.has(meta_names[idx])) {
      meta_name = meta_names[idx];
      break;
    }
  }
  return meta_name;
}

////////////////////////////////////////////////////////////////////////


NcVarInfo* UGridFile::find_by_name(const char * var_name) const
{
  for (int i = 0; i < Nvars; i++)
  {
    if (Var[i].name == var_name)
      return &Var[i];
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////


NcVarInfo* UGridFile::find_var_by_dim_name(const char *dim_name) const
{
  NcVarInfo *var = find_by_name(dim_name);
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

bool UGridFile::find_nc_vinfo_list(const char *var_name,
                                   std::vector<NcVarInfo *> &vinfo_list) const
{
  vinfo_list.clear();
  for (int i = 0; i < Nvars; i++) {
    if (Var[i].name.startswith(var_name)) vinfo_list.push_back(&Var[i]);
  }
  return vinfo_list.size() > 0;
}


////////////////////////////////////////////////////////////////////////


double UGridFile::getData(NcVar * var, const LongArray & a) const
{
  clock_t start_clock = clock();
  static const string method_name
      = "UGridFile::getData(NcVar *, const LongArray &) -> ";

  bool status = false;
  double d = bad_data_double;

  double fill_value;
  get_var_fill_value(var, fill_value);

  //status = get_nc_data(var, &d, a);
  status = get_nc_data(var, a);

  if (!status)
  {
    mlog << Error << "\nUGridFile::getData(NcVar *, const LongArray &) const -> "
         << "bad status for var->get()\n\n";
    exit(1);
  }

  //  done

  mlog << Debug(6) << method_name << "took "
       << (clock()-start_clock)/double(CLOCKS_PER_SEC) << " seconds\n";

  return d;
}


////////////////////////////////////////////////////////////////////////


bool UGridFile::getData(NcVar * v, const LongArray & a, DataPlane & plane) const
{
  clock_t start_clock = clock();
  static const string method_name_short
      = "UGridFile::getData(NcVar*, LongArray&, DataPlane&) ";
  static const string method_name
      = "UGridFile::getData(NcVar *, const LongArray &, DataPlane &) -> ";

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
         << "needed " << (dim_count) << " arguments for variable "
         << (GET_NC_NAME_P(v)) << ", got " << (a.n_elements()) << "\n\n";
    exit(1);
  }

  //  find varinfo's

  NcVarInfo *var = find_by_name(GET_NC_NAME_P(v).c_str());

  if (nullptr == var) {
    mlog << Error << "\n" << method_name
         << "variable " << (GET_NC_NAME_P(v)) << " not found!\n\n";
    exit(1);
  }

  //  check star positions and count

  //
  //  get the bad data values
  //

  double fill_value;
  double missing_value = get_var_missing_value(v);
  get_var_fill_value(v, fill_value);

  //  set up the DataPlane object

  const int nx = grid.nx();
  const int ny = grid.ny();

  plane.clear();
  plane.set_size(nx, ny);

  //  get the data
  const int plane_size = nx * ny;
  double *d = new double[plane_size];

  int length;
  size_t dim_size;
  LongArray offsets;
  LongArray lengths;
  for (int k=0; k<dim_count; k++) {
    length = 1;
    if (a[k] == vx_data2d_star) {
      offsets.add(0);
      length = plane_size;
    }
    else {
      offsets.add(a[k]);
      if (k != var->t_slot && k != var->z_slot) length = plane_size - a[k];
    }
    lengths.add(length);
    dim_size = v->getDim(k).getSize();
    if (dim_size < offsets[k]) {
      mlog << Error << "\n" << method_name
           << "offset (" << offsets[k] << ") at " << k
           << "th dimension (" << long(dim_size) << ") is too big for variable \""
           << GET_NC_NAME_P(v) << "\"\n\n";
      exit ( 1 );
    }
  }

  get_nc_data(v, d, lengths, offsets);

  double min_value = 10e10;
  double max_value = -min_value;
  for (int x = 0; x< nx; ++x) {
    double value = d[x];
    if( is_eq(value, missing_value) || is_eq(value, fill_value) ) {
      value = bad_data_double;
    }
    else {
      if (min_value > value) min_value = value;
      if (max_value < value) max_value = value;
    }

    plane.set(value, x, 0);

  }   //  for x

  if (nullptr != d) delete [] d;

  //  done
  ConcatString log_message;
  for (int idx=0; idx<a.n_elements(); idx++) {
    log_message << " " << (a[idx] == vx_data2d_star ? "*" : std::to_string(a[idx]));
  }
  mlog << Debug(6) << method_name << "took "
       << (clock()-start_clock)/double(CLOCKS_PER_SEC) << " seconds. "
       << GET_NC_NAME_P(v) << ": levels: (" << log_message << " )"
       << " min=" << min_value << ", max_value=" << max_value<< "\n";

  return true;
}


////////////////////////////////////////////////////////////////////////


bool UGridFile::getData(const char *var_name,
                        const LongArray &a, DataPlane &plane,
                        NcVarInfo *&info) const
{
  info = find_by_name(var_name);

  bool found = getData(info->var, a, plane);

  //  store the times
  unixtime valid_ut;
  if(info->t_slot >= 0) valid_ut = ValidTime[a[info->t_slot]];
  else                  valid_ut = ValidTime[0];

  //  if unset, set the init time to the valid time
  unixtime init_ut = InitTime;
  if(init_ut == 0 && valid_ut != 0) {
     mlog << Debug(4) << "UGridFile::getData() -> "
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


StringArray UGridFile::get_metadata_names(std::string &key) {
  StringArray empty;
  auto search = metadata_map.find(key);
  return search == metadata_map.end() ? empty : metadata_map[key];
}


////////////////////////////////////////////////////////////////////////

bool UGridFile::get_var_info() {

  // Pull out the variables
  if (Var) {
    delete [] Var;
    Var = (NcVarInfo *)nullptr;
  }

  NcDim dim;
  int max_dim_count = 0;
  ConcatString att_value;
  StringArray var_names;

  Nvars = get_var_names(_ncFile, &var_names);
  Var = new NcVarInfo [Nvars];

  for (int j=0; j<Nvars; ++j)  {
    NcVar v = get_var(_ncFile, var_names[j].c_str());

    Var[j].var = new NcVar(v);
    Var[j].name = GET_NC_NAME(v).c_str();

    int dim_count = GET_NC_DIM_COUNT(v);
    Var[j].Ndims = dim_count;
    if (dim_count > max_dim_count) max_dim_count = dim_count;

    Var[j].Dims = new NcDim * [dim_count];

    //  parse the variable attributes
    get_att_str( Var[j], long_name_att_name, Var[j].long_name_att );
    get_att_str( Var[j], units_att_name,     Var[j].units_att     );

  }   //  for j

  //  done

  return true;
}

////////////////////////////////////////////////////////////////////////


int UGridFile::lead_time() const
{
  unixtime dt = ValidTime[0] - InitTime;

  return (int) dt;
}


////////////////////////////////////////////////////////////////////////


void UGridFile::read_config(ConcatString config_filename) {
  const char *method_name = "UGridFile::read_config() ";
  double conf_value;
  ConcatString conf_value_s;
  MetConfig conf;

  // Read the default config file
  mlog << Debug(6) << method_name
       << "configuration from " << config_filename << " (" << replace_path(config_filename) << ")\n";
  conf.read(replace_path(config_filename).c_str());

  conf_value = parse_conf_ugrid_max_distance_km(&conf);
  if (!is_eq(conf_value, bad_data_double)) max_distance_km = conf_value;
  conf_value_s = parse_conf_ugrid_coordinates_file(&conf);
  if (0 < conf_value_s.length()) coordinate_file = conf_value_s;
  parse_add_conf_ugrid_metadata_map(&conf, &metadata_map);

  metadata_names.clear();
  for (std::map<ConcatString,StringArray>::iterator it=metadata_map.begin();
       it!=metadata_map.end(); ++it) {
    metadata_names.add(it->second);
  }

  mlog << Debug(6) << method_name
       << "map size: " << metadata_map.size() << ", dims_vars_count = " << metadata_names.n() << "\n";

}

////////////////////////////////////////////////////////////////////////


void UGridFile::read_netcdf_grid()
{
  // Loop through the variables looking for the first gridded variable.  We
  // will use this variable to pull out the grid information.  The CF
  // description allows for different fields in the same file to have different
  // grids, but with how the gridded information is used in MET, I'm making the
  // assumption that all fields are on the same grid.

  ConcatString units_value;
  const char *method_name = "UGridFile::read_netcdf_grid() -> ";

  double *_lat = new double[face_count];
  double *_lon = new double[face_count];
  
  if (IS_INVALID_NC_P(_latVar)) {
    mlog << Error << "\n" << method_name << "latitude variable is missing\n\n";
    exit(1);
  }
  else if (!get_nc_data(_latVar,_lat)) {
    mlog << Error << "\n" << method_name << "fail to read latitude values\n\n";
    exit(1);
  }

  if (IS_INVALID_NC_P(_lonVar)) {
    mlog << Error << "\n" << method_name << "longitude variable is missing\n\n";
    exit(1);
  }
  else if (!get_nc_data(_lonVar,_lon)) {
    mlog << Error << "\n" << method_name << "fail to read latitude values\n\n";
    exit(1);
  }

  if (get_var_units(_latVar, units_value)) {
    if (units_value == "rad" || units_value == "radian") {
      mlog << Debug(6) << method_name << "convert  " << units_value << " to degree for lat\n";
      for (int idx=0; idx<face_count; idx++) _lat[idx] /= rad_per_deg;
    }
  }
  if (get_var_units(_lonVar, units_value)) {
    if (units_value == "rad" || units_value == "radian") {
      mlog << Debug(6) << method_name << "  convert " << units_value << " to degree for lon\n";
      for (int idx=0; idx<face_count; idx++) _lon[idx] /= rad_per_deg;
    }
  }

  grid_data.set_points(face_count, _lon, _lat);
  grid_data.max_distance_km = max_distance_km;

  grid.set(grid_data);

  // Pull the grid projection from the variable information.  First, look for
  // a grid_mapping attribute.

  if (_lat) delete [] _lat;
  if (_lon) delete [] _lon;

}


////////////////////////////////////////////////////////////////////////

void UGridFile::set_dataset(ConcatString _dataset_name) {

  ConcatString ugrid_config_name;
  const string method_name = "UGridFile::set_dataset() ";

  if (0 == _dataset_name.length()) {
    mlog << Error << "\n" << method_name
         << "The \"" << conf_key_ugrid_dataset
         << "\" is not defined at the configuration file.\n\n";
    exit(1);
  }
  dataset_name = _dataset_name;
  if (file_exists(dataset_name.c_str())) {
    /* UGridConfig file was passed as the ugrid_dataset */
    ugrid_config_name = dataset_name;
  }
  else {
    ConcatString dataset_config(def_config_prefix);
    dataset_config.add(dataset_name);
    if (!file_exists(dataset_config.c_str())) {
      dataset_config = def_config_prefix2;
      dataset_config.add(dataset_name);
      dataset_config = replace_path(dataset_config.c_str());
    }
    ugrid_config_name = dataset_config;
  }
  if (file_exists(ugrid_config_name.c_str())) {
    read_config(ugrid_config_name.c_str());
  }
  else {
    mlog << Error << "\n" << method_name
         << "The UGrid dataset \"" << dataset_name << "\" is not supported. Please add \""
         << ugrid_config_name << "\".\n\n";
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////////

void UGridFile::set_map_config_file(ConcatString filename) {

  if (file_exists(filename.c_str())) {
    read_config(filename.c_str());
    get_var_info();
  }
  else {
    mlog << Error << "\nUGridFile::set_map_config_file()"
         << " The UGrid metadata mapping configuration file \""
         << filename << "\" does not exist.\n\n";
    exit(1);
  }

}

////////////////////////////////////////////////////////////////////////

void UGridFile::set_max_distance_km(double max_distance) {

  max_distance_km = max_distance;
  if (grid.is_set()) {
    UnstructuredData D;
    D.copy_from(grid.info().us);
    D.max_distance_km = max_distance;
    grid.set(D);
  }

}

////////////////////////////////////////////////////////////////////////
