// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

#include "afwa_file.h"
#include "afwa_pt_file.h"
#include "wwmca_grids.h"


////////////////////////////////////////////////////////////////////////


// pixel age time records are 4 bytes each
static const int pixel_time_record_size    =   4;

// awfa pixel age time begins at December 31, 1967 00:00:00
static const unixtime afwa_start = mdyhms_to_unix(12, 31, 1967, 0, 0, 0);

// the maximum length for a datetime string
static const int max_string_length         = 256;

// the lengths of the filename parts
static const int pt_filename_length        =  35;
static const int pt_filename_prefix_length =  22;
static const int pt_hemisphere_length      =   3;

// the starting and ending positions for the date and time string from the filename
static const int pt_datetime_start_pos     =  25;
static const int pt_datetime_end_pos       =  34;

// the date and time pieces starting and ending positions from the datetime string
static const int pt_year_start_pos         =   0;
static const int pt_year_end_pos           =   3;
static const int pt_month_start_po         =   4;
static const int pt_month_end_pos          =   5;
static const int pt_day_start_pos          =   6;
static const int pt_day_end_pos            =   7;
static const int pt_hour_start_pos         =   8;
static const int pt_hour_end_pos           =   9;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfwaPixelTimeFile
   //


////////////////////////////////////////////////////////////////////////


AfwaPixelTimeFile::AfwaPixelTimeFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfwaPixelTimeFile::~AfwaPixelTimeFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AfwaPixelTimeFile::AfwaPixelTimeFile(const AfwaPixelTimeFile & a)

{

init_from_scratch();

assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


AfwaPixelTimeFile & AfwaPixelTimeFile::operator=(const AfwaPixelTimeFile & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfwaPixelTimeFile::init_from_scratch()

{

Buf = (unsigned char *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaPixelTimeFile::clear()

{

if ( Buf )  { delete [] Buf;  Buf = (unsigned char *) 0; }

AfwaDataFile::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaPixelTimeFile::assign(const AfwaPixelTimeFile & a)

{

clear();

if ( !(a.Buf) )  return;

Buf = new unsigned char [afwa_nx*afwa_ny*pixel_time_record_size];

memset(Buf, 0, afwa_nx*afwa_ny*pixel_time_record_size);

AfwaDataFile::assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


bool AfwaPixelTimeFile::read(const char * filename, const char input_hemi)

{

clear();

   //  Call parent to parse metadata

AfwaDataFile::read(filename, input_hemi);

int fd = -1;
int bytes;

if ( (fd = open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\nAfwaPixelTimeFile::read(const char *) -> "
        << "can't open file \"" << filename << "\"\n\n";

   return ( false );

}

bytes = afwa_nx*afwa_ny*pixel_time_record_size;

Buf = new unsigned char [bytes];

if ( ::read(fd, Buf, bytes) != bytes )  {

   mlog << Error << "\nAfwaPixelTimeFile::read(const char *) -> "
        << "read error on file \"" << filename << "\"\n\n";

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


int AfwaPixelTimeFile::pixel_age_sec(int x, int y) const

{

int k, n;
unixtime t = 0;

n = two_to_one(x, y);   //  this function does range checking on x and y for us

int *ibuf = (int *)Buf;

int minutes = ibuf[n];

if ( native_endian != big_endian )  shuffle_4(&minutes);

if (minutes == 0)  k = 0;
else {

   t = 60LL * minutes;      // convert minutes to long long in seconds

   t += afwa_start;         // convert afwa time to unixtime

   k = (int) (Valid - t);   // subtract from the filetime (Valid) to get pixel age in seconds

}

return ( k );

}


////////////////////////////////////////////////////////////////////////

