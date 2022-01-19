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
DOCKERHUB_TAG=dtcenter/met:${branch_name}

DOCKERFILE_DIR=${GITHUB_WORKSPACE}/scripts/docker

echo "::group::Docker Build Command"
time_command docker build -t ${DOCKERHUB_TAG} \
       --build-arg SOURCE_BRANCH=$branch_name \
       $DOCKERFILE_DIR
echo "::endgroup::"
