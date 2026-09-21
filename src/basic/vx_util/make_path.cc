// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <string>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstdio>
#include <limits.h>
#include <cmath>

#include "make_path.h"
#include "vx_log.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


static int path_exists(const char *);


////////////////////////////////////////////////////////////////////////


int make_path(const char * path, int mode)

{


if ( path_exists(path) )  return 1;

int status;
const char *method_name = "make_path() ";


   //
   //  make subpath: everything up to, but not including, the last '/'
   //

if ( path )  {
   std::string subpath(path);
   auto slash = subpath.find_last_of('/');
   subpath = (slash == std::string::npos) ? std::string() : subpath.substr(0, slash);

   mlog << Debug(1) << "\n\n  " << method_name << "subpath = \"" << subpath << "\"\n\n";

   if ( subpath.empty() )  return 0;

   if ( !(path_exists(subpath.c_str())) )  {
      make_path(subpath.c_str(), mode);
   }

}

status = mkdir(path, mode);

if ( status < 0 )   return 0;

return 1;

}


////////////////////////////////////////////////////////////////////////


int path_exists(const char * path)

{

int status;
struct stat sbuf;


status = stat(path, &sbuf);

if ( status < 0 )  {

   if ( errno == ENOTDIR )  {

      mlog << Error << "\npath component not directory!\n\n";

      exit ( 1 );

   }

   if ( errno == ENOENT )  return 0;

      //
      //  must be some obscure error
      //

   mlog << Error << "\npath_exists() -> error ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

status = S_ISDIR(sbuf.st_mode);

if ( status )  return 1;

return 0;

}


////////////////////////////////////////////////////////////////////////


