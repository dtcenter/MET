#!/bin/bash
#
# Build a release for the MET
# (Model Evaluation Tools)
#================================================
#
# The MET_build script will build a release
# of the MET and will tar up the source code.
# This script is called by the
# MET_checkout_and_build.sh script.
#
# The desired version of the code must have alredy
# been checked out prior to calling this script.
#
# Usage: MET_build <version_number>
#
#================================================

# MET_DEVELOPMENT must be set to build a release
MET_DEVELOPMENT=true

# Store the number of arguments
NARGS=$#

# Get the current date
DATE=`date +%Y%m%d`

# Get the current revision hash
CUR_REV=`git rev-parse --short HEAD`

# Check for a met sub-directory
if [[ ! -e "met" ]]; then
  echo
  echo "ERROR: no \"met\" subdirectory found!"
  echo
  exit 1
fi

# Check for 0 or 1 arguments
if [[ ${NARGS} -eq 0 ]]; then
  VERSION="met-${CUR_REV}"
elif [[ ${NARGS} -eq 1 ]]; then
  VERSION=$1
else
  echo
  echo "USAGE: MET_build <version_number>"
  echo
  exit 1
fi

# Copy the current met directory
cp -r met ${VERSION}
cd ${VERSION}

# Set the MET build version for bootstrap by stripping off leading "met-"
export MET_BUILD_VERSION=`echo $VERSION | sed 's/met-//g'`
echo "Building MET version '${MET_BUILD_VERSION}'"

# Run 'make gen_sources' to generate source code
echo "Running 'make gen_sources' to create the generated source files."
make gen_sources > /dev/null

# Move Makefile_release over to Makefile, skipping the top-level one
echo "Renaming 'Makefile_release' to 'Makefile'."
for FILE in `find ./ -mindepth 2 -name Makefile_release`; do
  NEW=`echo ${FILE} | sed 's/_release//g'`
  mv ${FILE} ${NEW}
done

# Run 'make clean' to clean out partial build
echo "Running 'make clean' to clean out partial build."
make clean > /dev/null

# Rename top-level Makefile
echo "Renaming top-level 'Makefile_release' to 'Makefile'."
mv Makefile_release Makefile

# Remove top-level user_defs_dev.mk
echo "Removing top-level 'user_defs_dev.mk'."
rm -f user_defs_dev.mk

# Remove the development utilities
echo "Removing 'src/tools/dev_utils'."
rm -rf src/tools/dev_utils

# Remove the enum_to_string library
echo "Removing 'src/basic/enum_to_string'."
rm -rf src/basic/enum_to_string

# Remove any source code for test programs named 'test_*.cc'
echo "Removing source code for test programs named 'test_*.cc'."
rm -rf `find ./ -name "test_*.cc"`

# Remove any .project or .cproject files before building
echo "Removing any instances of '.*project'."
rm -rf `find ./ -name ".*project"`

# Remove any .svn directories before building
echo "Removing any instances of '.git'."
rm -rf `find ./ -name '.git'`

# Remove the top-level build directory
echo "Removing top-level 'build' directory."
rm -rf build

# Go back up one level
cd ..

# Tar up the newly built MET distribution
TAR_FILE="${VERSION}.${DATE}.tar.gz"
echo "Creating tar file '${TAR_FILE}'"
tar -czf ${TAR_FILE} ${TOP}

# Construct the desired name for the tar file.  autoconf
# creates the tar file using it's standard naming convention.
TAR_FILE="${VERSION}.${DATE}.tar.gz"

echo "Copying tar file to new name: '${TAR_FILE}'"
mv met-* ../${TAR_FILE}

