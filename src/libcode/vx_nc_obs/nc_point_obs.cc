// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <time.h>

#include <netcdf>
using namespace netCDF;

#include "vx_log.h"
#include "is_bad_data.h"

#include "nc_point_obs.h"

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetNcPointObs
   //


////////////////////////////////////////////////////////////////////////

MetNcPointObs::MetNcPointObs() {
   obs_data = new NcPointObsData();
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MetNcPointObs::~MetNcPointObs() {
   close();
}

////////////////////////////////////////////////////////////////////////

void MetNcPointObs::init_from_scratch() {
   MetPointData::init_from_scratch();

   keep_nc = false;
   obs_nc = (NcFile *) nullptr;
}

////////////////////////////////////////////////////////////////////////

void MetNcPointObs::close() {
   MetPointData::clear();

   if ( !keep_nc && obs_nc ) {
      delete obs_nc;
      obs_nc = (NcFile *) nullptr;
   }
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::open(const char * filename) {
   return set_netcdf(open_ncfile(filename));
}

////////////////////////////////////////////////////////////////////////

bool MetNcPointObs::set_netcdf(NcFile *nc_file, bool _keep_nc) {
   close();
   keep_nc = _keep_nc;
   obs_nc = nc_file;
   return IS_VALID_NC_P(obs_nc);
}

//////////////////////////////////////////////////////////////////////////

