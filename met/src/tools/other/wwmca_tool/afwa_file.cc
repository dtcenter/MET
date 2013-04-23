

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2013
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
#include "wwmca_grids.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfwaDataFile
   //


////////////////////////////////////////////////////////////////////////


AfwaDataFile::AfwaDataFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfwaDataFile::~AfwaDataFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AfwaDataFile::AfwaDataFile(const AfwaDataFile & a)

{

init_from_scratch();

assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


AfwaDataFile & AfwaDataFile::operator=(const AfwaDataFile & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfwaDataFile::init_from_scratch()

{

grid = (const Grid *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaDataFile::clear()

{

if ( grid )  { delete grid;  grid = (const Grid *) 0; }

Filename.clear();

Hemisphere = 'N';

Valid = (unixtime) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void AfwaDataFile::assign(const AfwaDataFile & a)

{

clear();

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
      mlog << Error << "\nAfwaDataFile::assign(const AfwaDataFile &) -> bad hemisphere ... " << Hemisphere << "\n\n";
      exit ( 1 );
      break;

}


return;

}


////////////////////////////////////////////////////////////////////////


bool AfwaDataFile::xy_is_ok(int x, int y) const

{

double lat, lon;

grid->xy_to_latlon((double) x, (double) y, lat, lon);

if ( (Hemisphere == 'N') && (lat < 0.0) )  return ( false );
if ( (Hemisphere == 'S') && (lat > 0.0) )  return ( false );

return ( true );

}


////////////////////////////////////////////////////////////////////////


int AfwaDataFile::two_to_one(int x, int y) const

{

if ( (x < 0) || (x >= afwa_nx) || (y < 0) || (y >= afwa_ny) )  {

   mlog << Error << "\nAfwaDataFile::two_to_one(int, int) const -> range check error\n\n";

   exit ( 1 );

}

int n, yy;

yy = afwa_ny - 1 - y;

n = yy*afwa_nx + x;

return ( n );

}


////////////////////////////////////////////////////////////////////////





