

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //
   //     Created by enum_to_string from file "wwmca_ref.h"
   //
   //     on July 7, 2010   11:15 am MDT
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


