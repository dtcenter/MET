

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //    This file is machine generated.
   //
   //    Do not edit by hand.
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include <vx_util/asciitablejust_to_string.h>


////////////////////////////////////////////////////////////////////////


void asciitablejust_to_string(const AsciiTableJust t, char * out)

{

switch ( t )  {

   case RightJust :    strcpy(out, "RightJust");    break;
   case LeftJust :     strcpy(out, "LeftJust");     break;
   case CenterJust :   strcpy(out, "CenterJust");   break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


