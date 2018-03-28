// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "obs_error.h"

////////////////////////////////////////////////////////////////////////

// Default observation error table file name
static const char default_obs_error_dir[] = "MET_BASE/table_files";

// Name of user-specified observation errror environment variable
static const char met_obs_error_tables[] =
   "MET_OBS_ERROR_TABLES";

static const int  n_obs_error_columns = 13;
static const char wildcard_str []     = "ALL";

////////////////////////////////////////////////////////////////////////

//
// Gloabal instance needs external linkage
//

ObsErrorTable obs_error_table;

////////////////////////////////////////////////////////////////////////
//
// Code for class ObsErrorEntry
//
////////////////////////////////////////////////////////////////////////

ObsErrorEntry::ObsErrorEntry() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ObsErrorEntry::~ObsErrorEntry() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ObsErrorEntry::ObsErrorEntry(const ObsErrorEntry & e) {

   init_from_scratch();

   assign(e);
}

////////////////////////////////////////////////////////////////////////

ObsErrorEntry & ObsErrorEntry::operator=(const ObsErrorEntry & e) {

   if(this == &e) return(*this);

   assign(e);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ObsErrorEntry::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorEntry::clear() {

   line_number = bad_data_int;

   var_name.clear();
   msg_type.clear();
   sid.clear();

   pb_rpt_type.clear();
   in_rpt_type.clear();
   inst_type.clear();

   hgt_range.clear();
   prs_range.clear();
   val_range.clear();

   bias_scale = bias_offset = bad_data_double;

   dist_type = DistType_None;
   dist_parm.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorEntry::assign(const ObsErrorEntry & e) {

   clear();

   line_number = e.line_number;

   var_name    = e.var_name;
   msg_type    = e.msg_type;
   sid         = e.sid;

   pb_rpt_type = e.pb_rpt_type;
   in_rpt_type = e.in_rpt_type;
   inst_type   = e.inst_type;

   hgt_range   = e.hgt_range;
   prs_range   = e.prs_range;
   val_range   = e.val_range;

   bias_scale  = e.bias_scale;
   bias_offset = e.bias_offset;

   dist_type   = e.dist_type;
   dist_parm   = e.dist_parm;

   return;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorEntry::dump(ostream & out, int depth) const {

   Indent prefix(depth);

   out << prefix << "ObsErrorEntry ... ";
   out << prefix << "line_number = " << line_number << "\n";
   out << prefix << "var_name: ";
   var_name.dump(out, depth+1);
   out << prefix << "msg_type: ";
   msg_type.dump(out, depth+1);
   out << prefix << "sid: ";
   sid.dump(out, depth+1);
   out << prefix << "pb_rpt_type: ";
   pb_rpt_type.dump(out, depth+1);
   out << prefix << "in_rpt_type: ";
   in_rpt_type.dump(out, depth+1);
   out << prefix << "inst_type: ";
   inst_type.dump(out, depth+1);
   out << prefix << "hgt_range: ";
   hgt_range.dump(out, depth+1);
   out << prefix << "prs_range: ";
   prs_range.dump(out, depth+1);
   out << prefix << "val_range: ";
   val_range.dump(out, depth+1);
   out << prefix << "bias_scale = " << bias_scale << "\n";
   out << prefix << "bias_offset = " << bias_offset << "\n";
   out << prefix << "dist_type = " << disttype_to_string(dist_type) << "\n";
   out << prefix << "dist_parm: ";
   dist_parm.dump(out, depth+1);

   return;
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorEntry::parse_line(const DataLine &dl) {

   // Initialize
   clear();

   // Check for blank line or header
   if(dl.n_items() == 0 || is_header(dl)) return(false);

   // Check for expected number of elements
   if(dl.n_items() != n_obs_error_columns) {
      mlog << Error << "\nObsErrorEntry::parse_line() -> "
           << "unexpected number of columns (" << dl.n_items()
           << " != " << n_obs_error_columns << " on line number "
           << dl.line_number() << " of file:\n"
           << dl.get_file()->filename() << "\n\n";
      exit(1);
   }

   line_number = dl.line_number();

   // Filtering parameters
   if(strcasecmp(dl[0],  wildcard_str) != 0)  var_name.parse_css(dl[0]);
   if(strcasecmp(dl[1],  wildcard_str) != 0)  msg_type.parse_css(dl[1]);
   if(strcasecmp(dl[5],  wildcard_str) != 0)       sid.parse_css(dl[2]);
   if(strcasecmp(dl[2],  wildcard_str) != 0) pb_rpt_type.add_css(dl[3]);
   if(strcasecmp(dl[3],  wildcard_str) != 0) in_rpt_type.add_css(dl[4]);
   if(strcasecmp(dl[4],  wildcard_str) != 0)   inst_type.add_css(dl[5]);
   if(strcasecmp(dl[6],  wildcard_str) != 0)   hgt_range.add_css(dl[6]);
   if(strcasecmp(dl[7],  wildcard_str) != 0)   prs_range.add_css(dl[7]);
   if(strcasecmp(dl[8],  wildcard_str) != 0)   val_range.add_css(dl[8]);

   // Observation error adjustments
   bias_scale = (strcmp(dl[9], na_str) == 0 ?
                 bad_data_double : atof(dl[9]));
   bias_offset = (strcmp(dl[10], na_str) == 0 ?
                  bad_data_double : atof(dl[10]));
   dist_type = string_to_disttype(dl[11]);
   if(dist_type != DistType_None) dist_parm.add_css(dl[12]);

   // Range check
   if((hgt_range.n() != 0 && hgt_range.n() != 2) ||
      (prs_range.n() != 0 && prs_range.n() != 2) ||
      (val_range.n() != 0 && val_range.n() != 2)) {
      mlog << Error << "\nObsErrorEntry::validate() -> "
           << "the HGT_RANGE, PRS_RANGE, and VAL_RANGE columns must be "
           << "set to \"" << wildcard_str << "\" or \"BEG,END\" to "
           << "specify the range of values on line number "
           << dl.line_number() << " of file:\n"
           << dl.get_file()->filename() << "\n\n";
      exit(1);
   }

   validate();

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorEntry::is_header(const DataLine &dl) {

   if(dl.n_items() > 0) {
      if(strcasecmp(dl[0], "OBS_VAR") == 0) return(true);
   }

   return(false);
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorEntry::is_match(const char *cur_var_name,
                             const char *cur_msg_type,
                             const char *cur_sid,
                             int cur_pb_rpt,
                             int cur_in_rpt,
                             int cur_inst,
                             double cur_hgt,
                             double cur_prs,
                             double cur_val) {

   // Check array filters
   if(var_name.n()    > 0 && !var_name.has(cur_var_name))  return(false);
   if(msg_type.n()    > 0 && !msg_type.has(cur_msg_type))  return(false);
   if(sid.n()         > 0 && !sid.has(cur_sid))            return(false);
   if(pb_rpt_type.n() > 0 && !pb_rpt_type.has(cur_pb_rpt)) return(false);
   if(in_rpt_type.n() > 0 && !in_rpt_type.has(cur_in_rpt)) return(false);
   if(inst_type.n()   > 0 && !inst_type.has(cur_inst))     return(false);

   // Check ranges
   if(!is_bad_data(cur_hgt) && hgt_range.n() == 2) {
       if(cur_hgt < hgt_range[0] || cur_hgt > hgt_range[1]) return(false);
   }
   if(!is_bad_data(cur_prs) && prs_range.n() == 2) {
       if(cur_prs < prs_range[0] || cur_prs > prs_range[1]) return(false);
   }
   if(!is_bad_data(cur_val) && val_range.n() == 2) {
       if(cur_val < val_range[0] || cur_val > val_range[1]) return(false);
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

void ObsErrorEntry::validate() {
   int n_req;

   // Number of distribution parameters
   if(dist_type == DistType_Gamma   ||
      dist_type == DistType_Uniform ||
      dist_type == DistType_Beta) n_req = 2;
   else                           n_req = 1;

   // Make sure we have the expected number of parameters
   if(dist_type != DistType_None &&
      dist_parm.n() != n_req) {
      mlog << Error << "\nObsErrorEntry::validate() -> "
           << "expected " << n_req << " parameter(s) but got "
           << dist_parm.n() << " for the "
           << disttype_to_string(dist_type) << " distribution.\n\n";
      exit(1);
   }

   // Pad with bad data out to length 2 to simplify later logic
   while(dist_parm.n() < 2) dist_parm.add(bad_data_double);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class ObsErrorTable
//
////////////////////////////////////////////////////////////////////////

ObsErrorTable::ObsErrorTable() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ObsErrorTable::~ObsErrorTable() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ObsErrorTable::ObsErrorTable(const ObsErrorTable &f) {

   init_from_scratch();

   assign(f);
}

////////////////////////////////////////////////////////////////////////

void ObsErrorTable::init_from_scratch() {

   e = (ObsErrorEntry *) 0;

   clear();
}

////////////////////////////////////////////////////////////////////////

void ObsErrorTable::clear() {
   int i;

   if(e) { delete [] e; e = (ObsErrorEntry *) 0; }

   IsSet      = false;
   N_elements = 0;
   N_alloc    = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorTable::dump(ostream & out, int depth) const {
   int i;
   Indent prefix(depth);

   out << prefix << "N_elements = " << N_elements << "\n";

   for(i=0; i<N_elements; i++) {
      out << prefix << "ObsErrorTable Entry # " << i+1 << " ...\n";
      e[i].dump(out, depth + 1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorTable::assign(const ObsErrorTable & f) {
   int i;

   clear();

   if(f.N_elements != 0 )  {

      IsSet = true;

      N_elements = N_alloc = f.N_elements;

      e = new ObsErrorEntry [N_elements];

      for(i=0; i<N_elements; i++) e[i] = f.e[i];
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorTable::extend(int n) {

   if(n <= N_alloc )  return;

   int i;
   ObsErrorEntry * u = (ObsErrorEntry *) 0;

   u = new ObsErrorEntry [n];

   for(i=0; i<N_elements; i++) u[i] = e[i];

   e = u;

   u = (ObsErrorEntry *) 0;

   N_alloc = n;

   return;
}


////////////////////////////////////////////////////////////////////////

void ObsErrorTable::initialize() {
   ConcatString path;
   ConcatString desc;
   StringArray file_names;
   char *ptr;

   //
   // Use MET_OBS_ERROR_TABLES, if set
   //
   ptr = getenv(met_obs_error_tables);
   if((ptr = getenv(met_obs_error_tables)) != NULL) {
      path = ptr;
      desc << "user-defined " << met_obs_error_tables;
   }
   //
   // Otherwise, read the default table file
   //
   else {
      path = replace_path(default_obs_error_dir);
      desc = "default observation error table";
   }

   // Search for file input file names
   file_names = get_filenames(path, "obs_error", ".txt", true);

   for(int i=0; i<file_names.n_elements(); i++) {

      mlog << Debug(1)
           << "Reading " << desc << " file: " << file_names[i] << "\n";

      if(!read(file_names[i])) {
         mlog << Error << "\nObsErrorTable::initialize() -> "
              << "unable to read " << desc << " file \""
              << file_names[i] << "\"\n\n";
         exit(1);
      }
   }

   IsSet = true;

   return;
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorTable::read(const char * file_name) {
   LineDataFile f;
   DataLine dl;
   ObsErrorEntry cur;

   if(!f.open(file_name)) {
      mlog << Warning << "ObsErrorTable::read() -> "
           << "unable to open input file \"" << file_name << "\"\n\n";
      return(false);
   }

   //
   // Allocate space for all the lines in this file
   //
   extend(N_elements + file_linecount(file_name));

   //
   // Read each line of the file
   //
   while(f >> dl) {
      if(cur.parse_line(dl)) {
         e[N_elements] = cur;
         N_elements++;
      }
   }

   f.close();

   return(true);
}

////////////////////////////////////////////////////////////////////////

ObsErrorEntry *ObsErrorTable::lookup(
   const char *cur_var_name, const char *cur_msg_type, const char *cur_sid,
   int cur_pb_rpt,           int cur_in_rpt,           int cur_inst,
   double cur_hgt,           double cur_prs,           double cur_val) {
   int i;
   ObsErrorEntry * e_match = (ObsErrorEntry *) 0;

   for(i=0; i<N_elements; i++) {

      if(e[i].is_match(cur_var_name, cur_msg_type, cur_sid,
                       cur_pb_rpt,   cur_in_rpt,   cur_inst,
                       cur_hgt,      cur_prs,      cur_val)) {
         e_match = &e[i];
         break;
      }
   }

   // Check for no match
   if(e_match == (ObsErrorEntry *) 0) {
      mlog << Warning << "\nObsErrorTable::lookup() -> "
           << "no observation error table match found for "
           << "var_name = \"" << cur_var_name
           << "\", msg_type = \"" << cur_msg_type
           << "\", sid = \"" << cur_sid
           << ", pb_rpt_typ = " << cur_pb_rpt
           << ", in_rpt_typ = " << cur_in_rpt
           << ", inst_typ = " << cur_inst
           << ", hgt = " << cur_hgt
           << ", prs = " << cur_prs
           << ", val = " << cur_val << "\n\n";
   }

   return(e_match);
}

////////////////////////////////////////////////////////////////////////

ObsErrorEntry *ObsErrorTable::lookup(
   const char *cur_var_name, const char *cur_msg_type, double cur_val) {
   int i;
   ObsErrorEntry * e_match = (ObsErrorEntry *) 0;

   for(i=0; i<N_elements; i++) {

      if(e[i].is_match(   cur_var_name,    cur_msg_type, bad_data_str,
                          bad_data_int,    bad_data_int, bad_data_int,
                       bad_data_double, bad_data_double, cur_val)) {
         e_match = &e[i];
         break;
      }
   }

   // Check for no match
   if(e_match == (ObsErrorEntry *) 0) {
      mlog << Warning << "\nObsErrorTable::lookup() -> "
           << "no observation error table match found for "
           << "var_name = \"" << cur_var_name
           << "\", msg_type = \"" << cur_msg_type
           << "\", val = " << cur_val << "\n\n";
   }

   return(e_match);
}

////////////////////////////////////////////////////////////////////////
//
// Code for struct ObsErrorInfo struct
//
////////////////////////////////////////////////////////////////////////

void ObsErrorInfo::clear() {
   field = FieldType_None;
   entry.clear();
   rng_ptr = (gsl_rng *) 0;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorInfo::validate() {

   // If set to NONE, no work to do
   if(field == FieldType_None) return;

   // Validate the ObsErrorEntry object
   entry.validate();

   // Make sure the rng_ptr is set
   if(rng_ptr == (gsl_rng *) 0) {
      mlog << Error << "\nObsErrorInfo::validate() -> "
           << "random number generator pointer is not set!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

ObsErrorInfo parse_conf_obs_error(Dictionary *dict, gsl_rng *rng_ptr) {
   Dictionary *err_dict = (Dictionary *) 0;
   ObsErrorInfo info;
   int i;
   double d;

   if(!dict) {
      mlog << Error << "\nparse_conf_obs_error() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Initialize
   info.clear();

   // Conf: obs_error
   err_dict = dict->lookup_dictionary(conf_key_obs_error);

   // Conf: field
   info.field = int_to_fieldtype(err_dict->lookup_int(conf_key_field));

   // If set to NONE, no work to do
   if(info.field == FieldType_None) return(info);

   // Conf: dist_type - optional
   i = err_dict->lookup_int(conf_key_dist_type, false);
   if(err_dict->last_lookup_status()) {
      info.entry.dist_type = int_to_disttype(i);
   }

   // Conf: dist_parm - optional
   info.entry.dist_parm = err_dict->lookup_num_array(
                             conf_key_dist_parm, false);

   // Conf: inst_bias_scale - optional
   info.entry.bias_scale = err_dict->lookup_double(
                              conf_key_inst_bias_scale, false);

   // Conf: inst_bias_offset - optional
   info.entry.bias_offset = err_dict->lookup_double(
                               conf_key_inst_bias_offset, false);

   // Store the RNG pointer
   info.rng_ptr = rng_ptr;

   info.entry.validate();

   return(info);
}

////////////////////////////////////////////////////////////////////////

double add_obs_error(const gsl_rng *r, FieldType t,
                     const ObsErrorEntry *e, double v) {
   double v_new = v;

   // Check for null pointer or bad input value
   if(!e || is_bad_data(v)) return(v);

   // Instrument bias correction for observations
   if(t == FieldType_Obs) {
      if(!is_bad_data(e->bias_scale))  v_new *= e->bias_scale;
      if(!is_bad_data(e->bias_offset)) v_new += e->bias_offset;
   }

   // Apply random perturbation
   if(e->dist_type != DistType_None) {
      v_new += ran_draw(r, e->dist_type,
                        e->dist_parm[0], e->dist_parm[1]);
   }

   // Detailed debug information
   if(mlog.verbosity_level() >= 4) {
      ConcatString cs;

      // Check for no updates
      if(is_bad_data(e->bias_scale) &&
         is_bad_data(e->bias_offset) &&
         e->dist_type == DistType_None) {
         cs << "Applying no observation error update for "
            << fieldtype_to_string(t) << " value " <<  v << ".\n";
      }
      // List update information
      else {
         cs << "Applying observation error update from "
            << fieldtype_to_string(t) << " value " << v << " to "
            << v_new << " for ";
         if(t == FieldType_Obs && !is_bad_data(e->bias_scale)) {
            cs << "bias scale (" << e->bias_scale << "), ";
         }
         if(t == FieldType_Obs && !is_bad_data(e->bias_offset)) {
            cs << "bias offset (" << e->bias_offset << "), ";
         }
         cs << dist_to_string(e->dist_type, e->dist_parm)
            << " perturbation.\n";
      }
      mlog << Debug(4) << cs;
   }

   return(v_new);
}

////////////////////////////////////////////////////////////////////////

DataPlane add_obs_error(const gsl_rng *r, FieldType t,
                        const ObsErrorEntry *e, const DataPlane &dp,
                        const char *var_name, const char *obtype) {
   int x, y;
   double v, v_new;
   DataPlane dp_new = dp;
   const ObsErrorEntry *e_new = e;

   // Apply random perturbation to each grid point
   for(x=0; x<dp_new.nx(); x++) {
      for(y=0; y<dp_new.ny(); y++) {

         // Get current value
         v = dp.get(x, y);

         // Check for a null pointer and do a table lookup
         if(!e) e_new = obs_error_table.lookup(var_name, obtype, v);

         // Apply perturbation
         v_new = add_obs_error(r, t, e_new, v);

         // Store perturbed value
         dp_new.set(v_new, x, y);
      }
   }

   return(dp_new);
}

////////////////////////////////////////////////////////////////////////
