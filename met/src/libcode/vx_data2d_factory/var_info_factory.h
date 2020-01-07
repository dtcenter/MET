// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_FACTORY_H__
#define __VAR_INFO_FACTORY_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"
#include "concat_string.h"
#include "grdfiletype_to_string.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfoFactory
{
   public:
      static VarInfo *new_var_info(GrdFileType t);
      static VarInfo *new_var_info(ConcatString s);
};

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_FACTORY_H__

///////////////////////////////////////////////////////////////////////////////
