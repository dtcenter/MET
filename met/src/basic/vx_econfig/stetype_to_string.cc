

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //
   //     Created by enum_to_string from file "symtab.h"
   //
   //     on December 14, 2010   8:25 am MST
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "stetype_to_string.h"


////////////////////////////////////////////////////////////////////////


void stetype_to_string(const SteType t, char * out)

{

switch ( t )  {

   case ste_integer:    strcpy(out, "ste_integer");    break;
   case ste_double:     strcpy(out, "ste_double");     break;
   case ste_variable:   strcpy(out, "ste_variable");   break;
   case ste_pwl:        strcpy(out, "ste_pwl");        break;
   case ste_function:   strcpy(out, "ste_function");   break;

   case ste_array:      strcpy(out, "ste_array");      break;
   case no_ste_type:    strcpy(out, "no_ste_type");    break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


