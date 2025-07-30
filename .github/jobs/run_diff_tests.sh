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
CMD_LOGFILE=/met/logs/comp_dir.log
if ! time_command ${MET_TEST_BASE}/bin/comp_dir.sh ${MET_TEST_TRUTH} ${MET_TEST_OUTPUT}; then
  echo "::group::ERROR: Test output comparison failed"
  cat /met/logs/comp_dir.log
  echo "::endgroup::"
  exit 1
fi

echo "Running copy_diff_files.py"
CMD_LOGFILE=/met/logs/copy_diff_files.log
if ! time_command ${MET_REPO_DIR}/.github/jobs/copy_diff_files.py; then
  echo "::group::ERROR: Copy diff files script failed"
  cat /met/logs/copy_diff_files.log
  echo "::endgroup::"
  exit 1
fi

echo "Success"
