// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <cmath>

#include "vx_cal.h"
#include "vx_util.h"

#include "conv_offsets.h"
#include "conv_record.h"
#include "rad_offsets.h"
#include "rad_record.h"
#include "gsi_util.h"

////////////////////////////////////////////////////////////////////////

ConvData parse_conv_data(const ConvRecord &r, const int i) {
   ConvData d;

   bool uv_flag = (str_trim(r.variable) == "uv");

   d.var        = str_trim(r.variable);
   d.sid        = r.station_name(i);
   d.lat        = r.rdiag_get_2d(conv_lat_index - 1, i);
   d.lon        = rescale_lon(r.rdiag_get_2d(conv_lon_index - 1, i));
   d.prs        = r.rdiag_get_2d(conv_pressure_index - 1, i);
   d.prs        = (d.var == "pw" ? bad_data_double : d.prs);

   d.elv        = r.rdiag_get_2d(conv_elevation_index - 1, i);

   d.obtype << cs_erase << nint(r.rdiag_get_2d(conv_obstype_index - 1, i));
   d.fcst_ut    = r.date;
   d.obs_ut     = nint(r.date + (r.rdiag_get_2d(conv_obs_hours_index - 1, i) * sec_per_hour));
   d.obs        = r.rdiag_get_2d(conv_obs_data_index - 1, i);
   d.guess      = r.rdiag_get_guess(i);

   if(uv_flag) {
      d.obs_v   = r.rdiag_get_2d(conv_obs_v_data_index - 1, i);
      d.guess_v = r.rdiag_get_guess_v(i);
   }

   d.obs_qc.add(nint(r.rdiag_get_2d(conv_input_qc_index - 1, i)));
   d.hgt        = nint(r.rdiag_get_2d(conv_height_index - 1, i));
   d.hgt        = (d.hgt < 0 ? bad_data_int : d.hgt);

   d.err_in     = r.rdiag_get_2d(conv_pb_inverse_index - 1, i);
   d.err_adj    = r.rdiag_get_2d(conv_read_pb_inverse_index - 1, i);
   d.err_fin    = r.rdiag_get_2d(conv_final_inverse_index - 1, i);

   d.prep_use   = nint(r.rdiag_get_2d(conv_usage_index - 1, i));
   d.anly_use   = nint(r.rdiag_get_2d(conv_analysis_use_index - 1, i));
   d.setup_qc   = nint(r.rdiag_get_2d(conv_setup_qc_index - 1, i));
   d.setup_qc   = (d.setup_qc == bad_setup_qc ? bad_data_double : d.setup_qc);
   d.qc_wght    = r.rdiag_get_2d(conv_qc_weight_index - 1, i);

   d.n_use      = 0;

   return(d);
}

////////////////////////////////////////////////////////////////////////

RadData parse_rad_data(const RadRecord  &r, const int i,
                       const int chval, const int use) {
   RadData d;

   d.var.format("TB_%02d", chval);

   d.lat       = r.diag_data(rad_lat_index - 1);
   d.lon       = rescale_lon(r.diag_data(rad_lon_index - 1));
   d.elv       = r.diag_data(rad_elevation_index - 1);

   d.fcst_ut   = r.date();
   d.obs_ut    = nint(r.date() + (r.diag_data(rad_dtime_index - 1) * sec_per_hour));
   d.obs       = r.diagchan_data(rad_btemp_chan_index - 1, i);
   d.guess     = d.obs - r.diagchan_data(rad_omg_bc_chan_index - 1, i);

   d.obs_qc.add(nint(r.diagchan_data(rad_qc_mark_index - 1, i)) * use);
   d.use       = use;
   d.scan_pos  = nint(r.diag_data(rad_scanpos_index - 1));
   d.sat_znth  = r.diag_data(rad_sat_zenith_index - 1);
   d.sat_azmth = r.diag_data(rad_sat_azimuth_index - 1);

   d.sun_znth  = r.diag_data(rad_sun_zenith_index - 1);
   d.sun_azmth = r.diag_data(rad_sun_azimuth_index - 1);
   d.sun_glnt  = r.diag_data(rad_glint_index - 1);

   d.frac_wtr  = r.diag_data(rad_water_frac_index - 1);
   d.frac_lnd  = r.diag_data(rad_land_frac_index - 1);
   d.frac_ice  = r.diag_data(rad_ice_frac_index - 1);
   d.frac_snw  = r.diag_data(rad_snow_frac_index - 1);

   d.sfc_twtr  = r.diag_data(rad_water_temp_index - 1);
   d.sfc_tlnd  = r.diag_data(rad_land_temp_index - 1);
   d.sfc_tice  = r.diag_data(rad_ice_temp_index - 1);
   d.sfc_tsnw  = r.diag_data(rad_snow_temp_index - 1);

   d.tsoil     = r.diag_data(rad_soil_temp_index - 1);
   d.soilm     = r.diag_data(rad_soil_moisture_index - 1);

   d.land_type = nint(r.diag_data(rad_land_type_index - 1));
   d.frac_veg  = r.diag_data(rad_veg_frac_index - 1);
   d.snw_dpth  = r.diag_data(rad_snow_depth_index - 1);
   d.sfc_wind  = r.diag_data(rad_wind_speed_index - 1);

   d.frac_cld  = r.diag_data(rad_cloud_frac_index - 1);
   d.ctop_prs  = r.diag_data(rad_cloud_top_pressure_index - 1);

   d.tfnd      = r.diag_data(rad_itref_index - 1);
   d.twarm     = r.diag_data(rad_idtw_index - 1);
   d.tcool     = r.diag_data(rad_idtc_index - 1);
   d.tzfnd     = r.diag_data(rad_itz_tr_index - 1);

   d.obs_err   = r.diagchan_data(rad_inv_chan_index - 1, i);
   d.fcst_nobc = d.obs - r.diagchan_data(rad_omg_nobc_chan_index - 1, i);
   d.sfc_emis  = r.diagchan_data(rad_surf_em_index - 1, i);
   d.stability = r.diagchan_data(rad_stability_index - 1, i);

   d.prs_max_wgt = (!r.has_extra() ?
                    bad_data_double :
                    r.extra_data(rad_extra_prs_max_wgt_index - 1, i));
   d.prs_max_wgt = (d.prs_max_wgt > 1.0E8 ?
                    bad_data_double :
                    d.prs_max_wgt);

   d.n_use     = 0;

   return(d);
}


////////////////////////////////////////////////////////////////////////

ConcatString get_conv_key(const ConvData &d) {
   ConcatString key;

   key << d.var << key_sep << d.sid << key_sep
       << d.lat << key_sep << d.lon << key_sep
       << d.hgt << key_sep << d.elv << key_sep
       << unix_to_yyyymmdd_hhmmss(d.fcst_ut) << key_sep
       << unix_to_yyyymmdd_hhmmss(d.obs_ut) << key_sep
       << d.obs;

   return(key);
}

////////////////////////////////////////////////////////////////////////

ConcatString get_rad_key(const RadData &d) {
   ConcatString key;

   key << d.var << key_sep
       << d.lat << key_sep << d.lon << key_sep << d.elv << key_sep
       << unix_to_yyyymmdd_hhmmss(d.fcst_ut) << key_sep
       << unix_to_yyyymmdd_hhmmss(d.obs_ut) << key_sep
       << d.obs;

   return(key);
}

////////////////////////////////////////////////////////////////////////

int key_to_integer(const char * key) {
   int int_key = 0;
   for (unsigned int idx=0; idx<strlen(key); idx++) {
      int_key += ((int)key[idx]) << (idx%3)*8;
      //int_key += (int)key[idx];
   }
   return(int_key);
}

////////////////////////////////////////////////////////////////////////

bool is_conv(const char *s) {
   return(strstr(get_short_name(s), conv_id_str) != (char *) 0);
}

////////////////////////////////////////////////////////////////////////

bool is_micro(const char *s) {
   bool status = false;

   for(int i=0; i<n_micro_id_str; i++) {
      if(strstr(get_short_name(s), micro_id_str[i]) != 0) {
         status = true;
         break;
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool is_retr(const char *s) {
   return(false);
}

////////////////////////////////////////////////////////////////////////

void setup_header(StatHdrColumns &shc,
                  const StringArray &name, const StringArray &value,
                  const char *default_line_type) {
   int index;
   SingleThresh st;

   // MODEL
   if(name.has("MODEL", index)) {
      shc.set_model(value[index].c_str());
   }
   else {
      shc.set_model(default_model);
   }

   // DESC
   if(name.has("DESC", index)) {
      shc.set_desc(value[index].c_str());
   }
   else {
      shc.set_desc(default_desc);
   }

   // FCST_LEAD
   if(name.has("FCST_LEAD", index)) {
      shc.set_fcst_lead_sec(timestring_to_sec(value[index].c_str()));
   }
   else {
      shc.set_fcst_lead_sec(default_lead);
   }

   // FCST_VALID_BEG, FCST_VALID_END
   if(name.has("FCST_VALID_BEG", index)) {
      shc.set_fcst_valid_beg(timestring_to_unix(value[index].c_str()));
      not_has_FCST_VALID_BEG = false;
   }
   if(name.has("FCST_VALID_END", index)) {
      shc.set_fcst_valid_end(timestring_to_unix(value[index].c_str()));
      not_has_FCST_VALID_END = false;
   }

   // OBS_LEAD
   if(name.has("OBS_LEAD", index)) {
      shc.set_obs_lead_sec(timestring_to_sec(value[index].c_str()));
   }
   else {
      shc.set_obs_lead_sec(default_lead);
   }

   // OBS_VALID_BEG, OBS_VALID_END
   if(name.has("OBS_VALID_BEG", index)) {
      shc.set_obs_valid_beg(timestring_to_unix(value[index].c_str()));
      not_has_OBS_VALID_BEG = false;
   }
   if(name.has("OBS_VALID_END", index)) {
      shc.set_obs_valid_end(timestring_to_unix(value[index].c_str()));
      not_has_OBS_VALID_END = false;
   }

   // FCST_VAR
   if(name.has("FCST_VAR", index)) {
      shc.set_fcst_var(value[index]);
      not_has_FCST_VAR = false;
   }

   // FCST_LEV
   if(name.has("FCST_LEV", index)) {
      shc.set_fcst_lev(value[index].c_str());
   }
   else {
      shc.set_fcst_lev(default_lev);
   }

   // OBS_VAR
   if(name.has("OBS_VAR", index)) {
      shc.set_obs_var(value[index]);
      not_has_OBS_VAR = false;
   }

   // OBS_LEV
   if(name.has("OBS_LEV", index)) {
      shc.set_obs_lev(value[index].c_str());
   }
   else {
      shc.set_obs_lev(default_lev);
   }

   // OBTYPE
   if(name.has("OBTYPE", index)) {
      shc.set_obtype(value[index].c_str());
      not_has_OBTYPE = false;
   }
   else {
      shc.set_obtype(default_obtype);
   }

   // VX_MASK
   if(name.has("VX_MASK", index)) {
      shc.set_mask(value[index].c_str());
   }
   else {
      shc.set_mask(default_vx_mask);
   }

   // INTERP_MTHD
   if(name.has("INTERP_MTHD", index)) {
      shc.set_interp_mthd(value[index]);
   }
   else {
     shc.set_interp_mthd((string)default_interp_mthd);
   }

   // INTERP_PNTS
   if(name.has("INTERP_PNTS", index)) {
      shc.set_interp_wdth(nint(sqrt(atof(value[index].c_str()))));
   }
   else {
      shc.set_interp_wdth(default_interp_wdth);
   }

   // FCST_THRESH
   if(name.has("FCST_THRESH", index)) st.set(value[index].c_str());
   else                                   st.set(default_thresh);
   shc.set_fcst_thresh(st);

   // OBS_THRESH
   if(name.has("OBS_THRESH", index))  st.set(value[index].c_str());
   else                                   st.set(default_thresh);
   shc.set_obs_thresh(st);

   // COV_THRESH
   if(name.has("COV_THRESH", index))  st.set(value[index].c_str());
   else                                   st.set(default_thresh);
   shc.set_cov_thresh(st);

   // ALPHA
   if(name.has("ALPHA", index)) {
      shc.set_alpha(atof(value[index].c_str()));
   }
   else {
      shc.set_alpha(default_alpha);
   }

   // LINE_TYPE
   if(name.has("LINE_TYPE", index)) {
      shc.set_line_type(value[index].c_str());
   }
   else {
      shc.set_line_type(default_line_type);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Justify the STAT AsciiTable objects
   justify_stat_cols(at);

   // Set the precision
   at.set_precision(default_precision);

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////
