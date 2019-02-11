// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __IS_BAD_DATA_H__
#define  __IS_BAD_DATA_H__


////////////////////////////////////////////////////////////////////////


#include <cmath>

#include "vx_cal.h"
#include "math_constants.h"


////////////////////////////////////////////////////////////////////////


inline int is_bad_data(int a)  {
   if(a == bad_data_int || isnan(a)) return(1);
   else                              return(0);
}

inline int is_bad_data(long long a)  {
   if(a == bad_data_ll || isnan(a)) return(1);
   else                             return(0);
}

inline int is_bad_data(double a) {
   if(fabs(a - bad_data_double) < default_tol || isnan(a)) return(1);
   else                                                    return(0);
}

inline int is_bad_data(float a) {
   if(fabs(a - bad_data_float) < default_tol || isnan(a)) return(1);
   else                                                   return(0);
}

inline int is_bad_data(char a) {
   return(a == bad_data_char);
}

inline int is_eq(double a, double b, double tol) {
   if(fabs(a - b) < tol) return(1);
   else                  return(0);
}

inline int is_eq(double a, double b) {
   return(is_eq(a, b, default_tol));
}

////////////////////////////////////////////////////////////////////////


#endif   //  __IS_BAD_DATA_H__


////////////////////////////////////////////////////////////////////////

