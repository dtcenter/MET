// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SUMMARY_NC_H__
#define  __SUMMARY_NC_H__

#include <netcdf>
using namespace netCDF;
//#include "vx_util.h"
#include "nc_utils.h"
#include "vx_summary.h"

extern string _secsToTimeString(const int secs);
extern void write_summary_attributes(NcFile *, TimeSummaryInfo);

////////////////////////////////////////////////////////////////////////


#endif   /*  __SUMMARY_NC_H__  */


////////////////////////////////////////////////////////////////////////
