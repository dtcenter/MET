#! /bin/bash

run_compile=true
run_push=false
run_unit_tests=false
run_diff=false
run_update_truth=false
met_base_repo=met-base
met_base_tag=v1.2
input_data_version=develop
truth_data_version=develop

if [ "${GITHUB_EVENT_NAME}" == "pull_request" ]; then
    
  # only run diff logic if pull request INTO 
  # branches not ending with -ref
  if [ "${GITHUB_BASE_REF: -4}" != "-ref" ]; then

    run_diff=true

    # if pull request into main_vX.Y branch,
    # set truth data version to branch name and set input data version to X.Y
    if [ "${GITHUB_BASE_REF:0:6}" == "main_v" ]; then
      truth_data_version=${GITHUB_BASE_REF}
      input_data_version=${GITHUB_BASE_REF:6}
    fi

  fi

elif [ "${GITHUB_EVENT_NAME}" == "push" ]; then

  branch_name=`cut -d "/" -f3 <<< "${GITHUB_REF}"`

  # if branch ends with -ref, update truth data from unit tests
  if [ "${branch_name: -4}" == -ref ]; then

    run_update_truth=true
    run_diff=true
    truth_data_version=${branch_name:0: -4}

    # if main_vX.Y-ref branch, use X.Y input data
    if [ "${branch_name:0:6}" == "main_v" ]; then
      input_data_version=${branch_name:6: -4}
    fi

  else

    # if develop or main_vX.Y branch, run diff tests using branch's truth data
    if [ "$branch_name" == "develop" ] ||
       [ "${branch_name:0:6}" == "main_v" ]; then

      run_diff=true
      truth_data_version=${branch_name}

      # if main_vX.Y branch, use X.Y input data
      if [ "${branch_name:0:6}" == "main_v" ]; then
        input_data_version=${branch_name:6}
      fi

    fi

    # check commit messages for skip or force keywords                                                                         
    if grep -q "ci-skip-all" <<< "$commit_msg"; then

      run_compile=false
      run_push=false
      run_unit_tests=false
      run_diff=false
      run_update_truth=false

    fi    
    
    # check commit messages for ci-skip or ci-run keywords
    if grep -q "ci-skip-compile" <<< "$commit_msg"; then

      run_compile=false

    fi

    if grep -q "ci-run-unit" <<< "$commit_msg"; then

      run_diff=true

    fi
    
  fi

elif [ "${GITHUB_EVENT_NAME}" == "workflow_dispatch" ]; then

  if [ "${force_tests}" == "true" ]; then
    run_diff=true
  fi

fi

# if updating truth or running diff, run unit tests
if [ "$run_update_truth" == "true" ] || [ "$run_diff" == "true" ]; then

  run_unit_tests=true

fi

# if running unit tests, use unit_test MET base image and push image
if [ "$run_unit_tests" == "true" ]; then

  met_base_repo=met-base-unit-test
  run_push=true

fi

echo "run_compile=${run_compile}" >> $GITHUB_OUTPUT
echo "run_push=${run_push}" >> $GITHUB_OUTPUT
echo "run_unit_tests=${run_unit_tests}" >> $GITHUB_OUTPUT
echo "run_diff=${run_diff}" >> $GITHUB_OUTPUT
echo "run_update_truth=${run_update_truth}" >> $GITHUB_OUTPUT
echo "met_base_repo=${met_base_repo}" >> $GITHUB_OUTPUT
echo "met_base_tag=${met_base_tag}" >> $GITHUB_OUTPUT
echo "input_data_version=${input_data_version}" >> $GITHUB_OUTPUT
echo "truth_data_version=${truth_data_version}" >> $GITHUB_OUTPUT

echo run_compile: $run_compile
echo run_push: $run_push
echo run_unit_tests: $run_unit_tests
echo run_diff: $run_diff
echo run_update_truth: $run_update_truth
echo met_base_repo: $met_base_repo
echo met_base_tag: $met_base_tag
echo input_data_version: $input_data_version
echo truth_data_version: $truth_data_version

# get name of branch
.github/jobs/get_branch_name.sh
