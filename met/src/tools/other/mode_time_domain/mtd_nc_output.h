// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_NC_OUTPUT_H__
#define  __MTD_NC_OUTPUT_H__


////////////////////////////////////////////////////////////////////////


#include "mtd_nc_defs.h"
#include "nc_grid.h"
#include "nc_utils_local.h"

#include "mtd_file_float.h"
#include "mtd_file_int.h"

#include "mtd_config_info.h"
#include "mm_engine.h"


////////////////////////////////////////////////////////////////////////


extern void do_mtd_nc_output(const MtdNcOutInfo &, const MM_Engine &, 
                             const MtdFloatFile & fcst_raw, const MtdFloatFile & obs_raw, 
                             const MtdIntFile   & fcst_obj, const MtdIntFile   & obs_obj, 
                             const MtdConfigInfo & config, 
                             const char * output_filename);

   //
   //  for single fields
   //


extern void do_mtd_nc_output(const MtdNcOutInfo &,
                             const MtdFloatFile & raw,
                             const MtdIntFile   & obj,
                             const MtdConfigInfo & config, 
                             const char * output_filename);



////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_NC_OUTPUT_H__  */


////////////////////////////////////////////////////////////////////////


