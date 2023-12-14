#! /bin/bash

echo "Running script to build MET in Docker"

source internal/scripts/environment/development.docker

# Check whether MET_GIT_NAME is defined
if [ -z ${MET_GIT_NAME+x} ]; then
  MET_GIT_NAME=`git name-rev --name-only HEAD`
  echo "Setting MET_GIT_NAME=${MET_GIT_NAME} based on the current branch."
fi

# Check whether MET_ENABLE_OPTS is defined
if [ -z ${MET_ENABLE_OPTS+x} ]; then
  MET_ENABLE_OPTS='--enable-all'
  echo "Setting MET_ENABLE_OPTS=${MET_ENABLE_OPTS} to compile all available options."
fi

# Create log directory
mkdir -p /met/logs

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_configure.log
echo "Running bootstrap for MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
./bootstrap > ${LOG_FILE} 2>&1
echo "Configuring MET ${MET_GIT_NAME} and appending to log file ${LOG_FILE}"
./configure \
  BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} \
  ${MET_ENABLE_OPTS} \
  CPPFLAGS="-I/usr/local/include -I/usr/local/include/freetype2 -I/usr/local/include/cairo" \
  LIBS="-ltirpc" >> ${LOG_FILE} 2>&1
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
