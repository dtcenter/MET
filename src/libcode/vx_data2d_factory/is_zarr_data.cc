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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "is_zarr_data.h"
#include "vx_util.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

bool is_zarr_data(const string &path) {
   string zgroup_file = path + "/.zgroup";
   string zarray_file = path + "/.zarray";

   // check for a directory containing .zgroup and/or .zarray
   bool has_zgroup = file_exists(zgroup_file.c_str());
   bool has_zarray = file_exists(zarray_file.c_str());

   return has_zgroup || has_zarray;
}

////////////////////////////////////////////////////////////////////////

