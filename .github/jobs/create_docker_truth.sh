#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

image_name=dtcenter/met-data-output:${TRUTH_DATA_VERSION}

time_command docker build -t ${image_name} \
       --build-arg TRUTH_DIR=met_test_truth \
       -f ${GITHUB_WORKSPACE}/.github/jobs/Dockerfile.truth \
       ${RUNNER_WORKSPACE}
if [ $? != 0 ]; then
  echo "ERROR: Docker build failed"
  exit 1
fi

time_command docker push ${image_name}
