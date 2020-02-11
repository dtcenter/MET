// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//    Filename:  data_cube.cc
//
//    Description:
//        Contains the definition of the DataCube class.
//
//    Mod    Date      Name           Description
//    ----   ----      ----           -----------
//    000    02-11-20  Fillmore       Initial definition
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include "data_cube.h"

#include "vx_log.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

DataCube::DataCube() {

    init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

DataCube::DataCube(const DataCube& d) {

    init_from_scratch();

    assign(d);
}

////////////////////////////////////////////////////////////////////////

DataCube::~DataCube() {

    clear();
}

////////////////////////////////////////////////////////////////////////

DataCube& DataCube::operator=(const DataCube& d) {

    if(this == &d) return(*this);

    assign(d);

    return(*this);
}

////////////////////////////////////////////////////////////////////////

void DataCube::clear() {

    Data.clear();

    Nx = 0;
    Ny = 0;
    Nz = 0;
    Nxyz = 0;

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::erase() {

    Data.resize(Nxyz);
    Data.assign(Nxyz, 0);

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::set_size(int nx, int ny, int nz) {

    Nx = nx;
    Ny = ny;
    Nz = nz;
    Nxyz = nx * ny * nz;

    Data.resize(Nxyz);
    Data.assign(Nxyz, 0);

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::set(double value, int i, int j, int k) {

    /*  note when i = Nx - 1, j = Ny - 1, k = Nz - 1
     *  n = Ny * Nz * (Nx - 1) + Nz * (Ny - 1) + Nz - 1
     *    = Nx * Ny * Nz - 1
     */
    int n = Ny * Nz * i + Nz * j + k;

    Data[n] = value;

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::set_constant(double value) {

    for (int i = 0; i < Nx; i++) {
        for (int j = 0; j < Ny; j++) {
            for (int k = 0; k < Nz; k++) {
                set(value, i, j, k);
            }
        }
    }

    return;
}

////////////////////////////////////////////////////////////////////////

double DataCube::get(int i, int j, int k) const {

    int n = Ny * Nz * i + Nz * j + k;

    return Data[n];
}

////////////////////////////////////////////////////////////////////////

void DataCube::init_from_scratch() {

    Nx = 0;
    Ny = 0;
    Nz = 0;
    Nxyz = 0;

    clear();
}

////////////////////////////////////////////////////////////////////////

void DataCube::assign(const DataCube& d) {

    clear();

    set_size(d.nx(), d.ny(), d.nz());

    Data = d.Data;

    return;
}

////////////////////////////////////////////////////////////////////////
