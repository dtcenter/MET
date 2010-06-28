

////////////////////////////////////////////////////////////////////////


#ifndef  __LAMBERT_GRID_DEFINITIONS_H__
#define  __LAMBERT_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct LambertData {

   const char * name;

   double scale_lat_1;     //  scale latitude #1
   double scale_lat_2;     //  scale latitude #2

   double lat_pin;
   double lon_pin;

   double x_pin;
   double y_pin;

   double lon_orient;            //  alignment longitude

   double d_km;
   double r_km;

   int nx;
   int ny;

};


////////////////////////////////////////////////////////////////////////


   //
   //  Lambert grid definitions
   //

                                               //          name    scale_lat_1   scale_lat_2    lat_pin    lon_pin   x_pin  y_pin    lon_orient     d_km        r_km    nx    ny
                                               //  ============    ===========   ===========   ========   ========   =====  =====    ==========   ======     =======   ===   ===
static const LambertData wrf22_data        = {           "wrf22",         30.0,         60.0,  19.86599,  124.1012,    0.0,   0.0,         98.0,  22.0,      6367.47,  259,  163 };
static const LambertData wrf_ihop_data     = {        "wrf_ihop",         30.0,         60.0,  23.9651,   112.565,     0.0,   0.0,         98.0,   3.0,      6367.47,  799,  749 };
static const LambertData ihop_reduced_data = {    "ihop_reduced",         30.0,         60.0,  23.9651,   112.565,     0.0,   0.0,         98.0,  11.985,    6367.47,  200,  187 };
                                                                                                                                                                             
static const LambertData wrf_full_data     = {    "wrf_full",             45.0,         45.0,   1.06576,  128.4995,    0.0,   0.0,         95.0,  13.545,    6367.47,  648,  647 };
static const LambertData g218_data         = {    "g218",                 25.0,         25.0,  12.190,    133.459,     0.0,   0.0,         95.0,  12.19058,  6367.47,  614,  428 };
static const LambertData tina_data         = {    "tina",                 39.3,         39.3,  31.5527,   106.7635,    0.0,   0.0,        110.0,  3.1,       6367.47,  910,  602 };

static const LambertData cindy_test_data   = {    "cindy_test",           30.0,         60.0,  34.83001,   81.03,     36.5,  30.0,         98.0,  30.0,      6367.47,   73,   60 };
static const LambertData crystal_data      = {    "crystal_test",         30.0,         60.0,  34.337,     94.957,     0.0,   0.0,         89.0,   5.0,      6367.47,  240,  216 };

static const LambertData julie_test_data   = {    "julie_test",           30.0,         60.0,  34.7848,   104.0818,    0.0,   0.0,         98.0,  30.0,      6370.0,   44,   44 };


////////////////////////////////////////////////////////////////////////


#endif   /*  __LAMBERT_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



