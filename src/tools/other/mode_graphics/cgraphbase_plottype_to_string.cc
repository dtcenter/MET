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
   //     Created by enum_to_string from file "cgraph_main.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "cgraphbase_plottype_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString cgraphbase_plottype_to_string(const CgraphBase::PlotType t)

{

const char * s = (const char *) nullptr;

switch ( t )  {

   case CgraphBase::png_type:              s = "CgraphBase::png_type";              break;
   case CgraphBase::ps_type:               s = "CgraphBase::ps_type";               break;
   case CgraphBase::eps_type:              s = "CgraphBase::eps_type";              break;
   case CgraphBase::svg_type:              s = "CgraphBase::svg_type";              break;
   case CgraphBase::buffer_type:           s = "CgraphBase::buffer_type";           break;

   case CgraphBase::no_cgraph_plot_type:   s = "CgraphBase::no_cgraph_plot_type";   break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ConcatString(s);

}


////////////////////////////////////////////////////////////////////////


