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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"
#include "vx_nc_util.h"
#include "vx_tc_util.h"
#include "vx_tc_nc_util.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    program_name = get_short_name(argv[0]);

    if(argc != 2) {
        mlog << Error
             << "\nusage: " << program_name
             << " inputfile\n\n";
        exit(1);
    }

    ConcatString inputfile = (string) argv[1];

    mlog << Debug(1) << "Reading " << inputfile << "\n";

    NcFile* nc_out = open_ncfile(inputfile.c_str(), false);

    return 0;
}

////////////////////////////////////////////////////////////////////////
