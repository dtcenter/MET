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
//        Definition of the DataCube class.
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

DataCube::DataCube(const DataCube& other) {

    init_from_scratch();

    assign(other);
}

////////////////////////////////////////////////////////////////////////

DataCube::~DataCube() {

    clear();
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

    if (Data.empty()) {
        mlog << Error << "\nDataCube::set_constant-> "
             << "data array is empty.\n\n";
        exit(1);
    }

    for (int n = 0; n < Nxyz; n++) {
        Data[n] = value;
    }

    return;
}

////////////////////////////////////////////////////////////////////////

bool DataCube::shape_equal(const DataCube& other) {

    return (Nx == other.Nx) && (Ny == other.Ny) && (Nz == other.Nz);
}

////////////////////////////////////////////////////////////////////////

void DataCube::check_shape_equal(const DataCube& other) {

    if (!(this->shape_equal(other))) {
        mlog << Error << "\nDataCube::check_shape_equal-> "
             << "(" << Nx << ", " << Ny << ", " << Nz << ") != ("
             << other.Nx << ", " << other.Ny << ", " << other.Nz << ")"
             << "\n\n";
        exit(1);
    }

    return;
}

////////////////////////////////////////////////////////////////////////

double DataCube::get(int i, int j, int k) const {

    int n = Ny * Nz * i + Nz * j + k;

    return Data[n];
}

////////////////////////////////////////////////////////////////////////

void DataCube::assign(const DataCube& other) {

    clear();

    set_size(other.Nx, other.Ny, other.Nz);

    for (int n = 0; n < Nxyz; n++) {
        Data[n] = other.Data[n];
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::increment(void) {

    for (int n = 0; n < Nxyz; n++) {
        Data[n]++;
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::add_assign(const DataCube& other) {

    this->check_shape_equal(other);

    for (int n = 0; n < Nxyz; n++) {
        Data[n] += other.Data[n];
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::subtract_assign(const DataCube& other) {

    this->check_shape_equal(other);

    for (int n = 0; n < Nxyz; n++) {
        Data[n] -= other.Data[n];
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::multiply_assign(const DataCube& other) {

    this->check_shape_equal(other);

    for (int n = 0; n < Nxyz; n++) {
        Data[n] *= other.Data[n];
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::divide_assign(int denom) {

    for (int n = 0; n < Nxyz; n++) {
        Data[n] = Data[n] / denom;
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::divide_assign(const DataCube& other) {

    this->check_shape_equal(other);

    for (int n = 0; n < Nxyz; n++) {
        Data[n] /= other.Data[n];
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::min_assign(const DataCube& other) {

    this->check_shape_equal(other);

    for (int n = 0; n < Nxyz; n++) {
        Data[n] = min(Data[n], other.Data[n]);
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::max_assign(const DataCube& other) {

    this->check_shape_equal(other);

    for (int n = 0; n < Nxyz; n++) {
        Data[n] = max(Data[n], other.Data[n]);
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::square() {

    for (int n = 0; n < Nxyz; n++) {
        Data[n] = Data[n] * Data[n];
    }

    return;
}

////////////////////////////////////////////////////////////////////////

void DataCube::square_root() {

    for (int n = 0; n < Nxyz; n++) {
        Data[n] = sqrt(Data[n]);
    }

    return;
}

////////////////////////////////////////////////////////////////////////

double DataCube::operator()(int i, int j, int k) const {

    return get(i, j, k);
}

////////////////////////////////////////////////////////////////////////

DataCube& DataCube::operator=(const DataCube& other) {

    if (this == &other) return *this;

    assign(other);

    return *this;
}

////////////////////////////////////////////////////////////////////////

DataCube& DataCube::operator+=(const DataCube& other) {

    add_assign(other);

    return *this;
}

////////////////////////////////////////////////////////////////////////

DataCube& DataCube::operator-=(const DataCube& other) {

    subtract_assign(other);

    return *this;
}

////////////////////////////////////////////////////////////////////////

DataCube& DataCube::operator*=(const DataCube& other) {

    multiply_assign(other);

    return *this;
}

////////////////////////////////////////////////////////////////////////

DataCube& DataCube::operator/=(const DataCube& other) {

    divide_assign(other);

    return *this;
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
