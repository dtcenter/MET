// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __IS_NUMBER_H__
#define  __IS_NUMBER_H__


////////////////////////////////////////////////////////////////////////


struct Number {

   int is_int;

   int i;

   double d;

};


////////////////////////////////////////////////////////////////////////


extern int is_number(const char *);   //  is integer or float

extern int is_integer(const char *);

extern int is_float(const char *);


////////////////////////////////////////////////////////////////////////


#endif   //  __IS_NUMBER_H__


////////////////////////////////////////////////////////////////////////


