

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __BUILTIN_H__
#define  __BUILTIN_H__

#include "concat_string.h"

////////////////////////////////////////////////////////////////////////


static const int max_builtin_args = 2;


////////////////////////////////////////////////////////////////////////


enum Builtin {


      //
      //  built-in functions of one variable
      //

   builtin_sin,
   builtin_cos,
   builtin_tan,

   builtin_sind,
   builtin_cosd,
   builtin_tand,

   builtin_asin,
   builtin_acos,
   builtin_atan,

   builtin_asind,
   builtin_acosd,
   builtin_atand,

   builtin_log,
   builtin_exp,

   builtin_log10,
   builtin_exp10,

   builtin_sqrt,
   builtin_abs,
   builtin_floor,
   builtin_ceil,
   builtin_nint,
   builtin_sign,

   builtin_step,

   // Functions defined in ConfigConstants
   // builtin_F_to_C,
   // builtin_C_to_F,


      //
      //  built-in functions of two variables
      //

   builtin_atan2,
   builtin_atan2d,

   builtin_arg,
   builtin_argd,

   builtin_min,
   builtin_max,

   builtin_mod,

      //
      //  built-in functions of three variables
      //

   // builtin_ifte

      //
      //  flag value
      //

   no_builtin

};


////////////////////////////////////////////////////////////////////////


typedef int (*ifunc_1) (int);
typedef int (*ifunc_2) (int, int);

typedef double (*dfunc_1) (double);
typedef double (*dfunc_2) (double, double);


////////////////////////////////////////////////////////////////////////


struct BuiltinInfo {

   const char * name;

   int n_args;   //  1 or 2

   Builtin id;

   ifunc_1 i1;
   ifunc_2 i2;

   dfunc_1 d1;
   dfunc_2 d2;

};


/*
class BuiltinInfo {

   private:

      init_from_scratch();

   public:

      BuiltinInfo();
     ~BuiltinInfo();
      BuiltinInfo(const BuiltinInfo &);
      BuiltinInfo & operator=(const BuiltinInfo &);

      ifunc_1 i1;
      ifunc_2 i2;

      dfunc_1 d1;
      dfunc_2 d2;

      char * name;

      int n_vars;   //  1 or 2

      int call_ifunc1(int);
      int call_ifunc2(int, int);

      double call_dfunc1(double);
      double call_dfunc2(double, double);


};
*/

////////////////////////////////////////////////////////////////////////


extern const BuiltinInfo binfo [];

extern const int n_binfos;


////////////////////////////////////////////////////////////////////////


extern int my_nint(double);

extern int my_isign(int);
extern int my_dsign(double);


////////////////////////////////////////////////////////////////////////


extern bool is_builtin(const ConcatString name, int & index);


////////////////////////////////////////////////////////////////////////


#endif   /*  __BUILTIN_H__  */


////////////////////////////////////////////////////////////////////////





