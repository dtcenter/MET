#!/bin/bash
# This script is intended to be run from a cron job nightly to
# check out the MET code tree and do a Fortify scan

# Configure run
SCRIPTS=`dirname $0`
EMAIL_LIST="johnhg@ucar.edu bullock@ucar.edu mccabe@ucar.edu"
TODAY=`date +%Y%m%d`
RUNDIR="/d3/projects/MET/MET_regression/NB${TODAY}/fortify_sca"
mkdir -p $RUNDIR
cd $RUNDIR
LOGFILE=${PWD}/run_fortify_sca_${TODAY}.log

# Variables required to build MET
export MET_DEVELOPMENT=true
export MET_NETCDF=/usr/local/netcdf
export MET_HDF5=/usr/local/hdf5
export MET_GSL=/d3/projects/MET/MET_releases/external_libs/gsl/gsl-1.13
export MET_GRIB2CINC=/d3/projects/MET/MET_releases/external_libs/g2clib/g2clib-1.4.0
export MET_GRIB2CLIB=/d3/projects/MET/MET_releases/external_libs/g2clib/g2clib-1.4.0
export MET_BUFRLIB=/d3/projects/MET/MET_releases/external_libs/bufrlib/bufrlib_gnu
export MET_HDF=/d3/projects/MET/MET_releases/external_libs/hdf/HDF4.2r3
export MET_HDFEOS=/d3/projects/MET/MET_releases/external_libs/hdf_eos/hdfeos
export MET_CAIROINC=/usr/include/cairo
export MET_CAIROLIB=/usr/lib
export MET_FREETYPEINC=/usr/include/freetype2
export MET_FREETYPELIB=/usr/lib

# Variables required for using Python numpy to create numpy array of data to be used by MET tools
export MET_PYTHON_CC="-I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7"
export MET_PYTHON_LD="-L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpython2.7 -lpthread -ldl  -lutil -lm"

# Variables required to run MET
export MET_FONT_DIR=/d3/projects/MET/MET_test_data/unit_test/fonts

# This is a cron script -- create the shell environment for this job
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:\
             /usr/bin/X11:/opt/bin:${MET_NETCDF}/bin"
export LD_LIBRARY_PATH=${MET_NETCDF}/lib:${MET_HDF5}/lib:/usr/local/lib:/usr/lib

# Run scan and check for bad return status
${SCRIPTS}/run_fortify_sca.sh trunk > ${LOGFILE}
if [[ $? -ne 0 ]]
then
  echo "$0: The nightly Fortify scan FAILED on ${TODAY}." >> ${LOGFILE}
  cat ${LOGFILE} | mail -s "MET Fortify Scan Failed (autogen msg)" ${EMAIL_LIST}
  exit 1
fi

exit 0
