// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


void sec_to_hhmmss(int in_sec, ConcatString& str)

{

int hour, minute, second;

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

return ( str );

}


////////////////////////////////////////////////////////////////////////


ConcatString sec_to_hhmmss_colon(int in_sec)

{

ConcatString s;
int hour, minute, second;

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

return(s);

}


////////////////////////////////////////////////////////////////////////


int hhmmss_to_sec(const char * text)

{

int i, hour, minute, second;

i = atoi(text);

hour   = i/10000;

minute = (i%10000)/100;

second = i%100;

i = hms_to_sec(hour, minute, second);

return ( i );

}


////////////////////////////////////////////////////////////////////////


void unix_to_yyyymmdd_hhmmss(unixtime u, ConcatString& str)

{

int year, month, day, hour, minute, second;

unix_to_mdyhms(u, month, day, year, hour, minute, second);

 str.format("%.4i%.2i%.2i_%.2i%.2i%.2i",
        year, month, day, hour, minute, second);
 
return;

}


////////////////////////////////////////////////////////////////////////

void unix_to_yyyymmdd_hhmmss(unixtime u, char * junk, size_t len)

{

int year, month, day, hour, minute, second;

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

return ( str );

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmdd_hhmmss_to_unix(const char * text)

{

int month, day, year, hour, minute, second;
unixtime t;
char junk[32];


substring_vx_cal(text, junk, 0, 3);

year = atoi(junk);

substring_vx_cal(text, junk, 4, 5);

month = atoi(junk);

substring_vx_cal(text, junk, 6, 7);

day = atoi(junk);

substring_vx_cal(text, junk, 9, 10);

hour = atoi(junk);

substring_vx_cal(text, junk, 11, 12);

minute = atoi(junk);

substring_vx_cal(text, junk, 13, 14);

second = atoi(junk);


t = mdyhms_to_unix(month, day, year, hour, minute, second);

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmdd_hh_to_unix(const char * text)

{

int month, day, year, hour;
unixtime t;
char junk[32];


substring_vx_cal(text, junk, 0, 3);

year  = atoi(junk);

substring_vx_cal(text, junk, 4, 5);

month = atoi(junk);

substring_vx_cal(text, junk, 6, 7);

day   = atoi(junk);

   //
   //  skip the "_"
   //

substring_vx_cal(text, junk, 9, 10);

hour   = atoi(junk);



t = mdyhms_to_unix(month, day, year, hour, 0, 0);

return ( t );

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmdd_to_unix(const char * text)

{

int month, day, year;
unixtime t;
char junk[32];


substring_vx_cal(text, junk, 0, 3);

year  = atoi(junk);

substring_vx_cal(text, junk, 4, 5);

month = atoi(junk);

substring_vx_cal(text, junk, 6, 7);

day   = atoi(junk);


t = mdyhms_to_unix(month, day, year, 0, 0, 0);

return ( t );

}


////////////////////////////////////////////////////////////////////////


void unix_to_yyyymmddhh(unixtime u, ConcatString& str)

{

int year, month, day, hour, minute, second;

unix_to_mdyhms(u, month, day, year, hour, minute, second);

str.format("%.4i%.2i%.2i%.2i", year, month, day, hour);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString unix_to_yyyymmddhh(unixtime u)

{

ConcatString str;

unix_to_yyyymmddhh(u, str);

return ( str );

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmddhh_to_unix(const char * text)

{

ConcatString str;

str << text << "0000";

 return ( yyyymmddhhmmss_to_unix(str.c_str()) );

}

////////////////////////////////////////////////////////////////////////


unixtime yyyymmddhhmm_to_unix(const char * text)

{

ConcatString str;

str << text << "00";

 return ( yyyymmddhhmmss_to_unix(str.c_str()) );

}

////////////////////////////////////////////////////////////////////////


unixtime yyyymmddhhmmss_to_unix(const char * text)

{

int month, day, year, hour, minute, second;
unixtime t;
char junk[32];


substring_vx_cal(text, junk, 0, 3);

year = atoi(junk);

substring_vx_cal(text, junk, 4, 5);

month = atoi(junk);

substring_vx_cal(text, junk, 6, 7);

day = atoi(junk);

substring_vx_cal(text, junk, 8, 9);

hour = atoi(junk);

substring_vx_cal(text, junk, 10, 11);

minute = atoi(junk);

substring_vx_cal(text, junk, 12, 13);

second = atoi(junk);

t = mdyhms_to_unix(month, day, year, hour, minute, second);

return ( t );

}


////////////////////////////////////////////////////////////////////////


void substring_vx_cal(const char * text, char * out, int first, int last)

{

if ( first > last )  {

   mlog << Error << "\nsubstring_vx_cal() -> bad input values!\n\n";

   exit ( 1 );

}

int j, n;

n = last - first + 1;

for (j=0; j<n; ++j)  {

   out[j] = text[first + j];

}

out[n] = (char) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void make_timestring(unixtime t, ConcatString& str)

{

int month, day, year, hour, minute, second;


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

return ( str );

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
else if ( strlen(text) == 0 ) {

   mlog << Error << "\ntimestring_to_unix(const char *) -> "
        << "empty time string!\n\n";
   exit ( 1 );

}
else if ( strcmp(text, bad_data_str) == 0 ||
          strcmp(text, na_str      ) == 0 )  t = (unixtime) 0;
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


return ( t );

}


////////////////////////////////////////////////////////////////////////


bool is_datestring(const char * text)

{

return ( is_yyyymmdd_hhmmss(text) ||
         is_yyyymmdd_hh(text)     ||
         is_yyyymmddhhmmss(text)  ||
         is_yyyymmddhhmm(text)    ||
         is_yyyymmddhh(text)      ||
         is_yyyymmdd(text) );

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmdd(const char * text)

{

return ( check_reg_exp("^[0-9]\\{8\\}$", text) );

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmddhh(const char * text)

{

return ( check_reg_exp("^[0-9]\\{10\\}$", text) );

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmddhhmm(const char * text)

{

return ( check_reg_exp("^[0-9]\\{12\\}$", text) );

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmddhhmmss(const char * text)

{

return ( check_reg_exp("^[0-9]\\{14\\}$", text) );

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmdd_hh(const char * text)

{

return ( check_reg_exp("^[0-9]\\{8\\}_[0-9]\\{2\\}$", text) );

}


////////////////////////////////////////////////////////////////////////


bool is_yyyymmdd_hhmmss(const char * text)

{

return ( check_reg_exp("^[0-9]\\{8\\}_[0-9]\\{6\\}$", text) );

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
else if ( strlen(text) == 0 ) {

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

return ( t );

}


////////////////////////////////////////////////////////////////////////


ConcatString sec_to_timestring(int s)

{

ConcatString str;

if      ( s == bad_data_int) str = na_str;
else if ( s % 3600 == 0 )    str = HH ( s / 3600 );
else                         str = sec_to_hhmmss ( s );

return ( str );

}


////////////////////////////////////////////////////////////////////////


bool is_hhmmss(const char * text)

{

// Allow negative times and 2 to 5 digits for the number of hours
return ( check_reg_exp("^-*[0-9]\\{6,9\\}$", text) );

}


////////////////////////////////////////////////////////////////////////


bool is_hh(const char * text)

{

// Allow negative times and 1 to 5 digits for the number of hours
return ( check_reg_exp("^-*[0-9]\\{1,5\\}$", text) );

}


////////////////////////////////////////////////////////////////////////


ConcatString HH(int hours)

{

ConcatString str;

str.format("%02d", hours);

return ( str );

}


////////////////////////////////////////////////////////////////////////
