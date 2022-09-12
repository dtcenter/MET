// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
   std::vector<Observation> observations;
   SummaryObs *summary_obs;
   TimeSummaryInfo summary_info;
};

////////////////////////////////////////////////////////////////////////

extern std::string _secsToTimeString(const int secs);

// Not moved to nc_obs_util to reduce the dependency (library)

extern void write_summary_attributes(NcFile *, TimeSummaryInfo);

////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_SUMMARY_H__  */


////////////////////////////////////////////////////////////////////////
