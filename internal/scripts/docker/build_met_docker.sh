#! /bin/bash

echo "Running script to build MET in Docker"

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_configure.log

echo "Configuring MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
./bootstrap
./configure --enable-grib2 --enable-mode_graphics --enable-modis --enable-lidar2nc --enable-python \
    MET_HDF=/usr/local/hdf MET_HDFEOS=/usr/local/hdfeos \
    MET_FREETYPEINC=/usr/include/freetype2 MET_FREETYPELIB=/usr/lib \
    MET_CAIROINC=/usr/include/cairo MET_CAIROLIB=/usr/lib \
    MET_PYTHON_CC='-I/usr/include/python3.6m' MET_PYTHON_LD='-lpython3.6m' > ${LOG_FILE}
if [ $? != 0 ]; then
    exit 1
fi

LOG_FILE=/met/MET-${MET_GIT_NAME}/make_install.log
echo "Compiling MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make install > ${LOG_FILE}
if [ $? != 0 ]; then
    exit 1
fi

LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_test.log
echo "Testing MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}"
make test > ${LOG_FILE} 2>&1
if [ $? != 0 ]; then
    exit 1
fi

if [[ $MET_GIT_NAME == "v"* ]]; then
    cd /met; rm -rf MET-*;
fi
