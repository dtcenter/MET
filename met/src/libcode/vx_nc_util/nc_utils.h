// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __NC_UTILS_H__
#define  __NC_UTILS_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <string.h>
#include <time.h>

#include <netcdf>
using namespace netCDF;
typedef signed char ncbyte; // from ncvalues.h

#include "concat_string.h"
#include "long_array.h"

////////////////////////////////////////////////////////////////////////

#define IS_INVALID_NC(ncObj)        ncObj.isNull()
#define IS_INVALID_NC_P(ncObjPtr)   (ncObjPtr == 0 || ncObjPtr->isNull())

#define GET_NC_NAME(ncObj)          ncObj.getName()
#define GET_NC_NAME_P(ncObjPtr)     ncObjPtr->getName()

#define GET_NC_SIZE(ncObj)          ncObj.getSize()
#define GET_NC_SIZE_P(ncObjPtr)     ncObjPtr->getSize()

//#define GET_NC_TYPE(ncObj)          ncObj.getType()
//#define GET_NC_TYPE_P(ncObjPtr)     ncObjPtr->getType()

#define GET_NC_TYPE_ID(ncObj)            ncObj.getType().getId()
#define GET_NC_TYPE_ID_P(ncObjPtr)       ncObjPtr->getType().getId()
#define GET_NC_TYPE_NAME(ncObj)          ncObj.getType().getName()
#define GET_NC_TYPE_NAME_P(ncObjPtr)     ncObjPtr->getType().getName()

#define GET_NC_DIM_COUNT(ncObj)          ncObj.getDimCount()
#define GET_NC_DIM_COUNT_P(ncObjPtr)     ncObjPtr->getDimCount()

#define GET_NC_VAR_COUNT(ncObj)          ncObj.getVarCount()
#define GET_NC_VAR_COUNT_P(ncObjPtr)     ncObjPtr->getVarCount()

#define DEF_DEFLATE_LEVEL   (0)

#define GET_NC_ATT_OBJ(nc_or_var, att_name)         nc_or_var.getAtt(att_name)
#define GET_NC_ATT_OBJ_BY_P(nc_or_var, att_name)    nc_or_var->getAtt(att_name)

#define DEF_NC_BUFFER_SIZE          (64*1024)

////////////////////////////////////////////////////////////////////////

//extern bool find_att(const NcFile *, const ConcatString &, NcGroupAtt &);
//extern bool find_att(const NcVar  *, const ConcatString &, NcVarAtt   &);

extern bool      get_att_value_chars  (const NcAtt *, ConcatString &);
extern int       get_att_value_int    (const NcAtt *);
extern long long get_att_value_llong  (const NcAtt *);
extern double    get_att_value_double (const NcAtt *);
extern double*   get_att_value_doubles(const NcAtt *);
extern float     get_att_value_float  (const NcAtt *);

extern bool      get_att_value_string(const NcVar *, const ConcatString &, ConcatString &);
extern int       get_att_value_int   (const NcVar *, const ConcatString &);
extern long long get_att_value_llong (const NcVar *, const ConcatString &);
extern double    get_att_value_double(const NcVar *, const ConcatString &);

extern bool      get_att_value_string(const NcFile *, const ConcatString& , ConcatString &);
extern int       get_att_value_int   (const NcFile *, const ConcatString& );
extern long long get_att_value_llong (const NcFile *, const ConcatString& );
extern double    get_att_value_double(const NcFile *, const ConcatString& );

extern bool      get_att_no_leap_year(const NcVar *);

extern NcVarAtt    get_nc_att(const NcVar  *, const ConcatString &, bool exit_on_error = false);
extern NcGroupAtt  get_nc_att(const NcFile *, const ConcatString &, bool exit_on_error = false);
//extern NcVarAtt   *get_nc_att(const NcVar  *, const ConcatString &, bool exit_on_error = true);
//extern NcGroupAtt *get_nc_att(const NcFile *, const ConcatString &, bool exit_on_error = true);
//extern NcGroupAtt get_att(const NcFile *, const ConcatString &, bool exit_on_error = true);


extern bool get_nc_att(const NcVarAtt *, ConcatString &);
extern bool get_nc_att(const NcVarAtt *, int          &, bool exit_on_error = true);
extern bool get_nc_att(const NcVarAtt *, float        &, bool exit_on_error = true);
extern bool get_nc_att(const NcVarAtt *, double       &, bool exit_on_error = true);
extern bool get_nc_att(const NcVar *, const ConcatString &, ConcatString &);
extern bool get_nc_att(const NcVar *, const ConcatString &, int          &, bool exit_on_error = false);
extern bool get_nc_att(const NcVar *, const ConcatString &, float        &, bool exit_on_error = false);
//extern NcVarAtt *get_var_att(const NcVar *, const ConcatString &, bool exit_on_error = true);

extern bool has_att(NcFile *, const char * name, bool exit_on_error = false);

extern bool get_global_att(const NcGroupAtt *, ConcatString &);
//extern bool get_global_att(const NcFile *, NcGroupAtt *, const char *, bool error_out = false);
extern bool get_global_att(const NcFile *, const ConcatString &, ConcatString &, bool error_out = false);
extern bool get_global_att(const NcFile *, const char *, int &, bool error_out = false);
extern bool get_global_att(const NcFile *, const char *, float &, bool error_out = false);
extern bool get_global_att(const NcFile *, const char *, double &, bool error_out = false);
extern bool get_global_att_double(const NcFile *, const ConcatString &, double &, bool error_out = false);



extern void add_att(NcFile *, const string, const int   );
extern void add_att(NcFile *, const string, const string);
extern void add_att(NcFile *, const string, const char *);
extern void add_att(NcVar  *, const string, const string);
extern void add_att(NcVar  *, const string, const int   );
extern void add_att(NcVar  *, const string, const float );
extern void add_att(NcVar  *, const string, const double);

extern int    get_var_names(NcFile *, StringArray *varNames);

extern bool     get_var_att_float (const NcVar *, const ConcatString &, float  &);
extern bool     get_var_att_double(const NcVar *, const ConcatString &, double &);

extern bool   get_var_units(const NcVar *, ConcatString &);

extern bool   get_var_level(const NcVar *, ConcatString &);

extern double get_var_missing_value(const NcVar *);

extern double get_var_fill_value(const NcVar *);

extern bool   args_ok(const LongArray &);

extern char   get_char_val(NcFile *, const char * var_name, const int index);

extern char   get_char_val(NcVar *var, const int index);

extern int    get_int_var(NcFile *, const char * var_name, const int index);

extern int    get_int_var(NcVar *, const int index);

extern double get_double_var(NcFile *, const char * var_name, const int index = 0);

extern double get_double_var(NcVar *, int index = 0);

extern float  get_float_var(NcFile *, const char * var_name, const int index = 0);

extern float  get_float_var(NcVar *, int const index = 0);

extern ConcatString* get_string_val(NcFile *, const char * var_name, const int index,
                    const int len, ConcatString &tmp_cs);

extern ConcatString* get_string_val(NcVar *var, const int index, const int len, ConcatString &tmp_cs);


extern bool get_nc_data(NcVar *, int    *data);
extern bool get_nc_data(NcVar *, char   *data);
extern bool get_nc_data(NcVar *, float  *data);
extern bool get_nc_data(NcVar *, double *data);
extern bool get_nc_data(NcVar *, time_t *data);

extern bool get_nc_data(NcVar *, int    *data, const long *cur);
extern bool get_nc_data(NcVar *, char   *data, const long *cur);
extern bool get_nc_data(NcVar *, short  *data, const long *cur);
extern bool get_nc_data(NcVar *, float  *data, const long *cur);
extern bool get_nc_data(NcVar *, double *data, const long *cur);

extern bool get_nc_data(NcVar *, int    *data, const long dim, const long cur=0);
extern bool get_nc_data(NcVar *, char   *data, const long dim, const long cur=0);
extern bool get_nc_data(NcVar *, float  *data, const long dim, const long cur=0);
extern bool get_nc_data(NcVar *, double *data, const long dim, const long cur=0);
extern bool get_nc_data(NcVar *, ncbyte *data, const long dim, const long cur=0);

extern bool get_nc_data(NcVar *, int    *data, const long *dim, const long *cur);
extern bool get_nc_data(NcVar *, char   *data, const long *dim, const long *cur);
extern bool get_nc_data(NcVar *, short  *data, const long *dim, const long *cur);
extern bool get_nc_data(NcVar *, float  *data, const long *dim, const long *cur);
extern bool get_nc_data(NcVar *, double *data, const long *dim, const long *cur);
extern bool get_nc_data(NcVar *, ncbyte *data, const long *dim, const long *cur);

extern bool get_nc_data(NcFile *, const char *var_name, int    *data, const long *cur, const long *dim);
extern bool get_nc_data(NcFile *, const char *var_name, char   *data, const long *cur, const long *dim);
extern bool get_nc_data(NcFile *, const char *var_name, float  *data, const long *cur, const long *dim);
extern bool get_nc_data(NcFile *, const char *var_name, double *data, const long *cur, const long *dim);
extern bool get_nc_data(NcFile *, const char *var_name, ncbyte *data, const long *cur, const long *dim);

extern bool put_nc_data(NcVar *, const int    *data );
extern bool put_nc_data(NcVar *, const char   *data );
extern bool put_nc_data(NcVar *, const float  *data );
extern bool put_nc_data(NcVar *, const double *data );
extern bool put_nc_data(NcVar *, const ncbyte *data );

extern bool put_nc_data(NcVar *, const int    data, const long offset0=0, const long offset1=-1, const long c2=-1);
extern bool put_nc_data(NcVar *, const char   data, const long offset0=0, const long offset1=-1, const long c2=-1);
extern bool put_nc_data(NcVar *, const float  data, const long offset0=0, const long offset1=-1, const long c2=-1);
extern bool put_nc_data(NcVar *, const double data, const long offset0=0, const long offset1=-1, const long c2=-1);
extern bool put_nc_data(NcVar *, const ncbyte data, const long offset0=0, const long offset1=-1, const long c2=-1);

extern bool put_nc_data(NcVar *, const int    *data, const long length, const long offset);
extern bool put_nc_data(NcVar *, const char   *data, const long length, const long offset);
extern bool put_nc_data(NcVar *, const float  *data, const long length, const long offset);
extern bool put_nc_data(NcVar *, const double *data, const long length, const long offset);
extern bool put_nc_data(NcVar *, const ncbyte *data, const long length, const long offset);
extern bool put_nc_data(NcVar *, const int    *data, const long *length, const long *offset);
extern bool put_nc_data(NcVar *, const char   *data, const long *length, const long *offset);
extern bool put_nc_data(NcVar *, const float  *data, const long *length, const long *offset);

extern bool put_nc_data_with_dims(NcVar *, const int *data, const int len0,
                                  const int len1=0, const int len2=0);
extern bool put_nc_data_with_dims(NcVar *, const int *data, const long len0,
                                  const long len1=0, const long len2=0);
extern bool put_nc_data_with_dims(NcVar *, const float *data, const int len0,
                                  const int len1=0, const int len2=0);
extern bool put_nc_data_with_dims(NcVar *, const float *data, const long len0,
                                  const long len1=0, const long len2=0);
extern bool put_nc_data_with_dims(NcVar *, const double *data, const int len0,
                                  const int len1=0, const int len2=0);
extern bool put_nc_data_with_dims(NcVar *, const double *data, const long len0,
                                  const long len1=0, const long len2=0);

extern NcVar  get_var(NcFile *, const char * var_name);

extern NcVar get_nc_var(NcFile *, const char * var_name);

//extern NcVar has_var(NcFile *, const char * var_name);
extern bool has_var(NcFile *, const char * var_name);
//extern int get_var_count(NcFile *);

extern NcVar  add_var(NcFile *, const string, const NcType, const int deflate_level=DEF_DEFLATE_LEVEL);
extern NcVar  add_var(NcFile *, const string, const NcType, const NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern NcVar  add_var(NcFile *, const string, const NcType, const NcDim, const NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern NcVar  add_var(NcFile *, const string, const NcType, const NcDim, const NcDim, const NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern NcVar  add_var(NcFile *, const string, const NcType, const NcDim, const NcDim, const NcDim, const NcDim, const int deflate_level=DEF_DEFLATE_LEVEL);
extern NcVar  add_var(NcFile *, const string, const NcType, const vector<NcDim>, const int deflate_level=DEF_DEFLATE_LEVEL);

extern NcDim  add_dim(NcFile *, string);
//extern NcDim  add_dim(NcFile *, string, long);
extern NcDim  add_dim(NcFile *, string, size_t);
extern bool   has_dim(NcFile *, const char * dim_name);
extern bool   get_dim(const NcFile *, const ConcatString &, int &, bool error_out = false);
extern int    get_dim_count(const NcVar *);
extern int    get_dim_count(const NcFile *);
extern int    get_dim_size(const NcDim *);
extern int    get_dim_value(const NcFile *, const string, bool error_out = false);
extern NcDim  get_nc_dim(const NcFile *, string dim_name);
extern NcDim  get_nc_dim(const NcVar *, string dim_name);
extern NcDim  get_nc_dim(const NcVar *, int dim_offset);
//extern bool   get_global_dims(const NcFile *, int *dim_count, NcDim * dimArray[], StringArray *dimNames);
//extern bool   get_dims(const NcVar *, int *dim_count, NcDim * dimArray[], StringArray *dimNames);
extern bool   get_dim_names(const NcVar *var, StringArray *dimNames);
extern bool   get_dim_names(const NcFile *nc, StringArray *dimNames);
//extern multimap<string,NcDim> get_global_dims(const NcFile *nc, int *dim_count);

extern NcFile* open_ncfile(const char * nc_name, bool write = false);

extern int get_data_size(NcVar *);

////////////////////////////////////////////////////////////////////////

#endif   /*  __NC_UTILS_H__  */

////////////////////////////////////////////////////////////////////////
