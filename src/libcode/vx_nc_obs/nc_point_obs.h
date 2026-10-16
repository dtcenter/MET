// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NC_POINT_OBS_H__
#define  __NC_POINT_OBS_H__


////////////////////////////////////////////////////////////////////////


#include <memory>
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

      //
      //  obs_nc is the active handle, whether this object opened the file or
      //  was handed one.  obs_nc_owner is non-null only in the first case, so
      //  ownership is carried by the type rather than by the old keep_nc flag.
      //

      std::unique_ptr<netCDF::NcFile> obs_nc_owner;
      netCDF::NcFile *obs_nc;      //  borrowed when obs_nc_owner is null
      NetcdfObsVars obs_vars;

      void init_from_scratch();

   public:

      MetNcPointObs();
     ~MetNcPointObs();

      bool open(const char * filename);
      void close();
      //  set_netcdf() always borrows; open() is the owning entry point
      bool set_netcdf(netCDF::NcFile *nc_file);

      bool is_using_obs_arr();

      //  variables

      //  data

};  // MetNcPointObs

////////////////////////////////////////////////////////////////////////

inline bool MetNcPointObs::is_using_obs_arr() { return use_arr_vars; }

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_POINT_OBS_H__  */


////////////////////////////////////////////////////////////////////////

