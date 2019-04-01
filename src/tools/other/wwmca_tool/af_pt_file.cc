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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <cstdio>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"
#include "vx_cal.h"

#include "af_file.h"
#include "af_pt_file.h"


////////////////////////////////////////////////////////////////////////


// pixel age time records are 4 bytes each
static const int pixel_time_record_size = 4;

// awfa pixel age time begins at December 31, 1967 00:00:00
static const unixtime af_start = mdyhms_to_unix(12, 31, 1967, 0, 0, 0);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AFPixelTimeFile
   //


////////////////////////////////////////////////////////////////////////


AFPixelTimeFile::AFPixelTimeFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AFPixelTimeFile::~AFPixelTimeFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AFPixelTimeFile::AFPixelTimeFile(const AFPixelTimeFile & a)

{

init_from_scratch();

assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


AFPixelTimeFile & AFPixelTimeFile::operator=(const AFPixelTimeFile & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AFPixelTimeFile::init_from_scratch()

{

Buf = (unsigned char *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AFPixelTimeFile::clear()

{

if ( Buf )  { delete [] Buf;  Buf = (unsigned char *) 0; }

AFDataFile::clear();

SwapEndian = true;

return;

}


////////////////////////////////////////////////////////////////////////


void AFPixelTimeFile::assign(const AFPixelTimeFile & a)

{

clear();

if ( !(a.Buf) )  return;

Buf = new unsigned char [af_nx*af_ny*pixel_time_record_size];

memset(Buf, 0, af_nx*af_ny*pixel_time_record_size);

AFDataFile::assign(a);

SwapEndian = a.SwapEndian;

return;

}


////////////////////////////////////////////////////////////////////////


bool AFPixelTimeFile::read(const char * filename, const char input_hemi)

{

clear();

   //  Call parent to parse metadata

AFDataFile::read(filename, input_hemi);

int fd = -1;
int bytes;

if ( (fd = met_open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\nAFPixelTimeFile::read(const char *) -> "
        << "can't open file \"" << filename << "\"\n\n";

   return ( false );

}

bytes = af_nx*af_ny*pixel_time_record_size;

Buf = new unsigned char [bytes];

if ( ::read(fd, Buf, bytes) != bytes )  {

   mlog << Error << "\nAFPixelTimeFile::read(const char *) -> "
        << "read error on file \"" << filename << "\"\n\n";

   ::close(fd);

   return ( false );

}

Filename = get_short_name(filename);


   //
   //  done
   //

::close(fd);   fd = -1;

return ( true );

}


////////////////////////////////////////////////////////////////////////


int AFPixelTimeFile::pixel_age_sec(int x, int y) const

{

int k, n;
unixtime t = 0;

n = two_to_one(x, y);   //  this function does range checking on x and y for us

int *ibuf = (int *)Buf;

int minutes = ibuf[n];

if ( SwapEndian )  shuffle_4(&minutes);

if (minutes == 0)  k = 0;
else {

   t = 60LL * minutes;      // convert minutes to long long in seconds

   t += af_start;           // convert af time to unixtime

   k = (int) (Valid - t);   // subtract from the filetime (Valid) to get pixel age in seconds

}

return ( k );

}


////////////////////////////////////////////////////////////////////////

