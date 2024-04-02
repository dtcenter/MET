#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=met-sonarqube-gha

DOCKERFILE_PATH=${GITHUB_WORKSPACE}/internal/scripts/docker/Dockerfile.sonarqube

CMD_LOGFILE=${GITHUB_WORKSPACE}/sonarqube_build.log

#
# Define the $SONAR_REFERENCE_BRANCH as the
#   - Target of any requests
#   - Manual setting for workflow dispatch
#   - Source branch for any pushes (e.g. develop)
#
if [ "${GITHUB_EVENT_NAME}" == "pull_request" ]; then
  export SONAR_REFERENCE_BRANCH=${GITHUB_BASE_REF}
elif [ "${GITHUB_EVENT_NAME}" == "workflow_dispatch" ]; then
  export SONAR_REFERENCE_BRANCH=${WD_REFERENCE_BRANCH}
else
  export SONAR_REFERENCE_BRANCH=${SOURCE_BRANCH}
fi

echo SONAR_REFERENCE_BRANCH=${SONAR_REFERENCE_BRANCH}

time_command docker build -t ${DOCKERHUB_TAG} \
    --build-arg MET_BASE_REPO \
    --build-arg MET_BASE_TAG \
    --build-arg SOURCE_BRANCH \
    --build-arg MET_CONFIG_OPTS \
    --build-arg SONAR_SCANNER_VERSION \
    --build-arg SONAR_HOST_URL \
    --build-arg SONAR_TOKEN \
    --build-arg SONAR_REFERENCE_BRANCH \
    -f $DOCKERFILE_PATH ${GITHUB_WORKSPACE}
if [ $? != 0 ]; then
  cat ${CMD_LOGFILE}
  exit 1
fi

time_command docker run -it --rm ${DOCKERHUB_TAG} ls /
time_command docker run -it --rm ${DOCKERHUB_TAG} ls /met
time_command docker run -it --rm ${DOCKERHUB_TAG} ls /met/.scannerwork
time_command docker run -it --rm ${DOCKERHUB_TAG} cat /met/.scannerwork/python-report-task.txt
time_command docker run -it --rm ${DOCKERHUB_TAG} cat /met/.scannerwork/cxx-report-task.txt

# Copy the .scannerwork directory from the image
id=$(docker create ${DOCKERHUB_TAG})
time_command docker cp $id:/met/.scannerwork /tmp/met_scannerwork 
docker rm -v $id

time_command ls /tmp/met_scannerwork
time_command cat /tmp/met_scannerwork/python-report-task.txt
time_command cat /tmp/met_scannerwork/cxx-report-task.txt
