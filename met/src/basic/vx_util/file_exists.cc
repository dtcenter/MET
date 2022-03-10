

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
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "file_exists.h"
#include "empty_string.h"


////////////////////////////////////////////////////////////////////////


bool file_exists(const char * path)

{

int status;

status = access(path, F_OK);

if ( status == 0 )  return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool directory_exists(const char * path)

{

if ( empty(path) )  {

   cerr << "\n\n  directory_exists(const char *) -> empty path!\n\n";

   exit ( 1 );

}

if ( ! file_exists(path) )  return ( false );

struct stat s;

if ( stat(path, &s) < 0 )  {

   cerr << "\n\n  directory_exists(const char *) -> unable to stat path \""
        << path << "\" ... " << strerror(errno) << "\n\n";

   exit ( 1 );

}

if ( S_ISDIR(s.st_mode) )  return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


