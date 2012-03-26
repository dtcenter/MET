

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"
#include "icodecell_to_result.h"
#include "celltype_to_string.h"
#include "resulttype_to_string.h"


////////////////////////////////////////////////////////////////////////


void icodecell_to_result(const IcodeCell & cell, Result & result)

{

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
      mlog << Error << "\nicodecell_to_result() -> "
           << "don't know how to handle icode cell type \""
           << celltype_to_string(cell.type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


void result_to_icodecell(const Result & result, IcodeCell & cell)

{

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
      mlog << Error << "\nresult_to_icodecell() -> "
           << "don't know how to handle result type \""
           << resulttype_to_string(result.type()) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch




return;

}


////////////////////////////////////////////////////////////////////////


