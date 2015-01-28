// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_REGRID_H__
#define  __MET_VX_REGRID_H__


////////////////////////////////////////////////////////////////////////


#include "vx_grid.h"
#include "data_plane.h"


////////////////////////////////////////////////////////////////////////


   //
   //  I'm setting the interp params to (void *) for now ... later it'll
   //    probably be a pointer (or reference) to a struct
   //


extern DataPlane upp_regrid (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, void * interp_params);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_REGRID_H__  */


////////////////////////////////////////////////////////////////////////


