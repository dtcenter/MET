// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "sat_utils.h"


////////////////////////////////////////////////////////////////////////


void parse_csl(void * line, StringArray & a)

{

ConcatString s;
const char * c = (const char *) line;


a.clear();

while ( *c )  {

   if ( *c == ',' )  {

      a.add(s);

      s.erase();

   } else {

      s << (*c);

   }

   ++c;

}   //  while

   //
   //  anything left over?
   //

if ( s.length() > 0 )  a.add(s);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString numbertype_to_string(const int nt)

{

ConcatString s;

switch ( nt )  {

   case nt_none_query:      s = "NONE/QUERY";      break;
   case nt_version:         s = "VERSION";         break;


   case  nt_char_8:         s = "CHAR8";           break;
   case nt_uchar_8:         s = "UCHAR8";          break;

   case nt_char16_uchar16:  s = "CHAR16/UCHAR16";  break;


   case nt_int_8:           s = "INT8";            break;
   case nt_uint_8:          s = "UINT8";           break;

   case  nt_int_16:         s = "INT16";           break;
   case nt_uint_16:         s = "UINT16";          break;

   case  nt_int_32:         s = "INT32";           break;
   case nt_uint_32:         s = "UINT32";          break;

   case  nt_int_64:         s = "INT64";           break;
   case nt_uint_64:         s = "UINT64";          break;

   case  nt_int_128:        s = "INT128";          break;
   case nt_uint_128:        s = "UINT128";         break;


   case nt_float_32:        s = "FLOAT32";         break;
   case nt_float_64:        s = "FLOAT64";         break;
   case nt_float_128:       s = "FLOAT128";        break;


   default:
      s = "???";
      break;

}   //  switch

return ( s );

}


////////////////////////////////////////////////////////////////////////



