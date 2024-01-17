#!/bin/bash

# tar -zxf OpenSSL_1_1_1w.tar.gz
# cd openssl-OpenSSL_1_1_1w/
# ./config --prefix=/contrib/met/12.0.0-beta2/external_libs --openssldir=/contrib/met/12.0.0-beta2/external_libs --libdir=/contrib/met/12.0.0-beta2/external_libs/lib
# make > openssl.make.log 2>&1
# make install > openssl.make_install.log 2>&1
# Run make clean if reinstallation is needed.

# wget https://www.python.org/ftp/python/3.10.4/Python-3.10.4.tgz
# tar -zxf Python-3.10.4.tgz
# mv Python-3.10.4 Python-3.10.4_gnu
# module load gnu/9.2.0

export FC=gfortran
export F77=gfortran
export F90=gfortran
export CC=gcc
export CXX=g++

DST=/contrib/met/12.0.0-beta2

# Python (minor) Version
PV=10_gnu

# For the RPATH stuff
export LDFLAGS="-Wl,--disable-new-dtags,-rpath,${DST}/met-python3.${PV}/lib:${DST}/lib:'\$\$ORIGIN'/../lib"
export LDFLAGS="$LDFLAGS -L${DST}/met-python3.${PV}/lib -L${DST}/lib"

TRY=1

./configure --prefix=${DST}/met-python3.${PV} \
    --enable-shared \
    --enable-optimizations \
    --with-dbmliborder=bdb:gdbm \
    --enable-loadable-sqlite-extensions \
    --with-system-expat \
    --with-computed-gotos \
    --with-system-ffi \
    --disable-ipv6 \
    --with-openssl=/contrib/met/12.0.0-beta2/external_libs  \
    --with-openssl-rpath=/contrib/met/12.0.0-beta2/external_libs/lib 2>&1 | tee configure-${OS}-`date +%Y%m%d`-${TRY}

# NOTE
# ...
# configure: WARNING: unrecognized options: --with-threads, --with-fpectl

echo "Running make"
make 2>&1 |tee make-${OS}-`date +%Y%m%d`-${TRY}

# echo "Running make check"
# make check 2>&1 |tee make-check-${OS}-`date +%Y%m%d`-${TRY}
# Doesn't work: "make: *** No rule to make target `check'.  Stop."

echo "Running install"
make install 2>&1 |tee make-install-${OS}-`date +%Y%m%d`-${TRY}

echo "Moving config.log"
mv config.log config.log-${OS}-`date +%Y%m%d`

echo "Running make distclean"
make distclean 2>&1 |tee make-distclean-${OS}-`date +%Y%m%d`-${TRY}

PATH=${DST}/met-python3.${PV}/bin:$PATH

# FIRST!!! See if pip needs to be upgraded. If it needs to be, it is *crucial*
pip3 install --upgrade pip 2>&1 |tee pip-upgrade-${OS}-`date +%Y%m%d`-${TRY}

PIPOPTS="-m pip"

# XARRAY
${DST}/met-python3.${PV}/bin/python3 ${PIPOPTS} install ${BLDOPTS} xarray 2>&1 |tee make-xarray-${OS}-`date +%Y%m%d`-${TRY}

# netCDF4

export HDF5_DIR=${DST}/external_libs
export NETCDF4_DIR=${DST}/external_libs

${DST}/met-python3.${PV}/bin/python3 ${PIPOPTS} install ${BLDOPTS} netCDF4 2>&1 |tee make-netCDF4-${OS}-`date +%Y%m%d`-${TRY}
