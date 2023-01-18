#! /bin/bash

echo "Running script to build MET in Docker"

source internal/scripts/environment/development.docker

mkdir -p /met/logs

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_configure.log
echo "Configuring MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
./bootstrap
./configure --enable-grib2 --enable-mode_graphics --enable-modis --enable-lidar2nc --enable-python CPPFLAGS=-I/met/external_libs/include:/met/external_libs/include/freetype2 > ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
  cat ${LOG_FILE}
  exit 1
fi

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_install.log
echo "Compiling MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make install > ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
  cat ${LOG_FILE}
  exit 1
fi

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_test.log
echo "Testing MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make test > ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
  cat ${LOG_FILE}
  exit 1
fi

if [[ $MET_GIT_NAME == "v"* ]]; then
    cd /met; rm -rf MET-*;
fi
