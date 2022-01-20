#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

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

time_command docker build -t ${DOCKERHUB_TAG} \
       --build-arg SOURCE_BRANCH=$branch_name \
       $DOCKERFILE_DIR
