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
#include "vx_log.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_grid.h"
#include "series_pdf.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    program_name = get_short_name(argv[0]);

    DataPlane dp;

    int nx = 90;
    int ny = 45;

    dp.set_size(nx, ny);

    for(int i = 0; i < nx; i++) {
        for(int j = 0; j < ny; j++) {
            double value = exp(-(i*i + j*j) / 10);
            dp.set(value, i, j);
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////
