// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"

#include "diag_file.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class DiagFile
//
////////////////////////////////////////////////////////////////////////

DiagFile::DiagFile() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

DiagFile::~DiagFile() {
   close();
}

////////////////////////////////////////////////////////////////////////

DiagFile::DiagFile(const DiagFile &) {
   mlog << Error << "\nDiagFile::DiagFile(const DiagFile &) -> "
        << "should never be called!\n\n";
   exit(1);
}

////////////////////////////////////////////////////////////////////////

DiagFile & DiagFile::operator=(const DiagFile &) {
   mlog << Error
        << "\nDiagFile::operator=(const DiagFile &) -> "
        << "should never be called!\n\n";
   exit(1);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void DiagFile::init_from_scratch() {

   // Initialize values
   StormID.clear();
   Basin.clear();
   Cyclone.clear();
   Technique.clear();
   InitTime = (unixtime) 0;

   close();

   return;
}

////////////////////////////////////////////////////////////////////////

bool DiagFile::open_tcdiag(const char *path, const char *model_name) {

   close();

   // JHG work here

   return ( false );
}

////////////////////////////////////////////////////////////////////////

bool DiagFile::open_lsdiag(const char *path, const char *model_name) {

   close();

   // JHG work here

   return ( false );
}

////////////////////////////////////////////////////////////////////////
