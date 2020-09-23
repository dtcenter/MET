// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

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
    MaskPlane mp;

    int nx = 90;
    int ny = 45;
    int i_min = -10;
    int i_max = 10;
    int j_min = -10;
    int j_max = 10;

    int nbin = 10;
    double min = 0;
    double max = 0.5;
    double delta = (max - min) / nbin;
    double width = 10;

    vector<int> pdf;

    init_pdf(nbin, pdf);

    dp.set_size(nx, ny);
    mp.set_size(nx, ny);

    bool mask;

    for(int i = 0; i < nx; i++) {
        for(int j = 0; j < ny; j++) {
            double value = exp(-(i*i + j*j) / (width * width));
            dp.set(value, i, j);
            mask = (i_min <= i && i <= i_max)
                && (j_min <= j && j <= j_max);
            if(mask) {
                mp.put(true, i, j);
            } else {
                mp.put(false, i, j);
            }
        }
    }

    update_pdf(min, delta, pdf, dp, mp);

    print_pdf(min, delta, pdf);

    return 0;
}

////////////////////////////////////////////////////////////////////////
