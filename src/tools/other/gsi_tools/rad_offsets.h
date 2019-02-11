// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __RADIANCE_OFFSETS_H__
#define  __RADIANCE_OFFSETS_H__


////////////////////////////////////////////////////////////////////////

   //
   //  these indices are 1-based
   //

////////////////////////////////////////////////////////////////////////


   //
   //  diag
   //


static const int rad_lat_index                =  1; // observation latitude (degrees)
static const int rad_lon_index                =  2; // observation longitude (degrees)
static const int rad_elevation_index          =  3; // model elevation (meters)
static const int rad_dtime_index              =  4; // observation time (hours relative to analysis time)

static const int rad_scanpos_index            =  5; // scan position
static const int rad_sat_zenith_index         =  6; // satellite zenith angle
static const int rad_sat_azimuth_index        =  7; // satellite azimuth angle
static const int rad_sun_zenith_index         =  8; // solar zenith angle
static const int rad_sun_azimuth_index        =  9; // solar azimuth angle

static const int rad_glint_index              = 10; // solar glint rad_angle
static const int rad_water_frac_index         = 11; // fractional water coverage
static const int rad_land_frac_index          = 12; // fractional land coverage
static const int rad_ice_frac_index           = 13; // fractional ice coverage
static const int rad_snow_frac_index          = 14; // fractional snow coverage

static const int rad_water_temp_index         = 15; // surface temperature over water
static const int rad_land_temp_index          = 16; // surface temperature over land
static const int rad_ice_temp_index           = 17; // surface temperature over ice
static const int rad_snow_temp_index          = 18; // surface temperature over snow
static const int rad_soil_temp_index          = 19; // soil temperature

static const int rad_soil_moisture_index      = 20; // soil moisture
static const int rad_land_type_index          = 21; // surface land type
static const int rad_veg_frac_index           = 22; // vegetation fraction
static const int rad_snow_depth_index         = 23; // snow depth
static const int rad_wind_speed_index         = 24; // wind speed (m/s)

static const int rad_cloud_frac_index         = 25; // cloud fraction (%)
static const int rad_cloud_top_pressure_index = 26; // cloud top pressure (hPa)

static const int rad_itref_index              = 27; // foundation temperature: Tr 
static const int rad_idtw_index               = 28; // diurnal warming: d(Tw) at depth zob
static const int rad_idtc_index               = 29; // sub-layer cooling: d(Tc) at depth zob
static const int rad_itz_tr_index             = 30; // d(Tz)/d(Tr)


////////////////////////////////////////////////////////////////////////


   //
   //  diagchan
   //

static const int rad_btemp_chan_index         = 1; // observed brightness temperature
static const int rad_omg_bc_chan_index        = 2; // obs - guess brightness temperature (with bias correction)
static const int rad_omg_nobc_chan_index      = 3; // obs - guess brightness temperature (without bias correction)
static const int rad_inv_chan_index           = 4; // inverse observation error
static const int rad_qc_mark_index            = 5; // quality control mark or event indicator
static const int rad_surf_em_index            = 6; // surface emissivity
static const int rad_stability_index          = 7; // stability index


////////////////////////////////////////////////////////////////////////


   //
   //  extra
   //

   
static const int rad_extra_prs_max_wgt_index  = 1; // pressure of maximum weighting function 


////////////////////////////////////////////////////////////////////////


#endif  /*  __RADIANCE_OFFSETS_H__  */


////////////////////////////////////////////////////////////////////////



