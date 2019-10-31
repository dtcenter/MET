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
    int n,
    vector<int>& pdf) {

    for(int k = 0; k < n; k++) {
        pdf.push_back(0);
    }
}

////////////////////////////////////////////////////////////////////////

void init_pdf(
    double min,
    double max,
    double delta,
    vector<int>& pdf) {

    int n = (max - min) / delta;
    for(int k = 0; k < n; k++) {
        pdf.push_back(0);
    }
}

////////////////////////////////////////////////////////////////////////

void update_pdf(
    double min,
    double delta,
    vector<int>& pdf,
    const DataPlane& dp,
    const MaskPlane& mp) {

    for(int i = 0; i < dp.nx(); i++) {
        for(int j = 0; j < dp.ny(); j++) {
            if(mp.s_is_on(i, j)) {
                double value = dp.get(i, j);
                int k = floor((value - min) / delta);
                if(k < 0) k = 0;
                if(k >= pdf.size()) k = pdf.size() - 1;
                pdf[k]++;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////

void print_pdf(
    double min,
    double delta,
    const vector<int>& pdf) {

    for(int k = 0; k < pdf.size(); k++) {
        double bin_min = min + k * delta;
        double bin_max = min + (k + 1) * delta;
        cout << "[" << bin_min << ", " << bin_max << "] "
             << pdf[k] << "\n";
    }
}

////////////////////////////////////////////////////////////////////////

void write_nc_pdf(
    NcFile* nc_out,
    const VarInfo&,
    double min,
    double delta,
    const vector<int>& pdf) {

}

////////////////////////////////////////////////////////////////////////
