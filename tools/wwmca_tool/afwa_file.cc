

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

#include "vx_util/vx_util.h"
#include "vx_cal/vx_cal.h"

#include "afwa_file.h"


////////////////////////////////////////////////////////////////////////


static const int afwa_nx = 1024;
static const int afwa_ny = 1024;


////////////////////////////////////////////////////////////////////////


static bool is_afwa_cloud_pct_filename(const char * filename, char & Hemisphere, unixtime & Valid);


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

grid = (const Grid *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaCloudPctFile::clear()

{

if ( Buf )  { delete [] Buf;  Buf = (unsigned char *) 0; }

if ( grid )  { delete grid;  grid = (const Grid *) 0; }

Filename.clear();

Hemisphere = 'N';

Valid = (unixtime) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaCloudPctFile::assign(const AfwaCloudPctFile & a)

{

clear();

if ( !(a.Buf) )  return;

Buf = new unsigned char [afwa_nx*afwa_ny];

memset(Buf, 0, afwa_nx*afwa_ny);

Filename = a.Filename;

Hemisphere = a.Hemisphere;

Valid = a.Valid;

switch ( Hemisphere )  {

   case 'N':
      grid = new Grid(wwmca_north_data);
      break;

   case 'S':
      grid = new Grid(wwmca_south_data);
      break;

   default:
      cerr << "\n\n  AfwaCloudPctFile::assign(const AfwaCloudPctFile &) -> bad hemisphere ... " << Hemisphere << "\n\n";
      exit ( 1 );
      break;

}


return;

}


////////////////////////////////////////////////////////////////////////


int AfwaCloudPctFile::nx() const

{

return ( afwa_nx );

}


////////////////////////////////////////////////////////////////////////


int AfwaCloudPctFile::ny() const

{

return ( afwa_ny );

}


////////////////////////////////////////////////////////////////////////


bool AfwaCloudPctFile::read(const char * filename)

{

clear();

if ( !(is_afwa_cloud_pct_filename(filename, Hemisphere, Valid)) )  {

   clear();

   cerr << "\n\n  AfwaCloudPctFile::read(const char *) -> can't parse filename \"" << filename << "\"\n\n";

   return ( false );

}

if ( Hemisphere == 'N' )  grid = new Grid(wwmca_north_data);
else                      grid = new Grid(wwmca_south_data);

int fd = -1;
int n_read, bytes;

if ( (fd = open(filename, O_RDONLY)) < 0 )  {

   cerr << "\n\n  AfwaCloudPctFile::read(const char *) -> can't open file \"" << filename << "\"\n\n";

   //  exit ( 1 );

   return ( false );

}

bytes = afwa_nx*afwa_ny;

Buf = new unsigned char [bytes];

if ( (n_read = ::read(fd, Buf, bytes)) != bytes )  {

   cerr << "\n\n  AfwaCloudPctFile::read(const char *) -> read error on file \"" << filename << "\"\n\n";

   //  exit ( 1 );

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


int AfwaCloudPctFile::get_value(int x, int y) const

{

int k, n;

n = two_to_one(x, y);   //  this functin does range checking on x and y for us

k = (int) (Buf[n]);

return ( k );

}


////////////////////////////////////////////////////////////////////////


bool AfwaCloudPctFile::xy_is_ok(int x, int y) const

{

double lat, lon;

grid->xy_to_latlon((double) x, (double) y, lat, lon);

if ( (Hemisphere == 'N') && (lat < 0.0) )  return ( false );
if ( (Hemisphere == 'S') && (lat > 0.0) )  return ( false );

return ( true );

}


////////////////////////////////////////////////////////////////////////


int AfwaCloudPctFile::two_to_one(int x, int y) const

{

if ( (x < 0) || (x >= afwa_nx) || (y < 0) || (y >= afwa_ny) )  {

   cerr << "\n\n  AfwaCloudPctFile::two_to_one(int, int) const -> range check error\n\n";

   exit ( 1 );

}

int n, yy;

yy = afwa_ny - 1 - y;

n = yy*afwa_nx + x;

return ( n );

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


if ( strlen(short_name) != 35 )  return ( false );

if ( strncmp(short_name, "WWMCA_TOTAL_CLOUD_PCT_", 22) != 0 )  return ( false );

     if ( strncmp(short_name + 22, "NH_", 3) == 0 )  Hemisphere = 'N';
else if ( strncmp(short_name + 22, "SH_", 3) == 0 )  Hemisphere = 'S';
else return ( false );

int j;

for (j=25; j<=34; ++j)  {

   if ( !isdigit(short_name[j]) )  return ( false );

}

int month, day, year, hour;
char junk[256];

substring(short_name, junk, 0, 3);

year = atoi(junk);

substring(short_name, junk, 4, 5);

month = atoi(junk);

substring(short_name, junk, 6, 7);

day = atoi(junk);

substring(short_name, junk, 8, 9);

hour = atoi(junk);


Valid = mdyhms_to_unix(month, day, year, hour, 0, 0);

   //
   //  ok
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////





