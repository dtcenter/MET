#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$(get_dockerhub_tag)

# Skip docker push if credentials are not set
if [ -z ${DOCKER_USERNAME+x} ] || [ -z ${DOCKER_PASSWORD+x} ]; then
    echo "DockerHub credentials not set. Skipping docker push"
    exit 0
fi

echo "$DOCKER_PASSWORD" | docker login --username "$DOCKER_USERNAME" --password-stdin

time_command docker push ${DOCKERHUB_TAG}

# For the release-docker-images.yml workflow, push X.Y-latest for vX.Y.Z versions
if [[ "${UPDATE_LATEST}" == "true" && "${SOURCE_BRANCH}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    LATEST_TAG=$(echo ${SOURCE_BRANCH} | sed 's/^v//g' | cut -f1,2 -d'.')-latest
    time_command docker tag ${DOCKERHUB_TAG} ${DOCKERHUB_REPO}:${LATEST_TAG}
    time_command docker push ${DOCKERHUB_REPO}:${LATEST_TAG}
fi
