#!/bin/bash
#
# Compare unit test output from two versions of MET
#=======================================================================
#
# This test_regression.sh script will check out, build and run the unit
# tests on the two specified revisions of MET and the test module.  This
# process is performed by calling the test_unit.sh script for each
# version.  Once the units tests have been run, the comp_dir utility is
# called to compare the output of the two sets of tests.  To run this
# script, use the following commands:
#
#    svn co https://svn-met-dev.cgd.ucar.edu/build/scripts
#    scripts/test_regression.sh version1 version2
#
# Usage: test_regression.sh version1 version2
#    where version1 and version2 each have the format
#       trunk [rev] | branch name [rev] | tag name [rev]
#
# For example, to test the tip of the trunk versus the 6.0 release:
#    scripts/test_regression.sh trunk tag met-6.0
#
#=======================================================================

# Constants
SVN_BASE="https://svn-met-dev.cgd.ucar.edu"

# Check for quiet flag
QUIET=0
if [[ $1 == "-q" ]]; then QUIET=1; shift; REDIR=">/dev/null 2>&1"; fi

function usage {
   [ $QUIET -eq 1 ] && return 0
        echo
   echo "USAGE: test_regression.sh {version1} {version2}"
   echo "   where version1 and version2 each have the format "
   echo "      trunk [rev] | branch name [rev] | tag name [rev]"
   echo "   and:"
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
if [ $# -lt 1 ]; then usage; exit; fi

# Parse the input parameters for each MET version
while [ $# -gt 0 ]; do

   case $1 in
      trunk)
         if [[ $# -ge 2 && $(echo $2 | perl -ne'print /^\d+$/ ? 1 : 0') -eq 1 ]]; then
            REVISION=$2;
            shift
         else
            REVISION=$(svn info $SVN_BASE | egrep '^Rev' | awk '{print $2}');
         fi
         ARGS="trunk $REVISION"
         SUFFIX="rev${REVISION}"
         ;;
      tag)
         if [ $# -lt 2 ]; then
            log "ERROR: tag not specified";
            usage;
            exit 1;
         fi
         NAME=$2
         if [[ $# -ge 3 && $(echo $3 | perl -ne'print /^\d+$/ ? 1 : 0') -eq 1 ]]; then
            REVISION=$3
            shift
         else
            REVISION=$(svn info $SVN_BASE | egrep '^Rev' | awk '{print $2}');
         fi
         ARGS="tag $NAME $REVISION"
         SUFFIX="tag_${NAME}_rev${REVISION}"
         shift
         ;;
      branch)
         if [ $# -lt 2 ]; then
            log "ERROR: branch not specified";
            usage;
            exit 1;
         fi
         NAME=$2
         if [[ $# -ge 3 && $(echo $3 | perl -ne'print /^\d+$/ ? 1 : 0') -eq 1 ]]; then
            REVISION=$3
            shift
         else
            REVISION=$(svn info $SVN_BASE | egrep '^Rev' | awk '{print $2}');
         fi
         ARGS="branch $NAME $REVISION"
         SUFFIX="branch_${NAME}_rev${REVISION}"
         shift
         ;;
      *)
         log "ERROR: unrecognized input argument '$1'";
         usage;
         exit 1;
         ;;
   esac
   shift

   # Assign the parameters for the appropriate version
   if   [[ $VER1 == "" ]]; then VER1=$ARGS; REV1=$REVISION; SUF1=$SUFFIX;
   elif [[ $VER2 == "" ]]; then VER2=$ARGS; REV2=$REVISION; SUF2=$SUFFIX;
   else
      log "ERROR: unexpected arguments: $1";
      usage;
      exit 1
   fi

done

# Run the unit test script for each set of parameters
PID_LIST=""
FAILURE=0
for V in $(seq 2); do

   VER=$(echo $(eval "echo \$VER${V}"))
   REV=$(echo $(eval "echo \$REV${V}"))
   SUF=$(echo $(eval "echo \$SUF${V}"))
   LOG="$(pwd)/test_unit_${SUF}.log"

   # If the unit test output already exists, do not run it again
   if [ -e test_output_$SUF ]; then
      log "found existing unit test output test_output_$SUF, skipping unit tests"

   # Otherwise, run the unit test for the specified version
   else
      # Set required environment variables
      export MET_BUILD_BASE=$(pwd)/met_$SUF
      export MET_BASE=${MET_BUILD_BASE}/share/met
      export MET_TEST_BASE=$(pwd)/test_$SUF
      export MET_TMP_DIR=$(pwd)/test_$SUF/tmp
      export MET_TEST_INPUT=${MET_TEST_INPUT:-/d3/projects/MET/MET_test_data/unit_test}
      export MET_TEST_OUTPUT=$(pwd)/test_output_$SUF

      if [ $REV -lt 4017 ]; then
         export MET_NETCDF=/d3/projects/MET/MET_releases/external_libs/netcdf/netcdf-4.1.3
         export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:${MET_NETCDF}/lib
      fi

      # Run the unit tests in the background 
      UNIT_CMD="scripts/test_unit.sh $VER"
      log "running unit tests on $VER: $UNIT_CMD"
      log "writing log file: $LOG"
      $UNIT_CMD > $LOG 2>&1 &
      PID_LIST="${PID_LIST} $!"

      # Sleep for 5 minutes to stagger the unit test start times
      sleep 300
   fi

done

# Wait for unit tests to finish
for PID in $PID_LIST; do
   wait $PID || let "FAILURE=1"
done

# Check return status 
if [ "$FAILURE" == "1" ]; then
   log "ERROR: unit test failed";
   exit 1
fi

# Run the directory comparison tool on the output from the unit tests
export MET_TEST_BASE=$(pwd)/test_$SUF2
COMP_CMD="${MET_TEST_BASE}/bin/comp_dir.sh test_output_$SUF1 test_output_$SUF2"
log "running unit test output comparison: $COMP_CMD"
$COMP_CMD
COMP_CMD="$(pwd)/test_${SUF1}/bin/comp_nc_features.sh test_output_${SUF1}/netcdf"
log "running unit test netcdf output comparison: $COMP_CMD"
$COMP_CMD

exit
