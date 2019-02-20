// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include "file_fxns.h"

////////////////////////////////////////////////////////////////////////
//
// Does replace(met_base_str, MET_BASE) on the output string, first
// checking the MET_BASE environment variable.
//
////////////////////////////////////////////////////////////////////////

ConcatString replace_path(const char *path) {
   ConcatString s, met_base_val;
   char *ptr;

   // Initialize
   s = path;

   // Use the MET_BASE environment variable, if set.
   // Otherwise, use the compile-time value.
   if((ptr = get_env(met_base_str)) != NULL) met_base_val = ptr;
   else                                      met_base_val = MET_BASE;

   s.replace(met_base_str, met_base_val);

   return(s);
}

////////////////////////////////////////////////////////////////////////
//
// Wrapper functions to expand instances of MET_BASE and then call the
// requested open function.  This groups all open calls to a single
// file for ease in tracking Fortify compliance.
//
////////////////////////////////////////////////////////////////////////

int met_open(const char *path, int oflag) {
   return(open(replace_path(path), oflag));
}

////////////////////////////////////////////////////////////////////////

void met_open(ifstream &in, const char *path) {
   in.open(replace_path(path));
   return;
}

////////////////////////////////////////////////////////////////////////

FILE *met_fopen(const char *path, const char *mode) {
   return(fopen(replace_path(path), mode));
}

////////////////////////////////////////////////////////////////////////

DIR *met_opendir(const char *path) {
   return(opendir(replace_path(path)));
}

////////////////////////////////////////////////////////////////////////
