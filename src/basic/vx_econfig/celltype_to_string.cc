// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



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

#include "celltype_to_string.h"


////////////////////////////////////////////////////////////////////////


void celltype_to_string(const CellType t, char * out)

{

switch ( t )  {

   case integer :            strcpy(out, "integer");            break;
   case floating_point :     strcpy(out, "floating_point");     break;
   case cell_mark :          strcpy(out, "cell_mark");          break;
   case op_add :             strcpy(out, "op_add");             break;
   case op_multiply :        strcpy(out, "op_multiply");        break;

   case op_divide :          strcpy(out, "op_divide");          break;
   case op_subtract :        strcpy(out, "op_subtract");        break;
   case op_power :           strcpy(out, "op_power");           break;
   case op_square :          strcpy(out, "op_square");          break;
   case op_negate :          strcpy(out, "op_negate");          break;

   case op_store :           strcpy(out, "op_store");           break;
   case op_recall :          strcpy(out, "op_recall");          break;
   case identifier :         strcpy(out, "identifier");         break;
   case builtin_func :       strcpy(out, "builtin_func");       break;
   case character_string :   strcpy(out, "character_string");   break;

   case no_cel_type :        strcpy(out, "no_cel_type");        break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


