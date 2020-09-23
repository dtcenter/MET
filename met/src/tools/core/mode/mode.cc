// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
//
////////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mode_exec.h"

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


///////////////////////////////////////////////////////////////////////


void usage();   //  needs external linkage


///////////////////////////////////////////////////////////////////////


static const char program_name [] = "mode";

static ModeExecutive mode_exec;   //  gotta make this global ... not sure why

static const char default_config_filename [] = "MET_BASE/config/MODEConfig_default";

static void singlevar_usage();
static void multivar_usage();


///////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

if ( argc == 1 )  usage();

int j;
int status;
ModeConfInfo config;
StringArray Argv;
string s;
bool has_field_index = false;
const char * const user_config_filename = argv[3];



file_id = 0;   //  I hate having to do this


for (j=0; j<argc; ++j)  {

   if ( strcmp(argv[j], "-field_index") == 0 )  has_field_index = true;

   s = argv[j];

   Argv.add(s);

}



config.read_config  (default_config_filename, user_config_filename);


// cout << "\n\n   mode: is_multivar = " << config.is_multivar() << "\n\n" << flush;
// 
// config.conf.dump(cout);


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


///////////////////////////////////////////////////////////////////////


void usage()

{


cout << "\n*** Model Evaluation Tools (MET" << met_version << ") ***\n\n";


cout << "if singlevar mode:\n"
        "==================\n";


singlevar_usage();


cout << "if multivar mode:\n"
     << "=================\n";


multivar_usage();


exit ( 1 );

return;

}


///////////////////////////////////////////////////////////////////////


void singlevar_usage()

{


cout << "\n\n"
     << "Usage: " << program_name << "\n"
     << "\tfcst_file\n"
     << "\tobs_file\n"
     << "\tconfig_file\n"
     << "\t[-config_merge merge_config_file]\n"
     << "\t[-outdir path]\n"
     << "\t[-log file]\n"
     << "\t[-v level]\n"
     << "\t[-compress level]\n\n"

     << "\twhere\n"

     << "\t\t\"fcst_file\" is a gridded forecast file "
     << "containing the field to be verified (required).\n"

     << "\t\t\"obs_file\" is a gridded observation file "
     << "containing the verifying field (required).\n"

     << "\t\t\"config_file\" is a MODEConfig file "
     << "containing the desired configuration settings (required).\n"

     << "\t\t\"-config_merge merge_config_file\" overrides the default "
     << "fuzzy engine settings for merging within the fcst/obs fields "
     << "(optional).\n"

     << "\t\t\"-outdir path\" overrides the default output directory ("
     << mode_exec.out_dir << ") (optional).\n"

     << "\t\t\"-log file\" outputs log messages to the specified "
     << "file (optional).\n"

     << "\t\t\"-v level\" overrides the default level of logging ("
     << mlog.verbosity_level() << ") (optional).\n"

     << "\t\t\"-compress level\" overrides the compression level of NetCDF variable ("
     << mode_exec.engine.conf_info.get_compression_level() << ") (optional).\n\n" 

     << flush;


return;

}


///////////////////////////////////////////////////////////////////////


void multivar_usage()

{

cout << "\n\n"
     << "\tUsage:  "
     << program_name
     << " fcst obs config "
     << "[ mode_options ]\n\n"

     << "\twhere\n"

     << "\t\t\"fcst\" is the name of a file containing the forecaset files to be used (required)\n"
     << "\t\t\"obs\"  is the name of a file containing the observation files to be used (required)\n"
     << "\t\t\"config\" is a MODEConfig file "
     << "containing the desired configuration settings (required).\n"

     << flush;


return;

}



///////////////////////////////////////////////////////////////////////




