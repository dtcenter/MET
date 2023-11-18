#!/bin/bash
#
# Run unit tests on a specified revision of MET
#=======================================================================
#
# This test_unit.sh script will check out the specified revision of MET,
# compile the code, and run the unit tests.
#
# Usage: test_unit.sh name
#    Test the specified branched version of MET:
#       test_unit.sh {branch name}
#    Test the specified tagged version of MET:
#       test_unit.sh {tag name}
#
#=======================================================================

# Constants
GIT_REPO="https://github.com/dtcenter/MET"
CXX_COMPILER=/usr/bin/g++
F77_COMPILER=/usr/bin/gfortran

function usage {
  echo
  echo "USAGE: test_unit.sh name"
  echo "   where \"name\" specifies a branch, tag, or hash."
  echo
}

# Check for arguments
if [ $# -lt 1 ]; then usage; exit 1; fi

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

# Clone repo into a sub-directory and checkout the requested version
REPO_DIR="MET-${1}"
if [ -e ${REPO_DIR} ]; then
  run_command "rm -rf ${REPO_DIR}"
fi
run_command "git clone ${GIT_REPO} ${REPO_DIR}"
run_command "cd ${REPO_DIR}"
run_command "git checkout ${1}"

# Set the compilers to be used
export CXX=${CXX_COMPILER}
export F77=${F77_COMPILER}

# Run the configure script
run_command "./configure --prefix=`pwd` \
            --enable-grib2 \
            --enable-modis \
            --enable-mode_graphics \
            --enable-lidar2nc \
            --enable-python \
            --enable-ugrid"

# Compile and install the build
run_command "make install"

# Check that MET_TEST_INPUT is defined
if [ -z ${MET_TEST_INPUT+x} ]; then
  echo "ERROR: ${MET_TEST_INPUT} must be defined!"
  exit 1
fi

# Run the unit tests
export MET_BUILD_BASE=$(pwd)
export MET_BASE=$MET_BUILD_BASE/share/met
export MET_TEST_BASE=$(pwd)/internal/test_unit
export MET_TEST_OUTPUT=$(pwd)/test_output
export MET_TMP_DIR=$(pwd)/internal/test_unit/tmp
run_command "mkdir -p ${MET_TMP_DIR}"
run_command "internal/test_unit/bin/unit_test.sh"
