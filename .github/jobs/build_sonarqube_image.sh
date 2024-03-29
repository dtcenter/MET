#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=met-sonarqube

DOCKERFILE_PATH=${GITHUB_WORKSPACE}/internal/scripts/docker/Dockerfile.sonarqube

CMD_LOGFILE=${GITHUB_WORKSPACE}/sonarqube_build.log

time_command docker build -t ${DOCKERHUB_TAG} \
    --build-arg MET_BASE_REPO \
    --build-arg MET_BASE_TAG \
    --build-arg SOURCE_BRANCH \
    --build-arg MET_CONFIG_OPTS \
    --build-arg SONAR_SCANNER_VERSION \
    --build-arg SONAR_HOST_URL \
    --build-arg SONAR_TOKEN \
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
