// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __IS_NUMBER_H__
#define  __IS_NUMBER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


struct Number {

   int is_int;

   int i;

   double d;

};


////////////////////////////////////////////////////////////////////////


inline ostream & operator<<(ostream & __out, const Number & __n)

{

if ( __n.is_int )  __out << (__n.i);
else               __out << (__n.d);

return ( __out );

}


////////////////////////////////////////////////////////////////////////


inline void set_int(Number & n, int k)

{

n.is_int = true;

n.i = k;

n.d = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


inline void set_double(Number & n, double x)

{

n.is_int = false;

n.i = 0;

n.d = x;

return;

}


////////////////////////////////////////////////////////////////////////


inline double as_double(const Number & n)

{

return ( (n.is_int) ? ((double) (n.i)) : (n.d) );

}


////////////////////////////////////////////////////////////////////////


extern int is_number(const char *);   //  is integer or float

extern int is_integer(const char *);

extern int is_float(const char *);


////////////////////////////////////////////////////////////////////////


#endif   //  __IS_NUMBER_H__


////////////////////////////////////////////////////////////////////////


