#!/bin/sh
#
# Build a release for the MET
# (Model Evaluation Tools)
#================================================
#
# The MET_build script will build a release
# of the MET and will tar up the source code.
# This script is called by the MET_checkout_and_build
# script.
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
if [ ${NARGS} -eq 0 ]; then
   VERSION="MET_${DATE}_rev${CUR_REV}"
   mv met ${VERSION}
elif [ ${NARGS} -eq 1 ]; then
   VERSION=$1
else
   echo "USAGE: MET_build <version_number>"
   exit
fi

# Set the top-level MET directory
TOP=${VERSION}

# Move over the met_data
for DIR in `ls met_data`; do
   mv met_data/${DIR}/* ${TOP}/data/${DIR}/.
done
rm -rf met_data

# Move over the met_doc
mv met_doc/*.pdf ${TOP}/doc/.
rm -rf met_doc

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
rm -rf `find ./ -name '.git'`

# Tar up the newly built MET distribution
tar -cvzf ${VERSION}.${DATE}.tar.gz ${TOP}
