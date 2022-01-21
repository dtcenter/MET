#! /bin/bash

source ${MET_REPO_DIR}/.github/jobs/bash_functions.sh

###
# Set environment variables needed to run unit tests
###

export MET_BASE=/usr/local/share/met

export MET_BUILD_BASE=${MET_REPO_DIR}/met
export MET_TEST_BASE=${MET_REPO_DIR}/test
export PERL5LIB=${MET_TEST_BASE}/lib

export MET_TEST_INPUT=/data/input/MET_test_data/unit_test
export MET_TEST_OUTPUT=/data/output/met_test_output
export MET_TEST_TRUTH=/data/output/met_test_truth
export MET_TEST_DIFF=/data/output/met_test_diff

export MET_TEST_RSCRIPT=/usr/bin/Rscript
export MET_TEST_MET_PYTHON_EXE=/usr/bin/python3

###
# Run MET unit tests
###

echo "Running MET unit tests..."
echo "TESTS_TO_RUN is X${TESTS_TO_RUN}X"
for testname in $TESTS_TO_RUN; do
  CMD_LOGFILE=/met/logs/unit_${testname}.log
  time_command ${MET_TEST_BASE}/perl/unit.pl ${MET_TEST_BASE}/xml/unit_${testname}.xml
  if [ $? != 0 ]; then
      echo "ERROR: Unit test ${testname} failed"
      exit 1
  fi
done


echo "Running comparison on test output"
for testname in $TESTS_TO_RUN; do
  CMD_LOGFILE=/met/logs/comp_dir_${testname}.log
  time_command ${MET_TEST_BASE}/bin/comp_dir.sh ${MET_TEST_TRUTH}/${testname} ${MET_TEST_OUTPUT}/${testname}
  if [ $? != 0 ]; then
      echo "ERROR: Test ${testname} output comparison failed"
      exit 1
  fi
done

echo "Running copy_diff_files.py"
CMD_LOGFILE=/met/logs/copy_diff_files.log
time_command ${MET_REPO_DIR}/.github/jobs/copy_diff_files.py
if [ $? != 0 ]; then
    echo "ERROR: Copy diff files script failed"
    exit 1
fi


echo "Success"
