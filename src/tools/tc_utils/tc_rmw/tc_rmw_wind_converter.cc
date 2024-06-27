// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_rmw.cc
//
//   Description:  Finds and manages the U/V grids in input and
//                 handles their conversion to tangential/radial winds
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000   05/11/22   Albo           Pulled the wind conversion into a class
//   001   09/28/22   Prestopnik     MET #2227 Remove namespace std from header files
//   002   05/03/24   Halley Gotway  MET #2841 Fix radial and tangential winds
//
////////////////////////////////////////////////////////////////////////

#include "tc_rmw_wind_converter.h"
#include "series_data.h"
#include "vx_regrid.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static void wind_ne_to_rt(const TcrmwGrid&,
                          const DataPlane&, const DataPlane&,
                          double*, double*);

////////////////////////////////////////////////////////////////////////

void TCRMW_WindConverter::_free_winds_arrays(void) {
  if (_windR != nullptr) {
    delete [] _windR;
    _windR = nullptr;
  }
  if (_windT != nullptr) {
    delete [] _windT;
    _windT = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////

TCRMW_WindConverter::TCRMW_WindConverter(void) :
  _windR(nullptr),
  _windT(nullptr),
  _foundUInInput(false),
  _foundVInInput(false),
  _unitsU("Unknown"),
  _conf(nullptr),
  _computeWinds(false) {
}

////////////////////////////////////////////////////////////////////////

TCRMW_WindConverter::~TCRMW_WindConverter(void) {
  _free_winds_arrays();
}

////////////////////////////////////////////////////////////////////////

void TCRMW_WindConverter::init(const TCRMWConfInfo *conf) {
  _conf = conf;
  _computeWinds = _conf->compute_tangential_and_radial_winds;
  _free_winds_arrays();
  _windR = new double[_conf->n_range*_conf->n_azimuth];
  _windT = new double[_conf->n_range*_conf->n_azimuth];
  _foundUInInput = false;
  _foundVInInput = false;
  _unitsU = "Unknown";
  _uIndexMap.clear();
  _vIndexMap.clear();

  if (!_computeWinds) {
    return;
  }

  VarInfo* data_info = (VarInfo*) nullptr;
  for(int i_var = 0; i_var < _conf->get_n_data(); i_var++) {
    data_info = _conf->data_info[i_var];
    string varname = data_info->name_attr().string();
    string varlevel = data_info->level_attr().string();
    if (varname == _conf->u_wind_field_name.string()) {
      _uIndexMap[varlevel] = i_var;
    }
    else if (varname == _conf->v_wind_field_name.string()) {
      _vIndexMap[varlevel] = i_var;
    }
  }

  // test for consistency
  if (_uIndexMap.size() != _vIndexMap.size()) {
    mlog << Warning << "Uneven number of u/v wind inputs, no wind conversion will be done:\n"
         << _conf->u_wind_field_name.string() << " has " << _uIndexMap.size() << " inputs\n"
         << _conf->v_wind_field_name.string() << " has " << _vIndexMap.size() << " inputs\n";
    _computeWinds = false;
  }
  if (_computeWinds) {
    map<string,int>::const_iterator iu, iv;
    for (iu=_uIndexMap.begin(), iv=_vIndexMap.begin(); iu!=_uIndexMap.end(); ++iu, ++iv) {
      if (iu->first != iv->first) {
        mlog << Warning << "Ordering of u/v wind input levels not the same, "
             << "not implemented, no wind conversions will be done:\n"
             << "    " << iu->first  << " " << iv->first << "\n";
        _computeWinds = false;
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////

void TCRMW_WindConverter::update_input(const string &variableName, const string &units) {
  if (_computeWinds) {
    if (variableName == _conf->u_wind_field_name.string()) {
      _foundUInInput = true;
      _unitsU = units;
    }
    else if (variableName == _conf->v_wind_field_name.string()) {
      _foundVInInput = true;
    }
  }
} 

////////////////////////////////////////////////////////////////////////

void TCRMW_WindConverter::append_nc_output_vars(map<string, vector<string> > &variable_levels,
                                                map<string, string> &variable_long_names,
                                                map<string, string> &variable_units) {
  if (!_computeWinds) return;

  if (_foundUInInput && _foundVInInput) {
    variable_levels[_conf->tangential_velocity_field_name] = variable_levels[_conf->u_wind_field_name.string()];
    variable_long_names[_conf->tangential_velocity_field_name] = _conf->tangential_velocity_long_field_name.string();
    variable_units[_conf->tangential_velocity_field_name] = _unitsU;
    variable_levels[_conf->radial_velocity_field_name] = variable_levels[_conf->u_wind_field_name.string()];
    variable_long_names[_conf->radial_velocity_field_name] = _conf->radial_velocity_long_field_name.string();
    variable_units[_conf->radial_velocity_field_name] = _unitsU;
  }
  else {
    if (!_foundUInInput) {
      mlog << Warning << "\nTCWRMW_WindConverter::append_nc_output_vars() -> "
           << "field not found in input \"" << _conf->u_wind_field_name << "\"\n\n";
    }
    if (!_foundVInInput) {
      mlog << Warning << "\nTCWRMW_WindConverter::append_nc_output_vars() -> "
           << "field not found in input \"" << _conf->v_wind_field_name << "\"\n\n";
    }
    mlog << Warning << "\nTCWRMW_WindConverter::append_nc_output_vars() -> "
         << "Not computing radial and tangential winds\n\n";
    _computeWinds = false;
  }
}

////////////////////////////////////////////////////////////////////////

bool TCRMW_WindConverter::compute_winds_if_input_is_u(int i_point,
                                                      const string &varName,
                                                      const string &varLevel,
                                                      unixtime valid_time,
                                                      const StringArray &data_files,
                                                      const GrdFileType &ftype,
                                                      const Grid &grid_in,
                                                      const Grid &grid_out,
                                                      const DataPlane &u_wind_dp,
                                                      const TcrmwGrid &tcrmw_grid) {
  if (!_computeWinds) {
    return false;
  }

  int uIndex = -1;
  int vIndex = -1;
  VarInfo *v_wind_info = (VarInfo *) nullptr;
  if (varName == _conf->u_wind_field_name.string()) {
    uIndex = _uIndexMap[varLevel];
    vIndex = _vIndexMap[varLevel];
    v_wind_info = _conf->data_info[vIndex];
    v_wind_info->set_valid(valid_time);
  }
  else {
    // not the U input
    return false;
  }

  DataPlane v_wind_dp;
  Grid v_wind_grid;
  get_series_entry(i_point, v_wind_info, data_files, ftype,
                   v_wind_dp, v_wind_grid);
  double dmin, dmax, dmin_rgd, dmax_rgd;
  v_wind_dp.data_range(dmin, dmax);
  v_wind_dp = met_regrid(v_wind_dp, v_wind_grid, grid_out, v_wind_info->regrid());
  v_wind_dp.data_range(dmin_rgd, dmax_rgd);

  mlog << Debug(4) << v_wind_info->magic_str()
       << " input range (" << dmin << ", " << dmax
       << "), regrid range (" << dmin_rgd << ", " << dmax_rgd << ")\n";

  // Compute the radial and tangential winds and store in _windR and _windT
  wind_ne_to_rt(tcrmw_grid, u_wind_dp, v_wind_dp, _windR, _windT);

  return true;
}

////////////////////////////////////////////////////////////////////////

void wind_ne_to_rt(const TcrmwGrid& tcrmw_grid,
                   const DataPlane& u_dp, const DataPlane& v_dp,
                   double* wind_r_arr, double* wind_t_arr) {

  int n_rng = tcrmw_grid.range_n();
  int n_azi = tcrmw_grid.azimuth_n(); 

  // Transform (u, v) to (radial, tangential) winds
  for(int ir = 0; ir < n_rng; ir++) {
    for(int ia = 0; ia < n_azi; ia++) {

      // Store data in reverse order
      int i_rev = (n_rng - ir - 1) * n_azi + ia;

      double azi_deg  = ia * tcrmw_grid.azimuth_delta_deg();
      double range_km = ir * tcrmw_grid.range_delta_km();

      double lat, lon;
      tcrmw_grid.range_azi_to_latlon(range_km, azi_deg, lat, lon);

      tcrmw_grid.wind_ne_to_rt(azi_deg, u_dp.data()[i_rev], v_dp.data()[i_rev],
                               wind_r_arr[i_rev], wind_t_arr[i_rev]);

      mlog << Debug(4) << "wind_ne_to_rt() -> "
           << "center lat/lon (" << tcrmw_grid.lat_center_deg()
           << ", " << tcrmw_grid.lon_center_deg()
           << "), range (km): " << range_km
           << ", azimuth (deg): " << azi_deg
           << ", point lat/lon (" << lat << ", " << lon
           << "), uv (" << u_dp.data()[i_rev] << ", " << v_dp.data()[i_rev]
           << "), radial wind: " << wind_r_arr[i_rev]
           << ", tangential wind: " << wind_t_arr[i_rev] << "\n";
    } // end for ia
  } // end for ir

  return;
}

////////////////////////////////////////////////////////////////////////
