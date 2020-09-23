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
#include <ctype.h>
#include <cmath>

#include "string_fxns.h"
#include "filename_suffix.h"
#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString filename_suffix(const char * filename, bool make_lowercase)

{

ConcatString suffix;
char t;

if ( !filename || !(*filename) )  return ( suffix );

const char * f = get_short_name(filename);   //  to avoid things like "./foo"

   //
   //  start at the end of the filename
   //

const char * s = f + (strlen(f) - 1);

   //
   //  move left until we see a period
   //

while ( (s >= f) && (*s != '.') )  --s;

if ( s < f )  return ( suffix );

if ( make_lowercase )  {

   while ( *s )  {
  
      t = (char) tolower(*s++);
      suffix.add( t );

   }

} else suffix = s;

   //
   //  done
   //

return ( suffix );

}


////////////////////////////////////////////////////////////////////////


