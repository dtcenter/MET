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
#include <time.h>

#include "vx_log.h"
#include "is_bad_data.h"

#include "met_point_data.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetPointData
   //


////////////////////////////////////////////////////////////////////////

MetPointData::MetPointData() {
   // Derived class should set obs_data
   obs_data = (MetPointObsData *) nullptr;
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MetPointData::~MetPointData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void MetPointData::init_from_scratch() {
   nobs = 0;
   nhdr = 0;
   qty_length = 0;

   use_var_id = false;
   use_arr_vars = false;
}

////////////////////////////////////////////////////////////////////////

void MetPointData::clear() {
   if (obs_data) obs_data->clear();
   header_data.clear();
}

////////////////////////////////////////////////////////////////////////

bool MetPointData::get_header(int header_offset, float hdr_arr[HDR_ARRAY_LEN],
                                  ConcatString &hdr_typ_str, ConcatString &hdr_sid_str,
                                  ConcatString &hdr_vld_str) {
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

bool MetPointData::get_header_type(int header_offset, int hdr_typ_arr[HDR_TYPE_ARR_LEN]) {
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

bool MetPointData::get_lats(float *hdr_lats) {
   for (int idx=0; idx<nhdr; idx++) {
      hdr_lats[idx] = header_data.lat_array[idx];
   }
   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetPointData::get_lons(float *hdr_lons) {
   for (int idx=0; idx<nhdr; idx++) {
      hdr_lons[idx] = header_data.lon_array[idx];
   }
   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetPointData::is_same_obs_values(const float obs_arr1[OBS_ARRAY_LEN],
                                          const float obs_arr2[OBS_ARRAY_LEN]) {
   return is_eq(obs_arr1[0], obs_arr1[0]) &&  is_eq(obs_arr1[2], obs_arr2[2])
          && is_eq(obs_arr1[3], obs_arr2[3]);
}

//////////////////////////////////////////////////////////////////////////

void MetPointData::set_hdr_cnt(int hdr_cnt) {
   nhdr = hdr_cnt;
   header_data.hdr_count = hdr_cnt;
}

//////////////////////////////////////////////////////////////////////////

void MetPointData::set_obs_cnt(int obs_cnt) {
   nobs = obs_cnt;
   obs_data->obs_cnt = obs_cnt;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class MetPointDataPython
//
////////////////////////////////////////////////////////////////////////

MetPointDataPython::MetPointDataPython() {
   obs_data = new MetPointObsData();
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MetPointDataPython::MetPointDataPython(MetPointDataPython &d) {
   init_from_scratch();
   obs_data = new MetPointObsData();
   MetPointObsData *from_obs_data = d.get_point_obs_data();
   if (from_obs_data) obs_data->assign(*from_obs_data);
   header_data.assign(*d.get_header_data());
}

////////////////////////////////////////////////////////////////////////

MetPointDataPython::~MetPointDataPython() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void MetPointDataPython::allocate(int obs_cnt) {
   set_obs_cnt(obs_cnt);
   obs_data->allocate();
}

///////////////////////////////////////////////////////////////////////////////
//
// Code for struct MetPointObsData
//
///////////////////////////////////////////////////////////////////////////////

MetPointObsData::MetPointObsData() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

void MetPointObsData::allocate() {
   if (is_obs_array) {
      obs_arr.resize(obs_cnt*OBS_ARRAY_LEN, bad_data_float);  // nobs * 5
   }
   else {
      obs_ids.resize(obs_cnt, bad_data_int);      // grib_code or var_id
      obs_hids.resize(obs_cnt, bad_data_int);
      obs_qids.resize(obs_cnt, bad_data_int);
      obs_lvls.resize(obs_cnt, bad_data_float);
      obs_hgts.resize(obs_cnt, bad_data_float);
      obs_vals.resize(obs_cnt, bad_data_float);
   }
}

///////////////////////////////////////////////////////////////////////////////

void MetPointObsData::assign(MetPointObsData &o) {
   clear();
   obs_cnt = o.obs_cnt;
   is_obs_array = o.is_obs_array;
   obs_arr = o.obs_arr;
   obs_ids = o.obs_ids;
   obs_hids = o.obs_hids;
   obs_qids = o.obs_qids;
   obs_lvls = o.obs_lvls;
   obs_hgts = o.obs_hgts;
   obs_vals = o.obs_vals;
   var_names = o.var_names;
   qty_names = o.qty_names;
}

///////////////////////////////////////////////////////////////////////////////

void MetPointObsData::clear() {
   obs_cnt = 0;
   is_obs_array = false;
   clear_numbers();
   clear_strings();
}

///////////////////////////////////////////////////////////////////////////////

void MetPointObsData::clear_numbers() {
   obs_arr.clear();
   obs_ids.clear();
   obs_hids.clear();
   obs_qids.clear();
   obs_lvls.clear();
   obs_hgts.clear();
   obs_vals.clear();
}

///////////////////////////////////////////////////////////////////////////////

void MetPointObsData::clear_strings() {
   var_names.clear();
   qty_names.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool MetPointObsData::fill_obs_buf(int buf_size, int offset,
                                   float *obs_arr_buf, int *qty_idx_arr) {
   bool result = false;
   const char *method_name = "fill_obs_data() -> ";

   if (obs_cnt < (buf_size + offset)) {
      mlog << Error << "\n" << method_name
           << "obs data is not ready\n\n";
   }
   else {
      float *tmp_obs_arr = obs_arr_buf;
      for(int index=0; index<buf_size; index++) {
         int data_offset = offset + index;
         *tmp_obs_arr++ = (float)obs_hids[data_offset];
         *tmp_obs_arr++ = (float)obs_ids[data_offset];
         *tmp_obs_arr++ = obs_lvls[data_offset];
         *tmp_obs_arr++ = obs_hgts[data_offset];
         *tmp_obs_arr++ = obs_vals[data_offset];
         qty_idx_arr[index] = obs_qids[data_offset];
      }
      result = true;
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////

float MetPointObsData::get_obs_val(int index) const {
   float obs_val = (is_obs_array ? obs_arr[index*obs_cnt+4] : obs_vals[index]);
   return obs_val;
}

///////////////////////////////////////////////////////////////////////////////

string MetPointObsData::get_obs_qty(int index) const {
   const char *method_name = "MetPointObsData::get_obs_qty() -> ";

   if(index < 0 || index >= obs_cnt) {
      mlog << Error << "\n" << method_name
           << "index value (" << index << ") out of range for "
           << obs_cnt << " observations.\n\n";
      exit(1);
   }
   if(obs_qids[index] < 0 || obs_qids[index] >= qty_names.n()) {
      mlog << Error << "\n" << method_name
           << "observation quality index (" << obs_qids[index]
           << ") out of range for " << qty_names.n()
           << " quality strings.\n\n";
      exit(1);
   }

   return qty_names[(obs_qids[index])];
}

///////////////////////////////////////////////////////////////////////////////

// struct MetPointHeader

MetPointHeader::MetPointHeader()
{
   reset_counters();
}

///////////////////////////////////////////////////////////////////////////////

void MetPointHeader::assign(MetPointHeader &h) {

   typ_len = h.typ_len;
   sid_len = h.sid_len;
   vld_len = h.vld_len;
   strl_len = h.strl_len;
   strll_len = h.strll_len;
   hdr_count = h.hdr_count;
   hdr_type_count = h.hdr_type_count;

   typ_array = h.typ_array;
   sid_array = h.sid_array;
   vld_array = h.vld_array;
   vld_num_array = h.vld_num_array;
   typ_idx_array = h.typ_idx_array;
   sid_idx_array = h.sid_idx_array;
   vld_idx_array = h.vld_idx_array;
   lat_array = h.lat_array;
   lon_array = h.lon_array;
   elv_array = h.elv_array;
   prpt_typ_array = h.prpt_typ_array;
   irpt_typ_array = h.irpt_typ_array;
   inst_typ_array = h.inst_typ_array;
}

///////////////////////////////////////////////////////////////////////////////

void MetPointHeader::clear() {
   reset_counters();

   typ_array.clear();
   sid_array.clear();
   vld_array.clear();
   vld_num_array.clear();
   typ_idx_array.clear();
   sid_idx_array.clear();
   vld_idx_array.clear();
   lat_array.clear();
   lon_array.clear();
   elv_array.clear();
   prpt_typ_array.clear();
   irpt_typ_array.clear();
   inst_typ_array.clear();

}

///////////////////////////////////////////////////////////////////////////////

void MetPointHeader::reset_counters() {
   //valid_point_obs = false;
   typ_len = 0;
   sid_len = 0;
   vld_len = 0;
   strl_len = 0;
   strll_len = 0;
   hdr_count = 0;
   hdr_type_count = 0;

   min_vld_time = -1;
   max_vld_time = -1;
}

///////////////////////////////////////////////////////////////////////////////
