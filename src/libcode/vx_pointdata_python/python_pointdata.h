// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __PYTHON_POINTDATA_H__
#define  __PYTHON_POINTDATA_H__


////////////////////////////////////////////////////////////////////////


#include "mask_filters.h"
#include "met_point_data.h"


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////

static const char python_key_nhdr           [] = "nhdr";
//static const char python_key_npbhdr         [] = "npbhdr";
static const char python_use_var_id         [] = "use_var_id";
static const char numpy_array_hdr_typ       [] = "hdr_typ"; // message type IDs
static const char numpy_array_hdr_sid       [] = "hdr_sid"; // station IDs
static const char numpy_array_hdr_vld       [] = "hdr_vld"; // valid time IDs
static const char numpy_array_hdr_lat       [] = "hdr_lat";
static const char numpy_array_hdr_lon       [] = "hdr_lon";
static const char numpy_array_hdr_elv       [] = "hdr_elv";
static const char numpy_array_hdr_typ_table [] = "hdr_typ_table";   // message type list
static const char numpy_array_hdr_sid_table [] = "hdr_sid_table";   // station ID list
static const char numpy_array_hdr_vld_table [] = "hdr_vld_table";   // valid time list
static const char numpy_array_prpt_typ_table[] = "prpt_typ_table";
static const char numpy_array_irpt_typ_table[] = "irpt_typ_table";
static const char numpy_array_inst_typ_table[] = "inst_typ_table";

static const char python_key_nobs           [] = "nobs";
static const char numpy_array_obs_qty       [] = "obs_qty"; // quality_id
static const char numpy_array_obs_hid       [] = "obs_hid"; // header id
static const char numpy_array_obs_vid       [] = "obs_vid"; // variable id or grib code
static const char numpy_array_obs_lvl       [] = "obs_lvl";
static const char numpy_array_obs_hgt       [] = "obs_hgt";
static const char numpy_array_obs_val       [] = "obs_val";
static const char numpy_array_obs_qty_table [] = "obs_qty_table";
static const char numpy_array_obs_var_table [] = "obs_var_table";   // variable names or grib codes as string

static const int point_data_debug_level = 10;


////////////////////////////////////////////////////////////////////////


extern bool python_point_data(const char * script_name, int script_argc,
                              char ** script_argv, MetPointDataPython &met_pd_out,
                              MaskFilters *filters);

//extern bool python_point_data(const char *python_command, const bool use_xarray,
//                              MetPointData & po_out);
extern void print_met_data(MetPointObsData *obs_data, MetPointHeader *header_data,
                           const char *caller, int debug_level=point_data_debug_level);


////////////////////////////////////////////////////////////////////////

#include "python_pointdata.hpp"

////////////////////////////////////////////////////////////////////////

#endif   /*  __PYTHON_POINTDATA_H__  */


////////////////////////////////////////////////////////////////////////

