// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NC_SUMMARY_H__
#define  __NC_SUMMARY_H__

#include <netcdf>
using namespace netCDF;
#include "nc_utils.h"
#include "vx_summary.h"

extern string _secsToTimeString(const int secs);

extern void init_netcdf_output(NcFile *, NetcdfObsVars &obs_vars,
      SummaryObs *summary_obs, vector<Observation> observations,
      string program_name, int raw_hdr_cnt=0, int deflate_level = 0);

extern bool write_observations(NcFile *, NetcdfObsVars &, SummaryObs *,
      vector<Observation>, const bool include_header,
      const int deflate_level = 0);

extern void write_summary_attributes(NcFile *, TimeSummaryInfo);

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_SUMMARY_H__  */


////////////////////////////////////////////////////////////////////////
