
////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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


cout << "if singlevar mode:\n"
        "==================\n";


singlevar_usage();


cout << "if multivar mode:\n"
     << "=================\n";


multivar_usage();


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void multivar_usage()

{

cout << "\n\n"
     << "\tUsage:  "
     << program_name
     << " fcst obs config "
     << "[ mode_options ]\n\n"

     << "\twhere\n"

     << "\t\t\"fcst\" is the name of a file containing the forecast files to be used (required)\n"
     << "\t\t\"obs\"  is the name of a file containing the observation files to be used (required)\n"
     << "\t\t\"config\" is a MODEConfig file "
     << "containing the desired configuration settings (required).\n\n"

     << flush;


return;

}


////////////////////////////////////////////////////////////////////////




