// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "vx_math/vx_math.h"
#include "vx_met_util/stat_columns.h"
#include "vx_met_util/apply_mask.h"
#include "vx_analysis_util/analysis_utils.h"
#include "vx_analysis_util/stat_job.h"

////////////////////////////////////////////////////////////////////////

static void timestring(const unixtime t, char * out);

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
   column    = (char *)     0;
   mask_grid = (char *)     0;
   mask_poly = (char *)     0;
   boot_rng  = (char *)     0;
   boot_seed = (char *)     0;
   tmp_dir   = (char *)     0;

   model.set_ignore_case(1);
   fcst_var.set_ignore_case(1);
   obs_var.set_ignore_case(1);
   fcst_lev.set_ignore_case(1);
   obs_lev.set_ignore_case(1);
   obtype.set_ignore_case(1);
   vx_mask.set_ignore_case(1);
   interp_mthd.set_ignore_case(1);
   line_type.set_ignore_case(1);
   column_min_name.set_ignore_case(1);
   column_max_name.set_ignore_case(1);

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::clear() {

   job_type = no_stat_job_type;

   model.clear();

   fcst_lead.clear();
   obs_lead.clear();

   fcst_valid_beg = fcst_valid_end = (unixtime) 0;
   obs_valid_beg  = obs_valid_end  = (unixtime) 0;
   fcst_init_beg  = fcst_init_end  = (unixtime) 0;
   obs_init_beg   = obs_init_end   = (unixtime) 0;

   fcst_init_hour.clear();
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

   alpha.clear();

   line_type.clear();

   column_min_name.clear();
   column_min_value.clear();

   column_max_name.clear();
   column_max_value.clear();

   if(dump_row) { delete [] dump_row; dump_row = (char *)    0; }
   if(column)   { delete [] column;   column   = (char *)    0; }

   close_dump_row_file();

   if(mask_grid) { delete [] mask_grid; mask_grid = (char *) 0; }
   if(mask_poly) { delete [] mask_poly; mask_poly = (char *) 0; }

   out_line_type = no_stat_line_type;

   out_fcst_thresh.clear();
   out_obs_thresh.clear();

   // Set to default values
   out_alpha      = default_alpha;
   boot_interval  = default_boot_interval;
   boot_rep_prop  = default_boot_rep_prop;
   n_boot_rep     = default_n_boot_rep;
   set_boot_rng (default_boot_rng);
   set_boot_seed(default_boot_seed);
   rank_corr_flag = default_rank_corr_flag;

   set_tmp_dir(default_tmp_dir);
   poly_mask.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::assign(const STATAnalysisJob & aj) {

   clear();

   job_type         = aj.job_type;

   model            = aj.model;

   fcst_lead        = aj.fcst_lead;
   obs_lead         = aj.obs_lead;

   fcst_valid_beg   = aj.fcst_valid_beg;
   fcst_valid_end   = aj.fcst_valid_end;
   obs_valid_beg    = aj.obs_valid_beg;
   obs_valid_end    = aj.obs_valid_end;
   fcst_init_beg    = aj.fcst_init_beg;
   fcst_init_end    = aj.fcst_init_end;
   obs_init_beg     = aj.obs_init_beg;
   obs_init_end     = aj.obs_init_end;

   fcst_init_hour   = aj.fcst_init_hour;
   obs_init_hour    = aj.obs_init_hour;

   fcst_var         = aj.fcst_var;
   obs_var          = aj.obs_var;

   fcst_lev         = aj.fcst_lev;
   obs_lev          = aj.obs_lev;

   obtype           = aj.obtype;

   vx_mask          = aj.vx_mask;

   interp_mthd      = aj.interp_mthd;
   interp_pnts      = aj.interp_pnts;

   fcst_thresh      = aj.fcst_thresh;
   obs_thresh       = aj.obs_thresh;
   cov_thresh       = aj.cov_thresh;

   alpha            = aj.alpha;

   line_type        = aj.line_type;

   column_min_name  = aj.column_min_name;
   column_min_value = aj.column_min_value;

   column_max_name  = aj.column_max_name;
   column_max_value = aj.column_max_value;

   out_line_type    = aj.out_line_type;

   out_fcst_thresh  = aj.out_fcst_thresh;
   out_obs_thresh   = aj.out_obs_thresh;
   out_alpha        = aj.out_alpha;

   boot_interval    = aj.boot_interval;
   boot_rep_prop    = aj.boot_rep_prop;
   n_boot_rep       = aj.n_boot_rep;

   rank_corr_flag   = aj.rank_corr_flag;

   set_dump_row (aj.dump_row);
   set_column   (aj.column);
   set_mask_grid(aj.mask_grid);
   set_mask_poly(aj.mask_poly);
   set_boot_rng (aj.boot_rng);
   set_boot_seed(aj.boot_seed);
   set_tmp_dir  (aj.tmp_dir);

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::dump(ostream & out, int depth) const {
   Indent prefix(depth);
   char junk[512];

   statjobtype_to_string(job_type, junk);
   out << prefix << "job type = " << junk << "\n";

   out << prefix << "model ...\n";
   model.dump(out, depth + 1);

   out << prefix << "fcst_lead ...\n";
   fcst_lead.dump(out, depth + 1);

   out << prefix << "obs_lead ...\n";
   obs_lead.dump(out, depth + 1);

   timestring(fcst_valid_beg, junk);
   out << prefix << "fcst_valid_beg = "
       << prefix << junk << "\n";

   timestring(fcst_valid_end, junk);
   out << prefix << "fcst_valid_end = "
       << prefix << junk << "\n";

   timestring(obs_valid_beg, junk);
   out << prefix << "obs_valid_beg = "
       << prefix << junk << "\n";

   timestring(obs_valid_end, junk);
   out << prefix << "obs_valid_end = "
       << prefix << junk << "\n";

   timestring(fcst_init_beg, junk);
   out << prefix << "fcst_init_beg = "
       << prefix << junk << "\n";

   timestring(fcst_init_end, junk);
   out << prefix << "fcst_init_end = "
       << prefix << junk << "\n";

   timestring(obs_init_beg, junk);
   out << prefix << "obs_init_beg = "
       << prefix << junk << "\n";

   timestring(obs_init_end, junk);
   out << prefix << "obs_init_end = "
       << prefix << junk << "\n";

   out << prefix << "fcst_init_hour ...\n";
   fcst_init_hour.dump(out, depth + 1);

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

   out << prefix << "alpha ...\n";
   alpha.dump(out, depth + 1);

   out << prefix << "line_type ...\n";
   line_type.dump(out, depth + 1);

   out << prefix << "column_min_name ...\n";
   column_min_name.dump(out, depth + 1);

   out << prefix << "column_min_value ...\n";
   column_min_value.dump(out, depth + 1);

   out << prefix << "column_max_name ...\n";
   column_max_name.dump(out, depth + 1);

   out << prefix << "column_max_value ...\n";
   column_max_value.dump(out, depth + 1);

   out << prefix << "dump_row = " << prefix
       << dump_row << "\n";

   out << prefix << "column = " << prefix
       << column << "\n";

   out << prefix << "mask_grid = " << prefix
       << mask_grid << "\n";

   out << prefix << "mask_poly = " << prefix
       << mask_poly << "\n";

   statlinetype_to_string(out_line_type, junk);
   out << prefix << "out_line_type = " << prefix
       << junk << "\n";

   out << prefix << "out_fcst_thresh ...\n";
   out_fcst_thresh.dump(out, depth + 1);

   out_obs_thresh.get_str(junk);
   out << prefix << "out_obs_thresh = " << prefix
       << junk << "\n";

   out << prefix << "out_alpha = " << prefix
       << out_alpha << "\n";

   out << prefix << "boot_interval = " << prefix
       << boot_interval << "\n";

   out << prefix << "boot_rep_prop = " << prefix
       << boot_rep_prop << "\n";

   out << prefix << "n_boot_rep = " << prefix
       << n_boot_rep << "\n";

   out << prefix << "boot_rng = " << prefix
       << boot_rng << "\n";

   out << prefix << "boot_seed = " << prefix
       << boot_seed << "\n";

   out << prefix << "rank_corr_flag = " << prefix
       << rank_corr_flag << "\n";

   out << prefix << "tmp_dir = " << prefix
       << tmp_dir << "\n";

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

int STATAnalysisJob::is_keeper(const STATLine & L) const {
   int j, h, found, n, c;
   double v;
   SingleThresh t;

   //
   // model
   //
   if(model.n_elements() > 0) {
      if(!(model.has(L.model()))) return(0);
   }

   //
   // fcst_lead (in seconds)
   //
   if(fcst_lead.n_elements() > 0) {

      n = fcst_lead.n_elements();
      h = L.fcst_lead();

      found = 0;
      for(j=0; j<n; ++j) {
         if(h == (int) fcst_lead[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
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
   // obs_lead (in seconds)
   //
   if(obs_lead.n_elements() > 0) {

      n = obs_lead.n_elements();
      h = L.obs_lead();

      found = 0;
      for(j=0; j<n; ++j) {
         if(h == (int) obs_lead[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
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
   // fcst_init_hour: if specified for the job, the fcst_init_beg and
   //                 fcst_init_end times must match
   //
   if(fcst_init_hour.n_elements() > 0) {

      // Check that fcst_init_beg = fcst_init_end
      if(L.fcst_init_beg() != L.fcst_init_end()) return(0);

      n = fcst_init_hour.n_elements();
      h = L.fcst_init_hour();

      found = 0;
      for(j=0; j<n; ++j) {
         if(h == (int) fcst_init_hour[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
   }

   //
   // obs_init_hour: if specified for the job, the obs_init_beg and
   //                obs_init_end times must match
   //
   if(obs_init_hour.n_elements() > 0) {

      // Check that obs_init_beg = obs_init_end
      if(L.obs_init_beg() != L.obs_init_end()) return(0);

      n = obs_init_hour.n_elements();
      h = L.obs_init_hour();

      found = 0;
      for(j=0; j<n; ++j) {
         if(h == (int) obs_init_hour[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
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

      n = interp_pnts.n_elements();
      h = L.interp_pnts();

      found = 0;
      for(j=0; j<n; ++j) {
         if(h == (int) interp_pnts[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
   }

   //
   // fcst_thresh
   //
   if(fcst_thresh.n_elements() > 0) {

      n = fcst_thresh.n_elements();
      t = L.fcst_thresh();

      found = 0;
      for(j=0; j<n; ++j) {
         if(t == fcst_thresh[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
   }

   //
   // obs_thresh
   //
   if(obs_thresh.n_elements() > 0) {

      n = obs_thresh.n_elements();
      t = L.obs_thresh();

      found = 0;
      for(j=0; j<n; ++j) {
         if(t == obs_thresh[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
   }

   //
   // cov_thresh
   //
   if(cov_thresh.n_elements() > 0) {

      n = cov_thresh.n_elements();
      t = L.cov_thresh();

      found = 0;
      for(j=0; j<n; ++j) {
         if(t == cov_thresh[j]) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
   }

   //
   // alpha
   //
   if(alpha.n_elements() > 0) {

      n = alpha.n_elements();
      v = L.alpha();

      found = 0;
      for(j=0; j<n; ++j) {
         if(is_eq(v, (double) alpha[j])) {
            found = 1;
            break;
         }
      }
      if(!found) return(0);
   }

   //
   // line_type
   //
   if(line_type.n_elements() > 0) {
      if(!(line_type.has(L.line_type()))) return(0);
   }

   //
   // column_min
   //
   if(column_min_name.n_elements() > 0) {

      n = column_min_name.n_elements();

      for(j=0; j<n; ++j) {

         //
         // Determine the column offset and retrieve the value
         //
         c = determine_column_offset(L.type(), column_min_name[j]);
         v = atof(L.get_item(c));

         //
         // Check if the column's value is bad data or is not in the
         // acceptable range
         //
         if(is_bad_data(v) ||
            v < column_min_value[j]) return(0);
      }
   }

   //
   // column_max
   //
   if(column_max_name.n_elements() > 0) {

      n = column_max_name.n_elements();

      for(j=0; j<n; ++j) {

         //
         // Determine the column offset and retrieve the value
         //
         c = determine_column_offset(L.type(), column_max_name[j]);
         v = atof(L.get_item(c));

         //
         // Check if the column's value is bad data or is not in the
         // acceptable range
         //
         if(is_bad_data(v) ||
            v > column_max_value[j]) return(0);
      }
   }

   return(1);
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::parse_job_command(const char *jobstring) {
   char *line = (char *) 0;
   char *c    = (char *) 0;
   char *lp   = (char *) 0;
   const char delim [] = " ";
   int n = strlen(jobstring);
   int i, k;

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

      if(     strcmp(jc_array[i], "-model"         ) == 0)
         model.clear();
      else if(strcmp(jc_array[i], "-fcst_lead"     ) == 0)
         fcst_lead.clear();
      else if(strcmp(jc_array[i], "-obs_lead"      ) == 0)
         obs_lead.clear();
      else if(strcmp(jc_array[i], "-fcst_init_hour") == 0)
         fcst_init_hour.clear();
      else if(strcmp(jc_array[i], "-obs_init_hour" ) == 0)
         obs_init_hour.clear();
      else if(strcmp(jc_array[i], "-fcst_var"      ) == 0)
         fcst_var.clear();
      else if(strcmp(jc_array[i], "-fcst_lev"      ) == 0)
         fcst_lev.clear();
      else if(strcmp(jc_array[i], "-obs_var"       ) == 0)
         obs_var.clear();
      else if(strcmp(jc_array[i], "-obs_lev"       ) == 0)
         obs_lev.clear();
      else if(strcmp(jc_array[i], "-obtype"        ) == 0)
         obtype.clear();
      else if(strcmp(jc_array[i], "-vx_mask"       ) == 0)
         vx_mask.clear();
      else if(strcmp(jc_array[i], "-interp_mthd"   ) == 0)
         interp_mthd.clear();
      else if(strcmp(jc_array[i], "-interp_pnts"   ) == 0)
         interp_pnts.clear();
      else if(strcmp(jc_array[i], "-fcst_thresh"   ) == 0)
         fcst_thresh.clear();
      else if(strcmp(jc_array[i], "-obs_thresh"    ) == 0)
         obs_thresh.clear();
      else if(strcmp(jc_array[i], "-cov_thresh"    ) == 0)
         cov_thresh.clear();
      else if(strcmp(jc_array[i], "-alpha"         ) == 0)
         alpha.clear();
      else if(strcmp(jc_array[i], "-line_type"     ) == 0)
         line_type.clear();
      else if(strcmp(jc_array[i], "-column_min"    ) == 0) {
         column_min_name.clear();
         column_min_value.clear();
      }
      else if(strcmp(jc_array[i], "-column_max"    ) == 0) {
         column_max_name.clear();
         column_max_value.clear();
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
            cerr << "\n\nERROR: parse_job_command() -> "
                 << "unrecognized job type specified \"" << jc_array[i]
                 << "\" in job command line: " << jobstring << "\n\n"
                 << flush;

            throw(1);
         }
         i++;
      }
      else if(strcmp(jc_array[i], "-model") == 0) {
         model.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_lead") == 0) {
         k = timestring_to_sec(jc_array[i+1]);
         fcst_lead.add(k);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_lead") == 0) {
         k = timestring_to_sec(jc_array[i+1]);
         obs_lead.add(k);
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
      else if(strcmp(jc_array[i], "-obs_valid_beg") == 0) {
         obs_valid_beg = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_valid_end") == 0) {
         obs_valid_end = timestring_to_unix(jc_array[i+1]);
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
      else if(strcmp(jc_array[i], "-obs_init_beg") == 0) {
         obs_init_beg = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_init_end") == 0) {
         obs_init_end = timestring_to_unix(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_init_hour") == 0) {
         k = timestring_to_sec(jc_array[i+1]);
         fcst_init_hour.add(k);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_init_hour") == 0) {
         k = timestring_to_sec(jc_array[i+1]);
         obs_init_hour.add(k);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_var") == 0) {
         fcst_var.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_lev") == 0) {
         fcst_lev.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_var") == 0) {
         obs_var.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_lev") == 0) {
         obs_lev.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obtype") == 0) {
         obtype.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-vx_mask") == 0) {
         vx_mask.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-interp_mthd") == 0) {
         interp_mthd.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-interp_pnts") == 0) {
         interp_pnts.add(atoi(jc_array[i+1]));
         i++;
      }
      else if(strcmp(jc_array[i], "-fcst_thresh") == 0) {
         fcst_thresh.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-obs_thresh") == 0) {
         obs_thresh.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-cov_thresh") == 0) {
         cov_thresh.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-alpha") == 0) {
         alpha.add(atof(jc_array[i+1]));
         i++;
      }
      else if(strcmp(jc_array[i], "-line_type") == 0) {
         line_type.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-column_min") == 0) {
         column_min_name.add(jc_array[i+1]);
         column_min_value.add(atof(jc_array[i+2]));
         i+=2;
      }
      else if(strcmp(jc_array[i], "-column_max") == 0) {
         column_max_name.add(jc_array[i+1]);
         column_max_value.add(atof(jc_array[i+2]));
         i+=2;
      }
      else if(strcmp(jc_array[i], "-dump_row") == 0) {
         set_dump_row(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-column") == 0) {
         set_column(jc_array[i+1]);
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
         out_line_type = string_to_statlinetype(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_fcst_thresh") == 0) {
         out_fcst_thresh.add(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_obs_thresh") == 0) {
         out_obs_thresh.set(jc_array[i+1]);
         i++;
      }
      else if(strcmp(jc_array[i], "-out_alpha") == 0) {
         out_alpha = atof(jc_array[i+1]);
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
      else if(strcmp(jc_array[i], "-tmp_dir") == 0) {
         set_tmp_dir(jc_array[i+1]);
         i++;
      }
      else {
         cerr << "\n\nERROR: parse_job_command() -> "
              << "unrecognized switch \"" << jc_array[i]
              << "\" in job command line: "
              << jobstring << "\n\n" << flush;

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

void STATAnalysisJob::set_column(const char *c) {

   if(column) { delete [] column; column = (char *) 0; }

   if(!c) return;

   column = new char [strlen(c) + 1];

   strcpy(column, c);

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

void STATAnalysisJob::set_tmp_dir(const char *c) {

   if(tmp_dir) { delete [] tmp_dir; tmp_dir = (char *) 0; }

   if(!c) return;

   tmp_dir = new char [strlen(c) + 1];

   strcpy(tmp_dir, c);

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::open_dump_row_file() {

   close_dump_row_file();

   if(!dump_row) return;

   dr_out = new ofstream;
   dr_out->open(dump_row);

   if(!dr_out) {
      cerr << "\n\nERROR: open_dump_row_file()-> "
           << "can't open the output file \"" << dump_row
           << "\" for writing!\n\n" << flush;

      throw(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::close_dump_row_file() {

   if(dr_out) {
      dr_out->close();
      delete dr_out;
      dr_out = (ofstream *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// The pointer passed in must contain enough space to hold the contents
// of the jobstring to be computed.
//
////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::get_jobstring(char *js) {
   int i;
   char junk[512];
   STATLineType type;

   // Initialize the jobstring
   strcpy(js, "");

   // job type
   statjobtype_to_string(job_type, junk);
   sprintf(js, "-job %s", junk);

   // model
   if(model.n_elements() > 0) {
      for(i=0; i<model.n_elements(); i++)
         sprintf(js, "%s -model %s", js, model[i]);
   }

   // fcst_lead
   if(fcst_lead.n_elements() > 0) {
      for(i=0; i<fcst_lead.n_elements(); i++) {
         sec_to_hhmmss(nint(fcst_lead[i]), junk);
         sprintf(js, "%s -fcst_lead %s", js, junk);
      }
   }

   // obs_lead
   if(obs_lead.n_elements() > 0) {
      for(i=0; i<obs_lead.n_elements(); i++) {
         sec_to_hhmmss(nint(obs_lead[i]), junk);
         sprintf(js, "%s -obs_lead %s", js, junk);
      }
   }

   // fcst_valid_beg and fcst_valid_end
   if(fcst_valid_beg > 0) {
      unix_to_yyyymmdd_hhmmss(fcst_valid_beg, junk);
      sprintf(js, "%s -fcst_valid_beg %s", js, junk);
   }
   if(fcst_valid_end > 0) {
      unix_to_yyyymmdd_hhmmss(fcst_valid_end, junk);
      sprintf(js, "%s -fcst_valid_end %s", js, junk);
   }

   // obs_valid_beg and obs_valid_end
   if(obs_valid_beg > 0) {
      unix_to_yyyymmdd_hhmmss(obs_valid_beg, junk);
      sprintf(js, "%s -obs_valid_beg %s", js, junk);
   }
   if(obs_valid_end > 0) {
      unix_to_yyyymmdd_hhmmss(obs_valid_end, junk);
      sprintf(js, "%s -obs_valid_end %s", js, junk);
   }

   // fcst_init_beg and fcst_init_end
   if(fcst_init_beg > 0) {
      unix_to_yyyymmdd_hhmmss(fcst_init_beg, junk);
      sprintf(js, "%s -fcst_init_beg %s", js, junk);
   }
   if(fcst_init_end > 0) {
      unix_to_yyyymmdd_hhmmss(fcst_init_end, junk);
      sprintf(js, "%s -fcst_init_end %s", js, junk);
   }

   // obs_init_beg and obs_init_end
   if(obs_init_beg > 0) {
      unix_to_yyyymmdd_hhmmss(obs_init_beg, junk);
      sprintf(js, "%s -obs_init_beg %s", js, junk);
   }
   if(obs_init_end > 0) {
      unix_to_yyyymmdd_hhmmss(obs_init_end, junk);
      sprintf(js, "%s -obs_init_end %s", js, junk);
   }

   // fcst_init_hour
   if(fcst_init_hour.n_elements() > 0) {
      for(i=0; i<fcst_init_hour.n_elements(); i++) {
         sec_to_hhmmss(nint(fcst_init_hour[i]), junk);
         sprintf(js, "%s -fcst_init_hour %s", js, junk);
      }
   }

   // obs_init_hour
   if(obs_init_hour.n_elements() > 0) {
      for(i=0; i<obs_init_hour.n_elements(); i++) {
         sec_to_hhmmss(nint(obs_init_hour[i]), junk);
         sprintf(js, "%s -obs_init_hour %s", js, junk);
      }
   }

   // fcst_var
   if(fcst_var.n_elements() > 0) {
      for(i=0; i<fcst_var.n_elements(); i++)
         sprintf(js, "%s -fcst_var %s", js, fcst_var[i]);
   }

   // obs_var
   if(obs_var.n_elements() > 0) {
      for(i=0; i<obs_var.n_elements(); i++)
         sprintf(js, "%s -obs_var %s", js, obs_var[i]);
   }

   // fcst_lev
   if(fcst_lev.n_elements() > 0) {
      for(i=0; i<fcst_lev.n_elements(); i++)
         sprintf(js, "%s -fcst_lev %s", js, fcst_lev[i]);
   }

   // obs_lev
   if(obs_lev.n_elements() > 0) {
      for(i=0; i<obs_lev.n_elements(); i++)
         sprintf(js, "%s -obs_lev %s", js, obs_lev[i]);
   }

   // obtype
   if(obtype.n_elements() > 0) {
      for(i=0; i<obtype.n_elements(); i++)
         sprintf(js, "%s -obtype %s", js, obtype[i]);
   }

   // vx_mask
   if(vx_mask.n_elements() > 0) {
      for(i=0; i<vx_mask.n_elements(); i++)
         sprintf(js, "%s -vx_mask %s", js, vx_mask[i]);
   }

   // interp_mthd
   if(interp_mthd.n_elements() > 0) {
      for(i=0; i<interp_mthd.n_elements(); i++)
         sprintf(js, "%s -interp_mthd %s", js, interp_mthd[i]);
   }

   // interp_pnts
   if(interp_pnts.n_elements() > 0) {
      for(i=0; i<interp_pnts.n_elements(); i++)
         sprintf(js, "%s -interp_pnts %i", js, nint(interp_pnts[i]));
   }

   // fcst_thresh
   if(fcst_thresh.n_elements() > 0) {
      for(i=0; i<fcst_thresh.n_elements(); i++) {
         fcst_thresh[i].get_str(junk);
         sprintf(js, "%s -fcst_thresh %s", js, junk);
      }
   }

   // obs_thresh
   if(obs_thresh.n_elements() > 0) {
      for(i=0; i<obs_thresh.n_elements(); i++) {
         obs_thresh[i].get_str(junk);
         sprintf(js, "%s -obs_thresh %s", js, junk);
      }
   }

   // cov_thresh
   if(cov_thresh.n_elements() > 0) {
      for(i=0; i<cov_thresh.n_elements(); i++) {
         cov_thresh[i].get_str(junk);
         sprintf(js, "%s -cov_thresh %s", js, junk);
      }
   }

   // alpha
   if(alpha.n_elements() > 0) {
      for(i=0; i<alpha.n_elements(); i++)
         sprintf(js, "%s -alpha %f", js, alpha[i]);
   }

   // line_type
   if(line_type.n_elements() > 0) {
      for(i=0; i<line_type.n_elements(); i++) {
         type = string_to_statlinetype(line_type[i]);
         statlinetype_to_string(type, junk);
         sprintf(js, "%s -line_type %s", js, junk);
      }
   }

   // column_min
   if(column_min_name.n_elements() > 0) {
      for(i=0; i<column_min_name.n_elements(); i++)
         sprintf(js, "%s -column_min %s %f", js,
                 column_min_name[i], column_min_value[i]);
   }

   // column_max
   if(column_max_name.n_elements() > 0) {
      for(i=0; i<column_max_name.n_elements(); i++)
         sprintf(js, "%s -column_max %s %f", js,
                 column_max_name[i], column_max_value[i]);
   }

   // dump_row
   if(dump_row) sprintf(js, "%s -dump_row %s", js, dump_row);

   // column
   if(column) sprintf(js, "%s -column %s", js, column);

   // mask_grid
   if(mask_grid) sprintf(js, "%s -mask_grid %s", js, mask_grid);

   // mask_poly
   if(mask_poly) sprintf(js, "%s -mask_poly %s", js, mask_poly);

   // out_line_type
   if(out_line_type != no_stat_line_type) {
      statlinetype_to_string(out_line_type, junk);
      sprintf(js, "%s -out_line_type %s", js, junk);
   }

   // out_fcst_thresh
   if(out_fcst_thresh.n_elements() > 0) {
      for(i=0; i<out_fcst_thresh.n_elements(); i++) {
         out_fcst_thresh[i].get_str(junk);
         sprintf(js, "%s -out_fcst_thresh %s", js, junk);
      }
   }

   // out_obs_thresh
   if(out_obs_thresh.type != thresh_na) {
      out_obs_thresh.get_str(junk);
      sprintf(js, "%s -out_obs_thresh %s", js, junk);
   }

   // Jobs which use out_alpha
   if(job_type       == stat_job_summary ||
       out_line_type == stat_cts         ||
       out_line_type == stat_cnt         ||
       out_line_type == stat_pstd        ||
       out_line_type == stat_nbrcts      ||
       out_line_type == stat_nbrcnt) {

      // out_alpha
      sprintf(js, "%s -out_alpha %f", js, out_alpha);
   }

   // Jobs which perform bootstrapping
   if(line_type.n_elements() > 0) {
      type = string_to_statlinetype(line_type[0]);
      if(type           == stat_mpr    &&
         (out_line_type == stat_cts    ||
          out_line_type == stat_cnt    ||
          out_line_type == stat_nbrcts ||
          out_line_type == stat_nbrcnt)) {

         // Bootstrap Information
         sprintf(js, "%s -boot_interval %i", js, boot_interval);
         sprintf(js, "%s -boot_rep_prop %f", js, boot_rep_prop);
         sprintf(js, "%s -n_boot_rep %i",    js, n_boot_rep);
         sprintf(js, "%s -boot_rng %s",      js, boot_rng);
         sprintf(js, "%s -boot_seed %s",     js, boot_seed);
         sprintf(js, "%s -tmp_dir %s",       js, tmp_dir);
      }
   }

   // Jobs which compute rank correlations
   if(out_line_type == stat_cnt) {

      // rank_corr_flag
      sprintf(js, "%s -rank_corr_flag %i", js, rank_corr_flag);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void STATAnalysisJob::process_mask_grid() {

   //
   // Locate the requested grid
   //
   if(find_grid(mask_grid, grid_mask) != 0) {
      cerr << "\n\nERROR: process_mask_grid() -> "
           << "Can't find requested masking grid name \""
           << mask_grid << "\".\n\n" << flush;
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

      grid_mask.latlon_to_xy(lat, lon, x, y);
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
   if(mask_poly) r = poly_mask.latlon_is_inside(lat, lon);

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
   else
      t = no_stat_job_type;

   return(t);
}

////////////////////////////////////////////////////////////////////////

void timestring(const unixtime t, char * out) {
   int month, day, year, hour, minute, second;

   unix_to_mdyhms(t, month, day, year, hour, minute, second);

   sprintf(out, "%s %d, %d   %02d:%02d:%02d",
           short_month_name[month], day, year,
           hour, minute, second);

   return;
}

////////////////////////////////////////////////////////////////////////
