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
   //     Created by enum_to_string from file "mtd_file_base.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "mtdfiletype_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString mtdfiletype_to_string(const MtdFileType t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case mtd_file_raw:       s = "mtd_file_raw";       break;
   case mtd_file_conv:      s = "mtd_file_conv";      break;
   case mtd_file_mask:      s = "mtd_file_mask";      break;
   case mtd_file_object:    s = "mtd_file_object";    break;
   case no_mtd_file_type:   s = "no_mtd_file_type";   break;


   default:
      s = "(bad value)";
      break;

}   //  switch


return ConcatString(s);

}


////////////////////////////////////////////////////////////////////////


bool string_to_mtdfiletype(const char * text, MtdFileType & t)

{

     if ( strcmp(text, "mtd_file_raw"    ) == 0 )   { t = mtd_file_raw;       return true; }
else if ( strcmp(text, "mtd_file_conv"   ) == 0 )   { t = mtd_file_conv;      return true; }
else if ( strcmp(text, "mtd_file_mask"   ) == 0 )   { t = mtd_file_mask;      return true; }
else if ( strcmp(text, "mtd_file_object" ) == 0 )   { t = mtd_file_object;    return true; }
else if ( strcmp(text, "no_mtd_file_type") == 0 )   { t = no_mtd_file_type;   return true; }
   //
   //  nope
   //

return false;

}


////////////////////////////////////////////////////////////////////////


