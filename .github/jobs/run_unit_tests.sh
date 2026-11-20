#! /bin/bash

source ${MET_REPO_DIR}/.github/jobs/bash_functions.sh

###
# Set environment variables needed to run unit tests
###

source ${MET_REPO_DIR}/.github/jobs/test_env_vars.sh

###
# Run MET unit tests
###

UNIT_OPTS=""
if [[ "${UNIT_TEST_VERBOSE:-false}" == "true" ]]; then
  UNIT_OPTS="-verbose"
  echo "Running unit tests verbosely; the log files will be much larger than usual."
fi

echo "Running MET unit tests with OMP_NUM_THREADS = ${OMP_NUM_THREADS} ..."
for testname in $TESTS_TO_RUN; do
  # A verbose log can run to many thousands of lines, so print only the end of
  # it after a failure and leave the rest to the uploaded logs artifact.
  LOG_OPTS=(-l "/met/logs/unit_${testname}.log")
  if [[ "${UNIT_TEST_VERBOSE:-false}" == "true" ]]; then
    LOG_OPTS+=(-t 500)
  fi

  if ! time_command "${LOG_OPTS[@]}" ${MET_TEST_BASE}/python/unit.py ${UNIT_OPTS} ${MET_TEST_BASE}/xml/unit_${testname}.xml; then
      echo "ERROR: Unit test ${testname} failed"
      exit 1
  fi
done

echo "Success"
