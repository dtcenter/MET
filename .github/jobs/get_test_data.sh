#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

time_command docker create --name met_input dtcenter/met-data-dev:develop
time_command docker create --name met_truth dtcenter/met-data-output:develop
