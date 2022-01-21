#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

volumes_from="--volumes-from met_input"

mount_args="-v ${RUNNER_WORKSPACE}/logs:/met/logs"

export TESTS_TO_RUN=$TESTS

# run unit test script inside Docker, mount MET input and truth data
cmd="\${MET_REPO_DIR}/.github/jobs/run_unit_tests.sh"
time_command docker run -e TESTS_TO_RUN ${volumes_from} ${mount_args} ${DOCKERHUB_TAG} bash -c \"${cmd}\"
if [ $? != 0 ]; then
  exit 1
fi
