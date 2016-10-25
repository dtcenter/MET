// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __NC_UTILS_H__
#define  __NC_UTILS_H__

////////////////////////////////////////////////////////////////////////

#include <netcdf.hh>

#include "concat_string.h"
#include "long_array.h"

////////////////////////////////////////////////////////////////////////

extern bool get_var_att(const NcVar *, const ConcatString &, ConcatString &);

extern bool get_var_att(const NcVar *, const ConcatString &, int &, bool exit_on_error = true);

extern bool get_var_att(const NcVar *, const ConcatString &, float &d, bool exit_on_error = true);

extern bool get_var_att_double(const NcVar *, const ConcatString &, double &);

extern bool get_var_units(const NcVar *, ConcatString &);

extern bool get_var_level(const NcVar *, ConcatString &);

extern double get_var_missing_value(const NcVar *);

extern double get_var_fill_value(const NcVar *);

extern bool args_ok(const LongArray &);

extern char get_char_val(NcFile *, const char * var_name, const int index);

extern char get_char_val(NcVar *var, const int index);

extern int get_int_var(NcFile *, const char * var_name, const int index);

extern int get_int_var(NcVar *, const int index);

extern double get_double_var(NcFile *, const char * var_name, const int index = 0);

extern double get_double_var(NcVar *, int index = 0);

extern float get_float_var(NcFile *, const char * var_name, const int index = 0);

extern float get_float_var(NcVar *, int const index = 0);

extern ConcatString* get_string_val(NcFile *, const char * var_name, const int index,
                    const int len, ConcatString &tmp_cs);

extern ConcatString* get_string_val(NcVar *var, const int index, const int len, ConcatString &tmp_cs);


extern bool get_nc_data(NcVar *, const long *cur, const long *dim, int *data);
extern bool get_nc_data(NcVar *, const long *cur, const long *dim, char *data);
extern bool get_nc_data(NcVar *, const long *cur, const long *dim, float *data);
extern bool get_nc_data(NcVar *, const long *cur, const long *dim, double *data);
extern bool get_nc_data(NcVar *, const long *cur, const long *dim, ncbyte *data);
extern bool get_nc_data(NcFile *, const char *var_name, const long *cur, const long *dim, int *data);
extern bool get_nc_data(NcFile *, const char *var_name, const long *cur, const long *dim, char *data);
extern bool get_nc_data(NcFile *, const char *var_name, const long *cur, const long *dim, float *data);
extern bool get_nc_data(NcFile *, const char *var_name, const long *cur, const long *dim, double *data);
extern bool get_nc_data(NcFile *, const char *var_name, const long *cur, const long *dim, ncbyte *data);

extern NcVar* get_nc_var(NcFile *, const char * var_name);

extern NcVar* has_var(NcFile *, const char * var_name);

extern NcDim* has_dim(NcFile *, const char * dim_name);

extern bool get_global_att(const NcFile *, const ConcatString &, ConcatString &, bool error_out = false);

extern bool get_global_att_double(const NcFile *, const ConcatString &, double &, bool error_out = false);

extern bool get_dim(const NcFile *, const ConcatString &, int &, bool error_out = false);

extern NcFile* open_ncfile(const char * nc_name, NcFile::FileMode file_mode = NcFile::ReadOnly);

////////////////////////////////////////////////////////////////////////

#endif   /*  __NC_UTILS_H__  */

////////////////////////////////////////////////////////////////////////
