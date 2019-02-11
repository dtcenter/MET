// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __ENUM_CODE_H__
#define  __ENUM_CODE_H__


////////////////////////////////////////////////////////////////////////


#include "info.h"


////////////////////////////////////////////////////////////////////////


extern void write_header    (const EnumInfo &);

extern void write_cs_header (const EnumInfo &);

extern void write_source    (const EnumInfo &);

extern void write_cs_source (const EnumInfo &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __ENUM_CODE_H__  */


////////////////////////////////////////////////////////////////////////


