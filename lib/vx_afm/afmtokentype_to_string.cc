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

#include "afmtokentype_to_string.h"


////////////////////////////////////////////////////////////////////////


void afmtokentype_to_string(const AfmTokenType t, char * out)

{

switch ( t )  {

   case afm_token_string:      strcpy(out, "afm_token_string");      break;
   case afm_token_name:        strcpy(out, "afm_token_name");        break;
   case afm_token_number:      strcpy(out, "afm_token_number");      break;
   case afm_token_integer:     strcpy(out, "afm_token_integer");     break;
   case afm_token_boolean:     strcpy(out, "afm_token_boolean");     break;

   case afm_token_keyword:     strcpy(out, "afm_token_keyword");     break;
   case afm_token_endofline:   strcpy(out, "afm_token_endofline");   break;
   case no_afm_token_type:     strcpy(out, "no_afm_token_type");     break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


