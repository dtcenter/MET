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
   //     Created by enum_to_string from file "icode.h"
   //


////////////////////////////////////////////////////////////////////////


#include <string.h>

#include "celltype_to_string.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


ConcatString celltype_to_string(const CellType t)

{

const char * s = (const char *) nullptr;

switch ( t )  {

   case integer:            s = "integer";            break;
   case floating_point:     s = "floating_point";     break;
   case boolean:            s = "boolean";            break;
   case cell_mark:          s = "cell_mark";          break;
   case op_add:             s = "op_add";             break;

   case op_multiply:        s = "op_multiply";        break;
   case op_divide:          s = "op_divide";          break;
   case op_subtract:        s = "op_subtract";        break;
   case op_power:           s = "op_power";           break;
   case op_square:          s = "op_square";          break;

   case op_negate:          s = "op_negate";          break;
   case op_store:           s = "op_store";           break;
   case op_recall:          s = "op_recall";          break;
   case identifier:         s = "identifier";         break;
   case user_func:          s = "user_func";          break;

   case builtin_func:       s = "builtin_func";       break;
   case local_var:          s = "local_var";          break;
   case character_string:   s = "character_string";   break;
   case no_cell_type:       s = "no_cell_type";       break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ConcatString(s);

}


////////////////////////////////////////////////////////////////////////


bool string_to_celltype(const char * text, CellType & t)

{

     if ( strcmp(text, "integer"         ) == 0 )   { t = integer;            return true; }
else if ( strcmp(text, "floating_point"  ) == 0 )   { t = floating_point;     return true; }
else if ( strcmp(text, "boolean"         ) == 0 )   { t = boolean;            return true; }
else if ( strcmp(text, "cell_mark"       ) == 0 )   { t = cell_mark;          return true; }
else if ( strcmp(text, "op_add"          ) == 0 )   { t = op_add;             return true; }

else if ( strcmp(text, "op_multiply"     ) == 0 )   { t = op_multiply;        return true; }
else if ( strcmp(text, "op_divide"       ) == 0 )   { t = op_divide;          return true; }
else if ( strcmp(text, "op_subtract"     ) == 0 )   { t = op_subtract;        return true; }
else if ( strcmp(text, "op_power"        ) == 0 )   { t = op_power;           return true; }
else if ( strcmp(text, "op_square"       ) == 0 )   { t = op_square;          return true; }

else if ( strcmp(text, "op_negate"       ) == 0 )   { t = op_negate;          return true; }
else if ( strcmp(text, "op_store"        ) == 0 )   { t = op_store;           return true; }
else if ( strcmp(text, "op_recall"       ) == 0 )   { t = op_recall;          return true; }
else if ( strcmp(text, "identifier"      ) == 0 )   { t = identifier;         return true; }
else if ( strcmp(text, "user_func"       ) == 0 )   { t = user_func;          return true; }

else if ( strcmp(text, "builtin_func"    ) == 0 )   { t = builtin_func;       return true; }
else if ( strcmp(text, "local_var"       ) == 0 )   { t = local_var;          return true; }
else if ( strcmp(text, "character_string") == 0 )   { t = character_string;   return true; }
else if ( strcmp(text, "no_cell_type"    ) == 0 )   { t = no_cell_type;       return true; }
   //
   //  nope
   //

return false;

}


////////////////////////////////////////////////////////////////////////


