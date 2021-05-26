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

#include "nc_point_obs.h"
#include "write_netcdf.h"

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetNcPointObs
   //


////////////////////////////////////////////////////////////////////////

MetNcPointObs::MetNcPointObs() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MetNcPointObs::~MetNcPointObs() {
   close();
}

////////////////////////////////////////////////////////////////////////

void MetNcPointObs::init_from_scratch() {
   obs_nc = (NcFile *) 0;

   nobs = 0;
   nhdr = 0;
   qty_length = 0;
   keep_nc = false;
   use_var_id = false;
   use_arr_vars = false;

   return;
}

////////////////////////////////////////////////////////////////////////

void MetNcPointObs::close() {
   if ( !keep_nc && obs_nc ) {
      delete obs_nc;
      obs_nc = (NcFile *) 0;
   }

   //obs_vars.reset();
   obs_data.clear();
   header_data.clear();
   return;
}

////////////////////////////////////////////////////////////////////////

int MetNcPointObs::get_qty_length() {
   qty_length = get_nc_string_length(&obs_vars.obs_qty_tbl_var);
   return qty_length;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::get_header(int header_offset, float hdr_arr[HDR_ARRAY_LEN],
      ConcatString &hdr_typ_str, ConcatString &hdr_sid_str, ConcatString &hdr_vld_str) {
   int hdr_idx;

   // Read the corresponding header array for this observation
   hdr_arr[0] = header_data.lat_array[header_offset];
   hdr_arr[1] = header_data.lon_array[header_offset];
   hdr_arr[2] = header_data.elv_array[header_offset];

   // Read the corresponding header type for this observation
   hdr_idx = use_arr_vars ? header_offset : header_data.typ_idx_array[header_offset];
   hdr_typ_str = header_data.typ_array[hdr_idx];

   // Read the corresponding header Station ID for this observation
   hdr_idx = use_arr_vars ? header_offset : header_data.sid_idx_array[header_offset];
   hdr_sid_str = header_data.sid_array[hdr_idx];

   // Read the corresponding valid time for this observation
   hdr_idx = use_arr_vars ? header_offset : header_data.vld_idx_array[header_offset];
   hdr_vld_str =  header_data.vld_array[hdr_idx];

   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::get_header_type(int header_offset, int hdr_typ_arr[HDR_TYPE_ARR_LEN]) {
   int hdr_idx;
   // Read the header integer types
   hdr_typ_arr[0] = (header_data.prpt_typ_array.n() > header_offset ?
                     header_data.prpt_typ_array[header_offset] : bad_data_int);
   hdr_typ_arr[1] = (header_data.irpt_typ_array.n() > header_offset ?
                     header_data.irpt_typ_array[header_offset] : bad_data_int);
   hdr_typ_arr[2] = (header_data.inst_typ_array.n() > header_offset ?
                     header_data.inst_typ_array[header_offset] : bad_data_int);

   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::get_lats(float *hdr_lats) {
   for (int idx=0; idx<nhdr; idx++) {
      hdr_lats[idx] = header_data.lat_array[idx];
   }
   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::get_lons(float *hdr_lons) {
   for (int idx=0; idx<nhdr; idx++) {
      hdr_lons[idx] = header_data.lon_array[idx];
   }
   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::open(const char * filename) {
   return set_netcdf(open_ncfile(filename));
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::set_netcdf(NcFile *nc_file, bool _keep_nc) {
   close();
   keep_nc = _keep_nc;
   obs_nc = nc_file;
   return IS_VALID_NC_P(obs_nc);
}

//////////////////////////////////////////////////////////////////////////

