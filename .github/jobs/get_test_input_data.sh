#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

time_command docker create --name met_input dtcenter/met-data-dev:${DATA_VERSION}
