#!/bin/bash
#
# Run nightly regression tests
#=======================================================================
#
# This test_nightly.sh script calls the test_regression.sh script to
# compare two different versions of MET.  It is intented to be run
# nightly through cron.  Output should be directed to the LOGFILE, per
# cron convention.  To run this script, use the following commands:
#
#    git clone https://github.com/dtcenter/MET
#    MET/scripts/test_nightly.sh name
#
# Usage: test_nightly.sh name
#    where "name" specifies a branch or tag for which a corresponding
#          "name-ref" branch or tag exists
#
# For example, test the develop branch:
#    test_nightly.sh develop
#
#=======================================================================

# Constants
#EMAIL_LIST="johnhg@ucar.edu bullock@ucar.edu hsoh@ucar.edu mccabe@ucar.edu fillmore@ucar.edu"
EMAIL_LIST="johnhg@ucar.edu"
KEEP_STUFF_DURATION=5    #This is in days

# Store the script directory
SCRIPT_DIR=`dirname $0`

# Usage statement
function usage {
  echo
  echo "USAGE: test_nightly.sh name"
  echo "   where \"name\" specifies a branch or tag for which a corresponding"
  echo "         \"name-ref\" branch or tag exists"
  echo
  exit 1
}

# Check for arguments
if [ $# -lt 1 ]; then usage; fi

# Set base directory based on the machine name
if [ `uname -a | egrep -i dakota | wc -l` -gt 0 ]; then
  echo "Running on dakota..."
  MET_DIR=/d3/projects/MET
elif [ `uname -a | egrep -i kiowa | wc -l` -gt 0 ]; then
  echo "Running on kiowa..."
  MET_DIR=/d1/projects/MET
else
  echo "ERROR: Unsupported machine name!"
  uname -a
  exit 1
fi

# Create and switch to the run directory
mkdir -p ${MET_DIR}/MET_regression/${1}
cd ${MET_DIR}/MET_regression/${1}

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
export MET_FONT_DIR=${MET_DIR}/MET_test_data/unit_test/fonts

# This is a cron script -- create the shell environment for this job
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:\
             /usr/bin/X11:/opt/bin:${MET_NETCDF}/bin"

# Exit codes
E_NOEXECSCRIPT=61
E_REGTEST_FAILED=62
E_FOUND_WARNING=63

# Keep it open
umask 0002

# Clean the place up
find . -mtime +${KEEP_STUFF_DURATION} -name "NB*" | xargs rm -rf

# Internals - create a daily log file
today=`date +%Y%m%d`
LOGFILE=${PWD}/NB${today}.out
>${LOGFILE}

# Create a daily directory
if [[ -e NB${today} ]];  then rm -rf NB${today}; fi
mkdir NB${today}
cd NB${today}

# Check that we have a script to run
if [[ ! -r ${SCRIPT_DIR}/test_regression.sh ]]
then
  echo "$0: FAILURE ${SCRIPT_DIR}/test_regression.sh not found" > ${LOGFILE}
  exit $E_NOEXECSCRIPT
fi

# Run the regression test fail if non-zero status
${SCRIPT_DIR}/test_regression.sh ${1}-ref ${1} >> ${LOGFILE} 2>&1
if [[ $? -ne 0 ]]
then
  echo "$0: FAILURE the regression test failed." >> ${LOGFILE}
  echo -e "Nightly Build Log: `hostname`:${LOGFILE}\n\n`tail -10 ${LOGFILE}`" | \
  mail -s "MET Nightly Build Failed for ${1} (autogen msg)" ${EMAIL_LIST}
  exit $E_REGTEST_FAILED
fi

# Look for errors and/or warnings
N_ERR=`egrep "ERROR:"   ${LOGFILE} | wc -l`
N_WRN=`egrep "WARNING:" ${LOGFILE} | wc -l`
echo "$0: Found $N_WRN WARNINGS and $N_ERR ERRORS in regtest" >> ${LOGFILE}

if [[ $N_ERR -gt 0 ]]      # check for errors
then
  echo "$0: FAILURE grep found ERRORS in regtest" >> ${LOGFILE}
  echo "Nightly Build Log: `hostname`:${LOGFILE}" | \
  mail -s "MET Nightly Build Failed for ${1} (autogen msg)" ${EMAIL_LIST}
  exit ${E_FOUNDWARNING}
fi

exit 0
