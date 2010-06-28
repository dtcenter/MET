

////////////////////////////////////////////////////////////////////////


#ifndef  __STEREOGRAPHIC_GRID_DEFINITIONS_H__
#define  __STEREOGRAPHIC_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct StereographicData {

   const char * name;

   char hemisphere;   //  'N' or 'S'

   double scale_lat;

   double lat_pin;
   double lon_pin;

   double x_pin;
   double y_pin;

   double lon_orient;

   double d_km;

   double r_km;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Stereographic grid definitions
   //


                                                    //       name        Hemisphere   scale_lat    lat_pin   lon_pin    x_pin  y_pin  lon_orient      d_km       r_km    nx    ny
                                                    //   ========        ==========   =========    =======   ========   =====  =====  ==========   ========   =======  ====  ====
static const StereographicData st4_ihop_data     = {        "st4_ihop",         'N',       60.0,   23.117,   119.017,     0.0,   0.0,      105.0,   4.762,    6367.47, 1121,  881 };
static const StereographicData spc_data          = {             "spc",         'N',       60.0,   29.8386,  109.9776,    0.0,   0.0,      105.0,   4.7625,   6367.47,  601,  501 };
static const StereographicData wwmca_north_data  = {     "wwmca_north",         'N',       60.0,      90.0,    0.0,     511.0, 511.0,       80.0,  23.79848,  6367.47, 1024, 1024 };
static const StereographicData wwmca_south_data  = {     "wwmca_south",         'S',      -60.0,     -90.0,    0.0,     511.0, 511.0,     -100.0,  23.79848,  6367.47, 1024, 1024 };


////////////////////////////////////////////////////////////////////////


#endif   /*  __STEREOGRAPHIC_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



