

////////////////////////////////////////////////////////////////////////


#ifndef  __EXPONENTIAL_GRID_DEFINITIONS_H__
#define  __EXPONENTIAL_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct ExpData {

   const char * name;

   double lat_origin_deg;
   double lon_origin_deg;

   double lat_2_deg;
   double lon_2_deg;

   double x_scale;   //  grid-units per fake-kilometer
   double y_scale;

   double x_offset;   //  grid-units
   double y_offset;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __EXPONENTIAL_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



