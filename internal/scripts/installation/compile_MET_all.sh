#!/bin/bash
#
# Compile and install MET
# (Model Evaluation Tools)
#================================================
#
# This compile_MET_all.sh script expects certain environment
# variables to be set:
# TEST_BASE, COMPILER, MET_SUBDIR, MET_TARBALL,
# and USE_MODULES.
#
# If compiling support for Python embedding, users will need to
# set MET_PYTHON, MET_PYTHON_CC, and
# MET_PYTHON_LD.
#
# For a description of these and other variables, visit the MET
# downloads page under "Sample Script For Compiling External
# Libraries And MET":
# https://dtcenter.org/community-code/model-evaluation-tools-met/download
#
# An easy way to set these necessary environment variables is
# in an environment configuration file (for example,
# install_met_env.<machine_name>).  This script and example
# environment config files for various machines can be found in
# the MET GitHub repository in the scripts/installation directory:
# https://github.com/dtcenter/MET
#
# USAGE: compile_MET_all.sh install_met_env.<machine_name>
#
# The compile_MET_all.sh script will compile and install MET and its
# external library dependencies, if needed, including:
# GSL, BUFRLIB, GRIB2C (with dependencies Z, PNG, JASPER),
# HDF5, NETCDF (C and CXX), HDF4 (optional for MODIS-Regrid
# and lidar2nc), HDFEOS (optional for MODIS-Regrid and lidar2nc),
# FREETYPE (optional for MODE Graphics), and CAIRO (optional
# for MODE Graphics).
#
# If these libraries have already been installed and don't need to be
# reinstalled or if you are compiling on a machine that uses modulefiles
# and you'd like to make use of the existing dependent libraries on
# your machine, there are more environment variables that you will
# need to set to let MET know where the library and header files are.
# Please supply values for the following environment variables
# in the input environment configuration file (install_met_env.<machine_name>:
# MET_GRIB2CLIB, MET_GRIB2CINC, GRIB2CLIB_NAME,
# MET_BUFRLIB, BUFRLIB_NAME, MET_HDF5, MET_NETCDF,
# MET_GSL, LIB_JASPER, LIB_PNG, LIB_Z.
#
# The optional libraries HDF4, HDFEOS, FREETYPE, and CAIRO are
# used for the following, not widely used tools, MODIS-Regrid,
# lidar2nc, and MODE Graphics.  To enable building of these libraries,
# directly modify this script changing the value of the compile flags
# for the library (e.g. COMPILE_HDF, COMPILE_HDFEOS) in the code
# below as directed by the comments.  If these libraries have already
# been installed and don't need to be reinstalled, please
# supply values for the following environment variables in the input
# environment configuration file (install_met_env.<machine_name>):
# MET_HDF, MET_HDFEOS, MET_FREETYPEINC, MET_FREETYPELIB,
# MET_CAIROINC, MET_CAIROLIB.
#
#================================================


if [ -z $1 ]; then
  echo
  echo "No environment configuration file provided (e.g. install_met_env.<machine_name>). Starting compilation with current environment."
else
  if [ ! -f "$1" ]; then
    echo "The file \"$1\" does not exist!"
    exit 1
  else
    source $1
  fi
fi

echo
echo "TEST_BASE = ${TEST_BASE? "ERROR: TEST_BASE must be set"}"
echo "COMPILER  = ${COMPILER?  "ERROR: COMPILER must be set"}"
echo "MET_SUBDIR = ${MET_SUBDIR? "ERROR: MET_SUBDIR must be set"}"
echo "MET_TARBALL = ${MET_TARBALL? "ERROR: MET_TARBALL must be set"}"
echo "USE_MODULES = ${USE_MODULES? "ERROR: USE_MODULES must be set to TRUE if using modules or FALSE otherwise"}"

export LIB_DIR=${TEST_BASE}/external_libs
MET_DIR=${MET_SUBDIR}
TAR_DIR=${TEST_BASE}/tar_files
MET_TARBALL=${TAR_DIR}/${MET_TARBALL}

# Create directory for libraries
mkdir -p ${LIB_DIR}

# Check that tar files exist
if [ ! -e $TAR_DIR ]; then
  echo "TAR File directory doesn't exist: ${TAR_DIR}"
  exit 1
fi

# Update library linker path
export LD_LIBRARY_PATH=${TEST_BASE}/external_libs/lib${MET_PYTHON:+:$MET_PYTHON/lib}${MET_NETCDF:+:$MET_NETCDF/lib}${MET_HDF5:+:$MET_HDF5/lib}${MET_BUFRLIB:+:$MET_BUFRLIB}${MET_GRIB2CLIB:+:$MET_GRIB2CLIB}${LIB_JASPER:+$LIB_JASPER}${LIB_LIBPNG:+:$LIB_JASPER}${LIB_Z:+$LIB_Z}${MET_GSL:+:$MET_GSL/lib}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
echo "LD_LIBRARY_PATH = ${LD_LIBRARY_PATH}"

# Constants
if [[ -z ${MET_GRIB2CLIB} ]] && [[ -z ${MET_GRIB2C} ]]; then
  COMPILE_ZLIB=1
  COMPILE_LIBPNG=1
  COMPILE_JASPER=1
  COMPILE_G2CLIB=1
else
  COMPILE_ZLIB=0
  COMPILE_LIBPNG=0
  COMPILE_JASPER=0
  COMPILE_G2CLIB=0
fi

if [ -z ${MET_BUFRLIB} ]; then
  COMPILE_BUFRLIB=1
else
  COMPILE_BUFRLIB=0
fi

if [ -z ${MET_NETCDF} ]; then
  COMPILE_NETCDF=1
else
  COMPILE_NETCDF=0
fi

if [ -z ${MET_GSL} ]; then
  COMPILE_GSL=1
else
  COMPILE_GSL=0
fi

if [[ -z ${MET_HDF} ]] && [[ -z ${MET_HDFEOS} ]]; then
  # Only set COMPILE_HDF and COMPILE_HDFEOS to 1 if you want to compile and enable MODIS-Regrid (not widely used)
  COMPILE_HDF=0
  COMPILE_HDFEOS=0
  if [[ $COMPILE_HDF -eq 1 && $COMPILE_HDFEOS -eq 1 ]]; then
    export MET_HDF=${LIB_DIR}
    export MET_HDFEOS=${LIB_DIR}
  fi
else
  # Only set COMPILE_HDF and COMPILE_HDFEOS to 1 if you have already compiled HDF4 and HDFEOS,
  # have set MET_HDF and MET_HDFEOS in your configuration file, and want to enable
  # MODIS-Regrid (not widely used)
  COMPILE_HDF=0
  COMPILE_HDFEOS=0
fi

if [[ ! -z ${MET_FREETYPE} ]]; then
  echo "ERROR: MET_FREETYPEINC and MET_FREETYPELIB must be set instead of MET_FREETYPE"
  exit 1
fi

if [[ ! -z ${MET_CAIRO} ]]; then
  echo	"ERROR: MET_CAIROINC and MET_CAIROLIB must be set instead of MET_CAIRO"
  exit 1
fi

if [[ -z ${MET_FREETYPEINC} && -z ${MET_FREETYPELIB} && -z ${MET_CAIROINC} && -z ${MET_CAIROLIB} ]]; then
  # Only set COMPILE_FREETYPE and COMPILE_CAIRO to 1 if you want to compile and enable MODE Graphics (not widely used)
  COMPILE_FREETYPE=0
  COMPILE_CAIRO=0
  if [[ $COMPILE_CAIRO -eq 1 && $COMPILE_FREETYPE -eq 1 ]]; then
    export MET_CAIROINC=${LIB_DIR}/include/cairo
    export MET_CAIROLIB=${LIB_DIR}/lib
    export MET_FREETYPEINC=${LIB_DIR}/include/freetype2
    export MET_FREETYPELIB=${LIB_DIR}/lib
  fi
else
  # Only set COMPILE_FREETYPE and COMPILE_CAIRO to 1 if you have compiled FREETYPE and CAIRO,
  # have set MET_FREETYPEINC, MET_FREETYPELIB, MET_CAIROINC, and MET_CAIROLIB in your
  # configuration file, and want to enable MODE Graphics (not widely used)
  COMPILE_FREETYPE=0
  COMPILE_CAIRO=0
fi

COMPILE_MET=1

if [ -z ${BIN_DIR_PATH} ]; then
  BIN_DIR_PATH=${TEST_BASE}/bin
else
  BIN_DIR_PATH=${BIN_DIR_PATH}
fi

if [ -z ${USE_MET_TAR_FILE} ]; then
  export USE_MET_TAR_FILE=TRUE
fi

echo
echo "Compiling libraries into: ${LIB_DIR}"

if [ ! -e ${LIB_DIR}/include ]; then
  mkdir ${LIB_DIR}/include
fi

if [ ! -e ${LIB_DIR}/lib ]; then
  mkdir ${LIB_DIR}/lib
fi

# Load compiler version
COMPILER_FAMILY=` echo $COMPILER | cut -d'_' -f1`
COMPILER_VERSION=`echo $COMPILER | cut -d'_' -f2`
COMPILER_MAJOR_VERSION=`echo $COMPILER_VERSION | cut -d'.' -f1`

echo
echo "USE_MODULES = ${USE_MODULES}"
echo

if [ ${USE_MODULES} = "TRUE" ]; then
  echo "module load ${COMPILER_FAMILY}/${COMPILER_VERSION}"
  echo ${COMPILER_FAMILY}/${COMPILER_VERSION}

  module load ${COMPILER_FAMILY}/${COMPILER_VERSION}
  if [ ${COMPILER_FAMILY} =  "PrgEnv-intel" ]; then
    module load craype
    module switch craype craype-sandybridge
  fi
fi

if [ ${COMPILER_FAMILY} = "gnu" ]; then
  if [ -z ${CC} ]; then
    export CC=`which gcc`
  else
    export CC=${CC}
  fi
  if [ -z ${CXX} ]; then
    export CXX=`which g++`
  else
    export CXX=${CXX}
  fi
  if [ -z ${FC} ]; then
    export FC=`which gfortran`
  else
    export FC=${FC}
  fi
  if [ -z ${F77} ]; then
    export F77=`which gfortran`
  else
    export F77=${F77}
  fi
  if [ -z ${F90} ]; then
    export F90=`which gfortran`
  else
    export F90=${F90}
  fi
elif [ ${COMPILER_FAMILY} = "pgi" ]; then
  if [ -z ${CC} ]; then
    export CC=`which pgcc`
  else
    export CC=${CC}
  fi
  if [ -z ${CXX} ]; then
    export CXX=`which pgc++`
  else
    export CXX=${CXX}
  fi
  if [ -z ${F90} ]; then
    export F90=`which pgf90`
    export F77=${F90}
    export FC=${F90}
  else
    export F90=${F90}
    export F77=${F90}
    export FC=${F90}
  fi
elif [[ ${COMPILER_FAMILY} == "intel" ]] || [[ ${COMPILER_FAMILY} == "ics" ]] || [[ ${COMPILER_FAMILY} == "ips" ]] || [[ ${COMPILER_FAMILY} == "PrgEnv-intel" ]]; then
  if [ -z ${CC} ]; then
    export CC=`which icc`
  else
    export CC=${CC}
  fi
  if [ -z ${CXX} ]; then
    export CXX=`which icc`
  else
    export CXX=${CXX}
  fi
  if [ -z ${FC} ]; then
    export FC=`which ifort`
  else
    export FC=${FC}
  fi
  if [ -z ${F77} ]; then
    export F77=`which ifort`
  else
    export F77=${F77}
  fi
  if [ -z ${F90} ]; then
    export F90=`which ifort`
  else
    export F90=${F90}
  fi
else
  echo "ERROR: \${COMPILER} must start with gnu, intel, ics, ips, PrgEnv-intel, or pgi"
  exit
fi

echo "export  CC=${CC}"
echo "export CXX=${CXX}"
echo "export  FC=${FC}"
echo "export F77=${F77}"
echo "export F90=${F90}"
echo

# Load Python module

if [ ${USE_MODULES} = "TRUE" ]; then
  if [ ! -z ${PYTHON_MODULE} ]; then
    PYTHON_NAME=` echo $PYTHON_MODULE | cut -d'_' -f1`
    PYTHON_VERSION_NUM=`echo $PYTHON_MODULE | cut -d'_' -f2`
    echo "module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}"
    echo ${PYTHON_NAME}/${PYTHON_VERSION_NUM}
    module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}
  fi
fi

if [[ ${MET_PYTHON}/bin/python3 ]]; then
  echo "Using python version: "
  ${MET_PYTHON}/bin/python3 --version
fi

# Compile GSL
if [ $COMPILE_GSL -eq 1 ]; then

  if [ ${COMPILER_FAMILY} = "pgi" ]; then
    vrs="1.11";
  else
    vrs="2.1";
  fi

  echo
  echo "Compiling GSL_${vrs} at `date`"
  mkdir -p ${LIB_DIR}/gsl
  cd ${LIB_DIR}/gsl
  rm -rf gsl*
  tar -xf ${TAR_DIR}/gsl-${vrs}.tar.gz
  cd gsl*
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
fi

# Compile BUFRLIB
if [ $COMPILE_BUFRLIB -eq 1 ]; then

  vrs="v11.3.0";

  echo
  echo "Compiling BUFRLIB_${vrs} at `date`"
  mkdir -p ${LIB_DIR}/bufrlib/BUFRLIB_${vrs}
  cd ${LIB_DIR}/bufrlib/BUFRLIB_${vrs}
  echo "cd `pwd`"
  rm -rf *
  tar -xf ${TAR_DIR}/BUFRLIB_`echo $vrs | sed 's/\./-/g'`.tar

  ${CC} -c -DUNDERSCORE `./getdefflags_C.sh` *.c >> make.log 2>&1

  # For GNU and Intel follow BUFRLIB11 instructions
  if [[ ${COMPILER_FAMILY} == "gnu" ]]; then
    if [[ ${COMPILER_MAJOR_VERSION} -ge 10 ]]; then
      ${FC} -c -fno-second-underscore -fallow-argument-mismatch `./getdefflags_F.sh` modv*.F moda*.F `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log 2>&1
    elif [[ ${COMPILER_MAJOR_VERSION} -lt 10 ]]; then
      ${FC} -c -fno-second-underscore -Wno-argument-mismatch `./getdefflags_F.sh` modv*.F moda*.F `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log 2>&1
    fi	
  elif [[ ${COMPILER_FAMILY} == "intel" ]] || [[ ${COMPILER_FAMILY} == "ics" ]] || [[ ${COMPILER_FAMILY} == "ips" ]] || [[ ${COMPILER_FAMILY} == "PrgEnv-intel" ]]; then
    ${FC} -c `./getdefflags_F.sh` modv*.F moda*.F `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log 2>&1
  elif [[ ${COMPILER_FAMILY} == "pgi" ]]; then
    ${FC} -c -Mnosecond_underscore `./getdefflags_F.sh` modv*.F moda*.F `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log 2>&1
  fi

  ar crv libbufr.a *.o >> make.log 2>&1
  cp *.a ${LIB_DIR}/lib/.
fi


# Compile ZLIB
if [ $COMPILE_ZLIB -eq 1 ]; then
  echo
  echo "Compiling ZLIB at `date`"
  mkdir -p ${LIB_DIR}/zlib
  cd ${LIB_DIR}/zlib
  rm -rf zlib*
  tar -xzf ${TAR_DIR}/zlib*.tar.gz
  cd zlib*
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
  echo "rm ${LIB_DIR}/lib/zlib.a"
  rm ${LIB_DIR}/lib/libz.a
fi

# Compile LIBPNG
if [[ $COMPILE_LIBPNG -eq 1 && $HOST != ys* ]]; then
  echo
  echo "Compiling LIBPNG at `date`"
  mkdir -p ${LIB_DIR}/libpng
  cd ${LIB_DIR}/libpng
  rm -rf libpng*
  tar -xzf ${TAR_DIR}/libpng*.tar.gz
  cd libpng*
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
fi

# Compile JASPER
if [ $COMPILE_JASPER -eq 1 ]; then
  echo
  echo "Compiling JASPER at `date`"
  mkdir -p ${LIB_DIR}/jasper
  cd ${LIB_DIR}/jasper
  rm -rf jasper*
  unzip ${TAR_DIR}/jasper*.zip > /dev/null 2>&1
  cd jasper*
  export CPPFLAGS="-I${LIB_DIR}/include"
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
fi

# Compile G2CLIB
if [ $COMPILE_G2CLIB -eq 1 ]; then
  echo
  echo "Compiling G2CLIB at `date`"
  mkdir -p ${LIB_DIR}/g2clib
  cd ${LIB_DIR}/g2clib
  rm -rf g2clib*
  tar -xf ${TAR_DIR}/g2clib*.tar
  cd g2clib*
  cat makefile | \
    sed -r 's/INC=.*/INC=-I${LIB_DIR}\/include -I${LIB_DIR}\/include\/jasper/g' | \
    sed 's/CC=gcc/CC=${CC_COMPILER}/g' | \
    sed 's/-D__64BIT__//g' \
    > makefile_new
  mv makefile_new makefile
  export CC_COMPILER=${CC}
  echo "cd `pwd`"
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  cp libg2c*.a ${LIB_DIR}/lib/libgrib2c.a
  cp *.h ${LIB_DIR}/include/.
fi

# Compile HDF
# Depends on jpeg
# Edit 'mfhdf/hdiff/Makefile' as follows:
#  From: LIBS = -ljpeg -lz
#  To: LIBS = -ljpeg -lz -lm
if [ $COMPILE_HDF -eq 1 ]; then
  echo
  echo "Compiling HDF at `date`"
  mkdir -p ${LIB_DIR}/hdf
  cd ${LIB_DIR}/hdf
  rm -rf HDF*
  tar -xf ${TAR_DIR}/HDF4.2*.tar.gz
  cd HDF*
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} --disable-netcdf --with-jpeg=${LIB_DIR} --with-zlib=${LIB_DIR} > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} --disable-netcdf --with-jpeg=${LIB_DIR} --with-zlib=${LIB_DIR} > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  cat mfhdf/hdiff/Makefile | \
    sed 's/LIBS = -ljpeg -lz/LIBS = -ljpeg -lz -lm/g' \
    > Makefile_new
  mv Makefile_new mfhdf/hdiff/Makefile
  if [[ ${COMPILER_MAJOR_VERSION} -ge 10 ]]; then
    cat hdf/src/Makefile | \
      sed 's/FFLAGS =  -O2/FFLAGS = -w -fallow-argument-mismatch -O2/g' \
      > Makefile_new
  elif [[ ${COMPILER_MAJOR_VERSION} -lt 10 ]]; then
    cat hdf/src/Makefile | \
      sed 's/FFLAGS =  -O2/FFLAGS = -w -Wno-argument-mismatch -O2/g' \
      > Makefile_new
  fi
  mv Makefile_new hdf/src/Makefile
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
fi

# Compile HDFEOS
# Depends on HDF
if [ $COMPILE_HDFEOS -eq 1 ]; then
  echo
  echo "Compiling HDFEOS at `date`"
  mkdir -p ${LIB_DIR}/hdfeos
  cd ${LIB_DIR}/hdfeos
  rm -rf HDF-EOS*
  tar -xzf ${TAR_DIR}/HDF-EOS*.tar.*
  cd hdfeos
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} --with-hdf4=${LIB_DIR} > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} --with-hdf4=${LIB_DIR} --with-jpeg=${LIB_DIR} > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
  cp include/*.h ${LIB_DIR}/include/
fi

# Compile NetCDF
if [ $COMPILE_NETCDF -eq 1 ]; then

  echo
  echo "Compiling HDF5 at `date`"
  mkdir -p ${LIB_DIR}/hdf5
  cd ${LIB_DIR}/hdf5
  rm -rf hdf5*
  tar -xzf ${TAR_DIR}/hdf5*.tar.gz
  cd hdf5*
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} --with-zlib=${LIB_DIR}/lib CFLAGS=-fPIC CXXFLAGS=-fPIC FFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} --with-zlib=${LIB_Z} CFLAGS=-fPIC CXXFLAGS=-fPIC FFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib:${LIB_Z} CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi

  echo
  echo "Compiling NetCDF-C at `date`"
  mkdir -p ${LIB_DIR}/netcdf
  cd ${LIB_DIR}/netcdf
  rm -rf netcdf*
  tar -xzf ${TAR_DIR}/netcdf-4*.tar.gz
  cd netcdf-c-4*
  export FC=''
  export F90=''
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi

  echo
  echo "Compiling NetCDF-CXX at `date`"
  cd ${LIB_DIR}/netcdf
  tar -xzf ${TAR_DIR}/netcdf-cxx*.tar.gz
  cd netcdf-cxx*
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
fi

# Compile FREETYPE
if [ $COMPILE_FREETYPE -eq 1 ]; then
  echo
  echo "Compiling FREETYPE at `date`"
  mkdir -p ${LIB_DIR}/freetype
  cd ${LIB_DIR}/freetype
  rm -rf freetype*
  tar -xzf ${TAR_DIR}/freetype*.tar.gz
  cd freetype*
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} --with-png=yes > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} --with-png=yes > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1 --with-zlib=${LIB_DIR} LDFLAGS=-L${LIB_DIR} CPPFLAGS=-I${LIB_DIR}"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
fi


# Compile CAIRO
if [ $COMPILE_CAIRO -eq 1 ]; then

  # If on Cray, compile PIXMAN
  if [ ${COMPILER_FAMILY} =  "PrgEnv-intel" ]; then
    echo
    echo "Compiling pixman at `date`"
    mkdir -p  ${LIB_DIR}/pixman
    cd ${LIB_DIR}/pixman
    rm -rf pixman*
    tar -xzf ${TAR_DIR}/pixman*.tar.gz
    cd pixman*
    echo "cd `pwd`"
    echo "./configure --prefix=${LIB_DIR} > configure.log 2>&1"
    ./configure --prefix=${LIB_DIR} > configure.log 2>&1
    ret=$?
    if [ $ret != 0 ]; then
      echo "configure returned with non-zero ($ret) status"
      exit 1
    fi
    echo "make > make.log 2>&1"
    make > make.log 2>&1
    ret=$?
    if [ $ret != 0 ]; then
      echo "make returned with non-zero ($ret) status"
      exit 1
    fi
    echo "make install > make_install.log 2>&1"
    make install > make_install.log 2>&1
    ret=$?
    if [ $? != 0 ]; then
      echo "make install returned with non-zero ($ret) status"
      exit 1
    fi
  fi

  echo
  echo "Compiling CAIRO at `date`"
  mkdir -p ${LIB_DIR}/cairo
  cd ${LIB_DIR}/cairo
  rm -rf cairo*
  tar -xf ${TAR_DIR}/cairo*.tar*
  cd cairo*
  export PKG_CONFIG=`which pkg-config`
  if [ ${COMPILER_FAMILY} =  "PrgEnv-intel" ]; then
    export PKG_CONFIG_PATH=${LIB_DIR}/lib/pkgconfig/
  fi
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} ax_cv_c_float_words_bigendian=no LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} ax_cv_c_float_words_bigendian=no LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi
  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi
fi

# Compile MET
if [ $COMPILE_MET -eq 1 ]; then

  echo
  echo "Compiling MET at `date`"
  cd ${MET_DIR}
  # If using source from a tar file remove everything and unpack the tar file
  # FALSE = compiling from github repo and we don't want to overwrite the files
  if [ ${USE_MET_TAR_FILE} = "TRUE" ]; then
    rm -rf MET*
    tar -xzf ${MET_TARBALL}
  fi
  cd MET*

  if [ -z ${MET_BUFRLIB} ]; then
    export MET_BUFRLIB=${LIB_DIR}/lib
    export BUFRLIB_NAME=-lbufr
  fi

  if [ -z ${MET_GRIB2CLIB} ]; then
    export MET_GRIB2CLIB=${LIB_DIR}/lib
    export MET_GRIB2CINC=${LIB_DIR}/include
    export LIB_JASPER=${LIB_DIR}/lib
    export LIB_LIBPNG=${LIB_DIR}/lib
    export LIB_Z=${LIB_DIR}/lib
    export GRIB2CLIB_NAME=-lgrib2c
  fi

  if [ -z ${MET_NETCDF} ]; then
    export MET_NETCDF=${LIB_DIR}
    export MET_HDF5=${LIB_DIR}
  fi

  if [ -z ${MET_GSL} ]; then
    export MET_GSL=${LIB_DIR}
  fi

  export MET_PYTHON_LD=${MET_PYTHON_LD}
  export MET_PYTHON_CC=${MET_PYTHON_CC}
  export LDFLAGS="-Wl,--disable-new-dtags"
  # https://www.gnu.org/software/bash/manual/html_node/Shell-Parameter-Expansion.html
  # ${parameter:+word}
  # If parameter is null or unset, nothing is substituted, otherwise the expansion of word is substituted.
  export LDFLAGS="${LDFLAGS} -Wl,-rpath,${LIB_DIR}/lib${MET_NETCDF:+:$MET_NETCDF/lib}${MET_HDF5:+:$MET_HDF5/lib}${MET_BUFRLIB:+:$MET_BUFRLIB}${MET_GRIB2CLIB:+:$MET_GRIB2CLIB}${MET_PYTHON:+:$MET_PYTHON/lib}${MET_GSL:+:$MET_GSL/lib}"
  export LDFLAGS="${LDFLAGS} -Wl,-rpath,${LIB_JASPER:+$LIB_JASPER}${LIB_LIBPNG:+:$LIB_PNG}${LIB_Z:+$LIB_Z}"
  export LDFLAGS="${LDFLAGS} ${LIB_JASPER:+-L$LIB_JASPER} ${LIB_LIBPNG:+-L$LIB_LIBPNG} ${MET_HDF5:+-L$MET_HDF5/lib}"
  export LIBS="${LIBS} -lhdf5_hl -lhdf5 -lz"
  export MET_FONT_DIR=${TEST_BASE}/fonts

  if [ "${SET_D64BIT}" = "TRUE" ]; then
    export CFLAGS="-D__64BIT__"
    export CXXFLAGS="-D__64BIT__"
  fi

  echo "MET Configuration settings..."
  printenv | egrep "^MET_" | sed -r 's/^/export /g'
  echo "LDFLAGS = ${LDFLAGS}"
  export OPT_ARGS=''
  if [[ $COMPILER_FAMILY == "pgi" ]]; then
    export OPT_ARGS="${OPT_ARGS} FFLAGS=-lpgf90"
  fi

  echo "cd `pwd`"
  if [[ -z ${MET_PYTHON_CC} && -z ${MET_PYTHON_LD} ]]; then
    if [[ ! -z ${MET_FREETYPEINC} && ! -z ${MET_FREETYPELIB} && ! -z ${MET_CAIROINC} && ! -z ${MET_CAIROLIB} ]]; then
      if [[ ! -z $MET_HDF && ! -z $MET_HDFEOS ]]; then
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1
      else
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-mode_graphics ${OPT_ARGS} > configure.log 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-mode_graphics ${OPT_ARGS} > configure.log 2>&1
      fi
    else
      if [[ ! -z $MET_HDF && ! -z $MET_HDFEOS ]]; then
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1
      else
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 ${OPT_ARGS} > configure.lo\
g 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 ${OPT_ARGS} > configure.log 2>&1
      fi
    fi
  else
    if [[ ! -z ${MET_FREETYPEINC} && ! -z ${MET_FREETYPELIB} && ! -z ${MET_CAIROINC} && ! -z ${MET_CAIROLIB} ]]; then
      if [[ ! -z $MET_HDF && ! -z $MET_HDFEOS ]]; then
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.lo\
g 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.log 2>&1
      else
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-mode_graphics --enable-python ${OPT_ARGS} > configure.lo\
g 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-mode_graphics --enable-python ${OPT_ARGS} > configure.log 2>&1
      fi
    else
      if [[ ! -z $MET_HDF && ! -z $MET_HDFEOS ]]; then
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.lo\
g 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.log 2>&1
      else
        echo "./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-python ${OPT_ARGS} > configure.lo\
g 2>&1"
        ./configure --prefix=${MET_DIR} --bindir=${BIN_DIR_PATH} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-python ${OPT_ARGS} > configure.log 2>&1
      fi
    fi
  fi

  ret=$?
  if [ $ret != 0 ]; then
    echo "configure returned with non-zero ($ret) status"
    exit 1
  fi

  echo "make > make.log 2>&1"
  make > make.log 2>&1
  ret=$?
  if [ $ret != 0 ]; then
    echo "make returned with non-zero ($ret) status"
    exit 1
  fi

  echo "make install > make_install.log 2>&1"
  make install > make_install.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make install returned with non-zero ($ret) status"
    exit 1
  fi

  echo "make test > make_test.log 2>&1"
  make test > make_test.log 2>&1
  ret=$?
  if [ $? != 0 ]; then
    echo "make test returned with non-zero ($ret) status"
    exit 1
  fi
fi

echo "Finished compiling at `date`"

