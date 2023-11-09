#! /bin/bash

echo "Running script to build MET in Docker"

source internal/scripts/environment/development.docker

mkdir -p /met/logs

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_configure.log
echo "Running bootstrap for MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
./bootstrap > ${LOG_FILE} 2>&1
echo "Configuring MET ${MET_GIT_NAME} and appending to log file ${LOG_FILE}"
./configure --enable-grib2 --enable-mode_graphics --enable-modis --enable-lidar2nc --enable-python CPPFLAGS="-I/usr/local/include -I/usr/local/include/freetype2 -I/usr/local/include/cairo" LIBS="-ltirpc" >> ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
  cat ${LOG_FILE}
  exit 1
fi

if [ ! -z "${MAKE_ARGS}" ]; then
  echo Adding make arguments: ${MAKE_ARGS}
fi

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_install.log
echo "Compiling MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make ${MAKE_ARGS} install > ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
  cat ${LOG_FILE}
  exit 1
fi

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_test.log
echo "Testing MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make ${MAKE_ARGS} test > ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
  cat ${LOG_FILE}
  exit 1
fi

if [[ $MET_GIT_NAME == "v"* ]]; then
    cd /met; rm -rf MET-*;
fi
