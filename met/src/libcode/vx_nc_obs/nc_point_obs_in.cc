// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
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
#include <time.h>

#include "vx_log.h"

#include "nc_point_obs_in.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetNcPointObs
   //


////////////////////////////////////////////////////////////////////////

MetNcPointObsIn::MetNcPointObsIn() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MetNcPointObsIn::~MetNcPointObsIn() {
   close();
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObsIn::check_nc(const char *nc_name, const char *caller) {
   bool exit_on_error = false;
   bool valid = obs_vars.is_valid(exit_on_error);
   if (!valid) {
      mlog << Error << "\n" << (0 != caller ? caller : "") << " -> "
           << "missing core data from the netCDF file: "
           << nc_name << "\n\n";
      exit(1);
   }
   return valid;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObsIn::read_dim_headers() {
   bool status = false;
   if( IS_VALID_NC_P(obs_nc) ) {
      status = true;
      obs_vars.read_dims_vars(obs_nc);
      nobs = obs_vars.obs_cnt = GET_NC_SIZE(obs_vars.obs_dim);
      nhdr = obs_vars.hdr_cnt = GET_NC_SIZE(obs_vars.hdr_dim);
      obs_vars.use_var_id = use_var_id = IS_VALID_NC(obs_vars.obs_vid_var);
      use_arr_vars = IS_VALID_NC(obs_vars.obs_arr_var);
      obs_vars.read_header_data(header_data);
   }
   return status;
}

//////////////////////////////////////////////////////////////////////

bool MetNcPointObsIn::read_obs_data() {
   bool status = read_obs_data_numbers() && read_obs_data_table_lookups();
   return status;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObsIn::read_obs_data(int buf_size, int start, float *obs_arr_block,
                                    int *obs_qty_idx_block, char *obs_qty_str_block) {
  return obs_vars.read_obs_data(buf_size, start, qty_length, obs_arr_block,
                                obs_qty_idx_block, obs_qty_str_block);
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObsIn::read_obs_data_numbers() {
   bool status = false;
   if( IS_VALID_NC_P(obs_nc) ) {
      status = ((NcPointObsData *)obs_data)->read_obs_data_numbers(obs_vars);
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObsIn::read_obs_data_table_lookups() {
   bool status = false;
   if( IS_VALID_NC_P(obs_nc) ) {
      status = ((NcPointObsData *)obs_data)->read_obs_data_table_lookups(obs_vars);
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

