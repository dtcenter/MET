#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

DOCKERFILE_PATH=${GITHUB_WORKSPACE}/scripts/docker/Dockerfile.copy

CMD_LOGFILE=${GITHUB_WORKSPACE}/docker_build.log

time_command docker build -t ${DOCKERHUB_TAG} \
    --build-arg SOURCE_BRANCH \
    --build-arg MET_BASE_IMAGE \
    -f $DOCKERFILE_PATH ${GITHUB_WORKSPACE}
