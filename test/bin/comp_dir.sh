#!/bin/bash

export LDFLAGS=-Wl,-rpath,/usr/local/hdf5-1.8.5patch1/lib:/usr/local/netcdf/lib
export LD_LIBRARY_PATH=/usr/local/netcdf/lib:/usr/local/hdf5-1.8.5patch1/lib
export NETCDF=/usr/local/netcdf
export NETCDF_INCLUDE=/usr/local/netcdf/include
export NETCDF_LIB=/usr/local/netcdf/lib

RSCRIPT=/usr/local/bin/Rscript
COMP_SCRIPT=/d3/projects/MET/MET_test/test/R_test/comp_dir.R

$RSCRIPT $COMP_SCRIPT $@

