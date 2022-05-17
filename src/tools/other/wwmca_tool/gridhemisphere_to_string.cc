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
   //     Created by enum_to_string from file "wwmca_ref.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "gridhemisphere_to_string.h"


////////////////////////////////////////////////////////////////////////


void gridhemisphere_to_string(const GridHemisphere t, char * out)

{

switch ( t )  {

   case north_hemisphere:   strcpy(out, "north_hemisphere");   break;
   case south_hemisphere:   strcpy(out, "south_hemisphere");   break;
   case both_hemispheres:   strcpy(out, "both_hemispheres");   break;
   case no_hemisphere:      strcpy(out, "no_hemisphere");      break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


