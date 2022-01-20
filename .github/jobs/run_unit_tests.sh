#! /bin/bash

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

export MET_TEST_RSCRIPT=/usr/bin/Rscript
export MET_TEST_MET_PYTHON_EXE=/usr/bin/python3

###
# Run MET unit tests
###

echo "Running all MET unit tests..."
echo "Writing logs to /met/unit_test.log"
${MET_TEST_BASE}/bin/unit_test.sh > /met/unit_test.log
if [ $? != 0 ]; then
    echo "ERROR: Unit tests failed"
    exit 1
fi


echo "Running comparison on test output"
echo "Writing logs to /met/comp_dir.log"
${MET_TEST_BASE}/bin/comp_dir.sh ${MET_TEST_TRUTH} ${MET_TEST_OUTPUT} > /met/comp_dir.log
if [ $? != 0 ]; then
    echo "ERROR: Test output comparison failed"
    exit 1
fi

echo "Success"
