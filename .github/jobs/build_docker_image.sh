#! /bin/bash

# utility function to run command get log the time it took to run
function time_command {
  local start_seconds=$SECONDS
  echo "RUNNING: $*"
  "$@"
  local error=$?

  local duration=$(( SECONDS - start_seconds ))
  echo "TIMING: Command took `printf '%02d' $(($duration / 60))`:`printf '%02d' $(($duration % 60))` (MM:SS): '$*'"
  if [ ${error} -ne 0 ]; then
    echo "ERROR: '$*' exited with status = ${error}"
  fi
  return $error
}

prefix=refs/heads/
branch_name=${GITHUB_REF#"$prefix"}
#branch_name=${GITHUB_HEAD_REF}
DOCKERHUB_TAG=dtcenter/met-dev:${branch_name}

DOCKERFILE_DIR=${GITHUB_WORKSPACE}/scripts/docker

time_command docker build -t ${DOCKERHUB_TAG} \
       --build-arg SOURCE_BRANCH=$branch_name \
       $DOCKERFILE_DIR

# skip docker push if credentials are not set
if [ -z ${DOCKER_USERNAME+x} ] || [ -z ${DOCKER_PASSWORD+x} ]; then
    echo "DockerHub credentials not set. Skipping docker push"
    exit 0
fi

echo "$DOCKER_PASSWORD" | docker login --username "$DOCKER_USERNAME" --password-stdin

time_command docker push ${DOCKERHUB_TAG}
