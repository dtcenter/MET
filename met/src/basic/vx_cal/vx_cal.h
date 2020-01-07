// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VX_CAL_H__
#define  __VX_CAL_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  These routines use the Gregorian Calendar.
   //
   //     The algorithms used are not valid before
   //
   //     about 4700 BC.
   //


////////////////////////////////////////////////////////////////////////


typedef long long unixtime;


////////////////////////////////////////////////////////////////////////


static const int mjd_ut0 = 40587;   //  mjd of Jan 1, 1970


////////////////////////////////////////////////////////////////////////

extern  unixtime  add_to_unixtime (unixtime u, int sec_per_unit, double time_value, bool no_leap);

extern  unixtime  doyhms_to_unix  (int doy, int year, int hour, int minute, int second);

extern  unixtime  mdyhms_to_unix  (int month, int day, int year, int hour, int minute, int second);

extern  void      unix_to_mdyhms  (unixtime u, int & month, int & day, int & year, int & hour, int & minute, int & second);


extern  int       date_to_mjd     (int m, int d, int y);

extern  void      mjd_to_date     (int mjd, int & month, int & day, int & year);


extern  int       day_dif         (int m1, int d1, int y1, int m2, int d2, int y2);

extern  int       day_of_week     (int month, int day, int year);

extern  int       is_leap_year    (int year);

extern  void      easter          (int & month, int & day, int year);


extern  int       is_dst          (int month, int day, int year, int hour, int minute, int second);

extern  int       is_dst          (unixtime);

extern  void      sec_to_hms      (int in_sec, int & hour, int & min, int & sec);

extern  int       hms_to_sec      (int hour, int min, int sec);

extern  int       unix_to_sec_of_day (unixtime u);

extern  long      unix_to_long_yyyymmddhh (unixtime u);

// Parse time strings

extern  void     substring_vx_cal       (const char * text, char * out, int first, int last);

extern  void         sec_to_hhmmss      (int, ConcatString&);
extern  ConcatString sec_to_hhmmss      (int);

extern  ConcatString sec_to_hhmmss_colon(int);

extern  int      hhmmss_to_sec          (const char *);

extern  void         unix_to_yyyymmdd_hhmmss(unixtime, ConcatString&);
extern  void         unix_to_yyyymmdd_hhmmss(unixtime, char *);
extern  ConcatString unix_to_yyyymmdd_hhmmss(unixtime);

extern  unixtime yyyymmdd_hhmmss_to_unix(const char *);

extern  unixtime yyyymmdd_hh_to_unix(const char *);

extern  unixtime yyyymmdd_to_unix(const char *);

extern  void         unix_to_yyyymmddhh(unixtime, char *);
extern  ConcatString unix_to_yyyymmddhh(unixtime);

extern  unixtime yyyymmddhh_to_unix(const char *);
extern  unixtime yyyymmddhhmm_to_unix(const char *);
extern  unixtime yyyymmddhhmmss_to_unix(const char *);

extern  void         make_timestring(unixtime, ConcatString&);
extern  ConcatString make_timestring(unixtime);

extern  unixtime timestring_to_unix(const char *);

extern  ConcatString HH(int hours);

extern  int          timestring_to_sec(const char *);
extern  ConcatString sec_to_timestring(int);


////////////////////////////////////////////////////////////////////////


extern  bool is_datestring(const char * text);

extern  bool is_yyyymmdd(const char * text);

extern  bool is_yyyymmddhh(const char * text);

extern  bool is_yyyymmddhhmm(const char * text);

extern  bool is_yyyymmddhhmmss(const char * text);

extern  bool is_yyyymmdd_hh(const char * text);

extern  bool is_yyyymmdd_hhmmss(const char * text);

extern  bool is_hhmmss(const char * text);

extern  bool is_hh(const char * text);


////////////////////////////////////////////////////////////////////////


extern const char * day_name[];

extern const char * short_day_name[];

extern const char * month_name[];

extern const char * short_month_name[];


////////////////////////////////////////////////////////////////////////


static const int Monday    = 1;
static const int Tuesday   = 2;
static const int Wednesday = 3;
static const int Thursday  = 4;
static const int Friday    = 5;
static const int Saturday  = 6;
static const int Sunday    = 7;


////////////////////////////////////////////////////////////////////////


   //
   //  Bad Data Values
   //

static const int        bad_data_int          = -9999;
static const long long  bad_data_ll           = -9999LL;
static const float      bad_data_float        = -9999.f;
static const double     bad_data_double       = -9999.0;
static const char       bad_data_str[]        = "-9999";
static const char       bad_data_char         = '\0';
static const char       na_str[]              = "NA";
static const string     na_string             = "NA";


////////////////////////////////////////////////////////////////////////


extern unixtime string_to_unixtime(const char *);

extern ConcatString unixtime_to_string(const unixtime);


////////////////////////////////////////////////////////////////////////


#include "time_array.h"


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_CAL_H__


////////////////////////////////////////////////////////////////////////



