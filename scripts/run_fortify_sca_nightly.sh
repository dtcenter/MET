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
#    git clone https://github.com/dtcenter/MET
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
export MET_DST=/usr/local
export MET_NETCDF=${MET_DST}/netcdf-4.7.0/gcc-6.3.0
export MET_HDF5=${MET_DST}/hdf5-1.8.20
export MET_HDFINC=${MET_DST}/hdf4-4.2.13/include/hdf
export MET_HDFLIB=${MET_DST}/hdf4-4.2.13/lib
export MET_HDFEOS=${MET_DST}/hdf-eos2-19v1
export MET_BUFR=${MET_DST}
export MET_GRIB2C=${MET_DST}
export MET_GSLINC=/usr/include/gsl
export MET_GSLLIB=/usr/lib
export MET_CAIROINC=/usr/include/cairo
export MET_CAIROLIB=/usr/lib
export MET_FREETYPEINC=/usr/include/freetype2
export MET_FREETYPELIB=/usr/lib

# For Python 3 in met-9.0
export MET_PYTHON="/var/autofs/mnt/linux-amd64/debian/stretch/local/met-python3"
export MET_PYTHON_CC="-I${MET_PYTHON}/include/python3.7m"
export MET_PYTHON_LD="-L${MET_PYTHON}/lib -lpython3.7m -lcrypt -lpthread -ldl -lutil -lm"

# -D__64BIT__ is required because we've compiled libgrib2c.a with that flag
export CFLAGS="-DUNDERSCORE -fPIC -D__64BIT__"
export CXXFLAGS=${CFLAGS}

# Set LDFLAGS to include -rpath settings when compiling MET
export LDFLAGS="-Wl,--disable-new-dtags"
export LDFLAGS="${LDFLAGS} -Wl,-rpath,${MET_DST}/lib:${MET_HDFEOS}/lib:${MET_NETCDF}/lib:${MET_DST}/zlib-1.2.11/lib:${MET_DST}/szip-2.1.1/lib"
export LDFLAGS="${LDFLAGS} -Wl,-rpath,${MET_HDFLIB}:${MET_HDF5}/lib:${MET_PYTHON}/lib"

# Variables required to run MET
export MET_FONT_DIR=/d3/projects/MET/MET_test_data/unit_test/fonts

# This is a cron script -- create the shell environment for this job
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:\
             /usr/bin/X11:/opt/bin:${MET_NETCDF}/bin"             
             
# Run scan and check for bad return status
${SCRIPTS}/run_fortify_sca.sh develop > ${LOGFILE}
if [[ $? -ne 0 ]]
then
  echo "$0: The nightly Fortify scan FAILED on ${TODAY}." >> ${LOGFILE}
  cat ${LOGFILE} | mail -s "MET Fortify Scan Failed for ${1} (autogen msg)" ${EMAIL_LIST}
  exit 1
fi
