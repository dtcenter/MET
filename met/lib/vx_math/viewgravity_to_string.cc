

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //
   //     Created by enum_to_string from file "affine.h"
   //
   //     on July 7, 2010   8:41 am MDT
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "vx_math/viewgravity_to_string.h"


////////////////////////////////////////////////////////////////////////


void viewgravity_to_string(const ViewGravity t, char * out)

{

switch ( t )  {

   case view_center_gravity:      strcpy(out, "view_center_gravity");      break;
   case view_north_gravity:       strcpy(out, "view_north_gravity");       break;
   case view_south_gravity:       strcpy(out, "view_south_gravity");       break;
   case view_east_gravity:        strcpy(out, "view_east_gravity");        break;
   case view_west_gravity:        strcpy(out, "view_west_gravity");        break;

   case view_northwest_gravity:   strcpy(out, "view_northwest_gravity");   break;
   case view_northeast_gravity:   strcpy(out, "view_northeast_gravity");   break;
   case view_southwest_gravity:   strcpy(out, "view_southwest_gravity");   break;
   case view_southeast_gravity:   strcpy(out, "view_southeast_gravity");   break;
   case no_view_gravity:          strcpy(out, "no_view_gravity");          break;


   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


bool string_to_viewgravity(const char * text, ViewGravity & t)

{

     if ( strcmp(text, "view_center_gravity"   ) == 0 )   { t = view_center_gravity;      return ( true ); }
else if ( strcmp(text, "view_north_gravity"    ) == 0 )   { t = view_north_gravity;       return ( true ); }
else if ( strcmp(text, "view_south_gravity"    ) == 0 )   { t = view_south_gravity;       return ( true ); }
else if ( strcmp(text, "view_east_gravity"     ) == 0 )   { t = view_east_gravity;        return ( true ); }
else if ( strcmp(text, "view_west_gravity"     ) == 0 )   { t = view_west_gravity;        return ( true ); }

else if ( strcmp(text, "view_northwest_gravity") == 0 )   { t = view_northwest_gravity;   return ( true ); }
else if ( strcmp(text, "view_northeast_gravity") == 0 )   { t = view_northeast_gravity;   return ( true ); }
else if ( strcmp(text, "view_southwest_gravity") == 0 )   { t = view_southwest_gravity;   return ( true ); }
else if ( strcmp(text, "view_southeast_gravity") == 0 )   { t = view_southeast_gravity;   return ( true ); }
else if ( strcmp(text, "no_view_gravity"       ) == 0 )   { t = no_view_gravity;          return ( true ); }
else {

   return ( false );   //  this final "else" clause isn't strictly needed

}

   //
   //  nope
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////


