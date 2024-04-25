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
#include <cmath>

#include "vx_cal.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


unixtime mdyhms_to_unix(int month, int day, int year, int hour, int minute, int second)

{

unixtime b;
unixtime g;
unixtime mjd;
unixtime answer;


b = (month - 14)/12;

g = year + 4900 + b;

b = month - 2 - 12*b;

mjd = (1461*(g - 100))/4 + (367*b)/12 - (3*(g/100))/4 + day - 2432076;

answer = 86400*(mjd - 40587) + 3600*hour + 60*minute + second;


return answer;

}



////////////////////////////////////////////////////////////////////////
