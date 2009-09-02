// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_GRIDS_GRID_H__
#define  __DATA_GRIDS_GRID_H__


////////////////////////////////////////////////////////////////////////


   //
   //  grid classes by Randy Bullock
   //
   // This is a top level example file that has various specific
   // Grid objects commonly used within verification
   //
   // See also: grid_defs.h, grid.cc
   //


////////////////////////////////////////////////////////////////////////


#include <vx_data_grids/st_grid.h>
#include <vx_data_grids/lc_grid.h>
#include <vx_data_grids/exp_grid.h>
#include <vx_data_grids/pc_grid.h>
#include <vx_data_grids/merc_grid.h>
#include <vx_data_grids/grid_defs.h>


////////////////////////////////////////////////////////////////////////


extern const Grid wrf10;
extern const Grid wrf22;

extern const Grid ruc40;
extern const Grid ruc20;
extern const Grid ruc13;

extern const Grid eta212;

extern const Grid stage4;
extern const Grid stage4_2002;
extern const Grid alaska;   //  grid 216 from grib doc

extern const Grid bamex_wrf;

extern const Grid afwa;
extern const Grid afwa2;

extern const Grid hires_alaska;

extern const Grid test_exp;

extern const Grid ncwd;
extern const Grid ncwf;
extern const Grid ncwf2_4km;
extern const Grid ncwf2_8km;
extern const Grid ncwf2_2005;

extern const Grid agrmet_north;

extern const Grid spc;

extern const Grid ts_wrf_june;

extern const Grid wrf8;

extern const Grid hirescip;

extern const Grid goesruc5;

extern const Grid ndfd5km;

extern const Grid ow_conus;
extern const Grid ow_gom;
extern const Grid ow_pac;
extern const Grid ow_nopac;
extern const Grid ow_world;

extern const Grid anc_ilia2004_brightband;
extern const Grid anc_ilia2004_gandi60;
extern const Grid anc_dfw2005_brightband;
extern const Grid anc_dfw2005_gandi60;
extern const Grid anc_dfw2006_cronus;
extern const Grid anc_dfw2006_merged;

////////////////////////////////////////////////////////////////////////


#endif   //  __DATA_GRIDS_GRID_H__


////////////////////////////////////////////////////////////////////////



