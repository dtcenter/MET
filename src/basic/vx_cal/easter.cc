// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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


void easter(int & month, int & day, int year)

{

int c, d, i, j, k, l, m, n, y;


y  = year;

c  = y/100;

n  = y - 19*(y/19);

k  = (c - 17)/25;

i  = c - c/4 - (c - k)/3 + 19*n + 15;

i -= 30*(i/30);

i -= (i/28)*(1 - (i/28)*(29/(i + 1))*((21 - n)/11));

j  = y + y/4 + i + 2 - c + c/4;

j -= 7*(j/7);

l  = i - j;

m  = 3 + (l + 40)/44;

d  = l + 28 - 31*(m/4);

month = m;
day   = d;

return;

}


////////////////////////////////////////////////////////////////////////



