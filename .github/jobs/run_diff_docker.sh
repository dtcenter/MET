#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=${DOCKERHUB_REPO}:${SOURCE_BRANCH}

LOCAL_DIFF_DIR=${RUNNER_WORKSPACE}/diff
DOCKER_DIFF_DIR=/data/output/met_test_diff

volumes_from="--volumes-from met_truth"

mount_args="-v $LOCAL_DIFF_DIR:$DOCKER_DIFF_DIR -v ${RUNNER_WORKSPACE}/logs:/met/logs"

export TESTS_TO_RUN=$TESTS

# run unit test script inside Docker, mount MET input and truth data
cmd="\${MET_REPO_DIR}/.github/jobs/run_diff_tests.sh"
#time_command docker run --name=run_unit -e TESTS_TO_RUN ${volumes_from} ${mount_args} ${DOCKERHUB_TAG} bash -c \"${cmd}\"
time_command docker exec -e TESTS_TO_RUN run_unit bash -c \"${cmd}\"
if [ $? != 0 ]; then
  exit 1
fi

# exit non-zero if there are any diff files
if [ "$(ls -A $LOCAL_DIFF_DIR)" ]; then
  echo "ERROR: Differences exist in the output"
  exit 1
fi
