// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __WWMCA_UTILS_H__
#define  __WWMCA_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include "wwmca_regrid_Conf.h"

#include "interp_base.h"
#include "max_interp.h"
#include "min_interp.h"
#include "ave_interp.h"
#include "nearest_interp.h"

#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Is the given grid confined to either the north
   //
   //    or south hemisphere, or does it straddle the
   //
   //    equator?
   //


extern GridHemisphere find_grid_hemisphere(const Grid &);


////////////////////////////////////////////////////////////////////////


   //
   //  Create an interpolator instance given the info
   //
   //    from the config file
   //


extern Interpolator * get_interpolator(wwmca_regrid_Conf &);


////////////////////////////////////////////////////////////////////////


   //
   //  Create the "to" grid from the grid info string in the config file
   //

// extern GridInfo get_grid(const char * gridinfo_string);


////////////////////////////////////////////////////////////////////////


#endif   /*  __WWMCA_UTILS_H__  */


////////////////////////////////////////////////////////////////////////


