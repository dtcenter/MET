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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include "vx_log.h"

#include "file_fxns.h"

////////////////////////////////////////////////////////////////////////
//
// Does replace(met_base_str, MET_BASE) on the output string, first
// checking the MET_BASE environment variable.
//
////////////////////////////////////////////////////////////////////////

ConcatString replace_path(const ConcatString path) {
   ConcatString s, met_base_env, met_base_val;

   // initialize
   s = path;
   
   // Use the MET_BASE environment variable, if set.
   // Otherwise, use the compile-time value.
   if(get_env(met_base_str, met_base_env)) met_base_val = met_base_env;
   else                                    met_base_val = MET_BASE;

   s.replace(met_base_str, met_base_val.c_str());

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString replace_path(const char * path) {
  return replace_path((string)path);
}

////////////////////////////////////////////////////////////////////////
//
// Wrapper functions to expand instances of MET_BASE and then call the
// requested open function.  This groups all open calls to a single
// file for ease in tracking Fortify compliance.
//
////////////////////////////////////////////////////////////////////////

int met_open(const char *path, int oflag) {
  return(open(replace_path(path).c_str(), oflag));
}

////////////////////////////////////////////////////////////////////////

void met_open(ifstream &in, const char *path) {
  in.open(replace_path(path).c_str());
   return;
}

////////////////////////////////////////////////////////////////////////

void met_open(ofstream &out, const char *path) {
  out.open(replace_path(path).c_str());
   return;
}

////////////////////////////////////////////////////////////////////////

FILE *met_fopen(const char *path, const char *mode) {
  return(fopen(replace_path(path).c_str(), mode));
}

////////////////////////////////////////////////////////////////////////

DIR *met_opendir(const char *path) {
  return(opendir(replace_path(path).c_str()));
}

////////////////////////////////////////////////////////////////////////


void met_closedir(DIR * & dp)

{

int status = ::closedir(dp);

dp = 0;

if ( status < 0 )  {

   mlog << Error
        << "\n\n  met_closedir(DIR *) -> trouble closing directory ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////



