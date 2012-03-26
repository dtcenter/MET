

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_2D_NC_PINTERP_UTILS_H__
#define  __DATA_2D_NC_PINTERP_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf.hh>

#include "long_array.h"


////////////////////////////////////////////////////////////////////////


extern int get_int_var(NcFile *, const char * var_name, int index);

extern bool args_ok(const LongArray &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __DATA_2D_NC_PINTERP_UTILS_H__  */


////////////////////////////////////////////////////////////////////////



