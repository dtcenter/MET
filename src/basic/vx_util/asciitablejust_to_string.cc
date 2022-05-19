// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
   //     Created by enum_to_string from file "ascii_table.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "asciitablejust_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString asciitablejust_to_string(const AsciiTableJust t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case RightJust:    s = "RightJust";    break;
   case LeftJust:     s = "LeftJust";     break;
   case CenterJust:   s = "CenterJust";   break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


