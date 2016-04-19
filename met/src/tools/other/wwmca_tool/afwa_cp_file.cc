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


bool AfwaCloudPctFile::read(const char * filename, const char hemisphere)

{

mlog << Debug(1)
     << "Reading input file: " << filename << "\n";

clear();

char filename_hemi;

Hemisphere = hemisphere;

if ( is_afwa_cloud_pct_filename(filename, filename_hemi, Valid) )  {

   if ( Hemisphere != filename_hemi )  {

      mlog << Error << "\nAfwaCloudPctFile::read(const char *) -> "
           << "inconsistent hemisphere definition (" << Hemisphere
           << " != " << filename_hemi << ") for filename \"" << filename
           << "\"\n\n";

      return ( false );

   }

}
else  {

   Valid = (unixtime) 0;

}

if ( Hemisphere == 'N' )  grid = new Grid(wwmca_north_data);
else                      grid = new Grid(wwmca_south_data);

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


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


   //
   //  example: WWMCA_TOTAL_CLOUD_PCT_NH_2009081718
   //

bool is_afwa_cloud_pct_filename(const char * filename, char & Hemisphere, unixtime & Valid)

{

if ( !filename )  return ( false );

const char * short_name = get_short_name(filename);


if ( (int)strlen(short_name) != cp_filename_length )  return ( false );

if ( strncmp(short_name, "WWMCA_TOTAL_CLOUD_PCT_", cp_filename_prefix_length) != 0 )  return ( false );

     if ( strncmp(short_name + cp_filename_prefix_length, "NH_", cp_hemisphere_length) == 0 )  Hemisphere = 'N';
else if ( strncmp(short_name + cp_filename_prefix_length, "SH_", cp_hemisphere_length) == 0 )  Hemisphere = 'S';
else return ( false );

int j;

for (j=cp_datetime_start_pos; j<=cp_datetime_end_pos; ++j)  {

   if ( !isdigit(short_name[j]) )  return ( false );

}

int month, day, year, hour;
char junk[max_string_length];
const char * c = short_name + cp_datetime_start_pos;

substring(c, junk, cp_year_start_pos, cp_year_end_pos);

year = atoi(junk);

substring(c, junk, cp_month_start_po, cp_month_end_pos);

month = atoi(junk);

substring(c, junk, cp_day_start_pos, cp_day_end_pos);

day = atoi(junk);

substring(c, junk, cp_hour_start_pos, cp_hour_end_pos);

hour = atoi(junk);


Valid = mdyhms_to_unix(month, day, year, hour, 0, 0);

   //
   //  ok
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////





