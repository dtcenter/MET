#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$(get_dockerhub_tag)

# Skip docker push if credentials are not set
if [[ -z ${DOCKER_USERNAME+x} ]] || [[ -z ${DOCKER_PASSWORD+x} ]]; then
    echo "DockerHub credentials not set. Skipping docker push"
    exit 0
fi

echo "$DOCKER_PASSWORD" | docker login --username "$DOCKER_USERNAME" --password-stdin

if ! time_command docker push ${DOCKERHUB_TAG}; then
    exit 1
fi

# For the release-docker-images.yml workflow, push X.Y and X.Y-latest for vX.Y.Z versions
# NOTE: eventually remove X.Y-latest tags in favor of just X.Y
#       keep -latest until all references to it have been replaced with X.Y
if [[ "${UPDATE_LATEST}" == "true" && "${SOURCE_BRANCH}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    XY_VERSION=$(echo ${SOURCE_BRANCH} | sed 's/^v//g' | cut -f1,2 -d'.')
    for tag in "${XY_VERSION}" "${XY_VERSION}-latest"; do
        if ! time_command docker tag ${DOCKERHUB_TAG} ${DOCKERHUB_REPO}:${tag}; then
            exit 1
        fi
        if ! time_command docker push ${DOCKERHUB_REPO}:${tag}; then
            exit 1
        fi
    done
fi
