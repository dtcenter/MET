

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

#include <netcdf.hh>

#include "is_pinterp_file.h"
#include "is_netcdf_file.h"


////////////////////////////////////////////////////////////////////////


static const char title_string  [] = "TITLE";

static const char target_string [] = "ON PRES LEVELS";


////////////////////////////////////////////////////////////////////////


bool is_pinterp_file(const char * filename)

{

if ( !filename )  return ( false );

int j, n;
bool found = false;
NcFile f (filename);
NcAtt * att = (NcAtt *) 0;

   //
   //  check that it's a netcdf file
   //

if ( ! is_netcdf_file(filename) )  return ( false );

   //
   //  look for the target string in the TITLE global attribute
   //

if ( ! f.is_valid() )  return ( false );

n = f.num_atts();

found = false;

for (j=0; j<n; ++j)  {

   att = f.get_att(j);

   if ( strcmp(att->name(), "TITLE") == 0 )  {

      if ( strstr(att->as_string(0), target_string) )  { found = true;  break; }

   }

}

if ( !found )  return ( false );

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////



