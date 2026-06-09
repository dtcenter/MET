#!/bin/bash

# check for environment variables, use defaults if necessary
export MET_TEST_BASE=${MET_TEST_BASE}
[ -z $MET_PYTHON_BIN_EXE ] && MET_PYTHON_BIN_EXE=python3
MGNC=$MET_TEST_BASE/python/mgnc.py

# get environment settings
. ${MET_TEST_BASE}/bin/set_env.sh

$MET_PYTHON_BIN_EXE $MGNC $@ 2>&1
