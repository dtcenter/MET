

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "icodecell_to_result.h"
#include "celltype_to_string.h"
#include "resulttype_to_string.h"


////////////////////////////////////////////////////////////////////////


void icodecell_to_result(const IcodeCell & cell, Result & result)

{

char junk[256];
bool tf = false;


result.clear();

switch ( cell.type )  {

   case integer:
      result.set_int(cell.val);
      break;

   case boolean:
      tf = ( cell.val ? true : false );
      result.set_boolean(tf);
      break;

   case floating_point:
      result.set_double(cell.d);
      break;


   case character_string:
      result.set_string(cell.text);
      break;


   default:
      celltype_to_string(cell.type, junk);
      cerr << "\n\n  icodecell_to_result() -> don't know how to handle icode cell type \""
           << junk << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


void result_to_icodecell(const Result & result, IcodeCell & cell)

{

char junk[256];

cell.clear();

switch ( result.type() )  {

   case result_int:
      cell.set_integer(result.ival());
      break;


   case result_boolean:
      cell.set_boolean(result.bval());
      break;


   case result_double:
      cell.set_double(result.dval());
      break;


   case result_string:
      cell.set_string(result.sval());
      break;


   default:
      resulttype_to_string(result.type(), junk);
      cerr << "\n\n  result_to_icodecell() -> don't know how to handle result type \""
           << junk << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch




return;

}


////////////////////////////////////////////////////////////////////////


