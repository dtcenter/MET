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

#include "fix_float.h"


////////////////////////////////////////////////////////////////////////


void fix_float(ConcatString &s)

{

fix_float_with_char(s, (char) 0);

return;

}


////////////////////////////////////////////////////////////////////////


void fix_float_with_blanks(ConcatString &s)

{

fix_float_with_char(s, ' ');

return;

}


////////////////////////////////////////////////////////////////////////


void fix_float_with_char(ConcatString &s, const char replacement)

{

   //
   //  test for "-0" or "+0"
   //

  if (  s == (string)"-0" || s == (string)"+0" )  {

   s = "0";

}

   //
   //  no decimal point? ... just return
   //

if ( s.find('.') == -1 )  return;

   //
   //  scientific notation? ... just return
   //

if ( s.find('e') != -1 || s.find('E') != -1 )  return;

   //
   //  get to work
   //

int j = s.length() - 1;

while ( j >= 0 )  {

  if ( s[j] == '.' )  { s.replace_char(j, replacement);  break; }

  if ( s[j] == '0' )  { s.replace_char(j--,replacement);  continue; }

   break;

}

   //
   //  test again for "-0" or "+0"
   //

if ( s == (string)"-0" || s == (string)"+0" )  {

   s = "0";

}

   //
   //  store result of c_str() to delete trailing nulls
   //

s = s.c_str();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


