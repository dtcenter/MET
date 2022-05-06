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
# The desired version of the code must have
# already been checked out prior to calling this
# script.
#
# Usage: MET_build <version_number>
#
#================================================

# MET_DEVELOPMENT must be set to build a release
MET_DEVELOPMENT=true

# Store the number of arguments
NARGS=$#

# Get the current date
TODAY=`date +%Y%m%d`

# Get the current revision hash
CUR_REV=`git rev-parse --short HEAD`

# Check for 0 or 1 arguments
if [ ${NARGS} -eq 0 ]; then
  VERSION="${CUR_REV}"
elif [ ${NARGS} -eq 1 ]; then
  VERSION="${1}"
else
  echo
  echo "USAGE: MET_build <version_number>"
  echo
  exit 1
fi

# Set the MET build version by stripping off leading "v" from "vX.Y"
export MET_BUILD_VERSION=`echo $VERSION | sed 's/v//g'`
echo "Building MET version '${MET_BUILD_VERSION}'"

# Copy the current met directory
CUR_PATH=`pwd`
CUR_DIR=`basename $CUR_PATH`
cd ..
cp -r ${CUR_DIR} MET-${MET_BUILD_VERSION}
cd MET-${MET_BUILD_VERSION}

# Cleanup
rm -f `find ./ -name ".gitignore"`

# Run the bootstrap program to prepare for running configure
echo "Running 'bootstrap' to prepare for running configure"
./bootstrap > /dev/null

# Patch vx_config Makefile.in by removing lex/yacc
cat src/basic/vx_config/Makefile.in | \
    sed 's/config.tab.yy//g'        | \
    sed 's/config_scanner.ll//g'    > \
    src/basic/vx_config/Makefile.new
mv  src/basic/vx_config/Makefile.new \
    src/basic/vx_config/Makefile.in

# Patch vx_color Makefile.in by removing lex/yacc
cat src/libcode/vx_color/Makefile.in | \
    sed 's/color_parser_yacc.yy//g'  | \
    sed 's/color_scanner.ll//g'      > \
    src/libcode/vx_color/Makefile.new
mv  src/libcode/vx_color/Makefile.new \
    src/libcode/vx_color/Makefile.in

# Figure out where to install MET
MET_INSTALL_DIR=`pwd`

# Now run configure to generate the Makefiles so we can do an
# initial build.
echo "Running configure to create the Makefiles"
./configure --prefix=$MET_INSTALL_DIR \
            --enable-grib2 \
            --enable-mode_graphics \
            --enable-modis \
            --enable-lidar2nc \
            --enable-python  > /dev/null

# Make the distribution file.  This will automatically create
# any needed distribution files like those created by yacc/lex
# and enum_to_string.
echo "Creating the distribution file"
make dist > /dev/null

# Construct the desired name for the tar file.  autoconf
# creates the tar file using it's standard naming convention.
TAR_FILE="met-${MET_BUILD_VERSION}.${TODAY}.tar.gz"

echo "Moving tar file to new name: '${TAR_FILE}'"
mv met-* ../${TAR_FILE}

