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

////////////////////////////////////////////////////////////////////////

extern bool python_tc_diag(const ConcatString &script_name,
               const ConcatString &tmp_file_name,
               std::map<std::string,std::string> &diag_map);

////////////////////////////////////////////////////////////////////////

#endif   /*  __PYTHON_TC_DIAG__  */

////////////////////////////////////////////////////////////////////////
