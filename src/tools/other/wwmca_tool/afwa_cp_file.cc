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
#include "afwa_cp_file.h"
#include "wwmca_grids.h"


////////////////////////////////////////////////////////////////////////


// cloud percent records are 1 byte each
static const int cloud_pct_record_size    =   1;

// the maximum length for a datetime string
static const int max_string_length         = 256;

// the lengths of the filename parts
static const int cp_filename_length        =  35;
static const int cp_filename_prefix_length =  22;
static const int cp_hemisphere_length      =   3;

// the starting and ending positions for the date and time string from the filename
static const int cp_datetime_start_pos     =  25;
static const int cp_datetime_end_pos       =  34;

// the date and time pieces starting and ending positions from the datetime string
static const int cp_year_start_pos         =   0;
static const int cp_year_end_pos           =   3;
static const int cp_month_start_po         =   4;
static const int cp_month_end_pos          =   5;
static const int cp_day_start_pos          =   6;
static const int cp_day_end_pos            =   7;
static const int cp_hour_start_pos         =   8;
static const int cp_hour_end_pos           =   9;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfwaCloudPctFile
   //


////////////////////////////////////////////////////////////////////////


AfwaCloudPctFile::AfwaCloudPctFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfwaCloudPctFile::~AfwaCloudPctFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AfwaCloudPctFile::AfwaCloudPctFile(const AfwaCloudPctFile & a)

{

init_from_scratch();

assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


AfwaCloudPctFile & AfwaCloudPctFile::operator=(const AfwaCloudPctFile & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfwaCloudPctFile::init_from_scratch()

{

Buf = (unsigned char *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaCloudPctFile::clear()

{

if ( Buf )  { delete [] Buf;  Buf = (unsigned char *) 0; }

AfwaDataFile::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaCloudPctFile::assign(const AfwaCloudPctFile & a)

{

clear();

if ( !(a.Buf) )  return;

Buf = new unsigned char [afwa_nx*afwa_ny*cloud_pct_record_size];

memset(Buf, 0, afwa_nx*afwa_ny*cloud_pct_record_size);

AfwaDataFile::assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


bool AfwaCloudPctFile::read(const char * filename, const char input_hemi)

{

clear();

   //  Call parent to parse metadata

AfwaDataFile::read(filename, input_hemi);

int fd = -1;
int bytes;

if ( (fd = open(filename, O_RDONLY)) < 0 )  {

   mlog << Error << "\nAfwaCloudPctFile::read(const char *) -> "
        << "can't open file \"" << filename << "\"\n\n";

   return ( false );

}

bytes = afwa_nx*afwa_ny*cloud_pct_record_size;

Buf = new unsigned char [bytes];

if ( ::read(fd, Buf, bytes) != bytes )  {

   mlog << Error << "\nAfwaCloudPctFile::read(const char *) -> "
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


int AfwaCloudPctFile::cloud_pct(int x, int y) const

{

int k, n;

n = two_to_one(x, y);   //  this function does range checking on x and y for us

k = (int) (Buf[n]);

return ( k );

}


////////////////////////////////////////////////////////////////////////

