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
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstdio>
#include <limits.h>
#include <cmath>

#include "make_path.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static int path_exists(const char *);


////////////////////////////////////////////////////////////////////////


int make_path(const char * path, int mode)

{


if ( path_exists(path) )  return 1;

int j;
int status;
const char *method_name = "make_path() ";


   //
   //  make subpath
   //

char *subpath = m_strcpy2(path, method_name);

if (subpath) {
   j = m_strlen(subpath) - 1;

   while ( (j >= 0) && (subpath[j] != '/') )  subpath[j--] = (char) 0;

   if ( j >= 0 ) subpath[j] = (char) 0;

   mlog << Debug(1) << "\n\n  " << method_name << "subpath = \"" << subpath << "\"\n\n";

   if ( m_strlen(subpath) == 0 ) {
      if (subpath) { delete [] subpath; subpath = (char *) 0; }
      return 0;
   }

   if ( !(path_exists(subpath)) )  {
      make_path(subpath, mode);
   }

}

status = mkdir(path, mode);

if (subpath) { delete [] subpath; subpath = (char *) 0; }

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


