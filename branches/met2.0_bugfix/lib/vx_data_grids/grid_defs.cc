// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <string.h>
#include <cmath>

#include <vx_data_grids/grid_defs.h>



////////////////////////////////////////////////////////////////////////


   //
   //  these need external linkage
   //  see :  <data_grids/grid.h>
   //


Grid wrf10 ( wrf10_data);
Grid wrf22 ( wrf22_data);

Grid ruc40 ( ruc40_data);
Grid ruc20 ( ruc20_data);
Grid ruc13 ( ruc13_data);

Grid eta212(eta212_data);

Grid stage4(stage4_data);
Grid stage4_2002(stage4_2002_data);
Grid alaska(alaska_data);

Grid afwa(afwa_data);
Grid afwa2(afwa2_data);

Grid hires_alaska(hires_alaska_data);

Grid test_exp(test_exp_data);

Grid bamex_wrf(bamex_wrf_data);

Grid ncwd(ncwd_data);

Grid ncwf(ncwf_data);

Grid ncwf2_4km(ncwf2_4km_data);

Grid ncwf2_8km(ncwf2_8km_data);

Grid ncwf2_2005(ncwf2_2005_data);

Grid ow_conus(ow_conus_data);

Grid ow_gom(ow_gom_data);

Grid ow_pac(ow_pac_data);

Grid ow_nopac(ow_nopac_data);

Grid ow_world(ow_world_data);

Grid agrmet_north(agrmet_north_data);

Grid spc(spc_data);   //  baldwin data

Grid ts_wrf_june(ts_wrf_june_data); // baldwin data

Grid wrf8(wrf8_data);

Grid hirescip(hirescip_data);

Grid goesruc5(goesruc5_data);

Grid ndfd5km(ndfd5km_data);

Grid anc_ilia2004_brightband(anc_ilia2004_brightband_data);

Grid anc_ilia2004_gandi60(anc_ilia2004_gandi60_data);

Grid anc_dfw2005_brightband(anc_dfw2005_brightband_data);

Grid anc_dfw2005_gandi60(anc_dfw2005_gandi60_data);

Grid anc_dfw2006_cronus(anc_dfw2006_cronus_data);

Grid anc_dfw2006_merged(anc_dfw2006_merged_data);

////////////////////////////////////////////////////////////////////////
//
// Search for the data grid matching the string provided.  If found,
// instantiate the grid and return 0.  If not found, return 1.
//
////////////////////////////////////////////////////////////////////////

int find_grid(const char *grid_name, Grid &in_grid) {
   int i;

   //
   // Search the PlateCarree Data array for the grid name specified
   //
   for(i=0; i<n_pc_grids_data; i++) {
      if(strcmp(pc_grids_data[i].name, grid_name) == 0) {
         Grid out_grid(pc_grids_data[i]);
         in_grid = out_grid;
         return(0);
      }
   }

   //
   // Search the Stereographic Data array for the grid name specified
   //
   for(i=0; i<n_st_grids_data; i++) {
      if(strcmp(st_grids_data[i].name, grid_name) == 0) {
         Grid out_grid(st_grids_data[i]);
         in_grid = out_grid;
         return(0);
      }
   }

   //
   // Search the Lambert Data array for the grid name specified
   //
   for(i=0; i<n_lc_grids_data; i++) {
      if(strcmp(lc_grids_data[i].name, grid_name) == 0) {
         Grid out_grid(lc_grids_data[i]);
         in_grid = out_grid;
         return(0);
      }
   }

   //
   // Search the Mercator Data array for the grid name specified
   //
   for(i=0; i<n_merc_grids_data; i++) {
      if(strcmp(merc_grids_data[i].name, grid_name) == 0) {
         Grid out_grid(merc_grids_data[i]);
         in_grid = out_grid;
         return(0);
      }
   }

   // If a matching grid was not found, return 1
   return(1);
}

////////////////////////////////////////////////////////////////////////
