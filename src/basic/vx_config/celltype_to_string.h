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


#ifndef  __CELLTYPE_TO_STRING_H__
#define  __CELLTYPE_TO_STRING_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "icode.h"


////////////////////////////////////////////////////////////////////////


extern ConcatString celltype_to_string(const CellType);


extern bool string_to_celltype(const char *, CellType &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __CELLTYPE_TO_STRING_H__  */


////////////////////////////////////////////////////////////////////////


