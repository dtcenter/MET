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
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <cstdio>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"
#include "vx_cal.h"

#include "af_file.h"
#include "af_cp_file.h"


////////////////////////////////////////////////////////////////////////


// cloud percent records are 1 byte each
static const int cloud_pct_record_size    =   1;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AFCloudPctFile
   //


////////////////////////////////////////////////////////////////////////


AFCloudPctFile::AFCloudPctFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AFCloudPctFile::~AFCloudPctFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AFCloudPctFile::AFCloudPctFile(const AFCloudPctFile & a)

{

init_from_scratch();

assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


AFCloudPctFile & AFCloudPctFile::operator=(const AFCloudPctFile & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AFCloudPctFile::init_from_scratch()

{

Buf = (unsigned char *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AFCloudPctFile::clear()

{

if ( Buf )  { delete [] Buf;  Buf = (unsigned char *) 0; }

AFDataFile::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AFCloudPctFile::assign(const AFCloudPctFile & a)

{

clear();

if ( !(a.Buf) )  return;

Buf = new unsigned char [af_nx*af_ny*cloud_pct_record_size];

memset(Buf, 0, af_nx*af_ny*cloud_pct_record_size);

AFDataFile::assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


bool AFCloudPctFile::read(const char * filename, const char input_hemi)

{

clear();

   //  Call parent to parse metadata

AFDataFile::read(filename, input_hemi);

int fd = -1;
int bytes;

if ( (fd = met_open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\nAFCloudPctFile::read(const char *) -> "
        << "can't open file \"" << filename << "\"\n\n";

   return ( false );

}

bytes = af_nx*af_ny*cloud_pct_record_size;

Buf = new unsigned char [bytes];

if ( ::read(fd, Buf, bytes) != bytes )  {

   mlog << Error << "\nAFCloudPctFile::read(const char *) -> "
        << "read error on file \"" << filename << "\"\n\n";

   ::close(fd);

   return ( false );

}

Filename = get_short_name(filename);

   //
   //  done
   //

::close(fd);

return ( true );

}


////////////////////////////////////////////////////////////////////////


int AFCloudPctFile::cloud_pct(int x, int y) const

{

int k, n;

n = two_to_one(x, y);   //  this function does range checking on x and y for us

k = (int) (Buf[n]);

return ( k );

}


////////////////////////////////////////////////////////////////////////

