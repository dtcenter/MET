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

#include "vx_econfig/stetype_to_string.h"


////////////////////////////////////////////////////////////////////////


void stetype_to_string(const SteType t, char * out)

{

switch ( t )  {

   case ste_integer :    strcpy(out, "ste_integer");    break;
   case ste_double :     strcpy(out, "ste_double");     break;
   case ste_variable :   strcpy(out, "ste_variable");   break;
   case ste_pwl :        strcpy(out, "ste_pwl");        break;
   case ste_function :   strcpy(out, "ste_function");   break;

   case ste_array :      strcpy(out, "ste_array");      break;
   case no_ste_type :    strcpy(out, "no_ste_type");    break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


