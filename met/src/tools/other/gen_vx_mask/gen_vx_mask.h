// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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
#include "data_plane.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"

////////////////////////////////////////////////////////////////////////

static const char *program_name = "gen_vx_mask";

////////////////////////////////////////////////////////////////////////

enum MaskType {
   MaskType_Poly,   // Polyline masking region
   MaskType_Circle, // Circle masking region
   MaskType_Track,  // Track masking region
   MaskType_Grid,   // Grid masking type
   MaskType_Data,   // Data masking type
   MaskType_None
};

extern MaskType string_to_masktype(const char *);
extern const char * masktype_to_string(MaskType);

////////////////////////////////////////////////////////////////////////
//
// Default values
//
////////////////////////////////////////////////////////////////////////

static const MaskType default_mask_type = MaskType_Poly;
static const double default_mask_val = 1.0;

////////////////////////////////////////////////////////////////////////
//
// // Variables for command line arguments
//
////////////////////////////////////////////////////////////////////////

// Input data file, mask file, and output NetCDF file
static ConcatString input_filename, mask_filename, out_filename;

// Optional arguments
static MaskType mask_type = default_mask_type;
static ConcatString input_config_str, mask_config_str;
static SetLogic set_logic = SetLogic_None;
static bool complement = false;
static SingleThresh thresh;
static double mask_val = default_mask_val;
static ConcatString mask_name;

// Masking polyline
static MaskPoly poly_mask;

// Grid on which the data field resides
static Grid grid, grid_mask;

// Configuration object for reading config strings
static MetConfig config;

////////////////////////////////////////////////////////////////////////

static void      process_command_line(int, char **);
static void      process_input_file(DataPlane &dp);
static void      process_mask_file(DataPlane &dp);
static void      get_data_plane(Met2dDataFile *mtddf_ptr,
                                const char *config_str, DataPlane &dp);
static bool      get_gen_vx_mask_data(Met2dDataFile *mtddf_ptr,
                                      DataPlane &dp);
static void      apply_poly_mask(DataPlane &dp);
static void      apply_circle_mask(DataPlane &dp);
static void      apply_track_mask(DataPlane &dp);
static void      apply_grid_mask(DataPlane &dp);
static void      apply_data_mask(DataPlane &dp);
static DataPlane combine(const DataPlane &dp_data, const DataPlane &dp_mask, SetLogic);
static void      write_netcdf(const DataPlane &dp);
static void      usage();
static void      set_type(const StringArray &);
static void      set_input_config(const StringArray &);
static void      set_mask_config(const StringArray &);
static void      set_complement(const StringArray &);
static void      set_union(const StringArray &);
static void      set_intersection(const StringArray &);
static void      set_symdiff(const StringArray &);
static void      set_thresh(const StringArray &);
static void      set_value(const StringArray &);
static void      set_name(const StringArray &);
static void      set_logfile(const StringArray &);
static void      set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

#endif   //  __GEN_VX_MASK_H__

////////////////////////////////////////////////////////////////////////
