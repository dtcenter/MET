

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


   //
   //  Exp grid definitions
   //


static const ExpData beijing_steps_data = { 

   "beijing_steps", 
   39.809, -116.472, 
   50.191, 63.528, 
   2.0, 2.0, 
   127.0, 127.0, 
   256, 256

};


static const ExpData beijing_bjanc_data = { 

   "beijing_bjanc", 
   37.52561, -113.6374, 
   52.47439, 66.3653, 
   1.0, 1.0, 
   0.0, 0.0, 
   500, 500

};




////////////////////////////////////////////////////////////////////////


#endif   /*  __EXPONENTIAL_GRID_DEFINITIONS_H__  */



////////////////////////////////////////////////////////////////////////



