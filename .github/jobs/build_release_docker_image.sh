#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$(get_dockerhub_tag)

# Checked out by release-docker-images.yml workflow 
DOCKERFILE_PATH=${GITHUB_WORKSPACE}/docker/Dockerfile

CMD_LOGFILE=${GITHUB_WORKSPACE}/docker_build_release.log

time_command docker build -t ${DOCKERHUB_TAG} \
    --build-arg SOURCE_BRANCH \
    --build-arg MET_BASE_REPO \
    --build-arg MET_BASE_TAG \
    --build-arg MET_CONFIG_OPTS \
    -f $DOCKERFILE_PATH ${GITHUB_WORKSPACE}
if [ $? != 0 ]; then
  cat ${CMD_LOGFILE}
  exit 1
fi

# Copy the log directory from the image
id=$(docker create ${DOCKERHUB_TAG})
time_command docker cp $id:/met/logs met_logs
mv met_logs/*.log ${GITHUB_WORKSPACE}/.
docker rm -v $id
