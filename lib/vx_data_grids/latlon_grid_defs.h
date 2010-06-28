

////////////////////////////////////////////////////////////////////////


#ifndef  __LATLON_GRID_DEFINITIONS_H__
#define  __LATLON_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct LatLonData {

   const char * name;   //  not allocated

   double lat_ll_deg;
   double lon_ll_deg;

   double delta_lat_deg;
   double delta_lon_deg;

   int Nlat;
   int Nlon;

};


////////////////////////////////////////////////////////////////////////


   //
   //  LatLon grid definitions
   //


                                                //  name        lat_ll_deg   lon_ll_deg   delta_lat_deg   delta_lon_deg       Nlat   Nlon
                                                //  ====        ==========   ==========   =============   =============       ====   ====
static const LatLonData paul_data           = {    "paul",            27.0,       113.0,           0.25,           0.25,        89,   141 };
static const LatLonData ar_data             = {    "ar",              15.0,       160.0,           0.25,           0.25,       160,   200 };
static const LatLonData ar_int_data         = {    "ar_int",          15.0,       160.0,           0.50,           0.50,        80,   100 };
static const LatLonData hurr_data           = {    "hurr",            20.0,       110.0,           0.10,           0.10,       301,   451 };

static const LatLonData beijing_cards_data  = {    "beijing_cards",   37.4089,   -114.0719,        0.02,           0.02,       240,   240 };
static const LatLonData beijing_grapes_data = {    "beijing_grapes",  36.08,     -112.2,           0.02,           0.02,       300,   400 };
static const LatLonData beijing_maple_data  = {    "beijing_maple",   37.56,     -113.54,          0.01,           0.01,       451,   587 };
static const LatLonData beijing_swirls_data = {    "beijing_swirls",  38.91125,  -115.3365,        0.018,          0.023174,   125,   100 };


////////////////////////////////////////////////////////////////////////


#endif   /*  __LATLON_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



