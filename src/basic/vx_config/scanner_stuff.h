// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CONFIG_SCANNER_STUFF_H__
#define  __CONFIG_SCANNER_STUFF_H__


////////////////////////////////////////////////////////////////////////


static const int max_id_length = 2048;


////////////////////////////////////////////////////////////////////////


struct Number {

   bool is_int;

   int i;

   double d;

};


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

double x;

if ( n.is_int )  x = (double) (n.i);
else             x = n.d;

return ( x );

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONFIG_SCANNER_STUFF_H__  */


////////////////////////////////////////////////////////////////////////


