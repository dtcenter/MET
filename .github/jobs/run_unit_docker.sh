#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

# Create directories to store output
mkdir -p ${RUNNER_WORKSPACE}/logs
mkdir -p ${RUNNER_WORKSPACE}/diff

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

# Pull MET Image from DockerHub
${GITHUB_WORKSPACE}/.github/jobs/pull_docker_image.sh ${DOCKERHUB_TAG}

# Get test input data
${GITHUB_WORKSPACE}/.github/jobs/get_test_input_data.sh ${INPUT_DATA_VERSION}

# Set up directories to mount
LOCAL_OUTPUT_DIR=${RUNNER_WORKSPACE}/output
DOCKER_OUTPUT_DIR=/data/output/met_test_output

LOCAL_DIFF_DIR=${RUNNER_WORKSPACE}/diff
DOCKER_DIFF_DIR=/data/output/met_test_diff

mount_args="-v $LOCAL_OUTPUT_DIR:$DOCKER_OUTPUT_DIR -v $LOCAL_DIFF_DIR:$DOCKER_DIFF_DIR -v ${RUNNER_WORKSPACE}/logs:/met/logs"

# Set up data volumes
volumes_from="--volumes-from met_input"

export TESTS_TO_RUN=$TESTS

# run unit test script inside Docker, mount MET input and truth data
cmd="\${MET_REPO_DIR}/.github/jobs/run_unit_tests.sh"
time_command docker run -e TESTS_TO_RUN ${volumes_from} ${mount_args} ${DOCKERHUB_TAG} bash -c \"${cmd}\"
if [ $? != 0 ]; then
  exit 1
fi
