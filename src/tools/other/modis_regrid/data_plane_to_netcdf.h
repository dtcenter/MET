// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_PLANE_TO_NETCDF_H__
#define  __DATA_PLANE_TO_NETCDF_H__


////////////////////////////////////////////////////////////////////////

#include "data_plane.h"
#include "vx_grid.h"
#include "var_info.h"

////////////////////////////////////////////////////////////////////////


/**
 * @brief writes a grid to netcdf
 * @param[in] var_name       variable name
 * @param[in] var_long_name  variable long name         
 * @param[in] var_units      variable units
 * @param[in] plane          data 
 * @param[in] grid           grid information
 * @param[out] out_filename  netcdf output filename
 */
extern void write_grid_to_netcdf(const DataPlane & plane, const Grid & grid, const char * out_filename, const char * var_name, const char * var_long_name, const char * var_units);


/**
 * @brief writes a grid to netcdf
 * @param[in] var_info       information about the variable / field
 * @param[in] plane          data 
 * @param[in] grid           grid information
 * @param[out] out_filename  netcdf output filename
 */
extern void write_grid_to_netcdf(const DataPlane & plane, const Grid & grid, const char * out_filename, const VarInfo & var_info);


////////////////////////////////////////////////////////////////////////


#endif   //  __DATA_PLANE_TO_NETCDF_H__


////////////////////////////////////////////////////////////////////////

