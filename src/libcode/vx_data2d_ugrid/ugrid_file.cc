// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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
#include "vx_log.h"
#include "config_util.h"

#include "ugrid_file.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////

constexpr char def_user_config[] = "UGridConfig_user";
constexpr char def_config_prefix[] = "UGridConfig_";
constexpr char def_config_prefix2[] = "MET_BASE/config/UGridConfig_";
constexpr double lat_epsilon = 0.00001;

array<string, UG_DIM_COUNT> DIM_KEYS = {
      "dim_face", "dim_node", "dim_edge", "dim_time", "dim_vert"
};

array<string, UG_META_VAR_COUNT> COORD_VAR_KEYS = {
      "time", "lat_face", "lon_face", "vert_face", "lat_edge",
      "lon_edge", "lat_node", "lon_node", "cell_id", "init_time"
};

static double get_nc_var_att_double(const NcVar *nc_var, const char *att_name,
                                    bool is_required=true);

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
  _zVar = (NcVar *)nullptr;
  _tVar = (NcVar *)nullptr;
  _init_time_var = (NcVar *)nullptr;

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
    _ncFile = nullptr;
  }

  if (_ncMetaFile) {
    delete _ncMetaFile;
    _ncMetaFile = nullptr;
  }

  // Reclaim the dimension pointers

  _numDims = 0;

  _dimNames.clear();
  metadata_map.clear();
  metadata_names.clear();

  _faceDim = _edgeDim = _tDim = (NcDim *)nullptr;

  // Reclaim the variable pointers

  if (Var) {
    for (int j = 0; j < Nvars; ++j) {
      if (Var[j].var) { delete Var[j].var; Var[j].var = nullptr; }
      if (Var[j].Dims) { delete[] Var[j].Dims; Var[j].Dims = nullptr; }
    }
    delete [] Var;
    Var = (NcVarInfo *)nullptr;
  }

  Nvars = 0;

  // Delete MetaVar Dims arrays
  for (int j = 0; j < UG_META_VAR_COUNT; ++j) {
    if (MetaVar[j].var) { delete MetaVar[j].var; MetaVar[j].var = nullptr; }
    if (MetaVar[j].Dims) { delete[] MetaVar[j].Dims; MetaVar[j].Dims = nullptr; }
  }

  // Clear other members
  _numDims = 0;
  _dimNames.clear();
  metadata_map.clear();
  metadata_names.clear();
  z_var_name.clear();
  max_distance_km = bad_data_double;

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
// Helper: Assign dimension from metadata

void UGridFile::assign_dim_from_metadata(const netCDF::NcFile* ncFile, netCDF::NcDim*& dim_ptr,
                                         const std::string& key, const StringArray& dim_names) {
  std::string meta_name = find_metadata_name(key, dim_names);
  if (!meta_name.empty()) {
    NcDim dim = get_nc_dim(ncFile, meta_name);
    dim_ptr = new NcDim(dim);
  }
  else {
    mlog << Debug(7) << "UGridFile::assign_dim_from_metadata() "
         << "dimension for " << key << " does not exist\n";
  }
}

////////////////////////////////////////////////////////////////////////


bool UGridFile::open(const char * filepath)
{

  // Close any open files and clear out the associated members
  close();

  // Open the file
  _ncFile = open_ncfile(filepath);

  if (IS_INVALID_NC_P(_ncFile)) {
    close();
    return false;
  }

  return get_var_info();
}


////////////////////////////////////////////////////////////////////////


bool UGridFile::open_metadata(const char * filepath)
{
  const char *method_name = "UGridFile::open_metadata() -> ";

  // Open the file
  _ncMetaFile = open_ncfile(filepath);

  mlog << Debug(7) << method_name << "open " << filepath << "\n";

  if (IS_INVALID_NC_P(_ncMetaFile)) {
    close();
    exit(1);
  }

  StringArray dim_names;
  get_dim_names(_ncMetaFile, &dim_names);

  // Face (cell) dimension
  assign_dim_from_metadata(_ncFile, _faceDim, DIM_KEYS[0], dim_names);
  if (IS_VALID_NC_P(_faceDim)) {
    string meta_name = find_metadata_name(DIM_KEYS[0], dim_names);
    if (!meta_name.empty()) {
      face_count = get_dim_size(_faceDim);
      NcDim face_dim = get_nc_dim(_ncFile, meta_name);
      int data_face_count = get_dim_size(&face_dim);
      if (face_count != data_face_count) {
        mlog << Error << "\n" << method_name
             << meta_name << " dimension is different: data file = "
             << data_face_count << ", coordinates file = " << face_count << "\n\n";
        exit(1);
      }
    }
  }

  assign_dim_from_metadata(_ncFile, _nodeDim, DIM_KEYS[1], dim_names);  // Node (vertex) dimension
  assign_dim_from_metadata(_ncFile, _edgeDim, DIM_KEYS[2], dim_names);  // Edge dimension
  assign_dim_from_metadata(_ncFile, _tDim, DIM_KEYS[3], dim_names);     // Time dimension
  assign_dim_from_metadata(_ncFile, _virtDim, DIM_KEYS[4], dim_names);  // Vertical dimension

  metadata_coord_variables();

  InitTime = 0;
  if (!metadata_time()) {
    mlog << Debug(4) << method_name
         << "could not extract valid time from the "
         << "time variable from " << filepath << "\n";
  }

  // Override InitTime if init_time is defined at the configuration file
  if (_init_time_var != nullptr) {
    InitTime = get_init_time(_init_time_var);
  }

  // Get InitTime from the forecast_reference_time
  if (InitTime == 0 ) InitTime = get_init_time(_ncFile);


  // Pull out the grid.  This must be done after pulling out the dimension
  // and variable information since this information is used to pull out the
  // grid.  This call sets the _faceDim and _edgeDim pointers.

  read_netcdf_grid();

  // Now go back through the variables and use _faceDim, _edgeDim, and _tDim
  // to set the slots.
  // Should be called after read_netcdf_grid() is called

  StringArray dimNames;
  string time_dim_name = find_metadata_name(DIM_KEYS[3], dim_names);
  string vert_dim_name = find_metadata_name(DIM_KEYS[4], dim_names);
  for (int j=0; j<Nvars; ++j) {

    int dim_count = Var[j].Ndims;
    const NcVar *v = Var[j].var;

    dimNames.clear();
    get_dim_names(v, &dimNames);

    for (int k=0; k<dim_count; ++k)  {
      const NcDim *dim_p = Var[j].Dims[k];
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
  if (IS_INVALID_NC_P(_zVar) && (!vert_dim_name.empty())) {
    NcVarInfo *info = find_var_by_dim_name(vert_dim_name.c_str());
    if (info) _zVar = info->var;
  }

  // Pull out the vertical levels
  if (IS_VALID_NC_P(_zVar)) {
    int z_count = get_data_size(_zVar);
    vector<double> z_values(z_count);

    if( get_nc_data(_zVar, z_values.data()) ) {
      for(int i=0; i<z_count; i++) {
        vlevels.add(z_values[i]);
      }
    }
  }

  //  done
  return true;
}


////////////////////////////////////////////////////////////////////////


void UGridFile::dump(ostream & out, int depth) const
{
  ConcatString cs;

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

  int month;
  int day;
  int year;
  int hour;
  int minute;
  int second;

  unix_to_mdyhms(InitTime, month, day, year, hour, minute, second);

  cs.format("%s %d, %d   %2d:%02d:%02d",
            short_month_name[month], day, year, hour, minute, second);

  out << cs << "\n";

  out << prefix << "\n";

  if (AccumTime > 0) {
    unix_to_mdyhms(AccumTime, month, day, year, hour, minute, second);
    cs.format("%2d:%02d:%02d (%d seconds)",
              hour, minute, second, (int)AccumTime);
    out << prefix << "Accum Time = ";
    out << cs << "\n";
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

std::string UGridFile::find_metadata_name(const std::string &key,
                                          const StringArray &available_names) {
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
  if (var == nullptr) {
    for (int i=0; i<Nvars; i++) {
      if (1 != Var[i].Ndims) continue;

      NcDim dim = get_nc_dim(Var[i].var, 0);
      if (GET_NC_NAME(dim) == dim_name) {
        var = &Var[i];
        break;
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
    if (Var[i].name.startswith(var_name)) vinfo_list.emplace_back(&Var[i]);
  }
  return !vinfo_list.empty();
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

  status = get_nc_data(var, a);

  if (!status)
  {
    mlog << Error << "\n" << method_name << "bad status for var->get()\n\n";
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
         << "needed " << dim_count << " arguments for variable "
         << (GET_NC_NAME_P(v)) << ", got " << a.n_elements() << "\n\n";
    exit(1);
  }

  //  find varinfo's

  const NcVarInfo *var = find_by_name(GET_NC_NAME_P(v).c_str());

  if (nullptr == var) {
    mlog << Error << "\n" << method_name
         << "variable " << GET_NC_NAME_P(v) << " not found!\n\n";
    return true;
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
  vector<double> d(plane_size);

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

  get_nc_data(v, d.data(), lengths, offsets);

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


StringArray UGridFile::get_metadata_names(const std::string &key) {
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


void UGridFile::metadata_coord_variables() {
  static const string method_name
      = "UGridFile::metadata_coord_variables() => ";

  //Variables at the data file first
  StringArray time_names = get_metadata_names(COORD_VAR_KEYS[0]);
  StringArray lat_names = get_metadata_names(COORD_VAR_KEYS[1]);
  StringArray lon_names = get_metadata_names(COORD_VAR_KEYS[2]);
  StringArray z_names = get_metadata_names(COORD_VAR_KEYS[3]);
  StringArray init_time_names = get_metadata_names(COORD_VAR_KEYS[9]);
  for (int j=0; j<Nvars; ++j) {
    if (time_names.has(Var[j].name)) {
      _tVar = Var[j].var;
      //_time_var_info = &Var[j];
    }
    else if (lat_names.has(Var[j].name)) _latVar = Var[j].var;
    else if (lon_names.has(Var[j].name)) _lonVar = Var[j].var;
    else if (z_names.has(Var[j].name)) {
      _zVar = Var[j].var;
      z_var_name = Var[j].name;
    }
    else if (init_time_names.has(Var[j].name)) {
      _init_time_var = Var[j].var;
      mlog << Debug(97) << method_name
           << "found _init_time_var (" << GET_NC_NAME_P(_init_time_var)
           << ") from data file\n";
    }
  }

  // Variables at the coordinate file (could be the same as the data file)
  StringArray var_names;
  get_var_names(_ncMetaFile, &var_names);
  for (int j=0; j<COORD_VAR_KEYS.size(); j++) {
    string meta_name = find_metadata_name(COORD_VAR_KEYS[j], var_names);
    if (0 < meta_name.length()) {
      NcVar v = get_var(_ncMetaFile, meta_name.c_str());

      MetaVar[j].var = new NcVar(v);
      MetaVar[j].name = GET_NC_NAME(v).c_str();

      int dim_count = GET_NC_DIM_COUNT(v);
      MetaVar[j].Ndims = dim_count;
      MetaVar[j].Dims = new NcDim * [dim_count];

      //  parse the variable attributes
      get_att_str( MetaVar[j], long_name_att_name, MetaVar[j].long_name_att );
      get_att_str( MetaVar[j], units_att_name,     MetaVar[j].units_att     );

      if (0 == j && nullptr == _tVar) {
        _tVar = MetaVar[j].var;
        //_time_var_info = &MetaVar[j];
      }
      else if (1 == j && nullptr == _latVar) _latVar = MetaVar[j].var;
      else if (2 == j && nullptr == _lonVar) _lonVar = MetaVar[j].var;
      else if (3 == j && nullptr == _zVar) _zVar = MetaVar[j].var;
      else if (9 == j && nullptr == _init_time_var) {
        _init_time_var = MetaVar[j].var;
        mlog << Debug(97) << method_name
             << "found _init_time_var (" << GET_NC_NAME_P(_init_time_var)
             << ") from data file\n";
      }
    }
  }   //  for j
}

////////////////////////////////////////////////////////////////////////

bool UGridFile::metadata_time() {
  const char *method_name = "UGridFile::metadata_time() -> ";

  // Pull out the valid and init times
  if (IS_INVALID_NC_P(_tVar)) {
    ValidTime.add(0);
    return false;
  }

  // Store the dimension for the time variable as the time dimension
  bool use_bounds_var = false;
  int time_dim_count = get_dim_count(_tVar);
  if (nullptr == _tDim && (time_dim_count == 1 || time_dim_count == 2)) {
     NcDim tDim = get_nc_dim(_tVar, 0);
     if (IS_VALID_NC(tDim)) {
       _tDim = new NcDim(tDim);
     }
  }

  int data_type = GET_NC_TYPE_ID_P(_tVar);
  bool is_string_time = (NC_CHAR == data_type || NC_STRING == data_type);

  // Determine the number of times present.
  int n_times = IS_VALID_NC_P(_tDim) ? get_dim_size(_tDim)
                                     : get_data_size(_tVar);
  vector<double> time_values(n_times);
  if(is_string_time) {   // String type: YYYY-MM-DD HH:MM:SS
    mlog << Debug(7) << method_name
           << "from " << GET_NC_NAME_P(_tVar) << "\n";
    for(int i=0; i<n_times; i++) {
      time_values[i] = get_nc_time(_tVar, i);
      ValidTime.add(time_values[i]);
      raw_times.add(time_values[i]);
      mlog << Debug(7) << method_name
           << "get time " << time_values[i] << " ("
           << unix_to_yyyymmdd_hhmmss(time_values[i]) << ")\n";
    }
  }
  else if( get_nc_data(_tVar, time_values.data()) ) {
    // Parse the units for the time variable.
    int sec_per_unit = 0;
    bool no_leap_year = true;
    unixtime ref_ut = get_reference_unixtime(_tVar, sec_per_unit,
                                             no_leap_year, method_name);
    if (ref_ut != 0) InitTime = ref_ut;

    if (use_bounds_var) {
      double bounds_diff;
      for(int i=0; i<n_times; i++) {
        ValidTime.add(add_to_unixtime(ref_ut, sec_per_unit, time_values[i*2+1],
                      no_leap_year));
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
        raw_times.add(time_values[i]);
        ValidTime.add(add_to_unixtime(ref_ut, sec_per_unit, time_values[i],
                                      no_leap_year));
      }
    }
  }
  else ValidTime.add(0);  //Initialize

  return true;
}

////////////////////////////////////////////////////////////////////////


void UGridFile::read_config(const ConcatString &config_filename) {
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
  if (!conf_value_s.empty()) coordinate_file = conf_value_s;
  parse_add_conf_ugrid_metadata_map(&conf, &metadata_map);

  metadata_names.clear();
  for (auto it=metadata_map.begin();
       it!=metadata_map.end(); ++it) {
    metadata_names.add(it->second);
  }

  mlog << Debug(6) << method_name
       << "map size: " << metadata_map.size() << ", dims_vars_count = " << metadata_names.n() << "\n";

}

////////////////////////////////////////////////////////////////////////

void UGridFile::radian_to_degree(vector<double> &lat_values, const int lat_count) const {
  const char *method_name = "UGridFile::radian_to_degree() -> ";
  int lat_adjusted = 0;
  int lat_adjusted_by_precision = 0;
  for (int idx=0; idx<lat_count; idx++) {
    lat_values[idx] /= rad_per_deg;
    if (lat_values[idx] > 90.0) {
      if (is_eq(lat_values[idx], 90.0, lat_epsilon)) lat_adjusted_by_precision++;
      else {
        mlog << Warning << "\n" << method_name << "adjusted " << lat_values[idx]
             << " (delta: " << (lat_values[idx] - 90.0) << ") to 90.0\n\n";
        lat_adjusted++;
      }
      lat_values[idx] = 90.0;
    }
    else if (lat_values[idx] < -90.0) {
      if (is_eq(lat_values[idx], -90.0, lat_epsilon)) lat_adjusted_by_precision++;
      else {
        mlog << Warning << "\n" << method_name << "adjusted " << lat_values[idx]
             << " (delta: " << (lat_values[idx] + 90.0) << ") to -90.0\n\n";
        lat_adjusted++;
      }
      lat_values[idx] = -90.0;
    }
  }

  if (lat_adjusted > 0) {
    mlog << Warning << "\n" << method_name << "adjusted " << lat_adjusted << " latitudes ("
         << lat_adjusted_by_precision << " by precision)\n\n";
  }
  else if (lat_adjusted_by_precision > 0) {
    mlog << Debug(4) << method_name << "adjusted " << lat_adjusted_by_precision << " latitudes by precision\n";
  }
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

  vector<double> _lat(face_count);
  vector<double> _lon(face_count);

  if (IS_INVALID_NC_P(_latVar)) {
    mlog << Error << "\n" << method_name << "latitude variable is missing\n\n";
    exit(1);
  }
  else if (!get_nc_data(_latVar,_lat.data())) {
    mlog << Error << "\n" << method_name << "fail to read latitude values\n\n";
    exit(1);
  }

  if (IS_INVALID_NC_P(_lonVar)) {
    mlog << Error << "\n" << method_name << "longitude variable is missing\n\n";
    exit(1);
  }
  else if (!get_nc_data(_lonVar,_lon.data())) {
    mlog << Error << "\n" << method_name << "fail to read latitude values\n\n";
    exit(1);
  }

  if (get_var_units(_latVar, units_value) &&
      (units_value == "rad" || units_value == "radian")) {
    mlog << Debug(6) << method_name << "convert " << units_value << " to degree for lat\n";
    radian_to_degree(_lat, face_count);
  }
  if (get_var_units(_lonVar, units_value) &&
      (units_value == "rad" || units_value == "radian")) {
    mlog << Debug(6) << method_name << "convert " << units_value << " to degree for lon\n";
    for (int idx=0; idx<face_count; idx++) _lon[idx] /= rad_per_deg;
  }

  // Convert longitude from degrees east to west
  for (int idx=0; idx<face_count; idx++) _lon[idx] = -1.0*rescale_deg(_lon[idx], -180, 180);

  grid_data.set_points(face_count, _lon.data(), _lat.data());
  grid_data.max_distance_km = max_distance_km;

  grid.set(grid_data);

  // Pull the grid projection from the variable information.  First, look for
  // a grid_mapping attribute.

}


////////////////////////////////////////////////////////////////////////

void UGridFile::set_dataset(const ConcatString &_dataset_name) {

  ConcatString ugrid_config_name;
  const string method_name = "UGridFile::set_dataset() ";

  if (_dataset_name.empty()) {
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

void UGridFile::set_map_config_file(const ConcatString &filename) {

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
