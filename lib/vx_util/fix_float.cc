

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util/fix_float.h"


////////////////////////////////////////////////////////////////////////


void fix_float(char * s)

{

fix_float_with_char(s, (char) 0);

return;

}


////////////////////////////////////////////////////////////////////////


void fix_float_with_blanks(char * s)

{

fix_float_with_char(s, ' ');

return;

}


////////////////////////////////////////////////////////////////////////


void fix_float_with_char(char * s, const char replacement)

{

   //
   //  test for "-0" or "+0"
   //

if ( (strcmp(s, "-0") == 0) || (strcmp(s, "+0") == 0) )  {

   s[0] = '0';

   s[1] = (char) 0;

}

   //
   //  no decimal point? ... just return
   //

if ( strchr(s, '.') == NULL )  return;

   //
   //  get to work
   //

int j = strlen(s) - 1;

while ( j >= 0 )  {

   if ( s[j] == '.' )  { s[j]   = replacement;  break; }

   if ( s[j] == '0' )  { s[j--] = replacement;  continue; }

   break;

}

   //
   //  test again for "-0" or "+0"
   //

if ( (strcmp(s, "-0") == 0) || (strcmp(s, "+0") == 0) )  {

   s[0] = '0';

   s[1] = (char) 0;

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


