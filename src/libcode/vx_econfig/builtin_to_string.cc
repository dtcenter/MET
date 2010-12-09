// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //    This file is machine generated.
   //
   //    Do not edit by hand.
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "builtin_to_string.h"


////////////////////////////////////////////////////////////////////////


void builtin_to_string(const Builtin t, char * out)

{

switch ( t )  {

   case builtin_sin :      strcpy(out, "builtin_sin");      break;
   case builtin_cos :      strcpy(out, "builtin_cos");      break;
   case builtin_tan :      strcpy(out, "builtin_tan");      break;
   case builtin_sind :     strcpy(out, "builtin_sind");     break;
   case builtin_cosd :     strcpy(out, "builtin_cosd");     break;

   case builtin_tand :     strcpy(out, "builtin_tand");     break;
   case builtin_asin :     strcpy(out, "builtin_asin");     break;
   case builtin_acos :     strcpy(out, "builtin_acos");     break;
   case builtin_atan :     strcpy(out, "builtin_atan");     break;
   case builtin_asind :    strcpy(out, "builtin_asind");    break;

   case builtin_acosd :    strcpy(out, "builtin_acosd");    break;
   case builtin_atand :    strcpy(out, "builtin_atand");    break;
   case builtin_log :      strcpy(out, "builtin_log");      break;
   case builtin_exp :      strcpy(out, "builtin_exp");      break;
   case builtin_log10 :    strcpy(out, "builtin_log10");    break;

   case builtin_exp10 :    strcpy(out, "builtin_exp10");    break;
   case builtin_sqrt :     strcpy(out, "builtin_sqrt");     break;
   case builtin_abs :      strcpy(out, "builtin_abs");      break;
   case builtin_floor :    strcpy(out, "builtin_floor");    break;
   case builtin_ceil :     strcpy(out, "builtin_ceil");     break;

   case builtin_nint :     strcpy(out, "builtin_nint");     break;
   case builtin_sign :     strcpy(out, "builtin_sign");     break;
   case builtin_step :     strcpy(out, "builtin_step");     break;
   case builtin_atan2 :    strcpy(out, "builtin_atan2");    break;
   case builtin_atan2d :   strcpy(out, "builtin_atan2d");   break;

   case builtin_arg :      strcpy(out, "builtin_arg");      break;
   case builtin_argd :     strcpy(out, "builtin_argd");     break;
   case builtin_min :      strcpy(out, "builtin_min");      break;
   case builtin_max :      strcpy(out, "builtin_max");      break;
   case builtin_mod :      strcpy(out, "builtin_mod");      break;

   case no_builtin :       strcpy(out, "no_builtin");       break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


