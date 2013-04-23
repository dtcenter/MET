

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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include <netcdf.hh>

#include "is_nccf_file.h"
#include "is_netcdf_file.h"


////////////////////////////////////////////////////////////////////////


static const char nccf_att_name[]  = "Conventions";
static const char nccf_att_value[] = "CF-1.0";


////////////////////////////////////////////////////////////////////////


bool is_nccf_file(const char * filename)

{

   //
   //  check that it's a netcdf file
   //
   //  this also checks that the filename string is non-null and nonempty
   //

if ( ! is_netcdf_file(filename) )  return ( false );

int j, n;
NcFile f (filename);
NcAtt * att = (NcAtt *) 0;

   //
   //  look for the global attribute name
   //

if ( ! f.is_valid() )  return ( false );

n = f.num_atts();

for (j=0; j<n; ++j)  {

   att = f.get_att(j);

   if ( strcmp(att->name(), nccf_att_name) == 0 )  {
	  
	   if ( strcmp(att->as_string(0), nccf_att_value) == 0 )  return ( true );

	}
}

   //
   //  done
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////



