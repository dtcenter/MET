

////////////////////////////////////////////////////////////////////////


#ifndef  __RADIANCE_OFFSETS_H__
#define  __RADIANCE_OFFSETS_H__


////////////////////////////////////////////////////////////////////////


   //
   //  these indices are 1-based
   //


static const int lat_index              =  1;    // observation latitude (degrees)
static const int lon_index              =  2;    // observation longitude (degrees)
static const int elevation_index        =  3;    // model elevation (meters)
static const int dtime_index            =  4;    // observation time (hours relative to analysis time)

static const int scanpos_index          =  5;    // scan position
static const int sat_zenith_angle_index =  6;    // satellite zenith angle
static const int sat_azimuth_index      =  7;    // satellite azimuth angle
static const int sun_zenith_angle_index =  8;    // solar zenith angle
static const int sun_azimuth_index      =  9;    // solar azimuth angle

static const int glint_index            = 10;    // solar glint angle
static const int water_frac_index       = 11;    // fractional water coverage
static const int land_frac_index        = 12;    // fractional land coverage
static const int ice_frac_index         = 13;    // fractional ice coverage
static const int snow_frac_index        = 14;    // fractional snow coverage

static const int water_temp_index       = 15;    // surface temperature over water
static const int land_temp_index        = 16;    // surface temperature over land
static const int ice_temp_index         = 17;    // surface temperature over ice
static const int snow_temp_index        = 18;    // surface temperature over snow
static const int soil_temp_index        = 19;    // soil temperature

static const int soil_moisture_index    = 20;    // soil moisture
static const int land_type_index        = 21;    // surface land type
static const int veg_frac_index         = 22;    // vegetation fraction
static const int snow_depth_index       = 23;    // snow depth
static const int wind_speed_index       = 24;    // wind speed (m/s)


////////////////////////////////////////////////////////////////////////


static const int btemp_chan_index       = 1;     //  observed brightness temperature
static const int omg_bc_chan_index      = 2;     //  obs - guess brightness temperature (with bias correction)
static const int omg_nobc_chan_index    = 3;     //  obs - guess brightness temperature (without bias correction)
static const int inv_chan_index         = 4;     //  inverse observation error


////////////////////////////////////////////////////////////////////////


#endif  /*  __RADIANCE_OFFSETS_H__  */


////////////////////////////////////////////////////////////////////////



