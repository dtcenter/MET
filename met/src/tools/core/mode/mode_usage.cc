
////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "mode_usage.h"
#include "mode_exec.h"

#include "util_constants.h"
#include "logger.h"


////////////////////////////////////////////////////////////////////////


extern const char * const program_name;

extern ModeExecutive mode_exec;


////////////////////////////////////////////////////////////////////////


void both_usage()

{


cout << "\n*** Model Evaluation Tools (MET" << met_version << ") ***\n\n";

cout << "Single Variable MODE:\n"
        "=====================\n";

singlevar_usage();

cout << "Multi Variable MODE:\n"
     << "====================\n";

multivar_usage();

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


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

     << "\t\t\"-outdir path\" overrides the default output directory "
     << "(./) (optional).\n"

     << "\t\t\"-log file\" outputs log messages to the specified "
     << "file (optional).\n"

     << "\t\t\"-v level\" overrides the default level of logging ("
     << mlog.verbosity_level() << ") (optional).\n"

     << "\t\t\"-compress level\" overrides the compression level of "
     << "NetCDF variable (optional).\n\n"

     << flush;


return;

}


////////////////////////////////////////////////////////////////////////


void multivar_usage()

{

cout << "\n\n"
     << "\tUsage: " << program_name << "\n"
     << "\tfcst_file_list\n"
     << "\tobs_file_list\n"
     << "\tconfig_file\n"
     << "\t[-outdir path]\n"
     << "\t[-log file]\n"
     << "\t[-v level]\n"

     << "\twhere\n"

     << "\t\t\"fcst_file_list\" is an ASCII file containing a list of "
     << "forecast files to be used (required).\n"

     << "\t\t\"obs_file_list\"  is an ASCII file containing a list of "
     << "observation files to be used (required).\n"

     << "\t\t\"config\" is a MODEConfig file containing the desired "
     << "configuration settings (required).\n"

     << "\t\t\"-outdir path\" overrides the default output directory "
     << "(./) (optional).\n"

     << "\t\t\"-log file\" outputs log messages to the specified file "
     << "(optional).\n"

     << "\t\t\"-v level\" overrides the default level of logging ("
     << mlog.verbosity_level() << ") (optional).\n\n"

     << flush;


return;

}


////////////////////////////////////////////////////////////////////////
