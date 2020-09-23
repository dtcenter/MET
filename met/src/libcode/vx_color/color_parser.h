// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////


#ifndef  __VX_COLOR_PARSER_STUFF_H__
#define  __VX_COLOR_PARSER_STUFF_H__


///////////////////////////////////////////////////////////////////////


struct ColorNumber {

   int is_int;

   int i;

   double d;

};


///////////////////////////////////////////////////////////////////////


   //
   //  r, g, b values from 0.0 to 255.0 inclusive
   //

struct Dcolor {

   double r;

   double g;

   double b;

};


///////////////////////////////////////////////////////////////////////


#endif   /*  __VX_COLOR_PARSER_STUFF_H__  */


///////////////////////////////////////////////////////////////////////


