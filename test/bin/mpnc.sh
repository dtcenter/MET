#!/bin/bash

# set the environment to use NetCDF4
export LD_LIBRARY_PATH=/usr/local/netcdf/lib:/usr/local/hdf5-1.8.5patch1/lib
export NETCDF=/usr/local/netcdf
export NETCDF_INCLUDE=/usr/local/netcdf/include
export NETCDF_LIB=/usr/local/netcdf/lib

# check for environment variables, use defaults if necessary
TEST_BASE=${TEST_BASE:-/d3/projects/MET/MET_test/test}
TEST_RSCRIPT=${TEST_RSCRIPT:-/usr/local/bin/Rscript}
MPNC=$TEST_BASE/R_test/mpnc.R

$TEST_RSCRIPT $MPNC $@ 2>&1

