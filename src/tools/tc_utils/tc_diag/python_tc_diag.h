// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __PYTHON_TC_DIAG__
#define  __PYTHON_TC_DIAG__

////////////////////////////////////////////////////////////////////////

extern "C" {

#include "Python.h"

}

#include "tc_diag.h"

////////////////////////////////////////////////////////////////////////

extern bool python_tc_diag(const ConcatString &script_name,
               TmpFileInfo &tmp_info);

////////////////////////////////////////////////////////////////////////

#endif   /*  __PYTHON_TC_DIAG__  */

////////////////////////////////////////////////////////////////////////
