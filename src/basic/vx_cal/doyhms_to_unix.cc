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


unixtime doyhms_to_unix(int doy, int year, int hour, int minute, int second)

{

unixtime answer;
int mjd, month, day;


mjd = date_to_mjd(1, 0, year) + doy;

mjd_to_date(mjd, month, day, year);

answer = mdyhms_to_unix(month, day, year, hour, minute, second);


return ( answer );

}


////////////////////////////////////////////////////////////////////////


void sec_to_hms(int in_sec, int & hour, int & minute, int & second)

{

if ( in_sec == bad_data_int )  {
   hour = minute = second = bad_data_int;
}
else  {
   hour   = (in_sec/3600);
   minute = (in_sec%3600)/60;
   second = (in_sec%3600)%60;
}

return;
   
}


////////////////////////////////////////////////////////////////////////


int hms_to_sec(int hour, int minute, int second)

{

int out_sec;
   
out_sec = hour*3600 + minute*60 + second;
   
return ( out_sec );

}


////////////////////////////////////////////////////////////////////////


int unix_to_sec_of_day(unixtime u) {

int s, mon, day, yr, hr, min, sec;

unix_to_mdyhms(u, mon, day, yr, hr, min, sec);

s = hms_to_sec(hr, min, sec);

return ( s );

}


////////////////////////////////////////////////////////////////////////
