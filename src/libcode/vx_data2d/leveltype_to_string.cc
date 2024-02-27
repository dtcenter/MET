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
   //     Created by enum_to_string from file "level_info.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "leveltype_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString leveltype_to_string(const LevelType t)

{

const char * s = (const char *) nullptr;

switch ( t )  {

   case LevelType_None:        s = "LevelType_None";        break;
   case LevelType_Accum:       s = "LevelType_Accum";       break;
   case LevelType_Vert:        s = "LevelType_Vert";        break;
   case LevelType_Pres:        s = "LevelType_Pres";        break;
   case LevelType_RecNumber:   s = "LevelType_RecNumber";   break;

   case LevelType_SFC:         s = "LevelType_SFC";         break;
   case LevelType_Time:        s = "LevelType_Time";        break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


bool string_to_leveltype(const char * text, LevelType & t)

{

     if ( strcmp(text, "LevelType_None"     ) == 0 )   { t = LevelType_None;        return ( true ); }
else if ( strcmp(text, "LevelType_Accum"    ) == 0 )   { t = LevelType_Accum;       return ( true ); }
else if ( strcmp(text, "LevelType_Vert"     ) == 0 )   { t = LevelType_Vert;        return ( true ); }
else if ( strcmp(text, "LevelType_Pres"     ) == 0 )   { t = LevelType_Pres;        return ( true ); }
else if ( strcmp(text, "LevelType_RecNumber") == 0 )   { t = LevelType_RecNumber;   return ( true ); }

else if ( strcmp(text, "LevelType_SFC"      ) == 0 )   { t = LevelType_SFC;         return ( true ); }
else if ( strcmp(text, "LevelType_Time"     ) == 0 )   { t = LevelType_Time;        return ( true ); }
   //
   //  nope
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////


