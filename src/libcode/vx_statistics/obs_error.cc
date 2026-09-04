// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#ifdef _OPENMP
  #include "omp.h"
#endif

#include "obs_error.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

// Default observation error table file name
static const char default_obs_error_dir[] = "MET_BASE/table_files";

// Name of user-specified observation errror environment variable
static const char met_obs_error_table[] =
   "MET_OBS_ERROR_TABLE";

static const int  n_obs_error_columns = 15;
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

   if(this == &e) return *this;

   assign(e);

   return *this;
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

   dist_type = DistType::None;
   dist_parm.clear();

   v_min = bad_data_double;
   v_max = bad_data_double;

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
   out << prefix << "min = " << v_min << "\n";
   out << prefix << "max = " << v_max << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

double ObsErrorEntry::variance() const {
   return dist_var(dist_type, dist_parm[0], dist_parm[1]);
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorEntry::need_bias_correction() const {
   return !is_bad_data(bias_scale) || !is_bad_data(bias_offset);
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorEntry::need_perturbation() const {
   return dist_type != DistType::None;
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorEntry::parse_line(const DataLine &dl) {

   // Initialize
   clear();

   // Check for blank line or header
   if(dl.n_items() == 0 || is_header(dl)) return false;

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

   // Observation variable name column
   if(strcasecmp(dl[0],  wildcard_str) != 0) {
      StringArray sa;
      ConcatString cs;

      // Parse entries and store as regular expressions
      sa.parse_css(dl[0]);
      for(int i=0; i<sa.n(); i++) {
         cs << cs_erase << "^" << sa[i] << "$";
         var_name.add(cs);
      }
   }

   // Filtering parameters
   if(strcasecmp(dl[1], wildcard_str) != 0)  msg_type.parse_css(dl[1]);
   if(strcasecmp(dl[5], wildcard_str) != 0)       sid.parse_css(dl[2]);
   if(strcasecmp(dl[2], wildcard_str) != 0) pb_rpt_type.add_css(dl[3]);
   if(strcasecmp(dl[3], wildcard_str) != 0) in_rpt_type.add_css(dl[4]);
   if(strcasecmp(dl[4], wildcard_str) != 0)   inst_type.add_css(dl[5]);
   if(strcasecmp(dl[6], wildcard_str) != 0)   hgt_range.add_css(dl[6]);
   if(strcasecmp(dl[7], wildcard_str) != 0)   prs_range.add_css(dl[7]);
   if(strcasecmp(dl[8], wildcard_str) != 0)   val_range.add_css(dl[8]);

   // Observation error adjustments
   bias_scale = (strcmp(dl[9], na_str) == 0 ?
                 bad_data_double : atof(dl[9]));
   bias_offset = (strcmp(dl[10], na_str) == 0 ?
                  bad_data_double : atof(dl[10]));
   dist_type = string_to_disttype(dl[11]);
   if(dist_type != DistType::None) dist_parm.add_css(dl[12]);

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

   // Valid range of perturbed values
   v_min = (strcmp(dl[13], na_str) == 0 ?
            bad_data_double : atof(dl[13]));
   v_max = (strcmp(dl[14], na_str) == 0 ?
            bad_data_double : atof(dl[14]));

   validate();

   return true;
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorEntry::is_header(const DataLine &dl) {

   if(dl.n_items() > 0) {
      if(strcasecmp(dl[0], "OBS_VAR") == 0) return true;
   }

   return false;
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
                             double cur_val,
                             bool skip_var_name) {

   // Check array filters
   // The var_name regex check is the most expensive (recompiles a
   // POSIX regex on every call), so callers that have already
   // subsetted on var_name can skip re-checking it here.
   if(!skip_var_name &&
      var_name.n()    > 0 && !var_name.reg_exp_match(cur_var_name))  return false;
   if(msg_type.n()    > 0 && !msg_type.has(cur_msg_type))            return false;
   if(sid.n()         > 0 && !sid.has(cur_sid))                      return false;
   if(pb_rpt_type.n() > 0 && !pb_rpt_type.has(cur_pb_rpt))           return false;
   if(in_rpt_type.n() > 0 && !in_rpt_type.has(cur_in_rpt))           return false;
   if(inst_type.n()   > 0 && !inst_type.has(cur_inst))               return false;

   // Check ranges
   if(!is_bad_data(cur_hgt) && hgt_range.n() == 2) {
       if((cur_hgt < hgt_range[0] && !is_eq(cur_hgt, hgt_range[0])) ||
          (cur_hgt > hgt_range[1] && !is_eq(cur_hgt, hgt_range[1]))) return false;
   }
   if(!is_bad_data(cur_prs) && prs_range.n() == 2) {
       if((cur_prs < prs_range[0] && !is_eq(cur_prs, prs_range[0])) ||
          (cur_prs > prs_range[1] && !is_eq(cur_prs, prs_range[1]))) return false;
   }
   if(!is_bad_data(cur_val) && val_range.n() == 2) {
       if((cur_val < val_range[0] && !is_eq(cur_val, val_range[0])) ||
          (cur_val > val_range[1] && !is_eq(cur_val, val_range[1]))) return false;
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorEntry::validate() {
   int n_req;

   // Number of distribution parameters
   if(dist_type == DistType::Gamma   ||
      dist_type == DistType::Uniform ||
      dist_type == DistType::Beta) n_req = 2;
   else                           n_req = 1;

   // Make sure we have the expected number of parameters
   if(dist_type != DistType::None &&
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

   e = (ObsErrorEntry *) nullptr;

   clear();
}

////////////////////////////////////////////////////////////////////////

void ObsErrorTable::clear() {

   if(e) { delete [] e; e = (ObsErrorEntry *) nullptr; }

   IsSet      = false;
   N_elements = 0;
   N_alloc    = 0;

   VarSubsetCache.clear();
   LastMatchIndex = -1;

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

void ObsErrorTable::extend(int len) {

   if(len <= N_alloc )  return;

   int i;
   ObsErrorEntry * u = (ObsErrorEntry *) nullptr;

   u = new ObsErrorEntry [len];

   for(i=0; i<N_elements; i++) u[i] = e[i];

   e = u;

   u = (ObsErrorEntry *) nullptr;

   N_alloc = len;

   return;
}


////////////////////////////////////////////////////////////////////////

void ObsErrorTable::initialize() {
   ConcatString path;
   ConcatString desc;
   StringArray file_names;

   //
   // Use MET_OBS_ERROR_TABLE, if set
   //
   if(get_env(met_obs_error_table, path)) {
      desc << "user-defined " << met_obs_error_table;
   }
   //
   // Otherwise, read the default table file
   //
   else {
      path = replace_path(default_obs_error_dir);
      desc = "default observation error table";
   }

   // Search for file input file names
   file_names = get_filenames(path, "^obs_error", ".txt$", true);

   for(int i=0; i<file_names.n(); i++) {

      mlog << Debug(1)
           << "Reading " << desc << " file: " << file_names[i] << "\n";

      if(!read(file_names[i].c_str())) {
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
      return false;
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

   return true;
}

////////////////////////////////////////////////////////////////////////

//
// Return the indices of the table rows whose var_name criteria could
// ever match cur_var_name, computing (and caching) the subset the
// first time it's requested for a given variable name. This avoids
// rescanning (and re-running regex matches over) the full table on
// every lookup() call.
//
const vector<int> & ObsErrorTable::var_subset(const char *cur_var_name) {
   string key(cur_var_name);

   auto it = VarSubsetCache.find(key);
   if(it != VarSubsetCache.end()) return it->second;

   vector<int> subset;
   for(int i=0; i<N_elements; i++) {
      if(e[i].var_name.n() == 0 || e[i].var_name.reg_exp_match(cur_var_name)) {
         subset.push_back(i);
      }
   }

   auto result = VarSubsetCache.emplace(std::move(key), std::move(subset));
   return result.first->second;
}

////////////////////////////////////////////////////////////////////////

ObsErrorEntry *ObsErrorTable::lookup(
   const char *cur_var_name, const char *cur_msg_type, const char *cur_sid,
   int cur_pb_rpt,           int cur_in_rpt,           int cur_inst,
   double cur_hgt,           double cur_prs,           double cur_val) {
   ObsErrorEntry * e_match = (ObsErrorEntry *) nullptr;

   // Check the most recently matched entry first since consecutive
   // lookups often resolve to the same table row
   if(LastMatchIndex >= 0 && LastMatchIndex < N_elements &&
      e[LastMatchIndex].is_match(cur_var_name, cur_msg_type, cur_sid,
                                 cur_pb_rpt,   cur_in_rpt,   cur_inst,
                                 cur_hgt,      cur_prs,      cur_val)) {
      e_match = &e[LastMatchIndex];
   }
   else {

      // Otherwise, scan only the rows whose var_name criteria could
      // match, skipping the (already-verified) var_name regex check
      const vector<int> &subset = var_subset(cur_var_name);
      for(int idx : subset) {
         if(e[idx].is_match(cur_var_name, cur_msg_type, cur_sid,
                            cur_pb_rpt,   cur_in_rpt,   cur_inst,
                            cur_hgt,      cur_prs,      cur_val, true)) {
            e_match = &e[idx];
            LastMatchIndex = idx;
            break;
         }
      }
   }

   // Check for no match
   if(e_match == (ObsErrorEntry *) 0 && mlog.verbosity_level() >= 4) {
      mlog << Debug(4) << "\nObsErrorTable::lookup() -> "
           << "skipping observation since no match found for "
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

   return e_match;
}

////////////////////////////////////////////////////////////////////////

ObsErrorEntry *ObsErrorTable::lookup(
   const char *cur_var_name, const char *cur_msg_type, double cur_val) {
   ObsErrorEntry * e_match = (ObsErrorEntry *) nullptr;

   // Check the most recently matched entry first since consecutive
   // lookups (e.g. adjacent grid points) often resolve to the same
   // table row
   if(LastMatchIndex >= 0 && LastMatchIndex < N_elements &&
      e[LastMatchIndex].is_match(   cur_var_name,    cur_msg_type, bad_data_str,
                                    bad_data_int,    bad_data_int, bad_data_int,
                                 bad_data_double, bad_data_double, cur_val)) {
      e_match = &e[LastMatchIndex];
   }
   else {

      // Otherwise, scan only the rows whose var_name criteria could
      // match, skipping the (already-verified) var_name regex check
      const vector<int> &subset = var_subset(cur_var_name);
      for(int idx : subset) {
         if(e[idx].is_match(   cur_var_name,    cur_msg_type, bad_data_str,
                               bad_data_int,    bad_data_int, bad_data_int,
                            bad_data_double, bad_data_double, cur_val, true)) {
            e_match = &e[idx];
            LastMatchIndex = idx;
            break;
         }
      }
   }

   // Check for no match
   if(e_match == (ObsErrorEntry *) 0 && mlog.verbosity_level() >= 4) {
      mlog << Debug(4) << "\nObsErrorTable::lookup() -> "
           << "no observation error table match found for "
           << "var_name = \"" << cur_var_name
           << "\", msg_type = \"" << cur_msg_type
           << "\", val = " << cur_val << "\n\n";
   }

   return e_match;
}

////////////////////////////////////////////////////////////////////////

bool ObsErrorTable::has(const char *cur_var_name,
                        const char *cur_msg_type) {

   for(int i=0; i<N_elements; i++) {
      if( (e[i].var_name.n() == 0 || e[i].var_name.reg_exp_match(cur_var_name)) &&
          (e[i].msg_type.n() == 0 || e[i].msg_type.has(cur_msg_type)) ) return true;
   }

   return false;
}

////////////////////////////////////////////////////////////////////////
//
// Code for struct ObsErrorInfo struct
//
////////////////////////////////////////////////////////////////////////


void ObsErrorInfo::clear() {
   flag = false;
   entry.clear();
   rng_ptr = (gsl_rng *) nullptr;
}

////////////////////////////////////////////////////////////////////////

void ObsErrorInfo::validate() {

   // Check for no work to do
   if(!flag) return;

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

ObsErrorInfo &ObsErrorInfo::operator=(const ObsErrorInfo &a) noexcept {
   if ( this != &a ) {
      flag = a.flag;
      entry = a.entry;
      rng_ptr = (a.rng_ptr==nullptr) ? nullptr : a.rng_ptr;
   }
   return *this;
}

////////////////////////////////////////////////////////////////////////

ObsErrorInfo parse_conf_obs_error(Dictionary *dict, gsl_rng *rng_ptr) {
   Dictionary *err_dict = (Dictionary *) nullptr;
   ObsErrorInfo info;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_obs_error() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Initialize
   info.clear();

   // Conf: obs_error
   err_dict = dict->lookup_dictionary(conf_key_obs_error);

   // Conf: flag
   info.flag = err_dict->lookup_bool(conf_key_flag);

   // If set to NONE, no work to do
   if(!info.flag) return info;

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

   // Conf: min and max - optional
   info.entry.v_min = err_dict->lookup_double(conf_key_min_flag, false);
   info.entry.v_max = err_dict->lookup_double(conf_key_max_flag, false);

   // Store the RNG pointer
   info.rng_ptr = rng_ptr;

   info.entry.validate();

   return info;
}

////////////////////////////////////////////////////////////////////////

double add_obs_error_inc(const gsl_rng *r, FieldType t,
                         const ObsErrorEntry *e, const double obs,
                         double v, bool log_detail) {
   double v_new = v;

   // Check for null pointer or bad input value
   if(!e || is_bad_data(v)) return v;

   // Apply the specified random perturbation
   if(e->dist_type != DistType::None) {
      v_new += ran_draw(r, e->dist_type,
                        e->dist_parm[0], e->dist_parm[1]);
   }

   // Apply range check
   if(!is_bad_data(e->v_min) && v_new < e->v_min) v_new = e->v_min;
   if(!is_bad_data(e->v_max) && v_new > e->v_max) v_new = e->v_max;

   // Detailed debug information
   if(log_detail && mlog.verbosity_level() >= 4) {

      // Check for no updates
      if(e->dist_type == DistType::None) {
         mlog << Debug(4)
              << "Applying no observation error update for "
              << fieldtype_to_string(t) << " value " <<  v
              << " and OBS value " << obs << ".\n";
      }
      // Print detailed update information
      else {
         mlog << Debug(4)
              << "Applying observation error update from "
              << fieldtype_to_string(t) << " value " << v << " to "
              << v_new << " for OBS value " << obs << " using the "
              << dist_to_string(e->dist_type, e->dist_parm)
              << " distribution.\n";
      }
   }

   return v_new;
}

////////////////////////////////////////////////////////////////////////

DataPlane add_obs_error_inc(const gsl_rng *r, FieldType t,
                            const ObsErrorEntry *in_e,
                            const DataPlane &in_dp,
                            const DataPlane &obs_dp,
                            const char *var_name, const char *obtype) {
   DataPlane out_dp(in_dp);
   int nx = in_dp.nx();
   int ny = in_dp.ny();

   // Check for matching dimensions
   if(nx != obs_dp.nx() || ny != obs_dp.ny()) {
      mlog << Error << "\nadd_obs_error_inc() -> "
           << "the data dimensions must match (" << nx
           << ", " << ny << ") != (" << obs_dp.nx()
           << ", " << obs_dp.ny() << ")!\n\n";
      exit(1);
   }

   // A single, resolved entry applies to every point: skip the loop
   // entirely when it requires no perturbation and no range clamp
   if(in_e && !in_e->need_perturbation() &&
      is_bad_data(in_e->v_min) && is_bad_data(in_e->v_max)) {
      return out_dp;
   }

   const double *in_buf  = in_dp.data();
   const double *obs_buf = obs_dp.data();
   vector<double> &out_buf = out_dp.buf();

   if(in_e) {

      // The entry is fixed for every point, so no table lookup (and
      // therefore no shared, mutable table state) is touched here -
      // safe to parallelize. Preserve the same x-outer, y-inner
      // traversal order used historically so that, run single
      // threaded, the exact same sequence of RNG draws lands on the
      // exact same grid points as before.
      int n_threads = 1;
#ifdef _OPENMP
      n_threads = omp_get_max_threads();
#endif

      if(n_threads <= 1) {
         for(int x=0; x<nx; x++) {
            for(int y=0; y<ny; y++) {
               int j = y*nx + x;
               out_buf[j] = add_obs_error_inc(r, t, in_e, obs_buf[j],
                                              in_buf[j], false);
            }
         }
      }
      else {

         // One independent RNG clone per thread
         vector<gsl_rng *> thread_rngs = rng_set_omp(r, n_threads);

#pragma omp parallel default(none) \
         shared(in_buf, obs_buf, out_buf, nx, ny, in_e, t, thread_rngs)
         {
            gsl_rng *my_r = thread_rngs[omp_get_thread_num()];

#pragma omp for collapse(2) schedule(static)
            for(int x=0; x<nx; x++) {
               for(int y=0; y<ny; y++) {
                  int j = y*nx + x;
                  out_buf[j] = add_obs_error_inc(my_r, t, in_e, obs_buf[j],
                                                 in_buf[j], false);
               }
            }
         }

         rng_free_omp(thread_rngs);
      }
   }
   else {

      // Do a table lookup for each point. ObsErrorTable::lookup()
      // mutates shared cache state, so this loop must stay serial,
      // in the same x-outer, y-inner order used historically.
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {
            int j = y*nx + x;
            const ObsErrorEntry *e = obs_error_table.lookup(
                                         var_name, obtype, obs_buf[j]);
            out_buf[j] = add_obs_error_inc(r, t, e, obs_buf[j], in_buf[j]);
         }
      }
   }

   return out_dp;
}

////////////////////////////////////////////////////////////////////////

double add_obs_error_bc(FieldType t,
                        const ObsErrorEntry *e, double v,
                        bool log_detail) {
   double v_new = v;

   // Check for null pointer or bad input value
   if(!e || is_bad_data(v)) return v;

   // Apply instrument bias correction
   if(!is_bad_data(e->bias_scale))  v_new *= e->bias_scale;
   if(!is_bad_data(e->bias_offset)) v_new += e->bias_offset;

   // Apply range check
   if(!is_bad_data(e->v_min) && v_new < e->v_min) v_new = e->v_min;
   if(!is_bad_data(e->v_max) && v_new > e->v_max) v_new = e->v_max;

   // Detailed debug information
   if(log_detail && mlog.verbosity_level() >= 4) {

      // Check for no updates
      if(is_bad_data(e->bias_scale) &&
         is_bad_data(e->bias_offset)) {
         mlog << Debug(4)
              << "Applying no observation error bias correction to "
              << fieldtype_to_string(t) << " value " <<  v << ".\n";
      }
      // Print detailed update information
      else {
         mlog << Debug(4)
              << "Applying observation error bias correction from "
              << fieldtype_to_string(t) << " value " << v << " to "
              << v_new << " for bias scale (" << e->bias_scale
              << ") and offset (" <<  e->bias_offset << ").\n";
      }
   }

   return v_new;
}

////////////////////////////////////////////////////////////////////////

DataPlane add_obs_error_bc(FieldType t,
                           const ObsErrorEntry *in_e,
                           const DataPlane &in_dp,
                           const DataPlane &obs_dp,
                           const char *var_name, const char *obtype) {
   DataPlane out_dp(in_dp);
   int nxy = in_dp.nxy();

   // Check for matching dimensions
   if(in_dp.nx() != obs_dp.nx() || in_dp.ny() != obs_dp.ny()) {
      mlog << Error << "\nadd_obs_error_bc() -> "
           << "the data dimensions must match (" << in_dp.nx()
           << ", " << in_dp.ny() << ") != (" << obs_dp.nx()
           << ", " << obs_dp.ny() << ")!\n\n";
      exit(1);
   }

   // A single, resolved entry applies to every point: skip the loop
   // entirely when it requires no bias correction and no range clamp
   if(in_e && !in_e->need_bias_correction() &&
      is_bad_data(in_e->v_min) && is_bad_data(in_e->v_max)) {
      return out_dp;
   }

   const double *in_buf  = in_dp.data();
   const double *obs_buf = obs_dp.data();
   vector<double> &out_buf = out_dp.buf();

   if(in_e) {

      // The entry is fixed for every point - no table lookup, so no
      // shared, mutable table state is touched here - safe to
      // parallelize with no RNG involved
#pragma omp parallel for default(none) \
      shared(in_buf, out_buf, nxy, in_e, t) schedule(static)
      for(int j=0; j<nxy; j++) {
         out_buf[j] = add_obs_error_bc(t, in_e, in_buf[j], false);
      }
   }
   else {

      // Do a table lookup for each point. ObsErrorTable::lookup()
      // mutates shared cache state, so this loop must stay serial.
      for(int j=0; j<nxy; j++) {
         const ObsErrorEntry *e = obs_error_table.lookup(
                                      var_name, obtype, obs_buf[j]);
         out_buf[j] = add_obs_error_bc(t, e, in_buf[j]);
      }
   }

   return out_dp;
}

////////////////////////////////////////////////////////////////////////

vector<const ObsErrorEntry *> build_obs_error_entry_grid(
      const DataPlane &val_dp, const char *var_name, const char *obtype) {

   int nxy = val_dp.nxy();
   vector<const ObsErrorEntry *> entry_grid(
      nxy, (const ObsErrorEntry *) nullptr);
   const double *val_buf = val_dp.data();

   // Serial: ObsErrorTable::lookup() mutates internal cache state
   for(int j=0; j<nxy; j++) {
      if(!is_bad_data(val_buf[j])) {
         entry_grid[j] = obs_error_table.lookup(var_name, obtype,
                                                val_buf[j]);
      }
   }

   return entry_grid;
}

////////////////////////////////////////////////////////////////////////

DataPlane add_obs_error_inc(const gsl_rng *r, FieldType t,
                            const vector<const ObsErrorEntry *> &entry_grid,
                            const DataPlane &in_dp,
                            const DataPlane &obs_dp) {
   DataPlane out_dp(in_dp);
   int nx  = in_dp.nx();
   int ny  = in_dp.ny();
   int nxy = in_dp.nxy();

   // Check for matching dimensions
   if(nx != obs_dp.nx() || ny != obs_dp.ny() ||
      (int) entry_grid.size() != nxy) {
      mlog << Error << "\nadd_obs_error_inc() -> "
           << "the data dimensions must match (" << nx
           << ", " << ny << ") != (" << obs_dp.nx()
           << ", " << obs_dp.ny() << ") or the entry_grid size ("
           << entry_grid.size() << ") does not match (" << nxy
           << ")!\n\n";
      exit(1);
   }

   const double *in_buf   = in_dp.data();
   const double *obs_buf  = obs_dp.data();
   vector<double> &out_buf = out_dp.buf();
   const ObsErrorEntry * const *entry_buf = entry_grid.data();

   // Entries are precomputed and read-only here, so it's safe to
   // parallelize. Preserve the same x-outer, y-inner traversal order
   // used historically so that, run single threaded, the exact same
   // sequence of RNG draws lands on the exact same grid points.
   int n_threads = 1;
#ifdef _OPENMP
   n_threads = omp_get_max_threads();
#endif

   if(n_threads <= 1) {
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {
            int j = y*nx + x;
            out_buf[j] = add_obs_error_inc(r, t, entry_buf[j], obs_buf[j],
                                           in_buf[j], false);
         }
      }
   }
   else {

      // One independent RNG clone per thread
      vector<gsl_rng *> thread_rngs = rng_set_omp(r, n_threads);

#pragma omp parallel default(none) \
      shared(in_buf, obs_buf, out_buf, entry_buf, nx, ny, t, thread_rngs)
      {
         gsl_rng *my_r = thread_rngs[omp_get_thread_num()];

#pragma omp for collapse(2) schedule(static)
         for(int x=0; x<nx; x++) {
            for(int y=0; y<ny; y++) {
               int j = y*nx + x;
               out_buf[j] = add_obs_error_inc(my_r, t, entry_buf[j], obs_buf[j],
                                              in_buf[j], false);
            }
         }
      }

      rng_free_omp(thread_rngs);
   }

   return out_dp;
}

////////////////////////////////////////////////////////////////////////

DataPlane add_obs_error_bc(FieldType t,
                           const vector<const ObsErrorEntry *> &entry_grid,
                           const DataPlane &in_dp) {
   DataPlane out_dp(in_dp);
   int nxy = in_dp.nxy();

   // Check for matching size
   if((int) entry_grid.size() != nxy) {
      mlog << Error << "\nadd_obs_error_bc() -> "
           << "the entry_grid size (" << entry_grid.size()
           << ") does not match the data size (" << nxy << ")!\n\n";
      exit(1);
   }

   const double *in_buf   = in_dp.data();
   vector<double> &out_buf = out_dp.buf();
   const ObsErrorEntry * const *entry_buf = entry_grid.data();

   // Entries are precomputed and read-only here, so it's safe to
   // parallelize with no RNG involved
#pragma omp parallel for default(none) \
   shared(in_buf, out_buf, entry_buf, nxy, t) schedule(static)
   for(int j=0; j<nxy; j++) {
      out_buf[j] = add_obs_error_bc(t, entry_buf[j], in_buf[j], false);
   }

   return out_dp;
}

////////////////////////////////////////////////////////////////////////
