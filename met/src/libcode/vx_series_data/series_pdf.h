// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __SERIES_PDF_H__
#define  __SERIES_PDF_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_grid.h"

////////////////////////////////////////////////////////////////////////

#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////

void init_pdf(
    float min, float max, float delta,
    vector<int>& pdf);

void update_pdf(
    float min, float delta,
    vector<int>& pdf,
    const Grid& grid,
    const DataPlane&,
    const MaskPlane&);

////////////////////////////////////////////////////////////////////////

#endif  // __SERIES_PDF_H__

////////////////////////////////////////////////////////////////////////
