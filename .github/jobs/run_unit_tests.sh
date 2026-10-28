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
  CMD_LOGFILE=/met/logs/unit_${testname}.log
  time_command ${MET_TEST_BASE}/python/unit.py ${UNIT_OPTS} ${MET_TEST_BASE}/xml/unit_${testname}.xml
  if [[ $? != 0 ]]; then
      echo "ERROR: Unit test ${testname} failed"

      # A verbose log can run to many thousands of lines, so print only the
      # end of it here and leave the rest to the uploaded logs artifact.
      if [[ -n "${UNIT_OPTS}" ]]; then
        echo "Showing the last 500 lines; see the logs artifact for the rest."
        tail -n 500 /met/logs/unit_${testname}.log
      else
        cat /met/logs/unit_${testname}.log
      fi
      exit 1
  fi
done

echo "Success"
