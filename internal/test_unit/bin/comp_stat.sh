#!/bin/bash

# check for environment variables, use defaults if necessary
MET_TEST_BASE=${MET_TEST_BASE}
MET_TEST_RSCRIPT=${MET_TEST_RSCRIPT:-/nrit/ral/bin/Rscript}
COMP_SCRIPT=$MET_TEST_BASE/R_test/comp_stat.R

# get environment settings
. ${MET_TEST_BASE}/bin/set_env.sh

$MET_TEST_RSCRIPT $COMP_SCRIPT $@ 2>&1

