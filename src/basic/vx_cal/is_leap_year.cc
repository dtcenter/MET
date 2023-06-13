// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"


using namespace std;


////////////////////////////////////////////////////////////////////////

static constexpr int DAYS_PER_MONTH = 30;
static constexpr int NO_OF_MONTHS   = 12;
static constexpr double DAY_EPSILON = 0.00002;
#define SEC_MONTH (86400*DAYS_PER_MONTH)
#define SEC_YEAR  (86400*DAYS_PER_MONTH*NO_OF_MONTHS)

////////////////////////////////////////////////////////////////////////


int is_leap_year(int year)

{

year = abs(year);


if ( year%4   )  return 0;

if ( year%100 )  return 1;

if ( year%400 )  return 0;


return 1;

}


////////////////////////////////////////////////////////////////////////

const int monthly_days[NO_OF_MONTHS] = {
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
    month = NO_OF_MONTHS;
    year--;
  }
}

////////////////////////////////////////////////////////////////////////

void increase_one_month(int &year, int &month) {
  month++;
  if (month > NO_OF_MONTHS) {
    month = 1;
    year++;
  }
}

////////////////////////////////////////////////////////////////////////

void adjuste_day_for_month_year_units(int &day, int &month, int &year, double month_fraction) {
  // Compute remaining days from the month fraction
  bool day_adjusted = false;
  const double day_offset = month_fraction * DAYS_PER_MONTH;
  const char *method_name = "adjuste_day() --> ";

  day += (int)(day_offset + 0.5);
  if (day == 1 && abs(month_fraction-0.5) < DAY_EPSILON) {
    day = 15;   // half month from the first day is 15, not 16
  }
  else if (day > DAYS_PER_MONTH) {
    day -= DAYS_PER_MONTH;
    increase_one_month(year, month);
  }

  int day_org = day;
  int max_day = monthly_days[month-1];
  if (day > max_day) {
    day = max_day;
    if (month == 2 && is_leap_year(year)) day = 29;
    day_adjusted = (day_org != day);
  }

  if (day_adjusted) {
    mlog << Debug(2) << method_name << "adjusted day " << day_org
         << " to " << day << " for " << year << "-" << month << "\n";
  }

}

////////////////////////////////////////////////////////////////////////

void adjuste_day_for_day_units(int &day, int &month, int &year, double time_value) {
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

}

////////////////////////////////////////////////////////////////////////

unixtime add_to_unixtime(unixtime base_unixtime, int sec_per_unit,
                         double time_value, bool no_leap) {
  int month;
  int day;
  int year;
  int hour;
  int minute;
  int second;
  unixtime ut;
  auto time_value_ut = (unixtime)time_value;
  double time_fraction = time_value - (double)time_value_ut;
  const char *method_name = "add_to_unixtime() -->";

  if (sec_per_unit == SEC_MONTH || sec_per_unit == SEC_YEAR) {
    if (time_value < 0) {
      mlog << Error << "\n" << method_name
           << " the negative offset (" << time_value
           << ") is not supported for unit months and years\n\n";
      exit(-1);
    }

    unix_to_mdyhms(base_unixtime, month, day, year, hour, minute, second);

    // Update year and compute remaining months
    int month_offset;
    if (sec_per_unit == SEC_YEAR) {
      year += time_value_ut;
      time_fraction *= NO_OF_MONTHS;    // 12 months/year
      month_offset = (int)time_fraction;
      time_fraction -= month_offset;
    }
    else month_offset = (int)time_value_ut;

    // Update month
    for (int idx=0; idx<month_offset; idx++) {
      increase_one_month(year, month);
    }
    // Compute remaining days
    adjuste_day_for_month_year_units(day, month, year, time_fraction);
    ut = mdyhms_to_unix(month, day, year, hour, minute, second);
  }
  else if (!no_leap || sec_per_unit != 86400) {
    // seconds, minute, hours, and day unit with leap year
    bool use_ut = true;
    // For the precision: case 1: 1.9999 to 2
    //                    case 2: 2.0001 to 2
    // Other cases are as floating number
    if ((1.0 - time_fraction) < TIME_EPSILON) time_value_ut += 1;
    else if (time_fraction > TIME_EPSILON) use_ut = false;
    if (use_ut) ut = base_unixtime + sec_per_unit * time_value_ut;
    else ut = base_unixtime + (unixtime)(sec_per_unit * time_value);
  }
  else {    //  no_leap year && unit = day
    unix_to_mdyhms(base_unixtime, month, day, year, hour, minute, second);
    adjuste_day_for_day_units(day, month, year, time_value);
    ut = mdyhms_to_unix(month, day, year, hour, minute, second);
    if (time_fraction > (1-TIME_EPSILON) ) ut += (unixtime)sec_per_unit;
    else if (time_fraction > TIME_EPSILON) ut += (unixtime)(time_fraction * sec_per_unit);
  }
  mlog << Debug(5) << method_name
       << unix_to_yyyymmdd_hhmmss(base_unixtime)
       << " plus " << time_value << " times " << sec_per_unit
       << " seconds = " << unix_to_yyyymmdd_hhmmss(ut) << "\n";
  
  return ut;
}

////////////////////////////////////////////////////////////////////////

