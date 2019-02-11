// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SATELLITE_UTILS_H__
#define  __SATELLITE_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static const int nt_none_query      =  0;
static const int nt_version         =  1;
 
static const int nt_uchar_8         =  3;
static const int  nt_char_8         =  4;
 
static const int nt_float_32        =  5;
static const int nt_float_64        =  6;
static const int nt_float_128       =  7;
 
static const int  nt_int_8          = 20;
static const int nt_uint_8          = 21;
 
static const int  nt_int_16         = 22;
static const int nt_uint_16         = 23;
 
static const int  nt_int_32         = 24;
static const int nt_uint_32         = 25;
 
static const int  nt_int_64         = 26;
static const int nt_uint_64         = 27;
 
static const int  nt_int_128        = 28;
static const int nt_uint_128        = 29;
 
static const int nt_char16_uchar16  = 42;


////////////////////////////////////////////////////////////////////////


extern void parse_csl(void * line, StringArray & a);

// extern void numbertype_to_string(const int, char * out);

extern ConcatString numbertype_to_string(const int);


////////////////////////////////////////////////////////////////////////


#endif   /*  __SATELLITE_UTILS_H__   */


////////////////////////////////////////////////////////////////////////


