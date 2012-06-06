#!/bin/bash

# set the environment to use NetCDF4
export LD_LIBRARY_PATH=/usr/local/netcdf/lib:/usr/local/hdf5-1.8.5patch1/lib
export NETCDF=/usr/local/netcdf
export NETCDF_INCLUDE=/usr/local/netcdf/include
export NETCDF_LIB=/usr/local/netcdf/lib

# check for environment variables, use defaults if necessary
MET_TEST_BASE=${MET_TEST_BASE:-/d3/projects/MET/MET_development/svn-met-dev.cgd.ucar.edu/trunk/test}
MET_TEST_RSCRIPT=${MET_TEST_RSCRIPT:-/usr/local/bin/Rscript}
MPNC=$MET_TEST_BASE/R_test/mpnc.R

$MET_TEST_RSCRIPT $MPNC $@ 2>&1

