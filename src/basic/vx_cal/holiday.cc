// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <stdlib.h>

#include "vx_cal.h"
#include "holiday.h"


////////////////////////////////////////////////////////////////////////


int is_sunday(int month, int day, int year)

{

return ( day_of_week(month, day, year) == Sunday );

}


////////////////////////////////////////////////////////////////////////


int is_holiday(int month, int day, int year)

{

if ( is_christmas(month, day, year) )        return ( 1 );

if ( is_july4(month, day, year) )            return ( 1 );

if ( is_thanksgiving(month, day, year) )     return ( 1 );

if ( is_newyears(month, day, year) )         return ( 1 );

if ( is_easter(month, day, year) )           return ( 1 );

if ( is_presidents_day(month, day, year) )   return ( 1 );

if ( is_mlk_day(month, day, year) )          return ( 1 );

if ( is_memorial_day(month, day, year) )     return ( 1 );

if ( is_labor_day(month, day, year) )        return ( 1 );

// if ( is_halloween(month, day, year) )        return ( 1 );


   //
   //  nope
   //


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_christmas(int month, int day, int year)

{

if ( (month == 12) && (day == 25) )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_july4(int month, int day, int year)

{

if ( (month == 7) && (day == 4) )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_halloween(int month, int day, int year)

{

if ( (month == 10) && (day == 31) )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_thanksgiving(int month, int day, int year)

   //
   //  4th thursday in November
   //

{

if ( month != 11 )  return ( 0 );

int turkey_day;

switch ( day_of_week(11, 1, year) )  {

   case Monday:     turkey_day = 25;   break;   //  example: 2004
   case Tuesday:    turkey_day = 24;   break;   //  example: 2005
   case Wednesday:  turkey_day = 23;   break;   //  example: 2000
   case Thursday:   turkey_day = 22;   break;   //  example: 2001
   case Friday:     turkey_day = 28;   break;   //  example: 2002

   case Saturday:   turkey_day = 27;   break;   //  example: 2003
   case Sunday:     turkey_day = 26;   break;   //  example: 2009

   default:
      cerr << "\n\n  is_thanksgiving() -> bad weekday\n\n";
      exit ( 1 );
      break;

}

return ( day == turkey_day );

}


////////////////////////////////////////////////////////////////////////


int is_presidents_day(int month, int day, int year)

   //
   //  3rd monday in February
   //

{

if ( month != 2 )  return ( 0 );

int pres_day;

switch ( day_of_week(2, 1, year) )  {

   case Monday:     pres_day = 15;   break;   //  example: 2010
   case Tuesday:    pres_day = 21;   break;   //  example: 2005
   case Wednesday:  pres_day = 20;   break;   //  example: 2006
   case Thursday:   pres_day = 19;   break;   //  example: 2001
   case Friday:     pres_day = 18;   break;   //  example: 2002

   case Saturday:   pres_day = 17;   break;   //  example: 2003
   case Sunday:     pres_day = 16;   break;   //  example: 2004

   default:
      cerr << "\n\n  is_presidents_day() -> bad weekday\n\n";
      exit ( 1 );
      break;

}

return ( day == pres_day );

}


////////////////////////////////////////////////////////////////////////


int is_mlk_day(int month, int day, int year)

   //
   //  3rd monday in January
   //

{

if ( month != 1 )  return ( 0 );

int mlk_day;

switch ( day_of_week(1, 1, year) )  {

   case Monday:     mlk_day = 15;   break;   //  example: 2001
   case Tuesday:    mlk_day = 21;   break;   //  example: 2002
   case Wednesday:  mlk_day = 20;   break;   //  example: 2003
   case Thursday:   mlk_day = 19;   break;   //  example: 2004
   case Friday:     mlk_day = 18;   break;   //  example: 2010

   case Saturday:   mlk_day = 17;   break;   //  example: 2005
   case Sunday:     mlk_day = 16;   break;   //  example: 2006

   default:
      cerr << "\n\n  is_mlk_day() -> bad weekday\n\n";
      exit ( 1 );
      break;

}

return ( day == mlk_day );

}


////////////////////////////////////////////////////////////////////////


int is_memorial_day(int month, int day, int year)

   //
   //  Memorial day is actually May 30, but it's officially
   //
   //    celebrated on the last monday in May
   //

{

if ( month != 5 )  return ( 0 );

int mem_day;

switch ( day_of_week(5, 1, year) )  {

   case Monday:     mem_day = 29;   break;   //  example: 2006
   case Tuesday:    mem_day = 28;   break;   //  example: 2001
   case Wednesday:  mem_day = 27;   break;   //  example: 2002
   case Thursday:   mem_day = 26;   break;   //  example: 2003
   case Friday:     mem_day = 25;   break;   //  example: 2009

   case Saturday:   mem_day = 31;   break;   //  example: 2004
   case Sunday:     mem_day = 30;   break;   //  example: 2005

   default:
      cerr << "\n\n  is_memorial_day() -> bad weekday\n\n";
      exit ( 1 );
      break;

}

return ( day == mem_day );

}


////////////////////////////////////////////////////////////////////////


int is_labor_day(int month, int day, int year)

   //
   //  first monday in September
   //

{

if ( month != 9 )  return ( 0 );

int lab_day;

switch ( day_of_week(9, 1, year) )  {

   case Monday:     lab_day =  1;   break;   //  example: 2003
   case Tuesday:    lab_day =  7;   break;   //  example: 2009
   case Wednesday:  lab_day =  6;   break;   //  example: 2004
   case Thursday:   lab_day =  5;   break;   //  example: 2005
   case Friday:     lab_day =  4;   break;   //  example: 2006

   case Saturday:   lab_day =  3;   break;   //  example: 2001
   case Sunday:     lab_day =  2;   break;   //  example: 2002

   default:
      cerr << "\n\n  is_labor_day() -> bad weekday\n\n";
      exit ( 1 );
      break;

}

return ( day == lab_day );

}


////////////////////////////////////////////////////////////////////////


int is_newyears(int month, int day, int year)

{

if ( (month == 1) && (day == 1) )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_payday(int month, int day, int year)

{

int j;

j = date_to_mjd(month, day, year);

if ( (j%14) == 9 )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_easter(int month, int day, int year)

{

int easter_month, easter_day;


easter(easter_month, easter_day, year);

if ( (month == easter_month) && (day == easter_day) )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////




