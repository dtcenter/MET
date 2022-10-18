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

#define SEC_MONTH (86400*30)
#define SEC_YEAR  (86400*30*12)
#define DAY_EPSILON 0.00002

unixtime add_to_unixtime(unixtime base_unixtime, int sec_per_unit,
                         double time_value, bool no_leap) {
  unixtime ut;
  int month, day, year, hour, minute, second;
  unixtime time_value_ut = (unixtime)time_value;
  double time_fraction = time_value - time_value_ut;
  const char *method_name = "add_to_unixtime() -->";

  if (sec_per_unit == SEC_MONTH || sec_per_unit == SEC_YEAR) {
    if (time_value < 0) {
      mlog << Error << "\n" << method_name
           << " the negative offset (" << time_value
           << ") is not supported for unit months and years\n\n";
      exit(-1);
    }

    unix_to_mdyhms(base_unixtime, month, day, year, hour, minute, second);

    int month_offset;
    double day_offset;
    if (sec_per_unit == SEC_YEAR) {
      year += time_value_ut;
      time_fraction *= 12;      // 12 months/year
      month_offset = (unixtime)time_fraction;
      time_fraction -= month_offset;
    }
    else month_offset = time_value_ut;

    for (int idx=0; idx<month_offset; idx++) {
      increase_one_month(year, month);
    }
    if (day == 1) {
      if (abs(time_fraction-0.5) < DAY_EPSILON) day = 15;
      else {
        day_offset = time_fraction * 30;
        day += (int)day_offset;
        if (day_offset - (int)day_offset > 0.5) day++;
      }
    }
    else {
      day_offset = time_fraction * 30;
      time_value_ut = (int)day_offset;
      day += time_value_ut;
      if (day_offset - time_value_ut > 0.5) day++;
      if (day > 30) {
         day -= 30;
         increase_one_month(year, month);
      }
    }

    int day_org = day;
    bool day_adjusted = false;
    int max_day = monthly_days[month-1];
    if (day > max_day) {
      day = max_day;
      day_adjusted = true;
      if (month == 2 && is_leap_year(year)) {
        if (day_org == 29) day_adjusted = false;
        day = 29;
      }
    }
    ut = mdyhms_to_unix(month, day, year, hour, minute, second);
    if (day_adjusted) {
      mlog << Debug(2) << method_name << "adjusted day " << day_org
           << " to " << day << " for " << year << "-" << month << "\n";
    }
  }
  else if (!no_leap || sec_per_unit != 86400) {
    // seconds, minute, hours, and day unit with leap year
    bool use_ut = true;
    // For the precision: case 1: 1.9999 to 2
    //                    case 2: 2.0001 to 2
    // Other cases are as floating number
    if ((1.0 - time_fraction) < TIME_EPSILON) time_value_ut += 1;
    else if (time_fraction > TIME_EPSILON) use_ut = false;
    if (use_ut) ut = (unixtime)(base_unixtime + sec_per_unit * time_value_ut);
    else ut = (unixtime)(base_unixtime + sec_per_unit * time_value);
  }
  else {    //  no_leap year && unit = day
    unix_to_mdyhms(base_unixtime, month, day, year, hour, minute, second);
    int day_offset = day + (int)time_value;
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
  }
  mlog << Debug(5) <<  method_name
       << unix_to_yyyymmdd_hhmmss(base_unixtime)
       << " plus " << time_value << " days = "
       << unix_to_yyyymmdd_hhmmss(ut) << "\n";
  
  return ut;
}

////////////////////////////////////////////////////////////////////////

