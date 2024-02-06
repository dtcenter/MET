// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NC_POINT_OBS_H__
#define  __NC_POINT_OBS_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "observation.h"
#include "nc_utils.h"
#include "nc_obs_util.h"
#include "nc_var_info.h"
#include "vx_summary.h"
#include "met_point_data.h"


////////////////////////////////////////////////////////////////////////


class MetNcPointObs : public MetPointData {

   protected:

      bool keep_nc;
      netCDF::NcFile *obs_nc;      //  allocated
      NetcdfObsVars obs_vars;

      void init_from_scratch();

   public:

      MetNcPointObs();
     ~MetNcPointObs();

      bool open(const char * filename);
      void close();
      bool set_netcdf(netCDF::NcFile *nc_file, bool _keep_nc=false);

      bool is_using_obs_arr();

      //  variables

      //  data

};  // MetNcPointObs

////////////////////////////////////////////////////////////////////////

inline bool MetNcPointObs::is_using_obs_arr() { return use_arr_vars; }

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_POINT_OBS_H__  */


////////////////////////////////////////////////////////////////////////

