// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __NC_UTILS_H__
#define  __NC_UTILS_H__

////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <time.h>

#include <netcdf>

#include "concat_string.h"
#include "int_array.h"
#include "long_array.h"
#include "num_array.h"
#include "nc_var_info.h"

#include "nc_utils.hpp"

////////////////////////////////////////////////////////////////////////

#ifndef uchar
typedef unsigned char uchar;
#endif   /*  uchar  */

////////////////////////////////////////////////////////////////////////

#define DEF_DEFLATE_LEVEL   (0)

#define GET_NC_ATT_OBJ(nc_or_var, att_name)         nc_or_var.getAtt(att_name)
#define GET_NC_ATT_OBJ_BY_P(nc_or_var, att_name)    nc_or_var->getAtt(att_name)

#define DEF_NC_BUFFER_SIZE              (64*1024)
#define NC_BUFFER_SIZE_32K              (32*1024)
#define NC_BUFFER_SIZE_16K              (16*1024)

#define HDR_ARRAY_LEN    3   // Observation header length
#define HDR_TYPE_ARR_LEN 3   // Observation header type length (prpt/irpt/inst)
#define OBS_ARRAY_LEN    5   // Observation values length
#define HEADER_STR_LEN   16  // Maximum length for header string
#define HEADER_STR_LEN2  40  // Maximum length for header string 2
#define HEADER_STR_LEN3  80  // Maximum length for header string 3

#define OBS_BUFFER_SIZE  (128 * 1024)

constexpr char nc_dim_nhdr[]              = "nhdr";
constexpr char nc_dim_nhdr_typ[]          = "nhdr_typ";
constexpr char nc_dim_nhdr_sid[]          = "nhdr_sid";
constexpr char nc_dim_nhdr_vld[]          = "nhdr_vld";
constexpr char nc_dim_npbhdr[]            = "npbhdr";
constexpr char nc_dim_nobs[]              = "nobs";
constexpr char nc_dim_nqty[]              = "nobs_qty";
constexpr char nc_dim_hdr_arr[]           = "hdr_arr_len";
constexpr char nc_dim_obs_arr[]           = "obs_arr_len";
constexpr char nc_dim_mxstr[]             = "mxstr";
constexpr char nc_dim_mxstr2[]            = "mxstr2";
constexpr char nc_dim_mxstr3[]            = "mxstr3";
constexpr char nc_dim_nvar[]              = "obs_var_num";
constexpr char nc_dim_unit[]              = "unit_len";
constexpr char nc_dim_desc[]              = "desc_len";
constexpr char nc_var_desc[]              = "obs_desc";
constexpr char nc_var_hdr_arr[]           = "hdr_arr";
constexpr char nc_var_hdr_lat[]           = "hdr_lat";
constexpr char nc_var_hdr_lon[]           = "hdr_lon";
constexpr char nc_var_hdr_elv[]           = "hdr_elv";
constexpr char nc_var_hdr_typ[]           = "hdr_typ";
constexpr char nc_var_hdr_sid[]           = "hdr_sid";
constexpr char nc_var_hdr_vld[]           = "hdr_vld";
constexpr char nc_var_hdr_prpt_typ[]      = "hdr_prpt_typ";
constexpr char nc_var_hdr_irpt_typ[]      = "hdr_irpt_typ";
constexpr char nc_var_hdr_inst_typ[]      = "hdr_inst_typ";
constexpr char nc_var_hdr_typ_tbl[]       = "hdr_typ_table";
constexpr char nc_var_hdr_sid_tbl[]       = "hdr_sid_table";
constexpr char nc_var_hdr_vld_tbl[]       = "hdr_vld_table";
constexpr char nc_var_obs_arr[]           = "obs_arr";
constexpr char nc_var_obs_hid[]           = "obs_hid";
constexpr char nc_var_obs_gc[]            = "obs_gc";
constexpr char nc_var_obs_vid[]           = "obs_vid";
constexpr char nc_var_obs_lvl[]           = "obs_lvl";
constexpr char nc_var_obs_hgt[]           = "obs_hgt";
constexpr char nc_var_obs_val[]           = "obs_val";
constexpr char nc_var_obs_qty[]           = "obs_qty";
constexpr char nc_var_obs_qty_tbl[]       = "obs_qty_table";
constexpr char nc_var_obs_var[]           = "obs_var";
constexpr char nc_var_unit[]              = "obs_unit";
constexpr char nc_att_use_var_id[]        = "use_var_id";
constexpr char nc_att_obs_version[]       = "MET_Obs_version";
constexpr char nc_att_met_point_nccf[]    = "MET_point_NCCF";

static const std::string add_offset_att_name           = "add_offset";
static const std::string axis_att_name                 = "axis";
static const std::string bounds_att_name               = "bounds";
static const std::string coordinates_att_name          = "coordinates";
static const std::string coordinate_axis_type_att_name = "_CoordinateAxisType";
static const std::string cf_att_name                   = "Conventions";
static const std::string description_att_name          = "description";
static const std::string grid_mapping_att_name         = "grid_mapping";
static const std::string grid_mapping_name_att_name    = "grid_mapping_name";
static const std::string long_name_att_name            = "long_name";
static const std::string projection_att_name           = "Projection";
static const std::string scale_factor_att_name         = "scale_factor";
static const std::string standard_name_att_name        = "standard_name";
static const std::string units_att_name                = "units";

constexpr char nc_time_unit_exp[]    = "^[a-z|A-Z]* *since *[0-9]\\{1,4\\}-[0-9]\\{1,2\\}-[0-9]\\{1,2\\}";
constexpr char nc_time_unit_ymd_exp[] = "[0-9]\\{1,4\\}-[0-9]\\{1,2\\}-[0-9]\\{1,2\\}";

constexpr char MET_NC_Obs_ver_1_2[]  = "1.02";
constexpr char MET_NC_Obs_version[]  = "1.02";

constexpr int exit_code_no_error    = 0;
constexpr int exit_code_no_dim      = 1;
constexpr int exit_code_no_hdr_vars = 2;
constexpr int exit_code_no_loc_vars = 3;
constexpr int exit_code_no_obs_vars = 4;

////////////////////////////////////////////////////////////////////////

extern bool      get_att_value(const netCDF::NcAtt *, int    &value);
extern bool      get_att_value(const netCDF::NcAtt *, float  &value);
extern bool      get_att_value(const netCDF::NcAtt *, double &value);

extern bool      get_att_value_chars  (const netCDF::NcAtt *, ConcatString &);
extern int       get_att_value_int    (const netCDF::NcAtt *);
extern long long get_att_value_llong  (const netCDF::NcAtt *);
extern double    get_att_value_double (const netCDF::NcAtt *);
extern void      get_att_value_doubles(const netCDF::NcAtt *, NumArray &);
extern float     get_att_value_float  (const netCDF::NcAtt *);

extern bool      get_att_value_string(const netCDF::NcVar *, const ConcatString &, ConcatString &);
extern int       get_att_value_int   (const netCDF::NcVar *, const ConcatString &);
extern long long get_att_value_llong (const netCDF::NcVar *, const ConcatString &);
extern double    get_att_value_double(const netCDF::NcVar *, const ConcatString &);

extern bool      get_att_value_string(const netCDF::NcFile *, const ConcatString& , ConcatString &);
extern int       get_att_value_int   (const netCDF::NcFile *, const ConcatString& );
extern long long get_att_value_llong (const netCDF::NcFile *, const ConcatString& );
extern double    get_att_value_double(const netCDF::NcFile *, const ConcatString& );
extern bool      get_att_no_leap_year(const netCDF::NcVar *);

extern bool      get_cf_conventions(const netCDF::NcFile *, ConcatString&);

extern bool get_nc_att_value(const netCDF::NcVarAtt *, std::string &);
extern bool get_nc_att_value(const netCDF::NcVarAtt *, int          &, bool exit_on_error = true);
extern bool get_nc_att_value(const netCDF::NcVarAtt *, float        &, bool exit_on_error = true);
extern bool get_nc_att_value(const netCDF::NcVarAtt *, double       &, bool exit_on_error = true);
extern bool get_nc_att_value(const netCDF::NcVar *, const ConcatString &, ConcatString &,
                             int nc_id=0, bool exit_on_error = false);
extern bool get_nc_att_value(const netCDF::NcVar *, const ConcatString &, int          &, bool exit_on_error = false);
extern bool get_nc_att_value(const netCDF::NcVar *, const ConcatString &, float        &, bool exit_on_error = false);
extern bool get_nc_att_value(const netCDF::NcVar *, const ConcatString &, double       &, bool exit_on_error = false);
extern bool get_nc_att_values(const netCDF::NcVar *, const ConcatString &, unsigned short *, bool exit_on_error = false);

extern bool has_att(netCDF::NcFile *, const ConcatString name, bool exit_on_error=false);
extern bool has_att(netCDF::NcVar *, const ConcatString name, bool do_log=false);
extern bool has_unsigned_attribute(netCDF::NcVar *);

extern bool get_global_att(const netCDF::NcGroupAtt *, ConcatString &);
extern bool get_global_att(const char *,   const ConcatString &, bool &);
extern bool get_global_att(const char *,   const ConcatString &, ConcatString &);
extern bool get_global_att(const netCDF::NcFile *, const ConcatString &, ConcatString &, bool error_out = false);
extern bool get_global_att(const netCDF::NcFile *, const ConcatString &, int &, bool error_out = false);
extern bool get_global_att(const netCDF::NcFile *, const ConcatString &, bool &, bool error_out = false);
extern bool get_global_att(const netCDF::NcFile *, const ConcatString &, float &, bool error_out = false);
extern bool get_global_att(const netCDF::NcFile *, const ConcatString &, double &, bool error_out = false);

extern  int get_version_no(const netCDF::NcFile *);
extern bool is_version_less_than_1_02(const netCDF::NcFile *nc);

extern void add_att(netCDF::NcFile *, const std::string &, const int   );
extern void add_att(netCDF::NcFile *, const std::string &, const std::string);
extern void add_att(netCDF::NcFile *, const std::string &, const char *);
extern void add_att(netCDF::NcFile *, const std::string &, const ConcatString);
extern void add_att(netCDF::NcVar  *, const std::string &, const std::string);
extern void add_att(netCDF::NcVar  *, const std::string &, const int   );
extern void add_att(netCDF::NcVar  *, const std::string &, const float );
extern void add_att(netCDF::NcVar  *, const std::string &, const double);

extern int    get_var_names(netCDF::NcFile *, StringArray *var_names);
extern int    get_var_names(netCDF::NcFile *, StringArray *var_names, StringArray &group_names);

extern bool   get_var_att_float (const netCDF::NcVar *, const ConcatString &, float  &);
extern bool   get_var_att_double(const netCDF::NcVar *, const ConcatString &, double &);

template <typename T>
extern bool   get_var_fill_value(const netCDF::NcVar *var, T &att_val);
extern bool   get_var_axis(const netCDF::NcVar *var, ConcatString &att_val);
extern bool   get_var_grid_mapping(const netCDF::NcVar *var, ConcatString &att_val);
extern bool   get_var_grid_mapping_name(const netCDF::NcVar *var, ConcatString &att_val);
extern bool   get_var_long_name(const netCDF::NcVar *, ConcatString &);
extern double get_var_missing_value(const netCDF::NcVar *);
extern bool   get_var_standard_name(const netCDF::NcVar *, ConcatString &);
extern bool   get_var_units(const netCDF::NcVar *, ConcatString &);

extern bool   args_ok(const LongArray &);

extern char   get_char_val(netCDF::NcFile *, const char * var_name, const int index);
extern char   get_char_val(netCDF::NcVar *var, const int index);

extern int    get_int_var(netCDF::NcFile *, const char * var_name, const int index);
extern int    get_int_var(netCDF::NcVar *, const int index);

extern double get_nc_time(netCDF::NcVar *, int index = 0);

extern float  get_float_var(netCDF::NcFile *, const char * var_name, const int index = 0);
extern float  get_float_var(netCDF::NcVar *, int const index = 0);

extern ConcatString* get_string_val(netCDF::NcFile *, const char * var_name, const int index,
                                    const int len, ConcatString &tmp_cs);
extern ConcatString* get_string_val(netCDF::NcVar *var, const int index, const int len, ConcatString &tmp_cs);

extern bool get_nc_data_ptr(netCDF::NcVar *, char            *data);
extern bool get_nc_data_ptr(netCDF::NcVar *, char           **data);
extern bool get_nc_data(netCDF::NcVar *, std::vector<int>    &data);
extern bool get_nc_data(netCDF::NcVar *, std::vector<char>   &data);
extern bool get_nc_data(netCDF::NcVar *, std::vector<long>   &data);
extern bool get_nc_data(netCDF::NcVar *, std::vector<uchar>  &data, bool allow_conversion=false);
extern bool get_nc_data(netCDF::NcVar *, std::vector<float>  &data);
extern bool get_nc_data(netCDF::NcVar *, std::vector<double> &data);
extern bool get_nc_data(netCDF::NcVar *, std::vector<time_t> &data);
extern bool get_nc_data(netCDF::NcVar *, std::vector<unsigned short> &data);

extern bool get_nc_data(netCDF::NcVar *, std::vector<int>    &data, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<char>   &data, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<short>  &data, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<float>  &data, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<double> &data, const LongArray &curs);

extern bool get_nc_data(netCDF::NcVar *, std::vector<int>    &data, const long dim, const long cur=0);
extern bool get_nc_data(netCDF::NcVar *, std::vector<char>   &data, const long dim, const long cur=0);
extern bool get_nc_data(netCDF::NcVar *, std::vector<float>  &data, const long dim, const long cur=0);
extern bool get_nc_data(netCDF::NcVar *, std::vector<double> &data, const long dim, const long cur=0);
extern bool get_nc_data(netCDF::NcVar *, std::vector<ncbyte> &data, const long dim, const long cur=0);
extern bool get_nc_data_ptr(netCDF::NcVar *, char            *data, const long dim, const long cur=0);

extern bool get_nc_data_ptr(netCDF::NcVar *, char            *data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data_ptr(netCDF::NcVar *, float           *data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data_ptr(netCDF::NcVar *, int             *data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<int>    &data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<char>   &data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<short>  &data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<float>  &data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<double> &data, const LongArray &dims, const LongArray &curs);
extern bool get_nc_data(netCDF::NcVar *, std::vector<ncbyte> &data, const LongArray &dims, const LongArray &curs);

extern bool get_nc_data_to_array(netCDF::NcVar  *, StringArray *);
extern bool get_nc_data_to_array(netCDF::NcFile *, const char *, StringArray *);
extern int  get_nc_string_length(netCDF::NcVar  *);
extern int  get_nc_string_length(netCDF::NcFile *, netCDF::NcVar, const char *var_name);

extern bool put_nc_data(netCDF::NcVar *, const std::vector<int>    &data );
extern bool put_nc_data(netCDF::NcVar *, const std::vector<char>   &data );
extern bool put_nc_data(netCDF::NcVar *, const std::vector<float>  &data );
extern bool put_nc_data(netCDF::NcVar *, const std::vector<double> &data );
extern bool put_nc_data(netCDF::NcVar *, const std::vector<ncbyte> &data );
extern bool put_nc_data_ptr(netCDF::NcVar *, const int             *data );
extern bool put_nc_data_ptr(netCDF::NcVar *, const char            *data );

extern bool put_nc_data_ptr(netCDF::NcVar *, const double data, const long offset0=0, const long offset1=-1, const long c2=-1);

extern bool put_nc_data(netCDF::NcVar *, const std::vector<int>   &data, const long length, const long offset);
extern bool put_nc_data(netCDF::NcVar *, const std::vector<char>  &data, const long length, const long offset);
extern bool put_nc_data(netCDF::NcVar *, const std::vector<float> &data, const long length, const long offset);

extern bool put_nc_data_ptr(netCDF::NcVar *, const int            *data, const long *lengths, const long *offsets);
extern bool put_nc_data_ptr(netCDF::NcVar *, const char           *data, const long *lengths, const long *offsets);
extern bool put_nc_data_ptr(netCDF::NcVar *, const float          *data, const long *lengths, const long *offsets);
extern bool put_nc_data(netCDF::NcVar *, const std::vector<int>   &data, const long *lengths, const long *offsets);
extern bool put_nc_data(netCDF::NcVar *, const std::vector<char>  &data, const long *lengths, const long *offsets);
extern bool put_nc_data(netCDF::NcVar *, const std::vector<float> &data, const long *lengths, const long *offsets);

extern bool put_nc_data_with_dims(netCDF::NcVar *, const std::vector<int> &data, const int len0,
                                  const int len1=0, const int len2=0);
extern bool put_nc_data_with_dims(netCDF::NcVar *, const std::vector<int> &data, const long len0,
                                  const long len1=0, const long len2=0);
extern bool put_nc_data_with_dims(netCDF::NcVar *, const std::vector<float> &data, const int len0,
                                  const int len1=0, const int len2=0);
extern bool put_nc_data_with_dims(netCDF::NcVar *, const std::vector<float> &data, const long len0,
                                  const long len1=0, const long len2=0);
extern bool put_nc_data_with_dims(netCDF::NcVar *, const std::vector<double> &data, const int len0,
                                  const int len1=0, const int len2=0);
extern bool put_nc_data_with_dims(netCDF::NcVar *, const std::vector<double> &data, const long len0,
                                  const long len1=0, const long len2=0);
//extern bool put_nc_data_with_dims_ptr(netCDF::NcVar *, const int *data, const long len0,
//                                  const long len1=0, const long len2=0);
//extern bool put_nc_data_with_dims_ptr(netCDF::NcVar *, const float *data, const long len0,
//                                  const long len1=0, const long len2=0);
extern bool put_nc_data_with_dims_ptr(netCDF::NcVar *, const double *data, const long len0,
                                  const long len1=0, const long len2=0);    // DataPlane

extern netCDF::NcGroup  get_nc_group(netCDF::NcFile *, const char *group_name);     // continue even though not exists

extern netCDF::NcVar    get_var(netCDF::NcFile *, const char *var_name);    // exit if exists but invalid
extern netCDF::NcVar    get_var(netCDF::NcFile *, const char *var_name,
                                const char *group_name);                    // continue even though not exists
extern netCDF::NcVar get_nc_var(netCDF::NcFile *, const char *var_name,
                                bool log_as_error=false);                   // continue even though not exists
extern netCDF::NcVar get_nc_var(netCDF::NcFile *, const ConcatString &var_name,
                                bool log_as_error=false);                   // continue even though not exists
extern netCDF::NcVar get_nc_var(netCDF::NcFile *, const char *var_name,
                                const char *group_name, bool log_as_error=false);   // continue even though not exists
extern netCDF::NcVar get_nc_var(netCDF::NcFile *, const ConcatString &var_name,
                                const char *group_name, bool log_as_error=false);   // continue even though not exists

extern netCDF::NcVar *copy_nc_var(netCDF::NcFile *,  netCDF::NcVar *,
                                  const int deflate_level=DEF_DEFLATE_LEVEL, const bool all_attrs=true);
extern void   copy_nc_att(netCDF::NcFile *, netCDF::NcVar *, const ConcatString attr_name);
extern void   copy_nc_att( netCDF::NcVar *,  netCDF::NcVar *, const ConcatString attr_name);
extern void  copy_nc_atts(netCDF::NcFile *, netCDF::NcFile *, const bool all_attrs=true);
extern void  copy_nc_atts( netCDF::NcVar *,  netCDF::NcVar *, const bool all_attrs=true);
extern void copy_nc_var_data(netCDF::NcVar *, netCDF::NcVar *);

extern bool has_nc_group(netCDF::NcFile *, const char *group_name);
extern bool has_var(netCDF::NcFile *, const char *var_name);
extern bool has_var(netCDF::NcFile *, const char *var_name, const char *group_name);

extern netCDF::NcVar  add_var(netCDF::NcFile *, const std::string &, const netCDF::NcType, const int deflate_level=DEF_DEFLATE_LEVEL);
extern netCDF::NcVar  add_var(netCDF::NcFile *, const std::string &, const netCDF::NcType, const netCDF::NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern netCDF::NcVar  add_var(netCDF::NcFile *, const std::string &, const netCDF::NcType, const netCDF::NcDim, const netCDF::NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern netCDF::NcVar  add_var(netCDF::NcFile *, const std::string &, const netCDF::NcType, const netCDF::NcDim, const netCDF::NcDim, const netCDF::NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern netCDF::NcVar  add_var(netCDF::NcFile *, const std::string &, const netCDF::NcType, const netCDF::NcDim, const netCDF::NcDim, const netCDF::NcDim, const netCDF::NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern netCDF::NcVar  add_var(netCDF::NcFile *, const std::string &, const netCDF::NcType, const std::vector<netCDF::NcDim>, const int deflate_level=DEF_DEFLATE_LEVEL);

extern netCDF::NcDim  add_dim(netCDF::NcFile *, const std::string &);
extern netCDF::NcDim  add_dim(netCDF::NcFile *, const std::string &, const size_t);
extern bool   has_dim(netCDF::NcFile *, const char *dim_name);
extern bool   get_dim(const netCDF::NcFile *, const ConcatString &, int &, bool error_out = false);
extern int    get_dim_count(const netCDF::NcFile *);
extern int    get_dim_value(const netCDF::NcFile *, const std::string &, const bool error_out = false);
extern bool   get_dim_names(const netCDF::NcVar *var, StringArray *dimNames);
extern bool   get_dim_names(const netCDF::NcFile *nc, StringArray *dimNames);

extern netCDF::NcVar  get_nc_var_lat(const netCDF::NcFile *nc);
extern netCDF::NcVar  get_nc_var_lon(const netCDF::NcFile *nc);
extern netCDF::NcVar  get_nc_var_time(const netCDF::NcFile *nc);
extern int    get_index_at_nc_data(netCDF::NcVar *var, double value, const std::string dim_name, bool is_time=false);
extern netCDF::NcFile* open_ncfile(const char * nc_name, bool write = false);

extern unixtime get_reference_unixtime(netCDF::NcVar *time_var, int &sec_per_unit,
                                       bool &no_leap_year);

extern bool is_nc_unit_time(const char *units);
extern bool is_nc_unit_longitude(const char *units);
extern bool is_nc_unit_latitude(const char *units);

extern void parse_cf_time_string(const char *str, unixtime &ref_ut,
                                 int &sec_per_unit);
extern void parse_time_string(const char *str, unixtime &ut);

////////////////////////////////////////////////////////////////////////

#endif   /*  __NC_UTILS_H__  */

////////////////////////////////////////////////////////////////////////
