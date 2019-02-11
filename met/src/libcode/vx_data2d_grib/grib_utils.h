// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __GRIB1_UTILS_H__
#define  __GRIB1_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include "vx_grid.h"

#include "grib_classes.h"


////////////////////////////////////////////////////////////////////////

extern void gds_to_grid (const Section2_Header & gds, Grid & gr);

extern void gds_to_order(const Section2_Header & gds, int & xdir, int & ydir, int & order);

extern void instantiate_grid(GribFile &, int rec_num, Grid & out);

////////////////////////////////////////////////////////////////////////


#endif   /*  __GRIB1_UTILS_H__  */


////////////////////////////////////////////////////////////////////////

