// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////
//
//   Filename:   mode.cc
//
//   Description:
//      Based on user specified parameters, this tool derives objects
//      within two gridded datasets using a convolution-thresholding
//      approach.  It then compares objects within the same field and
//      across fields, and calculates a total interest value for each
//      pair of objects using a fuzzy logic approach.  The interest
//      values are thresholded, and object pairs with a high enough
//      interest value are associated with one another.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    10-11-06  Halley Gotway  New
//   001    12-20-06  Halley Gotway  Write check_xy_ll to reduce
//                                   duplicated code.
//   002    01/11/08  Halley Gotway  Modify sprintf statements which
//                                   use the GRIB code abbreviation string
//   003    02/06/08  Halley Gotway  Modify to read the updated NetCDF
//                                   output of PCP-Combine
//   004    09/23/08  Halley Gotway  Add two output fields to the
//                                   NetCDF object file for the raw fcst/obs values.
//   005    09/23/08  Halley Gotway  Change argument sequence for the
//                                   get_grib_record routine.
//   006    05/03/10  Halley Gotway  Remove the variable/level info
//                                   from the output file naming convention.
//   007    05/11/10  Halley Gotway  Plot polyline lines thicker.
//   008    06/30/10  Halley Gotway  Enhance grid equality checks.
//   009    07/27/10  Halley Gotway  Add lat/lon variables to NetCDF.
//   010    08/09/10  Halley Gotway  Add valid time variable attributes
//                                   to NetCDF output.
//   011    10/28/11  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//   012    11/15/11  Holmes         Added code to enable reading of
//                                   multiple config files.
//
//   013    01/11/11  Bullock        Ported to new repository
//   014    05/10/12  Halley Gotway  Switch to using vx_config library.
//   015    05/10/12  Halley Gotway  Move -fcst_valid, -fcst_lead,
//                                   -obs_valid, and -obs_lead command line options
//                                   to config file.
//   016    02/25/15  Halley Gotway  Add automated regridding.
//   017    05/20/16  Prestopnik J   Removed -version (now in command_line.cc)
//   018    04/08/19  Halley Gotway  Add percentile thresholds.
//   019    04/01/19  Fillmore       Add FCST and OBS units.
//   020    07/06/22  Howard Soh     METplus-Internal #19 Rename main to met_main
//
////////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "main.h"
#include "string_array.h"
#include "mode_usage.h"
#include "mode_conf_info.h"

#ifdef WITH_PYTHON
#include "global_python.h"
#endif


///////////////////////////////////////////////////////////////////////
//
// Create output files using the following naming convention:
//
//    mode_YYYYMMDDI_FHH_HHA.ps/.txt
//
//    Where I indicates initilization time, L indicates lead time,
//    and A indicates accumulation time.
//
///////////////////////////////////////////////////////////////////////


extern int     mode_frontend(const StringArray &);
extern int multivar_frontend(const StringArray &);

extern const char * const program_name;   


///////////////////////////////////////////////////////////////////////


   //
   //  these need external linkage
   //

const char * const program_name = "mode";   


///////////////////////////////////////////////////////////////////////


static const char default_config_filename [] = "MET_BASE/config/MODEConfig_default";


///////////////////////////////////////////////////////////////////////


int met_main(int argc, char * argv [])
{

int j, n;
int status;
ModeConfInfo config;
StringArray Argv;
string s;
bool has_field_index = false;
const char * user_config_filename = 0;

for (j=0,n=0; j<argc; ++j)  {

   //
   //  all options take exactly one argument
   //

   if ( argv[j][0] == '-' )  j++;
   else                      n++;

   //
   //  the config file is the 4th required argv item
   //

   if ( n == 4 )  {

      user_config_filename = argv[j];
      break;

   }

}

   //
   //  check for enough required arguments
   //

if ( !user_config_filename )  both_usage();

for (j=0; j<argc; ++j)  {

   if ( strcmp(argv[j], "-field_index") == 0 )  has_field_index = true;

   s = argv[j];

   Argv.add(s);

}

config.read_config  (default_config_filename, user_config_filename);

if ( config.is_multivar() && !has_field_index )  {

   status = multivar_frontend(Argv);

} else {

   status = mode_frontend(Argv);

}

   //
   //  done
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////

const string get_tool_name()
{
   return "mode";
}

///////////////////////////////////////////////////////////////////////



