#!/bin/bash

# check for environment variables, use defaults if necessary
export MET_TEST_BASE=${MET_TEST_BASE}
[[ -z $MET_TEST_MET_PYTHON_EXE ]] && MET_TEST_MET_PYTHON_EXE=python3
MGNC=$MET_TEST_BASE/python/mgnc.py

# get environment settings
. ${MET_TEST_BASE}/bin/set_env.sh

$MET_TEST_MET_PYTHON_EXE $MGNC $@ 2>&1
