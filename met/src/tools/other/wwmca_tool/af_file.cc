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
#include "vx_grid.h"
#include "vx_cal.h"

#include "af_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AFDataFile
   //


////////////////////////////////////////////////////////////////////////


AFDataFile::AFDataFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AFDataFile::~AFDataFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AFDataFile::AFDataFile(const AFDataFile & a)

{

init_from_scratch();

assign(a);

return;

}


////////////////////////////////////////////////////////////////////////


AFDataFile & AFDataFile::operator=(const AFDataFile & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AFDataFile::init_from_scratch()

{

grid = (const Grid *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AFDataFile::clear()

{

if ( grid )  { delete grid;  grid = (const Grid *) 0; }

Filename.clear();

Hemisphere = bad_data_char;

Init = (unixtime) 0;

Valid = (unixtime) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void AFDataFile::assign(const AFDataFile & a)

{

clear();

Filename = a.Filename;

Hemisphere = a.Hemisphere;

Init = a.Init;

Valid = a.Valid;

switch ( Hemisphere )  {

   case 'N':
      grid = new Grid(wwmca_north_data);
      break;

   case 'S':
      grid = new Grid(wwmca_south_data);
      break;

   default:
      mlog << Error << "\nAFDataFile::assign(const AFDataFile &) -> bad hemisphere ... " << Hemisphere << "\n\n";
      exit ( 1 );
      break;

}


return;

}


////////////////////////////////////////////////////////////////////////


bool AFDataFile::read(const char * filename, const char input_hemi)

{

clear();

char file_hemi = bad_data_char;

mlog << Debug(1)
     << "Reading input file: " << filename << "\n";

   //  Parse the filename

parse_af_filename(filename, file_hemi, Init, Valid);

   //  Determine the hemisphere

if ( is_bad_data(input_hemi) && is_bad_data(file_hemi) )  {

   mlog << Warning << "\nAFDataFile::read(const char *) -> "
        << "cannot determine the hemisphere, assuming northern.\n\n";

   Hemisphere = 'N';

}
else if ( !is_bad_data(input_hemi) && !is_bad_data(file_hemi)  &&
          input_hemi != file_hemi )  {

   mlog << Warning << "\nAFDataFile::read(const char *) -> "
        << "the command line hemisphere (" << input_hemi
        << ") and filename hemisphere (" << file_hemi
        << ") do not match for \"" << filename
        << "\", using command line value.\n\n";

   Hemisphere = input_hemi;

}
else if ( !is_bad_data(input_hemi) )  Hemisphere = input_hemi;
else                                  Hemisphere = file_hemi;

   //  Check for unset valid time

if ( Valid == (unixtime) 0 )  {

   mlog << Warning << "\nAFDataFile::read(const char *) -> "
        << "unable to parse timing information from filename \"" << filename
        << "\"\n\n";

}

if ( Hemisphere == 'N' )  grid = new Grid(wwmca_north_data);
else                      grid = new Grid(wwmca_south_data);

return ( true );

}

////////////////////////////////////////////////////////////////////////


bool AFDataFile::xy_is_ok(int x, int y) const

{

double lat, lon;

grid->xy_to_latlon((double) x, (double) y, lat, lon);

if ( (Hemisphere == 'N') && (lat < 0.0) )  return ( false );
if ( (Hemisphere == 'S') && (lat > 0.0) )  return ( false );

return ( true );

}


////////////////////////////////////////////////////////////////////////


int AFDataFile::two_to_one(int x, int y) const

{

if ( (x < 0) || (x >= af_nx) || (y < 0) || (y >= af_ny) )  {

   mlog << Error << "\nAFDataFile::two_to_one(int, int) const -> range check error\n\n";

   exit ( 1 );

}

int n, yy;

yy = af_ny - 1 - y;

n = yy*af_nx + x;

return ( n );

}


////////////////////////////////////////////////////////////////////////


void parse_af_filename(const char * filename, char & Hemisphere,
                       unixtime & Init, unixtime & Valid)

{

int j;
StringArray tokens;
unixtime file_ut  = (unixtime) 0;
unixtime file_sec = bad_data_int;

   //  Initialize

Hemisphere = bad_data_char;
Valid = (unixtime) 0;
Init  = (unixtime) 0;

if ( !filename )  return;

   //  Tokenize the filename

tokens.parse_delim(get_short_name(filename), "_.");

   //  Loop through and interpret the tokens

for (j=0; j<tokens.n_elements(); ++j)  {

   //  Unixtime in the form YYYYMMDDHH[MMSS]
  if ( is_number(tokens[j].c_str()) && tokens[j].length() >= 10 )
      file_ut = timestring_to_unix(tokens[j].c_str());

   //  Lead time in hours as a 1 - 4 digit number
  else if ( is_number(tokens[j].c_str()) && tokens[j].length() <= 4)
      file_sec = atoi(tokens[j].c_str()) * sec_per_hour;

   else if ( strcasecmp(tokens[j].c_str(), "NH") == 0 )  Hemisphere = 'N';

   else if ( strcasecmp(tokens[j].c_str(), "SH") == 0 )  Hemisphere = 'S';

}

if ( is_bad_data(Hemisphere) )  {

   mlog << Warning << "\nparse_af_filename() -> "
        << "cannot determine the hemisphere from filename \""
        << filename << "\"\n\n";

}
else  {

   mlog << Debug(2)
        << "Parsed hemisphere (" << Hemisphere << ") from filename.\n";

}

   //  Interpret the timestamp of the file
if ( file_ut != (unixtime) 0 )  {

   //  If lead time undefined, store file time as valid time
   if ( is_bad_data(file_sec) )  {

      Init  = file_ut;
      Valid = file_ut;

      mlog << Debug(2) << "Parsed valid time ("
           << unix_to_yyyymmdd_hhmmss(Valid) << ") from filename.\n";

   }

   //  Otherwise, store file time as initialization time
   else  {

      Init  = file_ut;
      Valid = file_ut + file_sec;

      mlog << Debug(2) << "Parsed initialization ("
           << unix_to_yyyymmdd_hhmmss(Init) << "), lead ("
           << sec_to_hhmmss(Valid-Init) << "), and valid ("
           << unix_to_yyyymmdd_hhmmss(Valid) << ") times from filename.\n";

   }
}

return;

}


////////////////////////////////////////////////////////////////////////

