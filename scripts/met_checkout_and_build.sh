#!/bin/bash
#
# Checkout code and build a release of the MET
# (Model Evaluation Tools)
#=======================================================================
#
# This met_checkout_and_build.sh script will build a release
# of MET using either the latest version of the files or using
# an existing tag or branch name.  First, go to the directory where
# you'd like the release built.  Then run:
#    svn co https://svn-met-dev.cgd.ucar.edu/build/scripts
#    scripts/met_checkout_and_build.sh [new|tag|branch] [name]
#
# For a new release, this script will:
# (1) Checkout the latest version of the MET source code and
#     (optionally) tag it.
# (2) Build the release, name it, and tar it up.
#
# For an existing release, this script will:
# (1) Checkout an existing tag or branch of the MET source code.
# (2) Build the release, name it, and tar it up.
#
# Usage: met_checkout_and_build.sh <new, tag, branch> name
#    Build a new release without tagging:
#       met_checkout_and_build.sh new
#    Build a new release with tagging:
#       met_checkout_and_build.sh new_tag tag_name
#    Build a new release and create a branch:
#       met_checkout_and_build.sh new_branch branch_name
#    Build an existing tagged release:
#       MET_checkout_and_build tag tag_name
#    Build an exisitng branch:
#       MET_checkout_and_build branch branch_name
#
#=======================================================================

# Constants
SVN_BASE="https://svn-met-dev.cgd.ucar.edu"

# MET_DEVELOPMENT must be set to build a release
export MET_DEVELOPMENT=true

# Repository Structure:
#    Development version: SVN_BASE/trunk/met
#    Tagged versions:     SVN_BASE/tags/met/TAG_NAME
#    Branch versions:     SVN_BASE/branches/met/BRANCH_NAME

# Get the number of arguments.
NARGS=$#

# Print usage
usage() {
   echo
   echo "USAGE: met_checkout_and_build.sh <new, tag, branch> <name>"
   echo "       'met_checkout_and_build.sh new'             to build a new release with no TAG"
   echo "       'met_checkout_and_build.sh new_tag    NAME' to build a new release and tag it"
   echo "       'met_checkout_and_build.sh new_branch NAME' to build a new release and create a branch"
   echo "       'met_checkout_and_build.sh tag        NAME' to build an existing tag"
   echo "       'met_checkout_and_build.sh branch     NAME' to build an existing branch"
   echo
}

# Check for 1 or 2 arguments
if [[ ${NARGS} -ne 1 && ${NARGS} -ne 2 ]]; then
   usage
   exit 1
fi

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

# Run svn info to update contents of version.txt
get_svn_info() {

  echo "CALLING: svn info $1"
  svn info $1 |
    egrep '^URL:|^Last Changed Rev:|^Last Changed Date:' |
    sed 's=URL: =Repository:\t=' |
    sed 's=Last Changed Rev: =Revision:\t=' |
    sed 's=Last Changed Date: =Change Date:\t=' > $2/data/version.txt
}

# Parse the command line
if [[ ${NARGS} -eq 1 && $1 == "new" ]]; then

   echo "Checking out the latest version of MET without tagging."
   CMD='export'
   SVN_PATH="${SVN_BASE}/trunk/met"
   run_command "svn ${CMD} -q ${SVN_PATH}"
   BUILD_ARGS=""
   get_svn_info ${SVN_PATH} met

elif [[ ${NARGS} -eq 2 && $1 == "new_tag" ]]; then

   echo "Checking out the latest version of MET and tagging as '$2'."
   CMD='co'
   SVN_PATH="${SVN_BASE}/tags/met/$2"
   run_command "svn ${CMD} -q ${SVN_BASE}/trunk/met"
   echo "Tagging met $2" > log.msg
   run_command "svn cp ${SVN_BASE}/trunk/met ${SVN_PATH} -F log.msg"
   echo "Tagging test $2" > log.msg
   run_command "svn cp ${SVN_BASE}/trunk/test ${SVN_BASE}/tags/test/$2 -F log.msg"
   run_command "mv met $2"
   rm log.msg
   BUILD_ARGS="$2"
   get_svn_info ${SVN_PATH} ${BUILD_ARGS}

elif [[ ${NARGS} -eq 2 && $1 == "new_branch" ]]; then

   echo "Checking out the latest version of MET and creating branch '$2'."
   CMD='co'
   SVN_PATH="${SVN_BASE}/branches/met/$2"
   run_command "svn ${CMD} -q ${SVN_BASE}/trunk/met"
   echo "Creating met branch $2" > log.msg
   run_command "svn cp ${SVN_BASE}/trunk/met ${SVN_PATH} -F log.msg"
   run_command "svn ${CMD} -q ${SVN_BASE}/trunk/test"
   echo "Creating test branch $2" > log.msg
   run_command "svn cp ${SVN_BASE}/trunk/test ${SVN_BASE}/branches/test/$2 -F log.msg"
   run_command "mv met $2"
   rm log.msg
   BUILD_ARGS="$2"
   get_svn_info ${SVN_PATH} ${BUILD_ARGS}

elif [[ ${NARGS} -eq 2 && $1 == "tag" ]]; then

   echo "Checking out the '$2' tagged version of MET."
   CMD='export'
   SVN_PATH="${SVN_BASE}/tags/met/$2"
   run_command "svn ${CMD} -q ${SVN_PATH}"
   BUILD_ARGS="$2"
   get_svn_info ${SVN_PATH} ${BUILD_ARGS}

elif [[ ${NARGS} -eq 2 && $1 == "branch" ]]; then

   echo "Checking out the '$2' branch version of MET."
   CMD='export'
   SVN_PATH="${SVN_BASE}/branches/met/$2"
   run_command "svn ${CMD} -q ${SVN_PATH}"
   BUILD_ARGS="$2"
   get_svn_info ${SVN_PATH} ${BUILD_ARGS}

else
   usage
   exit 1
fi

# Call the build script to build the release
run_command "cp -f ${2:-met}/build/met_build.sh scripts/."
run_command "chmod +x scripts/met_build.sh"
run_command "scripts/met_build.sh ${BUILD_ARGS}"
run_command "rm -rf ${2:-met}"
