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

# Number of lines of the build log to print when the build fails
BUILD_LOGFILE_TAIL=200

# Only pass the METbaseimage build arguments that are set to a non-empty value.
# Passing --build-arg for an unset or empty variable overrides the Dockerfile
# default with an empty string, while omitting the flag leaves that default in
# place, which is how releases created before get_met_base_image.sh existed
# select their base image.
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
  # time_command unsets CMD_LOGFILE, so report the build log by its saved path.
  # Print only the tail of it: an --enable-all build log runs to tens of
  # thousands of lines, and the whole file is uploaded as a job artifact anyway.
  echo "ERROR: Last ${BUILD_LOGFILE_TAIL} lines of ${BUILD_LOGFILE}:"
  tail -n ${BUILD_LOGFILE_TAIL} ${BUILD_LOGFILE}
  exit 1
fi

# Copy the log directory from the image
id=$(docker create ${DOCKERHUB_TAG})
time_command docker cp $id:/met/logs met_logs
mv met_logs/*.log ${GITHUB_WORKSPACE}/.
docker rm -v $id
