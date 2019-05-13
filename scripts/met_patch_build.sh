#!/bin/ksh

# Set MET_DEVELOPMENT environment variable to build release.
export MET_DEVELOPMENT=true

SVN_REP="https://svn-met-dev.cgd.ucar.edu/build/scripts"

# Sub-routine for running a command and checking return status
run_command() {

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

#
# Parse the input arguments
#
  
case $# in
  1 )
    CHK1="tag";    export MET_DIST1="$1"
    CHK2="branch"; export MET_DIST2="${1}_bugfix"
    ;;

  2 )
    CHK1=$(echo $1 | perl -pe's|/.*||')
    export MET_DIST1=$(echo $1 | perl -pe's|.*/||')
    CHK2=$(echo $2 | perl -pe's|/.*||')
    export MET_DIST2=$(echo $2 | perl -pe's|.*/||')
    ;;

  * )
    echo
    echo "USAGE: met_patch_build.sh"
    echo "  $0 {'tag'|'branch'}/[dist1] {'tag'|'branch'}/[dist2]"
    echo "or"
    echo "  $0 [dist]"
    echo "    where dist expands to tag/[dist] and branch/[dist]_bugfix"
    echo

    exit 1;

esac

#
# Retrieve the MET distributions of interest
#

# build the first distribution
echo
echo "Building distribution $MET_DIST1..."
run_command "scripts/met_checkout_and_build.sh $CHK1 $MET_DIST1"
run_command "tar -xzf ${MET_DIST1}.*.tar.gz"

# build the second distribution
echo
echo "Building distribution $MET_DIST2..."
run_command "rm -f scripts/met_build.sh"
run_command "scripts/met_checkout_and_build.sh $CHK2 $MET_DIST2"
run_command "tar -xzf ${MET_DIST2}.*.tar.gz"

#
# Build the patch bundle
#

# find files that are present in MET_DIST2 but not in MET_DIST1
echo
echo "Finding differences..."
DIFF_LIST=""
for D in $(diff -rq $MET_DIST1 $MET_DIST2 | \
           perl -ne's|Only in (.*): |$1/| and m|$ENV{MET_DIST2}| and print'); do
  DIFF_LIST="$DIFF_LIST${DIFF_LIST:+ }$D"
done

# build a list of files to check for differences
for D in $(find $MET_DIST1 -type f | sort | egrep -v 'configure$|configure.ac$|python/Makefile$|Rscripts/Makefile$|Rscripts/include/Makefile$' | perl -pe's|$ENV{MET_DIST1}/||'); do
  [[ -e "$MET_DIST2/$D" &&
     ! -z $(diff $MET_DIST1/$D $MET_DIST2/$D | \
       perl -ne'( (/^[<>]/ && ! m|^[<>]\s*//|) || /^Binary/ ) and print')
  ]] && DIFF_LIST="$DIFF_LIST${DIFF_LIST:+ }$MET_DIST2/$D"
done

# build a tarball with the differing files
TARBALL="${MET_DIST1}_patches_$(date +%Y%m%d).tar.gz"
rm -f $TARBALL
echo
echo "Building patch tarball $TARBALL..."
TAR_CMD="tar czvf ../$TARBALL"
for D in $DIFF_LIST; do TAR_CMD="$TAR_CMD $(echo $D | perl -pe's|$ENV{MET_DIST2}/||')"; done
run_command "cd $MET_DIST2"
run_command "$TAR_CMD"
run_command "cd .."
echo
