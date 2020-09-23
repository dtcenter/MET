// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __FIX_FLOAT_H__
#define  __FIX_FLOAT_H__

#include "concat_string.h"

////////////////////////////////////////////////////////////////////////


extern void fix_float(ConcatString &);

extern void fix_float_with_blanks(ConcatString &);

extern void fix_float_with_char(ConcatString &, const char replacement_char);


////////////////////////////////////////////////////////////////////////


#endif   /*  __FIX_FLOAT_H__  */


////////////////////////////////////////////////////////////////////////


