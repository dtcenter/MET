// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <time.h>

#include <netcdf>

#include "vx_log.h"
#include "is_bad_data.h"

#include "nc_point_obs.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetNcPointObs
   //


////////////////////////////////////////////////////////////////////////

MetNcPointObs::MetNcPointObs() {
   obs_data = std::make_unique<NcPointObsData>();
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MetNcPointObs::~MetNcPointObs() {
   close();
}

////////////////////////////////////////////////////////////////////////

void MetNcPointObs::init_from_scratch() {
   MetPointData::init_from_scratch();

   obs_nc_owner.reset();
   obs_nc = (NcFile *) nullptr;
}

////////////////////////////////////////////////////////////////////////

void MetNcPointObs::close() {
   MetPointData::clear();

   obs_nc_owner.reset();
   obs_nc = (NcFile *) nullptr;
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::open(const char * filename) {
   close();
   obs_nc_owner = open_ncfile(filename);
   obs_nc = obs_nc_owner.get();
   return IS_VALID_NC_P(obs_nc);
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::set_netcdf(NcFile *nc_file) {
   close();
   obs_nc = nc_file;
   return IS_VALID_NC_P(obs_nc);
}

//////////////////////////////////////////////////////////////////////////

