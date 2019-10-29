// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "series_pdf.h"

////////////////////////////////////////////////////////////////////////

void init_pdf(
    float min, float max, float delta,
    vector<int>& pdf) {

    int n = (max - min) / delta;
    for(int k = 0; k < n; k++) {
        pdf.push_back(0);
    }
}

////////////////////////////////////////////////////////////////////////

void series_data_pdf(
    float min, float delta,
    vector<int>& pdf,
    const Grid& grid,
    const DataPlane& dp,
    const MaskPlane& mp) {

    for(int i = 0; i < grid.nx(); i++) {
        for(int j = 0; j < grid.ny(); j++) {
            float value = dp.get(i, j);
            int k = (value - min) / delta;
            if(k < 0) k = 0;
            if(k >= pdf.size()) k = pdf.size() - 1;
            pdf[k]++;
        }
    }
}

////////////////////////////////////////////////////////////////////////
