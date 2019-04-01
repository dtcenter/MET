// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "comma_string.h"


////////////////////////////////////////////////////////////////////////


void comma_string(long long i, ConcatString output)

{

// mlog << Debug(1) << "I = " << i << "\n";

if ( i == 0 )  { output = "0";  return; }

int d_count =  0;
int minus   =  0;
static char s[256];
int s_count = sizeof(s) - 1;

if ( i < 0 )  { i = -i;  minus = 1; }

s[s_count--] = (char) 0;

do {

   s[s_count--] = (char) ((i%10) + '0');

   ++d_count;

   i /= 10;

   if ( i && (d_count%3 == 0) )  s[s_count--] = ',';

}  while ( i );

if ( minus )  s[s_count--] = '-';

++s_count;

output = s + s_count;

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString comma_string(long long i)

{

ConcatString str;

comma_string(i, str);

return ( str );

}


////////////////////////////////////////////////////////////////////////


