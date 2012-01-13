

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2010
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


////////////////////////////////////////////////////////////////////////


static const char netcdf_magic  [] = "CDF";

static const int netcdf_magic_len  = strlen(netcdf_magic);


////////////////////////////////////////////////////////////////////////


bool is_netcdf_file(const char * filename)

{

if ( !filename || !(*filename) )  return ( false );

int fd = -1;
int n_read;
char buf[netcdf_magic_len];


if ( (fd = open(filename, O_RDONLY)) < 0 )  return ( false );

n_read = read(fd, buf, netcdf_magic_len);

close(fd);  fd = -1;

if ( n_read != netcdf_magic_len )  return ( false );

if ( strncmp(buf, netcdf_magic, netcdf_magic_len) == 0 )  return ( true );


   //
   //  done
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////


