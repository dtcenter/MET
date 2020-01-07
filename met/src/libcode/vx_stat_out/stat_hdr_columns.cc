// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

   interp_pnts = bad_data_int;

   alpha = bad_data_double;

   // Clear the ConcatStrings
   model.clear();
   desc.clear();

   fcst_lead_str.clear();
   fcst_valid_beg_str.clear();
   fcst_valid_end_str.clear();

   obs_lead_str.clear();
   obs_valid_beg_str.clear();
   obs_valid_end_str.clear();

   fcst_var.clear();
   fcst_units.clear();
   fcst_lev.clear();

   obs_var.clear();
   obs_units.clear();
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
   model = check_hdr_str((string)s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_desc(const char *s) {
   desc = check_hdr_str((string)s);
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

void StatHdrColumns::set_fcst_var(const ConcatString s) {
   fcst_var = check_hdr_str(s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_units(const ConcatString s) {
   fcst_units = check_hdr_str(s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_lev(const char *s) {
   fcst_lev = check_hdr_str((string)s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_var(const ConcatString s) {
   obs_var = check_hdr_str(s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_units(const ConcatString s) {
   obs_units = check_hdr_str(s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_lev(const char *s) {
   obs_lev = check_hdr_str((string)s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obtype(const char *s) {
   obtype = check_hdr_str((string)s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_mask(const char *s) {
   mask = check_hdr_str((string)s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_mthd(ConcatString s,
                                     GridTemplateFactory::GridTemplates shape) {
   ConcatString mthd = s;

   // Only append the interpolation shape when applicable
   if(shape != GridTemplateFactory::GridTemplate_None &&
      mthd  != interpmthd_none_str        &&
      mthd  != interpmthd_bilin_str       &&
      mthd  != interpmthd_nearest_str     &&
      mthd  != interpmthd_budget_str      &&
      mthd  != interpmthd_force_str       &&
      mthd  != interpmthd_upper_left_str  &&
      mthd  != interpmthd_upper_right_str &&
      mthd  != interpmthd_lower_right_str &&
      mthd  != interpmthd_lower_left_str ) {
      GridTemplateFactory gtf;
      mthd << '_' << gtf.enum2String(shape).c_str();
   }
   interp_mthd = check_hdr_str(mthd);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_mthd(const InterpMthd m,
                                     GridTemplateFactory::GridTemplates shape) {
   set_interp_mthd(interpmthd_to_string(m), shape);
   return;
}

////////////////////////////////////////////////////////////////////////
//
// Assume width of a square and set interp_pnts = w * w
//
////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_wdth(const int w) {
   interp_pnts = (is_bad_data(w) ? bad_data_int : w * w);
   interp_pnts_str << cs_erase << interp_pnts;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_pnts(const int n) {
   interp_pnts = n;
   interp_pnts_str << cs_erase << interp_pnts;
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_line_type(const char *s) {
   line_type = check_hdr_str((string)s);
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

void StatHdrColumns::set_cov_thresh(const ThreshArray t) {
   cov_thresh.clear();
   cov_thresh = t;
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

   // Check for probabilities and convert to a string
   cs << prob_thresh_to_string(fcst_thresh);

   // Append thresh_logic symbol
   if(fcst_thresh.n_elements() == 1 &&
      obs_thresh.n_elements()  == 1 &&
      thresh_logic != SetLogic_None) {

      if(fcst_thresh[0].get_type() != thresh_na &&
          obs_thresh[0].get_type() != thresh_na) {
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

   fcst_lead_str = sec_to_hhmmss(fcst_lead_sec);

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_valid_beg_str() {

   fcst_valid_beg_str = unix_to_yyyymmdd_hhmmss(fcst_valid_beg);

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_valid_end_str() {

   fcst_valid_end_str = unix_to_yyyymmdd_hhmmss(fcst_valid_end);

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_lead_str() {

   obs_lead_str = sec_to_hhmmss(obs_lead_sec);

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_valid_beg_str() {

   obs_valid_beg_str = unix_to_yyyymmdd_hhmmss(obs_valid_beg);

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_valid_end_str() {

   obs_valid_end_str = unix_to_yyyymmdd_hhmmss(obs_valid_end);

   return;
}

////////////////////////////////////////////////////////////////////////
