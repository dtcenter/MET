#!/bin/bash

# check for environment variables, use defaults if necessary
MET_TEST_BASE=${MET_TEST_BASE:-/d3/projects/MET/MET_development/svn-met-dev.cgd.ucar.edu/trunk/test}
MET_TEST_RSCRIPT=${MET_TEST_RSCRIPT:-/usr/local/bin/Rscript}
MGNC=$MET_TEST_BASE/R_test/mgnc.R

# get environment settings
. ${MET_TEST_BASE}/bin/set_env.sh

$MET_TEST_RSCRIPT $MGNC $@ 2>&1

