// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //     Created by enum_to_string from file "vx_ps.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "fontfamily_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString fontfamily_to_string(const FontFamily t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case ff_Helvetica:     s = "ff_Helvetica";     break;
   case ff_NewCentury:    s = "ff_NewCentury";    break;
   case ff_Palatino:      s = "ff_Palatino";      break;
   case ff_Times:         s = "ff_Times";         break;
   case ff_Courier:       s = "ff_Courier";       break;

   case ff_Bookman:       s = "ff_Bookman";       break;
   case no_font_family:   s = "no_font_family";   break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


