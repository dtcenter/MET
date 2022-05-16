// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
   //     Created by enum_to_string from file "data_file_type.h"
   //


////////////////////////////////////////////////////////////////////////


#ifndef  __GRDFILETYPE_TO_STRING_H__
#define  __GRDFILETYPE_TO_STRING_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "data_file_type.h"


////////////////////////////////////////////////////////////////////////


extern ConcatString grdfiletype_to_string(const GrdFileType);


extern bool string_to_grdfiletype(const char *, GrdFileType &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __GRDFILETYPE_TO_STRING_H__  */


////////////////////////////////////////////////////////////////////////


