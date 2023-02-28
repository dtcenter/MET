// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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


int unix_to_day_of_year(unixtime u) {

int mon, day, yr, hr, min, sec;

unix_to_mdyhms(u, mon, day, yr, hr, min, sec);

int sec_of_year = mdyhms_to_unix(mon, day, 1970, 0, 0, 0);

int sec_per_day = 60 * 60 * 24;

return ( sec_of_year / sec_per_day );

}


////////////////////////////////////////////////////////////////////////


long unix_to_long_yyyymmddhh(unixtime u) {

int mon, day, yr, hr, min, sec;
long yyyymmddhh;

unix_to_mdyhms(u, mon, day, yr, hr, min, sec);

yyyymmddhh = 1000000 * yr + 10000 * mon + 100 * day + hr;

return ( yyyymmddhh );

}


////////////////////////////////////////////////////////////////////////


int sec_of_day_diff(unixtime ut1, unixtime ut2) {

int sec_per_day = 60 * 60 * 24;

int s1 = unix_to_sec_of_day(ut1);

int s2 = unix_to_sec_of_day(ut2);

int dt = s2 - s1;

     if ( dt < -1 * sec_per_day/2 ) dt += sec_per_day;
else if ( dt >      sec_per_day/2 ) dt -= sec_per_day;

return ( dt );

}


////////////////////////////////////////////////////////////////////////


int day_of_year_diff(unixtime ut1, unixtime ut2) {


int sec_per_day  = 60 * 60 * 24;
int day_per_year = 365;

int d1 = unix_to_day_of_year(ut1);

int d2 = unix_to_day_of_year(ut2);

int dt = d2 - d1;

     if ( dt < -1 * day_per_year/2.0 ) dt += day_per_year;
else if ( dt >      day_per_year/2.0 ) dt -= day_per_year;

return ( dt );

}


////////////////////////////////////////////////////////////////////////

