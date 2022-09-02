#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

DOCKERFILE_PATH=${GITHUB_WORKSPACE}/internal/scripts/docker/Dockerfile.copy

CMD_LOGFILE=${GITHUB_WORKSPACE}/docker_build.log

time_command docker build -t ${DOCKERHUB_TAG} \
    --build-arg SOURCE_BRANCH \
    --build-arg MET_BASE_REPO \
    --build-arg MET_BASE_IMAGE \
    -f $DOCKERFILE_PATH ${GITHUB_WORKSPACE}
if [ $? != 0 ]; then
  cat ${GITHUB_WORKSPACE}/docker_build.log
  # Append the full make_install.log file
  echo "Appending make_install.log to docker_build.log. See the logs artifact for details."
  cat ${GITHUB_WORKSPACE}/make_install.log >> ${GITHUB_WORKSPACE}/docker_build.log
  exit 1
fi
