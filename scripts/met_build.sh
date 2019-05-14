#! /bin/sh
#
# Build a release for the MET
# (Model Evaluation Tools)
#================================================
#
# The MET_build script will build a release
# of the MET and will tar up the source code.
# This script is called by the MET_checkout_and_build
# script and should be done so from the top-level
# MET source code directory.
#
# The desired version of the code must have alredy
# been checked out prior to calling this script.
#
# Usage: MET_build <version_number>
#
#================================================

# Store the number of arguments
NARGS=$#

# Get the current date
DATE=`date +%Y%m%d`

# Get the current revision hash
CUR_REV=`git rev-parse --short HEAD`

# Check for 0 or 1 argument
if [[ ${NARGS} -eq 0 ]]; then
   VERSION="MET_rev${CUR_REV}"
elif [[ ${NARGS} -eq 1 ]]; then
   VERSION=$1
else
   echo "USAGE: MET_build <version_number>"
   exit
fi

# Go to the top-level MET directory
MET_PATH=`echo $0 | sed 's/build\/met_build.sh/./'`
cd ${MET_PATH}

# Make a new directory and copy the current
# directory into it
mkdir   ../${VERSION}
cp -r * ../${VERSION}
cd      ..

# Set the top-level MET directory
TOP=${VERSION}

# Move Makefile_release over to Makefile
for FILE in `find ${TOP} -name Makefile_release`; do
  NEW=`echo ${FILE} | sed 's/_release//g'`
  mv ${FILE} ${NEW}
done

# Copy default config files into data/config
for FILE in `find ${TOP} -name "*Config_default"`; do
  cp ${FILE} ${TOP}/data/config/.
done

# Cleanup
rm -rf ${TOP}/build

# Remove any .git directories before building
rm -rf `find ${TOP} -name '.git'`

# Tar up the newly built MET distribution
tar -cvzf ${VERSION}.${DATE}.tar.gz ${TOP}
