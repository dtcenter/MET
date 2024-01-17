#!/bin/bash

# wget https://www.python.org/ftp/python/3.10.13/Python-3.10.13.tgz
# tar -zxf Python-3.10.13.tgz

export FC=ifort
export F77=ifort
export F90=ifort
export CC=icc
export CXX=icpc

DST=/contrib/met/12.0.0-beta2

# Python (minor) Version
PV=10_intel

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
    --without-gcc \
    --with-libm=-limf \
    --with-cxx-main=`which icpc` \
    --enable-ipv6 \
    --with-signal-module \
    --with-openssl=/contrib/met/12.0.0-beta2/external_libs/intel \
    --with-openssl-rpath=/contrib/met/12.0.0-beta2/external_libs/intel/lib \
    CC=icc CXX=icpc LD=xild AR=xiar \
    LIBS="-lpthread -limf -lirc" \
    CFLAGS="-O3 -fp-model strict -fp-model source -xHost -ipo -prec-div -prec-sqrt" \
    LDFLAGS="-ipo" \
    CPPFLAGS="" \
    CPP="icc -E" 2>&1 | tee configure-${OS}-`date +%Y%m%d`-${TRY}

# NOTE
# ...
# configure: WARNING: unrecognized options: --with-threads, --with-fpectl

echo "Running make"
make 2>&1 |tee make-${OS}-`date +%Y%m%d`-${TRY}

echo "Running make check"
make check 2>&1 |tee make-check-${OS}-`date +%Y%m%d`-${TRY}

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
