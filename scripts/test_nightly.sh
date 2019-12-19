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
#    git clone https://github.com/NCAR/MET
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

function usage {
  echo
  echo "USAGE: test_nightly.sh name"
  echo "   where \"name\" specifies a branch or tag for which a corresponding"
  echo "         \"name-ref\" branch or tag exists"
  echo
}

# Check for arguments
if [ $# -lt 1 ]; then usage; exit 1; fi

# Configure run for dakota
RUNDIR="/d3/projects/MET/MET_regression/${1}"
EMAIL_LIST="johnhg@ucar.edu bullock@ucar.edu hsoh@ucar.edu mccabe@ucar.edu fillmore@ucar.edu"
KEEP_STUFF_DURATION=5    #This is in days

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

# Exit codes
E_NOEXECSCRIPT=61
E_REGTEST_FAILED=62
E_FOUND_WARNING=63

# Keep it open
umask 0002

# Get to the run directory
mkdir -p ${RUNDIR}
cd ${RUNDIR}

# Clean the place up
find . -mtime +${KEEP_STUFF_DURATION} -name "NB*" | xargs rm -rf

# Internals - create a daily log file
today=`date +%Y%m%d`
LOGFILE=${PWD}/NB${today}.out
>${LOGFILE}

# make a little home - always start from scratch
if [[ -e NB${today} ]];  then rm -rf NB${today}; fi
mkdir NB${today}
cd NB${today}

# Check that we have a script to run
if [[ ! -r ${SCRIPTS}/test_regression.sh ]]
then
  echo "$0: FAILURE ${SCRIPTS}/test_regression.sh not found" > ${LOGFILE}
  exit $E_NOEXECSCRIPT
fi

# Run the regression test fail if non-zero status
${SCRIPTS}/test_regression.sh ${1}-ref ${1} >> ${LOGFILE} 2>&1
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
