// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

#include "analysis_utils.h"


////////////////////////////////////////////////////////////////////////


StringArray parse_line(const char * line)

{

StringArray a;
const char * delim = " ";
char * c  = (char *) 0;
char * lp = (char *) 0;
char * L  = (char *) 0;
const char *method_name = "parse_line() -> ";
   //
   //  copy the line
   //

L = m_strcpy2(line, method_name);

   //
   //  tokenize the line
   //

lp = L;

while ( 1 )  {

   c = strtok(lp, delim);

   if ( !c )  break;

   a.add(c);

   lp = (char *) 0;

}


   //
   //  done
   //

if ( L )  { delete [] L;  L = (char *) 0; }

return ( a );

}


////////////////////////////////////////////////////////////////////////


int all_digits(const char * line)

{

int j, n;

n = m_strlen(line);

for (j=0; j<n; ++j)  {

   if ( !(isdigit(line[j])) )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////
