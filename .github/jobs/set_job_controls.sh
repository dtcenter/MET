#! /bin/bash

run_compile=true
run_push=false
run_unit_tests=false
run_diff=false
run_update_truth=false
#run_unit_tests=true
#run_diff=true
#run_update_truth=true
met_base_image=minimum
dockerhub_repo=dtcenter/met-dev

if [ "${GITHUB_EVENT_NAME}" == "pull_request" ]; then

  # only run diff logic if pull request INTO
  # branches not ending with -ref
  if [ "${GITHUB_BASE_REF: -4}" != "-ref" ]; then

    run_unit_tests=true
    run_diff=true

  fi

elif [ "${GITHUB_EVENT_NAME}" == "push" ]; then

  # check commit messages for skip or force keywords
  if grep -q "ci-skip-compile" <<< "$commit_msg"; then
    run_compile=false
  fi

  if grep -q "ci-run-unit" <<< "$commit_msg"; then
    run_unit_tests=true
    run_diff=true
  fi

  branch_name=`cut -d "/" -f3 <<< "${GITHUB_REF}"`

  # if branch ends with -ref, update truth data from unit tests
  if [ "${branch_name: -4}" == -ref ]; then

    run_compile=true
    run_push=true
    run_unit_tests=false
    run_diff=false
    run_update_truth=true

  # if develop branch, push Docker image to dtcenter/met
  # set all options to prevent commit override
  elif [ "$branch_name" == "develop" ]; then
     # || [ "${branch_name:0:6}" == "main_v" ]; then
    run_compile=true
    run_push=true
    run_unit_tests=false
    run_diff=false
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
echo ::set-output name=run_diff::$run_diff
echo ::set-output name=run_update_truth::$run_update_truth
echo ::set-output name=met_base_image::$met_base_image
echo ::set-output name=dockerhub_repo::$dockerhub_repo

echo run_compile: $run_compile
echo run_push: $run_push
echo run_unit_tests: $run_unit_tests
echo met_base_image: $met_base_image
echo dockerhub_repo: $dockerhub_repo

# get name of branch
.github/jobs/get_branch_name.sh
