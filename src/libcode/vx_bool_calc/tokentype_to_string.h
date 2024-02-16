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
   //     Created by enum_to_string from file "token.h"
   //


////////////////////////////////////////////////////////////////////////


#ifndef  __TOKENTYPE_TO_STRING_H__
#define  __TOKENTYPE_TO_STRING_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "token.h"


////////////////////////////////////////////////////////////////////////


extern ConcatString tokentype_to_string(const TokenType);


extern bool string_to_tokentype(const char *, TokenType &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __TOKENTYPE_TO_STRING_H__  */


////////////////////////////////////////////////////////////////////////


