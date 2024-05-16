// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __NC_UTILS_HPP__
#define  __NC_UTILS_HPP__

////////////////////////////////////////////////////////////////////////

#include "nc_utils_core.h"

////////////////////////////////////////////////////////////////////////

extern bool get_att_value(const netCDF::NcAtt *att, int &att_val);
extern bool get_att_value(const netCDF::NcAtt *att, ConcatString &value);
extern bool get_att_value(const netCDF::NcAtt *att, ncbyte &att_val);
extern bool get_att_value(const netCDF::NcAtt *att, short &att_val);
extern bool get_att_value(const netCDF::NcAtt *att, int &att_val);
extern bool get_att_value(const netCDF::NcAtt *att, unsigned int &att_val);
extern bool get_att_value(const netCDF::NcAtt *att, float &att_val);
extern bool get_att_value(const netCDF::NcAtt *att, double &att_val);
extern ConcatString get_log_msg_for_att(const netCDF::NcVarAtt *att);
extern ConcatString get_log_msg_for_att(const netCDF::NcVarAtt *att, std::string var_name,
                                        const ConcatString att_name);
extern double get_var_add_offset(const netCDF::NcVar *var);
extern double get_var_scale_factor(const netCDF::NcVar *var);
extern bool has_add_offset_attr(netCDF::NcVar *var);
extern bool has_scale_factor_attr(netCDF::NcVar *var);
extern void set_def_fill_value(ncbyte *val);
extern void set_def_fill_value(char   *val);
extern void set_def_fill_value(double *val);
extern void set_def_fill_value(float  *val);
extern void set_def_fill_value(int    *val);
extern void set_def_fill_value(long   *val);
extern void set_def_fill_value(short  *val);
extern void set_def_fill_value(long long          *val);
extern void set_def_fill_value(unsigned char      *val);
extern void set_def_fill_value(unsigned int       *val);
extern void set_def_fill_value(unsigned long      *val);
extern void set_def_fill_value(unsigned long long *val);
extern void set_def_fill_value(unsigned short     *val);

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_att_num_value_(const netCDF::NcAtt *att, T &att_val, int matching_type) {
   bool status = IS_VALID_NC_P(att);
   if (status) {
      int nc_type_id = GET_NC_TYPE_ID_P(att);
      if (matching_type == nc_type_id) {
         att->getValues(&att_val);
      }
      else if (NC_FLOAT == nc_type_id) {
         float att_value;
         att->getValues(&att_value);
         att_val = att_value;
      }
      else if (NC_DOUBLE == nc_type_id) {
         double att_value;
         att->getValues(&att_value);
         att_val = att_value;
      }
      else if (NC_CHAR == nc_type_id) {
         std::string att_value;
         att->getValues(att_value);
         if (matching_type == NC_FLOAT)
            att_val = atof(att_value.c_str());
         else if (matching_type == NC_DOUBLE)
            att_val = (double)atof(att_value.c_str());
         else // if (matching_type == NC_INT)
            att_val = atoi(att_value.c_str());
      }
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_nc_att_value_(const netCDF::NcVar *var, const ConcatString &att_name,
                       T &att_val, bool exit_on_error,
                       T bad_data, const char *caller_name) {
   // Initialize
   att_val = bad_data;

   //
   // Retrieve the NetCDF variable attribute.
   //
   netCDF::NcVarAtt *att = get_nc_att(var, att_name);
   bool status = get_att_value((netCDF::NcAtt *)att, att_val);
   if (!status) {
      mlog << Error << "\n" << caller_name
           << get_log_msg_for_att(att, GET_SAFE_NC_NAME_P(var), att_name);
   }
   if (att) delete att;
   if (!status && exit_on_error) exit(1);

   return status;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_nc_att_values_(const netCDF::NcVar *var, const ConcatString &att_name,
                        T *att_vals, bool exit_on_error,
                        const char *caller_name) {
   // caller should initialize att_vals

   //
   // Retrieve the NetCDF variable attribute.
   //
   netCDF::NcVarAtt *att = get_nc_att(var, att_name);
   bool status = IS_VALID_NC_P(att);
   if (status) att->getValues(att_vals);
   else {
      mlog << Error << "\n" << caller_name
           << get_log_msg_for_att(att, GET_SAFE_NC_NAME_P(var), att_name);
   }
   if (att) delete att;
   if (!status && exit_on_error) exit(1);

   return status;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_nc_att_value_(const netCDF::NcVarAtt *att, T &att_val, bool exit_on_error,
                       T bad_data, const char *caller_name) {

   // Initialize
   att_val = bad_data;

   //
   // Retrieve the NetCDF variable attribute.
   //
   bool status = get_att_value((netCDF::NcAtt *)att, att_val);
   if (!status) {
      mlog << Error << "\n" << caller_name << get_log_msg_for_att(att);

      if (exit_on_error) exit(1);
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_global_att_value_(const netCDF::NcFile *nc, const ConcatString& att_name,
                           T &att_val, T bad_data, bool error_out, const char *caller_name) {
   bool status = false;
   // Initialize
   att_val = bad_data;

   netCDF::NcGroupAtt *nc_att = get_nc_att(nc, att_name);
   if (IS_VALID_NC_P(nc_att)) {
      status = get_att_value((netCDF::NcAtt *)nc_att, att_val);
      std::string data_type = GET_NC_TYPE_NAME_P(nc_att);
      if (error_out && !status) {
         mlog << Error << caller_name
              << "The data type \"" << data_type
              << "\" for \"" << att_name << "\" does not match...\n\n";
      }
   }
   else if (error_out) {
      mlog << Error << caller_name
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
   }
   if (nc_att) delete nc_att;
   // Check error_out status
   if (error_out && !status) exit(1);

   return status;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_var_att_num_(const netCDF::NcVar *var, const ConcatString &att_name,
                      T &att_val, T bad_data) {
   bool status = false;

   // Initialize
   att_val = bad_data;

   netCDF::NcVarAtt *att = get_nc_att(var, att_name);
   // Look for a match
   if (IS_VALID_NC_P(att)) {
      att->getValues(&att_val);
      status = true;
   }
   if (att) delete att;

   return status;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_var_fill_value(const netCDF::NcVar *var, T &att_val) {
   bool found = false;

   netCDF::NcVarAtt *att = get_nc_att(var, fill_value_att_name);
   if (IS_INVALID_NC_P(att)) {
      if (att) delete att;
      att = get_nc_att(var, missing_value_att_name);
   }
   if (IS_VALID_NC_P(att)) {
      att->getValues(&att_val);
      found = true;
   }
   else set_def_fill_value(&att_val);

   if (att) delete att;

   return(found);
}

////////////////////////////////////////////////////////////////////////

template <typename T>
void apply_scale_factor_(T *data, const int cell_count,
                         double add_offset, double scale_factor,
                         const T nc_fill_value, const T met_fill_value,
                         bool has_fill_value,
                         const char *data_type, const char *var_name) {
   const int debug_level = 7;
   clock_t start_clock = clock();
   const char *method_name = "apply_scale_factor(T) ";

   if (cell_count > 0) {
      int idx;
      int positive_cnt = 0;
      int unpacked_count = 0;
      T min_value, max_value;
      T raw_min_val, raw_max_val;

      idx = 0;
      if (has_fill_value) {
         for (; idx<cell_count; idx++) {
            if (!is_eq(nc_fill_value, data[idx])) break;
            data[idx] = met_fill_value;
         }
      }

      raw_min_val = raw_max_val = data[idx];
      min_value = max_value = (data[idx] * scale_factor) + add_offset;
      for (; idx<cell_count; idx++) {
         if (has_fill_value && is_eq(nc_fill_value, data[idx]))
            data[idx] = met_fill_value;
         else {
            if (raw_min_val > data[idx]) raw_min_val = data[idx];
            if (raw_max_val < data[idx]) raw_max_val = data[idx];
            data[idx] = (data[idx] * scale_factor) + add_offset;
            if (data[idx] > 0) positive_cnt++;
            if (min_value > data[idx]) min_value = data[idx];
            if (max_value < data[idx]) max_value = data[idx];
            unpacked_count++;
         }
      }
      //cout << typeid(nc_fill_value).name();
      mlog << Debug(debug_level) << method_name << var_name
           << "(" << typeid(data[0]).name() << "): unpacked data: count="
           << unpacked_count << " out of " << cell_count
           << ", scale_factor=" << scale_factor<< " add_offset=" << add_offset
           << ". FillValue(" << data_type << ")=" << nc_fill_value << "\n";
      mlog << Debug(debug_level) << method_name
           << " data range [" << min_value << " - " << max_value
           << "] raw data: [" << raw_min_val << " - " << raw_max_val
           << "] Positive count: " << positive_cnt << "\n";
   }
   mlog << Debug(debug_level) << method_name << " took "
        << (clock()-start_clock)/double(CLOCKS_PER_SEC) << " seconds\n";
   return;
}

////////////////////////////////////////////////////////////////////////
// Note:
// - template <function_name>_t reads data as is (do not apply no scale_factor and add_offset)
// - template <function_name>_ reads data and applies scale_factor and add_offset.

template <typename T>
bool get_nc_data_t(netCDF::NcVar *var, T *data) {
   bool return_status = IS_VALID_NC_P(var);

   if (return_status) {
      var->getVar(data);
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_nc_data_(netCDF::NcVar *var, T *data, const T met_missing) {
   //const char *method_name = "get_nc_data_() ";

   int data_size = get_data_size(var);
   for (int idx1=0; idx1<data_size; idx1++) {
      data[idx1] = met_missing;
   }

   bool return_status = get_nc_data_t(var, data);

   if (return_status) {
      //scale_factor and add_offset
      if (has_add_offset_attr(var) || has_scale_factor_attr(var)) {
         T nc_missing;
         const int cell_count = get_data_size(var);
         double add_offset = get_var_add_offset(var);
         double scale_factor = get_var_scale_factor(var);
         bool has_missing_attr = get_var_fill_value(var, nc_missing);
         if (!has_missing_attr) nc_missing = met_missing;
         apply_scale_factor_(data, cell_count, add_offset, scale_factor,
                             nc_missing, met_missing, has_missing_attr,
                             "<T>", GET_NC_NAME_P(var).c_str());
      }
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_nc_data_(netCDF::NcVar *var, T *data, T bad_data, const LongArray &dims, const LongArray &curs) {
   bool return_status = false;
   const char *method_name = "get_nc_data_(T, *dims, *curs) ";

   if (IS_VALID_NC_P(var)) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      int idx =0;
      int data_size = 1;
      int dimC = get_dim_count(var);
      int dim_cnt = dims.n_elements();
      int off_cnt = curs.n_elements();
      int in_cnt = (dim_cnt > off_cnt) ? off_cnt : dim_cnt;

      // madis2nc shares the same dims & curs for 1D, 2D and 3D
      if (in_cnt > dimC) in_cnt = dimC;
      for (idx = 0 ; idx < in_cnt; idx++) {
         int dim_size = get_dim_size(var, idx);
         if ((curs[idx]+dims[idx]) > dim_size) {
            netCDF::NcDim nc_dim = get_nc_dim(var, idx);
            mlog << Error << "\n" << method_name << "The start offset and count ("
                 << curs[idx] << ", " << dims[idx] << ") exceeds the dimension["
                 << idx << "] " << dim_size << " "
                 << (IS_VALID_NC(nc_dim) ? GET_NC_NAME(nc_dim) : " ")
                 << " for the variable " << GET_NC_NAME_P(var) << ".\n\n";
            exit(1);
         }

         start.push_back((size_t)curs[idx]);
         count.push_back((size_t)dims[idx]);
         data_size *= dims[idx];
      }
      for (; idx < dimC; idx++) {
         int dim_size = get_dim_size(var, idx);
         start.push_back((size_t)0);
         count.push_back((size_t)dim_size);
         data_size *= dim_size;
      }

      for (int idx1=0; idx1<data_size; idx1++) {
         data[idx1] = bad_data;
      }

      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;

      //scale_factor and add_offset
      if (has_add_offset_attr(var) || has_scale_factor_attr(var)) {
         T nc_missing;
         double add_offset = get_var_add_offset(var);
         double scale_factor = get_var_scale_factor(var);
         bool has_missing_attr = get_var_fill_value(var, nc_missing);
         if (!has_missing_attr) nc_missing = bad_data;
         apply_scale_factor_(data, data_size, add_offset, scale_factor,
                             nc_missing, bad_data, has_missing_attr,
                             "<T>", GET_NC_NAME_P(var).c_str());
      }
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool get_nc_data_(netCDF::NcVar *var, T *data, T met_missing, const long dim, const long cur) {
   bool return_status = false;
   const char *method_name = "get_nc_data_(T, dim, cur) ";
   for (int idx=0; idx<dim; idx++) {
      data[idx] = met_missing;
   }

   if (IS_VALID_NC_P(var)) {
      int dim_idx = 0;
      std::vector<size_t> start;
      std::vector<size_t> count;
      start.push_back((size_t)cur);
      count.push_back((size_t)dim);
      int dim_size = get_dim_size(var, dim_idx);
      if (0 >= dim_size) {
         if ((cur > 0) || (dim > 1)) {
            mlog << Error << "\n" << method_name << "The start offset and count ("
                 << cur << ", " << dim << ") should be (0, 1) because of no dimension at the variable "
                 << GET_NC_NAME_P(var) << ".\n\n";
            exit(1);
         }
      }
      else if ((cur+dim) > dim_size) {
         netCDF::NcDim nc_dim = get_nc_dim(var, dim_idx);
         mlog << Error << "\n" << method_name << "The start offset and count ("
              << cur << " + " << dim << ") exceeds the dimension " << dim_size << " "
              << (IS_VALID_NC(nc_dim) ? GET_NC_NAME(nc_dim) : " ")
              << " for the variable " << GET_NC_NAME_P(var) << ".\n\n";
         exit(1);
      }

      //
      // Retrieve the variable value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;

      //scale_factor and add_offset
      if (has_add_offset_attr(var) || has_scale_factor_attr(var)) {
         T nc_missing;
         double add_offset = get_var_add_offset(var);
         double scale_factor = get_var_scale_factor(var);
         bool has_missing_attr = get_var_fill_value(var, nc_missing);
         if (!has_missing_attr) nc_missing = met_missing;
         apply_scale_factor_(data, dim, add_offset, scale_factor,
                             nc_missing, met_missing, has_missing_attr,
                             "<T>", GET_NC_NAME_P(var).c_str());
      }
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////
// read a single data

template <typename T>
bool get_nc_data_(netCDF::NcVar *var, T *data, T bad_data, const LongArray &curs) {
   bool return_status = false;
   //const char *method_name = "get_nc_data_(*curs) ";

   if (IS_VALID_NC_P(var)) {

      int dimC = get_dim_count(var);

      LongArray dims;
      for (int idx = 0 ; idx < dimC; idx++) {
         dims.add(1);
      }

      // Retrieve the NetCDF value from the NetCDF variable.
      return_status = get_nc_data_(var, data, bad_data, dims, curs);
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

template <typename T>
void copy_nc_data_t(netCDF::NcVar *var, float *data, const T *packed_data,
                    const int cell_count, const char *data_type,
                    double add_offset, double scale_factor,
                    bool has_missing, T missing_value) {
   clock_t start_clock = clock();
   const char *method_name = "copy_nc_data_t(float) ";

   if (cell_count > 0) {
      int idx, first_idx;
      float min_value, max_value;
      bool do_scale_factor = has_scale_factor_attr(var) || has_add_offset_attr(var);

      idx = first_idx = 0;
      if (do_scale_factor) {
         int positive_cnt = 0;
         int unpacked_count = 0;
         T raw_min_val, raw_max_val;

         if (has_missing) {
            for (; idx<cell_count; idx++) {
               if (!is_eq(missing_value, packed_data[idx])) {
                  first_idx = idx;
                  break;
               }
               data[idx] = bad_data_float;
            }
         }

         raw_min_val = raw_max_val = packed_data[first_idx];
         if (has_missing && is_eq(missing_value, raw_min_val)) {
            min_value = max_value = bad_data_float;
         }
         else min_value = max_value = ((float)raw_min_val * scale_factor) + add_offset;

         for (; idx<cell_count; idx++) {
            if (has_missing && is_eq(missing_value, packed_data[idx]))
               data[idx] = bad_data_float;
            else {
               if (raw_min_val > packed_data[idx]) raw_min_val = packed_data[idx];
               if (raw_max_val < packed_data[idx]) raw_max_val = packed_data[idx];
               data[idx] = ((float)packed_data[idx] * scale_factor) + add_offset;
               if (data[idx] > 0) positive_cnt++;
               if (min_value > data[idx]) min_value = data[idx];
               if (max_value < data[idx]) max_value = data[idx];
               unpacked_count++;
            }
         }
         mlog << Debug(7) << method_name << GET_NC_NAME_P(var)
              << " apply_scale_factor unpacked data: count="
              << unpacked_count << " out of " << cell_count
              << ". FillValue(" << data_type << ")=" << missing_value << "\n";
         mlog << Debug(7) << method_name
              << "data range [" << min_value << " - " << max_value
              << "] raw data: [" << raw_min_val << " - " << raw_max_val
              << "] Positive count: " << positive_cnt << "\n";
      }
      else {

         if (has_missing) {
            for (; idx<cell_count; idx++) {
               if (!is_eq(missing_value, packed_data[idx])) {
                  first_idx = idx;
                  break;
               }
               data[idx] = bad_data_float;
            }
         }
         min_value = max_value = (float)packed_data[first_idx];
         for (; idx<cell_count; idx++) {
            if (has_missing && is_eq(missing_value, packed_data[idx]))
               data[idx] = bad_data_float;
            else {
               data[idx] = (float)packed_data[idx];
               if (min_value > data[idx]) min_value = data[idx];
               if (max_value < data[idx]) max_value = data[idx];
            }
         }
         mlog << Debug(7) << method_name << "data range [" << min_value
              << " - " << max_value << "]\n";
      }
   }
   mlog << Debug(7) << method_name << "took "
        << (clock()-start_clock)/double(CLOCKS_PER_SEC) << " seconds\n";
   return;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
void copy_nc_data_(netCDF::NcVar *var, float *data, const T *packed_data,
                   const int cell_count, const char *data_type,
                   double add_offset, double scale_factor) {
   T missing_value;
   bool has_missing = get_var_fill_value(var, missing_value);
   copy_nc_data_t(var, data, packed_data, cell_count, data_type,
                  add_offset, scale_factor, has_missing, missing_value);
   return;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
void copy_nc_data_t(netCDF::NcVar *var, double *data, const T *packed_data,
                    const int cell_count, const char *data_type,
                    double add_offset, double scale_factor,
                    bool has_missing, T missing_value) {
   const char *method_name = "copy_nc_data_t(double) ";

   if (cell_count > 0) {
      int idx;
      int first_idx;
      int unpacked_count = 0;
      double min_value, max_value;
      bool do_scale_factor = has_scale_factor_attr(var) || has_add_offset_attr(var);

      idx = first_idx = 0;
      if (do_scale_factor) {
         int positive_cnt = 0;
         T raw_min_val, raw_max_val;

         if (has_missing) {
            // Skip missing values to find first min/max value
            for (; idx<cell_count; idx++) {
               if (!is_eq(missing_value, packed_data[idx])) {
                  first_idx = idx;
                  break;
               }
               data[idx] = bad_data_double;
            }
         }

         raw_min_val = raw_max_val = packed_data[first_idx];
         if (has_missing && is_eq(missing_value, raw_min_val)) {
            min_value = max_value = bad_data_double;
         }
         else min_value = max_value = ((double)raw_min_val * scale_factor) + add_offset;

         for (; idx<cell_count; idx++) {
            if (has_missing && is_eq(missing_value, packed_data[idx]))
               data[idx] = bad_data_double;
            else {
               if (raw_min_val > packed_data[idx]) raw_min_val = packed_data[idx];
               if (raw_max_val < packed_data[idx]) raw_max_val = packed_data[idx];
               data[idx] = ((double)packed_data[idx] * scale_factor) + add_offset;
               if (data[idx] > 0) positive_cnt++;
               if (min_value > data[idx]) min_value = data[idx];
               if (max_value < data[idx]) max_value = data[idx];
               unpacked_count++;
            }
         }
         mlog << Debug(7) << method_name << GET_NC_NAME_P(var)
              << " apply_scale_factor unpacked data: count="
              << unpacked_count << " out of " << cell_count
              << ". FillValue(" << data_type << ")=" << missing_value
              << " data range [" << min_value << " - " << max_value
              << "] raw data: [" << raw_min_val << " - " << raw_max_val
              << "] Positive count: " << positive_cnt << "\n";
      }
      else {
         if (has_missing) {
            for (; idx<cell_count; idx++) {
               if (!is_eq(missing_value, packed_data[idx])) {
                  first_idx = idx;
                  break;
               }
               data[idx] = bad_data_float;
            }
         }
         min_value = max_value = (double)packed_data[first_idx];
         for (; idx<cell_count; idx++) {
            if (has_missing && is_eq(missing_value, packed_data[idx]))
               data[idx] = bad_data_double;
            else {
               data[idx] = (double)packed_data[idx];
               if (min_value > data[idx]) min_value = data[idx];
               if (max_value < data[idx]) max_value = data[idx];
            }
         }
         mlog << Debug(7) << method_name << "data range [" << min_value
              << " - " << max_value << "]\n";
      }
   }
}

////////////////////////////////////////////////////////////////////////

template <typename T>
void copy_nc_data_(netCDF::NcVar *var, double *data, const T *packed_data,
                   const int cell_count, const char *data_type,
                   double add_offset, double scale_factor) {
   T missing_value;
   bool has_missing = get_var_fill_value(var, missing_value);
   copy_nc_data_t(var, data, packed_data, cell_count, data_type,
                  add_offset, scale_factor, has_missing, missing_value);
   return;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool put_nc_data_T(netCDF::NcVar *var, const T data, long offset0, long offset1, long offset2) {
   std::vector<size_t> offsets;
   offsets.push_back((size_t)offset0);
   if (0 <= offset1) {
     offsets.push_back((size_t)offset1);
   }
   if (0 <= offset2) {
     offsets.push_back((size_t)offset2);
   }
   var->putVar(offsets, data);
   return true;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool put_nc_data_T(netCDF::NcVar *var, const T *data,    const long length, const long offset) {
   std::vector<size_t> offsets, counts;
   int dim_count = get_dim_count(var);
   offsets.push_back(offset);
   if (dim_count >= 2) {
      offsets.push_back(0);
      counts.push_back(1);
   }
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool put_nc_data_T(netCDF::NcVar *var, const T *data , const long *lengths, const long *offsets) {
   int dim = get_dim_count(var);
   std::vector<size_t> nc_offsets, counts;
   for (int idx = 0 ; idx < dim; idx++) {
      nc_offsets.push_back(offsets[idx]);
   }
   for (int idx = 0 ; idx < dim; idx++) {
      counts.push_back(lengths[idx]);
   }
   var->putVar(nc_offsets, counts, data);
   return true;
}

////////////////////////////////////////////////////////////////////////

template <typename T>
bool put_nc_data_T_with_dims(netCDF::NcVar *var, const T *data,
                             const long len0, const long len1, const long len2) {
   std::vector<size_t> offsets, counts;
   if (0 < len0) {
     offsets.push_back(0);
     counts.push_back(len0);
   }
   if (0 < len1) {
     offsets.push_back(0);
     counts.push_back(len1);
   }
   if (0 < len2) {
     offsets.push_back(0);
     counts.push_back(len2);
   }
   var->putVar(offsets, counts, data);
   return true;
}


////////////////////////////////////////////////////////////////////////

#endif   /*  __NC_UTILS_HPP__  */

////////////////////////////////////////////////////////////////////////
