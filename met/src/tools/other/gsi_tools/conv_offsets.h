// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __CONV_OFFSETS_H__
#define  __CONV_OFFSETS_H__


////////////////////////////////////////////////////////////////////////


   //
   //  these indices are 1-based
   //


static const int conv_obstype_index         =  1;    // observation type
static const int conv_obssubtype_index      =  2;    // observation subtype
static const int conv_lat_index             =  3;    // observation latitude (degrees)
static const int conv_lon_index             =  4;    // observation longitude (degrees)

static const int conv_elevation_index       =  5;    // station elevation (meters)
static const int conv_pressure_index        =  6;    // observation pressure (hPa)
static const int conv_height_index          =  7;    // observation height (meters)
static const int conv_obs_hours_index       =  8;    // obs time (hours relative to analysis time)

static const int conv_input_qc_index        =  9;    // input prepbufr qc or event mark
static const int conv_setup_qc_index        = 10;    // setup qc or event mark (currently qtflg only)
static const int conv_usage_index           = 11;    // read_prepbufr data usage flag
static const int conv_analysis_use_index    = 12;    // analysis usage flag (1=use, -1=not used)

static const int conv_qc_weight_index       = 13;    // nonlinear qc relative weight
static const int conv_pb_inverse_index      = 14;    // prepbufr inverse obs error (K**-1)
static const int conv_read_pb_inverse_index = 15;    // read_prepbufr inverse obs error (K**-1)
static const int conv_final_inverse_index   = 16;    // final inverse observation error (K**-1)

static const int conv_obs_data_index        = 17;    // observation
static const int conv_omg_index             = 18;    // obs-ges used in analysis (K)
static const int conv_omg_no_bias_index     = 19;    // obs-ges w/o bias correction (K) (future slot)

static const int conv_obs_v_data_index      = 20;    // v-wind observation
static const int conv_omg_v_index           = 21;    // v-wind obs-ges using in analysis


////////////////////////////////////////////////////////////////////////


#endif  /*  __CONV_OFFSETS_H__  */


////////////////////////////////////////////////////////////////////////



