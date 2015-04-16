

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
static const char nccf_att_value[] = "CF-";


////////////////////////////////////////////////////////////////////////


bool is_nccf_file(const char * filename)
{
  // Look for the global attribute name

  NcFile nc_file(filename);

  if (!nc_file.is_valid())
    return false;
  
  int num_atts = nc_file.num_atts();
  
  for (int j = 0; j < num_atts; ++j)
  {
    NcAtt *att = nc_file.get_att(j);

    if (strcmp(att->name(), nccf_att_name) == 0)
    {
      if (strncmp(att->as_string(0), nccf_att_value,
		  strlen(nccf_att_value)) == 0)
	return true;
    }
  }

  //  If we get here, this is not a CF-compliant netCDF file

  return false;
}


////////////////////////////////////////////////////////////////////////



