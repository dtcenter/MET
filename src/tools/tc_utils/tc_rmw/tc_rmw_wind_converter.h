// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_rmw_wind_converter.h
//
//   Description:  Finds and manages the U/V grids in input and
//                 handles their conversion to tangential/radial winds
//
//   Mod#  Date      Name      Description
//   ----  ----      ----      -----------
//   000   05/11/22  DaveAlbo  New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __TC_RMW_WIND_CONVERTER_H__
#define  __TC_RMW_WIND_CONVERTER_H__

////////////////////////////////////////////////////////////////////////

#include "tc_rmw_conf_info.h"

#include <string>
#include <map>

using std::map;
using std::string;


////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Miscellaneous Variables
//
////////////////////////////////////////////////////////////////////////

class TCRMW_WindConverter {

 private:

  void _free_winds_arrays(void);

 public:

  // Wind arrays are created locally
  double* _windR;
  double* _windT;

  // flag when searching input data for U and V data
  bool _foundUInInput;
  bool _foundVInInput;

  // units common to U wind and radial and tangential velocity
  string _unitsU;

  // pointer to the configuration is passed into the init routine
  const TCRMWConfInfo *_conf;

  // local status yea/nea on computing winds, can be set to false if
  // an error occurs, even if configured for true
  bool _computeWinds;

  // maps from level strings to the u/v wind input index values
  map<string, int> _uIndexMap;
  map<string, int> _vIndexMap;

  // constructor
  TCRMW_WindConverter(void);

  // destructor
  ~TCRMW_WindConverter(void);

  // return the tangential wind array, computed and stored locally
  inline const double *get_wind_t_arr(void) const {return _windT;}

  // return the radial wind array, computed and stored locally
  inline const double *get_wind_r_arr(void) const {return _windR;}

  // initialize using the configuation
  void init(const TCRMWConfInfo *conf);

  // update input status as regards finding U and V data
  void update_input(const std::string &variableName, const std::string &units);

  // if computing winds, and found U and V,
  // append to the input maps for tangential and radial winds data
  // if configured to compute winds, but didn't find U or V, turn off
  // the wind computations and report an error
  void append_nc_output_vars(std::map<std::string, std::vector<std::string> > &variable_levels,
			     std::map<std::string, std::string> &variable_long_names,
			     std::map<std::string, std::string> &variable_units);

  // Check input varName against U, and if it's a match, lookup V using the
  // map members, and then compute tangential and radial winds if it is so
  // using the remaining inputs.
  // One such computation should happen for each varLevel as the calling
  // software loops through the inputs
  // If true if returned, the winds can be accessed by calls to
  // get_wind_t_arr() and get_wind_r_arr()
  bool compute_winds_if_input_is_u(int i_point,
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
				   const TcrmwGrid &tcrmw_grid);
};



////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_RMW_WIND_CONVERTER_H__  */

