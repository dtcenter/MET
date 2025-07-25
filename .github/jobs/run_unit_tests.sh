#! /bin/bash

source ${MET_REPO_DIR}/.github/jobs/bash_functions.sh

###
# Set environment variables needed to run unit tests
###

source ${MET_REPO_DIR}/.github/jobs/test_env_vars.sh

###
# Run MET unit tests
###

echo "Running MET unit tests with OMP_NUM_THREADS = ${OMP_NUM_THREADS} ..."
for testname in $TESTS_TO_RUN; do
  CMD_LOGFILE=/met/logs/unit_${testname}.log
  if ! time_command ${MET_TEST_BASE}/python/unit.py ${MET_TEST_BASE}/xml/unit_${testname}.xml; then
    echo "::group::ERROR: Unit test ${testname} failed"
    cat /met/logs/unit_${testname}.log
    echo "::endgroup::"
    exit 1
  fi
done

echo "Success"
