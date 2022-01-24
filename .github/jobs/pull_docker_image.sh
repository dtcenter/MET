#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$1

time_command docker pull ${DOCKERHUB_TAG}
