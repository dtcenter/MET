#!/bin/bash
#
# Run SonarQube Source Code Analyzer within a Docker container
#=======================================================================
#
# This build_met_sonarqube.sh script must be run from the top-level
# directory of the MET repository to be analyzed. It runs SonarQube to
# scan both the Python and C/C++ MET source code.
#
# Usage: internal/scripts/docker/build_met_sonarqube.sh
#
# Required Enviornment Variables:
#   SONAR_HOST_URL
#   SONAR_TOKEN
#   MET_GIT_NAME
#   SONAR_REFERENCE_BRANCH
#
#=======================================================================

# Check that this is being run from the top-level MET directory
if [ ! -e internal/scripts/docker/build_met_sonarqube.sh ]; then
  echo "ERROR: ${0} -> must be run from the top-level MET directory"
  exit 1
fi

echo "Running script to scan MET with SonarQube in Docker"

# Source the docker build environment
source ~/.bashrc
source internal/scripts/environment/development.docker 
source .github/jobs/bash_functions.sh

# Check required environment variables
if [ -z ${SONAR_HOST_URL+x} ]; then
  echo "ERROR: ${0} -> \$SONAR_HOST_URL not defined!"
  exit 1
fi
if [ -z ${SONAR_TOKEN+x} ]; then
  echo "ERROR: ${0} -> \$SONAR_TOKEN not defined!"
  exit 1
fi
if [ -z ${MET_GIT_NAME+x} ]; then
  echo "ERROR: ${0} -> \$MET_GIT_NAME not defined!"
  exit 1
fi
if [ -z ${SONAR_REFERENCE_BRANCH+x} ]; then
  echo "ERROR: ${0} -> \$SONAR_REFERENCE_BRANCH not defined!"
  exit 1
fi

# Check whether MET_CONFIG_OPTS is defined
if [ -z ${MET_CONFIG_OPTS+x} ]; then
  MET_CONFIG_OPTS='--enable-all'
  echo "Setting MET_CONFIG_OPTS=${MET_CONFIG_OPTS} to scan all available options."
fi

# Locate the wrapper
WRAPPER_NAME=build-wrapper-linux-x86-64
SONAR_WRAPPER=$(which $WRAPPER_NAME 2> /dev/null)

if [ ! -e $SONAR_WRAPPER ]; then
  echo "ERROR: ${0} -> $WRAPPER_NAME not found in the path"
  exit 1
else
  echo "SONAR_WRAPPER=$SONAR_WRAPPER"
fi

# Locate the scanner
SCANNER_NAME=sonar-scanner
SONAR_SCANNER=$(which $SCANNER_NAME 2> /dev/null)

if [ ! -e $SONAR_SCANNER ]; then
  echo "ERROR: ${0} -> $SCANNER_NAME not found in the path"
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

# Copy sonar-project.properties for Python code
[ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
sed -e "s|SONAR_TOKEN|$SONAR_TOKEN|" \
    -e "s|SONAR_HOST_URL|$SONAR_HOST_URL|" \
    -e "s|SONAR_PROJECT_KEY|MET-GHA-Python|" \
    -e "s|SONAR_PROJECT_NAME|MET GHA Python|" \
    -e "s|SONAR_BRANCH_NAME|$MET_GIT_NAME|" \
    -e "s|SONAR_REFERENCE_BRANCH|$SONAR_REFERENCE_BRANCH|" \
    $SONAR_PROPERTIES_DIR/python.sonar-project.properties > $SONAR_PROPERTIES

# Run SonarQube scan for Python code
time_command $SONAR_SCANNER

# Copy the Python scan report-task.txt file
mkdir -p /met/.scannerwork
cp /met/MET-*/.scannerwork/report-task.txt /met/.scannerwork/python-report-task.txt

# Copy sonar-project.properties for C/C++ code
[ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
sed -e "s|SONAR_TOKEN|$SONAR_TOKEN|" \
    -e "s|SONAR_HOST_URL|$SONAR_HOST_URL|" \
    -e "s|SONAR_PROJECT_KEY|MET-GHA-CXX|" \
    -e "s|SONAR_PROJECT_NAME|MET GHA CXX|" \
    -e "s|SONAR_BRANCH_NAME|$MET_GIT_NAME|" \
    -e "s|SONAR_REFERENCE_BRANCH|$SONAR_REFERENCE_BRANCH|" \
    $SONAR_PROPERTIES_DIR/sonar-project.properties > $SONAR_PROPERTIES

# Run the configure script
time_command ./configure \
  BUFRLIB_NAME=${BUFRLIB_NAME} \
  GRIB2CLIB_NAME=${GRIB2CLIB_NAME} \
  ${MET_CONFIG_OPTS} \
  CPPFLAGS="-I/usr/local/include -I/usr/local/include/freetype2 -I/usr/local/include/cairo" \
  LIBS="-ltirpc"

# Run make clean
time_command make clean

# Run SonarQube make
time_command $SONAR_WRAPPER --out-dir $SONARQUBE_OUT_DIR make

# Run SonarQube scan for C/C++ code
time_command $SONAR_SCANNER

# Copy the C/C++ scan report-task.txt file
mkdir -p /met/.scannerwork
cp /met/MET-*/.scannerwork/report-task.txt /met/.scannerwork/cxx-report-task.txt

[ -e $SONAR_PROPERTIES ] && rm $SONAR_PROPERTIES
