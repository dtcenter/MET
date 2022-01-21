#! /bin/bash

run_compile=true
run_push=false
run_unit_tests=false
met_base_image=minimum
dockerhub_repo=dtcenter/met-dev

if [ "${GITHUB_EVENT_NAME}" == "pull_request" ]; then

  run_unit_tests=true

elif [ "${GITHUB_EVENT_NAME}" == "push" ]; then

  # check commit messages for skip or force keywords
  if grep -q "ci-skip-compile" <<< "$commit_msg"; then
    run_compile=false
  fi

  if grep -q "ci-run-unit" <<< "$commit_msg"; then
    run_unit_tests=true
  fi

  # if develop branch, push Docker image to dtcenter/met
  # set all options to prevent commit override
  branch_name=`cut -d "/" -f3 <<< "${GITHUB_REF}"`
  if [ "$branch_name" == "develop" ]; then
     # || [ "${branch_name:0:6}" == "main_v" ]; then
    run_compile=true
    run_push=true
    run_unit_tests=false
    met_base_image=minimum
    dockerhub_repo=dtcenter/met
  fi

fi

# use unit_test MET base image if running unit tests
if [ "$run_unit_tests" == "true" ]; then

  met_base_image=unit_test
  run_push=true

fi

echo ::set-output name=run_compile::$run_compile
echo ::set-output name=run_push::$run_push
echo ::set-output name=run_unit_tests::$run_unit_tests
echo ::set-output name=met_base_image::$met_base_image
echo ::set-output name=dockerhub_repo::$dockerhub_repo

# get name of branch
.github/jobs/get_branch_name.sh
