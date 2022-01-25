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
time_command ${MET_TEST_BASE}/bin/comp_dir.sh ${MET_TEST_TRUTH} ${MET_TEST_OUTPUT}
if [ $? != 0 ]; then
    echo "ERROR: Test output comparison failed"
    cat /met/logs/comp_dir.log
    exit 1
fi

echo "Running copy_diff_files.py"
CMD_LOGFILE=/met/logs/copy_diff_files.log
time_command ${MET_REPO_DIR}/.github/jobs/copy_diff_files.py
if [ $? != 0 ]; then
    echo "ERROR: Copy diff files script failed"
    cat /met/logs/copy_diff_files.log
    exit 1
fi

echo "Success"
