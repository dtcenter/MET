// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __OBS_ERROR_H__
#define  __OBS_ERROR_H__

////////////////////////////////////////////////////////////////////////

#include "config_constants.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

extern double    add_obs_error(double,
                               FieldType, const ObsErrorInfo &);
extern DataPlane add_obs_error(const DataPlane &,
                               FieldType, const ObsErrorInfo &);

////////////////////////////////////////////////////////////////////////

#endif   // __OBS_ERROR_H__

////////////////////////////////////////////////////////////////////////
