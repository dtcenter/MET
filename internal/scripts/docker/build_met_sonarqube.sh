#!/bin/bash
#
# Run SonarQube Source Code Analyzer within a docker container
#=======================================================================
#
# This build_met_sonarqube.sh script must be run from the top-level
# directory of the MET repository to be analyzed. It builds MET with the
# SonarQube Source Code Analyzer.
#
# Usage: internal/scripts/docker/build_met_sonarqube.sh
#
#=======================================================================

# Check that this is being run from the top-level MET directory
if [ ! -e internal/scripts/docker/build_met_sonarqube.sh ]; then
  echo "ERROR: ${0} must be run from the top-level MET directory"
  exit 1
fi

# Source the docker build environment
source ~/.bashrc
source internal/scripts/environment/development.docker 
source .github/jobs/bash_functions.sh

# Locate the wrapper
WRAPPER_NAME=build-wrapper-linux-x86-64
SONAR_WRAPPER=$(which $WRAPPER_NAME 2> /dev/null)

if [ ! -e $SONAR_WRAPPER ]; then
  echo "ERROR: $WRAPPER_NAME not found in the path"
  exit 1
else
  echo "SONAR_WRAPPER=$SONAR_WRAPPER"
fi

# Locate the scanner
SCANNER_NAME=sonar-scanner
SONAR_SCANNER=$(which $SCANNER_NAME 2> /dev/null)

if [ ! -e $SONAR_SCANNER ]; then
  echo "ERROR: $SCANNER_NAME not found in the path"
  exit 1
else
  echo "SONAR_SCANNER=$SONAR_SCANNER"
fi

# Set output directory name
if [ -z ${SONARQUBE_OUT_DIR} ]; then
  export SONARQUBE_OUT_DIR=bw-outputs
fi

# Store the full path to the scripts directory
SONAR_PROPERTIES_DIR=internal/scripts/sonarqube
SONAR_PROPERTIES=sonar-project.properties

# Copy sonar-project.properties for C/C++ code
[ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
sed -e "s|SONAR_TOKEN_VALUE|$SONAR_TOKEN|" -e "s|SONAR_SERVER_URL|$SONAR_HOST_URL|" $SONAR_PROPERTIES_DIR/$SONAR_PROPERTIES > $SONAR_PROPERTIES

# Run the configure script
time_command ./configure --prefix=`pwd` --enable-all

# Run make clean
time_command make clean

# Run SonarQube make
time_command $SONAR_WRAPPER --out-dir $SONARQUBE_OUT_DIR make

# Run SonarQube scan for C/C++ code
time_command $SONAR_SCANNER

[ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
