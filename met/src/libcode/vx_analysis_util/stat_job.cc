// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "stat_columns.h"
#include "stat_job.h"
#include "analysis_utils.h"

#include "vx_statistics.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static ConcatString timestring(const unixtime t);
static bool         check_thresh_column(const ThreshArray &list,
                                        const ThreshArray &item);

////////////////////////////////////////////////////////////////////////
//
// Code for class STATAnalysisJob
//
////////////////////////////////////////////////////////////////////////

STATAnalysisJob::STATAnalysisJob() {

   init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

STATAnalysisJob::~STATAnalysisJob() {

   clear();
}

////////////////////////////////////////////////////////////////////////

STATAnalysisJob::STATAnalysisJob(const STATAnalysisJob & aj) {

   init_from_scratch();

   assign(aj);
}

////////////////////////////////////////////////////////////////////////

STATAnalysisJob & STATAnalysisJob::operator=(
   const STATAnalysisJob & aj) {

   if(this == &aj ) return ( * this );

   assign(aj);

   return (* this);
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::init_from_scratch() {

   dump_row  = (char *)     0;
   dr_out    = (ofstream *) 0;
   n_dump    =              0;
   stat_file = (char *)     0;
   stat_out  = (ofstream *) 0;
   mask_grid = (char *)     0;
   mask_poly = (char *)     0;
   boot_rng  = (char *)     0;
   boot_seed = (char *)     0;

   model.set_ignore_case(1);
   desc.set_ignore_case(1);
   fcst_var.set_ignore_case(1);
   obs_var.set_ignore_case(1);
   fcst_lev.set_ignore_case(1);
   obs_lev.set_ignore_case(1);
   obtype.set_ignore_case(1);
   vx_mask.set_ignore_case(1);
   interp_mthd.set_ignore_case(1);
   line_type.set_ignore_case(1);
   column.set_ignore_case(1);
   column_case.set_ignore_case(1);
   hdr_name.set_ignore_case(1);

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::clear() {

   precision = default_precision;

   job_type = no_stat_job_type;

   model.clear();
   desc.clear();

   fcst_lead.clear();
   obs_lead.clear();

   fcst_valid_beg = fcst_valid_end = (unixtime) 0;
   fcst_valid_hour.clear();

   obs_valid_beg  = obs_valid_end  = (unixtime) 0;
   obs_valid_hour.clear();

   fcst_init_beg  = fcst_init_end  = (unixtime) 0;
   fcst_init_hour.clear();

   obs_init_beg   = obs_init_end   = (unixtime) 0;
   obs_init_hour.clear();

   fcst_var.clear();
   obs_var.clear();

   fcst_lev.clear();
   obs_lev.clear();

   obtype.clear();

   vx_mask.clear();

   interp_mthd.clear();
   interp_pnts.clear();

   fcst_thresh.clear();
   obs_thresh.clear();
   cov_thresh.clear();

   thresh_logic = SetLogic_None;

   alpha.clear();

   line_type.clear();
   column.clear();
   weight.clear();

   column_thresh_map.clear();
   column_str_map.clear();

   column_case.clear();

   hdr_name.clear();
   hdr_value.clear();

   close_dump_row_file();
   close_stat_file();

   if(dump_row)  { delete [] dump_row;  dump_row  = (char *) 0; }
   if(stat_file) { delete [] stat_file; stat_file = (char *) 0; }
   if(mask_grid) { delete [] mask_grid; mask_grid = (char *) 0; }
   if(mask_poly) { delete [] mask_poly; mask_poly = (char *) 0; }

   out_line_type.clear();

   out_fcst_thresh.clear();
   out_obs_thresh.clear();
   out_cnt_logic = SetLogic_Union;

   out_fcst_wind_thresh.clear();
   out_obs_wind_thresh.clear();
   out_wind_logic = SetLogic_Union;

   // Initialize ramp job settings
   ramp_type       = default_ramp_type;
   ramp_time_fcst  = default_ramp_time;
   ramp_time_obs   = default_ramp_time;
   ramp_exact_fcst = default_ramp_exact;
   ramp_exact_obs  = default_ramp_exact;
   ramp_thresh_fcst.clear();
   ramp_thresh_obs.clear();
   ramp_window_beg = default_ramp_window;
   ramp_window_end = default_ramp_window;
   swing_width     = bad_data_double;

   // Set to default values
   out_alpha      = default_alpha;
   out_bin_size   = default_bin_size;
   boot_interval  = default_boot_interval;
   boot_rep_prop  = default_boot_rep_prop;
   n_boot_rep     = default_n_boot_rep;
   set_boot_rng (default_boot_rng);
   set_boot_seed(default_boot_seed);
   rank_corr_flag = default_rank_corr_flag;
   vif_flag       = default_vif_flag;

   poly_mask.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::assign(const STATAnalysisJob & aj) {

   clear();

   precision            = aj.precision;

   job_type             = aj.job_type;

   model                = aj.model;
   desc                 = aj.desc;

   fcst_lead            = aj.fcst_lead;
   obs_lead             = aj.obs_lead;

   fcst_valid_beg       = aj.fcst_valid_beg;
   fcst_valid_end       = aj.fcst_valid_end;
   fcst_valid_hour      = aj.fcst_valid_hour;

   obs_valid_beg        = aj.obs_valid_beg;
   obs_valid_end        = aj.obs_valid_end;
   obs_valid_hour       = aj.obs_valid_hour;

   fcst_init_beg        = aj.fcst_init_beg;
   fcst_init_end        = aj.fcst_init_end;
   fcst_init_hour       = aj.fcst_init_hour;

   obs_init_beg         = aj.obs_init_beg;
   obs_init_end         = aj.obs_init_end;
   obs_init_hour        = aj.obs_init_hour;

   fcst_var             = aj.fcst_var;
   obs_var              = aj.obs_var;

   fcst_lev             = aj.fcst_lev;
   obs_lev              = aj.obs_lev;

   obtype               = aj.obtype;

   vx_mask              = aj.vx_mask;

   interp_mthd          = aj.interp_mthd;
   interp_pnts          = aj.interp_pnts;

   fcst_thresh          = aj.fcst_thresh;
   obs_thresh           = aj.obs_thresh;
   cov_thresh           = aj.cov_thresh;

   thresh_logic         = aj.thresh_logic;

   alpha                = aj.alpha;

   line_type            = aj.line_type;
   column               = aj.column;
   weight               = aj.weight;

   column_thresh_map    = aj.column_thresh_map;
   column_str_map       = aj.column_str_map;

   column_case          = aj.column_case;

   hdr_name             = aj.hdr_name;
   hdr_value            = aj.hdr_value;

   out_line_type        = aj.out_line_type;

   out_fcst_thresh      = aj.out_fcst_thresh;
   out_obs_thresh       = aj.out_obs_thresh;
   out_cnt_logic        = aj.out_cnt_logic;
   out_fcst_wind_thresh = aj.out_fcst_wind_thresh;
   out_obs_wind_thresh  = aj.out_obs_wind_thresh;
   out_wind_logic       = aj.out_wind_logic;
   out_alpha            = aj.out_alpha;
   out_bin_size         = aj.out_bin_size;

   ramp_type            = aj.ramp_type;
   ramp_time_fcst       = aj.ramp_time_fcst;
   ramp_time_obs        = aj.ramp_time_obs;
   ramp_exact_fcst      = aj.ramp_exact_fcst;
   ramp_exact_obs       = aj.ramp_exact_obs;
   ramp_thresh_fcst     = aj.ramp_thresh_fcst;
   ramp_thresh_obs      = aj.ramp_thresh_obs;
   ramp_window_beg      = aj.ramp_window_beg;
   ramp_window_end      = aj.ramp_window_end;
   swing_width          = aj.swing_width;

   boot_interval        = aj.boot_interval;
   boot_rep_prop        = aj.boot_rep_prop;
   n_boot_rep           = aj.n_boot_rep;

   rank_corr_flag       = aj.rank_corr_flag;
   vif_flag             = aj.vif_flag;

   set_dump_row (aj.dump_row);
   set_stat_file(aj.stat_file);
   set_mask_grid(aj.mask_grid);
   set_mask_poly(aj.mask_poly);
   set_boot_rng (aj.boot_rng);
   set_boot_seed(aj.boot_seed);

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::dump(ostream & out, int depth) const {
   Indent prefix(depth);

   out << prefix << "job type = " << statjobtype_to_string(job_type) << "\n";

   out << prefix << "model ...\n";
   model.dump(out, depth + 1);

   out << prefix << "desc ...\n";
   desc.dump(out, depth + 1);

   out << prefix << "fcst_lead ...\n";
   fcst_lead.dump(out, depth + 1);

   out << prefix << "obs_lead ...\n";
   obs_lead.dump(out, depth + 1);

   out << prefix << "fcst_valid_beg = "
       << prefix << timestring(fcst_valid_beg) << "\n";

   out << prefix << "fcst_valid_end = "
       << prefix << timestring(fcst_valid_end) << "\n";

   out << prefix << "fcst_valid_hour ...\n";
   fcst_valid_hour.dump(out, depth + 1);

   out << prefix << "obs_valid_beg = "
       << prefix << timestring(obs_valid_beg) << "\n";

   out << prefix << "obs_valid_end = "
       << prefix << timestring(obs_valid_end) << "\n";

   out << prefix << "obs_valid_hour ...\n";
   obs_valid_hour.dump(out, depth + 1);

   out << prefix << "fcst_init_beg = "
       << prefix << timestring(fcst_init_beg) << "\n";

   out << prefix << "fcst_init_end = "
       << prefix << timestring(fcst_init_end) << "\n";

   out << prefix << "fcst_init_hour ...\n";
   fcst_init_hour.dump(out, depth + 1);

   out << prefix << "obs_init_beg = "
       << prefix << timestring(obs_init_beg) << "\n";

   out << prefix << "obs_init_end = "
       << prefix << timestring(obs_init_end) << "\n";

   out << prefix << "obs_init_hour ...\n";
   obs_init_hour.dump(out, depth + 1);

   out << prefix << "fcst_var ...\n";
   fcst_var.dump(out, depth + 1);

   out << prefix << "obs_var ...\n";
   obs_var.dump(out, depth + 1);

   out << prefix << "fcst_lev ...\n";
   fcst_lev.dump(out, depth + 1);

   out << prefix << "obs_lev ...\n";
   obs_lev.dump(out, depth + 1);

   out << prefix << "obtype ...\n";
   obtype.dump(out, depth + 1);

   out << prefix << "vx_mask ...\n";
   vx_mask.dump(out, depth + 1);

   out << prefix << "interp_mthd ...\n";
   interp_mthd.dump(out, depth + 1);

   out << prefix << "interp_pnts ...\n";
   interp_pnts.dump(out, depth + 1);

   out << prefix << "fcst_thresh ...\n";
   fcst_thresh.dump(out, depth + 1);

   out << prefix << "obs_thresh ...\n";
   obs_thresh.dump(out, depth + 1);

   out << prefix << "cov_thresh ...\n";
   cov_thresh.dump(out, depth + 1);

   out << prefix << "thresh_logic = "
       << setlogic_to_string(thresh_logic) << "\n";

   out << prefix << "alpha ...\n";
   alpha.dump(out, depth + 1);

   out << prefix << "line_type ...\n";
   line_type.dump(out, depth + 1);

   out << prefix << "column ...\n";
   column.dump(out, depth + 1);

   out << prefix << "weight ...\n";
   weight.dump(out, depth + 1);

   out << prefix << "column_thresh_map ...\n";
   for(map<ConcatString,ThreshArray>::const_iterator thr_it = column_thresh_map.begin();
       thr_it != column_thresh_map.end(); thr_it++) {
      out << prefix << thr_it->first << ": \n";
      thr_it->second.dump(out, depth + 1);
   }

   out << prefix << "column_str_map ...\n";
   for(map<ConcatString,StringArray>::const_iterator str_it = column_str_map.begin();
       str_it != column_str_map.end(); str_it++) {
      out << prefix << str_it->first << ": \n";
      str_it->second.dump(out, depth + 1);
   }

   out << prefix << "column_case ...\n";
   column_case.dump(out, depth + 1);

   out << prefix << "hdr_name ...\n";
   hdr_name.dump(out, depth + 1);

   out << prefix << "hdr_value ...\n";
   hdr_value.dump(out, depth + 1);

   out << prefix << "dump_row = "
       << dump_row << "\n";

   out << prefix << "stat_file = "
       << stat_file << "\n";

   out << prefix << "mask_grid = "
       << (mask_grid ? mask_grid : na_str) << "\n";

   out << prefix << "mask_poly = "
       << (mask_poly ? mask_poly : na_str) << "\n";

   out << prefix << "out_line_type ...\n";
   out_line_type.dump(out, depth + 1);

   out << prefix << "out_fcst_thresh ...\n";
   out_fcst_thresh.dump(out, depth + 1);

   out << prefix << "out_obs_thresh ...\n";
   out_obs_thresh.dump(out, depth + 1);

   out << prefix << "out_cnt_logic = "
       << setlogic_to_string(out_cnt_logic) << "\n";

   out << prefix << "out_fcst_wind_thresh ...\n";
   out_fcst_wind_thresh.get_str();

   out << prefix << "out_obs_wind_thresh ...\n";
   out_obs_wind_thresh.get_str();

   out << prefix << "out_wind_logic = "
       << setlogic_to_string(out_wind_logic) << "\n";

   out << prefix << "out_alpha = "
       << out_alpha << "\n";

   out << prefix << "out_bin_size = "
       << out_bin_size << "\n";

   out << prefix << "ramp_type = "
       << timeseriestype_to_string(ramp_type) << "\n";

   out << prefix << "ramp_time_fcst = "
       << sec_to_hhmmss(ramp_time_fcst) << "\n";

   out << prefix << "ramp_time_obs = "
       << sec_to_hhmmss(ramp_time_obs) << "\n";

   out << prefix << "ramp_exact_fcst = "
       << bool_to_string(ramp_exact_fcst) << "\n";

   out << prefix << "ramp_exact_obs = "
       << bool_to_string(ramp_exact_obs) << "\n";

   out << prefix << "ramp_thresh_fcst ... "
       << ramp_thresh_fcst.get_str() << "\n";

   out << prefix << "ramp_thresh_obs ... "
       << ramp_thresh_obs.get_str() << "\n";

   out << prefix << "ramp_window_beg = "
       << sec_to_hhmmss(ramp_window_beg) << "\n";

   out << prefix << "ramp_window_end = "
       << sec_to_hhmmss(ramp_window_end) << "\n";

   out << prefix << "swing_width = "
       << swing_width << "\n";

   out << prefix << "boot_interval = "
       << boot_interval << "\n";

   out << prefix << "boot_rep_prop = "
       << boot_rep_prop << "\n";

   out << prefix << "n_boot_rep = "
       << n_boot_rep << "\n";

   out << prefix << "boot_rng = "
       << boot_rng << "\n";

   out << prefix << "boot_seed = "
       << boot_seed << "\n";

   out << prefix << "rank_corr_flag = "
       << rank_corr_flag << "\n";

   out << prefix << "vif_flag = "
       << vif_flag << "\n";

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

int STATAnalysisJob::is_keeper(const STATLine & L) const {

   //
   // model
   //
   if(model.n_elements() > 0) {
      if(!(model.has(L.model()))) return(0);
   }

   //
   // desc
   //
   if(desc.n_elements() > 0) {
      if(!(desc.has(L.desc()))) return(0);
   }

   //
   // fcst_lead (in seconds)
   //
   if(fcst_lead.n_elements() > 0) {
      if(!fcst_lead.has(L.fcst_lead())) return(0);
   }

   //
   // fcst_valid_beg
   //
   if((fcst_valid_beg > 0) && (L.fcst_valid_beg() < fcst_valid_beg))
      return(0);

   //
   // fcst_valid_end
   //
   if((fcst_valid_end > 0) && (L.fcst_valid_end() > fcst_valid_end))
      return(0);

   //
   // fcst_valid_hour: if specified for the job, the fcst_valid_beg and
   //                  fcst_valid_end times must match
   //
   if(fcst_valid_hour.n_elements() > 0) {

      // Check that fcst_valid_beg = fcst_valid_end
      if(L.fcst_valid_beg() != L.fcst_valid_end()) return(0);

      if(!fcst_valid_hour.has(L.fcst_valid_hour())) return(0);
   }

   //
   // fcst_init_beg
   //
   if((fcst_init_beg > 0) && (L.fcst_init_beg() < fcst_init_beg))
      return(0);

   //
   // fcst_init_end
   //
   if((fcst_init_end > 0) && (L.fcst_init_end() > fcst_init_end))
      return(0);

   //
   // fcst_init_hour: if specified for the job, the fcst_init_beg and
   //                 fcst_init_end times must match
   //
   if(fcst_init_hour.n_elements() > 0) {

      // Check that fcst_init_beg = fcst_init_end
      if(L.fcst_init_beg() != L.fcst_init_end()) return(0);

      if(!fcst_init_hour.has(L.fcst_init_hour())) return(0);
   }

   //
   // obs_lead (in seconds)
   //
   if(obs_lead.n_elements() > 0) {
      if(!obs_lead.has(L.obs_lead())) return(0);
   }

   //
   // obs_valid_beg
   //
   if((obs_valid_beg > 0) && (L.obs_valid_beg() < obs_valid_beg))
      return(0);

   //
   // obs_valid_end
   //
   if((obs_valid_end > 0) && (L.obs_valid_end() > obs_valid_end))
      return(0);

   //
   // obs_valid_hour: if specified for the job, the obs_valid_beg and
   //                 obs_valid_end times must match
   //
   if(obs_valid_hour.n_elements() > 0) {

      // Check that obs_valid_beg = obs_valid_end
      if(L.obs_valid_beg() != L.obs_valid_end()) return(0);

      if(!obs_valid_hour.has(L.obs_valid_hour())) return(0);
   }

   //
   // obs_init_beg
   //
   if((obs_init_beg > 0) && (L.obs_init_beg() < obs_init_beg))
      return(0);

   //
   // obs_init_end
   //
   if((obs_init_end > 0) && (L.obs_init_end() > obs_init_end))
      return(0);

   //
   // obs_init_hour: if specified for the job, the obs_init_beg and
   //                 obs_init_end times must match
   //
   if(obs_init_hour.n_elements() > 0) {

      // Check that obs_init_beg = obs_init_end
      if(L.obs_init_beg() != L.obs_init_end()) return(0);

      if(!obs_init_hour.has(L.obs_init_hour())) return(0);
   }

   //
   // fcst_var
   //
   if(fcst_var.n_elements() > 0) {
      if(!(fcst_var.has(L.fcst_var()))) return(0);
   }

   //
   // fcst_lev
   //
   if(fcst_lev.n_elements() > 0) {
      if(!(fcst_lev.has(L.fcst_lev()))) return(0);
   }

   //
   // obs_var
   //
   if(obs_var.n_elements() > 0) {
      if(!(obs_var.has(L.obs_var()))) return(0);
   }

   //
   // obs_lev
   //
   if(obs_lev.n_elements() > 0) {
      if(!(obs_lev.has(L.obs_lev()))) return(0);
   }

   //
   // obtype
   //
   if(obtype.n_elements() > 0) {
      if(!(obtype.has(L.obtype()))) return(0);
   }

   //
   // vx_mask
   //
   if(vx_mask.n_elements() > 0) {
      if(!(vx_mask.has(L.vx_mask()))) return(0);
   }

   //
   // interp_mthd
   //
   if(interp_mthd.n_elements() > 0) {
      if(!(interp_mthd.has(L.interp_mthd()))) return(0);
   }

   //
   // interp_pnts
   //
   if(interp_pnts.n_elements() > 0) {
      if(!interp_pnts.has(L.interp_pnts())) return(0);
   }

   //
   // fcst_thresh
   //
   if(fcst_thresh.n_elements() > 0) {
      if(!check_thresh_column(fcst_thresh, L.fcst_thresh())) return(0);
   }

   //
   // obs_thresh
   //
   if(obs_thresh.n_elements() > 0) {
      if(!check_thresh_column(obs_thresh, L.obs_thresh())) return(0);
   }

   //
   // cov_thresh
   //
   if(cov_thresh.n_elements() > 0) {
      if(!check_thresh_column(cov_thresh, L.cov_thresh())) return(0);
   }

   //
   // thresh_logic
   //
   if(thresh_logic != SetLogic_None &&
      thresh_logic != L.thresh_logic()) return(0);

   //
   // alpha
   //
   if(alpha.n_elements() > 0) {
      if(!alpha.has(L.alpha())) return(0);
   }

   //
   // line_type
   //
   if(line_type.n_elements() > 0) {
      if(!(line_type.has(L.line_type()))) return(0);
   }

   //
   // column_thresh
   //
   for(map<ConcatString,ThreshArray>::const_iterator thr_it = column_thresh_map.begin();
       thr_it != column_thresh_map.end(); thr_it++) {

      //
      // Check if the current value meets the threshold criteria
      //
      if(!thr_it->second.check_dbl(atof(L.get_item(thr_it->first)))) return(0);
   }

   //
   // column_str
   //
   for(map<ConcatString,StringArray>::const_iterator str_it = column_str_map.begin();
       str_it != column_str_map.end(); str_it++) {

      //
      // Check if the current value is in the list for the column
      //
      if(!str_it->second.has(L.get_item(str_it->first, false))) return(0);
   }

   return(1);
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::parse_job_command(const char *jobstring) {
   char *line = (char *) 0;
   char *c    = (char *) 0;
   char *lp   = (char *) 0;
   const char delim [] = " ";
   ConcatString col_name;
   StringArray col_value;
   ConcatString thresh_cs;
   int i, n;

   // If jobstring is null, simply return;
   if(jobstring) n = strlen(jobstring);
   else          return;

   // Job Command Line Array
   StringArray jc_array;

   //
   // Create a temporary copy of the jobstring for use in parsing
   //
   line = new char [n + 1];
   memset(line, 0, n + 1);
   strcpy(line, jobstring);

   lp = line;

   //
   // Parse the command line entries into a StringArray object
   //
   while((c = strtok(lp, delim)) != NULL) {

      jc_array.add(c);

      lp = (char *) 0;
   }

   //
   // If command line switches are present, clear out the values
   // already specified in the job for that option.
   //
   for(i=0; i<jc_array.n_elements(); i++) {

      if(     strcmp(jc_array[i], "-model"          ) == 0)
         model.clear();
      else if(strcmp(jc_array[i], "-desc"           ) == 0)
         desc.clear();
      else if(strcmp(jc_array[i], "-fcst_lead"      ) == 0)
         fcst_lead.clear();
      else if(strcmp(jc_array[i], "-obs_lead"       ) == 0)
         obs_lead.clear();
      else if(strcmp(jc_array[i], "-fcst_valid_hour") == 0)
         fcst_valid_hour.clear();
      else if(strcmp(jc_array[i], "-obs_valid_hour" ) == 0)
         obs_valid_hour.clear();
      else if(strcmp(jc_array[i], "-fcst_init_hour" ) == 0)
         fcst_init_hour.clear();
      else if(strcmp(jc_array[i], "-obs_init_hour"  ) == 0)
         obs_init_hour.clear();
      else if(strcmp(jc_array[i], "-fcst_var"       ) == 0)
         fcst_var.clear();
      else if(strcmp(jc_array[i], "-fcst_lev"       ) == 0)
         fcst_lev.clear();
      else if(strcmp(jc_array[i], "-obs_var"        ) == 0)
         obs_var.clear();
      else if(strcmp(jc_array[i], "-obs_lev"        ) == 0)
         obs_lev.clear();
      else if(strcmp(jc_array[i], "-obtype"         ) == 0)
         obtype.clear();
      else if(strcmp(jc_array[i], "-vx_mask"        ) == 0)
         vx_mask.clear();
      else if(strcmp(jc_array[i], "-interp_mthd"    ) == 0)
         interp_mthd.clear();
      else if(strcmp(jc_array[i], "-interp_pnts"    ) == 0)
         interp_pnts.clear();
      else if(strcmp(jc_array[i], "-fcst_thresh"    ) == 0)
         fcst_thresh.clear();
      else if(strcmp(jc_array[i], "-obs_thresh"     ) == 0)
         obs_thresh.clear();
      else if(strcmp(jc_array[i], "-cov_thresh"     ) == 0)
         cov_thresh.clear();
      else if(strcmp(jc_array[i], "-alpha"          ) == 0)
         alpha.clear();
      else if(strcmp(jc_array[i], "-line_type"      ) == 0)
         line_type.clear();
      else if(strcmp(jc_array[i], "-column"         ) == 0)
         column.clear();
      else if(strcmp(jc_array[i], "-weight"         ) == 0)
         weight.clear();
      else if(strcmp(jc_array[i], "-column_min"     ) == 0 ||
              strcmp(jc_array[i], "-column_max"     ) == 0 ||
              strcmp(jc_array[i], "-column_eq"      ) == 0 ||
              strcmp(jc_array[i], "-column_thresh"  ) == 0) {
         column_thresh_map.clear();
      }
      else if(strcmp(jc_array[i], "-column_str"     ) == 0) {
         column_str_map.clear();
      }
      else if(strcmp(jc_array[i], "-set_hdr"        ) == 0) {
         hdr_name.clear();
         hdr_value.clear();
      }
      else if(strcmp(jc_array[i], "-by"             ) == 0) {
         column_case.clear();
      }
      else if(strcmp(jc_array[i], "-out_line_type"  ) == 0) {
         out_line_type.clear();
      }
      else if(strcmp(jc_array[i], "-out_thresh"     ) == 0) {
         out_fcst_thresh.clear();
         out_obs_thresh.clear();
      }
      else if(strcmp(jc_array[i], "-out_fcst_thresh") == 0) {
         out_fcst_thresh.clear();
      }
      else if(strcmp(jc_array[i], "-out_obs_thresh" ) == 0) {
         out_obs_thresh.clear();
      }
   }

   //
   // Parse the command line and set the options
   //
   for(i=0; i<jc_array.n_elements(); i++) {

      //
      // Parse the job command line switches
      //

      if(strcmp(jc_array[i], "-job") == 0) {

         if(set_job_type(jc_array[i+1]) != 0) {
            mlog << Error << "\nSTATAnalysisJob::STATAnalysisJob::parse_job_command() -> "
                 << "unrecognized job type specified \"" << jc_array[i]
                 << "\" in job command line: " << jobstring << "\n\n";

            throw(1);
         }
         i++;
      }
      else if(strcmp(jc_array[i], "-model") == 0) {
         model.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-desc") == 0) {
         desc.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_lead") == 0) {
         fcst_lead.add_css_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_lead") == 0) {
         obs_lead.add_css_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_valid_beg") == 0) {
         fcst_valid_beg = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_valid_end") == 0) {
         fcst_valid_end = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_valid_hour") == 0) {
         fcst_valid_hour.add_css_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_valid_beg") == 0) {
         obs_valid_beg = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_valid_end") == 0) {
         obs_valid_end = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_valid_hour") == 0) {
         obs_valid_hour.add_css_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_init_beg") == 0) {
         fcst_init_beg = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_init_end") == 0) {
         fcst_init_end = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_init_hour") == 0) {
         fcst_init_hour.add_css_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_init_beg") == 0) {
         obs_init_beg = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_init_end") == 0) {
         obs_init_end = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_init_hour") == 0) {
         obs_init_hour.add_css_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_var") == 0) {
         fcst_var.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_lev") == 0) {
         fcst_lev.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_var") == 0) {
         obs_var.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_lev") == 0) {
         obs_lev.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obtype") == 0) {
         obtype.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-vx_mask") == 0) {
         vx_mask.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-interp_mthd") == 0) {
         interp_mthd.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-interp_pnts") == 0) {
         interp_pnts.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_thresh") == 0) {
         fcst_thresh.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_thresh") == 0) {
         obs_thresh.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-cov_thresh") == 0) {
         cov_thresh.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-thresh_logic") == 0) {
         thresh_logic = string_to_setlogic(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-alpha") == 0) {
         alpha.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-line_type") == 0) {
         line_type.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-column") == 0) {
         column.add_css(to_upper(jc_array[i+1]));
         i++;
      }
      else if(strcmp(jc_array[i], "-weight") == 0) {
         weight.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-column_min") == 0) {
         thresh_cs << cs_erase << ">=" << jc_array[i+2];
         add_column_thresh(jc_array[i+1], thresh_cs);
         i+=2;
      }
      else if(strcmp(jc_array[i], "-column_max") == 0) {
         thresh_cs << cs_erase << "<=" << jc_array[i+2];
         add_column_thresh(jc_array[i+1], thresh_cs);
         i+=2;
      }
      else if(strcmp(jc_array[i], "-column_eq") == 0) {
         thresh_cs << cs_erase << "==" << jc_array[i+2];
         add_column_thresh(jc_array[i+1], thresh_cs);
         i+=2;
      }
      else if(strcmp(jc_array[i], "-column_thresh") == 0) {
         add_column_thresh(jc_array[i+1], jc_array[i+2]);
         i+=2;
      }
      else if(strcmp(jc_array[i], "-column_str") == 0) {

         // Parse the column name and value
         col_name = to_upper(jc_array[i+1]);
         col_value.clear();
         col_value.set_ignore_case(1);
         col_value.add_css(jc_array[i+2]);

         // If the column name is already present in the map, add to it
         if(column_str_map.count(col_name) > 0) {
            column_str_map[col_name].add(col_value);
         }
         // Otherwise, add a new map entry
         else {
            column_str_map.insert(pair<ConcatString, StringArray>(col_name, col_value));
         }
         i+=2;
      }
      else if(strcmp(jc_array[i], "-set_hdr") == 0) {
         n = METHdrTable.header(met_version, "STAT", na_str)->col_offset(to_upper(jc_array[i+1]));
         if(is_bad_data(n)) {
            mlog << Error << "\nSTATAnalysisJob::parse_job_command() -> "
                 << "no match found for header column named: \""
                 << to_upper(jc_array[i+1]) << "\"\n\n";
            throw(1);
         }
         hdr_name.add_css(to_upper(jc_array[i+1]));
         hdr_value.add_css(jc_array[i+2]);
         i+=2;
      }
      else if(strcmp(jc_array[i], "-by") == 0) {
         column_case.add_css(to_upper(jc_array[i+1]));
         i+=1;
      }
      else if(strcmp(jc_array[i], "-dump_row") == 0) {
         set_dump_row(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_stat") == 0) {
         set_stat_file(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-mask_grid") == 0) {
         set_mask_grid(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-mask_poly") == 0) {
         set_mask_poly(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_line_type") == 0) {
         out_line_type.add_css(to_upper(jc_array[i+1]));
         i++;
      }
      else if(strcmp(jc_array[i], "-out_thresh") == 0) {
         out_fcst_thresh.add_css(jc_array[i+1]);
         out_obs_thresh.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_fcst_thresh") == 0) {
         out_fcst_thresh.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_obs_thresh") == 0) {
         out_obs_thresh.add_css(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_cnt_logic") == 0) {
         out_cnt_logic = string_to_setlogic(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_wind_thresh") == 0) {
         out_fcst_wind_thresh.set(jc_array[i+1]);
         out_obs_wind_thresh.set(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_fcst_wind_thresh") == 0) {
         out_fcst_wind_thresh.set(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_obs_wind_thresh") == 0) {
         out_obs_wind_thresh.set(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_wind_logic") == 0) {
         out_wind_logic = string_to_setlogic(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_alpha") == 0) {
         out_alpha = atof(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_type") == 0) {
         ramp_type = string_to_timeseriestype(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_time") == 0) {
         ramp_time_fcst = ramp_time_obs = timestring_to_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_time_fcst") == 0) {
         ramp_time_fcst = timestring_to_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_time_obs") == 0) {
         ramp_time_obs = timestring_to_sec(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_exact") == 0) {
         ramp_exact_fcst = ramp_exact_obs = string_to_bool(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_exact_fcst") == 0) {
         ramp_exact_fcst = string_to_bool(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_exact_obs") == 0) {
         ramp_exact_obs = string_to_bool(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_thresh") == 0) {
         ramp_thresh_fcst.set(jc_array[i+1]);
         ramp_thresh_obs.set(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_thresh_fcst") == 0) {
         ramp_thresh_fcst.set(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-ramp_thresh_obs") == 0) {
         ramp_thresh_obs.set(jc_array[i+1]);
         i++;
      }
      // May be specified using 1 or 2 arguments:
      //  - 2 arguments for beginning and ending times
      //  - 1 argument for a symmetric time window
      else if(strcmp(jc_array[i], "-ramp_window") == 0) {

         // Parse beginning and ending times
         if(i+2 < jc_array.n_elements() && is_number(jc_array[i+2])) {
            ramp_window_beg = timestring_to_sec(jc_array[i+1]);
            ramp_window_end = timestring_to_sec(jc_array[i+2]);
            i+=2;
         }
         // Parse symmetric time window
         else {
            ramp_window_end = timestring_to_sec(jc_array[i+1]);
            ramp_window_beg = -1 * ramp_window_end;
            i+=1;
         }
      }
      else if(strcmp(jc_array[i], "-swing_width") == 0) {
         swing_width = atof(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_bin_size") == 0) {
         out_bin_size = atof(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-boot_interval") == 0) {
         boot_interval = atoi(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-boot_rep_prop") == 0) {
         boot_rep_prop = atof(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-n_boot_rep") == 0) {
         n_boot_rep = atoi(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-boot_rng") == 0) {
         set_boot_rng(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-boot_seed") == 0) {
         set_boot_seed(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-rank_corr_flag") == 0) {
         rank_corr_flag = atoi(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-vif_flag") == 0) {
         vif_flag = atoi(jc_array[i+1]);
         i++;
      }
      else {
         mlog << Error << "\nSTATAnalysisJob::parse_job_command() -> "
              << "unrecognized switch \"" << jc_array[i]
              << "\" in job command line: "
              << jobstring << "\n\n";

         throw(1);
      } // end if

   } // end for

   //
   // Deallocate memory
   //
   if(line) { delete [] line; line = (char *) 0; }
   lp = (char *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::add_column_thresh(const char *col_name, const char *thresh_str) {
   ThreshArray col_thresh;

   // Parse the threshold
   col_thresh.add_css(thresh_str);

   // If the column name is already present in the map, add to it
   if(column_thresh_map.count(col_name) > 0) {
      column_thresh_map[col_name].add(col_thresh);
   }
   // Otherwise, add a new map entry
   else {
      column_thresh_map.insert(pair<ConcatString, ThreshArray>(col_name, col_thresh));
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int STATAnalysisJob::set_job_type(const char *c) {

   job_type = string_to_statjobtype(c);

   if(job_type == no_stat_job_type) return(1);
   else                             return(0);
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::set_dump_row(const char *c) {

   if(dump_row) { delete [] dump_row; dump_row = (char *) 0; }

   if(!c) return;

   dump_row = new char [strlen(c) + 1];

   strcpy(dump_row, c);

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::set_stat_file(const char *c) {

   if(stat_file) { delete [] stat_file; stat_file = (char *) 0; }

   if(!c) return;

   stat_file = new char [strlen(c) + 1];

   strcpy(stat_file, c);

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::set_mask_grid(const char *c) {

   if(mask_grid) { delete [] mask_grid; mask_grid = (char *) 0; }

   if(!c) return;

   mask_grid = new char [strlen(c) + 1];

   strcpy(mask_grid, c);

   process_mask_grid();

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::set_mask_poly(const char *c) {

   if(mask_poly) { delete [] mask_poly; mask_poly = (char *) 0; }

   if(!c) return;

   mask_poly = new char [strlen(c) + 1];

   strcpy(mask_poly, c);

   process_mask_poly();

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::set_boot_rng(const char *c) {

   if(boot_rng) { delete [] boot_rng; boot_rng = (char *) 0; }

   if(!c) return;

   boot_rng = new char [strlen(c) + 1];

   strcpy(boot_rng, c);

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::set_boot_seed(const char *c) {

   if(boot_seed) { delete [] boot_seed; boot_seed = (char *) 0; }

   if(!c) return;

   boot_seed = new char [strlen(c) + 1];

   strcpy(boot_seed, c);

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::open_dump_row_file() {

   close_dump_row_file();

   if(!dump_row) return;

   dr_out = new ofstream;
   dr_out->open(dump_row);
   n_dump = 0;

   if(!(*dr_out)) {
      mlog << Error << "\nSTATAnalysisJob::open_dump_row_file()-> "
           << "can't open the output file \"" << dump_row
           << "\" for writing!\n\n";

      throw(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::close_dump_row_file() {

   if(dr_out) {

      //
      // Write any remaining lines
      //
      *(dr_out) << dump_at;

      dr_out->close();
      delete dr_out;
      dr_out = (ofstream *) 0;
      n_dump = 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::open_stat_file() {

   close_stat_file();

   if(!stat_file) return;

   stat_out = new ofstream;
   stat_out->open(stat_file);

   if(!(*stat_out)) {
      mlog << Error << "\nSTATAnalysisJob::open_stat_file()-> "
           << "can't open the output STAT file \"" << stat_file
           << "\" for writing!\n\n";

      throw(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::setup_stat_file(int n_row, int n) {
   STATLineType cur_lt, out_lt;
   StringArray out_sa;
   int i, c, n_col;

   //
   // Nothing to do unless output STAT file stream is defined
   //
   if(!stat_out) return;

   //
   // Check for a single output line type
   //
   out_sa = (out_line_type.n_elements() > 0 ?
             out_line_type : line_type);
   out_lt = (out_sa.n_elements() == 1 ?
             string_to_statlinetype(out_sa[0]) : no_stat_line_type);

   //
   // Loop through the output line types and determine the number of
   // output columns
   //
   for(i=0, n_col=0; i<out_sa.n_elements(); i++) {
      cur_lt = string_to_statlinetype(out_sa[i]);
      switch(cur_lt) {
         case stat_sl1l2:  c = n_sl1l2_columns;        break;
         case stat_sal1l2: c = n_sal1l2_columns;       break;
         case stat_vl1l2:  c = n_vl1l2_columns;        break;
         case stat_val1l2: c = n_val1l2_columns;       break;
         case stat_fho:    c = n_fho_columns;          break;
         case stat_ctc:    c = n_ctc_columns;          break;
         case stat_cts:    c = n_cts_columns;          break;
         case stat_mctc:   c = get_n_mctc_columns(n);  break;
         case stat_mcts:   c = n_mcts_columns;         break;
         case stat_cnt:    c = n_cnt_columns;          break;
         case stat_pct:    c = get_n_pct_columns(n);   break;
         case stat_pstd:   c = get_n_pstd_columns(n);  break;
         case stat_pjc:    c = get_n_pjc_columns(n);   break;
         case stat_prc:    c = get_n_prc_columns(n);   break;
         case stat_mpr:    c = n_mpr_columns;          break;
         case stat_nbrctc: c = n_nbrctc_columns;       break;
         case stat_nbrcts: c = n_nbrcts_columns;       break;
         case stat_nbrcnt: c = n_nbrcnt_columns;       break;
         case stat_isc:    c = n_isc_columns;          break;
         case stat_wdir:   c = n_job_wdir_columns;     break;
         case stat_rhist:  c = get_n_rhist_columns(n); break;
         case stat_phist:  c = get_n_phist_columns(n); break;
         case stat_orank:  c = n_orank_columns;        break;
         case stat_ssvar:  c = n_ssvar_columns;        break;
         default:
            mlog << Error << "\nSTATAnalysisJob::setup_stat_file() -> "
                 << "unexpected stat line type \"" << statlinetype_to_string(cur_lt)
                 << "\"!\n\n";
            exit(1);
            break;
      }
      if(c > n_col) n_col = c;
   }

   //
   // Add the header columns
   //
   n_col += n_header_columns;

   //
   // Setup the STAT table
   //
   stat_at.set_size(n_row, n_col);
   justify_stat_cols(stat_at);
   stat_at.set_precision(precision);
   stat_at.set_bad_data_value(bad_data_double);
   stat_at.set_bad_data_str(na_str);
   stat_at.set_delete_trailing_blank_rows(1);

   //
   // Write the STAT header row
   //
   switch(out_lt) {
      case stat_sl1l2:  write_header_row       (sl1l2_columns, n_sl1l2_columns, 1,       stat_at, 0, 0); break;
      case stat_sal1l2: write_header_row       (sal1l2_columns, n_sal1l2_columns, 1,     stat_at, 0, 0); break;
      case stat_vl1l2:  write_header_row       (vl1l2_columns, n_vl1l2_columns, 1,       stat_at, 0, 0); break;
      case stat_val1l2: write_header_row       (val1l2_columns, n_val1l2_columns, 1,     stat_at, 0, 0); break;
      case stat_fho:    write_header_row       (fho_columns, n_fho_columns, 1,           stat_at, 0, 0); break;
      case stat_ctc:    write_header_row       (ctc_columns, n_ctc_columns, 1,           stat_at, 0, 0); break;
      case stat_cts:    write_header_row       (cts_columns, n_cts_columns, 1,           stat_at, 0, 0); break;
      case stat_mctc:   write_mctc_header_row  (1, n,                                    stat_at, 0, 0); break;
      case stat_mcts:   write_header_row       (mcts_columns, n_mcts_columns, 1,         stat_at, 0, 0); break;
      case stat_cnt:    write_header_row       (cnt_columns, n_cnt_columns, 1,           stat_at, 0, 0); break;
      case stat_pct:    write_pct_header_row   (1, n,                                    stat_at, 0, 0); break;
      case stat_pstd:   write_pstd_header_row  (1, n,                                    stat_at, 0, 0); break;
      case stat_pjc:    write_pjc_header_row   (1, n,                                    stat_at, 0, 0); break;
      case stat_prc:    write_prc_header_row   (1, n,                                    stat_at, 0, 0); break;
      case stat_mpr:    write_header_row       (mpr_columns, n_mpr_columns, 1,           stat_at, 0, 0); break;
      case stat_nbrctc: write_header_row       (nbrctc_columns, n_sl1l2_columns, 1,      stat_at, 0, 0); break;
      case stat_nbrcts: write_header_row       (nbrcts_columns, n_sl1l2_columns, 1,      stat_at, 0, 0); break;
      case stat_nbrcnt: write_header_row       (nbrcnt_columns, n_sl1l2_columns, 1,      stat_at, 0, 0); break;
      case stat_isc:    write_header_row       (isc_columns, n_isc_columns, 1,           stat_at, 0, 0); break;
      case stat_wdir:   write_header_row       (job_wdir_columns, n_job_wdir_columns, 1, stat_at, 0, 0); break;
      case stat_rhist:  write_rhist_header_row (1, n,                                    stat_at, 0, 0); break;
      case stat_phist:  write_phist_header_row (1, n,                                    stat_at, 0, 0); break;
      case stat_orank:  write_header_row       (orank_columns, n_orank_columns, 1,       stat_at, 0, 0); break;
      case stat_ssvar:  write_header_row       (ssvar_columns, n_ssvar_columns, 1,       stat_at, 0, 0); break;

      //
      // Write only header columns for unspecified line type
      //
      case no_stat_line_type:
                        write_header_row       ((const char **) 0, 0, 1,                 stat_at, 0, 0); break;

      default:
         mlog << Error << "\nSTATAnalysisJob::setup_stat_file() -> "
              << "unexpected stat line type \"" << statlinetype_to_string(out_lt)
              << "\"!\n\n";
         exit(1);
         break;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::close_stat_file() {

   if(stat_out) {

      //
      // Write any remaining lines
      //
      *(stat_out) << stat_at;

      stat_out->close();
      delete stat_out;
      stat_out = (ofstream *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::dump_stat_line(const STATLine &line) {
   int i;

   //
   // Nothing to do with no dump file
   //
   if(!dr_out) return;

   //
   // Write a header before the first line
   //
   if(n_dump == 0) {

      //
      // Format the dump row AsciiTable
      //
      dump_at.set_size(dump_stat_buffer_rows, dump_stat_buffer_cols);
      justify_stat_cols(dump_at);
      dump_at.set_precision(precision);
      dump_at.set_bad_data_value(bad_data_double);
      dump_at.set_bad_data_str(na_str);
      dump_at.set_delete_trailing_blank_rows(1);

      //
      // If the line_type is set to a single value
      // write a full header line
      //
      if(line_type.n_elements() == 1) {

         switch(string_to_statlinetype(line_type[0])) {
            case(stat_fho):
               write_header_row(fho_columns, n_fho_columns, 1, dump_at, 0, 0);
               break;

            case(stat_ctc):
               write_header_row(ctc_columns, n_ctc_columns, 1, dump_at, 0, 0);
               break;

            case(stat_cts):
               write_header_row(cts_columns, n_cts_columns, 1, dump_at, 0, 0);
               break;

            case(stat_cnt):
               write_header_row(cnt_columns, n_cnt_columns, 1, dump_at, 0, 0);
               break;

            case(stat_sl1l2):
               write_header_row(sl1l2_columns, n_sl1l2_columns, 1, dump_at, 0, 0);
               break;

            case(stat_sal1l2):
               write_header_row(sal1l2_columns, n_sal1l2_columns, 1, dump_at, 0, 0);
               break;

            case(stat_vl1l2):
               write_header_row(vl1l2_columns, n_vl1l2_columns, 1, dump_at, 0, 0);
               break;

            case(stat_val1l2):
               write_header_row(val1l2_columns, n_val1l2_columns, 1, dump_at, 0, 0);
               break;

            case(stat_mpr):
               write_header_row(mpr_columns, n_mpr_columns, 1, dump_at, 0, 0);
               break;

            case(stat_nbrctc):
               write_header_row(nbrctc_columns, n_nbrctc_columns, 1, dump_at, 0, 0);
               break;

            case(stat_nbrcts):
               write_header_row(nbrcts_columns, n_nbrcts_columns, 1, dump_at, 0, 0);
               break;

            case(stat_nbrcnt):
               write_header_row(nbrcnt_columns, n_nbrcnt_columns, 1, dump_at, 0, 0);
               break;

            case(stat_isc):
               write_header_row(isc_columns, n_isc_columns, 1, dump_at, 0, 0);
               break;

            case(stat_ssvar):
               write_header_row(ssvar_columns, n_ssvar_columns, 1, dump_at, 0, 0);
               break;

            // Just write a STAT header line for indeterminant line types
            case(stat_mctc):
            case(stat_mcts):
            case(stat_pct):
            case(stat_pstd):
            case(stat_pjc):
            case(stat_prc):
            case(stat_rhist):
            case(stat_phist):
            case(stat_orank):
               write_header_row((const char **) 0, 0, 1, dump_at, 0, 0);
               break;

            default:
               mlog << Error << "\ndump_stat_line() -> "
                    << "unexpected line type value " << line_type[0] << "\n\n";
               throw(1);
         } // end switch
      }
      //
      // Otherwise, just write a STAT header line
      //
      else {
         write_header_row((const char **) 0, 0, 1, dump_at, 0, 0);
      }

      n_dump++;
   }

   //
   // Store the data line
   //
   for(i=0; i<line.n_items(); i++) {
     dump_at.set_entry(n_dump%dump_at.nrows(), i, line.get_item(i));
   }
   n_dump++;

   //
   // Write the buffer, if full
   //
   if(n_dump%dump_at.nrows() == 0) {
      *(dr_out) << dump_at;
      dump_at.empty();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString STATAnalysisJob::get_case_info(const STATLine & L) const {
   int i;
   ConcatString key = "";

   //
   // Retrieve value for each column_case option
   //
   for(i=0; i<column_case.n_elements(); i++) {
      key << (i == 0 ? "" : ":")
          << L.get_item(column_case[i]);
   }

   return(key);
}

////////////////////////////////////////////////////////////////////////

ConcatString STATAnalysisJob::get_jobstring() const {
   int i;
   STATLineType type;
   ConcatString js;

   // Initialize the jobstring
   js.clear();

   // job type
   if(job_type != no_stat_job_type) {
      js << "-job " << statjobtype_to_string(job_type) << " ";
   }

   // model
   if(model.n_elements() > 0) {
      for(i=0; i<model.n_elements(); i++)
         js << "-model " << model[i] << " ";
   }

   // desc
   if(desc.n_elements() > 0) {
      for(i=0; i<desc.n_elements(); i++)
         js << "-desc " << desc[i] << " ";
   }

   // fcst_lead
   if(fcst_lead.n_elements() > 0) {
      for(i=0; i<fcst_lead.n_elements(); i++) {
         js << "-fcst_lead " << sec_to_hhmmss(nint(fcst_lead[i])) << " ";
      }
   }

   // obs_lead
   if(obs_lead.n_elements() > 0) {
      for(i=0; i<obs_lead.n_elements(); i++) {
         js << "-obs_lead " << sec_to_hhmmss(nint(obs_lead[i])) << " ";
      }
   }

   // fcst_valid_beg and fcst_valid_end
   if(fcst_valid_beg > 0) {
      js << "-fcst_valid_beg " << unix_to_yyyymmdd_hhmmss(fcst_valid_beg) << " ";
   }
   if(fcst_valid_end > 0) {
      js << "-fcst_valid_end " << unix_to_yyyymmdd_hhmmss(fcst_valid_end) << " ";
   }

   // fcst_valid_hour
   if(fcst_valid_hour.n_elements() > 0) {
      for(i=0; i<fcst_valid_hour.n_elements(); i++) {
         js << "-fcst_valid_hour " << sec_to_hhmmss(nint(fcst_valid_hour[i])) << " ";
      }
   }

   // obs_valid_beg and obs_valid_end
   if(obs_valid_beg > 0) {
      js << "-obs_valid_beg " << unix_to_yyyymmdd_hhmmss(obs_valid_beg) << " ";
   }
   if(obs_valid_end > 0) {
      js << "-obs_valid_end " << unix_to_yyyymmdd_hhmmss(obs_valid_end) << " ";
   }

   // obs_valid_hour
   if(obs_valid_hour.n_elements() > 0) {
      for(i=0; i<obs_valid_hour.n_elements(); i++) {
         js << "-obs_valid_hour " << sec_to_hhmmss(nint(obs_valid_hour[i])) << " ";
      }
   }

   // fcst_init_beg and fcst_init_end
   if(fcst_init_beg > 0) {
      js << "-fcst_init_beg " << unix_to_yyyymmdd_hhmmss(fcst_init_beg) << " ";
   }
   if(fcst_init_end > 0) {
      js << "-fcst_init_end " << unix_to_yyyymmdd_hhmmss(fcst_init_end) << " ";
   }

   // fcst_init_hour
   if(fcst_init_hour.n_elements() > 0) {
      for(i=0; i<fcst_init_hour.n_elements(); i++) {
         js << "-fcst_init_hour " << sec_to_hhmmss(nint(fcst_init_hour[i])) << " ";
      }
   }

   // obs_init_beg and obs_init_end
   if(obs_init_beg > 0) {
      js << "-obs_init_beg " << unix_to_yyyymmdd_hhmmss(obs_init_beg) << " ";
   }
   if(obs_init_end > 0) {
      js << "-obs_init_end " << unix_to_yyyymmdd_hhmmss(obs_init_end) << " ";
   }

   // obs_init_hour
   if(obs_init_hour.n_elements() > 0) {
      for(i=0; i<obs_init_hour.n_elements(); i++) {
         js << "-obs_init_hour " << sec_to_hhmmss(nint(obs_init_hour[i])) << " ";
      }
   }

   // fcst_var
   if(fcst_var.n_elements() > 0) {
      for(i=0; i<fcst_var.n_elements(); i++)
         js << "-fcst_var " << fcst_var[i] << " ";
   }

   // obs_var
   if(obs_var.n_elements() > 0) {
      for(i=0; i<obs_var.n_elements(); i++)
         js << "-obs_var " << obs_var[i] << " ";
   }

   // fcst_lev
   if(fcst_lev.n_elements() > 0) {
      for(i=0; i<fcst_lev.n_elements(); i++)
         js << "-fcst_lev " << fcst_lev[i] << " ";
   }

   // obs_lev
   if(obs_lev.n_elements() > 0) {
      for(i=0; i<obs_lev.n_elements(); i++)
         js << "-obs_lev " << obs_lev[i] << " ";
   }

   // obtype
   if(obtype.n_elements() > 0) {
      for(i=0; i<obtype.n_elements(); i++)
         js << "-obtype " << obtype[i] << " ";
   }

   // vx_mask
   if(vx_mask.n_elements() > 0) {
      for(i=0; i<vx_mask.n_elements(); i++)
         js << "-vx_mask " << vx_mask[i] << " ";
   }

   // interp_mthd
   if(interp_mthd.n_elements() > 0) {
      for(i=0; i<interp_mthd.n_elements(); i++)
         js << "-interp_mthd " << interp_mthd[i] << " ";
   }

   // interp_pnts
   if(interp_pnts.n_elements() > 0) {
      for(i=0; i<interp_pnts.n_elements(); i++)
         js << "-interp_pnts " << nint(interp_pnts[i]) << " ";
   }

   // fcst_thresh
   if(fcst_thresh.n_elements() > 0) {
      for(i=0; i<fcst_thresh.n_elements(); i++) {
         js << "-fcst_thresh " << fcst_thresh[i].get_str() << " ";
      }
   }

   // obs_thresh
   if(obs_thresh.n_elements() > 0) {
      for(i=0; i<obs_thresh.n_elements(); i++) {
         js << "-obs_thresh " << obs_thresh[i].get_str() << " ";
      }
   }

   // cov_thresh
   if(cov_thresh.n_elements() > 0) {
      for(i=0; i<cov_thresh.n_elements(); i++) {
         js << "-cov_thresh " << cov_thresh[i].get_str() << " ";
      }
   }

   // thresh_logic
   if(thresh_logic != SetLogic_None) {
      js << "-thresh_logic " << setlogic_to_string(thresh_logic) << " ";
   }

   // alpha
   if(alpha.n_elements() > 0) {
      for(i=0; i<alpha.n_elements(); i++)
         js << "-alpha " << alpha[i] << " ";
   }

   // line_type
   if(line_type.n_elements() > 0) {
      for(i=0; i<line_type.n_elements(); i++) {
         type = string_to_statlinetype(line_type[i]);
         js << "-line_type " << statlinetype_to_string(type) << " ";
      }
   }

   // column
   if(column.n_elements() > 0) {
      for(i=0; i<column.n_elements(); i++) {
         js << "-column " << column[i] << " ";
      }
   }

   // weight
   if(weight.n_elements() > 0) {
      for(i=0; i<weight.n_elements(); i++) {
         js << "-weight " << weight[i] << " ";
      }
   }

   // column_thresh
   for(map<ConcatString,ThreshArray>::const_iterator thr_it = column_thresh_map.begin();
       thr_it != column_thresh_map.end(); thr_it++) {

      for(i=0; i<thr_it->second.n_elements(); i++) {
         js << "-column_thresh " << thr_it->first << " " << thr_it->second[i].get_str() << " ";
      }
   }

   // column_str
   for(map<ConcatString,StringArray>::const_iterator str_it = column_str_map.begin();
       str_it != column_str_map.end(); str_it++) {

      for(i=0; i<str_it->second.n_elements(); i++) {
         js << "-column_str " << str_it->first << " " << str_it->second[i] << " ";
      }
   }

   // column_case
   if(column_case.n_elements() > 0) {
      for(i=0; i<column_case.n_elements(); i++)
         js << "-by " << column_case[i] << " ";
   }

   // set_hdr
   if(hdr_name.n_elements() > 0) {
      for(i=0; i<hdr_name.n_elements(); i++)
         js << "-set_hdr " << hdr_name[i]
            << " " << hdr_value[i] << " ";
   }

   // dump_row
   if(dump_row) js << "-dump_row " << dump_row << " ";

   // out_stat
   if(stat_file) js << "-out_stat " << stat_file << " ";

   // mask_grid
   if(mask_grid) js << "-mask_grid " << mask_grid << " ";

   // mask_poly
   if(mask_poly) js << "-mask_poly " << mask_poly << " ";

   // out_line_type
   if(out_line_type.n_elements() > 0) {
      for(i=0; i<out_line_type.n_elements(); i++)
         js << "-out_line_type " << out_line_type[i] << " ";
   }

   // out_fcst_thresh == out_obs_thresh
   if(out_fcst_thresh == out_obs_thresh) {
      for(i=0; i<out_fcst_thresh.n_elements(); i++) {
         js << "-out_thresh " << out_fcst_thresh[i].get_str() << " ";
      }
   }
   else {

      // out_fcst_thresh
      for(i=0; i<out_fcst_thresh.n_elements(); i++) {
         js << "-out_fcst_thresh " << out_fcst_thresh[i].get_str() << " ";
      }

      // out_obs_thresh
      for(i=0; i<out_obs_thresh.n_elements(); i++) {
         js << "-out_obs_thresh " << out_obs_thresh[i].get_str() << " ";
      }
   }

   // out_cnt_logic
   if(job_type == stat_job_aggr_stat &&
      line_type.has(stat_mpr_str) &&
      (out_line_type.has(stat_cnt_str)  || out_line_type.has(stat_sl1l2_str)) &&
      (out_fcst_thresh.n_elements() > 0 || out_obs_thresh.n_elements() > 0)) {
      js << "-out_cnt_logic " << setlogic_to_string(out_cnt_logic) << " ";
   }

   // out_fcst_wind_thresh == out_obs_wind_thresh
   if(out_fcst_wind_thresh == out_obs_wind_thresh) {
      if(out_fcst_wind_thresh.get_type() != thresh_na) {
         js << "-out_wind_thresh " << out_fcst_wind_thresh.get_str() << " ";
      }
   }
   else {
      // out_fcst_wind_thresh
      if(out_fcst_wind_thresh.get_type() != thresh_na) {
         js << "-out_fcst_wind_thresh " << out_fcst_wind_thresh.get_str() << " ";
      }

      // out_obs_wind_thresh
      if(out_obs_wind_thresh.get_type() != thresh_na) {
         js << "-out_obs_wind_thresh " << out_obs_wind_thresh.get_str() << " ";
      }
   }

   // out_wind_logic
   if(job_type == stat_job_aggr_stat &&
      line_type.has(stat_mpr_str) &&
      out_line_type.has(stat_wdir_str) &&
      (out_fcst_wind_thresh.get_type() != thresh_na ||
       out_obs_wind_thresh.get_type()  != thresh_na)) {
      js << "-out_wind_logic " << setlogic_to_string(out_cnt_logic) << " ";
   }

   // Jobs which use out_alpha
   if(job_type == stat_job_summary        ||
       out_line_type.has(stat_cts_str)    ||
       out_line_type.has(stat_mcts_str)   ||
       out_line_type.has(stat_cnt_str)    ||
       out_line_type.has(stat_pstd_str)   ||
       out_line_type.has(stat_nbrcts_str) ||
       out_line_type.has(stat_nbrcnt_str)) {

      // out_alpha
      js << "-out_alpha " << out_alpha << " ";
   }

   // Ramp jobs
   if(job_type == stat_job_ramp) {

      // ramp_type
      js << "-ramp_type " << timeseriestype_to_string(ramp_type) << " ";

      if(ramp_type == TimeSeriesType_DyDt) {

         // ramp_time
         if(ramp_time_fcst == ramp_time_obs) {
            js << "-ramp_time " << sec_to_hhmmss(ramp_time_fcst) << " ";
         }
         else {
            js << "-ramp_time_fcst " << sec_to_hhmmss(ramp_time_fcst) << " "
               << "-ramp_time_obs "  << sec_to_hhmmss(ramp_time_obs)  << " ";
         }

         // ramp_exact
         if(ramp_exact_fcst != default_ramp_exact ||
            ramp_exact_obs  != default_ramp_exact) {
            if(ramp_exact_fcst == ramp_exact_obs) {
               js << "-ramp_exact " << bool_to_string(ramp_exact_fcst) << " ";
            }
            else {
               js << "-ramp_exact_fcst " << bool_to_string(ramp_exact_fcst) << " "
                  << "-ramp_exact_obs "  << bool_to_string(ramp_exact_obs)  << " ";
            }
         }
      }

      if(ramp_type == TimeSeriesType_Swing) {

         // swing_width
         js << "-swing_width " << swing_width << " ";
      }

      // ramp_thresh
      if(ramp_thresh_fcst == ramp_thresh_obs) {
         js << "-ramp_thresh " << ramp_thresh_fcst.get_str() << " ";
      }
      else {
         js << "-ramp_thresh_fcst " << ramp_thresh_fcst.get_str() << " "
            << "-ramp_thresh_obs "  << ramp_thresh_obs.get_str()  << " ";
      }

      // ramp_window
      js << "-ramp_window " << sec_to_hhmmss(ramp_window_beg)
         << " " << sec_to_hhmmss(ramp_window_end) << " ";
   }

   // Jobs which use out_bin_size
   if(line_type.n_elements() > 0) {
      if(string_to_statlinetype(line_type[0]) == stat_orank &&
         out_line_type.has(stat_phist_str)) {

         // out_bin_size
        js << "-out_bin_size " << out_bin_size << " ";
      }
   }

   // Jobs which perform bootstrapping
   if(line_type.n_elements() > 0) {
      type = string_to_statlinetype(line_type[0]);
      if(type == stat_mpr                    &&
         (out_line_type.has(stat_cts_str)    ||
          out_line_type.has(stat_mcts_str)   ||
          out_line_type.has(stat_cnt_str)    ||
          out_line_type.has(stat_nbrcts_str) ||
          out_line_type.has(stat_nbrcnt_str))) {

         // Bootstrap Information
         js << "-boot_interval " << boot_interval << " ";
         js << "-boot_rep_prop " << boot_rep_prop << " ";
         js << "-n_boot_rep "    << n_boot_rep    << " ";
         js << "-boot_rng "      << boot_rng      << " ";
         if(boot_seed) {
            if(strlen(boot_seed) == 0) {
               js << "-boot_seed '' ";
            }
            else {
               js << "-boot_seed "  << boot_seed     << " ";
            }
         }
      }
   }

   // Jobs which compute rank correlations
   if(out_line_type.has(stat_cnt_str)) {

      // rank_corr_flag
      js << "-rank_corr_flag " << rank_corr_flag << " ";
   }

   // Variance inflation factor for time series
   if(vif_flag) {

      // vif_flag
      js << "-vif_flag " << vif_flag << " ";
   }

   return(js);
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::process_mask_grid() {

   //
   // Locate the requested grid
   //
   if(!find_grid_by_name(mask_grid, grid_mask)) {
      mlog << Error << "\nSTATAnalysisJob::process_mask_grid() -> "
           << "Can't find requested masking grid name \""
           << mask_grid << "\".\n\n";

      throw(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// To apply a masking polyline, the -mask_grid option must first be
// specified to define the grid to which the polyline should applied.
// The lat/lon masking polyline is then converted to grid x/y.
//
////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::process_mask_poly() {

   poly_mask.clear();
   poly_mask.load(mask_poly);

   return;
}

////////////////////////////////////////////////////////////////////////

int STATAnalysisJob::is_in_mask_grid(double lat, double lon) const {
   int r = 1;
   double x, y;
   int x_int, y_int;

   //
   // Only check if a masking grid has been specified
   //
   if(mask_grid) {

      // Convert degrees_east to degrees_west.
      grid_mask.latlon_to_xy(lat, -1.0*lon, x, y);
      x_int = nint(x);
      y_int = nint(y);

      //
      // Convert lat/lon to grid x/y and check to see if it's on the
      // grid.
      //
      if(x_int < 0 || x_int >= grid_mask.nx() ||
         y_int < 0 || y_int >= grid_mask.ny()) r = 0;
   }

   return(r);
}

////////////////////////////////////////////////////////////////////////

int STATAnalysisJob::is_in_mask_poly(double lat, double lon) const {
   int r = 1;

   //
   // Only check if a masking poly has been specified
   //
   if(mask_poly) r = poly_mask.latlon_is_inside_dege(lat, lon);

   return(r);
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

const char * statjobtype_to_string(const STATJobType t) {
   return(statjobtype_str[t]);
}

////////////////////////////////////////////////////////////////////////

void statjobtype_to_string(const STATJobType t, char *out) {

   strcpy(out, statjobtype_to_string(t));

   return;
}

////////////////////////////////////////////////////////////////////////

STATJobType string_to_statjobtype(const char *str) {
   STATJobType t;

   if(     strcasecmp(str, statjobtype_str[0]) == 0)
      t = stat_job_filter;
   else if(strcasecmp(str, statjobtype_str[1]) == 0)
      t = stat_job_summary;
   else if(strcasecmp(str, statjobtype_str[2]) == 0)
      t = stat_job_aggr;
   else if(strcasecmp(str, statjobtype_str[3]) == 0)
      t = stat_job_aggr_stat;
   else if(strcasecmp(str, statjobtype_str[4]) == 0)
      t = stat_job_go_index;
   else if(strcasecmp(str, statjobtype_str[5]) == 0)
      t = stat_job_ss_index;
   else if(strcasecmp(str, statjobtype_str[6]) == 0)
      t = stat_job_ramp;
   else
      t = no_stat_job_type;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString timestring(const unixtime t) {
   ConcatString s;
   int month, day, year, hour, minute, second;

   unix_to_mdyhms(t, month, day, year, hour, minute, second);

   s.format("%s %d, %d   %02d:%02d:%02d",
            short_month_name[month], day, year,
            hour, minute, second);

   return(s);
}

////////////////////////////////////////////////////////////////////////

bool check_thresh_column(const ThreshArray &list, const ThreshArray &item) {

   // Return true for an empty search list.
   if(list.n_elements() == 0) return(true);

   // If the item is a single threshold, search for it in the list.
   if(item.n_elements() == 1) {
      return(list.has(item[0]));
   }

   // Otherwise, check that the list and item exactly match.
   return(list == item);
}

////////////////////////////////////////////////////////////////////////
