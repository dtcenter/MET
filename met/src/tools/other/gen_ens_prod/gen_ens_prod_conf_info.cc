// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

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

   // Initialize pointers
   rng_ptr  = (gsl_rng *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenEnsProdConfInfo::clear() {
   vector<VarInfo*>::const_iterator it;

   // Clear, erase, and initialize members
   model.clear();
   desc.clear();
   for(it = ens_info.begin(); it != ens_info.end(); it++) {
      if(*it) { delete *it; }
   }
   ens_info.clear();
   cdf_info.clear();
   ens_cat_ta.clear();
   ens_var_str.clear();
   nbrhd_prob.clear();
   nmep_smooth.clear();
   vld_ens_thresh = bad_data_double;
   vld_data_thresh = bad_data_double;
   nc_info.clear();
   tmp_dir.clear();
   version.clear();

   // Reset counts
   n_ens_var    = 0;
   max_n_cat_ta = 0;
   n_nbrhd      = 0;

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

void GenEnsProdConfInfo::process_config(GrdFileType etype) {
   int i;
   VarInfoFactory info_factory;
   Dictionary *edict = (Dictionary *) 0;
   Dictionary i_edict;
   InterpMthd mthd;

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

   // Conf: rng_type and rng_seed
   ConcatString rng_type, rng_seed;
   rng_type = conf.lookup_string(conf_key_rng_type);
   rng_seed = conf.lookup_string(conf_key_rng_seed);

   // Set the random number generator and seed value
   rng_set(rng_ptr, rng_type.c_str(), rng_seed.c_str());

   // Conf: ens.field
   edict = conf.lookup_array(conf_key_ens_field);

   // Determine the number of ensemble fields to be processed
   if((n_ens_var = parse_conf_n_vx(edict)) == 0) {
      mlog << Error << "\nGenEnsProdConfInfo::process_config() -> "
           << "At least one field must be specified in the \""
           << conf_key_ens_field << "\" dictionary!\n\n";
      exit(1);
   }

   // Parse the ensemble field information
   for(i=0,max_n_cat_ta=0; i<n_ens_var; i++) {

      // Allocate new VarInfo object
      ens_info.push_back(info_factory.new_var_info(etype));

      // Get the current dictionary
      i_edict = parse_conf_i_vx_dict(edict, i);

      // Set the current dictionary
      ens_info[i]->set_dict(i_edict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed ensemble field number " << i+1 << ":\n";
         ens_info[i]->dump(cout);
      }

      // Conf: ens_nc_var_str
      ens_var_str.add(parse_conf_string(&i_edict, conf_key_nc_var_str, false));

      // Conf: cat_thresh
      ens_cat_ta.push_back(i_edict.lookup_thresh_array(conf_key_cat_thresh));

      // Dump the contents of the current thresholds
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed thresholds for ensemble field number " << i+1 << ":\n";
         ens_cat_ta[i].dump(cout);
      }

      // Keep track of the maximum number of thresholds
      if(ens_cat_ta[i].n() > max_n_cat_ta) max_n_cat_ta = ens_cat_ta[i].n();

      // Conf: ensemble_flag
      nc_info.push_back(parse_nc_info(&i_edict));
   }

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
   n_nbrhd    = nbrhd_prob.width.n();

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

   return(cur);
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

   return(!status);
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
