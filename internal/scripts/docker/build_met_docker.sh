#! /bin/bash

echo "Running script to build MET in Docker"

source internal/scripts/environment/development.docker

export SKIP_LIBS=yes
export USE_MET_TAR_FILE=FALSE

LOG_FILE=/met/MET-${MET_GIT_NAME}/compile_MET_all.log
echo "Compiling MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
./internal/scripts/installation/compile_MET_all.sh > ${LOG_FILE}
if [ $? != 0 ]; then
  echo "ERROR: Compilation failed"
  exit 1
fi

if [[ $MET_GIT_NAME == "v"* ]]; then
    cd /met; rm -rf MET-*;
fi
