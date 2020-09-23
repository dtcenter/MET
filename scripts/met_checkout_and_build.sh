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
#    git clone https://github.com/dtcenter/MET
#    MET/scripts/met_checkout_and_build.sh [new|tag|branch] [name]
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
#       met_checkout_and_build tag tag_name
#    Build an existing branch:
#       met_checkout_and_build branch branch_name
#
#=======================================================================

# Constants
GIT_REPO="https://github.com/dtcenter/MET"

# MET_DEVELOPMENT must be set to build a release
export MET_DEVELOPMENT=true

# Get the number of arguments.
NARGS=$#

# Print usage
usage() {
   echo
   echo "USAGE: met_checkout_and_build.sh <new, tag, branch> <name>"
   echo "       'met_checkout_and_build.sh new'             to build a new release from develop with no TAG"
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
get_git_info() {
  echo "CALLING: git config, git rev-parse, and git log"
  git config --get remote.origin.url | sed -r 's/^/Repository:\t\t/g'       > met/data/version.txt
  git rev-parse --short HEAD         | sed -r 's/^/Last Changed Rev:\t/g'  >> met/data/version.txt
  git log -1 --format=%cd met        | sed -r 's/^/Last Changed Date:\t/g' >> met/data/version.txt
}

# Clone repo into a working directory
mkdir build; cd build
git clone ${GIT_REPO}
cd MET

# Parse the command line
if [[ ${NARGS} -eq 1 && $1 == "new" ]]; then

   echo "Checking out the latest MET develop branch without tagging."
   run_command "git checkout develop"
   BUILD_ARGS=""
   get_git_info met

elif [[ ${NARGS} -eq 2 && $1 == "new_tag" ]]; then

   echo "Checking out the latest MET develop branch and tagging as '$2'."
   run_command "git checkout develop"
   echo "Creating tag $2 from the develop branch" > log.msg
   run_command "git tag -a $2 -F log.msg"
   run_command "git push origin $2"
   rm log.msg
   BUILD_ARGS="$2"
   get_git_info ${BUILD_ARGS}

elif [[ ${NARGS} -eq 2 && $1 == "new_branch" ]]; then

   echo "Checking out the latest MET develop branch and creating branch '$2'."
   run_command "git checkout develop"
   echo "Creating met branch $2 from the develop branch" > log.msg
   run_command "git checkout -b $2 -F log.msg"
   run_command "git push -u origin $2"
   rm log.msg
   BUILD_ARGS="$2"
   get_git_info ${BUILD_ARGS}

elif [[ ${NARGS} -eq 2 && $1 == "tag" ]]; then

   echo "Checking out the '$2' tagged version of MET."
   run_command "git checkout $2"
   BUILD_ARGS="$2"
   get_git_info ${BUILD_ARGS}

elif [[ ${NARGS} -eq 2 && $1 == "branch" ]]; then

   echo "Checking out the '$2' branch version of MET."
   run_command "git checkout $2"
   BUILD_ARGS="$2"
   get_git_info ${BUILD_ARGS}

else
   usage
   exit 1
fi

# Check that the met_build.sh script exists
if [[ ! -e "scripts/met_build.sh" ]]; then

  echo
  echo "ERROR: scripts/met_build.sh does not exist!"
  echo
  exit 1

fi

# Call the build script to build the release
run_command "scripts/met_build.sh ${BUILD_ARGS}"
run_command "mv *.tar.gz ../../."
run_command "cd ../.."
run_command "rm -rf build"
