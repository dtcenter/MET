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
#    MET/internal/scripts/regression/test_nightly.sh name
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
EMAIL_LIST="johnhg@ucar.edu hsoh@ucar.edu jpresto@ucar.edu linden@ucar.edu mccabe@ucar.edu"
KEEP_DAYS=5

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

# Store the full path to the scripts directory
SCRIPT_DIR=`dirname $0`
if [[ ${0:0:1} != "/" ]]; then SCRIPT_DIR=$(pwd)/${SCRIPT_DIR}; fi 

# Define the development environment
ENV_FILE=${SCRIPT_DIR}/../environment/development.`hostname`
if [[ ! -e ${ENV_FILE} ]]; then
  echo "$0: ERROR -> Development environment file missing: ${ENV_FILE}"
  exit 1
fi
source ${ENV_FILE}

# Keep it open
umask 0002

# Delete old directories
find ${MET_PROJ_DIR}/MET_regression/${1} \
     -mtime +${KEEP_DAYS} -name "NB*" | \
     xargs rm -rf

# Create and switch to a run directory
TODAY=`date +%Y%m%d`
RUN_DIR=${MET_PROJ_DIR}/MET_regression/${1}/NB${TODAY}
if [[ -e ${RUN_DIR} ]]; then rm -rf ${RUN_DIR}; fi
mkdir -p ${RUN_DIR}
cd ${RUN_DIR}

# Create a logfile
LOGFILE=${RUN_DIR}/test_regression_${TODAY}.log
>${LOGFILE}

# Check that we have a script to run
if [[ ! -r ${SCRIPT_DIR}/test_regression.sh ]]; then
  echo "$0: ERROR -> ${SCRIPT_DIR}/test_regression.sh not found" > ${LOGFILE}
  exit 1
fi

# Run the regression test and check the return status
${SCRIPT_DIR}/test_regression.sh ${1}-ref ${1} >> ${LOGFILE} 2>&1
if [[ $? -ne 0 ]]; then
  echo "$0: ERROR -> the regression test failed." >> ${LOGFILE}
  echo -e "Nightly Build Log: `hostname`:${LOGFILE}\n\n`tail -10 ${LOGFILE}`" | \
  mail -s "MET Nightly Build Failed for ${1} in `basename ${RUN_DIR}` (autogen msg)" ${EMAIL_LIST}
  exit 1
fi

# Look for errors and/or warnings
N_ERR=`egrep "ERROR:"   ${LOGFILE} | wc -l`
N_WRN=`egrep "WARNING:" ${LOGFILE} | wc -l`
echo "$0: Found $N_WRN WARNINGS and $N_ERR ERRORS in regtest" >> ${LOGFILE}

# Check for non-zero errors
if [[ $N_ERR -gt 0 ]]; then
  echo "$0: ERROR -> grep found ERRORS in regtest" >> ${LOGFILE}
  echo "Nightly Build Log: `hostname`:${LOGFILE}" | \
  mail -s "MET Nightly Build Failed for ${1} in `basename ${RUN_DIR}` (autogen msg)" ${EMAIL_LIST}
  exit 1
fi

exit 0
