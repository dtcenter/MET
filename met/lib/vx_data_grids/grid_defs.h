// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

//
// This header file contains static const definitions used to create
// particular Grid objects commonly used within verification.
// See also: grid.h
//

#ifndef DATA_GRIDS_GRID_DEFS_H
#define DATA_GRIDS_GRID_DEFS_H

#include <vx_data_grids/grid_base.h>

////////////////////////////////////////////////////////////////////////

static const LambertData ts_wrf_june_data = {

   "ts_wrf_june",    //  name

   28.0,             //  secant lat 1
   45.0,             //  secant lat 2

   22.4844,          //  lower-left lat
   111.14,           //  lower-left lon

   92.5,             //  lcen

   4.0,              //  d_km

   6367.47,          // r_km
  
   970,              //  nx
   740               //  ny

};

static const LambertData wrf8_data   = {
                                          "wrf8",
                                          35.0, 35.0,
                                          22.98, 110.84,
                                          98.000,
                                          8.0000, 6367.47,
                                          336, 372
                                       };

static const LambertData wrf10_data  = { "wrf10", 30.0, 60.0, 31.29359, 109.2074, 98.000, 10.0000, 6367.47, 180, 150 };
static const LambertData wrf22_data  = { "wrf22", 30.0, 60.0, 19.86599, 124.1012, 98.000, 22.0000, 6367.47, 259, 163 };
static const LambertData ruc40_data  = { "ruc40", 25.0, 25.0, 16.28100, 126.1380, 95.000, 40.6350, 6367.47, 151, 113 };
static const LambertData ruc20_data  = { "ruc20", 25.0, 25.0, 16.28100, 126.1380, 95.000, 20.3175, 6367.47, 301, 225 };
static const LambertData ruc13_data  = { "ruc13", 25.0, 25.0, 16.28100, 126.1380, 95.000, 13.5450, 6367.47, 451, 337 };
static const LambertData eta212_data = {"eta212", 25.0, 25.0, 12.19000, 133.4590, 95.000, 40.6350, 6367.47, 185, 129 };
static const LambertData afwa_data   = {  "afwa", 60.0, 30.0, 21.19500, 122.9820, 99.109, 15.0000, 6367.47, 342, 210 };
static const LambertData afwa2_data  = { "afwa2", 60.0, 30.0, 20.583  , 122.247 , 96.0  , 15.0000, 6367.47, 342, 210 };
static const LambertData bamex_wrf_data   = {  "bamex_wrf", 45.0, 30.0, 30.4623, 105.444, 95.000,  4.0000, 6367.47, 500, 500 };

static const StereographicData stage4_data       = { "stage4",       60.0, 22.774, 120.376, 105.0,  4.763, 6367.47, 1160, 880 };
static const StereographicData stage4_2002_data  = { "stage4_2002",  60.0, 23.117, 119.017, 105.0,  4.762, 6367.47, 1121, 881 };
static const StereographicData alaska_data       = { "alaska",       60.0, 30.000, 173.000, 135.0, 45.000, 6367.47,  139, 107 };   //  grid 216 from grib doc
static const StereographicData hires_alaska_data = { "hires_alaska", 60.0, 30.000, 173.000, 135.0, 11.250, 6367.47,  553, 425 };   //  grid 216 from grib doc

static const StereographicData spc_data  = {

   "spc",

   60.0,

    29.8386,
   109.9776,

   105.0,

   4.7625,

   6367.47,

   601,
   501

};

static const StereographicData agrmet_north_data = {

   "agrmet_north",    //  name

             60.0,    //  p1_deg

          -19.133,    //  po_deg
          125.0,      //  l0_deg

           80.0,      //  lcen_deg

          46.305,     //  d_km

         6367.47,     //  r_km

            512,      //  nx
            512       //  ny

};


static const ExpData test_exp_data = {

 "test_exp", 40.0, 105.0, 90.0, 0.0, 0.025, 0.025,  75.0,  75.0, 151, 151

};

static const LambertData ncwf2_2005_data  = { "ncwf2_2005", 25.0, 25.0, 16.28100, 126.1380, 95.000, 4.0000, 6367.47, 1500, 1125 };

static const PlateCarreeData ncwd_data = { "ncwd", 20.017969, 129.980881, 0.035933, 0.038239, 918, 1830 };

static const PlateCarreeData ncwf_data = { "ncwf", 20.00899,  129.9904, 1.796406e-2, 1.9120448e-2, 1837, 3661 };

static const PlateCarreeData ncwf2_4km_data = { "ncwf2_4km", 20.017969, 129.980881, 0.035933, 0.038239, 918, 1830 };

static const PlateCarreeData ncwf2_8km_data = { "ncwf2_8km", 20.017969, 129.980881, 0.071866, 0.076478, 460, 915 };

static const PlateCarreeData ow_conus_data = { "ow_conus", 20.848291, 126.472260, 0.035933, 0.044993, 900, 1400 };

static const PlateCarreeData ow_gom_data = { "ow_gom", -14.975, 99.975, 0.05, 0.05, 1000, 1400 };

static const PlateCarreeData ow_pac_data = { "ow_pac", -39.984, -143.021, 0.05, 0.05, 1595, 2250 };

static const PlateCarreeData ow_nopac_data = { "ow_nopac", 15.7903, -119.981, 0.0449158, 0.0640821, 1315, 1250 };

static const PlateCarreeData ow_world_data = { "ow_world", -90.0, -80.0, 1.0, 1.0, 180, 360 };

static const LambertData hirescip_data  = {

   "hirescip",  //  name

   25.0,        //  secant lat 1
   25.0,        //  secant lat 2

   37.932,      //  lower-left lat
   86.665,      //  lower-left lon

   95.000,      //  lcen

   5.0,         //  d_km

   6371.204,    //  r_km

   158,         //  nx
   157          //  ny

};

static const LambertData goesruc5_data = {

   "goesruc5",   //  name

   25.0,         //  secant lat 1
   25.0,         //  secant lat 2

   16.30814,     //  lower-left lat ...  ruc20 coordinates (1.0/8.0, 1.0/8.0)
   126.12043,    //  lower-left lon

   95.000,       //  lcen

   20.3175/4.0,  //  d_km

   6367.47,      //  r_km

   1200,         //  nx
    896          //  ny

};

static const LambertData ndfd5km_data = {

   "ndfd5km",    // name

   25.0,         // secant lat 1
   25.0,         // secant lat 2

   20.192,       // lower-left lat
   121.544,      // lower-left lon

   95.0,         // lcen

   5.079406,     // d_km

   6367.47,      // r_km

   1073,         // nx
    689          // ny

};

static const ExpData anc_ilia2004_brightband_data = {

   "anc_ilia2004_brightband",

   40.3931, 85.9415,

   90.0, 0.0,

   -590.0, -380.0,

   1100, 760
};

static const ExpData anc_ilia2004_gandi60_data = {

   "anc_ilia2004_gandi60",

   40.3931, 85.9415,

   90.0, 0.0,

   2.0, 2.0,

   -490.0, -295.0,

   435, 310
};

static const ExpData anc_dfw2005_brightband_data = {

   "anc_dfw2005_brightband",

   32.03305, 97.16695,

   90.0, 0.0,

   1.0, 1.0,

   -590.0, -380.0,

   1100, 760
};

static const ExpData anc_dfw2005_gandi60_data = {

   "anc_dfw2005_gandi60",

   32.03305, 97.16695,

   90.0, 0.0,

   2.0, 2.0,

   -375.0, -300.0,

   360, 288
};

static const ExpData anc_dfw2006_cronus_data = {

   "anc_dfw2006_cronus",

   32.03305, 97.16695,

   2.0, 2.0,

   -375.0, -250.0,

   360, 330
};

static const ExpData anc_dfw2006_merged_data = {

   "anc_dfw2006_merged",

   32.03305, 97.16695,

   90.0, 0.0,

   1.0,  1.0,

   -590.0, -320.0,

   1100, 820
};

// 3 grids defined for the DTC
static const LambertData dtc_grid164_data = { "DTC164", 30, 48, 20.47, 122.042, 98.8, 13.3, 6367.47, 376, 280 };
static const LambertData dtc_grid165_data = { "DTC165", 30, 48, 20.653, 121.907, 98.8, 13.3, 6367.47, 168, 280 };
static const LambertData dtc_grid166_data = { "DTC166", 30, 48, 23.114, 100.997, 98.8, 13.3, 6367.47, 208, 280 };

// NCEP Grids defined in W3FI71.f of the W3 Library

// 23 Lat/Lon (PlateCarree) NCEP Grid definitions
static const PlateCarreeData ncep_grid002_data = { "G002", -90, -0, 2.5, 2.5, 73, 144 };
static const PlateCarreeData ncep_grid003_data = { "G003", -90, -0, 1, 1, 181, 360 };
static const PlateCarreeData ncep_grid004_data = { "G004", -90, -0, 0.5, 0.5, 361, 720 };
static const PlateCarreeData ncep_grid029_data = { "G029", 0, -0, 2.5, 2.5, 37, 145 };
static const PlateCarreeData ncep_grid030_data = { "G030", -90, -0, 2.5, 2.5, 37, 145 };
static const PlateCarreeData ncep_grid033_data = { "G033", 0, -0, 2, 2, 46, 181 };
static const PlateCarreeData ncep_grid034_data = { "G034", -90, -0, 2, 2, 46, 181 };
static const PlateCarreeData ncep_grid045_data = { "G045", -90, -0, 1.25, 1.25, 145, 288 };
static const PlateCarreeData ncep_grid085_data = { "G085", 0.5, -0.5, 1, 1, 90, 360 };
static const PlateCarreeData ncep_grid086_data = { "G086", -89.5, -0.5, 1, 1, 90, 360 };
static const PlateCarreeData ncep_grid110_data = { "G110", 25.063, 124.938, 0.125, 0.125, 224, 464 };
static const PlateCarreeData ncep_grid175_data = { "G175", 0, -130, 0.09, 0.09, 334, 556 };
static const PlateCarreeData ncep_grid228_data = { "G228", -90, -0, 2.5, 2.5, 73, 144 };
static const PlateCarreeData ncep_grid229_data = { "G229", -90, -0, 1, 1, 181, 360 };
static const PlateCarreeData ncep_grid230_data = { "G230", -90, -0, 0.5, 0.5, 361, 720 };
static const PlateCarreeData ncep_grid231_data = { "G231", 0, -0, 0.5, 0.5, 181, 720 };
static const PlateCarreeData ncep_grid232_data = { "G232", 0, -0, 1, 1, 91, 360 };
static const PlateCarreeData ncep_grid233_data = { "G233", -78, -0, 1, 1.25, 157, 288 };
static const PlateCarreeData ncep_grid234_data = { "G234", -45, 98, 0.25, 0.25, 241, 133 };
static const PlateCarreeData ncep_grid243_data = { "G243", 10, 170, 0.4, 0.4, 101, 126 };
static const PlateCarreeData ncep_grid248_data = { "G248", 14.5, 71.5, 0.075, 0.075, 101, 135 };
static const PlateCarreeData ncep_grid250_data = { "G250", 16.5, 162, 0.075, 0.075, 101, 135 };
static const PlateCarreeData ncep_grid251_data = { "G251", 26.35, 83.05, 0.1, 0.1, 210, 332 };

// 29 Polar Stereographic NCEP Grid definitions
// NOTE: Grids 28 and 224 are Polar Stereographic grids over the South Pole
// which is not currently supported
static const StereographicData ncep_grid005_data = { "G005", 60, 7.647, 133.443, 105, 190.5, 6367.47, 53, 57 };
static const StereographicData ncep_grid006_data = { "G006", 60, 7.647, 133.443, 105, 190.5, 6367.47, 53, 45 };
static const StereographicData ncep_grid027_data = { "G027", 60, -20.826, 125, 80, 381, 6367.47, 65, 65 };
static const StereographicData ncep_grid028_data = { "G028", -60, 20.826, -145, -100, 381, 6367.47, 65, 65 };
static const StereographicData ncep_grid055_data = { "G055", 60, -10.947, 154.289, 105, 254, 6367.47, 87, 71 };
static const StereographicData ncep_grid056_data = { "G056", 60, 7.647, 133.443, 105, 127, 6367.47, 87, 71 };
static const StereographicData ncep_grid087_data = { "G087", 60, 22.876, 120.491, 105, 68.153, 6367.47, 81, 62 };
static const StereographicData ncep_grid088_data = { "G088", 60, 10, 128, 105, 15, 6367.47, 580, 548 };
static const StereographicData ncep_grid100_data = { "G100", 60, 17.108, 129.296, 105, 91.452, 6367.47, 83, 83 };
static const StereographicData ncep_grid101_data = { "G101", 60, 10.528, 137.146, 105, 91.452, 6367.47, 113, 91 };
static const StereographicData ncep_grid103_data = { "G103", 60, 22.405, 121.352, 105, 91.452, 6367.47, 65, 56 };
static const StereographicData ncep_grid104_data = { "G104", 60, -0.268, 139.475, 105, 90.755, 6367.47, 147, 110 };
static const StereographicData ncep_grid105_data = { "G105", 60, 17.529, 129.296, 105, 90.755, 6367.47, 83, 83 };
static const StereographicData ncep_grid106_data = { "G106", 60, 17.533, 129.296, 105, 45.373, 6367.47, 165, 117 };
static const StereographicData ncep_grid107_data = { "G107", 60, 23.438, 120.168, 105, 45.373, 6367.47, 120, 92 };
static const StereographicData ncep_grid201_data = { "G201", 60, -20.826, 150, 105, 381, 6367.47, 65, 65 };
static const StereographicData ncep_grid202_data = { "G202", 60, 7.838, 141.028, 105, 190.5, 6367.47, 65, 43 };
static const StereographicData ncep_grid203_data = { "G203", 60, 19.132, 185.837, 150, 190.5, 6367.47, 45, 39 };
static const StereographicData ncep_grid205_data = { "G205", 60, 0.616, 84.904, 60, 190.5, 6367.47, 45, 39 };
static const StereographicData ncep_grid207_data = { "G207", 60, 42.085, 175.641, 150, 95.25, 6367.47, 49, 35 };
static const StereographicData ncep_grid213_data = { "G213", 60, 7.838, 141.028, 105, 95.25, 6367.47, 129, 85 };
static const StereographicData ncep_grid214_data = { "G214", 60, 42.085, 175.641, 150, 47.625, 6367.47, 97, 69 };
static const StereographicData ncep_grid216_data = { "G216", 60, 30, 173, 135, 45, 6367.47, 139, 107 };
static const StereographicData ncep_grid217_data = { "G217", 60, 30, 173, 135, 22.5, 6367.47, 277, 213 };
static const StereographicData ncep_grid223_data = { "G223", 60, -20.826, 150, 105, 190.5, 6367.47, 129, 129 };
static const StereographicData ncep_grid224_data = { "G224", 60, 20.826, -120, 105, 381, 6367.47, 65, 65 };
static const StereographicData ncep_grid240_data = { "G240", 60, 23.098, 119.036, 105, 4.7625, 6367.47, 1121, 881 };
static const StereographicData ncep_grid242_data = { "G242", 60, 30, 173, 135, 11.25, 6367.47, 553, 425 };
static const StereographicData ncep_grid249_data = { "G249", 60, 45.4, 171.6, 150, 9.868, 6367.47, 367, 343 };

// 20 Lambert Conformal NCEP Grid definitions
static const LambertData ncep_grid130_data = { "G130", 25, 25, 16.281, 126.138, 95.0, 13.545087, 6367.47, 451, 337 };
static const LambertData ncep_grid145_data = { "G145", 36, 46, 32.174, 90.159, 79.5, 12, 6367.47, 169, 145 };
static const LambertData ncep_grid146_data = { "G146", 36, 46, 32.353, 89.994, 79.5, 12, 6367.47, 166, 142 };
static const LambertData ncep_grid163_data = { "G163", 38, 38, 20.6, 118.3, 95, 5, 6367.47, 1008, 722 };
static const LambertData ncep_grid206_data = { "G206", 25, 25, 22.289, 117.991, 95, 81.271, 6367.47, 51, 41 };
static const LambertData ncep_grid209_data = { "G209", 45, 45, -4.85, 151.1, 111, 44, 6367.47, 275, 223 };
static const LambertData ncep_grid211_data = { "G211", 25, 25, 12.19, 133.459, 95, 81.271, 6367.47, 93, 65 };
static const LambertData ncep_grid212_data = { "G212", 25, 25, 12.19, 133.459, 95, 40.635, 6367.47, 185, 129 };
static const LambertData ncep_grid215_data = { "G215", 25, 25, 12.19, 133.459, 95, 20.318, 6367.47, 369, 257 };
static const LambertData ncep_grid218_data = { "G218", 25, 25, 12.19, 133.459, 95, 12.191, 6367.47, 614, 428 };
static const LambertData ncep_grid221_data = { "G221", 50, 50, 1, 145.5, 107, 32.463, 6367.47, 349, 277 };
static const LambertData ncep_grid222_data = { "G222", 45, 45, -4.85, 151.1, 111, 88, 6367.47, 138, 112 };
static const LambertData ncep_grid226_data = { "G226", 25, 25, 12.19, 133.459, 95, 10.159, 6367.47, 737, 513 };
static const LambertData ncep_grid227_data = { "G227", 25, 25, 12.19, 133.459, 95, 5.079, 6367.47, 1473, 1025 };
static const LambertData ncep_grid236_data = { "G236", 25, 25, 16.281, -233.862, 95, 40.635, 6367.47, 151, 113 };
static const LambertData ncep_grid237_data = { "G237", 50, 50, 16.201, -285.72, 107, 32.463, 6367.47, 54, 47 };
static const LambertData ncep_grid241_data = { "G241", 45, 45, -4.85, 151.1, 111, 22, 6367.47, 549, 445 };
static const LambertData ncep_grid245_data = { "G245", 35, 35, 22.98, 92.84, 80, 8, 6367.47, 336, 372 };
static const LambertData ncep_grid246_data = { "G246", 40, 40, 25.97, 127.973, 115, 8, 6367.47, 332, 371 };
static const LambertData ncep_grid247_data = { "G247", 35, 35, 22.98, 110.84, 98, 8, 6367.47, 336, 372 };
static const LambertData ncep_grid252_data = { "G252", 25, 25, 16.281, 126.138, 95, 20.317, 6367.47, 301, 225 };

// Mercator NCEP Grid definitions
static const MercatorData ncep_grid001_data = { "G001", -48.090, 0.000, 48.090, 0.000, 73, 23 };
// Do not define NCEP Grid number 8 since it's range of longitude is 363.104, and the latlon_to_xy
// routine won't be well defined.
static const MercatorData ncep_grid053_data = { "G053", -61.050, 0.000, 61.050, 0.000, 117, 51 };
static const MercatorData ncep_grid195_data = { "G195", 16.829, 68.196, 19.747, 63.972, 177, 129 };
static const MercatorData ncep_grid196_data = { "G196", 18.067, 161.626, 23.082, 153.969, 321, 225 };
static const MercatorData ncep_grid199_data = { "G199", 12.350, 216.314, 16.794, 179.960, 193, 193 };
static const MercatorData ncep_grid204_data = { "G204", -25.000, -110.000, 60.644, 109.129, 93, 68 };
static const MercatorData ncep_grid208_data = { "G208", 9.343, 167.315, 28.092, 145.878, 29, 27 };
static const MercatorData ncep_grid210_data = { "G210", 9.000, 77.000, 26.422, 58.625, 25, 25 };
static const MercatorData ncep_grid225_data = { "G225", -25.000, 250.000, 60.640, 109.129, 185, 135 };
static const MercatorData ncep_grid254_data = { "G254", -35.000, 250.000, 60.789, 109.129, 369, 300 };

// Arrays of Data Structures for each projection type
static const int n_pc_grids_data   = 32;
static const int n_st_grids_data   = 35;
static const int n_lc_grids_data   = 39;
static const int n_merc_grids_data = 10;

static const PlateCarreeData pc_grids_data[n_pc_grids_data] = {
   ncwd_data,
   ncwf_data,
   ncwf2_4km_data,
   ncwf2_8km_data,
   ow_conus_data,
   ow_gom_data,
   ow_pac_data,
   ow_nopac_data,
   ow_world_data,
   ncep_grid002_data,
   ncep_grid003_data,
   ncep_grid004_data,
   ncep_grid029_data,
   ncep_grid030_data,
   ncep_grid033_data,
   ncep_grid034_data,
   ncep_grid045_data,
   ncep_grid085_data,
   ncep_grid086_data,
   ncep_grid110_data,
   ncep_grid175_data,
   ncep_grid228_data,
   ncep_grid229_data,
   ncep_grid230_data,
   ncep_grid231_data,
   ncep_grid232_data,
   ncep_grid233_data,
   ncep_grid234_data,
   ncep_grid243_data,
   ncep_grid248_data,
   ncep_grid250_data,
   ncep_grid251_data
};

static const StereographicData st_grids_data[n_st_grids_data] = {
   stage4_data,
   stage4_2002_data,
   alaska_data,
   hires_alaska_data,
   spc_data,
   agrmet_north_data,
   ncep_grid005_data,
   ncep_grid006_data,
   ncep_grid027_data,
   ncep_grid028_data,
   ncep_grid055_data,
   ncep_grid056_data,
   ncep_grid087_data,
   ncep_grid088_data,
   ncep_grid100_data,
   ncep_grid101_data,
   ncep_grid103_data,
   ncep_grid104_data,
   ncep_grid105_data,
   ncep_grid106_data,
   ncep_grid107_data,
   ncep_grid201_data,
   ncep_grid202_data,
   ncep_grid203_data,
   ncep_grid205_data,
   ncep_grid207_data,
   ncep_grid213_data,
   ncep_grid214_data,
   ncep_grid216_data,
   ncep_grid217_data,
   ncep_grid223_data,
   ncep_grid224_data,
   ncep_grid240_data,
   ncep_grid242_data,
   ncep_grid249_data
};

static const LambertData lc_grids_data[n_lc_grids_data] = {
   ts_wrf_june_data,
   wrf8_data,
   wrf10_data,
   wrf22_data,
   ruc40_data,
   ruc20_data,
   ruc13_data,
   eta212_data,
   afwa_data,
   afwa2_data,
   bamex_wrf_data,
   ncwf2_2005_data,
   hirescip_data,
   goesruc5_data,
   ndfd5km_data,
   dtc_grid164_data,
   dtc_grid165_data,
   dtc_grid166_data,
   ncep_grid130_data,
   ncep_grid145_data,
   ncep_grid146_data,
   ncep_grid163_data,
   ncep_grid206_data,
   ncep_grid209_data,
   ncep_grid211_data,
   ncep_grid212_data,
   ncep_grid215_data,
   ncep_grid218_data,
   ncep_grid221_data,
   ncep_grid222_data,
   ncep_grid226_data,
   ncep_grid227_data,
   ncep_grid236_data,
   ncep_grid237_data,
   ncep_grid241_data,
   ncep_grid245_data,
   ncep_grid246_data,
   ncep_grid247_data,
   ncep_grid252_data
};

static const MercatorData merc_grids_data[n_merc_grids_data] = {
   ncep_grid001_data,
   ncep_grid053_data,
   ncep_grid195_data,
   ncep_grid196_data,
   ncep_grid199_data,
   ncep_grid204_data,
   ncep_grid208_data,
   ncep_grid210_data,
   ncep_grid225_data,
   ncep_grid254_data
};

extern int find_grid(const char *, Grid &);

////////////////////////////////////////////////////////////////////////


#endif   //  __DATA_GRIDS_GRID_DEFS_H__


////////////////////////////////////////////////////////////////////////
