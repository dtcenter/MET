// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include "tc_rmw_wind_converter.h"
#include "series_data.h"
#include "vx_regrid.h"

////////////////////////////////////////////////////////////////////////

static void wind_ne_to_ra(const TcrmwGrid&,
                          const DataPlane&, const DataPlane&,
                          const double*, const double*,
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

  VarInfo* data_info = (VarInfo*) 0;
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
      mlog << Warning << "\nTCWRMW_WindConverter::checkInputs() -> "
           << "field not found in input \"" << _conf->u_wind_field_name << "\"\n\n";
    }
    if (!_foundVInInput) {
      mlog << Warning << "\nTCWRMW_WindConverter::checkInputs() -> "
           << "field not found in input \"" << _conf->v_wind_field_name << "\"\n\n";
    }
    mlog << Warning << "\nTCWRMW_WindConverter::checkInputs() -> "
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
                                                      const Grid &latlon_arr,
                                                      const double *lat_arr,
                                                      const double *lon_arr,
                                                      const Grid &grid,
                                                      const DataPlane &data_dp,
                                                      const TcrmwGrid &tcrmw_grid) {
  if (!_computeWinds) {
    return false;
  }

  int uIndex = -1;
  int vIndex = -1;
  VarInfo *data_infoV = (VarInfo *) 0;
  if (varName == _conf->u_wind_field_name.string()) {
    uIndex = _uIndexMap[varLevel];
    vIndex = _vIndexMap[varLevel];
    data_infoV = _conf->data_info[vIndex];
    data_infoV->set_valid(valid_time);
  }
  else {
    // not the U input
    return false;
  }

  DataPlane data_dpV;
  Grid latlon_arrV;
  get_series_entry(i_point, data_infoV, data_files, ftype,
                   data_dpV, latlon_arrV);
  double dmin, dmax, dmin_rgd, dmax_rgd;
  data_dpV.data_range(dmin, dmax);
  data_dpV = met_regrid(data_dpV, latlon_arr, grid, data_infoV->regrid());
  data_dpV.data_range(dmin_rgd, dmax_rgd);

  mlog << Debug(4) << data_infoV->magic_str()
       << " input range (" << dmin << ", " << dmax
       << "), regrid range (" << dmin_rgd << ", " << dmax_rgd << ")\n";

  // Compute the radial and tangential winds and store in _windR and _windT
  wind_ne_to_ra(tcrmw_grid, data_dp, data_dpV, lat_arr, lon_arr,
                _windR, _windT);

  return true;
}

////////////////////////////////////////////////////////////////////////

void wind_ne_to_ra(const TcrmwGrid& tcrmw_grid,
                   const DataPlane& u_dp, const DataPlane& v_dp,
                   const double* lat_arr, const double* lon_arr,
                   double* wind_r_arr, double* wind_t_arr) {

  // Transform (u, v) to (radial, azimuthal)
  for(int ir = 0; ir < tcrmw_grid.range_n(); ir++) {
    for(int ia = 0; ia < tcrmw_grid.azimuth_n(); ia++) {
      int i = ir * tcrmw_grid.azimuth_n() + ia;
      double lat = lat_arr[i];
      double lon = -lon_arr[i]; // convert degrees east to west
      double u = u_dp.data()[i];
      double v = v_dp.data()[i];
      double wind_r;
      double wind_t;
      if(is_bad_data(u) || is_bad_data(v)) {
        mlog << Debug(4) << "wind_ne_to_ra() -> "
             << "latlon (" << lat << "," << lon
             << ") winds are missing\n";
        wind_r = bad_data_double;
        wind_t = bad_data_double;
      } else {
        tcrmw_grid.wind_ne_to_ra(lat, lon, u, v, wind_r, wind_t);
        mlog << Debug(4) << "wind_ne_to_ra() -> "
             << "latlon (" << lat << ", " << lon
             << "), uv (" << u << ", " << v
             << "), radial wind: " << wind_r
             << ", tangential wind: " << wind_t << "\n";
      }
      wind_r_arr[i] = wind_r;
      wind_t_arr[i] = wind_t;
    } // end for ia
  } // end for ir

  return;
}

////////////////////////////////////////////////////////////////////////
