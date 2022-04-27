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
   //
   //     Created by enum_to_string from file "vx_ps.h"
   //
   //     on April 27, 2022   2:35 pm MDT
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "documentmedia_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString documentmedia_to_string(const DocumentMedia t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case MediaLetter:         s = "MediaLetter";         break;
   case MediaA4:             s = "MediaA4";             break;
   case no_document_media:   s = "no_document_media";   break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


