#!/bin/bash
#
# Run SonarQube Source Code Analyzer on a specified revision of MET
#=======================================================================
#
# This run_sonarqube.sh script will check out the specified version
# of MET and run the SonarQube Source Code Analyzer on it.  First,
# go to the directory where you would like the scanning output written and
# then run:
#
#    git clone https://github.com/dtcenter/MET
#    MET/internal/scripts/sonarqube/run_sonarqube.sh name
#
# Usage: run_sonarqube.sh name
#    Test the specified branched version of MET:
#       run_sonarqube.sh {branch name}
#    Test the specified tagged version of MET:
#       run_sonarqube.sh {tag name}
#
#=======================================================================

# Constants
[ -z "$GIT_REPO_NAME" ] && GIT_REPO_NAME=MET
GIT_REPO="https://github.com/dtcenter/${GIT_REPO_NAME}"

function usage {
        echo
        echo "USAGE: $(basename $0) name"
        echo "   where \"name\" specifies a branch, tag, or hash."
        echo
}

# Check for arguments
if [[ $# -lt 1 ]]; then usage; exit; fi

# Check that SONARQUBE_WRAPPER_BIN is defined
if [ -z ${SONARQUBE_WRAPPER_BIN} ]; then
  SONAR_WRAPPER=$(which build-wrapper-linux-x86-64 2> /dev/null)
  if [ $? -ne 0 ]; then
    SONAR_WRAPPER=$(which build-wrapper 2> /dev/null)
    if [ $? -ne 0 ]; then
      echo "ERROR: SONARQUBE_WRAPPER_BIN must be set"
      exit 1
    fi
  fi
else
  SONAR_WRAPPER=${SONARQUBE_WRAPPER_BIN}/build-wrapper-linux-x86-64
fi
if [ ! -e ${SONAR_WRAPPER} ]; then
  echo "ERROR: ${SONAR_WRAPPER} does not exist"
  exit 1
fi

# Check that SONARQUBE_SCANNER_BIN is defined
SCANNER_NAME=sonar-scanner
if [ -z ${SONARQUBE_SCANNER_BIN} ]; then
  SONAR_SCANNER=$(which $SCANNER_NAME 2> /dev/null)
  if [ $? -ne 0 ]; then
    echo "ERROR: SONARQUBE_SCANNER_BIN must be set"
    exit 1
  fi
else
  SONAR_SCANNER=${SONARQUBE_SCANNER_BIN}/$SCANNER_NAME
fi
if [ ! -e $SONAR_SCANNER ]; then
  echo "ERROR: SONAR_SCANNER (${SONAR_SCANNER}) does not exist"
  exit 1
fi

if [ -z ${SONARQUBE_OUT_DIR} ]; then
  export SONARQUBE_OUT_DIR=bw-outputs
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


# Store the full path to the scripts directory
SCRIPT_DIR=`dirname $0`
if [[ ${0:0:1} != "/" ]]; then SCRIPT_DIR=$(pwd)/${SCRIPT_DIR}; fi 

# Clone repo into a sub-directory and checkout the requested version
REPO_DIR="MET-${1}"

if [ -e ${REPO_DIR} ]; then
  run_command "rm -rf ${REPO_DIR}"
fi
run_command "git clone ${GIT_REPO} ${REPO_DIR}"
run_command "cd ${REPO_DIR}"
run_command "git checkout ${1}"

# Do no manually set the CXX and F77 compilers.
# Let the configure script pick them.
# Otherwise, the SonarQube logic does not work.
export MET_DEVELOPMENT=true

# Run the configure script
run_command "./configure --prefix=`pwd` \
            --enable-grib2 \
            --enable-modis \
            --enable-mode_graphics \
            --enable-lidar2nc \
            --enable-python"

# Set the build id
#BUILD_ID="MET-${1}"

SONAR_PROPERTIES=sonar-project.properties

# Copy sonar-project.properties for Python code
[ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
[ -z "$SONAR_SERVER_URL" ] && SONAR_SERVER_URL="http://localhost:9000"
if [ -z "$SONAR_TOKEN_VALUE" ]; then
  echo "  == ERROR == SONAR_TOKEN_VALUE is not defined"
  exit 1
else
  sed -e "s|SONAR_TOKEN_VALUE|$SONAR_TOKEN_VALUE|" -e "s|SONAR_SERVER_URL|$SONAR_SERVER_URL|" $SCRIPT_DIR/python.sonar-project.properties > $SONAR_PROPERTIES

  # Run SonarQube scan for Python code
  run_command "$SONAR_SCANNER"

  # Copy sonar-project.properties for C/C++ code
  [ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
  sed -e "s|SONAR_TOKEN_VALUE|$SONAR_TOKEN_VALUE|" -e "s|SONAR_SERVER_URL|$SONAR_SERVER_URL|" $SCRIPT_DIR/$SONAR_PROPERTIES > $SONAR_PROPERTIES

  # Run SonarQube clean
  run_command "make clean"

  # Run SonarQube make
  run_command "$SONAR_WRAPPER --out-dir $SONARQUBE_OUT_DIR make"

  # Run SonarQube scan for C/C++ code
  run_command "$SONAR_SCANNER"

  [ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
fi

# Run SonarQube report generator to make a PDF file
#TODAY=`date +%Y%m%d`
