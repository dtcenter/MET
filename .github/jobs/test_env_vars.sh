export MET_BASE=/usr/local/share/met

export MET_TEST_BASE=${MET_REPO_DIR}/internal/test_unit
export PERL5LIB=${MET_TEST_BASE}/lib

export MET_TEST_INPUT=/data/input/MET_test_data/unit_test
export MET_TEST_OUTPUT=/data/output/met_test_output
export MET_TEST_TRUTH=/data/output/met_test_truth
export MET_TEST_DIFF=/data/output/met_test_diff

export MET_TEST_RSCRIPT=/usr/bin/Rscript
export MET_TEST_MET_PYTHON_EXE=/usr/local/bin/python3

# Use 4 threads for GitHub action runs
export OMP_NUM_THREADS=4
