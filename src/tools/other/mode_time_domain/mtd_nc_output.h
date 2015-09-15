

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_NC_OUTPUT_H__
#define  __MTD_NC_OUTPUT_H__


////////////////////////////////////////////////////////////////////////


#include "mtd_nc_defs.h"
#include "nc_grid.h"
#include "nc_utils.h"

#include "mtd_file_float.h"
#include "mtd_file_int.h"

#include "mtd_config_info.h"
#include "mm_engine.h"


////////////////////////////////////////////////////////////////////////


extern void do_mtd_nc_output(MtdNcOutInfo &, 
                             const MtdFloatFile & fcst_raw, const MtdFloatFile & obs_raw, 
                             const MtdIntFile   & fcst_obj, const MtdFloatFile & obs_obj, 
                             const MM_Engine &);



////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_NC_OUTPUT_H__  */


////////////////////////////////////////////////////////////////////////


