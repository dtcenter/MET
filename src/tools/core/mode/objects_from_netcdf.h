// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_OBJECTS_FROM_NETCDF_H__
#define  __MODE_OBJECTS_FROM_NETCDF_H__


////////////////////////////////////////////////////////////////////////


#include "two_d_array.h"


////////////////////////////////////////////////////////////////////////


//
//  grabs the objects from a MODE output netcdf file
//


////////////////////////////////////////////////////////////////////////
/* extern void  objects_from_arrays(bool do_clusters, */
/*                                  int *fcst_objects, int *obs_objects, int nx, int ny, */
/*                                  BoolPlane & fcst_out,  */
/*                                  BoolPlane & obs_out); */


extern void objects_from_netcdf(const char * netcdf_filename, 
                                bool do_clusters,     //  do we look at cluster objects or simple objects?
                                BoolPlane & fcst_out, 
                                BoolPlane & obs_out);
                                


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_OBJECTS_FROM_NETCDF_H__  */


////////////////////////////////////////////////////////////////////////


