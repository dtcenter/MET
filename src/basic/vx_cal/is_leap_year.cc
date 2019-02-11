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


int is_leap_year(int year)

{

year = abs(year);


if ( year%4   )  return ( 0 );

if ( year%100 )  return ( 1 );

if ( year%400 )  return ( 0 );


return ( 1 );

}


////////////////////////////////////////////////////////////////////////

const int monthly_days[12] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

////////////////////////////////////////////////////////////////////////

int get_days_from_mmdd(int month, int day, bool no_leap) {
  int days = day;
  int idx_month = month - 1;
  
  if (idx_month > 0) {
    for (int idx=0; idx<idx_month; idx++)
      days += monthly_days[idx];
    
    if ( !no_leap && month > 2) days++;
  }
  
  return days;
}

////////////////////////////////////////////////////////////////////////

unixtime add_to_unixtime(unixtime base_unixtime,
    int sec_per_unit, double time_value, bool no_leap) {
  unixtime ut;
  if (!no_leap || sec_per_unit != 86400) {
    ut = (unixtime)(base_unixtime + sec_per_unit * time_value);
  }
  else {
    int day_offset;
    int month, day, year, hour, minute, second;
    
    unix_to_mdyhms(base_unixtime, month, day, year, hour, minute, second);
    day_offset  = get_days_from_mmdd(month, day, no_leap) + (int)time_value;
    year       += (int)(day_offset / 365);
    day_offset += day_offset % 365;
    for (int idx = 0; idx<12; idx++) {
      if (day_offset < monthly_days[idx]) {
        month = (idx + 1);
        day = day_offset;
        break;
      }
      else {
        day_offset -= monthly_days[idx];
      }
    }
    ut = mdyhms_to_unix(month, day, year, hour, minute, second);
  }
  
  return ut;
}

////////////////////////////////////////////////////////////////////////




