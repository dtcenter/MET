

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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "is_grib_file.h"
#include "vx_util.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const char grib_magic [] = "GRIB";

static const int grib_magic_len = strlen(grib_magic);

static const int buf_size = 256;


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
   //  searches the buffer for the "GRIB" magic cookie
   //

bool check_grib(const char * filename, int & version)

{

int fd = -1;
char buf[buf_size];
bool found = false;
int i;
long long size, read_size;

   //
   //  open file
   //

if ( (fd = met_open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\ncheck_grib() -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  determine the number of bytes to read
   //
size      = file_size(filename);
read_size = ( buf_size < size ? buf_size : size );

   //
   //  read into the buffer
   //

if ( read(fd, buf, read_size) != read_size )  {

   mlog << Error << "\ncheck_grib() -> unable to read header from input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  close file
   //

close(fd);

   //
   //  search buffer for grib magic cookie
   //

for ( i=0; i<(read_size - grib_magic_len); ++i)  {
  
   if ( strncmp(&buf[i], grib_magic, grib_magic_len) == 0 )  {
      found = true;
      break;
   }
}

if( !found ) return ( false );

   //
   //  grab version number
   //

version = (int) (buf[i+7]);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////



