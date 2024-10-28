// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VX_LOG_H__
#define  __VX_LOG_H__


////////////////////////////////////////////////////////////////////////

#include "concat_string.h"
#include "indent.h"
#include "string_array.h"
#include "file_fxns.h"
#include "logger.h"
#include "str_wrappers.h"

////////////////////////////////////////////////////////////////////////

inline double get_exe_duration(clock_t start_clock, clock_t end_clock) {
   return ((double)(end_clock - start_clock)) / CLOCKS_PER_SEC;
}

inline double get_exe_duration(clock_t start_clock) {
   return get_exe_duration(start_clock, clock());
}

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_LOG_H__


////////////////////////////////////////////////////////////////////////




