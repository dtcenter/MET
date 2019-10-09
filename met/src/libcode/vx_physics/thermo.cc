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

#include "thermo.h"

////////////////////////////////////////////////////////////////////////

double virtual_temperature(double temperature, double mixing_ratio) {
    return temperature
        * (epsilon + mixing_ratio) / (epsilon * (1 + mixing_ratio));
}

////////////////////////////////////////////////////////////////////////

double mixing_ratio(double partial_pressure, double pressure) {
    return epsilon * partial_pressure / (pressure - partial_pressure);
}

////////////////////////////////////////////////////////////////////////

double saturation_vapor_pressure(double temperature) {
    return 6.112
        * exp(17.67 * (temperature - 273.15) / (temperature - 29.65));
}

////////////////////////////////////////////////////////////////////////

double saturation_mixing_ratio(double pressure, double temperature) {
    return mixing_ratio(
        saturation_vapor_pressure(temperature), pressure);
}

////////////////////////////////////////////////////////////////////////

double relative_humidity_from_mixing_ratio(double mixing_ratio,
    double temperature, double pressure) {
    return 100 * mixing_ratio
        / saturation_mixing_ratio(pressure, temperature);
}

////////////////////////////////////////////////////////////////////////

double mixing_ratio_from_relative_humidity(double relative_humidity,
    double temperature, double pressure) {
    return relative_humidity / 100
        * saturation_mixing_ratio(pressure, temperature);
}

////////////////////////////////////////////////////////////////////////

void height_from_pressure(int nlev,
    double surface_pressure,
    double* virtual_temperature,
    double* pressure, double* height) {

    for (int k = 0; k < nlev; k++) {
    }
}

////////////////////////////////////////////////////////////////////////
