// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
   if(a == bad_data_int || std::isnan(a)) return 1;
   else                                   return 0;
}

inline int is_bad_data(long long a)  {
   if(a == bad_data_ll || std::isnan(a)) return 1;
   else                                  return 0;
}

inline int is_bad_data(double a) {
   if(fabs(a - bad_data_double) < default_tol || std::isnan(a)) return 1;
   else                                                         return 0;
}

inline int is_bad_data(float a) {
   if(fabs(a - bad_data_float) < default_tol || std::isnan(a)) return 1;
   else                                                        return 0;
}

inline int is_bad_data(char a) {
   return (a == bad_data_char);
}

inline int is_eq(double a, double b, double tol) {
   if(fabs(a - b) < tol) return 1;
   else                  return 0;
}

inline int is_eq(double a, double b) {
   return is_eq(a, b, default_tol);
}

inline int is_eq(double a, int b) {
   return is_eq(a, (double)b);
}

inline int is_eq(int a, double b) {
   return is_eq((double)a, b);
}

inline int is_eq(double a, unixtime b) {
   return is_eq(a, (double)b);
}

inline int is_eq(unixtime a, double b) {
   return is_eq((double)a, b);
}

inline int is_eq(float a, float b) {
   return is_eq((double)a, (double)b);
}

template <typename T>
inline int is_eq(T a, T b) {
   return (a == b);
}

////////////////////////////////////////////////////////////////////////


inline double square(double v) {
   return (is_bad_data(v) ? bad_data_double : v*v);
}

inline double square_root(double v) {
   return (is_bad_data(v) ? bad_data_double : sqrt(v));
}


////////////////////////////////////////////////////////////////////////


#endif   //  __IS_BAD_DATA_H__


////////////////////////////////////////////////////////////////////////

