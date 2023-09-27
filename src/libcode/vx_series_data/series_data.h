// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __SERIES_DATA_H__
#define  __SERIES_DATA_H__

////////////////////////////////////////////////////////////////////////

#include <vector>

#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////

bool get_series_entry(int, VarInfo*, const StringArray&,
                      const GrdFileType, DataPlane&, Grid&,
                      bool error_out=true,
                      bool print_warning=true);

bool get_series_entries(int, std::vector<VarInfo*>&, const StringArray&,
                        const GrdFileType, std::vector<DataPlane>&, Grid&,
                        bool error_out=true,
                        bool print_warning=true);

////////////////////////////////////////////////////////////////////////

#endif  // __SERIES_DATA_H__

////////////////////////////////////////////////////////////////////////
