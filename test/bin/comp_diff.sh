#!/bin/bash

export LDFLAGS=-Wl,-rpath,/usr/local/hdf5-1.8.5patch1/lib:/usr/local/netcdf/lib
export LD_LIBRARY_PATH=/usr/local/netcdf/lib:/usr/local/hdf5-1.8.5patch1/lib
export NETCDF=/usr/local/netcdf
export NETCDF_INCLUDE=/usr/local/netcdf/include
export NETCDF_LIB=/usr/local/netcdf/lib

# check for environment variables, use defaults if necessary
MET_TEST_BASE=${MET_TEST_BASE:-/d3/projects/MET/MET_development/svn-met-dev.cgd.ucar.edu/trunk/test}
MET_TEST_RSCRIPT=${MET_TEST_RSCRIPT:-/usr/local/bin/Rscript}
COMP_SCRIPT=$MET_TEST_BASE/R_test/comp_diff.R

$MET_TEST_RSCRIPT $COMP_SCRIPT $@ 2>&1

