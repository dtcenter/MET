// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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


ConcatString afmtokentype_to_string(const AfmTokenType t)

{

ConcatString out;
  
switch ( t )  {

   case afm_token_string:      out = "afm_token_string";      break;
   case afm_token_name:        out = "afm_token_name";        break;
   case afm_token_number:      out = "afm_token_number";      break;
   case afm_token_integer:     out = "afm_token_integer";     break;
   case afm_token_boolean:     out = "afm_token_boolean";     break;

   case afm_token_keyword:     out = "afm_token_keyword";     break;
   case afm_token_endofline:   out = "afm_token_endofline";   break;
   case no_afm_token_type:     out = "no_afm_token_type";     break;

   default:
      out = "(bad value)";
      break;

}   //  switch


return ( out );

}


////////////////////////////////////////////////////////////////////////


