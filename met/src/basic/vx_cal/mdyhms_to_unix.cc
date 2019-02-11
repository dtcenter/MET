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
#include <cmath>

#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


unixtime mdyhms_to_unix(int month, int day, int year, int hour, int minute, int second)

{

unixtime b, g, mjd, answer;


b = (month - 14)/12;

g = year + 4900 + b;

b = month - 2 - 12*b;

mjd = (1461*(g - 100))/4 + (367*b)/12 - (3*(g/100))/4 + day - 2432076;

answer = 86400*(mjd - 40587) + 3600*hour + 60*minute + second;


return ( answer );

}



////////////////////////////////////////////////////////////////////////
