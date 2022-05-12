// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   point_stat.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/09/14  Halley Gotway   New
//   001    06/02/16  Halley Gotway   Add box masking type.
//   002    11/15/16  Halley Gotway   Add solar masking types.
//   003    06/03/21  Seth Linden     Changed default mask type to MaskType_None.
//   004    08/30/21  Halley Gotway   MET #1891 fix input and mask fields.
//   005    05/05/22  Halley Gotway   MET #2152 Add -type poly_xy.
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GEN_VX_MASK_H__
#define  __GEN_VX_MASK_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vx_util.h"
#include "vx_cal.h"
#include "mask_poly.h"
#include "vx_grid.h"
#include "data2d_nc_met.h"
#include "data_plane.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "shp_file.h"
#include "shp_poly_record.h"

////////////////////////////////////////////////////////////////////////

static const char *program_name = "gen_vx_mask";

////////////////////////////////////////////////////////////////////////

enum MaskType {

   MaskType_Poly,      // Polyline masking in lat/lon space
   MaskType_Poly_XY,   // Polyline masking in grid x/y space

   MaskType_Box,       // Box masking type
   MaskType_Circle,    // Circle masking region

   MaskType_Track,     // Track masking region
   MaskType_Grid,      // Grid masking type
   MaskType_Data,      // Data masking type

   MaskType_Solar_Alt, // Solar altitude masking type
   MaskType_Solar_Azi, // Solar azimuth masking type

   MaskType_Lat,       // Latitude masking type
   MaskType_Lon,       // Longitude masking type

   MaskType_Shape,     // Shapefile

   MaskType_None

};

extern bool is_solar_masktype(MaskType);
extern MaskType string_to_masktype(const char *);
extern const char * masktype_to_string(MaskType);

////////////////////////////////////////////////////////////////////////
//
// Default values
//
////////////////////////////////////////////////////////////////////////

static const MaskType default_mask_type = MaskType_None;
static const double default_mask_val = 1.0;

////////////////////////////////////////////////////////////////////////
//
// Variables for command line arguments
//
////////////////////////////////////////////////////////////////////////

// Input data file, mask file, and output NetCDF file
static ConcatString input_gridname, mask_filename, out_filename;
static MaskType mask_type = default_mask_type;
static bool type_is_set = false;

// Optional arguments
static ConcatString input_field_str, mask_field_str;
static SetLogic set_logic = SetLogic_None;
static bool complement = false;
static SingleThresh thresh;
static int height = bad_data_double;
static int width = bad_data_double;
static double mask_val = default_mask_val;
static ConcatString mask_name;
static unixtime solar_ut = (unixtime) 0;

static ShpPolyRecord shape;

static int shape_number = 0;

// Masking polyline
static MaskPoly poly_mask;

// Grid on which the data field resides
static Grid grid, grid_mask;

// Configuration object for reading config strings
static MetConfig global_config;
static int compress_level = -1;

////////////////////////////////////////////////////////////////////////

static void      process_command_line(int, char **);
static void      process_input_grid(DataPlane &dp);
static void      process_mask_file(DataPlane &dp);
static void      get_data_plane(const ConcatString &file_name,
                    const ConcatString &config_str, bool,
                    DataPlane &dp, Grid &dp_grid);
static bool      get_gen_vx_mask_config_str(MetNcMetDataFile *,
                    ConcatString &);
static void      get_shapefile_outline(ShpPolyRecord &shape);
static void      apply_poly_mask(DataPlane &dp);
static void      apply_poly_xy_mask(DataPlane &dp);
static void      apply_shape_mask(DataPlane &dp);
static void      apply_box_mask(DataPlane &dp);
static void      apply_circle_mask(DataPlane &dp);
static void      apply_track_mask(DataPlane &dp);
static void      apply_grid_mask(DataPlane &dp);
static void      apply_data_mask(DataPlane &dp);
static void      apply_solar_mask(DataPlane &dp);
static void      apply_lat_lon_mask(DataPlane &dp);
static DataPlane combine(const DataPlane &dp_data,
                    const DataPlane &dp_mask, SetLogic);
static void      write_netcdf(const DataPlane &dp);
static void      usage();
static void      set_type(const StringArray &);
static void      set_input_field(const StringArray &);
static void      set_mask_field(const StringArray &);
static void      set_complement(const StringArray &);
static void      set_union(const StringArray &);
static void      set_intersection(const StringArray &);
static void      set_symdiff(const StringArray &);
static void      set_thresh(const StringArray &);
static void      set_height(const StringArray &);
static void      set_width(const StringArray &);
static void      set_value(const StringArray &);
static void      set_name(const StringArray &);
static void      set_compress(const StringArray &);
static void      set_shapeno(const StringArray &);

////////////////////////////////////////////////////////////////////////

#endif   //  __GEN_VX_MASK_H__

////////////////////////////////////////////////////////////////////////
