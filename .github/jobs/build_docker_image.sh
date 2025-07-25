#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$(get_dockerhub_tag)

# For the release-docker-images.yml workflow
DOCKERFILE_PATH=${GITHUB_WORKSPACE}/${SOURCE_BRANCH}/internal/scripts/docker/Dockerfile

# For the testing.yml workflow
if [[ ! -f "${DOCKERFILE_PATH}" ]]; then
  DOCKERFILE_PATH=${GITHUB_WORKSPACE}/internal/scripts/docker/Dockerfile.copy
fi

CMD_LOGFILE=${GITHUB_WORKSPACE}/docker_build.log

if ! time_command docker build -t ${DOCKERHUB_TAG} \
     --build-arg SOURCE_BRANCH \
     --build-arg MET_BASE_REPO \
     --build-arg MET_BASE_TAG \
     --build-arg MET_CONFIG_OPTS \
     -f $DOCKERFILE_PATH ${GITHUB_WORKSPACE}; then
  echo "::group::${CMD_LOGFILE}"
  cat ${CMD_LOGFILE}
  echo "::endgroup::"
  exit 1
fi

# Copy the log directory from the image
id=$(docker create ${DOCKERHUB_TAG})
time_command docker cp $id:/met/logs met_logs
mv met_logs/*.log ${GITHUB_WORKSPACE}/.
docker rm -v $id
