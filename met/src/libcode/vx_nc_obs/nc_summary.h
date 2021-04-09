// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include "nc_obs_util.h"
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

// Not moved to nc_obs_util to reduce the dependency (library)

//extern bool init_netcdf_output(NcFile *, NetcdfObsVars *obs_vars,
//                               NcObsOutputData *nc_out_data, string program_name);
extern bool get_hdr_obs_count(NcFile *nc_file, NcObsOutputData *nc_out_data,
                              int *obs_cnt, int *hdr_cnt);

extern void set_nc_out_data(NcObsOutputData *out_data, vector<Observation> observations,
                            SummaryObs *summary_obs, TimeSummaryInfo summary_info);

//extern bool write_observations(NetcdfObsVars *, NcObsOutputData *nc_out_data);

extern void write_summary_attributes(NcFile *, TimeSummaryInfo);

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_SUMMARY_H__  */


////////////////////////////////////////////////////////////////////////
