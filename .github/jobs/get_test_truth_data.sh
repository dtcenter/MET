#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DATA_VERSION=$1

time_command docker create --name met_truth dtcenter/met-data-output:${DATA_VERSION}
