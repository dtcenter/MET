#! /bin/bash

source ${MET_REPO_DIR}/.github/jobs/bash_functions.sh

###
# Set environment variables needed to run unit tests
###

source ${MET_REPO_DIR}/.github/jobs/test_env_vars.sh

###
# Run comparison of MET unit test output (including saving diff files)
###

echo "Running comparison on test output"
COMP_DIR_LOGFILE=/met/logs/comp_dir.log
ENV_PYTHON=/usr/local/conda/envs/diff${VERSION_EXT}/bin/python3
if ! time_command -l "${COMP_DIR_LOGFILE}" ${ENV_PYTHON} ${MET_TEST_BASE}/python/comp_dir.py ${MET_TEST_TRUTH} ${MET_TEST_OUTPUT} -d ${MET_TEST_DIFF}; then
    echo "ERROR: Test output comparison failed"
    exit 1
fi

echo "Success"
