// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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
   //     Created by enum_to_string from file "token.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "tokentype_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString tokentype_to_string(const TokenType t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case tok_union:          s = "tok_union";          break;
   case tok_intersection:   s = "tok_intersection";   break;
   case tok_negation:       s = "tok_negation";       break;
   case tok_local_var:      s = "tok_local_var";      break;
   case tok_mark:           s = "tok_mark";           break;

   case tok_unmark:         s = "tok_unmark";         break;
   case tok_eof:            s = "tok_eof";            break;
   case no_token_type:      s = "no_token_type";      break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


bool string_to_tokentype(const char * text, TokenType & t)

{

     if ( strcmp(text, "tok_union"       ) == 0 )   { t = tok_union;          return ( true ); }
else if ( strcmp(text, "tok_intersection") == 0 )   { t = tok_intersection;   return ( true ); }
else if ( strcmp(text, "tok_negation"    ) == 0 )   { t = tok_negation;       return ( true ); }
else if ( strcmp(text, "tok_local_var"   ) == 0 )   { t = tok_local_var;      return ( true ); }
else if ( strcmp(text, "tok_mark"        ) == 0 )   { t = tok_mark;           return ( true ); }

else if ( strcmp(text, "tok_unmark"      ) == 0 )   { t = tok_unmark;         return ( true ); }
else if ( strcmp(text, "tok_eof"         ) == 0 )   { t = tok_eof;            return ( true ); }
else if ( strcmp(text, "no_token_type"   ) == 0 )   { t = no_token_type;      return ( true ); }
   //
   //  nope
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////


