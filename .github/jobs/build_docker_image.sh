#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$(get_dockerhub_tag)

# For the release-docker-images.yml workflow
DOCKERFILE_PATH=${GITHUB_WORKSPACE}/${SOURCE_BRANCH}/internal/scripts/docker/Dockerfile

# For the testing.yml workflow
if [[ ! -f "${DOCKERFILE_PATH}" ]]; then
  DOCKERFILE_PATH=${GITHUB_WORKSPACE}/internal/scripts/docker/Dockerfile.copy
fi

BUILD_LOGFILE=${GITHUB_WORKSPACE}/docker_build.log

# Only pass the METbaseimage build arguments that are set in the environment.
# Passing --build-arg for a variable that is unset leaves the default value from
# the Dockerfile in place, which is how releases created before
# get_met_base_image.sh existed select their base image.
BUILD_ARGS=(--build-arg SOURCE_BRANCH --build-arg MET_CONFIG_OPTS)
if [[ -n ${MET_BASE_REPO} ]]; then
  BUILD_ARGS+=(--build-arg MET_BASE_REPO)
fi
if [[ -n ${MET_BASE_TAG} ]]; then
  BUILD_ARGS+=(--build-arg MET_BASE_TAG)
fi

CMD_LOGFILE=${BUILD_LOGFILE}

time_command docker build -t ${DOCKERHUB_TAG} \
    "${BUILD_ARGS[@]}" \
    -f $DOCKERFILE_PATH ${GITHUB_WORKSPACE}
if [[ $? != 0 ]]; then
  # time_command unsets CMD_LOGFILE, so report the build log by its saved path
  cat ${BUILD_LOGFILE}
  exit 1
fi

# Copy the log directory from the image
id=$(docker create ${DOCKERHUB_TAG})
time_command docker cp $id:/met/logs met_logs
mv met_logs/*.log ${GITHUB_WORKSPACE}/.
docker rm -v $id
