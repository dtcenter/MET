// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __COMPUTE_EAS_H__
#define  __COMPUTE_EAS_H__

////////////////////////////////////////////////////////////////////////

#include <vector>

#include "vx_grid.h"
#include "config_constants.h"
#include "data_plane.h"

////////////////////////////////////////////////////////////////////////

extern void compute_eas(const std::vector<DataPlane> &,
                        const EASProbInfo &, const Grid &,
                        DataPlane &, DataPlane &, DataPlane &);

////////////////////////////////////////////////////////////////////////

#endif   //  __COMPUTE_EAS_H__

////////////////////////////////////////////////////////////////////////
