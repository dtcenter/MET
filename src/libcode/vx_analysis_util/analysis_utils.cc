// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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
