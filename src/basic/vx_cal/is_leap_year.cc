// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"


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

void decrease_one_month(int &year, int &month) {
  month--;
  if (month < 1) {
    month = 12;
    year--;
  }
}

////////////////////////////////////////////////////////////////////////

void increase_one_month(int &year, int &month) {
  month++;
  if (month > 12) {
    month = 1;
    year++;
  }
}

////////////////////////////////////////////////////////////////////////

unixtime add_to_unixtime(unixtime base_unixtime,
    int sec_per_unit, double time_value, bool no_leap) {
  unixtime ut;
  unixtime time_value_ut = (unixtime)time_value;
  double time_fraction = time_value - time_value_ut;
  if (!no_leap || sec_per_unit != 86400) {
    bool use_ut = true;
    if ((1.0 - time_fraction) < TIME_EPSILON) time_value_ut += 1;
    else if (time_fraction > TIME_EPSILON) use_ut = false;
    if (use_ut) ut = (unixtime)(base_unixtime + sec_per_unit * time_value_ut);
    else ut = (unixtime)(base_unixtime + sec_per_unit * time_value);
  }
  else {
    int day_offset;
    int month, day, year, hour, minute, second;
    
    unix_to_mdyhms(base_unixtime, month, day, year, hour, minute, second);
    day_offset = day + (int)time_value;
    if (day_offset < 0) {
      while (day_offset < 0) {
        decrease_one_month(year, month);
        day_offset += monthly_days[month-1];
      }
    }
    else {
      while (day_offset > monthly_days[month-1]) {
        day_offset -= monthly_days[month-1];
        increase_one_month(year, month);
      }
    }
    day = day_offset;
    if (day == 0) day = 1;
    ut = mdyhms_to_unix(month, day, year, hour, minute, second);
    if (time_fraction > (1-TIME_EPSILON) ) ut += sec_per_unit;
    else if (time_fraction > TIME_EPSILON) ut += (time_fraction * sec_per_unit);
    mlog << Debug(5) << "add_to_unixtime() -> "
         << unix_to_yyyymmdd_hhmmss(base_unixtime)
         << " plus " << time_value << " days = "
         << unix_to_yyyymmdd_hhmmss(ut) << "\n";
  }
  
  return ut;
}

////////////////////////////////////////////////////////////////////////

