// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //     Created by enum_to_string from file "afm_token_types.h"
   //


////////////////////////////////////////////////////////////////////////


#include <string.h>

#include "afmtokentype_to_string.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


ConcatString afmtokentype_to_string(const AfmTokenType t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case afm_token_string:      s = "afm_token_string";      break;
   case afm_token_name:        s = "afm_token_name";        break;
   case afm_token_number:      s = "afm_token_number";      break;
   case afm_token_integer:     s = "afm_token_integer";     break;
   case afm_token_boolean:     s = "afm_token_boolean";     break;

   case afm_token_keyword:     s = "afm_token_keyword";     break;
   case afm_token_endofline:   s = "afm_token_endofline";   break;
   case no_afm_token_type:     s = "no_afm_token_type";     break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


