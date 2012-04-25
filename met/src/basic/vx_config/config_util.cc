// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <sys/types.h>
#include <dirent.h>

#include "config_util.h"

#include "vx_math.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_version(Dictionary *dict) {
   ConcatString s;

   s = dict->lookup_string(conf_version);

   check_met_version(s);

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_model(Dictionary *dict) {
   ConcatString s;

   s = dict->lookup_string(conf_model);

   // Check that it's non-empty and contains no whitespace
   if(s.empty() || check_reg_exp(ws_reg_exp, s) == true) {
      mlog << Error << "\nparse_conf_model() -> "
           << "The model name (\"" << s << "\") must be non-empty and "
           << "contain no embedded whitespace.\n\n";
      exit(1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

map<STATLineType,STATOutputType> parse_conf_output_flag(Dictionary *dict) {
   Dictionary *out_dict = (Dictionary *) 0;
   map<STATLineType,STATOutputType> output_map;
   MetConfig conf;
   STATLineType line_type;
   STATOutputType output_type;
   int i, v;

   // Get the output flag dictionary
   out_dict = dict->lookup_dictionary(conf_output_flag);

   // Read the config file constants
   conf.read(replace_path(config_const_filename));
   
   // Loop over the output flag dictionary entries
   for(i=0; i<out_dict->n_entries(); i++) {

      // Get the line type for the current entry
      line_type = string_to_statlinetype((*out_dict)[i]->name());
     
      // Get the integer flag value for the current entry
      v = out_dict->lookup_int((*out_dict)[i]->name());

      // Convert integer to enumerated STATOutputType
           if(v == conf.lookup_int(conf_none)) output_type = STATOutputType_None;
      else if(v == conf.lookup_int(conf_stat)) output_type = STATOutputType_Stat;
      else if(v == conf.lookup_int(conf_both)) output_type = STATOutputType_Both;
      else {
         mlog << Error << "\nparse_conf_output_flag() -> "
              << "Unexpected config file value of " << v << " for \""
              << conf_output_flag << "." << (*out_dict)[i]->name()
              << "\".\n\n";
         exit(1);
      }

      // Store entry line type and corresponding output type
      output_map[line_type] = output_type;
   }
   
   return(output_map);
}

////////////////////////////////////////////////////////////////////////
//
// Compute the number of verification tasks specified in the current
// dictionary array.
//
////////////////////////////////////////////////////////////////////////

int parse_conf_n_vx(Dictionary *dict) {
   int i, total;

   // Check that this dictionary is an array
   if(!dict->is_array()) {
      mlog << Error << "\nparse_conf_n_vx() -> "
           << "This function must be passed a Dictionary array.\n\n";
      exit(1);
   }

   // Loop over the fields to be verified
   for(i=0,total=0; i<dict->n_entries(); i++) {

      // Increment count by the length of the level array
      total += (*dict)[i]->dict_value()->lookup_string_array(conf_level).n_elements();
   }

   return(total);
}

////////////////////////////////////////////////////////////////////////
//
// Retrieve the dictionary for the i-th verification task.
//
////////////////////////////////////////////////////////////////////////

Dictionary parse_conf_i_vx_dict(Dictionary *dict, int index) {
   Dictionary i_dict;
   DictionaryEntry entry;
   StringArray lvl;
   int i, total;
  
   // Check that this dictionary is an array
   if(!dict->is_array()) {
      mlog << Error << "\nparse_conf_i_vx_dict() -> "
           << "This function must be passed a Dictionary array.\n\n";
      exit(1);
   }
   
   // Loop over the fields to be verified
   for(i=0,total=0; i<dict->n_entries(); i++) {

      // Increment count by the length of the level array
      lvl     = (*dict)[i]->dict_value()->lookup_string_array(conf_level);
      total  += lvl.n_elements();

      // Check if we're in the correct entry
      if(total > index) {

         // Copy the current entry's dictionary
         i_dict = (*(*dict)[i]->dict_value());

         // Set up the new entry, taking only a single level value
         entry.set_string(conf_level, lvl[index-(total-lvl.n_elements())]);

         // Store the new entry
         i_dict.store(entry);

         break;
      }
   } // end for i

   return(i_dict);
}

////////////////////////////////////////////////////////////////////////

StringArray parse_conf_message_type(Dictionary *dict) {
   StringArray sa;
   int i;

   sa = dict->lookup_string_array(conf_message_type);

   // Check that at least one PrepBufr message type is provided
   if(sa.n_elements() == 0) {
      mlog << Error << "\nparse_conf_message_type() -> "
           << "At least one PrepBufr message type must be provided.\n\n";
      exit(1);
   }

   // Check that each PrepBufr message type provided is valid
   for(i=0; i<sa.n_elements(); i++) {

      if(strstr(vld_msg_typ_str, sa[i]) == NULL) {
         mlog << Error << "\nparse_conf_message_type() -> "
              << "Invalid message type string provided ("
              << sa[i] << ").\n\n";
         exit(1);
      }
   }

   return(sa);
}

////////////////////////////////////////////////////////////////////////

NumArray parse_conf_ci_alpha(Dictionary *dict) {
   NumArray na;
   int i;

   na = dict->lookup_num_array(conf_ci_alpha);

   // Check that at least one alpha value is provided
   if(na.n_elements() == 0) {
      mlog << Error << "\nparse_conf_ci_alpha() -> "
           << "At least one confidence interval alpha value must be "
           << "specified.\n\n";
      exit(1);
   }

   // Check that the values for alpha are between 0 and 1
   for(i=0; i<na.n_elements(); i++) {
      if(na[i] <= 0.0 || na[i] >= 1.0) {
         mlog << Error << "\nparse_conf_ci_alpha() -> "
              << "All confidence interval alpha values ("
              << na[i] << ") must be greater than 0 "
              << "and less than 1.\n\n";
         exit(1);
      }
   }

   return(na);
}

////////////////////////////////////////////////////////////////////////

BootInfo parse_conf_boot(Dictionary *dict) {
   MetConfig conf;
   BootInfo info;
   int v;

   // Read the config file constants
   conf.read(replace_path(config_const_filename));

   // Conf: boot.interval
   v = dict->lookup_int(conf_boot_interval);

   // Convert integer to enumerated BootIntervalType
        if(v == conf.lookup_int(conf_bca))    info.interval = BootIntervalType_BCA;
   else if(v == conf.lookup_int(conf_pctile)) info.interval = BootIntervalType_Percentile;
   else {
      mlog << Error << "\nparse_conf_boot() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_boot_interval << "\".\n\n";
      exit(1);
   }

   // Conf: boot.rep_prop
   info.rep_prop = dict->lookup_double(conf_boot_rep_prop);

   // Check that it is between 0 and 1
   if(info.rep_prop <= 0.0 || info.rep_prop > 1.0) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "The \"" << conf_boot_rep_prop
           << "\" parameter (" << info.rep_prop
           << ")must be between 0 and 1!\n\n";
      exit(1);
   }

   // Conf: boot.n_rep
   info.n_rep = dict->lookup_int(conf_boot_n_rep);

   // Check n_rep >= 0
   if(info.n_rep < 0) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "The number of bootstrap resamples in the \""
           << conf_boot_n_rep << "\" parameter (" << info.n_rep
           << ") must be >= 0.\n\n";
      exit(1);
   }

   // Conf: boot_rng
   info.rng = dict->lookup_string(conf_boot_rng);

   // Conf: boot_seed
   info.seed = dict->lookup_string(conf_boot_seed);

   return(info);
}


////////////////////////////////////////////////////////////////////////

InterpInfo parse_conf_interp(Dictionary *dict) {
   Dictionary *type_dict = (Dictionary *) 0;
   MetConfig conf;
   InterpInfo info;
   ConcatString method;
   int i, v, width;

   // Read the config file constants
   conf.read(replace_path(config_const_filename));

   // Conf: interp.thresh
   info.thresh = dict->lookup_double(conf_interp_thresh);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.thresh < 0.0 || info.thresh > 1.0) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "The \"" << conf_interp_thresh << "\" parameter ("
           << info.thresh << ") must be set between 0 and 1.\n\n";
      exit(1);
   }   

   // Conf: interp.type
   type_dict = dict->lookup_array(conf_interp_type);

   // Store the number of interpolation types
   info.n_interp = type_dict->n_entries();

   // Check that at least one interpolation type is provided
   if(info.n_interp == 0) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "At least one interpolation type must be provided.\n\n";
      exit(1);
   }
   
   // Loop over the interpolation type dictionary entries
   for(i=0; i<type_dict->n_entries(); i++) {

      // Get the method for the current entry
      v = (*type_dict)[i]->dict_value()->lookup_int(conf_method);

      // Convert integer to enumerated InterpMthd
           if(v == conf.lookup_int(interpmthd_min_str))     method = interpmthd_min_str;
      else if(v == conf.lookup_int(interpmthd_max_str))     method = interpmthd_max_str;
      else if(v == conf.lookup_int(interpmthd_median_str))  method = interpmthd_median_str;
      else if(v == conf.lookup_int(interpmthd_uw_mean_str)) method = interpmthd_uw_mean_str;
      else if(v == conf.lookup_int(interpmthd_dw_mean_str)) method = interpmthd_dw_mean_str;
      else if(v == conf.lookup_int(interpmthd_ls_fit_str))  method = interpmthd_ls_fit_str;
      else if(v == conf.lookup_int(interpmthd_nbrhd_str))   method = interpmthd_nbrhd_str;
      else if(v == conf.lookup_int(interpmthd_bilin_str))   method = interpmthd_bilin_str;
      else {
         mlog << Error << "\nparse_conf_interval() -> "
              << "Unexpected config file value of " << v << " for \""
              << conf_method << "\".\n\n";
         exit(1);
      }

      // Get the width for the current entry
      width = (*type_dict)[i]->dict_value()->lookup_int(conf_width);

      // Check for the nearest neighbor special case
      if(width == 1 && strcmp(method, interpmthd_uw_mean_str) != 0) {
         mlog << Warning << "\nparse_conf_interp() -> "
              << "For neareast neighbor interpolation method, resetting "
              << "method from \"" << method << "\" to \""
              << interpmthd_uw_mean_str << "\" since width = 1.\n\n";
         method = interpmthd_uw_mean_str;
      }

      // Check for the bilinear interpolation special case
      if(strcmp(method, interpmthd_bilin_str) == 0 && width != 2) {
         mlog << Warning << "\nparse_conf_interp() -> "
              << "For bilinear interpolation method, resetting "
              << "width from \"" << width << "\" to \"2\".\n\n";
         width = 2;
      }

      // Add the current entries
      info.method.add(method);
      info.width.add(width);
   } // end for i

   return(info);
}

////////////////////////////////////////////////////////////////////////

DuplicateType parse_conf_duplicate_flag(Dictionary *dict) {
   MetConfig conf;
   DuplicateType t;
   int v;

   // Read the config file constants
   conf.read(replace_path(config_const_filename));
   
   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_duplicate_flag);
      
   // Convert integer to enumerated DuplicateType
        if(v == conf.lookup_int(conf_none))   t = DuplicateType_None;
   else if(v == conf.lookup_int(conf_unique)) t = DuplicateType_Unique;
   else if(v == conf.lookup_int(conf_single)) t = DuplicateType_Single;
   else {
      mlog << Error << "\nparse_conf_duplicate_flag() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_duplicate_flag << "\".\n\n";
      exit(1);
   }

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_tmp_dir(Dictionary *dict) {
   ConcatString s;

   // Read the temporary directory
   s = dict->lookup_string(conf_tmp_dir);

   // Make sure that it exists
   if(opendir(s) == NULL ) {
      mlog << Error << "\nparse_conf_tmp_dir() -> "
           << "Cannot access the \"" << conf_tmp_dir << "\" directory: "
           << s << "\n\n";
      exit(1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void check_prob_thresh(const ThreshArray &ta) {
   int i, n;

   n = ta.n_elements();

   // Check for at least 3 thresholds beginning with 0 and ending with 1.
   if(n < 3 ||
      !is_eq(ta[0].thresh,   0.0) ||
      !is_eq(ta[n-1].thresh, 1.0)) {

      mlog << Error << "\ncheck_prob_thresh() -> "
           << "When verifying a probability field, you must "
           << "select at least 3 thresholds beginning with 0.0 "
           << "and ending with 1.0.\n\n";
      exit(1);
   }

   for(i=0; i<n; i++) {

      // Check that all threshold types are greater than or equal to
      if(ta[i].type != thresh_ge) {
         mlog << Error << "\ncheck_prob_thresh() -> "
              << "When verifying a probability field, all "
              << "thresholds must be set as equal to, "
              << "using \"ge\" or \">=\".\n\n";
         exit(1);
      }

      // Check that all thresholds are in [0, 1].
      if(ta[i].thresh < 0.0 ||
         ta[i].thresh > 1.0) {

         mlog << Error << "\ncheck_prob_thresh() -> "
              << "When verifying a probability field, all "
              << "thresholds must be between 0 and 1.\n\n";
         exit(1);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

const char * statlinetype_to_string(const STATLineType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case(stat_sl1l2):        s = stat_sl1l2_str;   break;
      case(stat_sal1l2):       s = stat_sal1l2_str;  break;
      case(stat_vl1l2):        s = stat_vl1l2_str;   break;
      case(stat_val1l2):       s = stat_val1l2_str;  break;
      case(stat_fho):          s = stat_fho_str;     break;
      case(stat_ctc):          s = stat_ctc_str;     break;
      case(stat_cts):          s = stat_cts_str;     break;
      case(stat_mctc):         s = stat_mctc_str;    break;
      case(stat_mcts):         s = stat_mcts_str;    break;
      case(stat_cnt):          s = stat_cnt_str;     break;
      case(stat_pct):          s = stat_pct_str;     break;
      case(stat_pstd):         s = stat_pstd_str;    break;
      case(stat_pjc):          s = stat_pjc_str;     break;
      case(stat_prc):          s = stat_prc_str;     break;
      case(stat_mpr):          s = stat_mpr_str;     break;
      case(stat_nbrctc):       s = stat_nbrctc_str;  break;
      case(stat_nbrcts):       s = stat_nbrcts_str;  break;
      case(stat_nbrcnt):       s = stat_nbrcnt_str;  break;
      case(stat_isc):          s = stat_isc_str;     break;
      case(stat_wdir):         s = stat_wdir_str;    break;
      case(stat_rhist):        s = stat_rhist_str;   break;
      case(stat_orank):        s = stat_orank_str;   break;
      case(no_stat_line_type): s = stat_na_str;      break;
      default:                 s = (const char *) 0; break;
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void statlinetype_to_string(const STATLineType t, char *out) {

   strcpy(out, statlinetype_to_string(t));

   return;
}

////////////////////////////////////////////////////////////////////////

STATLineType string_to_statlinetype(const char *s) {
   STATLineType t;

        if(strcasecmp(s, stat_sl1l2_str)  == 0) t = stat_sl1l2;
   else if(strcasecmp(s, stat_sal1l2_str) == 0) t = stat_sal1l2;
   else if(strcasecmp(s, stat_vl1l2_str)  == 0) t = stat_vl1l2;
   else if(strcasecmp(s, stat_val1l2_str) == 0) t = stat_val1l2;
   else if(strcasecmp(s, stat_fho_str)    == 0) t = stat_fho;
   else if(strcasecmp(s, stat_ctc_str)    == 0) t = stat_ctc;
   else if(strcasecmp(s, stat_cts_str)    == 0) t = stat_cts;
   else if(strcasecmp(s, stat_mctc_str)   == 0) t = stat_mctc;
   else if(strcasecmp(s, stat_mcts_str)   == 0) t = stat_mcts;
   else if(strcasecmp(s, stat_cnt_str)    == 0) t = stat_cnt;
   else if(strcasecmp(s, stat_pct_str)    == 0) t = stat_pct;
   else if(strcasecmp(s, stat_pstd_str)   == 0) t = stat_pstd;
   else if(strcasecmp(s, stat_pjc_str)    == 0) t = stat_pjc;
   else if(strcasecmp(s, stat_prc_str)    == 0) t = stat_prc;
   else if(strcasecmp(s, stat_mpr_str)    == 0) t = stat_mpr;
   else if(strcasecmp(s, stat_nbrctc_str) == 0) t = stat_nbrctc;
   else if(strcasecmp(s, stat_nbrcts_str) == 0) t = stat_nbrcts;
   else if(strcasecmp(s, stat_nbrcnt_str) == 0) t = stat_nbrcnt;
   else if(strcasecmp(s, stat_isc_str)    == 0) t = stat_isc;
   else if(strcasecmp(s, stat_wdir_str)   == 0) t = stat_wdir;
   else if(strcasecmp(s, stat_rhist_str)  == 0) t = stat_rhist;
   else if(strcasecmp(s, stat_orank_str)  == 0) t = stat_orank;
   else                                         t = no_stat_line_type;

   return(t);
}

////////////////////////////////////////////////////////////////////////
