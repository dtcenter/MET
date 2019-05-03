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

#include "vx_econfig/resulttype_to_string.h"


////////////////////////////////////////////////////////////////////////


void resulttype_to_string(const ResultType t, char * out)

{

switch ( t )  {

   case result_int :       strcpy(out, "result_int");       break;
   case result_double :    strcpy(out, "result_double");    break;
   case result_string :    strcpy(out, "result_string");    break;
   case no_result_type :   strcpy(out, "no_result_type");   break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


