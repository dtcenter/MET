// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

   obtype.clear();
   mask.clear();

   interp_mthd.clear();
   interp_pnts_str.clear();

   line_type.clear();

   fcst_thresh.clear();
   obs_thresh.clear();
   cov_thresh.clear();

   thresh_logic = SetLogic_None;

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

void StatHdrColumns::set_obtype(const char *s) {
   obtype = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_mask(const char *s) {
   mask = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_mthd(const char *s) {
   interp_mthd = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_mthd(const InterpMthd m) {
   set_interp_mthd(interpmthd_to_string(m));
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
   fcst_thresh.clear();
   fcst_thresh.add(t);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_thresh(const ThreshArray t) {
   fcst_thresh.clear();
   fcst_thresh = t;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_thresh(const SingleThresh t) {
   obs_thresh.clear();
   obs_thresh.add(t);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_thresh(const ThreshArray t) {
   obs_thresh.clear();
   obs_thresh = t;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_thresh_logic(const SetLogic t) {
   thresh_logic = t;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_cov_thresh(const SingleThresh t) {
   cov_thresh.clear();
   cov_thresh.add(t);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_alpha(const double a) {
   alpha = a;
   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString StatHdrColumns::get_fcst_thresh_str() const {
   ConcatString cs;

   cs << fcst_thresh.get_str();

   // Append thresh_logic symbol
   if(fcst_thresh.n_elements() == 1 &&
      obs_thresh.n_elements()  == 1 &&
      thresh_logic != SetLogic_None) {

      if(fcst_thresh[0].get_type() != thresh_na &&
         obs_thresh[0].get_type()  != thresh_na) {
         cs << setlogic_to_symbol(thresh_logic);
      }
   }
   return(cs);
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

void StatHdrColumns::set_interp_pnts_str() {
   char tmp_str[max_str_len];

   if(interp_wdth == bad_data_int) strcpy(tmp_str, na_str);
   else                            sprintf(tmp_str, "%i",
                                           interp_wdth*interp_wdth);
   interp_pnts_str = tmp_str;

   return;
}

////////////////////////////////////////////////////////////////////////
