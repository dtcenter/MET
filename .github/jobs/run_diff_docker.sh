#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh
source ${GITHUB_WORKSPACE}/.github/jobs/test_env_vars.sh

# Get truth output data
${GITHUB_WORKSPACE}/.github/jobs/get_test_truth_data.sh ${TRUTH_DATA_VERSION}

# Set up directories to mount
LOCAL_OUTPUT_DIR=${RUNNER_WORKSPACE}/output
DOCKER_OUTPUT_DIR=${MET_TEST_OUTPUT}

LOCAL_DIFF_DIR=${RUNNER_WORKSPACE}/diff
DOCKER_DIFF_DIR=${MET_TEST_DIFF}

LOCAL_LOG_DIR=${RUNNER_WORKSPACE}/logs
DOCKER_LOG_DIR=/met/logs

LOCAL_REPO_DIR=${GITHUB_WORKSPACE}
DOCKER_REPO_DIR=/met/MET

export LOCAL_METPLUS_DIR=${RUNNER_WORKSPACE}/metplus
DOCKER_METPLUS_DIR=${METPLUS_DIR}

# Create local directories to store output & for cloning METplus into
mkdir -p ${LOCAL_LOG_DIR}
mkdir -p ${LOCAL_DIFF_DIR}
mkdir -p ${LOCAL_METPLUS_DIR}

# METplus branch to use for diff testing script
METPLUS_BRANCH=develop

# Clone METplus into runner
time_command git clone --single-branch --branch ${METPLUS_BRANCH} https://github.com/dtcenter/METplus ${LOCAL_METPLUS_DIR}

# Docker image to use for running diffs
export VERSION_EXT=$(${GITHUB_WORKSPACE}/.github/jobs/get_diff_docker_version.py)
DOCKERHUB_TAG=dtcenter/metplus-envs:diff${VERSION_EXT}

# args to mount output and log dirs, mount GitHub files into MET_REPO_DIR
mount_args="-v ${LOCAL_OUTPUT_DIR}:${DOCKER_OUTPUT_DIR} -v ${LOCAL_DIFF_DIR}:${DOCKER_DIFF_DIR} -v ${LOCAL_LOG_DIR}:${DOCKER_LOG_DIR}"
mount_args_repos="-v ${LOCAL_REPO_DIR}:${DOCKER_REPO_DIR} -v ${LOCAL_METPLUS_DIR}:${DOCKER_METPLUS_DIR}"

# Set up data volumes
volumes_from="--volumes-from met_truth"

# set MET_REPO_DIR env var in Docker to mounted directory & pass other necessary env vars
env_arg="-e MET_REPO_DIR=${DOCKER_REPO_DIR} -e VERSION_EXT"

# run unit test script inside Docker, mount MET output and truth data
cmd="\${MET_REPO_DIR}/.github/jobs/run_diff_tests.sh"
time_command docker run ${volumes_from} ${mount_args} ${mount_args_repos} ${env_arg} ${DOCKERHUB_TAG} bash -c \"${cmd}\"
if [ $? != 0 ]; then
  exit 1
fi

if [ "$(ls -A ${LOCAL_DIFF_DIR})" ]; then
  echo "ERROR: Differences exist in the output"

  # only exit non-zero (job fails) if not updating truth data
  # this makes difference output available when updating truth data
  # so it is easier to see what changed with the update
  if [ "${RUN_UPDATE_TRUTH}" != "true" ]; then
    exit 1
  fi

fi
