// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "ordinal.h"
#include "vx_log.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


static const char th [] = "th";
static const char rd [] = "rd";
static const char nd [] = "nd";
static const char st [] = "st";


////////////////////////////////////////////////////////////////////////


void ordinal_suffix(int n, char * out)

{

const char *method_name = "ordinal_suffix() -> ";

   //
   //  suffix rule is the same for both
   //   negative numbers and positive numbers
   //

if ( n < 0 )  n = -n;

   //
   //  only depends on the last two digits
   //

n %= 100;

   //
   //  handle zero as a special case
   //

if ( n == 0 )   { m_strcpy(out, th, method_name, "out1");  return; }

   //
   //  "teen" numbers are an exception
   //

if ( (n >= 10) && (n <= 20) )   { m_strcpy(out, th, method_name, "out1");  return; }

   //
   //  if we get to this point, we only need to look
   //    at the last digit
   //

n %= 10;

const char * ans = (const char *) 0;

switch ( n )  {

   case 0:  ans = th;   break;
   case 1:  ans = st;   break;
   case 2:  ans = nd;   break;
   case 3:  ans = rd;   break;
   case 4:  ans = th;   break;

   case 5:  ans = th;   break;
   case 6:  ans = th;   break;
   case 7:  ans = th;   break;
   case 8:  ans = th;   break;
   case 9:  ans = th;   break;

   default:
         //
         //  should never happen
         //
      mlog << Error << "\n" << method_name << "totally confused!\n\n";
      exit ( 1 );

}   //  switch


m_strcpy(out, ans, method_name, "out3");


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


