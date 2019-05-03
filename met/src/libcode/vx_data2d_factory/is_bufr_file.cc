

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "is_bufr_file.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const char bufr_magic [] = "BUFR";

static const int bufr_magic_len = strlen(bufr_magic);

static const int buf_size = 8;


////////////////////////////////////////////////////////////////////////


   //
   //  assumes that the "BUFR" magic cookie is at offset zero
   //

bool is_bufr_file(const char * filename)

{

int fd = -1;
char buf[buf_size];

   //
   //  open file
   //

if ( (fd = met_open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\nis_bufr_file() -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

   // return ( false );

}

   //
   //  read first few bytes
   //

if ( read(fd, buf, buf_size) != buf_size )  {

   mlog << Error << "\nis_bufr_file() -> unable to read header from input file \"" << filename << "\"\n\n";

   exit ( 1 );

   // return ( false );

}

   //
   //  close file
   //

close(fd);  fd = -1;

   //
   //  check for bufr magic cookie
   //

if ( strncmp(buf, bufr_magic, bufr_magic_len) != 0 )  return ( false );

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////



