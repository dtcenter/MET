// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "gen_ens_prod_conf_info.h"
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_log.h"

#include "GridTemplate.h"

using namespace std;


////////////////////////////////////////////////////////////////////////
//
//  Code for class GenEnsProdConfInfo
//
////////////////////////////////////////////////////////////////////////

GenEnsProdConfInfo::GenEnsProdConfInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GenEnsProdConfInfo::~GenEnsProdConfInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdConfInfo::clear() {
   vector<GenEnsProdVarInfo*>::const_iterator var_it = ens_input.begin();

   // Clear, erase, and initialize members
   model.clear();
   desc.clear();

   for(; var_it != ens_input.end(); var_it++) {
     if(*var_it) { delete *var_it; }
   }

   ens_input.clear();
   cdf_info.clear();
   nbrhd_prob.clear();
   nmep_smooth.clear();
   vld_ens_thresh = bad_data_double;
   vld_data_thresh = bad_data_double;
   version.clear();

   // Reset counts
   n_var     = 0;
   max_n_cat = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdConfInfo::read_config(const ConcatString default_file_name,
                                     const ConcatString user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   conf.read(default_file_name.c_str());

   // Read the user-specified config file
   conf.read(user_file_name.c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdConfInfo::process_config(GrdFileType etype, StringArray * ens_files, bool use_ctrl) {
   int i, j;
   VarInfoFactory info_factory;
   Dictionary *edict = (Dictionary *) nullptr;
   Dictionary i_edict;
   InterpMthd mthd;
   VarInfo * next_var;

   int n_ens_files = ens_files->n();

   // Unset MET_ENS_MEMBER_ID in case it is set by the user
   unsetenv(met_ens_member_id);

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: version
   version = parse_conf_version(&conf);

   // Conf: model
   model = parse_conf_string(&conf, conf_key_model);

   // Conf: desc
   desc = parse_conf_string(&conf, conf_key_desc);

   // Conf: ens_member_ids
   ens_member_ids = parse_conf_ens_member_ids(&conf);

   // Conf: control_id
   control_id = parse_conf_string(&conf, conf_key_control_id, false);

    // Error check ens_member_ids and ensemble file list
   if(ens_member_ids.n() > 1) {

      // Only a single file should be provided if using ens_member_ids
      if(n_ens_files > 1) {
         mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
              << "The \"" << conf_key_ens_member_ids << "\" "
              << "must be empty if more than "
              << "one file is provided.\n\n";
         exit(1);
      }

      // The control ID must be set when the control file is specified
      if(control_id.empty() && use_ctrl) {
         mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
              << "the control_id must be set if processing a single input "
              << "file with the -ctrl option\n\n";
         exit(1);
      }

      // If control ID is set, it cannot be found in ens_member_ids
      if(!control_id.empty() && ens_member_ids.has(control_id)) {
          mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
               << "control_id (" << control_id << ") must not be found "
               << "in ens_member_ids\n\n";
          exit(1);
      }
   }

   // If no ensemble member IDs were provided, add an empty string
   if(ens_member_ids.n() == 0) {
      ens_member_ids.add("");
   }

   // Conf: ens.field
   edict = conf.lookup_array(conf_key_ens_field);

   // Determine the number of ensemble fields to be processed
   if((n_var = parse_conf_n_vx(edict)) == 0) {
      mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
           << "At least one field must be specified in the \""
           << conf_key_ens_field << "\" dictionary!\n\n";
      exit(1);
   }

   // Parse the ensemble field information
   for(i=0,max_n_cat=0; i<n_var; i++) {
      
      GenEnsProdVarInfo * ens_info = new GenEnsProdVarInfo();

      // Get the current dictionary
      i_edict = parse_conf_i_vx_dict(edict, i);

      // get VarInfo magic string without substituted values
      ens_info->raw_magic_str = raw_magic_str(i_edict, etype);

      // Loop over ensemble member IDs to substitute
      for(j=0; j<ens_member_ids.n(); j++) {

         // set environment variable for ens member ID
         setenv(met_ens_member_id, ens_member_ids[j].c_str(), 1);

         // Allocate new VarInfo object
         next_var = info_factory.new_var_info(etype);

         // Set the current dictionary
         next_var->set_dict(i_edict);

         // Dump the contents of the current VarInfo
         if(mlog.verbosity_level() >= 5) {
            mlog << Debug(5)
                 << "Parsed ensemble field number " << i+1
                 << " (" << j+1 << "):\n";
            next_var->dump(cout);
         }

         InputInfo input_info;
         input_info.var_info = next_var;
         input_info.file_index = 0;
         input_info.file_list = ens_files;
         input_info.ens_member_id = ens_member_ids[j];
         ens_info->add_input(input_info);

         // Add InputInfo to ens info list for each ensemble file provided
         // set var_info to nullptr to note first VarInfo should be used
         for(int k=1; k<n_ens_files; k++) {
            input_info.var_info = nullptr;
            input_info.file_index = k;
            input_info.file_list = ens_files;
            input_info.ens_member_id = ens_member_ids[j];
            ens_info->add_input(input_info);
         } // end for k

      } // end for j

      // Get field info for control member if set
      if(!control_id.empty()) {

         // Set environment variable for ens member ID
         setenv(met_ens_member_id, control_id.c_str(), 1);

         // Allocate new VarInfo object
         next_var = info_factory.new_var_info(etype);

         // Set the current dictionary
         next_var->set_dict(i_edict);

         ens_info->set_ctrl(next_var);
      }

      // Conf: nc_var_str
      ens_info->nc_var_str =parse_conf_string(&i_edict, conf_key_nc_var_str, false);

      // Conf: cat_thresh
      ens_info->cat_ta = i_edict.lookup_thresh_array(conf_key_cat_thresh);

      // Dump the contents of the current thresholds
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed thresholds for ensemble field number " << i+1 << ":\n";
         ens_info->cat_ta.dump(cout);
      }

      // Keep track of the maximum number of thresholds
      if(ens_info->cat_ta.n() > max_n_cat) max_n_cat = ens_info->cat_ta.n();

      // Conf: normalize
      ens_info->normalize = parse_conf_normalize(&i_edict);

      // Conf: ensemble_flag
      ens_info->nc_info = parse_nc_info(&i_edict);

      ens_input.push_back(ens_info);
   } // end for i

   // Conf: ens.ens_thresh
   vld_ens_thresh = conf.lookup_double(conf_key_ens_ens_thresh);

   // Check that the valid ensemble threshold is between 0 and 1.
   if(vld_ens_thresh < 0.0 || vld_ens_thresh > 1.0) {
      mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
           << "The \"" << conf_key_ens_ens_thresh << "\" parameter ("
           << vld_ens_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: ens.vld_thresh
   vld_data_thresh = conf.lookup_double(conf_key_ens_vld_thresh);

   // Check that the valid data threshold is between 0 and 1.
   if(vld_data_thresh < 0.0 || vld_data_thresh > 1.0) {
      mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
           << "The \"" << conf_key_ens_vld_thresh << "\" parameter ("
           << vld_data_thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: nbrhd_prob
   nbrhd_prob = parse_conf_nbrhd(edict, conf_key_nbrhd_prob);

   // Conf: nmep_smooth 
   nmep_smooth = parse_conf_interp(edict, conf_key_nmep_smooth);

   // Loop through the neighborhood probability smoothing options
   for(i=0; i<nmep_smooth.n_interp; i++) {

      mthd = string_to_interpmthd(nmep_smooth.method[i].c_str());

      // Check for unsupported neighborhood probability smoothing methods
      if(mthd == InterpMthd_DW_Mean ||
         mthd == InterpMthd_LS_Fit  ||
         mthd == InterpMthd_Bilin) {
         mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
              << "Neighborhood probability smoothing methods DW_MEAN, "
              << "LS_FIT, and BILIN are not supported for \""
              << conf_key_nmep_smooth << "\".\n\n";
         exit(1);
      }

      // Check for valid neighborhood probability interpolation widths
      if(nmep_smooth.width[i] < 1 || nmep_smooth.width[i]%2 == 0) {
         mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
              << "Neighborhood probability smoothing widths must be set "
              << "to odd values greater than or equal to 1 ("
              << nmep_smooth.width[i] << ") for \""
              << conf_key_nmep_smooth << "\".\n\n";
         exit(1);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

GenEnsProdNcOutInfo GenEnsProdConfInfo::parse_nc_info(Dictionary *dict) {
   GenEnsProdNcOutInfo cur;

   // Parse the ensemble flag
   const DictionaryEntry *e = dict->lookup(conf_key_ensemble_flag);

   if(!e) {
      mlog << Error
           << "\nGenEnsProdConfInfo::parse_nc_info() -> "
           << "lookup failed for key \"" << conf_key_ensemble_flag
           << "\"\n\n";
      exit(1);
   }

   // Boolean type
   if(e->type() == BooleanType) {
      if(e->b_value()) cur.set_all_true();
      else             cur.set_all_false();
   }
   // Dictionary type
   else {

      // Check the type
      if(e->type() != DictionaryType) {
         mlog << Error << "\nGenEnsProdConfInfo::parse_nc_info() -> "
              << "bad type (" << configobjecttype_to_string(e->type())
              << ") for key \"" << conf_key_ensemble_flag << "\"\n\n";
         exit(1);
      }

      // Parse the various entries
      Dictionary * d = e->dict_value();

      cur.do_latlon    = d->lookup_bool(conf_key_latlon_flag);
      cur.do_mean      = d->lookup_bool(conf_key_mean_flag);
      cur.do_stdev     = d->lookup_bool(conf_key_stdev_flag);
      cur.do_minus     = d->lookup_bool(conf_key_minus_flag);
      cur.do_plus      = d->lookup_bool(conf_key_plus_flag);
      cur.do_min       = d->lookup_bool(conf_key_min_flag);
      cur.do_max       = d->lookup_bool(conf_key_max_flag);
      cur.do_range     = d->lookup_bool(conf_key_range_flag);
      cur.do_vld       = d->lookup_bool(conf_key_vld_count_flag);
      cur.do_freq      = d->lookup_bool(conf_key_frequency_flag);
      cur.do_nep       = d->lookup_bool(conf_key_nep_flag);
      cur.do_nmep      = d->lookup_bool(conf_key_nmep_flag);
      cur.do_climo     = d->lookup_bool(conf_key_climo_flag);
      cur.do_climo_cdp = d->lookup_bool(conf_key_climo_cdp_flag);
   }

   return cur;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for struct GenEnsProdNcOutInfo
//
////////////////////////////////////////////////////////////////////////

GenEnsProdNcOutInfo::GenEnsProdNcOutInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdNcOutInfo::clear() {

   set_all_true();

   return;
}

////////////////////////////////////////////////////////////////////////

bool GenEnsProdNcOutInfo::all_false() const {

   bool status = do_latlon || do_mean || do_stdev || do_minus ||
                 do_plus   || do_min  || do_max   || do_range ||
                 do_vld    || do_freq || do_nep   || do_nmep  ||
                 do_climo  || do_climo_cdp;

   return !status;
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdNcOutInfo::set_all_false() {

   do_latlon    = false;
   do_mean      = false;
   do_stdev     = false;
   do_minus     = false;
   do_plus      = false;
   do_min       = false;
   do_max       = false;
   do_range     = false;
   do_vld       = false;
   do_freq      = false;
   do_nep       = false;
   do_nmep      = false;
   do_climo     = false;
   do_climo_cdp = false;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdNcOutInfo::set_all_true() {

   do_latlon    = true;
   do_mean      = true;
   do_stdev     = true;
   do_minus     = true;
   do_plus      = true;
   do_min       = true;
   do_max       = true;
   do_range     = true;
   do_vld       = true;
   do_freq      = true;
   do_nep       = true;
   do_nmep      = true;
   do_climo     = true;
   do_climo_cdp = true;

   return;
}

////////////////////////////////////////////////////////////////////////
