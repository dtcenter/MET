// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

#include "config_util.h"

#include "vx_math.h"
#include "vx_util.h"

#include "GridTemplate.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

static const double default_vld_thresh = 1.0;

///////////////////////////////////////////////////////////////////////////////

// MetConfig object containing config value constants
static MetConfig conf_const(replace_path(config_const_filename).c_str());

///////////////////////////////////////////////////////////////////////////////


GaussianInfo::GaussianInfo()
: weights(nullptr)
{
   clear();
}

///////////////////////////////////////////////////////////////////////////////

void GaussianInfo::clear() {
   weight_sum = 0.0;
   if (weights) {
      delete weights;
      weights = (double *)nullptr;
   }
   max_r = weight_cnt = 0;
   radius = dx = bad_data_double;
   trunc_factor = default_trunc_factor;
}

///////////////////////////////////////////////////////////////////////////////

int GaussianInfo::compute_max_r() {
   max_r = nint(radius / dx * trunc_factor);
   return max_r;
}

///////////////////////////////////////////////////////////////////////////////
//
// Compute the Gaussian filter
// g(x,y) = (1 / (2 * pi * sigma**2)) * exp(-(x**2 + y**2) / (2 * sigma**2))
//
///////////////////////////////////////////////////////////////////////////////

void GaussianInfo::compute() {
   double weight;
   double distance_sq;
   const double g_sigma = radius / dx;
   const double g_sigma_sq = g_sigma * g_sigma;
   const double f_sigma_exp_divider = (2 * g_sigma_sq);
   const double f_sigma_divider = (2 * M_PI * g_sigma_sq);
   const double max_r_sq = pow((g_sigma * trunc_factor), 2);

   validate();
   if (0 < max_r && weights) delete weights;
   compute_max_r();

   int index = 0;
   int g_nx = max_r * 2 + 1;
   weight_cnt = 0;
   weight_sum = 0.0;
   weights = new double[g_nx*g_nx];
   for(int idx_x=-max_r; idx_x<=max_r; idx_x++) {
      for(int idx_y=-max_r; idx_y<=max_r; idx_y++) {
         weight = 0.0;
         distance_sq = (double)idx_x*idx_x + idx_y*idx_y;
         if (distance_sq <= max_r_sq) {
            weight_cnt++;
            weight = exp(-distance_sq / f_sigma_exp_divider) / f_sigma_divider;
            weight_sum += weight;
         }
         weights[index++] = weight;
      } // end for idx_y
   } // end for idx_x

   mlog << Debug(7) << "GaussianInfo::compute() max_r: "  << max_r << "\n";
}

///////////////////////////////////////////////////////////////////////////////

void GaussianInfo::validate() {
   if (is_eq(radius, bad_data_double) || is_eq(radius, 0.)) {
      mlog << Error << "\nGaussianInfo::validate() -> "
           << "gaussian raduis is missing\n\n";
      exit(1);
   }
   if (is_eq(dx, bad_data_double) || is_eq(dx, 0.)) {
      mlog << Error << "\nGaussianInfo::validate() -> "
           << "gaussian dx is missing\n\n";
      exit(1);
   }
}


///////////////////////////////////////////////////////////////////////////////


void RegridInfo::clear() {
   enable = false;
   field = FieldType::None;
   vld_thresh = bad_data_double;
   name.clear();
   method = InterpMthd::None;
   width = bad_data_int;
   gaussian.clear();
   shape = GridTemplateFactory::GridTemplates::None;
   convert_fx.clear();
   censor_thresh.clear();
   censor_val.clear();
}

///////////////////////////////////////////////////////////////////////////////

RegridInfo::RegridInfo() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

void RegridInfo::validate() {

   // Check for unsupported regridding options
   if(method == InterpMthd::Best       ||
      method == InterpMthd::Geog_Match ||
      method == InterpMthd::Gaussian   ||
      method == InterpMthd::HiRA) {
      mlog << Error << "\nRegridInfo::validate() -> "
           << "\"" << interpmthd_to_string(method)
           << "\" not valid for regridding, only interpolating.\n\n";
      exit(1);
   }

   // Check the nearest neighbor special case
   if(width  == 1 &&
      method != InterpMthd::None &&
      method != InterpMthd::Nearest &&
      method != InterpMthd::Force &&
      method != InterpMthd::Upper_Left &&
      method != InterpMthd::Upper_Right &&
      method != InterpMthd::Lower_Right &&
      method != InterpMthd::Lower_Left &&
      method != InterpMthd::AW_Mean &&
      method != InterpMthd::MaxGauss) {
      mlog << Warning << "\nRegridInfo::validate() -> "
           << "Resetting the regridding method from \""
           << interpmthd_to_string(method) << "\" to \""
           << interpmthd_nearest_str
           << "\" since the regridding width is 1.\n\n";
      method = InterpMthd::Nearest;
   }

   // Check for some methods, that width is 1
   if((method == InterpMthd::Nearest ||
       method == InterpMthd::Force ||
       method == InterpMthd::Upper_Left ||
       method == InterpMthd::Upper_Right ||
       method == InterpMthd::Lower_Right ||
       method == InterpMthd::Lower_Left ||
       method == InterpMthd::AW_Mean) &&
      width != 1) {
      mlog << Warning << "\nRegridInfo::validate() -> "
           << "Resetting regridding width from "
           << width << " to 1 for interpolation method \""
           << interpmthd_to_string(method) << "\".\n\n";
      width = 1;
   }

   // Check the bilinear and budget special cases
   if((method == InterpMthd::Bilin ||
       method == InterpMthd::Budget) &&
      width != 2) {
      mlog << Warning << "\nRegridInfo::validate() -> "
           << "Resetting the regridding width from "
           << width << " to 2 for regridding method \""
           << interpmthd_to_string(method) << "\".\n\n";
      width = 2;
   }

   // Check the Gaussian filter
   if(method == InterpMthd::MaxGauss && gaussian.radius < gaussian.dx) {
      mlog << Error << "\nRegridInfo::validate() -> "
           << "The radius of influence (" << gaussian.radius
           << ") is less than the delta distance (" << gaussian.dx
           << ") for regridding method \"" << interpmthd_to_string(method) << "\".\n\n";
      exit(1);
   }

   // Check for equal number of censor thresholds and values
   if(censor_thresh.n() != censor_val.n()) {
      mlog << Error << "\nRegridInfo::validate() -> "
           << "The number of censor thresholds in \""
           << conf_key_censor_thresh << "\" (" << censor_thresh.n()
           << ") must match the number of replacement values in \""
           << conf_key_censor_val << "\" (" << censor_val.n() << ").\n\n";
      exit(1);
   }

}

///////////////////////////////////////////////////////////////////////////////

void RegridInfo::validate_point() {

   // Check for unsupported regridding options
   if(method != InterpMthd::Max &&
      method != InterpMthd::Min &&
      method != InterpMthd::Median &&
      method != InterpMthd::UW_Mean) {
      mlog << Warning << "\nRegridInfo::validate_point() -> "
           << "Resetting the regridding method from \""
           << interpmthd_to_string(method) << "\" to \""
           << interpmthd_uw_mean_str << ".\n"
           << "\tAvailable methods: "
           << interpmthd_to_string(InterpMthd::UW_Mean) << ", "
           << interpmthd_to_string(InterpMthd::Max) << ", "
           << interpmthd_to_string(InterpMthd::Min) << ", "
           << interpmthd_to_string(InterpMthd::Median) << ".\n\n";
      method = InterpMthd::UW_Mean;
   }

}

///////////////////////////////////////////////////////////////////////////////

RegridInfo &RegridInfo::operator=(const RegridInfo &a) noexcept {
   if ( this != &a ) {
      enable = a.enable;
      field = a.field;
      vld_thresh = a.vld_thresh;
      name = a.name;
      method = a.method;
      width = a.width;
      gaussian = a.gaussian;
      shape = a.shape;
      convert_fx = a.convert_fx;
      censor_thresh = a.censor_thresh;
      censor_val = a.censor_val;
      hook = a.hook;
   }
   return *this;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Enumeration>
auto enum_class_as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

///////////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_version(Dictionary *dict) {
   ConcatString s;

   if(!dict) {
      mlog << Error << "\nparse_conf_version() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   s = dict->lookup_string(conf_key_version);

   if(dict->last_lookup_status()) {
     check_met_version(s.c_str());
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_string(Dictionary *dict, const char *conf_key,
                               bool check_empty) {
   ConcatString s;
   const char *method_name = "parse_conf_string() -> ";

   if(!dict) {
      mlog << Error << "\n" << method_name << "empty dictionary! (" << conf_key <<")\n\n";
      exit(1);
   }

   s = dict->lookup_string(conf_key);
   if(dict->last_lookup_status()) {

      // Check for an empty string
      if(check_empty && s.empty()) {
         mlog << Error << "\n" << method_name
              << "The \"" << conf_key << "\" entry (\"" << s
              << "\") cannot be empty.\n\n";
         exit(1);
      }

      // Check for embedded whitespace in non-empty strings
      if(!s.empty() && check_reg_exp(ws_reg_exp, s.c_str()) == true) {
         mlog << Error << "\n" << method_name
              << "The \"" << conf_key << "\" entry (\"" << s
              << "\") cannot contain embedded whitespace.\n\n";
         exit(1);
      }
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

StringArray parse_conf_string_array(Dictionary *dict, const char *conf_key, const char *caller) {
   StringArray sa;
   StringArray cur;
   StringArray sid_sa;
   if(!dict) {
      mlog << Error << "\n" << caller << "empty dictionary!\n\n";
      exit(1);
   }

   return dict->lookup_string_array(conf_key);
}

///////////////////////////////////////////////////////////////////////////////

GrdFileType parse_conf_file_type(Dictionary *dict) {
   GrdFileType t = FileType_None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_file_type() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_file_type, false);

   if(dict->last_lookup_status()) {
      // Convert integer to enumerated GrdFileType
           if(v == conf_const.lookup_int(conf_val_grib1))         t = FileType_Gb1;
      else if(v == conf_const.lookup_int(conf_val_grib2))         t = FileType_Gb2;
      else if(v == conf_const.lookup_int(conf_val_netcdf_met))    t = FileType_NcMet;
      else if(v == conf_const.lookup_int(conf_val_netcdf_wrf))    t = FileType_NcWrf;
      else if(v == conf_const.lookup_int(conf_val_netcdf_pint))   t = FileType_NcPinterp;
      else if(v == conf_const.lookup_int(conf_val_netcdf_nccf))   t = FileType_NcCF;
      else if(v == conf_const.lookup_int(conf_val_netcdf_ugrid))  t = FileType_UGrid;
      else if(v == conf_const.lookup_int(conf_val_python_numpy))  t = FileType_Python_Numpy;
      else if(v == conf_const.lookup_int(conf_val_python_xarray)) t = FileType_Python_Xarray;
      else {
         mlog << Error << "\nparse_conf_file_type() -> "
              << "Unexpected config file value of " << v << " for \""
              << conf_key_file_type << "\".\n\n";
         exit(1);
      }
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

map<STATLineType,STATOutputType> parse_conf_output_flag(Dictionary *dict,
                                 const STATLineType *line_type, int n_lty) {
   map<STATLineType,STATOutputType> output_map;
   STATOutputType t = STATOutputType::None;
   ConcatString cs;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_output_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Loop over the requested line types
   for(int i=0; i<n_lty; i++) {

      // Build the string
      cs << cs_erase << conf_key_output_flag << "."
         << statlinetype_to_string(line_type[i]);
      cs.set_lower();

      // Get the integer flag value for the current entry
      v = dict->lookup_int(cs.c_str());

      // Convert integer to enumerated STATOutputType
           if(v == conf_const.lookup_int(conf_val_none)) t = STATOutputType::None;
      else if(v == conf_const.lookup_int(conf_val_stat)) t = STATOutputType::Stat;
      else if(v == conf_const.lookup_int(conf_val_both)) t = STATOutputType::Both;
      else {
         mlog << Error << "\nparse_conf_output_flag() -> "
              << "Unexpected config file value of " << v << " for \""
              << cs << "\".\n\n";
         exit(1);
      }

      // Store entry line type and corresponding output type
      output_map[line_type[i]] = t;
   }

   // Make sure the map is the expected size
   if((int) output_map.size() != n_lty) {
      mlog << Error << "\nparse_conf_output_flag() -> "
           << "Unexpected number of entries found in \""
           << conf_key_output_flag << "\" ("
           << (int) output_map.size()
           << " != " << n_lty << ").\n\n";
      exit(1);
   }

   return output_map;
}

///////////////////////////////////////////////////////////////////////////////

map<STATLineType,StringArray> parse_conf_output_stats(Dictionary *dict) {
   Dictionary *out_dict = (Dictionary *) nullptr;
   map<STATLineType,StringArray> output_map;
   STATLineType line_type;
   StringArray sa;

   if(!dict) {
      mlog << Error << "\nparse_conf_output_stats() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the output flag dictionary
   out_dict = dict->lookup_dictionary(conf_key_output_stats);

   // Loop over the output flag dictionary entries
   for(int i=0; i<out_dict->n_entries(); i++) {

      // Get the line type for the current entry
      line_type = string_to_statlinetype((*out_dict)[i]->name().c_str());

      // Get the StringArray value for the current entry
      sa = out_dict->lookup_string_array((*out_dict)[i]->name().c_str());

      // Set ignore case to true
      sa.set_ignore_case(true);

      // Store entry line type and corresponding list of statistics
      output_map[line_type].add(sa);
   }

   return output_map;
}

///////////////////////////////////////////////////////////////////////////////
//
// Compute the number of verification tasks specified in the current
// dictionary array.
//
///////////////////////////////////////////////////////////////////////////////

int parse_conf_n_vx(Dictionary *dict) {
   int total = 0;
   StringArray lvl;

   if(!dict) return 0;

   // Check that this dictionary is an array
   if(!dict->is_array()) {
      mlog << Error << "\nparse_conf_n_vx() -> "
           << "This function must be passed a Dictionary array.\n\n";
      exit(1);
   }

   // Loop over the fields to be verified
   for(int i=0; i<dict->n_entries(); i++) {

      // Get the level array, which may or may not be defined.
      // If defined, use its length.  If not, use a length of 1.
      lvl = (*dict)[i]->dict_value()->lookup_string_array(conf_key_level, false);

      // Increment count by the length of the level array
      total += (lvl.n() > 0 ? lvl.n() : 1);
   }

   return total;
}

///////////////////////////////////////////////////////////////////////////////
//
// Retrieve the dictionary for the i-th verification task.
//
///////////////////////////////////////////////////////////////////////////////

Dictionary parse_conf_i_vx_dict(Dictionary *dict, int index) {
   Dictionary i_dict;
   DictionaryEntry entry;
   StringArray lvl;
   int n_lvl;
   int total;

   if(!dict) {
      mlog << Error << "\nparse_conf_i_vx_dict() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Check that this dictionary is an array
   if(!dict->is_array()) {
      mlog << Error << "\nparse_conf_i_vx_dict() -> "
           << "This function must be passed a Dictionary array.\n\n";
      exit(1);
   }

   // Loop over the fields to be verified
   for(int i=0,total=0; i<dict->n_entries(); i++) {

      // Get the level array, which may or may not be defined.
      // If defined, use its length.  If not, use a length of 1.
      lvl    = (*dict)[i]->dict_value()->lookup_string_array(conf_key_level, false);
      n_lvl  = (lvl.n() > 0 ? lvl.n() : 1);
      total += n_lvl;

      // Check if we're in the correct entry
      if(total > index) {

         // Copy the current entry's dictionary
         i_dict = *((*dict)[i]->dict_value());

         // Set up the new entry, taking only a single level value
         if(lvl.n() > 0) {
            entry.set_string(conf_key_level, lvl[index-(total-n_lvl)].c_str());
            i_dict.store(entry);
         }

         break;
      }
   } // end for i

   return i_dict;
}

///////////////////////////////////////////////////////////////////////////////

StringArray parse_conf_tc_model(Dictionary *dict, bool error_out) {
   const char *method_name = "parse_conf_tc_model() -> ";

   StringArray sa = parse_conf_string_array(dict, conf_key_model, method_name);

   // Print a warning if AVN appears in the model list
   for(int i=0; i<sa.n(); i++) {
      if(sa[i].find("AVN") != string::npos) {
         mlog << Warning << "\n" << method_name
              << "Requesting tropical cyclone model name \""  << sa[i]
              << "\" will yield no results since \"AVN\" is automatically "
              << "replaced with \"GFS\" when reading ATCF inputs. Please use "
              << "\"GFS\" in the \"" << conf_key_model << "\" entry of the "
              << "configuration file to read/process \"AVN\" entries.\n\n";
      }
   }

   return sa;
}

///////////////////////////////////////////////////////////////////////////////

StringArray parse_conf_message_type(Dictionary *dict, bool error_out) {
   const char *method_name = "parse_conf_message_type() -> ";

   StringArray sa = parse_conf_string_array(dict, conf_key_message_type, method_name);

   // Check that at least one message type is provided
   if(error_out && sa.n() == 0) {
      mlog << Error << "\n" << method_name
           << "At least one message type must be provided.\n\n";
      exit(1);
   }

   return sa;
}

///////////////////////////////////////////////////////////////////////////////

StringArray parse_conf_sid_list(Dictionary *dict, const char *conf_key) {
   StringArray sa, cur, sid_sa;
   ConcatString mask_name;
   const char *method_name = "parse_conf_sid_list() -> ";

   sa = parse_conf_string_array(dict, conf_key, method_name);

   // Parse station ID's to exclude from each entry
   for(int i=0; i<sa.n(); i++) {
     parse_sid_mask(string(sa[i]), cur, mask_name);
      sid_sa.add(cur);
   }

   mlog << Debug(4) << method_name
        << "Station ID \"" << conf_key << "\" list contains "
        << sid_sa.n() << " entries.\n";

   return sid_sa;
}

///////////////////////////////////////////////////////////////////////////////
//
// This function is passed a string containing either a file name or a list of
// values.  If it's a filename, parse out whitespace-separated values.  The
// first value is the name of the mask and the remaining values are the station
// ID's to be used.  If it's a string, interpret anything before a colon as the
// name of the mask and parse after the colon as a comma-separated list of
// station ID's to be used.  If no colon is present, select a default mask name.
// Store the results in the output StringArray.
//
///////////////////////////////////////////////////////////////////////////////

void parse_sid_mask(const ConcatString &mask_sid_str,
                    StringArray &mask_sid, ConcatString &mask_name) {
   ifstream in;
   ConcatString tmp_file;
   std::string sid_str;

   // Initialize
   mask_sid.clear();
   mask_name = na_str;

   // Check for an empty length string
   if(mask_sid_str.empty()) return;

   // Replace any instances of MET_BASE with it's expanded value
   tmp_file = replace_path(mask_sid_str.c_str());

   // Process file name
   if(file_exists(tmp_file.c_str())) {

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "parsing station ID masking file \"" << tmp_file << "\"\n";

      // Open the mask station id file specified
      in.open(tmp_file.c_str());

      if(!in) {
         mlog << Error << "\nparse_sid_mask() -> "
              << "Can't open the station ID masking file \""
              << tmp_file << "\".\n\n";
         exit(1);
      }

      // Store the first entry as the name of the mask
      in >> sid_str;
      mask_name = sid_str;

      // Store the rest of the entries as masking station ID's
      while(in >> sid_str) mask_sid.add(sid_str.c_str());

      // Close the input file
      in.close();

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "parsed " << mask_sid.n() << " station ID's for the \""
           << mask_name << "\" mask from file \"" << tmp_file << "\"\n";
   }
   // Process list of strings
   else {

      // Print a warning if the string contains a dot which suggests
      // the user was trying to specify a file name.
      if(check_reg_exp("[.]", mask_sid_str.c_str())) {
         mlog << Warning << "\nparse_sid_mask() -> "
              << "unable to process \"" << mask_sid_str
              << "\" as a file name and processing it as a single "
              << "station ID mask instead.\n\n";
      }

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "storing single station ID mask \"" << mask_sid_str << "\"\n";

      // Check for embedded whitespace or slashes
      if(check_reg_exp(ws_reg_exp, mask_sid_str.c_str()) ||
         check_reg_exp("[/]", mask_sid_str.c_str())) {
         mlog << Error << "\nparse_sid_mask() -> "
              << "masking station ID string can't contain whitespace or "
              << "slashes \"" << mask_sid_str << "\".\n\n";
         exit(1);
      }

      // Check for the optional mask name
      StringArray sa;
      sa = mask_sid_str.split(":");

      // One elements means no colon was specified
      if(sa.n() == 1) {
         mask_sid.add_css(sa[0]);
         mask_name = ( mask_sid.n() == 1 ? mask_sid[0] : "MASK_SID" );
      }
      // Two elements means one colon was specified
      else if(sa.n() == 2) {
         mask_name = sa[0];
         mask_sid.add_css(sa[1]);
      }
      else {
         mlog << Error << "\nparse_sid_mask() -> "
              << "masking station ID string may contain at most one colon to "
              << "specify the mask name \"" << mask_sid_str << "\".\n\n";
         exit(1);
      }

   }

   // Sort the mask_sid's
   mask_sid.sort();
   
   return;
}

///////////////////////////////////////////////////////////////////////////////


void MaskLatLon::clear() {
   name.clear();
   lat_thresh.clear();
   lon_thresh.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool MaskLatLon::operator==(const MaskLatLon &v) const {
   bool match = true;

   if(!(name       == v.name      ) ||
      !(lat_thresh == v.lat_thresh) ||
      !(lon_thresh == v.lon_thresh)) {
      match = false;
   }

   return match;
}

///////////////////////////////////////////////////////////////////////////////

MaskLatLon &MaskLatLon::operator=(const MaskLatLon &a) noexcept {
   if ( this != &a ) {
      name = a.name;
      lat_thresh = a.lat_thresh;
      lon_thresh = a.lon_thresh;
   }
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

vector<MaskLatLon> parse_conf_llpnt_mask(Dictionary *dict) {
   const DictionaryEntry *entry;
   Dictionary *llpnt_dict;
   vector<MaskLatLon> v;
   MaskLatLon m;
   int n_entries;

   if(!dict) {
      mlog << Error << "\nparse_conf_llpnt_mask() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Lookup the mask.point entry
   entry = dict->lookup(conf_key_mask_llpnt);

   // Process an array of dictionaries
   if(entry->is_array()) {
      llpnt_dict = entry->array_value();
      n_entries  = llpnt_dict->n_entries();
   }
   // Process a single dictionary
   else {
      llpnt_dict = entry->dict_value();
      n_entries  = 1;
   }

   // Loop through the array entries
   for(int i=0; i<n_entries; i++) {

      // Get the methods and widths for the current entry
      if(entry->type() == ArrayType) {
         m.name       = (*llpnt_dict)[i]->dict_value()->lookup_string(conf_key_name);
         m.lat_thresh = (*llpnt_dict)[i]->dict_value()->lookup_thresh(conf_key_lat_thresh);
         m.lon_thresh = (*llpnt_dict)[i]->dict_value()->lookup_thresh(conf_key_lon_thresh);
      }
      else {
         m.name       = llpnt_dict->lookup_string(conf_key_name);
         m.lat_thresh = llpnt_dict->lookup_thresh(conf_key_lat_thresh);
         m.lon_thresh = llpnt_dict->lookup_thresh(conf_key_lon_thresh);
      }

      // Add current MaskLatLon to the vector
      v.push_back(m);
   }

   return v;
}

///////////////////////////////////////////////////////////////////////////////

StringArray parse_conf_obs_qty_inc(Dictionary *dict) {
   StringArray sa;
   const char *method_name = "parse_conf_obs_qty_inc() -> ";

   // Check for old "obs_quality" entry
   sa = dict->lookup_string_array(conf_key_obs_qty, false);

   // Print a warning if the deprecated option was used
   if(dict->last_lookup_status()) {
      mlog << Warning << "\nparse_conf_obs_qty_inc() -> "
           << "Set the \"" << conf_key_obs_qty_inc << "\" value ("
           << write_css(sa) << ") from the deprecated \""
           << conf_key_obs_qty << "\" configuration entry.\n"
           << "Replace \"" << conf_key_obs_qty << "\" with \""
           << conf_key_obs_qty_inc << "\"!\n\n";
   }
   else {
      sa = parse_conf_string_array(dict, conf_key_obs_qty_inc, method_name);
   }

   return sa;
}

///////////////////////////////////////////////////////////////////////////////

StringArray parse_conf_obs_qty_exc(Dictionary *dict) {
   const char *method_name = "parse_conf_obs_qty_exc() -> ";
   
   StringArray sa = parse_conf_string_array(dict, conf_key_obs_qty_exc, method_name);
   
   return sa;
}

///////////////////////////////////////////////////////////////////////////////

NumArray parse_conf_ci_alpha(Dictionary *dict) {
   NumArray na;

   if(!dict) {
      mlog << Error << "\nparse_conf_ci_alpha() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   na = dict->lookup_num_array(conf_key_ci_alpha);

   // Check that at least one alpha value is provided
   if(na.n() == 0) {
      mlog << Error << "\nparse_conf_ci_alpha() -> "
           << "At least one confidence interval alpha value must be "
           << "specified.\n\n";
      exit(1);
   }

   // Check that the values for alpha are between 0 and 1
   for(int i=0; i<na.n(); i++) {
      if(na[i] <= 0.0 || na[i] >= 1.0) {
         mlog << Error << "\nparse_conf_ci_alpha() -> "
              << "All confidence interval alpha values ("
              << na[i] << ") must be greater than 0 "
              << "and less than 1.\n\n";
         exit(1);
      }
   }

   return na;
}

///////////////////////////////////////////////////////////////////////////////

NumArray parse_conf_eclv_points(Dictionary *dict) {
   NumArray na;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_eclv_points() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   na = dict->lookup_num_array(conf_key_eclv_points);

   // Check that at least one value is provided
   if(na.n() == 0) {
      mlog << Error << "\nparse_conf_eclv_points() -> "
           << "At least one \"" << conf_key_eclv_points
           << "\" entry must be specified.\n\n";
      exit(1);
   }

   // Intrepet a single value as the step size
   if(na.n() == 1) {
      for(i=2; i*na[0] < 1.0; i++) na.add(na[0]*i);
   }

   // Range check cost/loss ratios
   for(i=0; i<na.n(); i++) {
      if(na[i] <= 0.0 || na[i] >= 1.0) {
         mlog << Error << "\nparse_conf_eclv_points() -> "
              << "All cost/loss ratios (" << na[i]
              << ") must be greater than 0 and less than 1.\n\n";
         exit(1);
      }
   }

   return na;
}

///////////////////////////////////////////////////////////////////////////////

TimeSummaryInfo parse_conf_time_summary(Dictionary *dict) {
   Dictionary *ts_dict = (Dictionary *) nullptr;
   TimeSummaryInfo info;
   bool is_correct_type = false;

   if(!dict) {
      mlog << Error << "\nparse_conf_time_summary() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: time_summary
   ts_dict = dict->lookup_dictionary(conf_key_time_summary);

   // Conf: flag
   info.flag = ts_dict->lookup_bool(conf_key_flag);

   // Conf: flag
   info.raw_data = ts_dict->lookup_bool(conf_key_raw_data);

   // Conf: beg
   info.beg = timestring_to_sec(ts_dict->lookup_string(conf_key_beg).c_str());

   // Conf: end
   info.end = timestring_to_sec(ts_dict->lookup_string(conf_key_end).c_str());

   // Conf: step
   info.step = ts_dict->lookup_int(conf_key_step);
   if(info.step <= 0) {
      mlog << Error << "\nparse_conf_time_summary() -> "
           << "The \"" << conf_key_step << "\" parameter (" << info.step
           << ") must be greater than 0!\n\n";
      exit(1);
   }

   // Conf: width
   const DictionaryEntry * entry = ts_dict->lookup(conf_key_width);

   // Check that width is specified correctly
   if(entry) is_correct_type = (entry->type() == IntegerType ||
                                entry->type() == DictionaryType);

   if(!entry || !is_correct_type) {
      mlog << Error << "\nparse_conf_time_summary() -> "
           << "Lookup failed for name \"" << conf_key_width << "\"\n\n";
      exit(1);
   }

   // Parse width as an integer centered on the current timestamp
   if(entry->type() == IntegerType) {
      if(entry->i_value() <= 0) {
         mlog << Error << "\nparse_conf_time_summary() -> "
              << "The \"" << conf_key_width << "\" parameter ("
              << entry->i_value() << ") must be greater than 0!\n\n";
         exit(1);
      }
      info.width     = entry->i_value();
      info.width_beg = -1.0*nint(info.width/2.0);
      info.width_end = nint(info.width/2.0);
   }
   // Parse width as a dictionary
   else {
      parse_conf_range_int(entry->dict_value(), info.width_beg, info.width_end);
      info.width = info.width_end - info.width_beg;
   }

   // Conf: grib_code
   info.grib_code = ts_dict->lookup_int_array(conf_key_grib_code, false);
   info.obs_var   = ts_dict->lookup_string_array(conf_key_obs_var, false);

   // Conf: type
   info.type = ts_dict->lookup_string_array(conf_key_type);

   // Conf: vld_freq
   info.vld_freq = ts_dict->lookup_int(conf_key_vld_freq);

   // Conf: vld_thresh
   info.vld_thresh = ts_dict->lookup_double(conf_key_vld_thresh);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.vld_thresh < 0.0 || info.vld_thresh > 1.0) {
      mlog << Error << "\nparse_conf_time_summary() -> "
           << "The \"" << conf_key_time_summary << "."
           << conf_key_vld_thresh << "\" parameter (" << info.vld_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   return info;
}

///////////////////////////////////////////////////////////////////////////////
// Conf: map_name: message_type_map, obs_var_map, etc

void parse_add_conf_key_value_map(
      Dictionary *dict, const char *conf_key_map_name, map<ConcatString,ConcatString> *m) {

   if(!dict) {
      mlog << Error << "\nparse_add_conf_key_value_map() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   ConcatString key, val;
   Dictionary *map_dict = dict->lookup_array(conf_key_map_name);

   // Loop through the array entries
   for(int i=0; i<map_dict->n_entries(); i++) {

      // Lookup the key and value
      key = (*map_dict)[i]->dict_value()->lookup_string(conf_key_key);
      val = (*map_dict)[i]->dict_value()->lookup_string(conf_key_val);

      if(m->count(key) >= 1) {
         (*m)[key] = val;
      }
      else {
         // Add entry to the map
         m->insert(pair<ConcatString, ConcatString>(key, val));
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
// Conf: map_name: ugrid_metadata_maps

void parse_add_conf_key_values_map(
      Dictionary *dict, const char *conf_key_map_name,
      map<ConcatString,StringArray> *sa_map, const char *caller) {
   const char *method_name = (nullptr != caller) ? caller : "parse_conf_key_values_map() -> ";

   if(!dict) {
      mlog << Error << "\n" << method_name
           << "empty dictionary!\n\n";
      exit(1);
   }

   StringArray sa;
   map<ConcatString,ConcatString> cs_map = parse_conf_key_value_map(dict, conf_key_map_name, method_name);

   // Loop through the array entries and convert input comma-separated strings to StringArray
   for(map<ConcatString,ConcatString>::const_iterator it=cs_map.begin(); it!= cs_map.end(); it++) {
      sa.parse_css(it->second);
      (*sa_map)[it->first] = sa;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

map<ConcatString,ConcatString> parse_conf_key_value_map(
      Dictionary *dict, const char *conf_key_map_name, const char *caller) {
   Dictionary *map_dict = (Dictionary *) nullptr;
   map<ConcatString,ConcatString> m;
   ConcatString key, val;
   const char *method_name = (nullptr != caller) ? caller : "parse_conf_key_value_map() -> ";

   if(!dict) {
      mlog << Error << "\n" << method_name << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: map_name: message_type_map, obs_var_map, etc
   map_dict = dict->lookup_array(conf_key_map_name);

   // Loop through the array entries
   for(int i=0; i<map_dict->n_entries(); i++) {

      // Lookup the key and value
      key = (*map_dict)[i]->dict_value()->lookup_string(conf_key_key);
      val = (*map_dict)[i]->dict_value()->lookup_string(conf_key_val);

      if(m.count(key) >= 1) {
         mlog << Warning << "\n" << method_name
              << "found multiple entries for key \""
              << key << "\"!\n\n";
      }

      // Add entry to the map
      m.insert(pair<ConcatString, ConcatString>(key, val));
   }

   return m;
}


///////////////////////////////////////////////////////////////////////////////

map<ConcatString,StringArray> parse_conf_key_values_map(
      Dictionary *dict, const char *conf_key, const char *caller) {
   StringArray sa;
   map<ConcatString,ConcatString> cs_map;
   map<ConcatString,ConcatString>::const_iterator it;
   map<ConcatString,StringArray> sa_map;
   const char *method_name = (nullptr != caller) ? caller : "parse_conf_key_values_map() -> ";

   cs_map = parse_conf_key_value_map(dict, conf_key, method_name);

   // Convert input comma-separated strings to StringArray
   for(it=cs_map.begin(); it!= cs_map.end(); it++) {
      sa.parse_css(it->second);
      sa_map[it->first] = sa;
   }

   return sa_map;
}


///////////////////////////////////////////////////////////////////////////////

map<ConcatString,ConcatString> parse_conf_message_type_map(Dictionary *dict) {
   return parse_conf_key_value_map(dict, conf_key_message_type_map);
}

///////////////////////////////////////////////////////////////////////////////

map<ConcatString,StringArray> parse_conf_message_type_group_map(Dictionary *dict) {
   const char *method_name = "parse_conf_message_type_group_map() -> ";
   return parse_conf_key_values_map(dict, conf_key_message_type_group_map, method_name);
}

///////////////////////////////////////////////////////////////////////////////

map<ConcatString,StringArray> parse_conf_metadata_map(Dictionary *dict) {
   const char *method_name = "parse_conf_metadata_map() -> ";
   return parse_conf_key_values_map(dict, conf_key_metadata_map, method_name);
}

///////////////////////////////////////////////////////////////////////////////

map<ConcatString,ConcatString> parse_conf_obs_name_map(Dictionary *dict) {
   const char *method_name = "parse_conf_obs_name_map() -> ";
   return parse_conf_key_value_map(dict, conf_key_obs_name_map);
}

///////////////////////////////////////////////////////////////////////////////

map<ConcatString,StringArray> parse_conf_obs_to_qc_map(Dictionary *dict) {
   const char *method_name = "parse_conf_obs_to_qc_map() -> ";
   return parse_conf_key_values_map(dict, conf_key_obs_to_qc_map, method_name);
}

///////////////////////////////////////////////////////////////////////////////

map<ConcatString,UserFunc_1Arg> parse_conf_key_convert_map(
      Dictionary *dict, const char *conf_key_map_name, const char *caller) {
   Dictionary *map_dict = (Dictionary *) nullptr;
   StringArray sa;
   ConcatString key;
   UserFunc_1Arg fx;
   map<ConcatString,UserFunc_1Arg> m;
   const char *method_name = (nullptr != caller) ? caller : "parse_conf_key_convert_map() -> ";

   if(!dict) {
      mlog << Error << "\n" << method_name << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: diag_convert_map
   map_dict = dict->lookup_array(conf_key_map_name);

   // Loop through the array entries
   for(int i=0; i<map_dict->n_entries(); i++) {

      // Lookup the key and convert function
      sa =   (*map_dict)[i]->dict_value()->lookup_string_array(conf_key_key);
      fx.clear();
      fx.set((*map_dict)[i]->dict_value()->lookup(conf_key_convert));

      // Check the function
      if(!fx.is_set()) {
         mlog << Error << "\n" << method_name
              << "lookup for \"" << conf_key_convert << "\" failed in the \""
              << conf_key_map_name << "\" map!\n\n";
         exit(1);
      }

      // Add map entry for each string
      for(int j=0; j<sa.n(); j++) {

         key = sa[j];

         if(m.count(key) >= 1) {
            mlog << Warning << "\n" << method_name
                 << "found multiple entries for key \"" << key << "\" in the \""
                 << conf_key_map_name << "\" map!\n\n";
         }

         // Add entry to the map
         m.insert(pair<ConcatString,UserFunc_1Arg>(key,fx));

      } // end for j
   } // end for i

   return m;
}


///////////////////////////////////////////////////////////////////////////////


TimeSummaryInfo &TimeSummaryInfo::operator=(const TimeSummaryInfo &a) noexcept {
   if ( this != &a ) {
      flag = a.flag;
      raw_data = a.raw_data;
      beg = a.beg;
      end = a.end;
      step = a.step;
      width_beg = a.width_beg;
      width_end = a.width_end;
      width = a.width;
      grib_code = a.grib_code;
      obs_var = a.obs_var;
      type = a.type;
      vld_thresh = a.vld_thresh;
      vld_freq = a.vld_freq;
   }
   return *this;
}


///////////////////////////////////////////////////////////////////////////////


BootInfo & BootInfo::operator=(const BootInfo &a) noexcept {
   if ( this != &a ) {
     interval = a.interval;
     rep_prop = a.rep_prop;
     n_rep = a.n_rep;
     rng = a.rng;
     seed = a.seed;
   }
   return *this;
}

///////////////////////////////////////////////////////////////////////////////

void BootInfo::clear() {
   interval = BootIntervalType::None;
   rep_prop = bad_data_double;
   n_rep    = 0;
   rng.clear();
   seed.clear();
}


///////////////////////////////////////////////////////////////////////////////

BootInfo parse_conf_boot(Dictionary *dict) {
   BootInfo info;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: boot.interval
   v = dict->lookup_int(conf_key_boot_interval);

   // Convert integer to enumerated BootIntervalType
   if(v == conf_const.lookup_int(conf_val_bca)) {
      info.interval = BootIntervalType::BCA;
   }
   else if(v == conf_const.lookup_int(conf_val_pctile)) {
      info.interval = BootIntervalType::PCTile;
   }
   else {
      mlog << Error << "\nparse_conf_boot() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_boot_interval << "\".\n\n";
      exit(1);
   }

   // Conf: boot.rep_prop
   info.rep_prop = dict->lookup_double(conf_key_boot_rep_prop);

   // Check that it is between 0 and 1
   if(info.rep_prop <= 0.0 || info.rep_prop > 1.0) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "The \"" << conf_key_boot_rep_prop
           << "\" parameter (" << info.rep_prop
           << ")must be between 0 and 1!\n\n";
      exit(1);
   }

   // Conf: boot.n_rep
   info.n_rep = dict->lookup_int(conf_key_boot_n_rep);

   // Check n_rep >= 0
   if(info.n_rep < 0) {
      mlog << Error << "\nparse_conf_boot() -> "
           << "The number of bootstrap resamples in the \""
           << conf_key_boot_n_rep << "\" parameter (" << info.n_rep
           << ") must be >= 0.\n\n";
      exit(1);
   }

   // Conf: boot_rng
   info.rng = dict->lookup_string(conf_key_boot_rng);

   // Conf: boot_seed
   info.seed = dict->lookup_string(conf_key_boot_seed);

   return info;
}


///////////////////////////////////////////////////////////////////////////////

RegridInfo parse_conf_regrid(Dictionary *dict, bool error_out) {
   Dictionary *regrid_dict = (Dictionary *) nullptr;
   RegridInfo info;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_regrid() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: regrid
   regrid_dict = dict->lookup_dictionary(conf_key_regrid, false);

   // Check that the regrid dictionary is present
   if(!regrid_dict) {
      if(error_out) {
         mlog << Error << "\nparse_conf_regrid() -> "
              << "can't find the \"regrid\" dictionary!\n\n";
         exit(1);
      }
      else {
         return info;
      }
   }

   // Parse to_grid as an integer
   v = regrid_dict->lookup_int(conf_key_to_grid, false, false);

   // If integer lookup successful, convert to FieldType.
   if(regrid_dict->last_lookup_status()) {
      info.field  = int_to_fieldtype(v);
      info.enable = (info.field == FieldType::Fcst ||
                     info.field == FieldType::Obs);
   }
   // If integer lookup unsuccessful, parse vx_grid as a string.
   // Do not error out since to_grid isn't specified for climo.regrid.
   else {
      info.name   = regrid_dict->lookup_string(conf_key_to_grid, false);
      info.enable = true;
   }

   // Conf: vld_thresh
   double thr      = regrid_dict->lookup_double(conf_key_vld_thresh, false);
   info.vld_thresh = (is_bad_data(thr) ? default_vld_thresh : thr);

   // Parse the method and width
   info.method = int_to_interpmthd(regrid_dict->lookup_int(conf_key_method));
   info.width  = regrid_dict->lookup_int(conf_key_width);

   // Conf: shape
   v = regrid_dict->lookup_int(conf_key_shape, false);
   if (regrid_dict->last_lookup_status()) {
      info.shape = int_to_gridtemplate(v);
   }
   else {
      // If not specified, use the default square shape
      info.shape = GridTemplateFactory::GridTemplates::Square;
   }

   // Conf: gaussian dx and radius
   double conf_value = regrid_dict->lookup_double(conf_key_gaussian_dx, false);
   info.gaussian.dx = (is_bad_data(conf_value) ? default_gaussian_dx : conf_value);
   conf_value = regrid_dict->lookup_double(conf_key_gaussian_radius, false);
   info.gaussian.radius = (is_bad_data(conf_value) ? default_gaussian_radius : conf_value);
   conf_value = regrid_dict->lookup_double(conf_key_trunc_factor, false);
   info.gaussian.trunc_factor = (is_bad_data(conf_value) ? default_trunc_factor : conf_value);
   if (info.method == InterpMthd::Gaussian || info.method == InterpMthd::MaxGauss) info.gaussian.compute();

   // MET#2437 Do not search the higher levels of config file context for convert,
   //          censor_thresh, and censor_val. They must be specified within the
   //          regrid dictionary itself.

   // Conf: convert
   info.convert_fx.set(regrid_dict->lookup(conf_key_convert, false));

   // Conf: censor_thresh
   info.censor_thresh = regrid_dict->lookup_thresh_array(conf_key_censor_thresh, false, true, false);

   // Conf: censor_val
   info.censor_val = regrid_dict->lookup_num_array(conf_key_censor_val, false, true, false);

   // Validate the settings
   info.validate();

   return info;
}

///////////////////////////////////////////////////////////////////////////////

void InterpInfo::clear() {
   field = FieldType::None;
   vld_thresh = bad_data_double;
   n_interp = 0;
   method.clear();
   width.clear();
   gaussian.clear();
   shape = GridTemplateFactory::GridTemplates::None;
}

///////////////////////////////////////////////////////////////////////////////

void InterpInfo::validate() {

   for(int i=0; i<n_interp; i++) {

      InterpMthd methodi = string_to_interpmthd(method[i].c_str());

      // Check the nearest neighbor special case
      if(width[i] == 1 &&
         methodi  != InterpMthd::None &&
         methodi  != InterpMthd::Nearest &&
         methodi  != InterpMthd::Force &&
         methodi  != InterpMthd::Upper_Left &&
         methodi  != InterpMthd::Upper_Right &&
         methodi  != InterpMthd::Lower_Right &&
         methodi  != InterpMthd::Lower_Left &&
         methodi  != InterpMthd::Gaussian &&
         methodi  != InterpMthd::MaxGauss) {
         mlog << Warning << "\nInterpInfo::validate() -> "
              << "Resetting interpolation method " << (int) i << " from \""
              << method[i] << "\" to \""
              << interpmthd_nearest_str
              << "\" since the interpolation width is 1.\n\n";
         method.set(i, interpmthd_nearest_str);
      }

      // Check for some methods, that width is 1
      if((methodi == InterpMthd::Nearest ||
          methodi == InterpMthd::Upper_Left ||
          methodi == InterpMthd::Upper_Right ||
          methodi == InterpMthd::Lower_Right ||
          methodi == InterpMthd::Lower_Left) &&
         width[i] != 1) {
         mlog << Warning << "\nInterpInfo::validate() -> "
              << "Resetting interpolation width " << (int) i << " from "
              << width[i] << " to 1 for interpolation method \""
              << method[i] << "\".\n\n";
         width.set(i, 1);
      }

      // Check the bilinear and budget special cases
      if((methodi == InterpMthd::Bilin ||
          methodi == InterpMthd::Budget) &&
         width[i] != 2) {
         mlog << Warning << "\nInterpInfo::validate() -> "
              << "Resetting interpolation width " << (int) i << " from "
              << width[i] << " to 2 for interpolation method \""
              << method[i] << "\".\n\n";
         width.set(i, 2);
      }

      // Check the Gaussian filter
      if(methodi == InterpMthd::Gaussian ||
         methodi == InterpMthd::MaxGauss) {
         if (gaussian.radius < gaussian.dx) {
            mlog << Error << "\n"
                 << "The radius of influence (" << gaussian.radius
                 << ") is less than the delta distance (" << gaussian.dx
                 << ") for regridding method \"" << method[i] << "\".\n\n";
            exit(1);
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

bool InterpInfo::operator==(const InterpInfo &v) const {
   bool match = true;

   if(!(field      == v.field     ) ||
      !(vld_thresh == v.vld_thresh) ||
      !(n_interp   == v.n_interp  ) ||
      !(method     == v.method    ) ||
      !(width      == v.width     ) ||
      !(shape      == v.shape     )) {
      match = false;
   }

   return match;
}

///////////////////////////////////////////////////////////////////////////////

InterpInfo & InterpInfo::operator=(const InterpInfo &a) noexcept {
   if ( this != &a ) {
      field = a.field;
      vld_thresh = a.vld_thresh;
      n_interp = a.n_interp;
      method = a.method;
      width = a.width;
      gaussian = a.gaussian;
      shape = a.shape;
   }
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

InterpInfo parse_conf_interp(Dictionary *dict, const char *conf_key) {
   Dictionary *interp_dict = (Dictionary *) nullptr;
   Dictionary *type_dict = (Dictionary *) nullptr;
   InterpInfo info;
   NumArray mthd_na;
   NumArray wdth_na;
   InterpMthd method;

   int v;
   int width;
   int n_entries;
   bool is_correct_type = false;

   if(!dict) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: interp
   interp_dict = dict->lookup_dictionary(conf_key);

   // Conf: field - may be missing
   v = interp_dict->lookup_int(conf_key_field, false);

   // If found, interpret value.  Otherwise, set to a default value.
   if(interp_dict->last_lookup_status()) info.field = int_to_fieldtype(v);
   else                                  info.field = FieldType::None;

   // Conf: vld_thresh
   double thr      = interp_dict->lookup_double(conf_key_vld_thresh, false);
   info.vld_thresh = (is_bad_data(thr) ? default_vld_thresh : thr);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.vld_thresh < 0.0 || info.vld_thresh > 1.0) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "The \"" << conf_key << "." << conf_key_vld_thresh
           << "\" parameter (" << info.vld_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: shape
   v = interp_dict->lookup_int(conf_key_shape, false);
   if (interp_dict->last_lookup_status()) {
      info.shape = int_to_gridtemplate(v);
   }
   else {
      // If not specified, use the default square shape
      info.shape = GridTemplateFactory::GridTemplates::Square;
   }

   // Conf: gaussian dx and radius
   double conf_value = interp_dict->lookup_double(conf_key_gaussian_dx, false);
   info.gaussian.dx = (is_bad_data(conf_value) ? default_gaussian_dx : conf_value);
   conf_value = interp_dict->lookup_double(conf_key_gaussian_radius, false);
   info.gaussian.radius = (is_bad_data(conf_value) ? default_gaussian_radius : conf_value);
   conf_value = interp_dict->lookup_double(conf_key_trunc_factor, false);
   info.gaussian.trunc_factor = (is_bad_data(conf_value) ? default_trunc_factor : conf_value);

   // Conf: type
   const DictionaryEntry * type_entry = interp_dict->lookup(conf_key_type);

   if(type_entry) is_correct_type = (type_entry->type() == ArrayType ||
                                     type_entry->type() == DictionaryType);

   // Check that type is a dictionary or array of dictionaries
   if(!type_entry || !is_correct_type) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "Lookup failed for name \"" << conf_key_type << "\"\n\n";
      exit(1);
   }

   if(type_entry->type() == ArrayType) {
      type_dict = type_entry->array_value();
      n_entries = type_dict->n_entries();
   }
   else {
      type_dict = type_entry->dict_value();
      n_entries = 1;
   }

   // Loop over the interpolation type dictionary entries
   info.n_interp = 0;
   for(int i=0; i<n_entries; i++) {

      // Get the methods and widths for the current entry
      if(type_entry->type() == ArrayType) {
         mthd_na = (*type_dict)[i]->dict_value()->lookup_num_array(conf_key_method);
         wdth_na = (*type_dict)[i]->dict_value()->lookup_num_array(conf_key_width);
      }
      else {
         mthd_na = type_dict->lookup_num_array(conf_key_method);
         wdth_na = type_dict->lookup_num_array(conf_key_width);
      }

      // Loop over the methods
      for(int j=0; j<mthd_na.n(); j++) {

         // Store interpolation method as a string
         method = int_to_interpmthd(mthd_na[j]);

         // Check for unsupported interpolation options
         if(method == InterpMthd::Budget ||
            method == InterpMthd::Force) {
            mlog << Error << "\nparse_conf_interp() -> "
                 << "\"" << interpmthd_to_string(method)
                 << "\" not valid for interpolating, only regridding.\n\n";
            exit(1);
         }

         // Loop over the widths
         for(int k=0; k<wdth_na.n(); k++) {

            // Store the current width
            width = nint(wdth_na[k]);

            // Add the current entries
            info.n_interp += 1;
            info.method.add(interpmthd_to_string(method));
            info.width.add(width);

         } // end for k

         if(method == InterpMthd::Gaussian || method == InterpMthd::MaxGauss) {
            info.gaussian.compute();
         }
      } // end for j
   } // end for i

   // Check for at least one interpolation method
   if(info.n_interp == 0) {
      mlog << Error << "\nparse_conf_interp() -> "
           << "Must define at least one interpolation method in the config "
           << "file.\n\n";
      exit(1);
   }

   info.validate();

   return info;
}

///////////////////////////////////////////////////////////////////////////////

void ClimoCDFInfo::clear() {
   flag = false;
   n_bin = 0;
   cdf_ta.clear();
   write_bins = false;
   direct_prob = false;
}

///////////////////////////////////////////////////////////////////////////////

ClimoCDFInfo::ClimoCDFInfo() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

void ClimoCDFInfo::set_cdf_ta(int n_bin, bool &center) {

   // Must be greater than 0
   if(n_bin <= 0) {
      mlog << Error << "\nClimoCDFInfo::set_cdf_ta() -> "
           << "The \"" << conf_key_cdf_bins << "\" entry (" << n_bin
           << ") must be greater than zero.\n\n";
      exit(1);
   }

   // Initialize
   cdf_ta.clear();

   // Even number of bins cannot be centered
   if(n_bin%2 == 0 && center) {
      mlog << Warning <<  "\nClimoCDFInfo::set_cdf_ta() -> "
           << "Resetting \"" << conf_key_center_bins
           << "\" to false since the \"" << conf_key_cdf_bins
           << "\" entry (" << n_bin << ") is even.\n\n";
      center = false;
   }

   // For a single bin, set center to false
   if(n_bin == 1) center = false;

   // Add the first threshold for 0.0
   cdf_ta.add(0.0, thresh_ge);

   double cdf_inc = (center ? 1.0/(n_bin - 1.0) : 1.0/n_bin);
   double cdf_val = (center ? cdf_inc/2         : cdf_inc  );

   // Add thresholds between 0.0 and 1.0
   while(cdf_val < 1.0 && !is_eq(cdf_val, 1.0)) {
      cdf_ta.add(cdf_val, thresh_ge);
      cdf_val += cdf_inc;
   }

   // Add the last threshold for 1.0
   cdf_ta.add(1.0, thresh_ge);

   if(n_bin == 1) {
      mlog << Debug(4) << "ClimoCDFInfo::set_cdf_ta() -> "
           << "Since \"" << conf_key_cdf_bins << "\" = 1, "
           << "no climatology CDF bins will be applied.\n";
   }
   else {
      mlog << Debug(4) << "ClimoCDFInfo::set_cdf_ta() -> "
           << "For \"" << conf_key_cdf_bins << "\" (" << n_bin << ") and \""
           << conf_key_center_bins << "\" (" << bool_to_string(center)
           << "), defined climatology CDF thresholds: "
           << write_css(cdf_ta) << "\n";
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

ClimoCDFInfo &ClimoCDFInfo::operator=(const ClimoCDFInfo &a) noexcept {
   if ( this != &a ) {
      flag = a.flag;
      n_bin = a.n_bin;
      cdf_ta = a.cdf_ta;
      write_bins = a.write_bins;
      direct_prob = a.direct_prob;
   }
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

ClimoCDFInfo parse_conf_climo_cdf(Dictionary *dict) {
   Dictionary *cdf_dict = (Dictionary *) nullptr;
   ClimoCDFInfo info;
   NumArray bins;
   bool center;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_climo_cdf() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: climo_cdf
   cdf_dict = dict->lookup_dictionary(conf_key_climo_cdf);

   // Conf: cdf_bins
   bins = cdf_dict->lookup_num_array(conf_key_cdf_bins);

   // Conf: center_bins
   center = cdf_dict->lookup_bool(conf_key_center_bins);

   // Conf: write_bins
   // Used by Grid-Stat and Point-Stat
   // Not used by Ensemble-Stat or Series-Analysis
   info.write_bins = cdf_dict->lookup_bool(conf_key_write_bins, false, false);

   // Conf: direct_prob
   info.direct_prob = cdf_dict->lookup_bool(conf_key_direct_prob, false, false);

   // Check that at least one value is provided
   if(bins.n() == 0) {
      mlog << Error << "\nparse_conf_climo_cdf() -> "
           << "At least one \"" << conf_key_cdf_bins
           << "\" entry must be specified.\n\n";
      exit(1);
   }

   // The bins are explicitly defined
   if(bins.n() > 1) {
      for(i=0; i<bins.n(); i++) info.cdf_ta.add(bins[i], thresh_ge);
   }
   // Interpret a single value as the number of bins
   else {
      info.set_cdf_ta(bins[0], center);
   }

   // Sanity check the end points
   if(!is_eq(info.cdf_ta[0].get_value(), 0.0) ||
      !is_eq(info.cdf_ta[info.cdf_ta.n()-1].get_value(), 1.0)) {
      mlog << Error << "\nparse_conf_climo_cdf() -> "
           << "The \"" << conf_key_cdf_bins << "\" entries must "
           << "start with 0 and end with 1.\n\n";
      exit(1);
   }

   // Sanity check the interior points
   for(i=0; i<info.cdf_ta.n(); i++) {
      if(info.cdf_ta[i].get_value() < 0 ||
         info.cdf_ta[i].get_value() > 1.0) {
         mlog << Error << "\nparse_conf_climo_cdf() -> "
              << "The \"" << conf_key_cdf_bins << "\" entries ("
              << info.cdf_ta[i].get_value()
              << ") must be between 0 and 1.\n\n";
         exit(1);
      }
   }

   // Should be monotonically increasing
   info.cdf_ta.check_bin_thresh();

   // Set the number of bins and the flag
   info.n_bin = info.cdf_ta.n() - 1;
   info.flag  = (info.n_bin > 1);

   return info;
}

///////////////////////////////////////////////////////////////////////////////


void NbrhdInfo::clear() {
   field = FieldType::None;
   vld_thresh = bad_data_double;
   width.clear();
   cov_ta.clear();
   shape = GridTemplateFactory::GridTemplates::None;
}

///////////////////////////////////////////////////////////////////////////////

NbrhdInfo &NbrhdInfo::operator=(const NbrhdInfo &a) noexcept {
   if ( this != &a ) {
      field = a.field;
      vld_thresh = a.vld_thresh;
      width = a.width;
      cov_ta = a.cov_ta;
      shape = a.shape;
   }
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

NbrhdInfo parse_conf_nbrhd(Dictionary *dict, const char *conf_key) {
   Dictionary *nbrhd_dict = (Dictionary *) nullptr;
   NbrhdInfo info;
   int i;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_nbrhd() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: nbrhd
   nbrhd_dict = dict->lookup_dictionary(conf_key);

   // Conf: field - may be missing

   // Default info.field to BOTH
   info.field = FieldType::Both;

   // Skip lookup for conf_key_nbrhd_prob
   if(strncmp(conf_key, conf_key_nbrhd_prob, strlen(conf_key_nbrhd_prob)) != 0) {

      v = nbrhd_dict->lookup_int(conf_key_field, false);

      // If found, interpret value
      if(nbrhd_dict->last_lookup_status()) info.field = int_to_fieldtype(v);
   }

   // Conf: vld_thresh
   info.vld_thresh = nbrhd_dict->lookup_double(conf_key_vld_thresh);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.vld_thresh < 0.0 || info.vld_thresh > 1.0) {
      mlog << Error << "\nparse_conf_nbrhd() -> "
           << "The \"" << conf_key << "." << conf_key_vld_thresh
           << "\" parameter (" << info.vld_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: width
   info.width = nbrhd_dict->lookup_num_array(conf_key_width);

   // Check that at least one neighborhood width is provided
   if(info.width.n() == 0) {
      mlog << Error << "\nparse_conf_nbrhd() -> "
           << "At least one neighborhood width must be provided.\n\n";
      exit(1);
   }

   // Check for valid widths
   for(i=0; i<info.width.n(); i++) {

      if(info.width[i] < 1 || info.width[i]%2 == 0) {
         mlog << Error << "\nparse_conf_nbrhd() -> "
              << "The neighborhood widths must be odd values greater "
              << "than or equal to 1 (" << info.width[i] << ").\n\n";
         exit(1);
      }
   }

   // Conf: shape
   v = nbrhd_dict->lookup_int(conf_key_shape, false);
   if (nbrhd_dict->last_lookup_status()) {
      info.shape = int_to_gridtemplate(v);
   }
   else {
      // If not specified, use the default square shape
      info.shape = GridTemplateFactory::GridTemplates::Square;
   }

   // Conf: cov_thresh
   info.cov_ta = nbrhd_dict->lookup_thresh_array(conf_key_cov_thresh, false);

   // Check for valid coverage thresholds
   for(i=0; i<info.cov_ta.n(); i++) {

      if(info.cov_ta[i].get_value() < 0.0 ||
         info.cov_ta[i].get_value() > 1.0) {
         mlog << Error << "\nparse_conf_nbrhd() -> "
              << "The neighborhood coverage threshold value must be set "
              << "between 0 and 1.\n\n";
         exit(1);
      }
   }

   return info;
}

///////////////////////////////////////////////////////////////////////////////


void HiRAInfo::clear() {
   flag = false;
   width.clear();
   vld_thresh = bad_data_double;
   cov_ta.clear();
   prob_cat_ta.clear();
   shape = GridTemplateFactory::GridTemplates::None;
}

///////////////////////////////////////////////////////////////////////////////

HiRAInfo &HiRAInfo::operator=(const HiRAInfo &a) noexcept {
   if ( this != &a ) {
      flag = a.flag;
      width = a.width;
      vld_thresh = a.vld_thresh;
      cov_ta = a.cov_ta;
      prob_cat_ta = a.prob_cat_ta;
      shape = a.shape;
   }
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

HiRAInfo::HiRAInfo() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

HiRAInfo parse_conf_hira(Dictionary *dict) {
   Dictionary *hira_dict = (Dictionary *) nullptr;
   HiRAInfo info;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_hira() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: hira
   hira_dict = dict->lookup_dictionary(conf_key_hira);

   // Conf: flag
   info.flag = hira_dict->lookup_bool(conf_key_flag);

   // If disabled, skip remainder of the dictionary.
   if(!info.flag) return info;

   // Conf: vld_thresh
   info.vld_thresh = hira_dict->lookup_double(conf_key_vld_thresh);

   // Check that the interpolation threshold is between 0 and 1.
   if(info.vld_thresh < 0.0 || info.vld_thresh > 1.0) {
      mlog << Error << "\nparse_conf_hira() -> "
           << "The \"" << conf_key_hira << "." << conf_key_vld_thresh
           << "\" parameter (" << info.vld_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

   // Conf: width
   info.width = hira_dict->lookup_num_array(conf_key_width);

   // Check that at least one width is provided
   if(info.width.n() == 0) {
      mlog << Error << "\nparse_conf_hira() -> "
           << "At least one HiRA width must be provided.\n\n";
      exit(1);
   }

   // Check for valid widths
   for(int i=0; i<info.width.n(); i++) {

      if(info.width[i] < 1) {
         mlog << Error << "\nparse_conf_hira() -> "
              << "The HiRA widths must be greater than or equal to 1 ("
              << info.width[i] << ").\n\n";
         exit(1);
      }
   }

   // Conf: shape
   v = hira_dict->lookup_int(conf_key_shape, false);
   if (hira_dict->last_lookup_status()) {
      info.shape = int_to_gridtemplate(v);
   }
   else {
      // If not specified, use the default square shape
      info.shape = GridTemplateFactory::GridTemplates::Square;
   }

   // Conf: cov_thresh
   info.cov_ta = hira_dict->lookup_thresh_array(conf_key_cov_thresh);

   // Pass coverage thresholds through probaiblity logic
   info.cov_ta = string_to_prob_thresh(info.cov_ta.get_str().c_str());

   // Error check the coverage (probability) thresholds
   check_prob_thresh(info.cov_ta);

   // Conf: prob_cat_thresh
   info.prob_cat_ta = hira_dict->lookup_thresh_array(conf_key_prob_cat_thresh);

   return info;
}

///////////////////////////////////////////////////////////////////////////////

GridWeightType parse_conf_grid_weight_flag(Dictionary *dict) {
   GridWeightType t = GridWeightType::None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_grid_weight_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_grid_weight_flag);

   // Convert integer to enumerated GridWeightType
        if(v == conf_const.lookup_int(conf_val_none))    t = GridWeightType::None;
   else if(v == conf_const.lookup_int(conf_val_cos_lat)) t = GridWeightType::Cos_Lat;
   else if(v == conf_const.lookup_int(conf_val_area))    t = GridWeightType::Area;
   else {
      mlog << Error << "\nparse_conf_grid_weight_flag() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_grid_weight_flag << "\".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

DuplicateType parse_conf_duplicate_flag(Dictionary *dict) {
   DuplicateType t = DuplicateType::None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_duplicate_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_duplicate_flag);

   // Convert integer to enumerated DuplicateType
        if(v == conf_const.lookup_int(conf_val_none))   t = DuplicateType::None;
   else if(v == conf_const.lookup_int(conf_val_unique)) t = DuplicateType::Unique;
   else if(v == conf_const.lookup_int(conf_val_single)) {
     mlog << Error << "\nparse_conf_duplicate_flag() -> "
          << "duplicate_flag = SINGLE has been deprecated\n"
          << "Please use obs_summary = NEAREST;\n\n";
     exit(1);
   }
   else {
      mlog << Error << "\nparse_conf_duplicate_flag() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_duplicate_flag << "\".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ObsSummary parse_conf_obs_summary(Dictionary *dict) {
   ObsSummary t = ObsSummary::None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_obs_summary() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_obs_summary);

   // Convert integer to enumerated ObsSummary
        if(v == conf_const.lookup_int(conf_val_none))    t = ObsSummary::None;
   else if(v == conf_const.lookup_int(conf_val_nearest)) t = ObsSummary::Nearest;
   else if(v == conf_const.lookup_int(conf_val_min))     t = ObsSummary::Min;
   else if(v == conf_const.lookup_int(conf_val_max))     t = ObsSummary::Max;
   else if(v == conf_const.lookup_int(conf_val_uw_mean)) t = ObsSummary::UW_Mean;
   else if(v == conf_const.lookup_int(conf_val_dw_mean)) t = ObsSummary::DW_Mean;
   else if(v == conf_const.lookup_int(conf_val_median))  t = ObsSummary::Median;
   else if(v == conf_const.lookup_int(conf_val_perc))    t = ObsSummary::Perc;
   else {
      mlog << Error << "\nparse_conf_obs_summary() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_obs_summary << "\".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

int parse_conf_percentile(Dictionary *dict) {
   int i = bad_data_int;

   if(!dict) {
      mlog << Error << "\nparse_conf_percentile() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   i = dict->lookup_int(conf_key_percentile);

   // Check that the values for alpha are between 0 and 1
   if(i <= 0 || i >= 100) {
      mlog << Error << "\nparse_conf_percentile() -> "
           << "Percentile value ("
           << i << ") must be greater than 0 "
           << "and less than 100.\n\n";
         exit(1);
   }

   return i;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_tmp_dir(Dictionary *dict) {
   DIR* odir = nullptr;
   ConcatString tmp_dir_path;

   if(!get_env("MET_TMP_DIR", tmp_dir_path)) {
      if(!dict) {
         mlog << Error << "\nparse_conf_tmp_dir() -> "
              << "empty dictionary!\n\n";
         exit(1);
      }
      // Read the temporary directory
      tmp_dir_path = dict->lookup_string(conf_key_tmp_dir);
   }

   // Make sure that it exists
   if((odir = met_opendir(tmp_dir_path.c_str())) == nullptr) {
      mlog << Error << "\nparse_conf_tmp_dir() -> "
           << "Cannot access the \"" << conf_key_tmp_dir << "\" directory: "
           << tmp_dir_path << "\n\n";
      exit(1);
   }
   else {
      met_closedir(odir);
   }

   return tmp_dir_path;
}

///////////////////////////////////////////////////////////////////////////////

GridDecompType parse_conf_grid_decomp_flag(Dictionary *dict) {
   GridDecompType t = GridDecompType::None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_grid_decomp_flag() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_grid_decomp_flag);

   // Convert integer to enumerated GridDecompType
        if(v == conf_const.lookup_int(conf_val_none)) t = GridDecompType::None;
   else if(v == conf_const.lookup_int(conf_val_auto)) t = GridDecompType::Auto;
   else if(v == conf_const.lookup_int(conf_val_tile)) t = GridDecompType::Tile;
   else if(v == conf_const.lookup_int(conf_val_pad))  t = GridDecompType::Pad;
   else {
      mlog << Error << "\nparse_conf_grid_decomp_flag() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_grid_decomp_flag << "\".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

WaveletType parse_conf_wavelet_type(Dictionary *dict) {
   WaveletType t = WaveletType::None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_wavelet_type() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_wavelet_type);

   // Convert integer to enumerated WaveletType
        if(v == conf_const.lookup_int(conf_val_none))         t = WaveletType::None;
   else if(v == conf_const.lookup_int(conf_val_haar))         t = WaveletType::Haar;
   else if(v == conf_const.lookup_int(conf_val_haar_cntr))    t = WaveletType::Haar_Cntr;
   else if(v == conf_const.lookup_int(conf_val_daub))         t = WaveletType::Daub;
   else if(v == conf_const.lookup_int(conf_val_daub_cntr))    t = WaveletType::Daub_Cntr;
   else if(v == conf_const.lookup_int(conf_val_bspline))      t = WaveletType::BSpline;
   else if(v == conf_const.lookup_int(conf_val_bspline_cntr)) t = WaveletType::BSpline_Cntr;
   else {
      mlog << Error << "\nparse_conf_wavelet_type() -> "
           << "Unexpected config file value of " << v << " for \""
           << conf_key_wavelet_type << "\".\n\n";
      exit(1);
   }

   return t;
}


///////////////////////////////////////////////////////////////////////////////


ConcatString parse_conf_ugrid_coordinates_file(Dictionary *dict) {
   return dict ? dict->lookup_string(conf_key_ugrid_coordinates_file, false) : "";
}

///////////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_ugrid_dataset(Dictionary *dict) {
   return dict ? dict->lookup_string(conf_key_ugrid_dataset, false) : "";
}

///////////////////////////////////////////////////////////////////////////////

double parse_conf_ugrid_max_distance_km(Dictionary *dict) {
   return dict ? dict->lookup_double(conf_key_ugrid_max_distance_km, false) : bad_data_double;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_ugrid_map_config(Dictionary *dict) {
   return dict ? dict->lookup_string(conf_key_ugrid_map_config, false) : "";
}

///////////////////////////////////////////////////////////////////////////////

void parse_add_conf_ugrid_metadata_map(Dictionary *dict, map<ConcatString,StringArray> *m) {
   parse_add_conf_key_values_map(dict, conf_key_ugrid_metadata_map, m,
                                 "parse_add_conf_ugrid_metadata_map");
}


///////////////////////////////////////////////////////////////////////////////


void PlotInfo::clear() {
   flag = true; // enabled by default
   color_table.clear();
   plot_min = bad_data_double;
   plot_max = bad_data_double;
   colorbar_flag = true; // plot colorbar by default
}

///////////////////////////////////////////////////////////////////////////////

PlotInfo &PlotInfo::operator=(const PlotInfo &a) noexcept {
   if ( this != &a ) {
      flag = a.flag;
      color_table = a.color_table;
      plot_min = a.plot_min;
      plot_max = a.plot_max;
      colorbar_flag = a.colorbar_flag;
   }
   return *this;
}


///////////////////////////////////////////////////////////////////////////////

PlotInfo parse_conf_plot_info(Dictionary *dict) {
   PlotInfo info;

   if(!dict) {
      mlog << Error << "\nparse_conf_plot_info() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the flag, true if not present
   info.flag = dict->lookup_bool(conf_key_flag, false);
   if(!dict->last_lookup_status()) info.flag = true;

   // Get the color table
   info.color_table = dict->lookup_string(conf_key_color_table);

   // Get the minimum plot value, 0 if not present
   info.plot_min = dict->lookup_double(conf_key_plot_min, false);
   if(is_bad_data(info.plot_min)) info.plot_min = 0.0;

   // Get the maximum plot value, 0 if not present
   info.plot_max = dict->lookup_double(conf_key_plot_max, false);
   if(is_bad_data(info.plot_max)) info.plot_max = 0.0;

   // Get the colorbar flag, true if not present
   info.colorbar_flag = dict->lookup_bool(conf_key_colorbar_flag, false);
   if(!dict->last_lookup_status()) info.colorbar_flag = true;

   return info;
}

///////////////////////////////////////////////////////////////////////////////

map<ConcatString,ThreshArray> parse_conf_filter_attr_map(
      Dictionary *dict) {
   map<ConcatString,ThreshArray> m;
   SingleThresh st;
   StringArray sa;
   ThreshArray ta;
   ThreshArray ta_entry;

   if(!dict) {
      mlog << Error << "\nparse_conf_filter_attr_map() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: filter_attr_name and filter_attr_thresh
   sa = dict->lookup_string_array(conf_key_filter_attr_name);
   ta = dict->lookup_thresh_array(conf_key_filter_attr_thresh);

   // Check for equal number of names and thresholds
   if(sa.n() != ta.n()) {
      mlog << Error << "\nparse_conf_filter_attr_map() -> "
           << "The \"" << conf_key_filter_attr_name << "\" and \""
           << conf_key_filter_attr_thresh
           << "\" arrays must have the same length.\n\n";
      exit(1);
   }

   // Append area_thresh, if present
   st = dict->lookup_thresh(conf_key_area_thresh, false);
   if(dict->last_lookup_status()) {
      sa.add("AREA");
      ta.add(st);
   }

   // Append inten_perc_thresh, if present
   st = dict->lookup_thresh(conf_key_inten_perc_thresh, false);
   if(dict->last_lookup_status()) {
      ConcatString cs;
      cs << "INTENSITY_" << dict->lookup_int(conf_key_inten_perc_value);
      sa.add(cs);
      ta.add(st);
   }

   // Process each array entry
   for(int i=0; i<sa.n(); i++) {

      // Add threshold to existing map entry
     if(m.count(string(sa[i])) >= 1) {
       m[string(sa[i])].add(ta[i]);
      }
      // Add a new map entry
      else {
         ta_entry.clear();
         ta_entry.add(ta[i]);
         m[string(sa[i])] = ta_entry;
      }
   }

   return m;
}

///////////////////////////////////////////////////////////////////////////////

void parse_conf_range_int(Dictionary *dict, int &beg, int &end) {

   if(!dict) {
      mlog << Error << "\nparse_conf_range_int() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Lookup the integer values
   beg = dict->lookup_int(conf_key_beg);
   end = dict->lookup_int(conf_key_end);

   // Check the range
   if(beg > end) {
      mlog << Error << "\nparse_conf_range_int() -> "
           << "the ending value (" << end
           << ") must be >= the beginning value (" << beg << ").\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void parse_conf_range_double(Dictionary *dict, double &beg, double &end) {

   if(!dict) {
      mlog << Error << "\nnparse_conf_range_double -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Lookup the double values
   beg = dict->lookup_double(conf_key_beg);
   end = dict->lookup_double(conf_key_end);

   // Check the range
   if(beg > end) {
      mlog << Error << "\nparse_conf_range_double() -> "
           << "the ending value (" << end
           << ") must be >= the beginning value (" << beg << ").\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void check_mask_names(const StringArray &sa) {
   StringArray sa_uniq = sa.uniq();

   // Check for unique mask names
   if(sa_uniq.n() < sa.n()) {
      mlog << Error << "\ncheck_mask_names() -> "
           << "found non-unique strings in the list of masking region names ("
           << write_css(sa) << ")!\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void check_climo_n_vx(Dictionary *dict, const int n_vx) {
   int n;

   // Check for a valid number of climatology mean fields
   n = parse_conf_n_vx(dict->lookup_array(conf_key_climo_mean_field, false));
   if(n != 0 && n != n_vx) {
      mlog << Error << "\ncheck_climo_n_vx() -> "
           << "The number of climatology mean fields in \""
           << conf_key_climo_mean_field
           << "\" must be zero or match the number (" << n_vx
           << ") in \"" << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   // Check for a valid number of climatology standard deviation fields
   n = parse_conf_n_vx(dict->lookup_array(conf_key_climo_stdev_field, false));
   if(n != 0 && n != n_vx) {
      mlog << Error << "\ncheck_climo_n_vx() -> "
           << "The number of climatology standard deviation fields in \""
           << conf_key_climo_stdev_field
           << "\" must be zero or match the number ("
           << n_vx << ") in \"" << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

InterpMthd int_to_interpmthd(int i) {
   InterpMthd m = InterpMthd::None;

        if(i == conf_const.lookup_int(interpmthd_none_str))        m = InterpMthd::None;
   else if(i == conf_const.lookup_int(interpmthd_min_str))         m = InterpMthd::Min;
   else if(i == conf_const.lookup_int(interpmthd_max_str))         m = InterpMthd::Max;
   else if(i == conf_const.lookup_int(interpmthd_median_str))      m = InterpMthd::Median;
   else if(i == conf_const.lookup_int(interpmthd_uw_mean_str))     m = InterpMthd::UW_Mean;
   else if(i == conf_const.lookup_int(interpmthd_dw_mean_str))     m = InterpMthd::DW_Mean;
   else if(i == conf_const.lookup_int(interpmthd_aw_mean_str))     m = InterpMthd::AW_Mean;
   else if(i == conf_const.lookup_int(interpmthd_ls_fit_str))      m = InterpMthd::LS_Fit;
   else if(i == conf_const.lookup_int(interpmthd_bilin_str))       m = InterpMthd::Bilin;
   else if(i == conf_const.lookup_int(interpmthd_nbrhd_str))       m = InterpMthd::Nbrhd;
   else if(i == conf_const.lookup_int(interpmthd_nearest_str))     m = InterpMthd::Nearest;
   else if(i == conf_const.lookup_int(interpmthd_budget_str))      m = InterpMthd::Budget;
   else if(i == conf_const.lookup_int(interpmthd_force_str))       m = InterpMthd::Force;
   else if(i == conf_const.lookup_int(interpmthd_best_str))        m = InterpMthd::Best;
   else if(i == conf_const.lookup_int(interpmthd_upper_left_str))  m = InterpMthd::Upper_Left;
   else if(i == conf_const.lookup_int(interpmthd_upper_right_str)) m = InterpMthd::Upper_Right;
   else if(i == conf_const.lookup_int(interpmthd_lower_right_str)) m = InterpMthd::Lower_Right;
   else if(i == conf_const.lookup_int(interpmthd_lower_left_str))  m = InterpMthd::Lower_Left;
   else if(i == conf_const.lookup_int(interpmthd_gaussian_str))    m = InterpMthd::Gaussian;
   else if(i == conf_const.lookup_int(interpmthd_maxgauss_str))    m = InterpMthd::MaxGauss;
   else if(i == conf_const.lookup_int(interpmthd_geog_match_str))  m = InterpMthd::Geog_Match;
   else if(i == conf_const.lookup_int(interpmthd_hira_str))        m = InterpMthd::HiRA;
   else {
      mlog << Error << "\nconf_int_to_interpmthd() -> "
           << "Unexpected value of " << i
           << " for \"" << conf_key_method << "\".\n\n";
      exit(1);
   }

   return m;
}

///////////////////////////////////////////////////////////////////////////////

void check_mctc_thresh(const ThreshArray &ta) {

   // Check that the threshold values are monotonically increasing
   // and the threshold types are inequalities that remain the same
   for(int i=0; i<ta.n()-1; i++) {

      if(ta[i].get_value() >  ta[i+1].get_value() ||
         ta[i].get_type()  != ta[i+1].get_type()  ||
        (ta[i].get_type()  != thresh_lt           &&
         ta[i].get_type()  != thresh_le           &&
         ta[i].get_type()  != thresh_gt           &&
         ta[i].get_type()  != thresh_ge)) {
         mlog << Error << "\ncheck_mctc_thresh() -> "
              << "when verifying using multi-category contingency "
              << "tables, the thresholds must be monotonically "
              << "increasing and be of the same inequality type "
              << "(lt, le, gt, or ge).\n\n";
         exit(1);
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

const char * statlinetype_to_string(const STATLineType t) {
   const char *s = (const char *) nullptr;

   switch(t) {
      case stat_sl1l2:        s = stat_sl1l2_str;     break;
      case stat_sal1l2:       s = stat_sal1l2_str;    break;
      case stat_vl1l2:        s = stat_vl1l2_str;     break;
      case stat_val1l2:       s = stat_val1l2_str;    break;
      case stat_vcnt:         s = stat_vcnt_str;      break;

      case stat_fho:          s = stat_fho_str;       break;
      case stat_ctc:          s = stat_ctc_str;       break;
      case stat_cts:          s = stat_cts_str;       break;
      case stat_mctc:         s = stat_mctc_str;      break;
      case stat_mcts:         s = stat_mcts_str;      break;

      case stat_cnt:          s = stat_cnt_str;       break;
      case stat_pct:          s = stat_pct_str;       break;
      case stat_pstd:         s = stat_pstd_str;      break;
      case stat_pjc:          s = stat_pjc_str;       break;
      case stat_prc:          s = stat_prc_str;       break;

      case stat_eclv:         s = stat_eclv_str;      break;
      case stat_mpr:          s = stat_mpr_str;       break;
      case stat_seeps:        s = stat_seeps_str;     break;
      case stat_seeps_mpr:    s = stat_seeps_mpr_str; break;
      case stat_nbrctc:       s = stat_nbrctc_str;    break;

      case stat_nbrcts:       s = stat_nbrcts_str;    break;
      case stat_nbrcnt:       s = stat_nbrcnt_str;    break;
      case stat_grad:         s = stat_grad_str;      break;
      case stat_dmap:         s = stat_dmap_str;      break;
      case stat_isc:          s = stat_isc_str;       break;

      case stat_wdir:         s = stat_wdir_str;      break;
      case stat_ecnt:         s = stat_ecnt_str;      break;
      case stat_rps:          s = stat_rps_str;       break;
      case stat_rhist:        s = stat_rhist_str;     break;
      case stat_phist:        s = stat_phist_str;     break;

      case stat_orank:        s = stat_orank_str;     break;
      case stat_ssvar:        s = stat_ssvar_str;     break;
      case stat_relp:         s = stat_relp_str;      break;
      case stat_genmpr:       s = stat_genmpr_str;    break;
      case stat_ssidx:        s = stat_ssidx_str;     break;
 
      case stat_header:       s = stat_header_str;    break;

      case no_stat_line_type:
      default:                 s = stat_na_str;        break;
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

void statlinetype_to_string(const STATLineType t, char *out) {
   const char *method_name = "statlinetype_to_string() -> ";

   m_strcpy(out, statlinetype_to_string(t), method_name);

   return;
}

///////////////////////////////////////////////////////////////////////////////

STATLineType string_to_statlinetype(const char *s) {
   STATLineType t;

        if(strcasecmp(s, stat_sl1l2_str)     == 0) t = stat_sl1l2;
   else if(strcasecmp(s, stat_sal1l2_str)    == 0) t = stat_sal1l2;
   else if(strcasecmp(s, stat_vl1l2_str)     == 0) t = stat_vl1l2;
   else if(strcasecmp(s, stat_val1l2_str)    == 0) t = stat_val1l2;
   else if(strcasecmp(s, stat_vcnt_str)      == 0) t = stat_vcnt;

   else if(strcasecmp(s, stat_fho_str)       == 0) t = stat_fho;
   else if(strcasecmp(s, stat_ctc_str)       == 0) t = stat_ctc;
   else if(strcasecmp(s, stat_cts_str)       == 0) t = stat_cts;
   else if(strcasecmp(s, stat_mctc_str)      == 0) t = stat_mctc;
   else if(strcasecmp(s, stat_mcts_str)      == 0) t = stat_mcts;

   else if(strcasecmp(s, stat_cnt_str)       == 0) t = stat_cnt;
   else if(strcasecmp(s, stat_pct_str)       == 0) t = stat_pct;
   else if(strcasecmp(s, stat_pstd_str)      == 0) t = stat_pstd;
   else if(strcasecmp(s, stat_pjc_str)       == 0) t = stat_pjc;
   else if(strcasecmp(s, stat_prc_str)       == 0) t = stat_prc;

   else if(strcasecmp(s, stat_eclv_str)      == 0) t = stat_eclv;
   else if(strcasecmp(s, stat_mpr_str)       == 0) t = stat_mpr;
   else if(strcasecmp(s, stat_seeps_str)     == 0) t = stat_seeps;
   else if(strcasecmp(s, stat_seeps_mpr_str) == 0) t = stat_seeps_mpr;
   else if(strcasecmp(s, stat_nbrctc_str)    == 0) t = stat_nbrctc;

   else if(strcasecmp(s, stat_nbrcts_str)    == 0) t = stat_nbrcts;
   else if(strcasecmp(s, stat_nbrcnt_str)    == 0) t = stat_nbrcnt;
   else if(strcasecmp(s, stat_grad_str)      == 0) t = stat_grad;
   else if(strcasecmp(s, stat_dmap_str)      == 0) t = stat_dmap;
   else if(strcasecmp(s, stat_isc_str)       == 0) t = stat_isc;

   else if(strcasecmp(s, stat_wdir_str)      == 0) t = stat_wdir;
   else if(strcasecmp(s, stat_ecnt_str)      == 0) t = stat_ecnt;
   else if(strcasecmp(s, stat_rps_str)       == 0) t = stat_rps;
   else if(strcasecmp(s, stat_rhist_str)     == 0) t = stat_rhist;
   else if(strcasecmp(s, stat_phist_str)     == 0) t = stat_phist;

   else if(strcasecmp(s, stat_orank_str)     == 0) t = stat_orank;
   else if(strcasecmp(s, stat_ssvar_str)     == 0) t = stat_ssvar;
   else if(strcasecmp(s, stat_relp_str)      == 0) t = stat_relp;
   else if(strcasecmp(s, stat_genmpr_str)    == 0) t = stat_genmpr;
   else if(strcasecmp(s, stat_ssidx_str)     == 0) t = stat_ssidx;

   else if(strcasecmp(s, stat_header_str)    == 0) t = stat_header;

   else                                            t = no_stat_line_type;

   return t;
}

///////////////////////////////////////////////////////////////////////////////

const char * bootintervaltype_to_string(const BootIntervalType t) {
   auto s = (const char *) nullptr;

   switch(t) {
      case BootIntervalType::BCA:    s = conf_val_bca;     break;
      case BootIntervalType::PCTile: s = conf_val_pctile;  break;
      default:                       s = conf_val_none;    break;
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

BootIntervalType string_to_bootintervaltype(const char *s) {
   BootIntervalType t;

        if(strcasecmp(s, conf_val_bca)    == 0) t = BootIntervalType::BCA;
   else if(strcasecmp(s, conf_val_pctile) == 0) t = BootIntervalType::PCTile;
   else                                         t = BootIntervalType::None;

   return t;
}

///////////////////////////////////////////////////////////////////////////////

FieldType int_to_fieldtype(int v) {
   FieldType t = FieldType::None;

   // Convert integer to enumerated FieldType
        if(v == conf_const.lookup_int(conf_val_none)) t = FieldType::None;
   else if(v == conf_const.lookup_int(conf_val_both)) t = FieldType::Both;
   else if(v == conf_const.lookup_int(conf_val_fcst)) t = FieldType::Fcst;
   else if(v == conf_const.lookup_int(conf_val_obs))  t = FieldType::Obs;
   else {
      mlog << Error << "\nint_to_fieldtype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

GridTemplateFactory::GridTemplates int_to_gridtemplate(int v) {
   GridTemplateFactory::GridTemplates t = GridTemplateFactory::GridTemplates::Square;

   // Convert integer to enumerated FieldType
   if(v == conf_const.lookup_int(conf_val_square)) {
      t = GridTemplateFactory::GridTemplates::Square;
   }
   else if(v == conf_const.lookup_int(conf_val_circle)) {
      t = GridTemplateFactory::GridTemplates::Circle;
   }
   else {
      mlog << Error << "\nint_to_gridtemplate() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString fieldtype_to_string(FieldType type) {
   ConcatString s;

   // Convert enumerated FieldType to string
   switch(type) {
      case FieldType::None: s = conf_val_none; break;
      case FieldType::Both: s = conf_val_both; break;
      case FieldType::Fcst: s = conf_val_fcst; break;
      case FieldType::Obs:  s = conf_val_obs; break;
      default:
         mlog << Error << "\nfieldtype_to_string() -> "
              << "Unexpected FieldType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

SetLogic int_to_setlogic(int v) {
   SetLogic t = SetLogic::None;

   // Convert integer to enumerated SetLogic
        if(v == conf_const.lookup_int(conf_val_none))         t = SetLogic::None;
   else if(v == conf_const.lookup_int(conf_val_union))        t = SetLogic::Union;
   else if(v == conf_const.lookup_int(conf_val_intersection)) t = SetLogic::Intersection;
   else if(v == conf_const.lookup_int(conf_val_symdiff))      t = SetLogic::SymDiff;
   else {
      mlog << Error << "\nint_to_setlogic() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

SetLogic string_to_setlogic(const char *s) {
   SetLogic t = SetLogic::None;

   // Convert string to enumerated SetLogic
        if(strcasecmp(s, conf_val_none)                == 0) t = SetLogic::None;

   else if(strcasecmp(s, conf_val_union)               == 0 ||
           strcasecmp(s, setlogic_abbr_union)          == 0 ||
           strcasecmp(s, setlogic_symbol_union)        == 0) t = SetLogic::Union;

   else if(strcasecmp(s, conf_val_intersection)        == 0 ||
           strcasecmp(s, setlogic_abbr_intersection)   == 0 ||
           strcasecmp(s, setlogic_symbol_intersection) == 0) t = SetLogic::Intersection;

   else if(strcasecmp(s, conf_val_symdiff)             == 0 ||
           strcasecmp(s, setlogic_abbr_symdiff)        == 0 ||
           strcasecmp(s, setlogic_symbol_symdiff)      == 0) t = SetLogic::SymDiff;

   else {
      mlog << Error << "\nstring_to_setlogic() -> "
           << "Unexpected SetLogic string \"" << s << "\".\n\n";
      exit(1);
   }

   return t;
}


///////////////////////////////////////////////////////////////////////////////

ConcatString setlogic_to_string(SetLogic type) {
   ConcatString s;

   // Convert enumerated SetLogic to string
   switch(type) {
      case SetLogic::None:         s = conf_val_none;         break;
      case SetLogic::Union:        s = conf_val_union;        break;
      case SetLogic::Intersection: s = conf_val_intersection; break;
      case SetLogic::SymDiff:      s = conf_val_symdiff;      break;
      default:
         mlog << Error << "\nsetlogic_to_string() -> "
              << "Unexpected SetLogic value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString setlogic_to_abbr(SetLogic type) {
   ConcatString s;

   // Convert enumerated SetLogic to an abbreviation
   switch(type) {
      case SetLogic::None:         s = na_str;                     break;
      case SetLogic::Union:        s = setlogic_abbr_union;        break;
      case SetLogic::Intersection: s = setlogic_abbr_intersection; break;
      case SetLogic::SymDiff:      s = setlogic_abbr_symdiff;      break;
      default:
         mlog << Error << "\nsetlogic_to_abbr() -> "
              << "Unexpected SetLogic value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString setlogic_to_symbol(SetLogic type) {
   ConcatString s;

   // Convert enumerated SetLogic to a symbol
   switch(type) {
      case SetLogic::None:         s = na_str;                       break;
      case SetLogic::Union:        s = setlogic_symbol_union;        break;
      case SetLogic::Intersection: s = setlogic_symbol_intersection; break;
      case SetLogic::SymDiff:      s = setlogic_symbol_symdiff;      break;
      default:
         mlog << Error << "\nsetlogic_to_symbol() -> "
              << "Unexpected SetLogic value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

SetLogic check_setlogic(SetLogic t1, SetLogic t2) {
   SetLogic t = SetLogic::None;

   // If not equal, select the non-default logic type
        if(t1 == t2)            t = t1;
   else if(t1 == SetLogic::None) t = t2;
   else if(t2 == SetLogic::None) t = t1;
   // If not equal and both non-default, error out
   else {
      mlog << Error << "\ncheck_setlogic() -> "
           << "The forecast and observed logic must be consistent: "
           << setlogic_to_string(t1) << " != " << setlogic_to_string(t2)
           << "\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

TrackType int_to_tracktype(int v) {
   TrackType t = TrackType::None;

   // Convert integer to enumerated TrackType
        if(v == conf_const.lookup_int(conf_val_none))  t = TrackType::None;
   else if(v == conf_const.lookup_int(conf_val_both))  t = TrackType::Both;
   else if(v == conf_const.lookup_int(conf_val_adeck)) t = TrackType::ADeck;
   else if(v == conf_const.lookup_int(conf_val_bdeck)) t = TrackType::BDeck;
   else {
      mlog << Error << "\nint_to_tracktype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

TrackType string_to_tracktype(const char *s) {
   TrackType t = TrackType::None;

   // Convert string to enumerated TrackType
        if(strcasecmp(s, conf_val_none)  == 0) t = TrackType::None;
   else if(strcasecmp(s, conf_val_both)  == 0) t = TrackType::Both;
   else if(strcasecmp(s, conf_val_adeck) == 0) t = TrackType::ADeck;
   else if(strcasecmp(s, conf_val_bdeck) == 0) t = TrackType::BDeck;
   else {
      mlog << Error << "\nstring_to_tracktype() -> "
           << "Unexpected TrackType string \"" << s << "\".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString tracktype_to_string(TrackType type) {
   ConcatString s;

   // Convert enumerated TrackType to string
   switch(type) {
      case TrackType::None:  s = conf_val_none; break;
      case TrackType::Both:  s = conf_val_both; break;
      case TrackType::ADeck: s = conf_val_adeck; break;
      case TrackType::BDeck: s = conf_val_bdeck; break;
      default:
         mlog << Error << "\ntracktype_to_string() -> "
              << "Unexpected TrackType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

DiagType string_to_diagtype(const char *s) {
   DiagType t = DiagType::None;

   // Convert string to enumerated DiagType, storing unknown strings
   // as the default type
        if(strcasecmp(s, cira_diag_rt_str)   == 0) t = DiagType::CIRA_RT;
   else if(strcasecmp(s, cira_diag_dev_str)  == 0) t = DiagType::CIRA_Dev;
   else if(strcasecmp(s, ships_diag_rt_str)  == 0) t = DiagType::SHIPS_RT;
   else if(strcasecmp(s, ships_diag_dev_str) == 0) t = DiagType::SHIPS_Dev;
   else                                            t = DiagType::None;

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString diagtype_to_string(DiagType type) {
   ConcatString s;

   // Convert enumerated DiagType to string
   switch(type) {
      case DiagType::None:      s = conf_val_none;      break;
      case DiagType::CIRA_RT:   s = cira_diag_rt_str;   break;
      case DiagType::CIRA_Dev:  s = cira_diag_dev_str;  break;
      case DiagType::SHIPS_RT:  s = ships_diag_rt_str;  break;
      case DiagType::SHIPS_Dev: s = ships_diag_dev_str; break;
      default:
         mlog << Error << "\ndiagtype_to_string() -> "
              << "Unexpected DiagType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

Interp12Type int_to_interp12type(int v) {
   Interp12Type t = Interp12Type::None;

   // Convert integer to enumerated Interp12Type
        if(v == conf_const.lookup_int(conf_val_none))    t = Interp12Type::None;
   else if(v == conf_const.lookup_int(conf_val_fill))    t = Interp12Type::Fill;
   else if(v == conf_const.lookup_int(conf_val_replace)) t = Interp12Type::Replace;
   else {
      mlog << Error << "\nint_to_interp12type() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

Interp12Type string_to_interp12type(const char *s) {
   Interp12Type t = Interp12Type::None;

   // Convert string to enumerated Interp12Type
        if(strcasecmp(s, conf_val_none)    == 0) t = Interp12Type::None;
   else if(strcasecmp(s, conf_val_fill)    == 0) t = Interp12Type::Fill;
   else if(strcasecmp(s, conf_val_replace) == 0) t = Interp12Type::Replace;
   else {
      mlog << Error << "\nstring_to_interp12type() -> "
           << "Unexpected Interp12Type string \"" << s << "\".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString interp12type_to_string(Interp12Type type) {
   ConcatString s;

   // Convert enumerated Interp12Type to string
   switch(type) {
      case Interp12Type::None:    s = conf_val_none;    break;
      case Interp12Type::Fill:    s = conf_val_fill;    break;
      case Interp12Type::Replace: s = conf_val_replace; break;
      default:
         mlog << Error << "\ninterp12type_to_string() -> "
              << "Unexpected Interp12Type value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

MergeType int_to_mergetype(int v) {
   MergeType t = MergeType::None;

   // Convert integer to enumerated MergeType
        if(v == conf_const.lookup_int(conf_val_none))   t = MergeType::None;
   else if(v == conf_const.lookup_int(conf_val_both))   t = MergeType::Both;
   else if(v == conf_const.lookup_int(conf_val_thresh)) t = MergeType::Thresh;
   else if(v == conf_const.lookup_int(conf_val_engine)) t = MergeType::Engine;
   else {
      mlog << Error << "\nint_to_mergetype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString mergetype_to_string(MergeType type) {
   ConcatString s;

   // Convert enumerated MergeType to string
   switch(type) {
      case MergeType::None:   s = conf_val_none;   break;
      case MergeType::Both:   s = conf_val_both;   break;
      case MergeType::Thresh: s = conf_val_thresh; break;
      case MergeType::Engine: s = conf_val_engine; break;
      default:
         mlog << Error << "\nmergetype_to_string() -> "
              << "Unexpected MergeType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString obssummary_to_string(ObsSummary type, int perc_val) {
   ConcatString s;

   // Convert enumerated ObsSummary to string
   switch(type) {
      case ObsSummary::None:    s = conf_val_none;       break;
      case ObsSummary::Nearest: s = conf_val_nearest;    break;
      case ObsSummary::Min:     s = conf_val_min;        break;
      case ObsSummary::Max:     s = conf_val_max;        break;
      case ObsSummary::UW_Mean: s = conf_val_uw_mean;    break;
      case ObsSummary::DW_Mean: s = conf_val_dw_mean;    break;
      case ObsSummary::Median:  s = conf_val_median;     break;
      case ObsSummary::Perc:
         s << conf_val_perc << "(" << perc_val << ")";
         break;
      default:
         mlog << Error << "\nobssummary_to_string() -> "
              << "Unexpected ObsSummary value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

MatchType int_to_matchtype(int v) {
   MatchType t = MatchType::None;

   // Convert integer to enumerated MatchType
        if(v == conf_const.lookup_int(conf_val_none))       t = MatchType::None;
   else if(v == conf_const.lookup_int(conf_val_merge_both)) t = MatchType::MergeBoth;
   else if(v == conf_const.lookup_int(conf_val_merge_fcst)) t = MatchType::MergeFcst;
   else if(v == conf_const.lookup_int(conf_val_no_merge))   t = MatchType::NoMerge;
   else {
      mlog << Error << "\nint_to_matchtype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString matchtype_to_string(MatchType type) {
   ConcatString s;

   // Convert enumerated MatchType to string
   switch(type) {
      case MatchType::None:      s = conf_val_none;       break;
      case MatchType::MergeBoth: s = conf_val_merge_both; break;
      case MatchType::MergeFcst: s = conf_val_merge_fcst; break;
      case MatchType::NoMerge:   s = conf_val_no_merge;   break;
      default:
         mlog << Error << "\nmatchtype_to_string() -> "
              << "Unexpected MatchType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

DistType int_to_disttype(int v) {
   DistType t = DistType::None;

   // Convert integer to enumerated DistType
        if(v == conf_const.lookup_int(conf_val_none))        t = DistType::None;
   else if(v == conf_const.lookup_int(conf_val_normal))      t = DistType::Normal;
   else if(v == conf_const.lookup_int(conf_val_exponential)) t = DistType::Exponential;
   else if(v == conf_const.lookup_int(conf_val_chisquared))  t = DistType::ChiSquared;
   else if(v == conf_const.lookup_int(conf_val_gamma))       t = DistType::Gamma;
   else if(v == conf_const.lookup_int(conf_val_uniform))     t = DistType::Uniform;
   else if(v == conf_const.lookup_int(conf_val_beta))        t = DistType::Beta;
   else {
      mlog << Error << "\nint_to_disttype() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

DistType string_to_disttype(const char *s) {
   DistType t = DistType::None;

   // Convert string to enumerated DistType
        if(strcasecmp(s, conf_val_none)        == 0) t = DistType::None;
   else if(strcasecmp(s, conf_val_normal)      == 0) t = DistType::Normal;
   else if(strcasecmp(s, conf_val_exponential) == 0) t = DistType::Exponential;
   else if(strcasecmp(s, conf_val_chisquared)  == 0) t = DistType::ChiSquared;
   else if(strcasecmp(s, conf_val_gamma)       == 0) t = DistType::Gamma;
   else if(strcasecmp(s, conf_val_uniform)     == 0) t = DistType::Uniform;
   else if(strcasecmp(s, conf_val_beta)        == 0) t = DistType::Beta;
   else {
      mlog << Error << "\nstring_to_disttype() -> "
           << "Unexpected DistType string \"" << s << "\".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString disttype_to_string(DistType type) {
   ConcatString s;

   // Convert enumerated DistType to string
   switch(type) {
      case DistType::None:        s = conf_val_none;        break;
      case DistType::Normal:      s = conf_val_normal;      break;
      case DistType::Exponential: s = conf_val_exponential; break;
      case DistType::ChiSquared:  s = conf_val_chisquared;  break;
      case DistType::Gamma:       s = conf_val_gamma;       break;
      case DistType::Uniform:     s = conf_val_uniform;     break;
      case DistType::Beta:        s = conf_val_beta;        break;
      default:
         mlog << Error << "\ndisttype_to_string() -> "
              << "Unexpected DistType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString dist_to_string(DistType type, const NumArray &parm) {
   ConcatString s;

   s = disttype_to_string(type);

   // Append distribution parameters
   if(type != DistType::None && parm.n() == 2) {
      s << "(" << parm[0];
      if(type == DistType::Gamma   ||
         type == DistType::Uniform ||
         type == DistType::Beta) {
         s << ", " << parm[1];
      }
      s << ")";
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString griddecomptype_to_string(GridDecompType type) {
   ConcatString s;

   // Convert enumerated GridDecompType to string
   switch(type) {
      case GridDecompType::None: s = conf_val_none; break;
      case GridDecompType::Auto: s = conf_val_auto; break;
      case GridDecompType::Tile: s = conf_val_tile; break;
      case GridDecompType::Pad:  s = conf_val_pad; break;
      default:
         mlog << Error << "\ngriddecomptype_to_string() -> "
              << "Unexpected GridDecompType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString wavelettype_to_string(WaveletType type) {
   ConcatString s;

   // Convert enumerated WaveletType to string
   switch(type) {
      case WaveletType::None:          s = conf_val_none; break;
      case WaveletType::Haar:          s = conf_val_haar; break;
      case WaveletType::Haar_Cntr:     s = conf_val_haar_cntr; break;
      case WaveletType::Daub:          s = conf_val_daub; break;
      case WaveletType::Daub_Cntr:     s = conf_val_daub_cntr; break;
      case WaveletType::BSpline:       s = conf_val_bspline; break;
      case WaveletType::BSpline_Cntr:  s = conf_val_bspline_cntr; break;
      default:
         mlog << Error << "\nwavlettype_to_string() -> "
              << "Unexpected WaveletType value of " << enum_class_as_integer(type) << ".\n\n";
         exit(1);
   }

   return s;
}

///////////////////////////////////////////////////////////////////////////////

StringArray parse_conf_ens_member_ids(Dictionary *dict) {
   const char *method_name = "parse_conf_ens_member_ids() -> ";

   StringArray sa = parse_conf_string_array(dict, conf_key_ens_member_ids, method_name);

   if(sa.n() > 0) {
      mlog << Debug(4) << method_name
           << "Ensemble Member IDs \"" << conf_key_ens_member_ids << "\" list contains "
           << sa.n() << " entries.\n";
   }

   return sa;
}

///////////////////////////////////////////////////////////////////////////////

NormalizeType parse_conf_normalize(Dictionary *dict) {
   NormalizeType t = NormalizeType::None;
   int v;

   if(!dict) {
      mlog << Error << "\nparse_conf_normalize() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Get the integer flag value for the current entry
   v = dict->lookup_int(conf_key_normalize);

   // Convert integer to enumerated NormalizeType
        if(v == conf_const.lookup_int(normalizetype_none_str))           t = NormalizeType::None;
   else if(v == conf_const.lookup_int(normalizetype_climo_anom_str))     t = NormalizeType::ClimoAnom;
   else if(v == conf_const.lookup_int(normalizetype_climo_std_anom_str)) t = NormalizeType::ClimoStdAnom;
   else if(v == conf_const.lookup_int(normalizetype_fcst_anom_str))      t = NormalizeType::FcstAnom;
   else if(v == conf_const.lookup_int(normalizetype_fcst_std_anom_str))  t = NormalizeType::FcstStdAnom;
   else {
      mlog << Error << "\nparse_conf_normalize() -> "
           << "Unexpected value of " << v << ".\n\n";
      exit(1);
   }

   return t;
}

///////////////////////////////////////////////////////////////////////////////
//
// Print consistent error message and exit
//
///////////////////////////////////////////////////////////////////////////////

void python_compile_error(const char *caller) {

   const char *method_name = (nullptr != caller) ? caller : "python_compile_error() -> ";

   mlog << Error << "\n" << method_name
        << "Support for Python has not been compiled!\n"
        << "To run Python scripts, recompile with the --enable-python option.\n\n";

   exit(1);
}

///////////////////////////////////////////////////////////////////////////////

void ugrid_compile_error(const char *caller) {

   const char *method_name = (nullptr != caller) ? caller : "ugrid_compile_error() -> ";

   mlog << Error << "\n" << method_name
        << "Support for the unstructured grid has not been compiled!\n"
        << "To use the filetype NETCDF_UGRID (unstructured grid), recompile with the --enable-ugrid option.\n\n";

   exit(1);
}

///////////////////////////////////////////////////////////////////////////////
