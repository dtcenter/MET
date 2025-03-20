#!/bin/bash

# check for environment variables, use defaults if necessary
MET_TEST_BASE=${MET_TEST_BASE}
COMP_SCRIPT=$MET_TEST_BASE/python/comp_dir.py

# get environment settings
. ${MET_TEST_BASE}/bin/set_env.sh

$COMP_SCRIPT $@ 2>&1
