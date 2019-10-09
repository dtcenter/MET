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

double virtual_temperature(double temperature, double mixing) {
    return temperature * (epsilon + mixing) / (epsilon * (1 + mixing));
}

////////////////////////////////////////////////////////////////////////

double mixing_ratio(double part_press, double tot_press) {
    return epsilon * part_press / (tot_press - part_press);
}

////////////////////////////////////////////////////////////////////////

double saturation_vapor_pressure(double temperature) {
    return 6.112 * exp(17.67 * (temperature - 273.15) / (temperature - 29.65));
}

////////////////////////////////////////////////////////////////////////

double saturation_mixing_ratio(double tot_press, double temperature) {
    return mixing_ratio(saturation_vapor_pressure(temperature), tot_press);
}

////////////////////////////////////////////////////////////////////////
