// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __ICODECELL_TO_RESULT_H__
#define  __ICODECELL_TO_RESULT_H__


////////////////////////////////////////////////////////////////////////


#include "icode.h"
#include "result.h"


////////////////////////////////////////////////////////////////////////


extern void icodecell_to_result(const IcodeCell &, Result &);

extern void result_to_icodecell(const Result &, IcodeCell &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __ICODECELL_TO_RESULT_H__  */


////////////////////////////////////////////////////////////////////////


