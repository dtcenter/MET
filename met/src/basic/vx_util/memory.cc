// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <cmath>

#include "memory.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////

void oom() {

   mlog << Error << "\nOut of memory!  Exiting!\n\n";
   exit(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void oom_grib2() {

   mlog << Error << "\nOut of memory reading GRIB2 data!  Exiting!\n"
	<< "Check that MET and the GRIB2C library were compiled "
        << "consistently, either with or without the -D__64BIT__ "
        << "flag.\n\n";
   exit(1);

   return;
}

////////////////////////////////////////////////////////////////////////
