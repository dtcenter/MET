#!/bin/bash
#
# Run nightly SonarQube scan
#=======================================================================
#
# This run_nightly.sh script calls the run_sonarqube.sh script.
# It is intented to be run nightly through cron. Output should be
# directed to the LOGFILE, per cron convention. To run this script, use
# the following commands:
#
#    git clone https://github.com/dtcenter/MET
#    MET/internal/scripts/sosnarqube/run_nightly.sh name
#
# Usage: run_nightly.sh name
#    where "name" specifies a branch, tag, or hash
#
# For example, scan the develop branch:
#    run_nightly.sh develop
#
#=======================================================================

# Constants
EMAIL_LIST="hsoh@ucar.edu"  # overridden by $MET_CRON_EMAIL_LIST_MET or $MET_CRON_EMAIL_LIST
KEEP_DAYS=5
GIT_REPO_NAME=MET

function usage {
  echo
  echo "USAGE: run_nightly.sh name"
  echo "   where \"name\" specifies a branch, tag, or hash."
  echo
}

# Check for arguments
if [ $# -lt 1 ]; then usage; exit 1; fi

# Store the full path to the scripts directory
SCRIPT_DIR=`dirname $0`
if [[ ${0:0:1} != "/" ]]; then SCRIPT_DIR=$(pwd)/${SCRIPT_DIR}; fi 

# Define the development environment
ENV_FILE=${SCRIPT_DIR}/../environment/development.`hostname`
if [[ ! -e ${ENV_FILE} ]]; then
  echo "$0: ERROR -> Development environment file missing: ${ENV_FILE}"
  exit 1
fi
source ${ENV_FILE}

# Delete old directories
find ${MET_PROJ_DIR}/MET_regression/sonarqube \
     -mtime +${KEEP_DAYS} -name "NB*" | \
     xargs rm -rf

# Create and switch to a run directory
TODAY=`date +%Y%m%d`
YESTERDAY=`date -d "1 day ago" +%Y%m%d`
RUN_DIR=${MET_PROJ_DIR}/MET_regression/sonarqube/NB${TODAY}
if [[ -e ${RUN_DIR} ]]; then rm -rf ${RUN_DIR}; fi
mkdir -p ${RUN_DIR}
cd ${RUN_DIR}

# Create a logfile
LOGFILE=${RUN_DIR}/run_sonarqube_${TODAY}.log

# Run scan and check for bad return status
${SCRIPT_DIR}/run_sonarqube.sh ${1} > ${LOGFILE}
if [[ $? -ne 0 ]]; then
  my_cmd="echo \$MET_CRON_EMAIL_LIST_${GIT_REPO_NAME}"
  _EMAIL_LIST="$(eval $my_cmd)"
  [ -z "$_EMAIL_LIST" ] && _EMAIL_LIST="$MET_CRON_EMAIL_LIST"
  [ -z "$_EMAIL_LIST" ] && _EMAIL_LIST="$EMAIL_LIST"
  echo "$0: The nightly SonarQube scanning for $GIT_REPO_NAME FAILED in `basename ${RUN_DIR}`." >> ${LOGFILE}
  cat ${LOGFILE} | mail -s "$GIT_REPO_NAME SonarQube scanning failed for ${1} in `basename ${RUN_DIR}` (autogen msg)" ${_EMAIL_LIST}
  exit 1
fi

# Convert SonarQube report from pdf to html

exit 0
