

////////////////////////////////////////////////////////////////////////


#ifndef  __MERCATOR_GRID_DEFINITIONS_H__
#define  __MERCATOR_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct MercatorData {

   const char * name;

   double lat_ll_deg;
   double lon_ll_deg;

   double lat_ur_deg;
   double lon_ur_deg;

   // double delta_i;
   // double delta_j;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Mercator grid definitions
   //

                                           //              name    lat_ll_deg  lon_ll_deg  lat_ur_deg  lon_ur_deg   Nx    Ny
                                           //   ===============    ==========  ==========  ==========  ==========  ===   ===
static const MercatorData fw_test_data = {     "Full World Test",       -25.0,      110.0,     60.0  ,   110.0  ,  500,  100 };
static const MercatorData g204_data    = {                "G204",       -25.0,     -110.0,     60.644,   109.129,   93,   68 };


////////////////////////////////////////////////////////////////////////


#endif   /*  __MERCATOR_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



