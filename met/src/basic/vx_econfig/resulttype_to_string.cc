

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //
   //     Created by enum_to_string from file "result.h"
   //
   //     on December 14, 2010   8:25 am MST
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "resulttype_to_string.h"


////////////////////////////////////////////////////////////////////////


void resulttype_to_string(const ResultType t, char * out)

{

switch ( t )  {

   case result_int:       strcpy(out, "result_int");       break;
   case result_boolean:   strcpy(out, "result_boolean");   break;
   case result_double:    strcpy(out, "result_double");    break;
   case result_string:    strcpy(out, "result_string");    break;
   case no_result_type:   strcpy(out, "no_result_type");   break;


   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


