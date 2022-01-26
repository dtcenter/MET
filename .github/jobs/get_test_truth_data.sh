#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DATA_VERSION=$1

time_command docker create --name met_truth dtcenter/met-data-output:${DATA_VERSION}
if [ $? != 0 ]; then
  echo "Image tag ${DATA_VERSION} does not exist. Using develop..."
  time_command docker create --name met_truth dtcenter/met-data-output:develop
fi
