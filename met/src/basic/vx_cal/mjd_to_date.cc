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
#include <cmath>

#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


void mjd_to_date(int mjd, int & month, int & day, int & year)

{

int i, j, n, l, d, m, y;


l  = mjd + 2468570;

n  = (4*l)/146097;

l -= (146097*n + 3)/4;

i  = (4000*(l + 1))/1461001;

l += 31 - (1461*i)/4;

j  = (80*l)/2447;

d  = l - (2447*j)/80;

l  = j/11;

m  = j + 2 - 12*l;

y  = 100*(n - 49) + i + l;

month = m;
day   = d;
year  = y;

return;

}


////////////////////////////////////////////////////////////////////////



