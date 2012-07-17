// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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


void sec_to_hhmmss(int in_sec, char *str)

{

int hour, minute, second;

sec_to_hms(in_sec, hour, minute, second);

sprintf(str, "%.2i%.2i%.2i", hour, minute, second);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString sec_to_hhmmss(int in_sec)

{

char junk[1024];

sec_to_hhmmss(in_sec, junk);

ConcatString str = junk;

return ( str );

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


void unix_to_yyyymmdd_hhmmss(unixtime u, char *str)

{

int year, month, day, hour, minute, second;

unix_to_mdyhms(u, month, day, year, hour, minute, second);

sprintf(str, "%.4i%.2i%.2i_%.2i%.2i%.2i", 
        year, month, day, hour, minute, second);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString unix_to_yyyymmdd_hhmmss(unixtime u)

{

char junk[1024];

unix_to_yyyymmdd_hhmmss(u, junk);

ConcatString str = junk;

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


void unix_to_yyyymmddhh(unixtime u, char *str)

{

int year, month, day, hour, minute, second;

unix_to_mdyhms(u, month, day, year, hour, minute, second);

sprintf(str, "%.4i%.2i%.2i%.2i", year, month, day, hour);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString unix_to_yyyymmddhh(unixtime u)

{

char junk[1024];

unix_to_yyyymmddhh(u, junk);

ConcatString str = junk;

return ( str );

}


////////////////////////////////////////////////////////////////////////


unixtime yyyymmddhh_to_unix(const char * text)

{

int month, day, year, hour;
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


t = mdyhms_to_unix(month, day, year, hour, 0, 0);

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


void make_timestring(unixtime t, char * junk)

{

int month, day, year, hour, minute, second;


unix_to_mdyhms(t, month, day, year, hour, minute, second);

sprintf(junk, "%s %2d, %d  %02d:%02d:%02d",
        short_month_name[month], day, year,
        hour, minute, second);

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString make_timestring(unixtime u)

{

char junk[1024];

make_timestring(u, junk);

ConcatString str = junk;

return ( str );

}


////////////////////////////////////////////////////////////////////////


unixtime timestring_to_unix(const char * text)

{

unixtime t;

t = (unixtime) 0;

     if ( !text                     )  t = (unixtime) 0;
else if ( strlen(text) == 0         )  t = (unixtime) 0;
else if ( is_yyyymmdd_hhmmss (text) )  t = yyyymmdd_hhmmss_to_unix (text);
else if ( is_yyyymmdd_hh     (text) )  t = yyyymmdd_hh_to_unix     (text);
else if ( is_yyyymmddhh      (text) )  t = yyyymmddhh_to_unix      (text);
else if ( is_yyyymmdd        (text) )  t = yyyymmdd_to_unix        (text);
else {

   mlog << Error << "\ntimestring_to_unix(const char *) -> can't parse date/time string \"" << text << "\"\n\n";

   exit ( 1 );

}


return ( t );

}


////////////////////////////////////////////////////////////////////////


int is_yyyymmdd(const char * text)

{

if ( strlen(text) != 8 )  return ( 0 );

int j;

for (j=0; j<8; ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_yyyymmddhh(const char * text)

{

if ( strlen(text) != 10 )  return ( 0 );

int j;

for (j=0; j<10; ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_yyyymmdd_hh(const char * text)

{

if ( strlen(text) != 11 )  return ( 0 );

int j;

for (j=0; j<8; ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}

if ( text[8] != '_' )  return ( 0 );

for (j=9; j<11; ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_yyyymmdd_hhmmss(const char * text)

{

if ( strlen(text) != 15 )  return ( 0 );

int j;

for (j=0; j<8; ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}

if ( text[8] != '_' )  return ( 0 );

for (j=9; j<15; ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int timestring_to_sec(const char * text)

{

int t;

t = 0;

     if ( !text             )  t = 0;
else if ( strlen(text) == 0 )  t = 0;
else if ( is_hhmmss (text)  )  t = hhmmss_to_sec(text);
else if ( is_hh     (text)  )  t = hms_to_sec(atoi(text), 0, 0);
else {

   mlog << Error << "\ntimestring_to_sec(const char *) -> can't parse time string \"" << text << "\"\n\n";

   exit ( 1 );

}


return ( t );

}


////////////////////////////////////////////////////////////////////////


int is_hhmmss(const char * text)

{

// Allow 2 or 3 digits for the number of hours
if ( strlen(text) != 6 && strlen(text) != 7)  return ( 0 );

unsigned int j;

for (j=0; j<strlen(text); ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_hh(const char * text)

{

// Allow 1, 2, or 3 digits for the number of hours
if ( strlen(text) < 1 || strlen(text) > 3 )  return ( 0 );

unsigned int j;

for (j=0; j<strlen(text); ++j)  {

   if ( !(isdigit(text[j])) )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


ConcatString HH(int hours)

{

char junk[1024];

sprintf(junk, "%02d", hours);

ConcatString str = junk;

return ( str );

}


////////////////////////////////////////////////////////////////////////
