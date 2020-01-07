// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __READ_CLIMO_H__
#define  __READ_CLIMO_H__

////////////////////////////////////////////////////////////////////////

#include "vx_config.h"
#include "vx_grid.h"
#include "vx_cal.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

extern DataPlane read_climo_data_plane(Dictionary *, int,
                                       unixtime, const Grid &);

extern DataPlaneArray read_climo_data_plane_array(Dictionary *, int,
                                                  unixtime, const Grid &);

////////////////////////////////////////////////////////////////////////

#endif   // __READ_CLIMO_H__

////////////////////////////////////////////////////////////////////////
