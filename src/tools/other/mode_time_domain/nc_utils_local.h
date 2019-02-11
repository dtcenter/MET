// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_NETCDF_UTILS_H__
#define  __MTD_NETCDF_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "vx_cal.h"
#include "nc_utils.h"

////////////////////////////////////////////////////////////////////////


extern const char *  string_att           (const NcFile &, const char * name);
extern int           string_att_as_int    (const NcFile &, const char * name);
extern long long     string_att_as_ll     (const NcFile &, const char * name);
extern double        string_att_as_double (const NcFile &, const char * name);


////////////////////////////////////////////////////////////////////////


extern unixtime parse_start_time(const char *);

extern ConcatString start_time_string(const unixtime);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_NETCDF_UTILS_H__  */


////////////////////////////////////////////////////////////////////////


