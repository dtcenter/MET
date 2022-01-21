#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

DIFF_DIR=/data/output/met_test_diff

volumes_from="--volumes-from met_input --volumes-from met_truth"

mount_args="-v ${RUNNER_WORKSPACE}/diff:${DIFF_DIR} -v ${RUNNER_WORKSPACE}/logs:/met/logs"

# run unit test script inside Docker, mount MET input and truth data
cmd="\${MET_REPO_DIR}/.github/jobs/run_unit_tests.sh ${TESTS}"
time_command docker run ${volumes_from} ${mount_args} ${DOCKERHUB_TAG} bash -c \"${cmd}\"

# exit non-zero if there are any diff files
if [ "$(ls -A $DIFF_DIR)" ]; then
  exit 1
fi
