// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

#include "documentorientation_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString documentorientation_to_string(const DocumentOrientation t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case OrientationPortrait:       s = "OrientationPortrait";       break;
   case OrientationLandscape:      s = "OrientationLandscape";      break;
   case no_document_orientation:   s = "no_document_orientation";   break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ConcatString(s);

}


////////////////////////////////////////////////////////////////////////


