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
   VERSION="met-${CUR_REV}"
elif [ ${NARGS} -eq 1 ]; then
   VERSION="met-${1}"
else
   echo
   echo "USAGE: MET_build <version_number>"
   echo
   exit 1
fi

# Move Makefile_release over to Makefile
for FILE in `find met -name Makefile_release`; do
  NEW=`echo ${FILE} | sed 's/_release//g'`
  mv ${FILE} ${NEW}
done

# Copy default config files into data/config
for FILE in `find met -name "*Config_default"`; do
  cp ${FILE} met/data/config/.
done

# Cleanup
rm -rf met/build

# Tar up the newly built MET distribution
mv met ${VERSION}
tar -cvzf ${VERSION}.${DATE}.tar.gz ${VERSION}
