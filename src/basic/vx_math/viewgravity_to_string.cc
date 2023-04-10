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
   //     Created by enum_to_string from file "affine.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "viewgravity_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString viewgravity_to_string(const ViewGravity t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case view_center_gravity:      s = "view_center_gravity";      break;
   case view_north_gravity:       s = "view_north_gravity";       break;
   case view_south_gravity:       s = "view_south_gravity";       break;
   case view_east_gravity:        s = "view_east_gravity";        break;
   case view_west_gravity:        s = "view_west_gravity";        break;

   case view_northwest_gravity:   s = "view_northwest_gravity";   break;
   case view_northeast_gravity:   s = "view_northeast_gravity";   break;
   case view_southwest_gravity:   s = "view_southwest_gravity";   break;
   case view_southeast_gravity:   s = "view_southeast_gravity";   break;
   case fill_viewport:            s = "fill_viewport";            break;

   case no_view_gravity:          s = "no_view_gravity";          break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


