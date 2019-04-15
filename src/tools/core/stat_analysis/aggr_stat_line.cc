// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   aggr_stat_line.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add aggr_rhist_lines and
//                    aggr_orank_lines.
//   002    06/09/10  Halley Gotway   Add aggr_mctc_lines.
//   003    06/21/10  Halley Gotway   Add support for vif_flag.
//   004    07/28/10  Halley Gotway   Write out lines prior to error
//                    checking and add input line to error messages.
//   005    03/07/13  Halley Gotway   Add aggregate SSVAR lines.
//   006    06/03/14  Halley Gotway   Add aggregate PHIST lines.
//   007    07/28/14  Halley Gotway   Add aggregate_stat for MPR to WDIR.
//   008    02/05/15  Halley Gotway   Add StatHdrInfo to keep track of
//                    unique header entries for each aggregation.
//   009    03/30/15  Halley Gotway   Add ramp job type.
//   010    06/09/17  Halley Gotway   Add aggregate RELP lines.
//   011    06/09/17  Halley Gotway   Add aggregate GRAD lines.
//   012    03/01/18  Halley Gotway   Update summary job type.
//   013    04/25/18  Halley Gotway   Add ECNT line type.
//   014    04/01/19  Fillmore        Add FCST and OBS units.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>

#include "vx_log.h"
#include "vx_data2d_grib.h"
#include "vx_data2d_nc_pinterp.h"

#include "aggr_stat_line.h"
#include "parse_stat_line.h"

////////////////////////////////////////////////////////////////////////

static bool is_precip_var_name(const ConcatString &s);
static const std::string case_str = "CASE";

////////////////////////////////////////////////////////////////////////
//
// Code for StatHdrInfo structure.
//
////////////////////////////////////////////////////////////////////////

StatHdrInfo::StatHdrInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void StatHdrInfo::clear() {
   model.clear();
   desc.clear();
   fcst_lead.clear();
   fcst_valid_beg = fcst_valid_end = (unixtime) 0;
   obs_lead.clear();
   obs_valid_beg = obs_valid_end = (unixtime) 0;
   fcst_var.clear();
   fcst_units.clear();
   fcst_lev.clear();
   obs_var.clear();
   obs_units.clear();
   obs_lev.clear();
   obtype.clear();
   vx_mask.clear();
   interp_mthd.clear();
   interp_pnts.clear();
   fcst_thresh.clear();
   obs_thresh.clear();
   cov_thresh.clear();
   alpha.clear();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Keep track the unique STAT header entries for each line.
//
////////////////////////////////////////////////////////////////////////

void StatHdrInfo::add(const STATLine &line) {
   ConcatString cs;
   if(!model.has(line.model()))
      model.add(line.model());
   if(!desc.has(line.desc()))
      desc.add(line.desc());
   if(!fcst_lead.has(line.fcst_lead()))
      fcst_lead.add(line.fcst_lead());
   if(fcst_valid_beg == (unixtime) 0 || line.fcst_valid_beg() < fcst_valid_beg)
      fcst_valid_beg = line.fcst_valid_beg();
   if(fcst_valid_end == (unixtime) 0 || line.fcst_valid_end() > fcst_valid_end)
      fcst_valid_end = line.fcst_valid_end();
   if(!obs_lead.has(line.obs_lead()))
      obs_lead.add(line.obs_lead());
   if(obs_valid_beg == (unixtime) 0 || line.obs_valid_beg() < obs_valid_beg)
      obs_valid_beg = line.obs_valid_beg();
   if(obs_valid_end == (unixtime) 0 || line.obs_valid_end() > obs_valid_end)
      obs_valid_end = line.obs_valid_end();
   if(!fcst_var.has(line.fcst_var()))
      fcst_var.add(line.fcst_var());
   if(!fcst_units.has(line.fcst_units()))
      fcst_units.add(line.fcst_units());
   if(!fcst_lev.has(line.fcst_lev()))
      fcst_lev.add(line.fcst_lev());
   if(!obs_var.has(line.obs_var()))
      obs_var.add(line.obs_var());
   if(!obs_units.has(line.obs_units()))
      obs_units.add(line.obs_units());
   if(!obs_lev.has(line.obs_lev()))
      obs_lev.add(line.obs_lev());
   if(!obtype.has(line.obtype()))
      obtype.add(line.obtype());
   if(!vx_mask.has(line.vx_mask()))
      vx_mask.add(line.vx_mask());
   if(!interp_mthd.has(line.interp_mthd()))
      interp_mthd.add(line.interp_mthd());
   if(!interp_pnts.has(line.interp_pnts()))
      interp_pnts.add(line.interp_pnts());
   cs = line.get_item("FCST_THRESH", false);
   cs.strip_paren();
   if(!fcst_thresh.has(cs)) fcst_thresh.add(cs);
   cs = line.get_item("OBS_THRESH", false);
   cs.strip_paren();
   if(!obs_thresh.has(cs)) obs_thresh.add(cs);
   cs = line.get_item("COV_THRESH", false);
   cs.strip_paren();
   if(!cov_thresh.has(cs)) cov_thresh.add(cs);
   if(!alpha.has(line.alpha()))
      alpha.add(line.alpha());

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check for and print debug and warning messages about multiple header
// column values found.
//
////////////////////////////////////////////////////////////////////////

void StatHdrInfo::check_shc(const ConcatString &cur_case) {

   // MODEL
   if(model.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << model.n_elements()
           << " unique MODEL values: "
           << write_css(model) << "\n";
   }

   // DESC
   if(desc.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << desc.n_elements()
           << " unique DESC values: "
           << write_css(desc) << "\n";
   }

   // FCST_LEAD
   if(fcst_lead.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_lead.n_elements()
           << " unique FCST_LEAD values: "
           << write_css_hhmmss(fcst_lead) << "\n";
   }

   // OBS_LEAD
   if(obs_lead.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_lead.n_elements()
           << " unique OBS_LEAD values: "
           << write_css_hhmmss(obs_lead) << "\n";
   }

   // FCST_VAR
   if(fcst_var.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_var.n_elements()
           << " unique FCST_VAR values: "
           << write_css(fcst_var) << "\n";
   }

   // FCST_UNITS
   if(fcst_units.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_units.n_elements()
           << " unique FCST_UNITS values: "
           << write_css(fcst_units) << "\n";
   }

   // FCST_LEV
   if(fcst_lev.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_lev.n_elements()
           << " unique FCST_LEV values: "
           << write_css(fcst_lev) << "\n";
   }

   // OBS_VAR
   if(obs_var.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_var.n_elements()
           << " unique OBS_VAR values: "
           << write_css(obs_var) << "\n";
   }

   // OBS_UNITS
   if(obs_units.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_units.n_elements()
           << " unique OBS_UNITS values: "
           << write_css(obs_units) << "\n";
   }

   // OBS_LEV
   if(obs_lev.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_lev.n_elements()
           << " unique OBS_LEV values: "
           << write_css(obs_lev) << "\n";
   }

   // OBTYPE
   if(obtype.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obtype.n_elements()
           << " unique OBTYPE values: "
           << write_css(obtype) << "\n";
   }

   // VX_MASK
   if(vx_mask.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << vx_mask.n_elements()
           << " unique VX_MASK values: "
           << write_css(vx_mask) << "\n";
   }

   // INTERP_MTHD
   if(interp_mthd.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << interp_mthd.n_elements()
           << " unique INTERP_MTHD values: "
           << write_css(interp_mthd) << ".\n";
   }

   // INTERP_PNTS
   if(interp_pnts.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << interp_pnts.n_elements()
           << " unique INTERP_PNTS values: "
           << write_css(interp_pnts) << ".\n";
   }

   // FCST_THRESH
   if(fcst_thresh.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << fcst_thresh.n_elements()
           << " unique FCST_THRESH values: "
           << write_css(fcst_thresh) << "\n";
   }

   // OBS_THRESH
   if(obs_thresh.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << obs_thresh.n_elements()
           << " unique OBS_THRESH values: "
           << write_css(obs_thresh) << "\n";
   }

   // COV_THRESH
   if(cov_thresh.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << cov_thresh.n_elements()
           << " unique COV_THRESH values: "
           << write_css(cov_thresh) << ".\n";
   }

   // ALPHA
   if(alpha.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << alpha.n_elements()
           << " unique ALPHA values: "
           << write_css(alpha) << ".\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Use the StatHdrInfo struct to populate a StatHdrColumns object
//
////////////////////////////////////////////////////////////////////////

StatHdrColumns StatHdrInfo::get_shc(const ConcatString &cur_case,
                                    const StringArray  &case_cols,
                                    const StringArray  &hdr_cols,
                                    const StringArray  &hdr_vals,
                                    const STATLineType lt) {
   ConcatString css;
   StringArray case_vals;
   ThreshArray ta;
   double out_alpha;
   int index, wdth;
   StatHdrColumns shc;

   // Split up the current case into values
   case_vals = cur_case.split(":");

   // MODEL
   shc.set_model(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "MODEL", model, false).c_str());

   // DESC
   shc.set_desc(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "DESC", desc, false).c_str());

   // FCST_LEAD
   css = write_css_hhmmss(fcst_lead);
   if(fcst_lead.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_lead.n_elements()
           << " unique FCST_LEAD values: " << css << "\n";
   }
   if(hdr_cols.has("FCST_LEAD", index)) {
      shc.set_fcst_lead_sec(timestring_to_sec(hdr_vals[index].c_str()));
   }
   else {
      shc.set_fcst_lead_sec(fcst_lead.max());
   }

   // FCST_VALID_BEG, FCST_VALID_END
   if(hdr_cols.has("FCST_VALID_BEG", index)) {
      shc.set_fcst_valid_beg(timestring_to_unix(hdr_vals[index].c_str()));
   }
   else {
      shc.set_fcst_valid_beg(fcst_valid_beg);
   }
   if(hdr_cols.has("FCST_VALID_END", index)) {
      shc.set_fcst_valid_end(timestring_to_unix(hdr_vals[index].c_str()));
   }
   else {
      shc.set_fcst_valid_end(fcst_valid_end);
   }

   // OBS_LEAD
   css = write_css_hhmmss(obs_lead);
   if(obs_lead.n_elements() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_lead.n_elements()
           << " unique OBS_LEAD values: " << css << "\n";
   }
   if(hdr_cols.has("OBS_LEAD", index)) {
      shc.set_obs_lead_sec(timestring_to_sec(hdr_vals[index].c_str()));
   }
   else {
      shc.set_obs_lead_sec(obs_lead.max());
   }

   // OBS_VALID_BEG, OBS_VALID_END
   if(hdr_cols.has("OBS_VALID_BEG", index)) {
      shc.set_obs_valid_beg(timestring_to_unix(hdr_vals[index].c_str()));
   }
   else {
      shc.set_obs_valid_beg(obs_valid_beg);
   }
   if(hdr_cols.has("OBS_VALID_END", index)) {
      shc.set_obs_valid_end(timestring_to_unix(hdr_vals[index].c_str()));
   }
   else {
      shc.set_obs_valid_end(obs_valid_end);
   }

   // FCST_VAR
   shc.set_fcst_var(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "FCST_VAR", fcst_var, false));

   // FCST_UNITS
   shc.set_fcst_units(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "FCST_UNITS", fcst_units, false));

   // FCST_LEV
   shc.set_fcst_lev(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "FCST_LEV", fcst_lev, false).c_str());

   // OBS_VAR
   shc.set_obs_var(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "OBS_VAR", obs_var, false));

   // OBS_UNITS
   shc.set_obs_units(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "OBS_UNITS", obs_units, false));

   // OBS_LEV
   shc.set_obs_lev(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "OBS_LEV", obs_lev, false).c_str());

   // OBTYPE
   shc.set_obtype(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "OBTYPE", obtype, false).c_str());

   // VX_MASK
   shc.set_mask(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "VX_MASK", vx_mask, false).c_str());

   // INTERP_MTHD
   shc.set_interp_mthd(
      get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                  "INTERP_MTHD", interp_mthd, true));

   // INTERP_PNTS
   css = write_css(interp_pnts);
   if(interp_pnts.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << interp_pnts.n_elements()
           << " unique INTERP_PNTS values: " << css << ".\n";
      wdth = bad_data_int;
   }
   else {
      wdth = nint(sqrt(interp_pnts[0]));
   }

   if(hdr_cols.has("INTERP_PNTS", index)) {
      wdth = nint(sqrt(atof(hdr_vals[index].c_str())));
   }
   shc.set_interp_wdth(wdth);

   // FCST_THRESH
   ta.clear();
   ta.add_css(get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                          "FCST_THRESH", fcst_thresh, true).c_str());
   shc.set_fcst_thresh(ta);

   // OBS_THRESH
   ta.clear();
   ta.add_css(get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                          "OBS_THRESH", obs_thresh, true).c_str());
   shc.set_obs_thresh(ta);

   // COV_THRESH
   ta.clear();
   ta.add_css(get_shc_str(cur_case, case_cols, case_vals, hdr_cols, hdr_vals,
                          "COV_THRESH", cov_thresh, true).c_str());
   shc.set_cov_thresh(ta);

   // ALPHA
   css = write_css(alpha);
   if(alpha.n_elements() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << alpha.n_elements()
           << " unique ALPHA values: " << css << ".\n";
      out_alpha = bad_data_double;
   }
   else {
      out_alpha = alpha[0];
   }

   if(hdr_cols.has("ALPHA", index)) {
      out_alpha = atof(hdr_vals[index].c_str());
   }
   shc.set_alpha(out_alpha);

   // LINE_TYPE
   shc.set_line_type(statlinetype_to_string(lt));

   return(shc);
}

////////////////////////////////////////////////////////////////////////

ConcatString StatHdrInfo::get_shc_str(const ConcatString &cur_case,
                                      const StringArray  &case_cols,
                                      const StringArray  &case_vals,
                                      const StringArray  &hdr_cols,
                                      const StringArray  &hdr_vals,
                                      const char         *col_name,
                                      const StringArray  &col_vals,
                                      bool               warning) {
   ConcatString css, shc_str;
   int hdr_index, case_index;


   // Build comma-separated list of column values
   css = write_css(col_vals);

   // Check for multiple entries
   if(col_vals.n_elements() > 1) {
      ConcatString msg;
      msg << "For case \"" << cur_case << "\", found "
          << col_vals.n_elements() << " unique " << col_name
          << " values: " << css << "\n";
      if(warning) mlog << Warning  << msg;
      else        mlog << Debug(2) << msg;
   }

   // Check the header options.
   if(hdr_cols.has(col_name, hdr_index)) {

      // Check for the full CASE string.
      if(case_str.compare(hdr_vals[hdr_index]) == 0) {
         shc_str = cur_case;
      }
      // Check for one of the case columns.
      else if(case_cols.has(hdr_vals[hdr_index], case_index)) {
         shc_str = case_vals[case_index];
      }
      // Otherwise, use the constant header string.
      else {
         shc_str = hdr_vals[hdr_index];
      }
   }
   // Otherwise, use the comma-separated list of values.
   else {
      shc_str = css;
   }

   return(shc_str);
}

////////////////////////////////////////////////////////////////////////
//
// Code for AggrTimeSeriesInfo structure.
//
////////////////////////////////////////////////////////////////////////

AggrTimeSeriesInfo::AggrTimeSeriesInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void AggrTimeSeriesInfo::clear() {
   hdr.clear();
   fcst_var.clear();
   obs_var.clear();
   f_na.clear();
   o_na.clear();
   init_ts.clear();
   valid_ts.clear();
}

////////////////////////////////////////////////////////////////////////

void AggrTimeSeriesInfo::sort() {
   int j;
   AggrTimeSeriesInfo ri_sort;

   // Copy
   ri_sort.hdr      = hdr;
   ri_sort.fcst_var = fcst_var;
   ri_sort.obs_var  = obs_var;

   // Sort by valid time
   if(valid_ts.n_elements() == f_na.n_elements()) {

      ri_sort.valid_ts = valid_ts;
      ri_sort.valid_ts.sort_array();

      for(int i=0; i<valid_ts.n_elements(); i++) {
         j = valid_ts.index(ri_sort.valid_ts[i]);
         ri_sort.f_na.add(f_na[j]);
         ri_sort.o_na.add(o_na[j]);
         ri_sort.init_ts.add(init_ts[j]);
      }
   }
   // Sort by initialization time
   else if(init_ts.n_elements() == f_na.n_elements()) {

      ri_sort.init_ts = init_ts;
      ri_sort.init_ts.sort_array();
      ri_sort.valid_ts = valid_ts; // Single value

      for(int i=0; i<init_ts.n_elements(); i++) {
         j = init_ts.index(ri_sort.init_ts[i]);
         ri_sort.f_na.add(f_na[j]);
         ri_sort.o_na.add(o_na[j]);
      }
   }
   else {
      mlog << Warning << "\nAggrTimeSeriesInfo::sort() -> "
           << "can't sort when the number of times and the data values "
           << "differ.\n\n";
      return;
   }

   // Copy sorted version
   *this = ri_sort;

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_summary_lines(LineDataFile &f, STATAnalysisJob &j,
                        map<ConcatString, AggrSummaryInfo> &m,
                        int &n_in, int &n_out) {
   STATLine line;
   AggrSummaryInfo aggr;
   ConcatString key, cs;
   StringArray sa, req_stat, req_lty, req_col;
   STATLineType lty;
   NumArray empty_na;
   int i, n_add;
   double v, w;

   //
   // Objects for derived statistics
   //
   CTSInfo   cts_info;
   SL1L2Info sl1l2_info;
   CNTInfo   cnt_info;
   VL1L2Info vl1l2_info;

   //
   // Build list of requested line types and column names
   //
   for(i=0; i<j.column.n_elements(); i++) {

      // Split -column entry on colon
      sa = ConcatString(j.column[i]).split(":");

      // Option 1: Use -line_type once with no colons in -column option
      if(sa.n_elements() == 1 && j.line_type.n_elements() == 1) {
         cs << cs_erase << j.line_type[0] << ":" << sa[0];
         req_stat.add(cs);
         req_lty.add(j.line_type[0]);
         req_col.add(sa[0]);
      }
      // Option 2: Use colons in -column option
      else if(sa.n_elements() == 2) {
         req_stat.add(j.column[i]);
         req_lty.add(sa[0]);
         req_col.add(sa[1]);
      }
      else {
         mlog << Error << "\naggr_summary_lines() -> "
              << "trouble parsing \"-column " << j.column[i]
              << "\" option.  Either use \"-line_type\" exactly once "
              << "or use format \"-column LINE_TYPE:COLUMN\".\n\n";
         throw(1);
      }
   }

   //
   // When -column_union is true, store a list of unique line types and columns
   //
   if(j.column_union) {
      cs << cs_erase << write_css(req_lty.uniq()) << ":" << write_css(req_col.uniq());
      req_stat.set(0, cs);
   }

   //
   // Check for derived types:
   //   FHO, CTC      -> CTS
   //   SL1L2, SAL1L2 -> CNT
   //   VL1L2         -> VCNT
   //
   bool do_cts  = j.do_derive && req_lty.has(stat_cts_str);
   if(do_cts)  mlog << Debug(3)
                    << "Deriving CTS statistics from input FHO/CTC lines.\n";
   bool do_cnt  = j.do_derive && req_lty.has(stat_cnt_str);
   if(do_cnt)  mlog << Debug(3)
                    << "Deriving CNT statistics from input SL1L2/SAL1L2 lines.\n";
   bool do_vcnt = j.do_derive && req_lty.has(stat_vcnt_str);
   if(do_vcnt) mlog << Debug(3)
                    << "Deriving VCNT statistics from input VL1L2 lines.\n";

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.val.clear();
            aggr.wgt.clear();
            for(i=0; i<req_stat.n_elements(); i++) {
               aggr.val[req_stat[i]] = empty_na;
               aggr.wgt[req_stat[i]] = empty_na;
            }
            m[key] = aggr;
         }

         //
         // Derive categorical statistics
         //
         if(do_cts && line.type() == stat_fho) {
            parse_fho_ctable(line, cts_info.cts);
         }
         else if(do_cts && line.type() == stat_ctc) {
            parse_ctc_ctable(line, cts_info.cts);
         }

         //
         // Derive continuous statistics
         //
         else if(do_cnt && (line.type() == stat_sl1l2 ||
                            line.type() == stat_sal1l2)) {
            parse_sl1l2_line(line, sl1l2_info);
            compute_cntinfo(sl1l2_info, 0, cnt_info);
         }

         //
         // Derive vector continuous statistics
         //
         else if(do_vcnt && line.type() == stat_vl1l2) {
            parse_vl1l2_line(line, vl1l2_info);
         }

         //
         // Update the map entries for each requested statistic
         //
         for(i=0,n_add=0; i<req_col.n_elements(); i++) {

            //
            // Store the current requested line type
            //
            lty = string_to_statlinetype(req_lty[i].c_str());

            //
            // Retrieve the column value, checking dervied stats
            //
            if((line.type() == stat_fho ||
                line.type() == stat_ctc) && lty == stat_cts) {
               v = cts_info.get_stat(req_col[i].c_str());
               w = cts_info.cts.n();
            }
            else if(line.type() == stat_sl1l2 && lty == stat_cnt) {
               v = cnt_info.get_stat(req_col[i].c_str());
               w = cnt_info.n;
            }
            else if(line.type() == stat_sal1l2 && lty == stat_cnt) {
               v = cnt_info.get_stat(req_col[i].c_str());
               w = cnt_info.n;
            }
            else if(line.type() == stat_vl1l2 && lty == stat_vcnt) {
               v = vl1l2_info.get_stat(req_col[i].c_str());
               w = vl1l2_info.vcount;
            }
            else if(line.type() != lty) {
               continue;
            }
            else {
               v = j.get_column_double(line, req_col[i]);
               w = atoi(line.get_item("TOTAL"));
            }

            //
            // Check for bad data
            //
            if(is_bad_data(v)) continue;

            //
            // Add the current column value and weight
            // If computing a union, concatentate them all
            //
            if(j.column_union) {
               m[key].val[req_stat[0]].add(v);
               m[key].wgt[req_stat[0]].add(w);
            }
            else {
               m[key].val[req_stat[i]].add(v);
               m[key].wgt[req_stat[i]].add(w);
            }

            // Keep track of the unique header column entries
            m[key].hdr.add(line);

            n_add++;
         }

         //
         // Dump only the lines that were actually used
         //
         if(n_add > 0) {
            j.dump_stat_line(line);
            n_out++;
         }
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The aggr_ctc_lines routine should only be called when the
// -line_type option has been used exactly once.
//
////////////////////////////////////////////////////////////////////////

void aggr_ctc_lines(LineDataFile &f, STATAnalysisJob &j,
                    map<ConcatString, AggrCTCInfo> &m,
                    int &n_in, int &n_out) {
   STATLine line;
   AggrCTCInfo aggr;
   CTSInfo cur;
   ConcatString key;
   unixtime ut;
   int n, n_ties;
   map<ConcatString, AggrCTCInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         //
         // Zero out the contingency table object
         //
         cur.cts.zero_out();

         //
         // Switch on the line type looking only for contingency
         // table types of lines
         //
         switch(line.type()) {

            case(stat_fho):
               parse_fho_ctable(line, cur.cts);
               break;

            case(stat_ctc):
               parse_ctc_ctable(line, cur.cts);
               break;

            case(stat_nbrctc):
               parse_nbrctc_ctable(line, cur.cts);
               break;

            default:
               mlog << Error << "\naggr_ctc_lines() -> "
                    << "line type value of " << statlinetype_to_string(line.type())
                    << " not currently supported for the aggregation job.\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.valid_ts.clear();
            aggr.baser_ts.clear();
            aggr.fmean_ts.clear();
            aggr.acc_ts.clear();
            aggr.pody_ts.clear();
            aggr.podn_ts.clear();
            aggr.pofd_ts.clear();
            aggr.far_ts.clear();
            aggr.csi_ts.clear();
            aggr.hk_ts.clear();
            aggr.cts_info = cur;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment counts in the existing map entry
         //
         else {
            m[key].cts_info.cts.set_fy_oy(m[key].cts_info.cts.fy_oy() +
                                          cur.cts.fy_oy());
            m[key].cts_info.cts.set_fy_on(m[key].cts_info.cts.fy_on() +
                                          cur.cts.fy_on());
            m[key].cts_info.cts.set_fn_oy(m[key].cts_info.cts.fn_oy() +
                                          cur.cts.fn_oy());
            m[key].cts_info.cts.set_fn_on(m[key].cts_info.cts.fn_on() +
                                          cur.cts.fn_on());
         }

         //
         // Keep track of scores for each time step for VIF
         //
         if(j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = line.fcst_valid_beg();

            if(m[key].valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_ctc_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               m[key].valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            cur.compute_stats();

            //
            // Append the stats
            //
            m[key].baser_ts.add(cur.baser.v);
            m[key].fmean_ts.add(cur.fmean.v);
            m[key].acc_ts.add(cur.acc.v);
            m[key].pody_ts.add(cur.pody.v);
            m[key].podn_ts.add(cur.podn.v);
            m[key].pofd_ts.add(cur.pofd.v);
            m[key].far_ts.add(cur.far.v);
            m[key].csi_ts.add(cur.csi.v);
            m[key].hk_ts.add(cur.hk.v);
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   //
   // Loop over the map entries and adjust for VIF
   //
   for(it = m.begin(); it != m.end(); it++) {

      //
      // Check for the minimum length of time series
      //
      if(j.vif_flag && it->second.valid_ts.n_elements() < min_time_series) {
         mlog << Warning << "\naggr_ctc_lines() -> "
              << "the variance inflation factor adjustment can only "
              << "be computed for at least " << min_time_series
              << " unique valid times.\n\n";
         j.vif_flag = 0;
      }

      //
      // Compute the auto-correlations for VIF
      //
      if(j.vif_flag) {

         //
         // Sort the valid times
         //
         n = it->second.valid_ts.rank_array(n_ties);

         if(n_ties > 0 || n != it->second.valid_ts.n_elements()) {
            mlog << Error << "\naggr_ctc_lines() -> "
                 << "should be no ties in the valid time array.\n\n";
            throw(1);
         }

         //
         // Sort the stats into time order
         //
         it->second.baser_ts.reorder(it->second.valid_ts);
         it->second.fmean_ts.reorder(it->second.valid_ts);
         it->second.acc_ts.reorder(it->second.valid_ts);
         it->second.pody_ts.reorder(it->second.valid_ts);
         it->second.podn_ts.reorder(it->second.valid_ts);
         it->second.pofd_ts.reorder(it->second.valid_ts);
         it->second.far_ts.reorder(it->second.valid_ts);
         it->second.csi_ts.reorder(it->second.valid_ts);
         it->second.hk_ts.reorder(it->second.valid_ts);

         //
         // Compute the lag 1 autocorrelation
         //
         it->second.cts_info.baser.vif = compute_vif(it->second.baser_ts);
         it->second.cts_info.fmean.vif = compute_vif(it->second.fmean_ts);
         it->second.cts_info.acc.vif   = compute_vif(it->second.acc_ts);
         it->second.cts_info.pody.vif  = compute_vif(it->second.pody_ts);
         it->second.cts_info.podn.vif  = compute_vif(it->second.podn_ts);
         it->second.cts_info.pofd.vif  = compute_vif(it->second.pofd_ts);
         it->second.cts_info.far.vif   = compute_vif(it->second.far_ts);
         it->second.cts_info.csi.vif   = compute_vif(it->second.csi_ts);
         it->second.cts_info.hk.vif    = compute_vif(it->second.hk_ts);

      } // end if vif
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mctc_lines(LineDataFile &f, STATAnalysisJob &j,
                     map<ConcatString, AggrMCTCInfo> &m,
                     int &n_in, int &n_out) {
   STATLine line;
   AggrMCTCInfo aggr;
   MCTSInfo cur;
   ConcatString key;
   unixtime ut;
   int i, k, n, n_ties;
   map<ConcatString, AggrMCTCInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         //
         // Check for expected line type
         //
         if(line.type() != stat_mctc) {
            mlog << Error << "\naggr_mctc_lines() -> "
                 << "should only encounter multi-category contingency table count "
                 << "(MCTC) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current MCTC line
         //
         parse_mctc_ctable(line, cur.cts);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.valid_ts.clear();
            aggr.acc_ts.clear();
            aggr.mcts_info = cur;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment counts in the existing map entry
         //
         else {

            //
            // The size of the contingency table must remain the same
            //
            if(m[key].mcts_info.cts.nrows() != cur.cts.nrows()) {
               mlog << Error << "\naggr_mctc_lines() -> "
                    << "when aggregating MCTC lines the size of the "
                    << "contingency table must remain the same for all "
                    << "lines.  Try setting \"-column_eq N_CAT n\", "
                    << m[key].mcts_info.cts.nrows() << " != "
                    << cur.cts.nrows() << "\n\n";
               throw(1);
            }

            //
            // Increment the counts
            //
            for(i=0; i<m[key].mcts_info.cts.nrows(); i++) {
               for(k=0; k<m[key].mcts_info.cts.ncols(); k++) {
                  m[key].mcts_info.cts.set_entry(i, k,
                                                 m[key].mcts_info.cts.entry(i, k) +
                                                 cur.cts.entry(i, k));
               } //end for k
            } // end for i
         } // end else

         //
         // Keep track of scores for each time step for VIF
         //
         if(j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = line.fcst_valid_beg();

            if(m[key].valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_mctc_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               m[key].valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            cur.compute_stats();

            //
            // Append the stats
            //
            m[key].acc_ts.add(cur.acc.v);
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   //
   // Loop over the map entries and adjust for VIF
   //
   for(it = m.begin(); it != m.end(); it++) {

      //
      // Check for the minimum length of time series
      //
      if(j.vif_flag && it->second.valid_ts.n_elements() < min_time_series) {
         mlog << Warning << "\naggr_mctc_lines() -> "
              << "the variance inflation factor adjustment can only "
              << "be computed for at least " << min_time_series
              << " unique valid times.\n\n";
         j.vif_flag = 0;
      }

      //
      // Compute the auto-correlations for VIF
      //
      if(j.vif_flag) {

         //
         // Sort the valid times
         //
         n = it->second.valid_ts.rank_array(n_ties);

         if(n_ties > 0 || n != it->second.valid_ts.n_elements()) {
            mlog << Error << "\naggr_mctc_lines() -> "
                 << "should be no ties in the valid time array.\n\n";
            throw(1);
         }

         //
         // Sort the stats into time order
         //
         it->second.acc_ts.reorder(it->second.valid_ts);

         //
         // Compute the lag 1 autocorrelation
         //
         it->second.mcts_info.acc.vif = compute_vif(it->second.acc_ts);

      } // end if vif
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_pct_lines(LineDataFile &f, STATAnalysisJob &j,
                    map<ConcatString, AggrPCTInfo> &m,
                    int &n_in, int &n_out) {
   STATLine line;
   AggrPCTInfo aggr;
   PCTInfo cur;
   ConcatString key;
   unixtime ut;
   int i, n, oy, on, n_ties;
   map<ConcatString, AggrPCTInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         //
         // Check for expected line type
         //
         if(line.type() != stat_pct) {
            mlog << Error << "\naggr_pct_lines() -> "
                 << "should only encounter probability contingency table (PCT) "
                 << "line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current PCT line
         //
         parse_nx2_ctable(line, cur.pct);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.valid_ts.clear();
            aggr.baser_ts.clear();
            aggr.brier_ts.clear();
            aggr.pct_info = cur;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment counts in the existing map entry
         //
         else {

            //
            // The size of the contingency table must remain the same
            //
            if(m[key].pct_info.pct.nrows() != cur.pct.nrows()) {
               mlog << Error << "\naggr_pct_lines() -> "
                    << "when aggregating PCT lines the number of "
                    << "thresholds must remain the same for all lines, "
                    << m[key].pct_info.pct.nrows() << " != "
                    << cur.pct.nrows() << "\n\n";
               throw(1);
            }

            //
            // Increment the counts
            //
            for(i=0; i<m[key].pct_info.pct.nrows(); i++) {

               //
               // The threshold values must remain the same
               //
               if(!is_eq(m[key].pct_info.pct.threshold(i), cur.pct.threshold(i))) {
                  mlog << Error << "\naggr_pct_lines() -> "
                       << "when aggregating PCT lines the threshold "
                       << "values must remain the same for all lines, "
                       << m[key].pct_info.pct.threshold(i) << " != "
                       << cur.pct.threshold(i) << "\n\n";
                  throw(1);
               }

               oy = m[key].pct_info.pct.event_count_by_row(i);
               on = m[key].pct_info.pct.nonevent_count_by_row(i);

               m[key].pct_info.pct.set_entry(i, nx2_event_column,
                                             oy + cur.pct.event_count_by_row(i));
               m[key].pct_info.pct.set_entry(i, nx2_nonevent_column,
                                             on + cur.pct.nonevent_count_by_row(i));
            } // end for i
         } // end else

         //
         // Keep track of scores for each time step for VIF
         //
         if(j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = line.fcst_valid_beg();

            if(m[key].valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_pct_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               m[key].valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            cur.compute_stats();

            //
            // Append the stats
            //
            m[key].baser_ts.add(cur.baser.v);
            m[key].brier_ts.add(cur.brier.v);
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   //
   // Loop over the map entries and adjust for VIF
   //
   for(it = m.begin(); it != m.end(); it++) {

      //
      // Check for the minimum length of time series
      //
      if(j.vif_flag && it->second.valid_ts.n_elements() < min_time_series) {
         mlog << Warning << "\naggr_pct_lines() -> "
              << "the variance inflation factor adjustment can only "
              << "be computed for at least " << min_time_series
              << " unique valid times.\n\n";
         j.vif_flag = 0;
      }

      //
      // Compute the auto-correlations for VIF
      //
      if(j.vif_flag) {

         //
         // Sort the valid times
         //
         n = it->second.valid_ts.rank_array(n_ties);

         if(n_ties > 0 || n != it->second.valid_ts.n_elements()) {
            mlog << Error << "\naggr_pct_lines() -> "
                 << "should be no ties in the valid time array.\n\n";
            throw(1);
         }

         //
         // Sort the stats into time order
         //
         it->second.baser_ts.reorder(it->second.valid_ts);
         it->second.brier_ts.reorder(it->second.valid_ts);

         //
         // Compute the lag 1 autocorrelation
         //
         it->second.pct_info.baser.vif = compute_vif(it->second.baser_ts);
         it->second.pct_info.brier.vif = compute_vif(it->second.brier_ts);

      } // end if vif
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_psum_lines(LineDataFile &f, STATAnalysisJob &j,
                     map<ConcatString, AggrPSumInfo> &m,
                     int &n_in, int &n_out) {
   STATLine line;
   AggrPSumInfo aggr;
   SL1L2Info cur_sl1l2;
   VL1L2Info cur_vl1l2;
   NBRCNTInfo cur_nbrcnt;
   CNTInfo cur_cnt;
   ConcatString key;
   unixtime ut;
   int n, n_ties;
   map<ConcatString, AggrPSumInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         //
         // Initialize
         //
         cur_sl1l2.clear();
         cur_vl1l2.clear();
         cur_nbrcnt.clear();

         //
         // Switch on the line type.
         // For each partial sum line type, clear out the object,
         // parse the new line, and add it to running sum.
         //
         switch(line.type()) {

            case(stat_sl1l2):
               parse_sl1l2_line(line, cur_sl1l2);
               break;

            case(stat_sal1l2):
               parse_sal1l2_line(line, cur_sl1l2);
               break;

            case(stat_vl1l2):
               parse_vl1l2_line(line, cur_vl1l2);
               break;

            case(stat_val1l2):
               parse_val1l2_line(line, cur_vl1l2);
               break;

            case(stat_nbrcnt):
               parse_nbrcnt_line(line, cur_nbrcnt);
               break;

            default:
               mlog << Error << "\naggr_psum_lines() -> "
                    << "should only encounter partial sum line types.\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.valid_ts.clear();
            aggr.fbar_ts.clear();
            aggr.obar_ts.clear();
            aggr.me_ts.clear();
            aggr.sl1l2_info  = cur_sl1l2;
            aggr.vl1l2_info  = cur_vl1l2;
            aggr.nbrcnt_info = cur_nbrcnt;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment sums in the existing map entry
         //
         else {

            m[key].sl1l2_info  += cur_sl1l2;
            m[key].vl1l2_info  += cur_vl1l2;
            m[key].nbrcnt_info += cur_nbrcnt;
         }

         //
         // Keep track of scores for each time step for VIF
         //
         if(line.type() == stat_sl1l2 && j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = line.fcst_valid_beg();

            if(m[key].valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_psum_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               m[key].valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            compute_cntinfo(cur_sl1l2, 0, cur_cnt);

            //
            // Append the stats
            //
            m[key].fbar_ts.add(cur_cnt.fbar.v);
            m[key].obar_ts.add(cur_cnt.obar.v);
            m[key].me_ts.add(cur_cnt.me.v);
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   //
   // Loop over the map entries and adjust for VIF
   //
   for(it = m.begin(); it != m.end(); it++) {

      //
      // Check for the minimum length of time series
      //
      if(j.vif_flag && j.out_line_type.has(stat_cnt_str) &&
         it->second.valid_ts.n_elements() < min_time_series) {
         mlog << Warning << "\naggr_psum_lines() -> "
              << "the variance inflation factor adjustment can only "
              << "be computed for at least " << min_time_series
              << " unique valid times.\n\n";
         j.vif_flag = 0;
      }

      //
      // Compute the auto-correlations for VIF
      //
      if(j.vif_flag && j.out_line_type.has(stat_cnt_str)) {

         //
         // Sort the valid times
         //
         n = it->second.valid_ts.rank_array(n_ties);

         if(n_ties > 0 || n != it->second.valid_ts.n_elements()) {
            mlog << Error << "\naggr_psum_lines() -> "
                 << "should be no ties in the valid time array.\n\n";
            throw(1);
         }

         //
         // Sort the stats into time order
         //
         it->second.fbar_ts.reorder(it->second.valid_ts);
         it->second.obar_ts.reorder(it->second.valid_ts);
         it->second.me_ts.reorder(it->second.valid_ts);

         //
         // Compute the lag 1 autocorrelation
         //
         it->second.cnt_info.fbar.vif = compute_vif(it->second.fbar_ts);
         it->second.cnt_info.obar.vif = compute_vif(it->second.obar_ts);
         it->second.cnt_info.me.vif   = compute_vif(it->second.me_ts);

      } // end if vif
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_grad_lines(LineDataFile &f, STATAnalysisJob &j,
                      map<ConcatString, AggrGRADInfo> &m,
                      int &n_in, int &n_out) {
   STATLine line;
   AggrGRADInfo aggr;
   GRADInfo cur;
   ConcatString key;
   map<ConcatString, AggrENSInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_grad) {
            mlog << Error << "\naggr_grad_lines() -> "
                 << "should only encounter gradient (GRAD) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current PHIST line
         //
         parse_grad_line(line, cur);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.grad_info = cur;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment counts in the existing map entry
         //
         else {

            //
            // Check for DY and DY remaining constant
            //
            if(m[key].grad_info.dx != cur.dx ||
               m[key].grad_info.dy != cur.dy) {
               mlog << Error << "\naggr_grad_lines() -> "
                    << "the \"DX\" and \"DY\" columns must remain constant ("
                    << m[key].grad_info.dx << " and " << m[key].grad_info.dy
                    << " != " << cur.dx << " and " << cur.dy
                    << ").  Try setting \"-column_eq DX n -column_eq DY n\""
                    << " or \"-by DX,DY\".\n\n";
               throw(1);
            }

            //
            // Aggregate the GRAD partial sums
            //
            m[key].grad_info += cur;

         } // end else

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_wind_lines(LineDataFile &f, STATAnalysisJob &j,
                     map<ConcatString, AggrWindInfo> &m,
                     int &n_in, int &n_out) {
   STATLine line;
   AggrWindInfo aggr;
   VL1L2Info cur;
   ConcatString key;
   double uf, vf, uo, vo;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         //
         // Initialize
         //
         cur.clear();

         //
         // Switch on the line type.
         // For each partial sum line type, clear out the object,
         // parse the new line and convert to unit vectors.
         //
         switch(line.type()) {

            case(stat_vl1l2):
               parse_vl1l2_line(line, cur);
               convert_u_v_to_unit(cur.uf_bar, cur.vf_bar, uf, vf);
               convert_u_v_to_unit(cur.uo_bar, cur.vo_bar, uo, vo);
               break;

            case(stat_val1l2):
               parse_val1l2_line(line, cur);
               convert_u_v_to_unit(cur.ufa_bar, cur.vfa_bar, uf, vf);
               convert_u_v_to_unit(cur.uoa_bar, cur.voa_bar, uo, vo);
               break;

            default:
               mlog << Error << "\naggr_wind_lines() -> "
                    << "should only encounter vector partial sum line types.\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.uf_na.clear();
            aggr.vf_na.clear();
            aggr.uo_na.clear();
            aggr.vo_na.clear();
            aggr.vl1l2_info = cur;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment sums in the existing map entry
         //
         else {
            m[key].vl1l2_info += cur;
         }

         //
         // Append the unit vectors
         //
         m[key].uf_na.add(uf);
         m[key].vf_na.add(vf);
         m[key].uo_na.add(uo);
         m[key].vo_na.add(vo);

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_wind_lines(LineDataFile &f, STATAnalysisJob &j,
                         map<ConcatString, AggrWindInfo> &m,
                         int &n_in, int &n_out) {
   STATLine line;
   AggrWindInfo aggr;
   VL1L2Info v_info;
   MPRData cur;
   ConcatString hdr, key;
   double uf, vf, uo, vo, fwind, owind;
   bool is_ugrd;
   int i;
   map<ConcatString, AggrWindInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         parse_mpr_line(line, cur);
         is_ugrd = (cur.fcst_var == ugrd_abbr_str);
         uf      = (is_ugrd ? cur.fcst        : bad_data_double);
         uo      = (is_ugrd ? cur.obs         : bad_data_double);
         vf      = (is_ugrd ? bad_data_double : cur.fcst);
         vo      = (is_ugrd ? bad_data_double : cur.obs);

         //
         // Build header string for matching UGRD and VGRD lines
         //
         hdr << cs_erase
             << line.model() << ":"
             << line.desc() << ":"
             << sec_to_hhmmss(line.fcst_lead()) << ":"
             << unix_to_yyyymmdd_hhmmss(line.fcst_valid_beg()) << ":"
             << unix_to_yyyymmdd_hhmmss(line.fcst_valid_end()) << ":"
             << sec_to_hhmmss(line.obs_lead()) << ":"
             << unix_to_yyyymmdd_hhmmss(line.obs_valid_beg()) << ":"
             << unix_to_yyyymmdd_hhmmss(line.obs_valid_end()) << ":"
             << line.fcst_lev() << ":"
             << line.obs_lev() << ":"
             << line.obtype() << ":"
             << line.vx_mask() << ":"
             << line.interp_mthd() << ":"
             << line.interp_pnts() << ":"
             << cur.obs_sid << ":"
             << cur.obs_lat << ":"
             << cur.obs_lon << ":"
             << cur.obs_lvl << ":"
             << cur.obs_elv;

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {

            //
            // Clear contents
            //
            aggr.vl1l2_info.clear();
            aggr.hdr_sa.clear();
            aggr.uf_na.clear();
            aggr.vf_na.clear();
            aggr.uo_na.clear();
            aggr.vo_na.clear();

            //
            // Initialize values
            //
            aggr.hdr_sa.add(hdr);
            aggr.uf_na.add(uf);
            aggr.vf_na.add(vf);
            aggr.uo_na.add(uo);
            aggr.vo_na.add(vo);

            //
            // Add the new map entry
            //
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Otherwise, add data to existing map entry
         //
         else {

            //
            // Add data for existing header entry
            //
            if(m[key].hdr_sa.has(hdr, i)) {

               //
               // Check for duplicate UGRD lines
               //
               if((!is_bad_data(uf) && !is_bad_data(m[key].uf_na[i])) ||
                  (!is_bad_data(uo) && !is_bad_data(m[key].uo_na[i]))) {
                  mlog << Warning << "\naggr_mpr_wind_lines() -> "
                       << "found duplicate UGRD lines for header:\n"
                       << hdr << "\n\n";
               }

               //
               // Check for duplicate VGRD lines
               //
               if((!is_bad_data(vf) && !is_bad_data(m[key].vf_na[i])) ||
                  (!is_bad_data(vo) && !is_bad_data(m[key].vo_na[i]))) {
                  mlog << Warning << "\naggr_mpr_wind_lines() -> "
                       << "found duplicate VGRD lines for header:\n"
                       << hdr << "\n\n";
               }

               //
               // Update the existing values
               //
               if(!is_bad_data(uf)) m[key].uf_na.set(i, uf);
               if(!is_bad_data(vf)) m[key].vf_na.set(i, vf);
               if(!is_bad_data(uo)) m[key].uo_na.set(i, uo);
               if(!is_bad_data(vo)) m[key].vo_na.set(i, vo);
            }
            //
            // Add data for a new header entry
            //
            else {
               m[key].hdr_sa.add(hdr);
               m[key].uf_na.add(uf);
               m[key].vf_na.add(vf);
               m[key].uo_na.add(uo);
               m[key].vo_na.add(vo);
            }
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   //
   // Loop over the map entries, discarding orphans and
   // applyling the wind speed thresholds.
   //
   for(it = m.begin(); it != m.end(); it++) {

      //
      // Initialize
      //
      aggr.hdr = it->second.hdr;
      aggr.vl1l2_info.clear();
      aggr.hdr_sa.clear();
      aggr.uf_na.clear();
      aggr.vf_na.clear();
      aggr.uo_na.clear();
      aggr.vo_na.clear();

      //
      // Loop over the pairs for the current map entry
      //
      for(i=0; i<it->second.hdr_sa.n_elements(); i++) {

         //
         // Check for missing UGRD data
         //
         if(is_bad_data(it->second.uf_na[i]) ||
            is_bad_data(it->second.uo_na[i])) {
            mlog << Warning << "\naggr_mpr_wind_lines() -> "
                 << "could not find matching UGRD line for header:\n"
                 << it->second.hdr_sa[i] << "\n\n";
            continue;
         }

         //
         // Check for missing VGRD data
         //
         if(is_bad_data(it->second.vf_na[i]) ||
            is_bad_data(it->second.vo_na[i])) {
            mlog << Warning << "\naggr_mpr_wind_lines() -> "
                 << "could not find matching VGRD line for header:\n"
                 << it->second.hdr_sa[i] << "\n\n";
            continue;
         }

         //
         // Apply the wind speed thresholds and logic
         //
         if(j.out_fcst_wind_thresh.get_type() != thresh_na ||
            j.out_obs_wind_thresh.get_type()  != thresh_na) {

            // Compute wind speeds
            fwind = convert_u_v_to_wind(it->second.uf_na[i], it->second.vf_na[i]);
            owind = convert_u_v_to_wind(it->second.uo_na[i], it->second.vo_na[i]);

            if(!check_fo_thresh(fwind, j.out_fcst_wind_thresh,
                                owind, j.out_obs_wind_thresh,
                                j.out_wind_logic)) {
               mlog << Debug(4) << "aggr_mpr_wind_lines() -> "
                    << "skipping vector forecast ("
                    << it->second.uf_na[i] << ", " << it->second.vf_na[i]
                    << ") wind speed threshold (" << fwind << " "
                    << j.out_fcst_wind_thresh.get_str()
                    << ") and vector observation ("
                    << it->second.uo_na[i] << ", " << it->second.vo_na[i]
                    << ") wind speed threshold (" << owind << " "
                    << j.out_obs_wind_thresh.get_str()
                    << ") with " << setlogic_to_string(j.out_wind_logic)
                    << " logic for header:\n"
                    << it->second.hdr_sa[i] << "\n";
               continue;
            }
         }

         //
         // Keep running partial sums of matches
         //
         v_info.vcount      = 1;
         v_info.uf_bar      = it->second.uf_na[i];
         v_info.vf_bar      = it->second.vf_na[i];
         v_info.uo_bar      = it->second.uo_na[i];
         v_info.vo_bar      = it->second.vo_na[i];
         v_info.uvfo_bar    = it->second.uf_na[i]*it->second.uo_na[i] +
                              it->second.vf_na[i]*it->second.vo_na[i];
         v_info.uvff_bar    = it->second.uf_na[i]*it->second.uf_na[i] +
                              it->second.vf_na[i]*it->second.vf_na[i];
         v_info.uvoo_bar    = it->second.uo_na[i]*it->second.uo_na[i] +
                              it->second.vo_na[i]*it->second.vo_na[i];
         v_info.f_speed_bar = sqrt(it->second.uf_na[i]*it->second.uf_na[i] +
                                   it->second.vf_na[i]*it->second.vf_na[i]);
         v_info.o_speed_bar = sqrt(it->second.uo_na[i]*it->second.uo_na[i] +
                                   it->second.vo_na[i]*it->second.vo_na[i]);
         aggr.vl1l2_info += v_info;

         //
         // Check for vectors of length zero
         //
         if((is_eq(it->second.uf_na[i], 0.0) &&
             is_eq(it->second.vf_na[i], 0.0)) ||
            (is_eq(it->second.uo_na[i], 0.0) &&
             is_eq(it->second.vo_na[i], 0.0))) {
            mlog << Debug(4) << "aggr_mpr_wind_lines() -> "
                 << "angle not defined for zero forecast ("
                 << it->second.uf_na[i] << ", " << it->second.vf_na[i]
                 << ") or observation ("
                 << it->second.uo_na[i] << ", " << it->second.vo_na[i]
                 << ") vector for header:\n"
                 << it->second.hdr_sa[i] << "\n";
            continue;
         }

         //
         // Convert to and append unit vectors
         //
         aggr.hdr_sa.add(it->second.hdr_sa[i]);
         convert_u_v_to_unit(it->second.uf_na[i], it->second.vf_na[i], uf, vf);
         convert_u_v_to_unit(it->second.uo_na[i], it->second.vo_na[i], uo, vo);
         aggr.uf_na.add(uf);
         aggr.vf_na.add(vf);
         aggr.uo_na.add(uo);
         aggr.vo_na.add(vo);
      }

      //
      // Reset the map entry
      //
      it->second = aggr;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines(LineDataFile &f, STATAnalysisJob &j,
                    map<ConcatString, AggrMPRInfo> &m,
                    int &n_in, int &n_out) {
   STATLine line;
   AggrMPRInfo aggr;
   MPRData cur;
   ConcatString key;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         //
         // Check for expected line type
         //
         if(line.type() != stat_mpr) {
            mlog << Error << "\naggr_mpr_lines() -> "
                 << "should only encounter matched pair (MPR) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current MPR line
         //
         parse_mpr_line(line, cur);

         //
         // Check for bad data
         //
         if(is_bad_data(cur.fcst) || is_bad_data(cur.obs)) continue;

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Apply the continuous statistics filtering threshold
         //
         if((j.out_line_type.has(stat_cnt_str) ||
             j.out_line_type.has(stat_sl1l2_str)) &&
            (j.out_fcst_thresh.n_elements() > 0 ||
             j.out_obs_thresh.n_elements()  > 0)) {

            SingleThresh fst, ost;
            if(j.out_fcst_thresh.n_elements() > 0) fst = j.out_fcst_thresh[0];
            if(j.out_obs_thresh.n_elements()  > 0) ost = j.out_obs_thresh[0];

            if(!check_fo_thresh(cur.fcst, fst, cur.obs, ost, j.out_cnt_logic)) {
               mlog << Debug(4) << "aggr_mpr_lines() -> "
                    << "skipping forecast ("
                    << cur.fcst << " " << j.out_fcst_thresh.get_str()
                    << ") and observation ("
                    << cur.obs << " " << j.out_obs_thresh.get_str()
                    << ") matched pair with "
                    << setlogic_to_string(j.out_cnt_logic)
                    << " logic.\n";
               continue;
            }
         }

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.f_na.clear();
            aggr.o_na.clear();
            aggr.cmn_na.clear();
            aggr.csd_na.clear();
            aggr.cdf_na.clear();
            aggr.f_na.add(cur.fcst);
            aggr.o_na.add(cur.obs);
            aggr.cmn_na.add(cur.climo_mean);
            aggr.csd_na.add(cur.climo_stdev);
            aggr.cdf_na.add(cur.climo_cdf);
            aggr.fcst_var = cur.fcst_var;
            aggr.obs_var = cur.obs_var;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment sums in the existing map entry
         //
         else {

            m[key].f_na.add(cur.fcst);
            m[key].o_na.add(cur.obs);
            m[key].cmn_na.add(cur.climo_mean);
            m[key].csd_na.add(cur.climo_stdev);
            m[key].cdf_na.add(cur.climo_cdf);

            //
            // Only aggregate consistent variable names
            //
            if(m[key].fcst_var != cur.fcst_var ||
               m[key].obs_var  != cur.obs_var) {
               mlog << Error << "\nread_mpr_lines() -> "
                    << "both the forecast and observation variable types must "
                    << "remain constant.  Try setting \"-fcst_var\" and/or "
                    << "\"-obs_var\".\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
            }
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_isc_lines(LineDataFile &ldf, STATAnalysisJob &j,
                    map<ConcatString, AggrISCInfo> &m,
                    int &n_in, int &n_out) {
   STATLine line;
   AggrISCInfo aggr;
   ISCInfo cur;
   ConcatString key;
   int i, k, iscale;
   double total, w, den, baser_fbias_sum;
   map<ConcatString, AggrISCInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(ldf >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_isc) {
            mlog << Error << "\naggr_isc_lines() -> "
                 << "should only encounter intensity-scale "
                 << "(ISC) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current ISC line
         //
         parse_isc_line(line, cur, iscale);

         //
         // Check for bad data
         //
         if(is_bad_data(cur.total) || is_bad_data(cur.mse) ||
            is_bad_data(cur.fen)   || is_bad_data(cur.oen) ||
            is_bad_data(cur.baser) || is_bad_data(cur.fbias)) continue;

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.isc_info.clear();
            aggr.total_na = aggr.mse_na   = aggr.fen_na   = (NumArray *) 0;
            aggr.oen_na   = aggr.baser_na = aggr.fbias_na = (NumArray *) 0;
            aggr.hdr.clear();
            m[key] = aggr;
         }

         //
         // After reading the first ISC line, setup the isc_info
         // object to store the data.  Also, store the number
         // of scales and make sure that it doesn't change.
         //
         if(m[key].isc_info.n_scale == 0) {

            // Allocate room to store results for each scale
            m[key].isc_info.allocate_n_scale(cur.n_scale);
            m[key].isc_info.zero_out();

            //
            // Initialize tile_dim, tile_xll, and tile_yll.
            // If they stay the same over all the lines, write them out.
            // Otherwise, write out bad data.
            //
            m[key].isc_info.tile_dim = cur.tile_dim;
            m[key].isc_info.tile_xll = cur.tile_xll;
            m[key].isc_info.tile_yll = cur.tile_yll;

            // Allocate room to store values for each scale
            m[key].total_na = new NumArray [m[key].isc_info.n_scale + 2];
            m[key].mse_na   = new NumArray [m[key].isc_info.n_scale + 2];
            m[key].fen_na   = new NumArray [m[key].isc_info.n_scale + 2];
            m[key].oen_na   = new NumArray [m[key].isc_info.n_scale + 2];
            m[key].baser_na = new NumArray [m[key].isc_info.n_scale + 2];
            m[key].fbias_na = new NumArray [m[key].isc_info.n_scale + 2];
         }

         //
         // Check that the number of scales remains constant
         //
         if(m[key].isc_info.n_scale != cur.n_scale) {
            mlog << Error << "\naggr_isc_lines() -> "
                 << "the number of scales must remain constant "
                 << "when aggregating ISC lines.  Use the "
                 << "\"-column_min NSCALE n\" and "
                 << "\"-column_max NSCALE n\" options to "
                 << "filter out only those lines you'd like "
                 << "to aggregate.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Check to see if tile_dim, tile_xll, or tile_yll has changed.
         // If so, write out bad data.
         //
         if(m[key].isc_info.tile_dim != bad_data_int &&
            m[key].isc_info.tile_dim != cur.tile_dim) {
            m[key].isc_info.tile_dim = bad_data_int;
         }
         if( (m[key].isc_info.tile_xll != bad_data_int &&
              m[key].isc_info.tile_xll != cur.tile_xll) ||
             (m[key].isc_info.tile_yll != bad_data_int &&
              m[key].isc_info.tile_yll != cur.tile_yll) ) {
            m[key].isc_info.tile_xll = bad_data_int;
            m[key].isc_info.tile_yll = bad_data_int;
         }

         //
         // Store the data for this ISC line
         //
         m[key].total_na[iscale].add(cur.total);
         m[key].mse_na[iscale].add(cur.mse);
         m[key].fen_na[iscale].add(cur.fen);
         m[key].oen_na[iscale].add(cur.oen);
         m[key].baser_na[iscale].add(cur.baser);
         m[key].fbias_na[iscale].add(cur.fbias);

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   //
   // Return if no lines were read
   //
   if(n_out == 0) return;

   //
   // Loop over the map entries and compute weighted scores
   //
   for(it = m.begin(); it != m.end(); it++) {

      //
      // Get the sum of the totals, compute the weight, and sum the
      // weighted scores
      //
      for(i=0; i<it->second.isc_info.n_scale+2; i++) {

         // Total number of points for this scale
         total = it->second.total_na[i].sum();

         // Initialize
         baser_fbias_sum = 0.0;

         // Loop through all scores for this scale
         for(k=0; k<it->second.total_na[i].n_elements(); k++) {

            // Compute the weight for each score to be aggregated
            // based on the number of points it represents
            w = it->second.total_na[i][k]/total;

            // Sum scores for the binary fields
            if(i == 0) {
               it->second.isc_info.mse    += w*it->second.mse_na[0][k];
               it->second.isc_info.fen    += w*it->second.fen_na[0][k];
               it->second.isc_info.oen    += w*it->second.oen_na[0][k];
               it->second.isc_info.baser  += w*it->second.baser_na[0][k];
               baser_fbias_sum += w*it->second.baser_na[0][k]*it->second.fbias_na[0][k];
            }
            // Weighted sum of scores for each scale
            else {
               it->second.isc_info.mse_scale[i-1] += w*it->second.mse_na[i][k];
               it->second.isc_info.fen_scale[i-1] += w*it->second.fen_na[i][k];
               it->second.isc_info.oen_scale[i-1] += w*it->second.oen_na[i][k];
            }
         }

         //
         // Compute the aggregated scores for the binary fields
         //
         if(i == 0) {

            // Total
            it->second.isc_info.total = nint(it->second.total_na[0].sum());

            // Aggregated FBIAS
            it->second.isc_info.fbias = baser_fbias_sum/it->second.isc_info.baser;

            // Compute the aggregated ISC score.  For the binary fields
            // do not divide by the number of scales.
            den = (it->second.isc_info.fbias*
                   it->second.isc_info.baser*
                   (1.0 - it->second.isc_info.baser) +
                   it->second.isc_info.baser*
                   (1.0 - it->second.isc_info.fbias*
                   it->second.isc_info.baser));

            if(is_bad_data(it->second.isc_info.fbias) ||
               is_bad_data(it->second.isc_info.baser) ||
               is_eq(den, 0.0)) {
               it->second.isc_info.isc = bad_data_double;
            }
            else {
               it->second.isc_info.isc = 1.0 - it->second.isc_info.mse/den;
            }
         }
         //
         // Compute the aggregated scores for each scale
         //
         else {

            // Compute the aggregated ISC score.  For each scale, divide
            // by the number of scales.
            den = (it->second.isc_info.fbias*
                   it->second.isc_info.baser*
                   (1.0 - it->second.isc_info.baser) +
                   it->second.isc_info.baser*
                   (1.0 - it->second.isc_info.fbias*
                   it->second.isc_info.baser))
                  /(it->second.isc_info.n_scale+1);

            if(is_bad_data(it->second.isc_info.fbias) ||
               is_bad_data(it->second.isc_info.baser) ||
               is_eq(den, 0.0)) {
               it->second.isc_info.isc_scale[i-1] = bad_data_double;
            }
            else {
              it->second.isc_info.isc_scale[i-1] = 1.0 - it->second.isc_info.mse_scale[i-1]/den;
            }
         }
      } // end for i

      //
      // Deallocate memory
      //
      if(it->second.total_na) { delete [] it->second.total_na; it->second.total_na = (NumArray *) 0; }
      if(it->second.mse_na  ) { delete [] it->second.mse_na;   it->second.mse_na   = (NumArray *) 0; }
      if(it->second.fen_na  ) { delete [] it->second.fen_na;   it->second.fen_na   = (NumArray *) 0; }
      if(it->second.oen_na  ) { delete [] it->second.oen_na;   it->second.oen_na   = (NumArray *) 0; }
      if(it->second.baser_na) { delete [] it->second.baser_na; it->second.baser_na = (NumArray *) 0; }
      if(it->second.fbias_na) { delete [] it->second.fbias_na; it->second.fbias_na = (NumArray *) 0; }

   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_ecnt_lines(LineDataFile &f, STATAnalysisJob &j,
                     map<ConcatString, AggrENSInfo> &m,
                     int &n_in, int &n_out) {

   STATLine line;
   AggrENSInfo aggr;
   ECNTData cur;
   ConcatString key;
   double crps_fcst, crps_climo, v;
   map<ConcatString, AggrENSInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_ecnt) {
            mlog << Error << "\naggr_ecnt_lines() -> "
                 << "should only encounter ranked histogram "
                 << "(ECNT) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current RHIST line
         //
         parse_ecnt_line(line, cur);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.ens_pd.clear();
            aggr.crps_climo_na.clear();
            aggr.hdr.clear();
            m[key] = aggr;
         }

         //
         // Check for N_ENS remaining constant
         //
         if(m[key].ens_pd.n_ens == 0) m[key].ens_pd.set_ens_size(cur.n_ens);
         else if(m[key].ens_pd.n_ens != cur.n_ens) {
            mlog << Warning << "\naggr_ecnt_lines() -> "
                 << "the \"N_ENS\" column changed from " << m[key].ens_pd.n_ens
                 << " to " << cur.n_ens << ".\n\n";
         }

         //
         // Store the current statistics and weight (TOTAL column)
         //
         m[key].ens_pd.crps_na.add(cur.crps);
         m[key].ens_pd.ign_na.add(cur.ign);
         m[key].ens_pd.spread_na.add(cur.spread);
         m[key].ens_pd.spread_oerr_na.add(cur.spread_oerr);
         m[key].ens_pd.spread_plus_oerr_na.add(cur.spread_plus_oerr);
         m[key].ens_pd.wgt_na.add(cur.total);

         //
         // Store the summary statistics
         //
         m[key].me_na.add(cur.me);
         m[key].mse_na.add((is_bad_data(cur.rmse) ?
                            bad_data_double :
                            cur.rmse * cur.rmse));
         m[key].me_oerr_na.add(cur.me_oerr);
         m[key].mse_oerr_na.add((is_bad_data(cur.rmse_oerr) ?
                                 bad_data_double :
                                 cur.rmse_oerr * cur.rmse_oerr));

         //
         // Compute and store climatological CRPS
         //
         if(!is_bad_data(cur.crps) && !is_bad_data(cur.crpss) &&
            !is_eq(cur.crpss, 1.0)) {
            crps_climo = cur.crps / (1.0 - cur.crpss);
            m[key].crps_climo_na.add(crps_climo);
         }
         else {
            m[key].crps_climo_na.add(bad_data_double);
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   //
   // Loop over the map entries and compute summary stats
   //
   for(it = m.begin(); it != m.end(); it++) {

      // Set n_obs as the sum of the TOTAL column
      it->second.ens_pd.n_pair    = it->second.ens_pd.wgt_na.sum();

      // Compute ME and RMSE as weighted averages
      it->second.ens_pd.me        = it->second.me_na.wmean(it->second.ens_pd.wgt_na);
      v                           = it->second.mse_na.wmean(it->second.ens_pd.wgt_na);
      it->second.ens_pd.rmse      = (is_bad_data(v) ? bad_data_double : sqrt(v));
      it->second.ens_pd.me_oerr   = it->second.me_oerr_na.wmean(it->second.ens_pd.wgt_na);
      v                           = it->second.mse_oerr_na.wmean(it->second.ens_pd.wgt_na);
      it->second.ens_pd.rmse_oerr = (is_bad_data(v) ? bad_data_double : sqrt(v));

      crps_fcst  = it->second.ens_pd.crps_na.wmean(it->second.ens_pd.wgt_na);
      crps_climo = it->second.crps_climo_na.wmean(it->second.ens_pd.wgt_na);

      // Compute aggregated CRPSS
      if(!is_bad_data(crps_fcst) && !is_bad_data(crps_climo) &&
         !is_eq(crps_climo, 0.0)) {
         it->second.ens_pd.crpss = (crps_climo - crps_fcst)/crps_climo;
      }
      else {
         it->second.ens_pd.crpss = bad_data_double;
      }

   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_rhist_lines(LineDataFile &f, STATAnalysisJob &j,
                      map<ConcatString, AggrENSInfo> &m,
                      int &n_in, int &n_out) {
   STATLine line;
   AggrENSInfo aggr;
   RHISTData cur;
   ConcatString key;
   int i;
   map<ConcatString, AggrENSInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_rhist) {
            mlog << Error << "\naggr_rhist_lines() -> "
                 << "should only encounter ranked histogram "
                 << "(RHIST) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current RHIST line
         //
         parse_rhist_line(line, cur);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.ens_pd.clear();
            aggr.hdr.clear();
            for(i=0; i<cur.n_rank; i++) aggr.ens_pd.rhist_na.add(0);
            m[key] = aggr;
         }

         //
         // Check for N_RANK remaining constant
         //
         if(m[key].ens_pd.rhist_na.n_elements() != cur.n_rank) {
            mlog << Error << "\naggr_rhist_lines() -> "
                 << "the \"N_RANK\" column must remain constant ("
                 << m[key].ens_pd.rhist_na.n_elements() << " != " << cur.n_rank
                 << ").  Try setting \"-column_eq N_RANK n\".\n\n";
            throw(1);
         }

         //
         // Store the current weight (TOTAL column)
         //
         m[key].ens_pd.wgt_na.add(cur.total);

         //
         // Aggregate the ranked histogram counts
         //
         for(i=0; i<m[key].ens_pd.rhist_na.n_elements(); i++) {
            m[key].ens_pd.rhist_na.set(i, m[key].ens_pd.rhist_na[i] + cur.rhist_na[i]);
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_phist_lines(LineDataFile &f, STATAnalysisJob &j,
                      map<ConcatString, AggrENSInfo> &m,
                      int &n_in, int &n_out) {
   STATLine line;
   AggrENSInfo aggr;
   PHISTData cur;
   ConcatString key;
   int i;
   map<ConcatString, AggrENSInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_phist) {
            mlog << Error << "\naggr_phist_lines() -> "
                 << "should only encounter probability integral "
                 << "transform histogram (PHIST) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current PHIST line
         //
         parse_phist_line(line, cur);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.ens_pd.phist_bin_size = cur.bin_size;
            aggr.ens_pd.phist_na = cur.phist_na;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment counts in the existing map entry
         //
         else {

            //
            // Check for BIN_SIZE remaining constant
            //
            if(!is_eq(m[key].ens_pd.phist_bin_size, cur.bin_size)) {
               mlog << Error << "\naggr_phist_lines() -> "
                    << "the \"BIN_SIZE\" column must remain constant ("
                    << m[key].ens_pd.phist_bin_size << " != " << cur.bin_size
                    << ").  Try setting \"-column_eq BIN_SIZE n\".\n\n";
               throw(1);
            }

            //
            // Aggregate the probability integral transform histogram counts
            //
            for(i=0; i<m[key].ens_pd.phist_na.n_elements(); i++) {
               m[key].ens_pd.phist_na.set(i, m[key].ens_pd.phist_na[i] + cur.phist_na[i]);
            }
         } // end else

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_relp_lines(LineDataFile &f, STATAnalysisJob &j,
                      map<ConcatString, AggrENSInfo> &m,
                      int &n_in, int &n_out) {
   STATLine line;
   AggrENSInfo aggr;
   RELPData cur;
   ConcatString key;
   int i;
   map<ConcatString, AggrENSInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_relp) {
            mlog << Error << "\naggr_relp_lines() -> "
                 << "should only encounter relative position (RELP) "
                 << "line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current PHIST line
         //
         parse_relp_line(line, cur);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.ens_pd.relp_na = cur.relp_na;
            aggr.hdr.clear();
            m[key] = aggr;
         }
         //
         // Increment counts in the existing map entry
         //
         else {

            //
            // Check for N_ENS remaining constant
            //
            if(m[key].ens_pd.relp_na.n_elements() != cur.n_ens) {
               mlog << Error << "\naggr_relp_lines() -> "
                    << "the \"N_ENS\" column must remain constant ("
                    << m[key].ens_pd.relp_na.n_elements() << " != " << cur.n_ens
                    << ").  Try setting \"-column_eq N_ENS n\".\n\n";
               throw(1);
            }

            //
            // Aggregate the RELP histogram counts
            //
            for(i=0; i<m[key].ens_pd.relp_na.n_elements(); i++) {
               m[key].ens_pd.relp_na.set(i, m[key].ens_pd.relp_na[i] + cur.relp_na[i]);
            }
         } // end else

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_orank_lines(LineDataFile &f, STATAnalysisJob &j,
                      map<ConcatString, AggrENSInfo> &m,
                      int &n_in, int &n_out) {
   STATLine line;
   AggrENSInfo aggr;
   ORANKData cur;
   ConcatString key;
   int i, n_valid, n_bin;
   double esum, esumsq, crps, ign, pit;
   map<ConcatString, AggrENSInfo>::iterator it;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_orank) {
            mlog << Error << "\naggr_orank_lines() -> "
                 << "should only encounter observation rank "
                 << "(ORANK) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current ORANK line
         //
         parse_orank_line(line, cur);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Skip missing data
         //
         if(is_bad_data(cur.rank)) continue;

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            aggr.ens_pd.clear();
            aggr.ens_pd.obs_error_flag = !is_bad_data(cur.ens_mean_oerr);
            aggr.ens_pd.set_ens_size(cur.n_ens);
            for(i=0; i<cur.n_ens+1; i++) aggr.ens_pd.rhist_na.add(0);
            aggr.ens_pd.phist_bin_size = j.out_bin_size;
            n_bin = ceil(1.0/aggr.ens_pd.phist_bin_size);
            for(i=0; i<n_bin; i++) aggr.ens_pd.phist_na.add(0);
            aggr.ens_pd.ssvar_bin_size = j.out_bin_size;
            aggr.hdr.clear();
            m[key] = aggr;
         }

         //
         // Check for N_ENS remaining constant
         //
         if(m[key].ens_pd.n_ens != cur.n_ens) {
            mlog << Error << "\naggr_orank_lines() -> "
                 << "the \"N_ENS\" column must remain constant.  "
                 << "Try setting \"-column_eq N_ENS n\".\n\n";
            throw(1);
         }

         //
         // Store the observation, ensemble mean, climatology,
         // ensemble spread, ensemble member values, and
         // valid ensemble count
         //
         m[key].ens_pd.add_obs(0.0, 0.0, cur.obs, cur.climo, bad_data_double);
         m[key].ens_pd.skip_ba.add(false);
         m[key].ens_pd.n_pair++;
         m[key].ens_pd.r_na.add(cur.rank);
         m[key].ens_pd.spread_na.add(cur.spread);
         m[key].ens_pd.spread_oerr_na.add(cur.spread_oerr);
         m[key].ens_pd.spread_plus_oerr_na.add(cur.spread_plus_oerr);
         m[key].ens_pd.mn_na.add(cur.ens_mean);
         m[key].ens_pd.mn_oerr_na.add(cur.ens_mean_oerr);

         for(i=0, n_valid=0, esum=0.0, esumsq=0.0;
             i<m[key].ens_pd.n_ens; i++) {
            m[key].ens_pd.add_ens(i, cur.ens_na[i]);
            if(!is_bad_data(cur.ens_na[i])) {
               esum   += cur.ens_na[i];
               esumsq += cur.ens_na[i]*cur.ens_na[i];
               n_valid++;
            }
         }
         m[key].ens_pd.esum_na.add(esum);
         m[key].ens_pd.esumsq_na.add(esumsq);
         m[key].ens_pd.v_na.add(n_valid);

         // Compute CRPS, IGN, and PIT for the current point
         compute_crps_ign_pit(cur.obs, cur.ens_na, crps, ign, pit);
         m[key].ens_pd.crps_na.add(crps);
         m[key].ens_pd.ign_na.add(ign);
         m[key].ens_pd.pit_na.add(pit);

         //
         // Increment the RHIST counts
         //
         i = cur.rank - 1;
         m[key].ens_pd.rhist_na.set(i, m[key].ens_pd.rhist_na[i] + 1);

         //
         // Increment the PHIST counts
         //
         if(!is_bad_data(cur.pit)) {
            i = (is_eq(cur.pit, 1.0) ?
                 m[key].ens_pd.phist_na.n_elements() - 1:
                 floor(cur.pit / m[key].ens_pd.phist_bin_size));
            m[key].ens_pd.phist_na.set(i, m[key].ens_pd.phist_na[i] + 1);
         }

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_ssvar_lines(LineDataFile &f, STATAnalysisJob &j,
                      map<ConcatString, AggrSSVARInfo> &m,
                      int &n_in, int &n_out) {
   STATLine line;
   AggrSSVARInfo aggr;
   SSVARInfo cur;
   ConcatString case_key, bin_key;
   ConcatString fcst_var, obs_var;
   double bin_width = bad_data_double;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         if(line.type() != stat_ssvar) {
            mlog << Error << "\naggr_ssvar_lines() -> "
                 << "should only encounter spread-skill variance "
                 << "(SSVAR) line types.\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Only aggregate consistent variable names
         //
         if(fcst_var.empty()) fcst_var = line.fcst_var();
         if(obs_var.empty())  obs_var  = line.obs_var();

         if(fcst_var != line.fcst_var() ||
            obs_var  != line.obs_var()) {
            mlog << Error << "\naggr_ssvar_lines() -> "
                 << "both the forecast and observation variable types must "
                 << "remain constant.  Try setting \"-fcst_var\" and/or "
                 << "\"-obs_var\".\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current SSVAR line
         //
         parse_ssvar_line(line, cur);

         //
         // Check for consistent bin width
         //
         if(is_bad_data(bin_width)) bin_width = cur.var_max - cur.var_min;

         if(!is_eq(bin_width, cur.var_max - cur.var_min)) {
            mlog << Warning << "\naggr_ssvar_lines() -> "
                 << "the SSVAR bin width changed from " << bin_width << " to "
                 << cur.var_max - cur.var_min << " on STAT line:\n" << line
                 << "\n\n";
            bin_width = cur.var_max - cur.var_min;
         }

         //
         // Build the case map and bin map keys for the current line
         //
         case_key = j.get_case_info(line);
         bin_key << cs_erase << cur.var_min << ":" << cur.var_max;

         //
         // Add a new case map entry, if necessary
         //
         if(m.count(case_key) == 0) {
            aggr.ssvar_bins.clear();
            aggr.hdr.clear();
            m[case_key] = aggr;
         }

         //
         // Add a new bin map entry, if necessary
         //
         if(m[case_key].ssvar_bins.count(bin_key) == 0) {
            m[case_key].ssvar_bins[bin_key] = cur;
         }
         //
         // Otherwise, aggregate with the existing bin entry
         //
         else {
            m[case_key].ssvar_bins[bin_key] += cur;
         }

         //
         // Keep track of the unique header column entries
         //
         m[case_key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_time_series_lines(LineDataFile &f, STATAnalysisJob &j,
                            map<ConcatString, AggrTimeSeriesInfo> &m,
                            int &n_in, int &n_out) {
   STATLine line;
   AggrTimeSeriesInfo cur;
   ConcatString key;
   int lead_sec;
   unixtime init_ut, valid_ut;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      if(line.is_header()) continue;

      n_in++;

      if(j.is_keeper(line)) {

         j.dump_stat_line(line);

         //
         // Build the map key for the current line
         //
         key = j.get_case_info(line);

         //
         // Add a new map entry, if necessary
         //
         if(m.count(key) == 0) {
            cur.clear();
            cur.fcst_var = line.fcst_var();
            cur.obs_var  = line.obs_var();
            m[key] = cur;
         }

         //
         // Only aggregate consistent variable names
         //
         if(m[key].fcst_var != line.fcst_var() ||
            m[key].obs_var  != line.obs_var()) {
            mlog << Error << "\naggr_time_series_lines() -> "
                 << "both the forecast and observation variable names must "
                 << "remain constant for case \"" << key
                 << "\".  Try setting \"-fcst_var\" and/or \"-obs_var\".\n"
                 << line << "\n\n";
            throw(1);
         }

         //
         // Parse the init and valid times
         //
         lead_sec = line.fcst_lead();
         valid_ut = line.fcst_valid_beg();
         init_ut  = valid_ut - lead_sec;

         //
         // Add times the the first point or for a series of valid times:
         // - Store valid and init times
         //
         if(!m[key].valid_ts.has(valid_ut)) {
            m[key].init_ts.add(init_ut);
            m[key].valid_ts.add(valid_ut);
         }
         //
         // Add times for a series of initialization times:
         // - Store multiple initialization times for a single valid time
         //
         else if(!m[key].init_ts.has(init_ut)) {
            m[key].init_ts.add(init_ut);
         }
         else {
            mlog << Warning << "\naggr_time_series_lines() -> "
                 << "skipping time series line for case \"" << key
                 << "\" with " << unix_to_yyyymmdd_hhmmss(init_ut)
                 << " initialization, " << sec_to_hhmmss(lead_sec)
                 << " lead, and " << unix_to_yyyymmdd_hhmmss(valid_ut)
                 << " valid times.\n\n";
            continue;
         }

         //
         // Add forecast and observation values
         //
         m[key].f_na.add(atof(line.get_item(j.column[0].c_str())));
         m[key].o_na.add(atof(line.get_item(j.column[1].c_str())));

         //
         // Keep track of the unique header column entries
         //
         m[key].hdr.add(line);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void mpr_to_ctc(STATAnalysisJob &j, const AggrMPRInfo &info,
                CTSInfo &cts_info) {
   int i;
   int n = info.f_na.n_elements();
   SingleThresh ft = j.out_fcst_thresh[0];
   SingleThresh ot = j.out_obs_thresh[0];

   //
   // Initialize
   //
   cts_info.clear();

   //
   // Update the contingency table counts
   //
   for(i=0; i<n; i++) {

      if(      ft.check(info.f_na[i]) &&  ot.check(info.o_na[i]))
         cts_info.cts.inc_fy_oy();
      else if( ft.check(info.f_na[i]) && !ot.check(info.o_na[i]))
         cts_info.cts.inc_fy_on();
      else if(!ft.check(info.f_na[i]) &&  ot.check(info.o_na[i]))
         cts_info.cts.inc_fn_oy();
      else if(!ft.check(info.f_na[i]) && !ot.check(info.o_na[i]))
         cts_info.cts.inc_fn_on();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void mpr_to_cts(STATAnalysisJob &j, const AggrMPRInfo &info,
                CTSInfo &cts_info, const char *tmp_dir,
                gsl_rng *rng_ptr) {
   CTSInfo *cts_info_ptr = (CTSInfo *) 0;

   //
   // Initialize
   //
   cts_info.clear();

   //
   // If there are no matched pairs to process, return
   //
   if(info.f_na.n_elements() == 0 || info.o_na.n_elements() == 0) return;

   //
   // Store the out_alpha value
   //
   cts_info.allocate_n_alpha(1);
   cts_info.alpha[0] = j.out_alpha;

   //
   // Store the thresholds
   //
   cts_info.fthresh = j.out_fcst_thresh[0];
   cts_info.othresh = j.out_obs_thresh[0];

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   cts_info_ptr = &cts_info;
   if(j.boot_interval == boot_bca_flag) {
      compute_cts_stats_ci_bca(rng_ptr, info.f_na, info.o_na,
         j.n_boot_rep,
         cts_info_ptr, 1, 1,
         j.rank_corr_flag, tmp_dir);
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, info.f_na, info.o_na,
         j.n_boot_rep, j.boot_rep_prop,
         cts_info_ptr, 1, 1,
         j.rank_corr_flag, tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void mpr_to_mctc(STATAnalysisJob &j, const AggrMPRInfo &info,
                 MCTSInfo &mcts_info) {
   int i;
   int n = info.f_na.n_elements();

   //
   // Initialize
   //
   mcts_info.clear();

   //
   // Setup
   //
   mcts_info.cts.set_size(j.out_fcst_thresh.n_elements() + 1);
   mcts_info.set_fthresh(j.out_fcst_thresh);
   mcts_info.set_othresh(j.out_obs_thresh);

   //
   // Update the contingency table counts
   //
   for(i=0; i<n; i++) mcts_info.add(info.f_na[i], info.o_na[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void mpr_to_mcts(STATAnalysisJob &j, const AggrMPRInfo &info,
                 MCTSInfo &mcts_info, const char *tmp_dir,
                 gsl_rng *rng_ptr) {

   //
   // Initialize
   //
   mcts_info.clear();

   //
   // If there are no matched pairs to process, return
   //
   if(info.f_na.n_elements() == 0 || info.o_na.n_elements() == 0) return;

   //
   // Setup
   //
   mcts_info.cts.set_size(j.out_fcst_thresh.n_elements() + 1);
   mcts_info.set_fthresh(j.out_fcst_thresh);
   mcts_info.set_othresh(j.out_obs_thresh);

   //
   // Store the out_alpha value
   //
   mcts_info.allocate_n_alpha(1);
   mcts_info.alpha[0] = j.out_alpha;

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(j.boot_interval == boot_bca_flag) {
      compute_mcts_stats_ci_bca(rng_ptr, info.f_na, info.o_na,
         j.n_boot_rep,
         mcts_info, 1,
         j.rank_corr_flag, tmp_dir);
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, info.f_na, info.o_na,
         j.n_boot_rep, j.boot_rep_prop,
         mcts_info, 1,
         j.rank_corr_flag, tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void mpr_to_cnt(STATAnalysisJob &j, const AggrMPRInfo &info,
                CNTInfo &cnt_info, const char *tmp_dir,
                gsl_rng *rng_ptr) {
   bool precip_flag = false;
   NumArray w_na;
   int i;

   //
   // Initialize
   //
   cnt_info.clear();

   //
   // If there are no matched pairs to process, return
   //
   if(info.f_na.n_elements() == 0 || info.o_na.n_elements() == 0) return;

   //
   // Set the precip flag based on fcst_var and obs_var
   //
   if(is_precip_var_name(info.fcst_var) && is_precip_var_name(info.obs_var))
      precip_flag = true;

   //
   // Store the out_alpha value
   //
   cnt_info.allocate_n_alpha(1);
   cnt_info.alpha[0] = j.out_alpha;

   //
   // Set weights to default values
   //
   for(i=0; i<info.f_na.n_elements(); i++) w_na.add(default_grid_weight);

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(j.boot_interval == boot_bca_flag) {

      compute_cnt_stats_ci_bca(rng_ptr,
         info.f_na, info.o_na, info.cmn_na, w_na,
         precip_flag, j.rank_corr_flag, j.n_boot_rep,
         cnt_info, tmp_dir);
   }
   else {

      compute_cnt_stats_ci_perc(rng_ptr,
         info.f_na, info.o_na, info.cmn_na, w_na,
         precip_flag, j.rank_corr_flag, j.n_boot_rep, j.boot_rep_prop,
         cnt_info, tmp_dir);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void mpr_to_psum(STATAnalysisJob &j, const AggrMPRInfo &info,
                 SL1L2Info &s_info) {
   int i;
   int n = info.f_na.n_elements();
   int scount, sacount;
   double f, o, c;
   double f_sum,  o_sum,  ff_sum,  oo_sum,  fo_sum;
   double fa_sum, oa_sum, ffa_sum, ooa_sum, foa_sum;
   double abs_err_sum;

   //
   // Initialize the SL1L2Info object and counts
   //
   s_info.clear();
   scount = sacount = 0;
   f_sum  = o_sum  =  ff_sum  = oo_sum  = fo_sum  = 0.0;
   fa_sum = oa_sum =  ffa_sum = ooa_sum = foa_sum = 0.0;
   abs_err_sum = 0.0;

   //
   // Update the partial sums
   //
   for(i=0; i<n; i++) {

      //
      // Update the counts for this matched pair
      //
      f = info.f_na[i];
      o = info.o_na[i];
      c = info.cmn_na[i];

      f_sum       += f;
      o_sum       += o;
      ff_sum      += f*f;
      oo_sum      += o*o;
      fo_sum      += f*o;
      abs_err_sum += fabs(f-o);
      scount   += 1;

      //
      // Check c for valid data
      //
      if(!is_bad_data(c)) {

         fa_sum    += f-c;
         oa_sum    += o-c;
         ffa_sum   += (f-c)*(f-c);
         ooa_sum   += (o-c)*(o-c);
         foa_sum   += (f-c)*(o-c);
         sacount   += 1;
      }
   } // end for

   if(scount != 0) {
      s_info.scount = scount;
      s_info.fbar   = f_sum/scount;
      s_info.obar   = o_sum/scount;
      s_info.fobar  = fo_sum/scount;
      s_info.ffbar  = ff_sum/scount;
      s_info.oobar  = oo_sum/scount;
      s_info.mae    = abs_err_sum/scount;
   }

   if(sacount != 0) {
      s_info.sacount  = sacount;
      s_info.fabar    = fa_sum/sacount;
      s_info.oabar    = oa_sum/sacount;
      s_info.foabar   = foa_sum/sacount;
      s_info.ffabar   = ffa_sum/sacount;
      s_info.ooabar   = ooa_sum/sacount;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void mpr_to_pct(STATAnalysisJob &j, const AggrMPRInfo &info,
                PCTInfo &pct_info) {
   int pstd_flag;
   NumArray c_na;

   //
   // Initialize
   //
   pct_info.clear();

   //
   // If there are no matched pairs to process, return
   //
   if(info.f_na.n_elements() == 0 || info.o_na.n_elements() == 0) return;

   //
   // Set up the PCTInfo thresholds and alpha values
   //
   pct_info.fthresh = j.out_fcst_thresh;
   pct_info.othresh = j.out_obs_thresh[0];
   pct_info.allocate_n_alpha(1);
   pct_info.alpha[0] = j.out_alpha;

   if(j.out_line_type.has(stat_pstd_str)) pstd_flag = 1;
   else                                   pstd_flag = 0;

   //
   // Compute the climatological probabilities
   //
   c_na = derive_climo_prob(info.cmn_na, info.csd_na, pct_info.othresh);

   //
   // Compute the probabilistic counts and statistics
   //
   compute_pctinfo(info.f_na, info.o_na, c_na, pstd_flag, pct_info);

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_vif(NumArray &na) {
   double corr, vif;

   // Compute the lag1 autocorrelation
   corr = stats_lag1_autocorrelation(na);

   // Compute the variance inflation factor
   vif = 1 + 2.0*fabs(corr) - 2.0*fabs(corr)/na.n_elements();

   return(vif);
}

////////////////////////////////////////////////////////////////////////

bool is_precip_var_name(const ConcatString &s) {

   bool match = has_prefix(pinterp_precipitation_names,
                           n_pinterp_precipitation_names, s.c_str()) ||
                has_prefix(grib_precipitation_abbr,
                           n_grib_precipitation_abbr, s.c_str());

   return(match);
}

////////////////////////////////////////////////////////////////////////
