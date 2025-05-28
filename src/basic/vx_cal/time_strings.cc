// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


void sec_to_hhmmss(int in_sec, ConcatString& str)

{

int hour;
int minute;
int second;

if ( in_sec == bad_data_int )  {
   str = na_str;
}
else  {

   sec_to_hms(in_sec, hour, minute, second);

   if(in_sec < 0) {
      str.format("-%.2i%.2i%.2i", abs(hour), abs(minute), abs(second));
   }
   else {
      str.format("%.2i%.2i%.2i", hour, minute, second);
   }
}

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString sec_to_hhmmss(int in_sec)

{

ConcatString str;
 
sec_to_hhmmss(in_sec, str);

return str;

}


////////////////////////////////////////////////////////////////////////


ConcatString sec_to_hhmmss_colon(int in_sec)

{

ConcatString s;
int hour;
int minute;
int second;

if ( in_sec == bad_data_int )  {
   s = na_str;
}
else  {

   sec_to_hms(in_sec, hour, minute, second);

   if(in_sec < 0) {
      s.format("-%.2i:%.2i:%.2i", abs(hour), abs(minute), abs(second));
   }
   else {
      s.format("%.2i:%.2i:%.2i", hour, minute, second);
   }
}

return s;

}


////////////////////////////////////////////////////////////////////////


int hhmmss_to_sec(const char * text)

{

int i;
int hour;
int minute;
int second;

i = atoi(text);

hour   = i/10000;

minute = (i%10000)/100;

second = i%100;

i = hms_to_sec(hour, minute, second);

return i;

}


////////////////////////////////////////////////////////////////////////


void unix_to_yyyymmdd_hhmmss(unixtime u, ConcatString& str)

{

int year;
int month;
int day;
int hour;
int minute;
int second;

unix_to_mdyhms(u, month, day, year, hour, minute, second);

 str.format("%.4i%.2i%.2i_%.2i%.2i%.2i",
        year, month, day, hour, minute, second);
 
return;

}


////////////////////////////////////////////////////////////////////////

void unix_to_yyyymmdd_hhmmss(unixtime u, char * junk, size_t len)

{

int year;
int month;
int day;
int hour;
int minute;
int second;

unix_to_mdyhms(u, month, day, year, hour, minute, second);

snprintf(junk, len, "%.4i%.2i%.2i_%.2i%.2i%.2i",
         year, month, day, hour, minute, second);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString unix_to_yyyymmdd_hhmmss(unixtime u)

{

ConcatString str;

unix_to_yyyymmdd_hhmmss(u, str);

return str;

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmdd_hhmmss_to_unix(const char * text)

{

string s(text);

int year   = stoi(s.substr(0,  4));
int month  = stoi(s.substr(4,  2));
int day    = stoi(s.substr(6,  2));
int hour   = stoi(s.substr(9,  2));
int minute = stoi(s.substr(11, 2));
int second = stoi(s.substr(13, 2));

return mdyhms_to_unix(month, day, year, hour, minute, second);

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmddThhmmss_to_unix(const char * text)

{

string s(text);

// Parse format YYYY-MM-DDTHH:MM:SS
int year   = stoi(s.substr(0,  4));
int month  = stoi(s.substr(5,  2));
int day    = stoi(s.substr(8,  2));
int hour   = stoi(s.substr(11, 2));
int minute = stoi(s.substr(14, 2));
int second = stoi(s.substr(17, 2));

return mdyhms_to_unix(month, day, year, hour, minute, second);

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmdd_hh_to_unix(const char * text)

{

string s(text);

int year  = stoi(s.substr(0, 4));
int month = stoi(s.substr(4, 2));
int day   = stoi(s.substr(6, 2));
int hour  = stoi(s.substr(9, 2));

return mdyhms_to_unix(month, day, year, hour, 0, 0);

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmdd_to_unix(const char * text)

{

string s(text);

int year  = stoi(s.substr(0, 4));
int month = stoi(s.substr(4, 2));
int day   = stoi(s.substr(6, 2));

return mdyhms_to_unix(month, day, year, 0, 0, 0);

}


////////////////////////////////////////////////////////////////////////


void unix_to_yyyymmddhh(unixtime u, ConcatString& str)

{

int month;
int day;
int year;
int hour;
int minute;
int second;

unix_to_mdyhms(u, month, day, year, hour, minute, second);

str.format("%.4i%.2i%.2i%.2i", year, month, day, hour);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString unix_to_yyyymmddhh(unixtime u)

{

ConcatString str;

unix_to_yyyymmddhh(u, str);

return str;

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmddhh_to_unix(const char * text)

{

ConcatString str;

str << text << "0000";

return yyyymmddhhmmss_to_unix(str.c_str());

}

////////////////////////////////////////////////////////////////////////


unixtime yyyymmddhhmm_to_unix(const char * text)

{

ConcatString str;

str << text << "00";

return yyyymmddhhmmss_to_unix(str.c_str());

}

////////////////////////////////////////////////////////////////////////


unixtime yyyymmddhhmmss_to_unix(const char * text)

{

string s(text);

int year   = stoi(s.substr(0,  4));
int month  = stoi(s.substr(4,  2));
int day    = stoi(s.substr(6,  2));
int hour   = stoi(s.substr(8,  2));
int minute = stoi(s.substr(10, 2));
int second = stoi(s.substr(12, 2));

return mdyhms_to_unix(month, day, year, hour, minute, second);

}


////////////////////////////////////////////////////////////////////////


void make_timestring(unixtime t, ConcatString& str)

{

int month;
int day;
int year;
int hour;
int minute;
int second;


unix_to_mdyhms(t, month, day, year, hour, minute, second);

str.format("%s %2d, %d  %02d:%02d:%02d",
           short_month_name[month], day, year,
           hour, minute, second);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString make_timestring(unixtime u)

{

ConcatString str;

make_timestring(u, str);

return str;

}


////////////////////////////////////////////////////////////////////////


unixtime timestring_to_unix(const char * text)

{

unixtime t;

t = (unixtime) 0;

if ( !text ) {

   mlog << Error << "\ntimestring_to_unix(const char *) -> "
        << "null pointer!\n\n";

   exit ( 1 );

}
else if ( m_strlen(text) == 0 ) {

   mlog << Error << "\ntimestring_to_unix(const char *) -> "
        << "empty time string!\n\n";
   exit ( 1 );

}
else if ( strcmp(text, bad_data_str) == 0 ||
          strcmp(text, na_str      ) == 0 )  t = (unixtime) 0;
else if ( is_yyyymmddThhmmss (text)       )  t = yyyymmddThhmmss_to_unix (text);
else if ( is_yyyymmdd_hhmmss (text)       )  t = yyyymmdd_hhmmss_to_unix (text);
else if ( is_yyyymmdd_hh     (text)       )  t = yyyymmdd_hh_to_unix     (text);
else if ( is_yyyymmddhhmmss  (text)       )  t = yyyymmddhhmmss_to_unix  (text);
else if ( is_yyyymmddhhmm    (text)       )  t = yyyymmddhhmm_to_unix    (text);
else if ( is_yyyymmddhh      (text)       )  t = yyyymmddhh_to_unix      (text);
else if ( is_yyyymmdd        (text)       )  t = yyyymmdd_to_unix        (text);
else {

   mlog << Error << "\ntimestring_to_unix(const char *) -> "
        << "can't parse date/time string \"" << text << "\"\n\n";

   exit ( 1 );

}


return t;

}


////////////////////////////////////////////////////////////////////////


time_t timestring_to_time_t(const char * text) {

int month;
int day;
int year;
int hour;
int minute;
int second;

unix_to_mdyhms(timestring_to_unix(text),
               month, day, year, hour, minute, second);

struct tm time_struct;
memset(&time_struct, 0, sizeof(time_struct));
time_struct.tm_year = year - 1900;
time_struct.tm_mon  = month - 1;
time_struct.tm_mday = day;
time_struct.tm_hour = hour;
time_struct.tm_min  = minute;
time_struct.tm_sec  = second;

return timegm(&time_struct);
}


////////////////////////////////////////////////////////////////////////


bool is_datestring(const char * text)

{

return is_yyyymmdd_hhmmss(text) ||
       is_yyyymmddThhmmss(text) ||
       is_yyyymmdd_hh(text)     ||
       is_yyyymmddhhmmss(text)  ||
       is_yyyymmddhhmm(text)    ||
       is_yyyymmddhh(text)      ||
       is_yyyymmdd(text);

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmdd(const char * text)

{

return check_reg_exp("^[0-9]\\{8\\}$", text);

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmddhh(const char * text)

{

return check_reg_exp("^[0-9]\\{10\\}$", text);

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmddhhmm(const char * text)

{

return check_reg_exp("^[0-9]\\{12\\}$", text);

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmddhhmmss(const char * text)

{

return check_reg_exp("^[0-9]\\{14\\}$", text);

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmdd_hh(const char * text)

{

return check_reg_exp("^[0-9]\\{8\\}_[0-9]\\{2\\}$", text);

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmdd_hhmmss(const char * text)

{

return check_reg_exp("^[0-9]\\{8\\}_[0-9]\\{6\\}$", text);

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmddThhmmss(const char * text)

{

return check_reg_exp("^[0-9]\\{4\\}-[0-9]\\{2\\}-[0-9]\\{2\\}T[0-9]\\{2\\}:[0-9]\\{2\\}:[0-9]\\{2\\}Z$", text);

}


////////////////////////////////////////////////////////////////////////


int timestring_to_sec(const char * text)

{

int t;

t = 0;

if ( !text ) {

   mlog << Error << "\ntimestring_to_sec(const char *) -> "
        << "null pointer!\n\n";

   exit ( 1 );

}
else if ( m_strlen(text) == 0 ) {

   mlog << Error << "\ntimestring_to_sec(const char *) -> "
        << "empty time string!\n\n";

   exit ( 1 );

}
else if ( strcmp(text,       na_str) == 0 ||
          strcmp(text, bad_data_str) == 0 )
                               t = bad_data_int;
else if ( is_hhmmss (text)  )  t = hhmmss_to_sec(text);
else if ( is_hh     (text)  )  t = hms_to_sec(atoi(text), 0, 0);
else {

   mlog << Error << "\ntimestring_to_sec(const char *) -> can't parse time string \"" << text << "\"\n\n";

   exit ( 1 );

}

return t;

}


////////////////////////////////////////////////////////////////////////


ConcatString sec_to_timestring(int s)

{

ConcatString str;

if      ( s == bad_data_int) str = na_str;
else if ( s % 3600 == 0 )    str = HH ( s / 3600 );
else                         str = sec_to_hhmmss ( s );

return str;

}


////////////////////////////////////////////////////////////////////////


bool is_hhmmss(const char * text)

{

// Allow negative times and 2 to 5 digits for the number of hours
return check_reg_exp("^-*[0-9]\\{6,9\\}$", text);

}


////////////////////////////////////////////////////////////////////////


bool is_hh(const char * text)

{

// Allow negative times and 1 to 5 digits for the number of hours
return check_reg_exp("^-*[0-9]\\{1,5\\}$", text);

}


////////////////////////////////////////////////////////////////////////


ConcatString HH(int hours)

{

ConcatString str;

str.format("%02d", hours);

return str;

}


////////////////////////////////////////////////////////////////////////
