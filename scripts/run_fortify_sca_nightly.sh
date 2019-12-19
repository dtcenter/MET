#!/bin/bash
#
# Run nightly Fortify scan
#=======================================================================
#
# This run_fortify_sca_nightly.sh script calls the run_fortify_sca.sh
# script.  It is intented to be run nightly through cron.  Output
# should be directed to the LOGFILE, per cron convention.  To run this
# script, use the following commands:
#
#    git clone https://github.com/NCAR/MET
#    MET/scripts/run_fortify_sca_nightly.sh name
#
# Usage: run_fortify_sca_nightly.sh name
#    where "name" specifies a branch, tag, or hash
#
# For example, scan the develop branch:
#    run_fortify_sca_nightyl.sh develop
#
#=======================================================================

function usage {
  echo
  echo "USAGE: run_fortify_sca_nightly.sh name"
  echo "   where \"name\" specifies a branch, tag, or hash."
  echo
}

# Check for arguments
if [ $# -lt 1 ]; then usage; exit 1; fi

# Configure run for dakota
TODAY=`date +%Y%m%d`
RUNDIR="/d3/projects/MET/MET_regression/${1}/NB${TODAY}/fortify_sca"
mkdir -p $RUNDIR
cd $RUNDIR
LOGFILE=${PWD}/run_fortify_sca_${TODAY}.log
EMAIL_LIST="johnhg@ucar.edu bullock@ucar.edu mccabe@ucar.edu"

# Store the scripts location
SCRIPTS=`dirname $0`

# Variables required to build MET
export MET_DEVELOPMENT=true
export MET_NETCDF=/usr/local/netcdf
export MET_HDF5=/usr/local/hdf5
export MET_HDFINC=/usr/local/hdf4/include/hdf
export MET_HDFLIB=/usr/local/hdf4/lib
export MET_HDFEOS=/usr/local/hdfeos
export MET_BUFR=/usr/local
export MET_GSL=/d3/projects/MET/MET_releases/external_libs/gnu_6.3.0
export MET_GRIB2C=/d3/projects/MET/MET_releases/external_libs/gnu_6.3.0
export MET_CAIROINC=/usr/include/cairo
export MET_CAIROLIB=/usr/lib
export MET_FREETYPEINC=/usr/include/freetype2
export MET_FREETYPELIB=/usr/lib
export MET_PYTHON_CC="-I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7"
export MET_PYTHON_LD="-L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpython2.7 -lpthread -ldl  -lutil -lm"

# Variables required to run MET
export MET_FONT_DIR=/d3/projects/MET/MET_test_data/unit_test/fonts

# This is a cron script -- create the shell environment for this job
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:\
             /usr/bin/X11:/opt/bin:${MET_NETCDF}/bin"
export LD_LIBRARY_PATH=${MET_NETCDF}/lib:${MET_HDF5}/lib:/usr/local/lib:/usr/lib

# Run scan and check for bad return status
${SCRIPTS}/run_fortify_sca.sh develop > ${LOGFILE}
if [[ $? -ne 0 ]]
then
  echo "$0: The nightly Fortify scan FAILED on ${TODAY}." >> ${LOGFILE}
  cat ${LOGFILE} | mail -s "MET Fortify Scan Failed for ${1} (autogen msg)" ${EMAIL_LIST}
  exit 1
fi
