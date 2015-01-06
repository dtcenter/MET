

////////////////////////////////////////////////////////////////////////


#ifndef  __RADIANCE_OFFSETS_H__
#define  __RADIANCE_OFFSETS_H__


////////////////////////////////////////////////////////////////////////


   //
   //  these indices are 1-based
   //


static const int obstype_index         =  1;    // observation type
static const int obssubtype_index      =  2;    // observation subtype
static const int lat_index             =  3;    // observation latitude (degrees)
static const int lon_index             =  4;    // observation longitude (degrees)

static const int elevation_index       =  5;    // station elevation (meters)
static const int pressure_index        =  6;    // observation pressure (hPa)
static const int height_index          =  7;    // observation height (meters)
static const int obs_hours_index       =  8;    // obs time (hours relative to analysis time)

static const int input_qc_index        =  9;    // input prepbufr qc or event mark
static const int setup_qc_index        = 10;    // setup qc or event mark (currently qtflg only)
static const int usage_index           = 11;    // read_prepbufr data usage flag
static const int analysis_use_index    = 12;    // analysis usage flag (1=use, -1=not used)

static const int qc_weight_index       = 13;    // nonlinear qc relative weight
static const int pb_inverse_index      = 14;    // prepbufr inverse obs error (K**-1)
static const int read_pb_inverse_index = 15;    // read_prepbufr inverse obs error (K**-1)
static const int final_inverse_index   = 16;    // final inverse observation error (K**-1)

static const int obs_data_index        = 17;    // observation
static const int omg_index             = 18;    // obs-ges used in analysis (K)
static const int omg_no_bias_index     = 19;    // obs-ges w/o bias correction (K) (future slot)


////////////////////////////////////////////////////////////////////////


#endif  /*  __RADIANCE_OFFSETS_H__  */


////////////////////////////////////////////////////////////////////////



