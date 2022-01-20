#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=dtcenter/met:${SOURCE_BRANCH}

volumes_from="--volumes-from met_input --volumes-from met_truth"

mount_args="-v ${RUNNER_WORKSPACE}/diff:/data/output/met_data_diff -v ${RUNNER_WORKSPACE}/logs:/met/logs"

# run unit test script inside Docker, mount MET input and truth data
cmd="\${MET_REPO_DIR}/.github/jobs/run_unit_tests.sh"
time_command docker run ${volumes_from} ${mount_args} ${DOCKERHUB_TAG} bash -c \"${cmd}\"
