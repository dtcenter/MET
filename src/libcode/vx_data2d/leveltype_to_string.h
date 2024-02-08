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
   //     Created by enum_to_string from file "level_info.h"
   //


////////////////////////////////////////////////////////////////////////


#ifndef  __LEVELTYPE_TO_STRING_H__
#define  __LEVELTYPE_TO_STRING_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "level_info.h"


////////////////////////////////////////////////////////////////////////


extern ConcatString leveltype_to_string(const LevelType);


extern bool string_to_leveltype(const char *, LevelType &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __LEVELTYPE_TO_STRING_H__  */


////////////////////////////////////////////////////////////////////////


