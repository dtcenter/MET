#!/bin/bash
#
# Run unit tests on a specified revision of MET
#=======================================================================
#
# This test_unit.sh script will check out the specified revision
# of MET and the test module, build the development instances of
# the MET tools and run the unit tests on the built tools.  First,
# go to the directory where you would like the unit tests performed,
# then run:
#    svn co https://svn-met-dev.cgd.ucar.edu/build/scripts
#    scripts/test_unit.sh trunk [rev] | branch name [rev] | tag name [rev]
#
# Usage: test_unit.sh [-q] trunk [rev] | branch name [rev] | tag name [rev]
#    Test the latest version of MET repository trunk:
#       test_unit.sh trunk
#    Test the specified revision of MET:
#       test_unit.sh trunk [rev]
#    Test the specified branched version of MET:
#       test_unit.sh tag branch name [rev]
#    Test the specified tagged version of MET:
#       test_unit.sh tag name [rev]
#
#=======================================================================

# Constants
SVN_BASE="https://svn-met-dev.cgd.ucar.edu"
CXX_COMPILER=/usr/bin/g++
F77_COMPILER=/usr/bin/gfortran

# Check for quiet flag
QUIET=0
if [[ $1 == "-q" ]]; then QUIET=1; shift; REDIR=">/dev/null 2>&1"; fi

function usage {
   [ $QUIET -eq 1 ] && return
        echo
        echo "USAGE: test_unit.sh [-q] trunk [rev] | branch name [rev] | tag name [rev]"
        echo "   where"
        echo "                  -q specifies no text output, exit status indicates success"
        echo "         trunk [rev] specifies the repository trunk"
        echo "      tag name [rev] specifies a named tag, e.g. met-6.0"
        echo "   branch name [rev] specifies a named branch, e.g. met-6.0_bugfix"
        echo "               [rev] optionally specifies a revision number, e.g. met-6.0_bugfix 4160"
        echo
}

function log {
   [ $QUIET -eq 0 ] && echo $*
}

# Check for arguments
if [[ $# -lt 1 ]]; then usage; exit; fi

# Parse the input parameters
case $1 in
   trunk)
      MET_BASE_DIR="trunk/met"
      TEST_BASE_DIR="trunk/test"
      if [ $# -lt 2 ]; then
         REV=$(svn info $SVN_BASE | egrep '^Rev' | awk '{print $2}')
      else
         REV=$2
      fi
      NAME="rev${REV}"
      ;;
   tag)
      if [ $# -lt 2 ]; then
         log "ERROR: tag not specified";
         usage;
         exit 1;
      fi
      MET_BASE_DIR="tags/met/$2"
      TEST_BASE_DIR="tags/test/$2"
      if [ $# -lt 3 ]; then
         REV=$(svn info $SVN_BASE/tags/met/$2 | egrep 'Last Changed Rev' | awk '{print $4}')
      else
         REV=$3
      fi
      NAME="tag_$2_rev${REV}"
      ;;
   branch)
      if [ $# -lt 2 ]; then
         log "ERROR: branch not specified";
         usage;
         exit 1;
      fi
      MET_BASE_DIR="branches/met/$2"
      TEST_BASE_DIR="branches/test/$2"
      if [ $# -lt 3 ]; then
         REV=$(svn info $SVN_BASE/branches/met/$2 | egrep 'Last Changed Rev' | awk '{print $4}')
      else
         REV=$3
      fi
      NAME="branch_$2_rev${REV}"
      ;;
   *)
      log "ERROR: unrecognized input argument '$1'";
      usage;
      exit 1;
      ;;
esac

# Check out the requested version of MET
[ -e "met_$NAME" ] && eval "rm -rf met_$NAME"
SVN_MET="svn export -q -r ${REV} ${SVN_BASE}/${MET_BASE_DIR} met_$NAME"
log "checking out MET:  $SVN_MET"
eval "$SVN_MET" $REDIR
if [ $? -ne 0 ]; then log "ERROR: checkout of met_$NAME failed"; exit 1; fi

# Check out the requested version of the test module
[ -e "test_$NAME" ] && eval "rm -rf test_$NAME"
SVN_TEST="svn export -q -r ${REV} ${SVN_BASE}/${TEST_BASE_DIR} test_$NAME"
log "checking out test module:  $SVN_TEST"
eval "$SVN_TEST" $REDIR
if [ $? -ne 0 ]; then log "ERROR: checkout of test_$NAME failed"; exit 1; fi

# Build the MET instance
cd met_$NAME
log "building met_$NAME..."

# Create log file
LOG=../build_$NAME.log
rm -f $LOG

# Run bootstrap
eval "./bootstrap" >> $LOG
if [ $? -ne 0 ]; then log "ERROR: bootstrap of met_$NAME failed"; exit 1; fi

# Set the compilers to be used
export CXX=${CXX_COMPILER}
export F77=${F77_COMPILER}

# Run the configure script
eval "./configure --prefix=`pwd` \
            --enable-grib2 \
            --enable-modis \
            --enable-mode_graphics \
            --enable-lidar2nc \
            --enable-python > /dev/null" >> $LOG
if [ $? -ne 0 ]; then log "ERROR: configuration of met_$NAME failed"; exit 1; fi

# Compile and install the build
eval "make install" >> $LOG
if [ $? -ne 0 ]; then log "ERROR: build of met_$NAME failed"; exit 1; fi
cd ..

# Run the unit tests
log "running unit tests..."
export MET_BUILD_BASE=$(pwd)/met_$NAME
export MET_BASE=$MET_BUILD_BASE/share/met
export MET_TEST_BASE=$(pwd)/test_$NAME
export MET_TEST_OUTPUT=$(pwd)/test_output_$NAME
[ -e "test_output_$NAME" ] && eval "rm -rf test_output_$NAME"
test_$NAME/bin/unit_test.sh $REDIR
if [ $? -ne 0 ]; then log "ERROR: unit test of met_$NAME failed"; exit 1; fi

exit
