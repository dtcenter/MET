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
   //     Created by enum_to_string from file "object_types.h"
   //


////////////////////////////////////////////////////////////////////////


#include <string.h>

#include "configobjecttype_to_string.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


ConcatString configobjecttype_to_string(const ConfigObjectType t)

{

const char * s = (const char *) nullptr;

switch ( t )  {

   case IntegerType:             s = "IntegerType";             break;
   case FloatType:               s = "FloatType";               break;
   case BooleanType:             s = "BooleanType";             break;
   case StringType:              s = "StringType";              break;
   case DictionaryType:          s = "DictionaryType";          break;

   case ArrayType:               s = "ArrayType";               break;
   case PwlFunctionType:         s = "PwlFunctionType";         break;
   case ThresholdType:           s = "ThresholdType";           break;
   case UserFunctionType:        s = "UserFunctionType";        break;
   case no_config_object_type:   s = "no_config_object_type";   break;


   default:
      s = "(bad value)";
      break;

}   //  switch


return ConcatString (s);

}


////////////////////////////////////////////////////////////////////////


