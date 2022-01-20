#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

prefix=refs/heads/
branch_name=${GITHUB_REF#"$prefix"}
DOCKERHUB_TAG=dtcenter/met:${branch_name}

# skip docker push if credentials are not set
if [ -z ${DOCKER_USERNAME+x} ] || [ -z ${DOCKER_PASSWORD+x} ]; then
    echo "DockerHub credentials not set. Skipping docker push"
    exit 0
fi

echo "$DOCKER_PASSWORD" | docker login --username "$DOCKER_USERNAME" --password-stdin

time_command docker push ${DOCKERHUB_TAG}
