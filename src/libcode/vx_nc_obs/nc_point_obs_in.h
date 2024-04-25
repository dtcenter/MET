// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __NC_POINT_OBS_IN_H__
#define  __NC_POINT_OBS_IN_H__

////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "observation.h"
#include "nc_utils.h"
#include "nc_obs_util.h"
#include "nc_var_info.h"
#include "nc_point_obs.h"


////////////////////////////////////////////////////////////////////////


class MetNcPointObsIn : public MetNcPointObs {

   public:

      MetNcPointObsIn();
     ~MetNcPointObsIn();

      bool check_nc(const char *nc_name, const char *caller=empty_name);
      bool read_dim_headers();
      bool read_obs_data();
      bool read_obs_data(int buf_size, int start, float *obs_arr_block,
                         int *obs_qty_idx_block, char *obs_qty_str_block);
      bool read_obs_data_numbers();
      bool read_obs_data_table_lookups();

      //  variables

      //  data

};  // MetNcPointObs

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_POINT_OBS_IN_H__  */


////////////////////////////////////////////////////////////////////////

