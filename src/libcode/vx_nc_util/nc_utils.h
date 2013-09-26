

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2013
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

extern bool get_var_units(const NcVar *, ConcatString &);

extern bool get_var_level(const NcVar *, ConcatString &);

extern bool args_ok(const LongArray &);

extern int  get_int_var(NcFile *, const char * var_name, int index);

extern double  get_double_var(NcFile *, const char * var_name, int index = 0);

extern NcVar* has_var(NcFile *, const char * var_name);

////////////////////////////////////////////////////////////////////////

#endif   /*  __NC_UTILS_H__  */

////////////////////////////////////////////////////////////////////////
