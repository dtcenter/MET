#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

time_command docker pull ${DOCKERHUB_TAG}
