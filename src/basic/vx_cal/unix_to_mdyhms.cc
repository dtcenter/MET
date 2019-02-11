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


const char * day_name[] = {

   "Sunday",    //  0 - not used

   "Monday",    //  1
   "Tuesday",   //  2
   "Wednesday", //  3
   "Thursday",  //  4
   "Friday",    //  5
   "Saturday",  //  6
   "Sunday"     //  7

 };


const char * short_day_name[] = {

   "Sun",   //  0 - not used

   "Mon",   //  1
   "Tue",   //  2
   "Wed",   //  3
   "Thu",   //  4
   "Fri",   //  5
   "Sat",   //  6
   "Sun"    //  7

 };

const char * month_name[] = {

   "December",      //   0 - not used

   "January",       //   1
   "February",      //   2
   "March",         //   3
   "April",         //   4
   "May",           //   5
   "June",          //   6
   "July",          //   7
   "August",        //   8
   "September",     //   9
   "October",       //  10
   "November",      //  11
   "December"       //  12

};


const char * short_month_name[] = {

   "Dec",    //   0 - not used

   "Jan",    //   1
   "Feb",    //   2
   "Mar",    //   3
   "Apr",    //   4
   "May",    //   5
   "Jun",    //   6
   "Jul",    //   7
   "Aug",    //   8
   "Sep",    //   9
   "Oct",    //  10
   "Nov",    //  11
   "Dec"     //  12

};


////////////////////////////////////////////////////////////////////////


void unix_to_mdyhms(unixtime u, int & month, int & day, int & year, int & hour, int & minute, int & second)

{

unixtime i, j, n, l, d, m, y;


n = u/86400;

u -= 86400*n;

if ( u < 0 )  { u += 86400; --n; }

l  = n + 2509157;

n  = (4*l)/146097;

l -= (146097*n + 3)/4;

i  = (4000*(l + 1))/1461001;

l += 31 - (1461*i)/4;

j  = (80*l)/2447;

d  = l - (2447*j)/80;

l  = j/11;

m  = j + 2 - 12*l;

y  = 100*(n - 49) + i + l;

month   = m;
day     = d;
year    = y;

hour    =      u/3600;
minute  = (u%3600)/60;
second  =        u%60;

return;

}


////////////////////////////////////////////////////////////////////////
