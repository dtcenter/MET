// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////


#ifndef  __DATA2D_FACTORY_UTILS_H__
#define  __DATA2D_FACTORY_UTILS_H__


///////////////////////////////////////////////////////////////////////////////


#include "vx_data2d.h"


///////////////////////////////////////////////////////////////////////////////


extern GrdFileType grd_file_type(const char * filename);

extern void update_mtddf_grid(Met2dDataFile *, VarInfo *);


///////////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA2D_FACTORY_UTILS_H__  */

///////////////////////////////////////////////////////////////////////////////
