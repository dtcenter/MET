#! /bin/bash

source ${MET_REPO_DIR}/.github/jobs/bash_functions.sh

###
# Set environment variables needed to run unit tests
###

source ${MET_REPO_DIR}/.github/jobs/test_env_vars.sh

###
# Run comparison of MET unit test output
###

echo "Running comparison on test output"
for testname in $TESTS_TO_RUN; do
  CMD_LOGFILE=/met/logs/comp_dir_${testname}.log
  time_command ${MET_TEST_BASE}/bin/comp_dir.sh ${MET_TEST_TRUTH}/${testname} ${MET_TEST_OUTPUT}/${testname}
  if [ $? != 0 ]; then
      echo "ERROR: Test ${testname} output comparison failed"
      exit 1
  fi
done

test_list=($TESTS_TO_RUN)
first_test=${test_list[0]}

echo "Running copy_diff_files.py"
CMD_LOGFILE=/met/logs/copy_diff_files_${first_test}.log
time_command ${MET_REPO_DIR}/.github/jobs/copy_diff_files.py
if [ $? != 0 ]; then
    echo "ERROR: Copy diff files script failed"
    exit 1
fi

echo "Success"
