#!/bin/bash

# check for environment variables, use defaults if necessary
export MET_TEST_BASE=${MET_TEST_BASE}
[[ -z $MET_TEST_MET_PYTHON_EXE ]] && MET_TEST_MET_PYTHON_EXE=python3
MPNC=$MET_TEST_BASE/python/mpnc.py

# get environment settings
. ${MET_TEST_BASE}/bin/set_env.sh

$MET_TEST_MET_PYTHON_EXE $MPNC $@ 2>&1
