// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

////////////////////////////////////////////////////////////////////////
// struct definition

struct NcObsOutputData {
   int  processed_hdr_cnt;
   int  deflate_level;
   vector<Observation> observations;
   SummaryObs *summary_obs;
   TimeSummaryInfo summary_info;
};

////////////////////////////////////////////////////////////////////////

extern string _secsToTimeString(const int secs);

extern void init_netcdf_output(NcFile *, NetcdfObsVars &obs_vars,
      NcObsOutputData &nc_out_data, string program_name);

extern bool write_observations(NcFile *, NetcdfObsVars &, NcObsOutputData &nc_out_data);

extern void write_summary_attributes(NcFile *, TimeSummaryInfo);

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_SUMMARY_H__  */


////////////////////////////////////////////////////////////////////////
