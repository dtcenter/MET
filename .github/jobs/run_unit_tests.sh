#! /bin/bash

source ${MET_REPO_DIR}/.github/jobs/bash_functions.sh

###
# Set environment variables needed to run unit tests
###

source ${MET_REPO_DIR}/.github/jobs/test_env_vars.sh

###
# Run MET unit tests
###

echo "Running MET unit tests..."
for testname in $TESTS_TO_RUN; do
  CMD_LOGFILE=/met/logs/unit_${testname}.log
  time_command ${MET_TEST_BASE}/perl/unit.pl ${MET_TEST_BASE}/xml/unit_${testname}.xml
  if [ $? != 0 ]; then
      echo "ERROR: Unit test ${testname} failed"
      cat /met/logs/unit_${testname}.log
      exit 1
  fi
done

echo "Success"
