

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

#include "is_grib_file.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const char grib_magic [] = "GRIB";

static const int grib_magic_len = strlen(grib_magic);

static const int buf_size = 8;


////////////////////////////////////////////////////////////////////////


static bool check_grib(const char * filename, int & version);


////////////////////////////////////////////////////////////////////////


bool is_grib1_file(const char * filename)

{

int version;
bool status = check_grib(filename, version);

if ( !status )  return ( false );

   //
   //  done
   //

return ( version == 1 );

}


////////////////////////////////////////////////////////////////////////


bool is_grib2_file(const char * filename)

{

int version;
bool status = check_grib(filename, version);

if ( !status )  return ( false );

   //
   //  done
   //

return ( version == 2 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  assumes that the "GRIB" magic cookie is at offset zero
   //

bool check_grib(const char * filename, int & version)

{

int fd = -1;
char buf[buf_size];

   //
   //  open file
   //

if ( (fd = open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\n  check_grib() -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

   // return ( false );

}

   //
   //  read first few bytes
   //

if ( read(fd, buf, buf_size) != buf_size )  {

   mlog << Error << "\n  check_grib() -> unable to read header from input file \"" << filename << "\"\n\n";

   exit ( 1 );

   // return ( false );

}

   //
   //  close file
   //

close(fd);  fd = -1;

   //
   //  check for grib magic cookie
   //

if ( strncmp(buf, grib_magic, grib_magic_len) != 0 )  return ( false );

   //
   //  grab version number
   //

version = (int) (buf[7]);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////



