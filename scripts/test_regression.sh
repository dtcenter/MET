#!/bin/bash
#
# Compare unit test output from two versions of MET
#=======================================================================
#
# This test_regression.sh script will check out, build and run the unit
# tests on the two specified revisions of MET.  This is done by running
# the test_unit.sh script for each version.  Once the units tests have
# been run, the comp_dir utility is called to compare the output of the
# two sets of tests.  To run this script, use the following commands:
#
#    git clone https://github.com/NCAR/MET
#    MET/scripts/test_regression.sh version1 version2
#
# Usage: test_regression.sh version1 version2
#    where version1 and version2 are named branches, tags, or hashes.
#
# For example, test the met-8.1 release versus the develop branch:
#    test_regression.sh met-8.1 develop
#
#=======================================================================

function usage {
   echo
   echo "USAGE: test_regression.sh version1 version2"
   echo "   where \"version1\" and \"version2\" specify a branch, tag, or hash."
   echo
}

# Check for arguments
if [ $# -lt 2 ]; then usage; exit 1; fi

if [[ ${1} == ${2} ]]; then
  echo "ERROR: the two versions must be different!"
  exit 1
fi

# Sub-routine for running a command and checking return status
function run_command() {

  # Print the command being called
  echo "CALLING: $1"

  # Run the command and store the return status
  $1
  STATUS=$?

  # Check return status
  if [[ ${STATUS} -ne 0 ]]; then
     echo "ERROR: Command returned with non-zero status ($STATUS): $1"
     exit ${STATUS}
  fi

  return ${STATUS}
}

# Store the scripts location
SCRIPTS=`dirname $0`

# Run the unit test script for each version
PID_LIST=""
FAILURE=0
for VERSION in `echo "${1} ${2}"`; do

  LOG="$(pwd)/test_unit_${VERSION}.log"

  # Run the unit tests in the background
  UNIT_CMD="${SCRIPTS}/test_unit.sh ${VERSION}"
  echo "CALLING: ${UNIT_CMD}"
  echo "LOGGING: ${LOG}"
  $UNIT_CMD > $LOG 2>&1 &
  PID_LIST="${PID_LIST} $!"

  # Sleep for 5 minutes to stagger the unit test start times
  sleep 300
done

# Wait for unit tests to finish
for PID in $PID_LIST; do
   wait $PID || let "FAILURE=1"
done

# Check return status
if [ "$FAILURE" == "1" ]; then
   echo "ERROR: unit test failed";
   exit 1
fi

# Run the directory comparison tool on the output from the unit tests
export MET_BASE=$(pwd)/MET-${2}/met/share/met
export MET_TEST_BASE=$(pwd)/MET-${2}/test
run_command "${MET_TEST_BASE}/bin/comp_dir.sh MET-${1}/test_output MET-${2}/test_output"
