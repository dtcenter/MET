#! /bin/bash

echo "Running script to build MET in Docker"

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_configure.log

source internal/scripts/environment/development.docker

echo "Configuring MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
./bootstrap
./configure --enable-grib2 --enable-mode_graphics --enable-modis --enable-lidar2nc --enable-python > ${LOG_FILE}
if [ $? != 0 ]; then
    exit 1
fi

LOG_FILE=/met/MET-${MET_GIT_NAME}/make_install.log
echo "Compiling MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make -j install > ${LOG_FILE}
if [ $? != 0 ]; then
    exit 1
fi

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_test.log
echo "Testing MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make -j test > ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
    exit 1
fi

if [[ $MET_GIT_NAME == "v"* ]]; then
    cd /met; rm -rf MET-*;
fi
