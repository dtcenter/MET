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
   //
   //     Created by enum_to_string from file "mtd_file_base.h"
   //
   //     on April 27, 2022   2:39 pm MDT
   //


////////////////////////////////////////////////////////////////////////


#ifndef  __MTDFILETYPE_TO_STRING_H__
#define  __MTDFILETYPE_TO_STRING_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "mtd_file_base.h"


////////////////////////////////////////////////////////////////////////


extern ConcatString mtdfiletype_to_string(const MtdFileType);


extern bool string_to_mtdfiletype(const char *, MtdFileType &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTDFILETYPE_TO_STRING_H__  */


////////////////////////////////////////////////////////////////////////


