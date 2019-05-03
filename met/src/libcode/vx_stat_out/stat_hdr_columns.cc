// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <string.h>

#include "stat_hdr_columns.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class StatHdrColumns
//
////////////////////////////////////////////////////////////////////////

StatHdrColumns::StatHdrColumns() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

StatHdrColumns::~StatHdrColumns() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::clear() {

   // Set values to zero
   fcst_lead_sec    = 0;
   fcst_valid_beg   = (unixtime) 0;
   fcst_valid_end   = (unixtime) 0;
   obs_lead_sec     = 0;
   obs_valid_beg    = (unixtime) 0;
   obs_valid_end    = (unixtime) 0;

   interp_mthd = InterpMthd_None;
   interp_wdth = bad_data_int;

   alpha = bad_data_double;

   // Clear the ConcatStrings
   model.clear();

   fcst_lead_str.clear();
   fcst_valid_beg_str.clear();
   fcst_valid_end_str.clear();

   obs_lead_str.clear();
   obs_valid_beg_str.clear();
   obs_valid_end_str.clear();

   fcst_var.clear();
   fcst_lev.clear();

   obs_var.clear();
   obs_lev.clear();

   msg_typ.clear();
   mask.clear();

   interp_mthd_str.clear();
   interp_pnts_str.clear();

   line_type.clear();

   fcst_thresh.clear();
   fcst_thresh_str.clear();

   obs_thresh.clear();
   obs_thresh_str.clear();

   cov_thresh.clear();
   cov_thresh_str.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::clear_fcst_thresh() {
   fcst_thresh.clear();
   fcst_thresh_str = na_str;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::clear_obs_thresh() {
   obs_thresh.clear();
   obs_thresh_str = na_str;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::clear_cov_thresh() {
   cov_thresh.clear();
   cov_thresh_str = na_str;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_model(const char *s) {
   model = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_lead_sec(const int s) {
   fcst_lead_sec = s;
   set_fcst_lead_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_valid_beg(const unixtime ut) {
   fcst_valid_beg = ut;
   set_fcst_valid_beg_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_valid_end(const unixtime ut) {
   fcst_valid_end = ut;
   set_fcst_valid_end_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_lead_sec(const int s) {
   obs_lead_sec = s;
   set_obs_lead_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_valid_beg(const unixtime ut) {
   obs_valid_beg = ut;
   set_obs_valid_beg_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_valid_end(const unixtime ut) {
   obs_valid_end = ut;
   set_obs_valid_end_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_var(const char *s) {
   fcst_var = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_lev(const char *s) {
   fcst_lev = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_var(const char *s) {
   obs_var = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_lev(const char *s) {
   obs_lev = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_msg_typ(const char *s) {
   msg_typ = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_mask(const char *s) {
   mask = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_mthd(const InterpMthd m) {
   interp_mthd = m;
   set_interp_mthd_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_wdth(const int w) {
   interp_wdth = w;
   set_interp_pnts_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_line_type(const char *s) {
   line_type = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_thresh(const SingleThresh t) {
   fcst_thresh = t;
   set_fcst_thresh_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_thresh(const ThreshArray t) {
   char tmp_str[max_str_len];

   fcst_thresh.clear();
   fcst_thresh_str.clear();

   // Concatenate all of the forecast thresholds used
   t.get_str(",", tmp_str);
   fcst_thresh_str << tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_thresh(const SingleThresh t) {
   obs_thresh = t;
   set_obs_thresh_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_thresh(const ThreshArray t) {
   char tmp_str[max_str_len];

   obs_thresh.clear();
   obs_thresh_str.clear();

   // Concatenate all of the forecast thresholds used
   t.get_str(",", tmp_str);
   obs_thresh_str << tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_cov_thresh(const SingleThresh t) {
   cov_thresh = t;
   set_cov_thresh_str();
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_alpha(const double a) {
   alpha = a;
   return;
}

////////////////////////////////////////////////////////////////////////
//
// Private routines
//
////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_lead_str() {
   int hr, min, sec;
   char tmp_str[max_str_len];

   // Convert lead_sec to hour, minute, second
   sec_to_hms(fcst_lead_sec, hr, min, sec);

   // Format the time HHMMSS
   sprintf(tmp_str, "%.2i%.2i%.2i", hr, min, sec);

   // Set lead_str
   fcst_lead_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_valid_beg_str() {
   int yr, mon, day, hr, min, sec;
   char tmp_str[max_str_len];

   // Convert fcst_valid_beg to month, day, year, hour, minute, second
   unix_to_mdyhms(fcst_valid_beg, mon, day, yr, hr, min, sec);

   // Format the time YYYYMMDD_HHMMSS
   sprintf(tmp_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
           yr, mon, day, hr, min, sec);

   // Set fcst_valid_beg_str
   fcst_valid_beg_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_valid_end_str() {
   int yr, mon, day, hr, min, sec;
   char tmp_str[max_str_len];

   // Convert fcst_valid_end to month, day, year, hour, minute, second
   unix_to_mdyhms(fcst_valid_end, mon, day, yr, hr, min, sec);

   // Format the time YYYYMMDD_HHMMSS
   sprintf(tmp_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
           yr, mon, day, hr, min, sec);

   // Set fcst_valid_end_str
   fcst_valid_end_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_lead_str() {
   int hr, min, sec;
   char tmp_str[max_str_len];

   // Convert lead_sec to hour, minute, second
   sec_to_hms(obs_lead_sec, hr, min, sec);

   // Format the time HHMMSS
   sprintf(tmp_str, "%.2i%.2i%.2i", hr, min, sec);

   // Set lead_str
   obs_lead_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_valid_beg_str() {
   int yr, mon, day, hr, min, sec;
   char tmp_str[max_str_len];

   // Convert obs_valid_beg to month, day, year, hour, minute, second
   unix_to_mdyhms(obs_valid_beg, mon, day, yr, hr, min, sec);

   // Format the time YYYYMMDD_HHMMSS
   sprintf(tmp_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
           yr, mon, day, hr, min, sec);

   // Set obs_valid_beg_str
   obs_valid_beg_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_valid_end_str() {
   int yr, mon, day, hr, min, sec;
   char tmp_str[max_str_len];

   // Convert obs_valid_end to month, day, year, hour, minute, second
   unix_to_mdyhms(obs_valid_end, mon, day, yr, hr, min, sec);

   // Format the time YYYYMMDD_HHMMSS
   sprintf(tmp_str, "%.4i%.2i%.2i_%.2i%.2i%.2i",
           yr, mon, day, hr, min, sec);

   // Set obs_valid_end_str
   obs_valid_end_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_mthd_str() {

   interp_mthd_str = interpmthd_to_string(interp_mthd);

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_pnts_str() {
   char tmp_str[max_str_len];

   if(interp_wdth == bad_data_int) strcpy(tmp_str, na_str);
   else                            sprintf(tmp_str, "%i",
                                           interp_wdth*interp_wdth);
   interp_pnts_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_thresh_str() {
   char tmp_str[max_str_len];

   fcst_thresh.get_str(tmp_str);
   fcst_thresh_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_thresh_str() {
   char tmp_str[max_str_len];

   obs_thresh.get_str(tmp_str);
   obs_thresh_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_cov_thresh_str() {
   char tmp_str[max_str_len];

   cov_thresh.get_str(tmp_str);
   cov_thresh_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////
