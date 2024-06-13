// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <string.h>

#include "stat_hdr_columns.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static const string case_str = "CASE";

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

   thresh_logic = SetLogic::None;

   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_model(const char *s) {
   model = check_hdr_str(conf_key_model, (string) s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_desc(const char *s) {
   desc = check_hdr_str(conf_key_desc, (string) s);
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
   fcst_var = check_hdr_str(conf_key_fcst_var, s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_units(const ConcatString s) {
   fcst_units = check_hdr_str(conf_key_fcst_units, s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_fcst_lev(const char *s) {
   fcst_lev = check_hdr_str(conf_key_fcst_lev, (string) s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_var(const ConcatString s) {
   obs_var = check_hdr_str(conf_key_obs_var, s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_units(const ConcatString s) {
   obs_units = check_hdr_str(conf_key_obs_units, s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obs_lev(const char *s) {
   obs_lev = check_hdr_str(conf_key_obs_lev, (string) s, true);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_obtype(const char *s) {
   obtype = check_hdr_str(conf_key_obtype, (string) s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_mask(const char *s) {
   mask = check_hdr_str(conf_key_vx_mask, (string) s);
   return;
}

////////////////////////////////////////////////////////////////////////

void StatHdrColumns::set_interp_mthd(ConcatString s,
                                     GridTemplateFactory::GridTemplates shape) {
   ConcatString mthd = s;

   // Only append the interpolation shape when applicable
   if(shape != GridTemplateFactory::GridTemplates::None &&
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
   interp_mthd = check_hdr_str(conf_key_interp_mthd, mthd);
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
   line_type = check_hdr_str(conf_key_line_type, (string) s);
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

void StatHdrColumns::apply_set_hdr_opts(
        const StringArray &hdr_cols, const StringArray &hdr_vals) {
   StringArray case_cols;
   StringArray case_vals;

   // Call other implementation without case information
   apply_set_hdr_opts(hdr_cols, hdr_vals, case_cols, case_vals);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Use the current -set_hdr options to populate the STAT header columns,
// substituting in case-specific values, as needed.
//
////////////////////////////////////////////////////////////////////////

void StatHdrColumns::apply_set_hdr_opts(
        const StringArray &hdr_cols, const StringArray &hdr_vals,
        const StringArray &case_cols, const StringArray &case_vals) {

   // No updates needed
   if(hdr_cols.n() == 0) return;

   // Sanity check lengths
   if(hdr_cols.n() != hdr_vals.n()) {
      mlog << Error << "\napply_set_hdr_opts() -> "
           << "the number of -set_hdr columns names (" << hdr_cols.n()
           << " and values (" << hdr_vals.n() << " must match!\n\n";
      exit(1);
   }
   if(case_cols.n()  != case_vals.n()) {
      mlog << Error << "\napply_set_hdr_opts() -> "
           << "the number of case columns names (" << case_cols.n()
           << " and values (" << case_vals.n() << " must match!\n\n";
      exit(1);
   }

   int index;
   ConcatString cs;
   SingleThresh st;

   // MODEL
   if(hdr_cols.has("MODEL", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_model(cs.c_str());
   }

   // DESC
   if(hdr_cols.has("DESC", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_desc(cs.c_str());
   }

   // FCST_LEAD
   if(hdr_cols.has("FCST_LEAD", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_fcst_lead_sec(timestring_to_sec(cs.c_str()));
   }

   // FCST_VALID_BEG, FCST_VALID_END
   if(hdr_cols.has("FCST_VALID_BEG", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_fcst_valid_beg(timestring_to_unix(cs.c_str()));
   }
   if(hdr_cols.has("FCST_VALID_END", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_fcst_valid_end(timestring_to_unix(cs.c_str()));
   }

   // OBS_LEAD
   if(hdr_cols.has("OBS_LEAD", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_obs_lead_sec(timestring_to_sec(cs.c_str()));
   }

   // OBS_VALID_BEG, OBS_VALID_END
   if(hdr_cols.has("OBS_VALID_BEG", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_obs_valid_beg(timestring_to_unix(cs.c_str()));
   }
   if(hdr_cols.has("OBS_VALID_END", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_obs_valid_end(timestring_to_unix(cs.c_str()));
   }

   // FCST_VAR
   if(hdr_cols.has("FCST_VAR", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_fcst_var(cs.c_str());
   }

   // FCST_UNITS
   if(hdr_cols.has("FCST_UNITS", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_fcst_units(cs.c_str());
   }

   // FCST_LEV
   if(hdr_cols.has("FCST_LEV", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_fcst_lev(cs.c_str());
   }

   // OBS_VAR
   if(hdr_cols.has("OBS_VAR", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_obs_var(cs.c_str());
   }

   // OBS_UNITS
   if(hdr_cols.has("OBS_UNITS", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_obs_units(cs.c_str());
   }

   // OBS_LEV
   if(hdr_cols.has("OBS_LEV", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_obs_lev(cs.c_str());
   }

   // OBTYPE
   if(hdr_cols.has("OBTYPE", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_obtype(cs.c_str());
   }

   // VX_MASK
   if(hdr_cols.has("VX_MASK", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_mask(cs.c_str());
   }

   // INTERP_MTHD
   if(hdr_cols.has("INTERP_MTHD", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_interp_mthd(cs.c_str());
   }

   // INTERP_PNTS
   if(hdr_cols.has("INTERP_PNTS", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_interp_wdth(nint(sqrt(atof(cs.c_str()))));
   }

   // FCST_THRESH
   if(hdr_cols.has("FCST_THRESH", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      st.set(cs.c_str());
      set_fcst_thresh(st);
   }

   // OBS_THRESH
   if(hdr_cols.has("OBS_THRESH", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      st.set(cs.c_str());
      set_obs_thresh(st);
   }

   // COV_THRESH
   if(hdr_cols.has("COV_THRESH", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      st.set(cs.c_str());
      set_cov_thresh(st);
   }

   // ALPHA
   if(hdr_cols.has("ALPHA", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_alpha(atof(cs.c_str()));
   }

   // LINE_TYPE
   if(hdr_cols.has("LINE_TYPE", index)) {
      cs = get_set_hdr_str(hdr_vals[index], case_cols, case_vals);
      set_line_type(cs.c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString StatHdrColumns::get_set_hdr_str(const std::string &hdr_val,
                const StringArray &case_cols, const StringArray &case_vals) {
   ConcatString cs;
   int index;

   // Check for the full CASE string
   if(case_str.compare(hdr_val) == 0) {
      cs = case_vals.serialize(":");
   }
   // Check for one of the case columns
   else if(case_cols.has(hdr_val, index)) {
      cs = case_vals[index];
   }
   // Otherwise, use the current header value
   else {
      cs = hdr_val;
   }

   // Sanity check for empty strings
   if(cs.empty()) cs = na_str;

   return cs;
}

////////////////////////////////////////////////////////////////////////

ConcatString StatHdrColumns::get_fcst_thresh_str() const {
   ConcatString cs;

   // Check for probabilities and convert to a string
   cs << prob_thresh_to_string(fcst_thresh);

   // Append thresh_logic symbol
   if(fcst_thresh.n_elements() == 1 &&
      obs_thresh.n_elements()  == 1 &&
      thresh_logic != SetLogic::None) {

      if(fcst_thresh[0].get_type() != thresh_na &&
          obs_thresh[0].get_type() != thresh_na) {
         cs << setlogic_to_symbol(thresh_logic);
      }
   }
   return cs;
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
