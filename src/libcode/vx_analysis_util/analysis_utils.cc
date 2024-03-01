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

#include "analysis_utils.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


StringArray parse_line(const char * line)

{

StringArray a;
const char * delim = " ";
char * c  = (char *) nullptr;
char * L  = (char *) nullptr;
char *temp_ptr = (char *) nullptr;
const char *method_name = "parse_line() -> ";
   //
   //  copy the line
   //

L = m_strcpy2(line, method_name);

   //
   //  tokenize the line
   //

while((c = strtok_r(L, delim, &temp_ptr)) != nullptr) {

   a.add(c);

}


   //
   //  done
   //

if ( L )  { delete [] L;  L = (char *) nullptr; }

return a;

}


////////////////////////////////////////////////////////////////////////


int all_digits(const char * line)

{

int j, n;

n = m_strlen(line);

for (j=0; j<n; ++j)  {

   if ( !(isdigit(line[j])) )  return 0;

}


return 1;

}


////////////////////////////////////////////////////////////////////////
