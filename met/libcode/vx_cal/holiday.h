// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __HOLIDAY_H__
#define  __HOLIDAY_H__


////////////////////////////////////////////////////////////////////////


extern int is_sunday           (int month, int day, int year);

extern int is_holiday          (int month, int day, int year);


////////////////////////////////////////////////////////////////////////


extern int is_christmas        (int month, int day, int year);

extern int is_july4            (int month, int day, int year);

extern int is_thanksgiving     (int month, int day, int year);

extern int is_newyears         (int month, int day, int year);

extern int is_easter           (int month, int day, int year);

extern int is_presidents_day   (int month, int day, int year);

extern int is_mlk_day          (int month, int day, int year);

extern int is_memorial_day     (int month, int day, int year);

extern int is_labor_day        (int month, int day, int year);

extern int is_halloween        (int month, int day, int year);

extern int is_payday           (int month, int day, int year);


////////////////////////////////////////////////////////////////////////


#endif   //  __HOLIDAY_H__


////////////////////////////////////////////////////////////////////////



