// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>

#include "vx_log.h"

#include "stat_hdr_info.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static const string case_str = "CASE";

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
// Keep track the unique STAT header entries for each input STAT line.
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
// Keep track the unique STAT header entries for each pair.
//
////////////////////////////////////////////////////////////////////////

void StatHdrInfo::add(
   const int &lead_sec, const unixtime &valid_ut,
   const string &fcst_var_str, const string &obs_var_str,
   const string &obtype_str) {

   if(!fcst_lead.has(lead_sec))
      fcst_lead.add(lead_sec);
   if(fcst_valid_beg == (unixtime) 0 || valid_ut < fcst_valid_beg)
      fcst_valid_beg = valid_ut;
   if(fcst_valid_end == (unixtime) 0 || valid_ut > fcst_valid_end)
      fcst_valid_end = valid_ut;
   if(!obs_lead.has(lead_sec))
      obs_lead.add(lead_sec);
   if(obs_valid_beg == (unixtime) 0 || valid_ut < obs_valid_beg)
      obs_valid_beg = valid_ut;
   if(obs_valid_end == (unixtime) 0 || valid_ut > obs_valid_end)
      obs_valid_end = valid_ut;
   if(!fcst_var.has(fcst_var_str))
      fcst_var.add(fcst_var_str);
   if(!obs_var.has(obs_var_str))
      obs_var.add(obs_var_str);
   if(!obtype.has(obtype_str))
      obtype.add(obtype_str);

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
   if(model.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << model.n()
           << " unique MODEL values: "
           << write_css(model) << "\n";
   }

   // DESC
   if(desc.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << desc.n()
           << " unique DESC values: "
           << write_css(desc) << "\n";
   }

   // FCST_LEAD
   if(fcst_lead.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_lead.n()
           << " unique FCST_LEAD values: "
           << write_css_hhmmss(fcst_lead) << "\n";
   }

   // OBS_LEAD
   if(obs_lead.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_lead.n()
           << " unique OBS_LEAD values: "
           << write_css_hhmmss(obs_lead) << "\n";
   }

   // FCST_VAR
   if(fcst_var.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_var.n()
           << " unique FCST_VAR values: "
           << write_css(fcst_var) << "\n";
   }

   // FCST_UNITS
   if(fcst_units.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_units.n()
           << " unique FCST_UNITS values: "
           << write_css(fcst_units) << "\n";
   }

   // FCST_LEV
   if(fcst_lev.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_lev.n()
           << " unique FCST_LEV values: "
           << write_css(fcst_lev) << "\n";
   }

   // OBS_VAR
   if(obs_var.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_var.n()
           << " unique OBS_VAR values: "
           << write_css(obs_var) << "\n";
   }

   // OBS_UNITS
   if(obs_units.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_units.n()
           << " unique OBS_UNITS values: "
           << write_css(obs_units) << "\n";
   }

   // OBS_LEV
   if(obs_lev.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_lev.n()
           << " unique OBS_LEV values: "
           << write_css(obs_lev) << "\n";
   }

   // OBTYPE
   if(obtype.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obtype.n()
           << " unique OBTYPE values: "
           << write_css(obtype) << "\n";
   }

   // VX_MASK
   if(vx_mask.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << vx_mask.n()
           << " unique VX_MASK values: "
           << write_css(vx_mask) << "\n";
   }

   // INTERP_MTHD
   if(interp_mthd.n() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << interp_mthd.n()
           << " unique INTERP_MTHD values: "
           << write_css(interp_mthd) << ".\n";
   }

   // INTERP_PNTS
   if(interp_pnts.n() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << interp_pnts.n()
           << " unique INTERP_PNTS values: "
           << write_css(interp_pnts) << ".\n";
   }

   // FCST_THRESH
   if(fcst_thresh.n() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << fcst_thresh.n()
           << " unique FCST_THRESH values: "
           << write_css(fcst_thresh) << "\n";
   }

   // OBS_THRESH
   if(obs_thresh.n() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << obs_thresh.n()
           << " unique OBS_THRESH values: "
           << write_css(obs_thresh) << "\n";
   }

   // COV_THRESH
   if(cov_thresh.n() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << cov_thresh.n()
           << " unique COV_THRESH values: "
           << write_css(cov_thresh) << ".\n";
   }

   // ALPHA
   if(alpha.n() > 1) {
      mlog << Warning
           << "For case \"" << cur_case << "\", found "
           << alpha.n()
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
   ThreshArray ta;
   ConcatString css;
   double out_alpha;
   int wdth;
   StatHdrColumns shc;

   // MODEL
   if(!hdr_cols.has("MODEL")) {
      shc.set_model(get_col_css(cur_case, "MODEL", model, false).c_str());
   }

   // DESC
   if(!hdr_cols.has("DESC")) {
      shc.set_desc(get_col_css(cur_case, "DESC", desc, false).c_str());
   }

   // FCST_LEAD
   css = write_css_hhmmss(fcst_lead);
   if(fcst_lead.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << fcst_lead.n()
           << " unique FCST_LEAD values: " << css << "\n";
   }
   shc.set_fcst_lead_sec(fcst_lead.max());

   // FCST_VALID_BEG, FCST_VALID_END
   shc.set_fcst_valid_beg(fcst_valid_beg);
   shc.set_fcst_valid_end(fcst_valid_end);

   // OBS_LEAD
   css = write_css_hhmmss(obs_lead);
   if(obs_lead.n() > 1) {
      mlog << Debug(2)
           << "For case \"" << cur_case << "\", found "
           << obs_lead.n()
           << " unique OBS_LEAD values: " << css << "\n";
   }
   shc.set_obs_lead_sec(obs_lead.max());

   // OBS_VALID_BEG, OBS_VALID_END
   shc.set_obs_valid_beg(obs_valid_beg);
   shc.set_obs_valid_end(obs_valid_end);

   // FCST_VAR
   if(!hdr_cols.has("FCST_VAR")) {
      shc.set_fcst_var(get_col_css(cur_case, "FCST_VAR", fcst_var, false));
   }

   // FCST_UNITS
   if(!hdr_cols.has("FCST_UNITS")) {
      shc.set_fcst_units(get_col_css(cur_case, "FCST_UNITS", fcst_units, false));
   }

   // FCST_LEV
   if(!hdr_cols.has("FCST_LEV")) {
      shc.set_fcst_lev(get_col_css(cur_case, "FCST_LEV", fcst_lev, false).c_str());
   }

   // OBS_VAR
   if(!hdr_cols.has("OBS_VAR")) {
      shc.set_obs_var(get_col_css(cur_case, "OBS_VAR", obs_var, false));
   }

   // OBS_UNITS
   if(!hdr_cols.has("OBS_UNITS")) {
      shc.set_obs_units(get_col_css(cur_case, "OBS_UNITS", obs_units, false));
   }

   // OBS_LEV
   if(!hdr_cols.has("OBS_LEV")) {
      shc.set_obs_lev(get_col_css(cur_case, "OBS_LEV", obs_lev, false).c_str());
   }

   // OBTYPE
   if(!hdr_cols.has("OBTYPE")) {
      shc.set_obtype(get_col_css(cur_case, "OBTYPE", obtype, false).c_str());
   }

   // VX_MASK
   if(!hdr_cols.has("VX_MASK")) {
      shc.set_mask(get_col_css(cur_case, "VX_MASK", vx_mask, false).c_str());
   }

   // INTERP_MTHD
   if(!hdr_cols.has("INTERP_MTHD")) {
      shc.set_interp_mthd(get_col_css(cur_case, "INTERP_MTHD", interp_mthd, true));
   }

   // INTERP_PNTS
   if(!hdr_cols.has("INTERP_PNTS")) {
      css = write_css(interp_pnts);
      if(interp_pnts.n() == 0 || interp_pnts.n() > 1) {
         mlog << Warning
              << "For case \"" << cur_case << "\", found "
              << interp_pnts.n()
              << " unique INTERP_PNTS values: " << css << ".\n";
         wdth = bad_data_int;
      }
      else {
         wdth = nint(sqrt(interp_pnts[0]));
      }
      shc.set_interp_wdth(wdth);
   }

   // FCST_THRESH
   if(!hdr_cols.has("FCST_THRESH")) {
      ta.clear();
      ta.add_css(get_col_css(cur_case, "FCST_THRESH", fcst_thresh, true).c_str());
      shc.set_fcst_thresh(ta);
   }

   // OBS_THRESH
   if(!hdr_cols.has("OBS_THRESH")) {
      ta.clear();
      ta.add_css(get_col_css(cur_case, "OBS_THRESH", obs_thresh, true).c_str());
      shc.set_obs_thresh(ta);
   }

   // COV_THRESH
   if(!hdr_cols.has("COV_THRESH")) {
      ta.clear();
      ta.add_css(get_col_css(cur_case, "COV_THRESH", cov_thresh, true).c_str());
      shc.set_cov_thresh(ta);
   }

   // ALPHA
   if(!hdr_cols.has("ALPHA")) {
      css = write_css(alpha);
      if(alpha.n() == 0 || alpha.n() > 1) {
         mlog << Warning
              << "For case \"" << cur_case << "\", found "
              << alpha.n()
              << " unique ALPHA values: " << css << ".\n";
         out_alpha = bad_data_double;
      }
      else {
         out_alpha = alpha[0];
      }
      shc.set_alpha(out_alpha);
   }

   // LINE_TYPE
   shc.set_line_type(statlinetype_to_string(lt));

   // Apply the -set_hdr options
   StringArray case_vals = cur_case.split(":");
   shc.apply_set_hdr_opts(hdr_cols, hdr_vals, case_cols, case_vals);

   return shc;
}

////////////////////////////////////////////////////////////////////////

ConcatString StatHdrInfo::get_col_css(const ConcatString &cur_case,
                                      const char         *col_name,
                                      const StringArray  &col_vals,
                                      bool                warning) const {

   // Build comma-separated list of column values
   ConcatString css(write_css(col_vals));

   // Check for multiple entries
   if(col_vals.n() > 1) {
      ConcatString msg;
      msg << "For case \"" << cur_case << "\", found "
          << col_vals.n() << " unique " << col_name
          << " values: " << css << "\n";
      if(warning) mlog << Warning  << msg;
      else        mlog << Debug(2) << msg;
   }

   return css;
}

////////////////////////////////////////////////////////////////////////
