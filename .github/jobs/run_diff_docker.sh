#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

# Get truth output data
${GITHUB_WORKSPACE}/.github/jobs/get_test_truth_data.sh ${TRUTH_DATA_VERSION}

# Set up directories to mount
LOCAL_OUTPUT_DIR=${RUNNER_WORKSPACE}/output
DOCKER_OUTPUT_DIR=/data/output/met_test_output

LOCAL_DIFF_DIR=${RUNNER_WORKSPACE}/diff
DOCKER_DIFF_DIR=/data/output/met_test_diff

LOCAL_LOG_DIR=${RUNNER_WORKSPACE}/logs
DOCKER_LOG_DIR=/met/logs

# Create local directories to store output
mkdir -p ${LOCAL_LOG_DIR}
mkdir -p ${LOCAL_DIFF_DIR}

mount_args="-v ${LOCAL_OUTPUT_DIR}:${DOCKER_OUTPUT_DIR} -v ${LOCAL_DIFF_DIR}:${DOCKER_DIFF_DIR} -v ${LOCAL_LOG_DIR}:${DOCKER_LOG_DIR}"

# Set up data volumes
volumes_from="--volumes-from met_truth"

# run unit test script inside Docker, mount MET input and truth data
cmd="\${MET_REPO_DIR}/.github/jobs/run_diff_tests.sh"
time_command docker run ${volumes_from} ${mount_args} ${DOCKERHUB_TAG} bash -c \"${cmd}\"
if [ $? != 0 ]; then
  exit 1
fi

if [ "$(ls -A ${LOCAL_DIFF_DIR})" ]; then
  echo "ERROR: Differences exist in the output"
  exit 1
fi
