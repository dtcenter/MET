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
#include <ctype.h>
#include <cmath>

#include "roman_numeral.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const char rn_1000     = 'M';
static const char rn_500      = 'D';
static const char rn_100      = 'C';
static const char rn_50       = 'L';
static const char rn_10       = 'X';
static const char rn_5        = 'V';
static const char rn_1        = 'I';


////////////////////////////////////////////////////////////////////////


static void rn_add(char * & c, int & n, const int modulus, const char ones, const char fives, const char tens);


////////////////////////////////////////////////////////////////////////


void roman_numeral(int n, char * out, const int lower_case_flag)

{

   //
   //  range check
   //

if ( (n < roman_numeral_min) || (n > roman_numeral_max) )  {

   mlog << Error << "\nroman_numeral() -> range check error!\n\n";

   exit ( 1 );

}

char s[128];   //  should be plenty large enough
char * c = s;

memset(s, 0, sizeof(s));

   //
   //  handle thousands
   //

while ( n >= 1000 )  {

   *c++ = rn_1000;

   n -= 1000;

}

   //
   //  now n < 1000
   //

rn_add(c, n, 100, rn_100, rn_500, rn_1000);
rn_add(c, n,  10,  rn_10,  rn_50,  rn_100);
rn_add(c, n,   1,   rn_1,   rn_5,   rn_10);


   //
   //  convert to lower case, if needed
   //

if ( lower_case_flag )  {

   int j;
   const int k = strlen(s);

   for (j=0; j<k; ++j)  {

      s[j] = (char) tolower(s[j]);

   }

}


   //
   //  done
   //

strcpy(out, s);

return;

}


////////////////////////////////////////////////////////////////////////


void rn_add(char * & c, int & n, const int modulus, const char ones, const char fives, const char tens)

{

switch ( n/modulus )  {

   case 0:
      break;

   case 1:
      *c++ = ones;
      break;

   case 2:
      *c++ = ones;
      *c++ = ones;
      break;

   case 3:
      *c++ = ones;
      *c++ = ones;
      *c++ = ones;
      break;

   case 4:
      *c++ = ones;
      *c++ = fives;
      break;

   case 5:
      *c++ = fives;
      break;

   case 6:
      *c++ = fives;
      *c++ = ones;
      break;

   case 7:
      *c++ = fives;
      *c++ = ones;
      *c++ = ones;
      break;

   case 8:
      *c++ = fives;
      *c++ = ones;
      *c++ = ones;
      *c++ = ones;
      break;

   case 9:
      *c++ = ones;
      *c++ = tens;
      break;

   default:  //  shouldn't ever happen
      mlog << Error << "\nrn_add() -> can't handle integer " << n << "\n";
      exit ( 1 );
      break;

}   // switch




   //
   //  done
   //

n %= modulus;

return;

}


////////////////////////////////////////////////////////////////////////


