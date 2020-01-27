

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>

#include "vx_log.h"

#include "file_size.h"


////////////////////////////////////////////////////////////////////////


long long file_size(const char * path)

{

int status;
struct stat sbuf;

status = stat(path, &sbuf);

if ( status < 0 )  {

   mlog << Error << "\nfile_size() -> can't open file: " << path << "\n\n";

   exit ( 1 );

}

   //
   //  return the file size in bytes
   //

return( (long long) sbuf.st_size );

}


////////////////////////////////////////////////////////////////////////


