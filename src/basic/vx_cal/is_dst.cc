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


struct DstInfo {

   int month_start;
   int day_start;

   int month_stop;
   int day_stop;

};


static DstInfo dst_info[] = {

  { 4, 2, 10, 29 },    //   0
  { 4, 1, 10, 28 },    //   1

  { 4, 7, 10, 27 },    //   2
  { 4, 6, 10, 26 },    //   3

  { 4, 5, 10, 25 },    //   4
  { 4, 4, 10, 31 },    //   5

  { 4, 3, 10, 30 },    //   6
  { 4, 1, 10, 28 },    //   7

  { 4, 7, 10, 27 },    //   8
  { 4, 6, 10, 26 },    //   9

  { 4, 5, 10, 25 },    //  10
  { 4, 4, 10, 31 },    //  11

  { 4, 3, 10, 30 },    //  12
  { 4, 2, 10, 29 }     //  13

};


////////////////////////////////////////////////////////////////////////


int is_dst(int month, int day, int year, int hour, int minute, int second)

{

int mjd0, ly, year_type;
unixtime T, dst_start, dst_stop;


ly = is_leap_year(year);

mjd0 = date_to_mjd(1, 1, year);

year_type = 7*ly + (mjd0 + 3)%7;


T = mdyhms_to_unix(month, day, year, hour, minute, second);


dst_start = mdyhms_to_unix(dst_info[year_type].month_start,
                           dst_info[year_type].day_start,
                           year, 2, 0, 0);

if ( T < dst_start )  return ( 0 );



dst_stop  = mdyhms_to_unix(dst_info[year_type].month_stop,
                           dst_info[year_type].day_stop,
                           year, 2, 0, 0);

if ( T > dst_stop )  return ( 0 );



return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_dst(unixtime T)

{

int a;
int month, day, year, hour, minute, second;

unix_to_mdyhms(T, month, day, year, hour, minute, second);

a = is_dst(month, day, year, hour, minute, second);

return ( a );

}


////////////////////////////////////////////////////////////////////////



