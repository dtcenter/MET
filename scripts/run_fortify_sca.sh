#!/bin/bash
#
# Run Fortify Source Code Analyzer on a specified revision of MET
#=======================================================================
#
# This run_fortify_sca.sh script will check out the specified version
# of MET and run the Fortify Source Code Analyzer on it.  First,
# go to the directory where you would like the SCA output written and
# then run:
#
#    git clone https://github.com/NCAR/MET
#    MET/scripts/run_fortify_sca.sh name
#
# Usage: run_fortify_sca.sh name
#    Test the specified branched version of MET:
#       run_fortify_sca.sh master_v8.1
#    Test the specified tagged version of MET:
#       run_fortify_sca.sh met-8.1
#
#=======================================================================

# Constants
GIT_REPO="https://github.com/NCAR/MET"
FORTIFY_BIN=/d3/projects/Fortify/Fortify_SCA_and_Apps_18.10/bin

function usage {
        echo
        echo "USAGE: run_fortify_sca.sh name"
        echo "   where \"name\" specifies a branch, tag, or hash."
        echo
}

# Check for arguments
if [[ $# -lt 1 ]]; then usage; exit; fi

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

# Build the MET instance
run_command "cd met"

# Run bootstrap
run_command "./bootstrap"

# Do no manually set the CXX and F77 compilers.
# Let the configure script pick them.
# Otherwise, the Fortify logic does not work.
export MET_DEVELOPMENT=true

# Run the configure script
run_command "./configure --prefix=`pwd` \
            --enable-grib2 \
            --enable-modis \
            --enable-mode_graphics \
            --enable-lidar2nc \
            --enable-python"

# Set the build id
BUILD_ID="MET-${1}"

# Run Fortify SCA clean
run_command "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -clean"

# Run Fortify SCA make
run_command "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -logfile translate_${BUILD_ID}.echo -debug -verbose make"

# Run Fortify SCA scan
run_command "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -scan -f ${BUILD_ID}.fpr"

# Run Fortify to make an MBS file
run_command "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -export-build-session ${BUILD_ID}.mbs"

# Run Fortify report generator to make a PDF file
TODAY=`date +%Y%m%d`
run_command "${FORTIFY_BIN}/ReportGenerator -format pdf -f ${BUILD_ID}_${TODAY}_rev${REV}.pdf -source ${BUILD_ID}.fpr"
