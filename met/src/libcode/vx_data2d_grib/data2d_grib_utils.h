

////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_2D_GRIB_UTILS_H__
#define  __DATA_2D_GRIB_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include "var_info_grib.h"
#include "grib_classes.h"
#include "vx_grid.h"
#include "data_plane.h"


////////////////////////////////////////////////////////////////////////


extern bool is_exact_match(const VarInfoGrib &, const GribRecord &);

extern bool is_range_match(const VarInfoGrib &, const GribRecord &);

extern bool get_data_plane(const GribRecord &, DataPlane &);

extern void read_pds(const GribRecord &, int & bms_flag, unixtime & init_ut, unixtime & valid_ut, int & accum);

extern void read_pds_prob(const GribRecord &,
                          int &p_code, double &p_thresh_lo, double &p_thresh_hi);

extern void read_pds_level(const GribRecord &, int &lower, int &upper);


////////////////////////////////////////////////////////////////////////


#endif   /*  __DATA_2D_GRIB_UTILS_H__  */


////////////////////////////////////////////////////////////////////////



