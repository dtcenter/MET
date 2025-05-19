// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
   //     Created by enum_to_string from file "wwmca_ref.h"
   //


////////////////////////////////////////////////////////////////////////


#include <string.h>

#include "gridhemisphere_to_string.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


ConcatString gridhemisphere_to_string(const GridHemisphere t)

{

const char * s = (const char *) nullptr;

switch ( t )  {

   case north_hemisphere:   s = "north_hemisphere";   break;
   case south_hemisphere:   s = "south_hemisphere";   break;
   case both_hemispheres:   s = "both_hemispheres";   break;
   case no_hemisphere:      s = "no_hemisphere";      break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ConcatString(s);

}


////////////////////////////////////////////////////////////////////////


