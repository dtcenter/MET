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

#include "afm_keywords.h"


////////////////////////////////////////////////////////////////////////


int is_afm_keyword(ConcatString text, AfmKeyword & a)

{

int j;

a = no_afm_keyword;

for (j=0; j<n_kw_infos; ++j)  {

  if ( text == kw_info[j].text )  {

      a = kw_info[j].key;

      return ( 1 );

   }

}


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


