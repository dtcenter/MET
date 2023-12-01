#!/bin/bash
#
# Compile and install MET
# (Model Evaluation Tools)
#================================================
#
# This compile_MET_all.sh script expects certain environment
# variables to be set:
# TEST_BASE, COMPILER (or COMPILER_FAMILY and COMPILER_VERSION),
# MET_SUBDIR, MET_TARBALL, and USE_MODULES.
#
# If compiling support for Python embedding, users will need to
# set MET_PYTHON, MET_PYTHON_BIN_EXE, MET_PYTHON_CC, and MET_PYTHON_LD.
# Users can directly set the python module to be loaded by setting
# either PYTHON_MODULE or by setting PYTHON_NAME and PYTHON_VERSION:
# - PYTHON_MODULE (only used if USE_MODULES=TRUE) - format is the name
#   of the Python module to load followed by an underscore and then the
#   version number (e.g. python_3.10.4, The script will then run "module
#   load python/3.10.4")
# - PYTHON_NAME = python (or e.g. python3, etc.)
# - PYTHON_VERSION = 3.10.4
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
# PROJ (with dependency SQLITE >= 3.11), GSL, BUFRLIB, 
# GRIB2C (with dependencies Z, PNG, JASPER), HDF5, NETCDF (C and CXX), 
# HDF4 (optional for MODIS-Regrid and lidar2nc), HDFEOS (optional for
# MODIS-Regrid and lidar2nc), FREETYPE (optional for MODE Graphics),
# and CAIRO (optional for MODE Graphics).
#
# If these libraries have already been installed and don't need to be
# reinstalled or if you are compiling on a machine that uses modulefiles
# and you'd like to make use of the existing dependent libraries on
# your machine, there are more environment variables that you will
# need to set to let MET know where the library and header files are.
# Please supply values for the following environment variables
# in the input environment configuration file (install_met_env.<machine_name>:
# MET_GRIB2CLIB, MET_GRIB2CINC, GRIB2CLIB_NAME, MET_BUFRLIB, BUFRLIB_NAME, 
# MET_HDF5, MET_NETCDF, MET_PROJ, MET_GSL, LIB_JASPER, LIB_PNG, LIB_Z,
# SQLITE_INCLUDE_DIR, SQLITE_LIB_DIR.
#
# The optional libraries ecKit and atlas offer support for unstructured
# grids. The optional libraries HDF4, HDFEOS, FREETYPE, and CAIRO are
# used for the following, not widely used tools, MODIS-Regrid,
# lidar2nc, and MODE Graphics. To enable building of these libraries,
# set the compile flags for the library (e.g. COMPILE_ECKIT, COMPILE_ATLAS,
# COMPILE_HDF, COMPILE_HDFEOS) to any value in the environment config
# file. If these libraries have already been installed and don't need
# to be reinstalled, please supply values for the following environment
# variables in the input environment configuration file
# (install_met_env.<machine_name>): MET_ECKIT, MET_ATLAS, MET_HDF,
# MET_HDFEOS, MET_FREETYPEINC, MET_FREETYPELIB, MET_CAIROINC,
# MET_CAIROLIB.
#
# Users can speed up the compilation of MET and its dependent libraries
# by adding the following to their environment configuration file:
# export MAKE_ARGS=-j #
# replacing the # with the number of cores to use (integer) or simply
# specifying:
# export MAKE_ARGS=-j
# with no integer argument to start as many processes in parallel as
# possible.
#================================================

# print command, run it, then error and exit if non-zero value is returned
function run_cmd {
  echo $*
  eval "$@"
  ret=$?
  if [ $ret != 0 ]; then
    echo "ERROR: Command returned with non-zero ($ret) status: $*"
    exit $ret
  fi
}

if [ -z $1 ]; then
  echo
  echo "No environment configuration file provided (e.g. install_met_env.<machine_name>). Starting compilation with current environment."
else
  if [ ! -f "$1" ]; then
    echo "The file \"$1\" does not exist!"
    exit 1
  fi

  source $1
fi

echo
echo "TEST_BASE = ${TEST_BASE? "ERROR: TEST_BASE must be set"}"
echo "MET_SUBDIR = ${MET_SUBDIR? "ERROR: MET_SUBDIR must be set"}"
echo "MET_TARBALL = ${MET_TARBALL? "ERROR: MET_TARBALL must be set"}"
echo "USE_MODULES = ${USE_MODULES? "ERROR: USE_MODULES must be set to TRUE if using modules or FALSE otherwise"}"
if [[ -z "$COMPILER" ]] && [[ -z "$COMPILER_FAMILY" && -z "$COMPILER_VERSION" ]]; then
   echo "ERROR: COMPILER or COMPILER_FAMILY and COMPILER_VERSION must be set"
   exit 1
fi
echo ${MAKE_ARGS:+MAKE_ARGS = $MAKE_ARGS}


LIB_DIR=${TEST_BASE}/external_libs
MET_DIR=${MET_SUBDIR}

if [ -z "${BIN_DIR_PATH}" ]; then
  if [ -z "${MET_INSTALL_DIR}" ]; then
    BIN_DIR_PATH=${TEST_BASE}/bin
  else
    BIN_DIR_PATH=${MET_INSTALL_DIR}/bin
  fi
fi

if [ -z "${MET_INSTALL_DIR}" ]; then
  MET_INSTALL_DIR=${MET_DIR}
else
  LIB_DIR=${MET_INSTALL_DIR}
fi

TAR_DIR=${TEST_BASE}/tar_files
MET_TARBALL=${TAR_DIR}/${MET_TARBALL}

# Create directory for libraries
mkdir -p ${LIB_DIR}

# Check that tar files exist
if [ ! -e $TAR_DIR ]; then
  echo "TAR File directory doesn't exist: ${TAR_DIR}"
  exit 1
fi

# If MET_PYTHON_LIB is not set in the environment file, set it to the
# lib directory so it can be use to install MET with Python Embedding
# support
if [[ -z "$MET_PYTHON_LIB" ]]; then
  MET_PYTHON_LIB=${MET_PYTHON}/lib
fi


# Print library linker path
echo "LD_LIBRARY_PATH = ${LD_LIBRARY_PATH}"

# if LIB_Z is not set in the environment file, set it to the
# lib directory so it can be used to install HDF5 with zlib support
if [[ -z "$LIB_Z" ]]; then
  LIB_Z=${LIB_DIR}/lib
fi

# if SQLITE is not defined in the environment file, enable its compilation
if [[ -z ${SQLITE_INCLUDE_DIR} ]] && [[ -z ${SQLITE_LIB_DIR} ]]; then
   COMPILE_SQLITE=1
else
   COMPILE_SQLITE=0       
fi

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

if [ -z ${MET_PROJ} ]; then
  COMPILE_PROJ=1
else
  COMPILE_PROJ=0
fi

if [ -z ${MET_GSL} ]; then
  COMPILE_GSL=1
else
  COMPILE_GSL=0
fi

# Only set COMPILE_ECKIT and COMPILE_ATLAS if you want to compile and enable support for unstructued grids
if [ ! -z "${COMPILE_ECKIT}" ]; then COMPILE_ECKIT=1; else COMPILE_ECKIT=0; fi
if [ ! -z "${COMPILE_ATLAS}" ]; then COMPILE_ATLAS=1; else COMPILE_ATLAS=0;  fi

if [[ -z ${MET_ECKIT} ]] && [[ -z ${MET_ATLAS} ]]; then
  if [[ $COMPILE_ECKIT -eq 1 && $COMPILE_ATLAS -eq 1 ]]; then
    export MET_ECKIT=${LIB_DIR}
    export MET_ATLAS=${LIB_DIR}
  fi
else
  # Only set COMPILE_ECKIT and COMPILE_ATLAS to 1 if you have already compiled ECKIT and ATLAS,
  # have set MET_ECKIT and MET_ATLAS in your configuration file, and want to enable
  # unstructured grids 
  COMPILE_ECKIT=0
  COMPILE_ATLAS=0
fi

# Only set COMPILE_HDF and COMPILE_HDFEOS if you want to compile and enable MODIS-Regrid (not widely used)
if [ ! -z "${COMPILE_HDF}" ]; then COMPILE_HDF=1; else COMPILE_HDF=0; fi
if [ ! -z "${COMPILE_HDFEOS}" ]; then COMPILE_HDFEOS=1; else COMPILE_HDFEOS=0;  fi

if [[ -z ${MET_HDF} ]] && [[ -z ${MET_HDFEOS} ]]; then
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

# Only set COMPILE_FREETYPE and COMPILE_CAIRO if you want to compile and enable MODE Graphics (not widely used) 
if [ ! -z "${COMPILE_FREETYPE}" ]; then COMPILE_FREETYPE=1; else COMPILE_FREETYPE=0; fi
if [ ! -z "${COMPILE_CAIRO}" ]; then COMPILE_CAIRO=1; else COMPILE_CAIRO=0; fi


if [[ ! -z ${MET_FREETYPE} ]]; then
  echo "ERROR: MET_FREETYPEINC and MET_FREETYPELIB must be set instead of MET_FREETYPE"
  exit 1
fi

if [[ ! -z ${MET_CAIRO} ]]; then
  echo	"ERROR: MET_CAIROINC and MET_CAIROLIB must be set instead of MET_CAIRO"
  exit 1
fi

if [[ -z ${MET_FREETYPEINC} && -z ${MET_FREETYPELIB} && -z ${MET_CAIROINC} && -z ${MET_CAIROLIB} ]]; then
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

# skip compilation of MET if SKIP_MET is set
if [ ! -z "${SKIP_MET}" ]; then COMPILE_MET=0; fi

# skip compilation of external libraries if SKIP_LIBS is set
if [ ! -z "${SKIP_LIBS}" ]; then
  COMPILE_PROJ=0
  COMPILE_GSL=0
  COMPILE_BUFRLIB=0
  COMPILE_ZLIB=0
  COMPILE_LIBPNG=0
  COMPILE_JASPER=0
  COMPILE_G2CLIB=0
  COMPILE_ECKIT=0
  COMPILE_ATLAS=0
  COMPILE_HDF=0
  COMPILE_HDFEOS=0
  COMPILE_NETCDF=0
  COMPILE_FREETYPE=0
  COMPILE_CAIRO=0
fi

if [ -z ${BIN_DIR_PATH} ]; then
  BIN_DIR_PATH=${TEST_BASE}/bin
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
if [ -z ${COMPILER_FAMILY} ]; then
  COMPILER_FAMILY=` echo $COMPILER | cut -d'_' -f1`
fi

# Check for "oneapi" in compiler family name
#if echo ${COMPILER_FAMILY} | grep -E "^intel"; then
if [[ ${COMPILER_FAMILY} == *intel* ]]; then
  COMPILER_FAMILY_SUFFIX=` echo $COMPILER_FAMILY | cut -d'-' -f2`
fi

if [ -z ${COMPILER_VERSION} ]; then
  COMPILER_VERSION=`echo $COMPILER | cut -d'_' -f2`
fi

echo "COMPILER = $COMPILER"
echo "COMPILER_FAMILY = $COMPILER_FAMILY"
echo "COMPILER_FAMILY_SUFFIX = $COMPILER_FAMILY_SUFFIX"
echo "COMPILER_VERSION = $COMPILER_VERSION"
COMPILER_MAJOR_VERSION=`echo $COMPILER_VERSION | cut -d'.' -f1`
COMPILER_MINOR_VERSION=`echo $COMPILER_VERSION | cut -d'.' -f2`

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

# After loading the compiler module, strip any extra
# characters off of "gnu" (e.g. "gnu9")
if [[ ${COMPILER_FAMILY} == *gnu* ]]; then
    export COMPILER_FAMILY="gnu"
fi

if [ ${COMPILER_FAMILY} = "gnu" ]; then
  if [ -z ${CC} ]; then export CC=`which gcc`; fi
  if [ -z ${CXX} ]; then export CXX=`which g++`; fi
  if [ -z ${FC} ]; then export FC=`which gfortran`; fi
  if [ -z ${F77} ]; then export F77=`which gfortran`; fi
  if [ -z ${F90} ]; then export F90=`which gfortran`;  fi
elif [ ${COMPILER_FAMILY} = "pgi" ]; then
  if [ -z ${CC} ]; then export CC=`which pgcc`; fi
  if [ -z ${CXX} ]; then export CXX=`which pgc++`; fi
  if [ -z ${FC} ]; then export FC=`which pgf90`; fi
  if [ -z ${F77} ]; then export F77=`which pgf90`; fi
  if [ -z ${F90} ]; then export F90=`which pgf90`; fi
elif [[ ${COMPILER_FAMILY} == *intel* && ${CC} == "icc" ]] || \
     [[ ${COMPILER_FAMILY} == "ics" ]] || \
     [[ ${COMPILER_FAMILY} == "ips" ]] || \
     [[ ${COMPILER_FAMILY} == "intel-classic" ]] || \
     [[ ${COMPILER_FAMILY} == "PrgEnv-intel" ]]; then
  if [ -z ${CC} ]; then export CC=`which icc`; fi
  if [ -z ${CXX} ]; then export CXX=`which icpc`; fi
  if [ -z ${FC} ]; then export FC=`which ifort`; fi
  if [ -z ${F77} ]; then export F77=`which ifort`; fi
  if [ -z ${F90} ]; then export F90=`which ifort`; fi
elif [[ ${COMPILER_FAMILY} == *intel* ]] && [[ ${CC} == *icx* ]]; then
  export CXX=`which icpx`
  export FC=`which ifx`
  export F77=`which ifx`
  export F90=`which ifx`
elif [[ ${COMPILER_FAMILY_SUFFIX} == oneapi ]]; then
  export CC=`which icx`
  export CXX=`which icpx`
  export FC=`which ifx`
  export F77=`which ifx`
  export F90=`which ifx`
else
  echo "ERROR: \${COMPILER} must start with gnu, intel, ics, ips, intel-classic, PrgEnv-intel, or pgi"
  exit
fi

echo "export  CC=${CC}"
echo "export CXX=${CXX}"
echo "export  FC=${FC}"
echo "export F77=${F77}"
echo "export F90=${F90}"
echo

# Figure out what kind of OS is being used
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

# change sed command and extension for dynamic library files
if [[ $machine == "Mac" ]]; then
    sed_inline="sed -i ''"
    dynamic_lib_ext="dylib"
else
    sed_inline="sed -i''"
    dynamic_lib_ext="so"
fi

# Load Python module

if [ ${USE_MODULES} = "TRUE" ]; then
  if [ ! -z ${PYTHON_MODULE} ]; then
    PYTHON_NAME=`echo $PYTHON_MODULE | cut -d'_' -f1`
    PYTHON_VERSION_NUM=`echo $PYTHON_MODULE | cut -d'_' -f2`
    echo "module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}"
    echo ${PYTHON_NAME}/${PYTHON_VERSION_NUM}
    module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}
  # Allow the user to specify the name and version of the module to load
  elif [[ ! -z ${PYTHON_NAME} && ! -z ${PYTHON_VERSION_NUM} ]]; then
    echo "module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}"
    echo ${PYTHON_NAME}/${PYTHON_VERSION_NUM}
    module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}
  fi
fi

if [[ ${MET_PYTHON}/bin/python3 ]]; then
  echo "Using python version: "
  ${MET_PYTHON}/bin/python3 --version
fi

# Compile Proj
if [ $COMPILE_PROJ -eq 1 ]; then

  if [ $COMPILE_SQLITE -eq 1 ]; then
    echo
    echo "Compiling SQLITE at `date`"
    mkdir -p ${LIB_DIR}/sqlite
    rm -rf ${LIB_DIR}/sqlite/sqlite*
    tar -xf ${TAR_DIR}/sqlite*.tar.gz -C ${LIB_DIR}/sqlite > /dev/null 2>&1
    cd ${LIB_DIR}/sqlite/sqlite*
    echo "cd `pwd`"
    run_cmd "./configure --enable-shared --prefix=${LIB_DIR} > $(pwd)/sqlite.configure.log 2>&1"
    run_cmd "make ${MAKE_ARGS} > $(pwd)/sqlite.make.log 2>&1"
    run_cmd "make ${MAKE_ARGS} install > $(pwd)/sqlite.make_install.log 2>&1"
    export SQLITE_INCLUDE_DIR=${LIB_DIR}/include
    export SQLITE_LIB_DIR=${LIB_DIR}/lib
  fi

  vrs="7.1.0"

  echo
  echo "Compiling PROJ_${vrs} at `date`"
  mkdir -p ${LIB_DIR}/proj
  rm -rf ${LIB_DIR}/proj/proj*
  tar -xf ${TAR_DIR}/proj-${vrs}.tar.gz -C ${LIB_DIR}/proj
  cd ${LIB_DIR}/proj/proj*
  echo "cd `pwd`"
  export PATH=${LIB_DIR}/bin:${PATH}
  run_cmd "mkdir build; cd build"

  tiff_arg=""
  # add tiff library argument if necessary
  if [[ ! -z "$LIB_TIFF" ]]; then
      tiff_arg+="-DTIFF_LIBRARY_RELEASE=${LIB_TIFF}"
  fi

  cmd="cmake -DCMAKE_INSTALL_PREFIX=${LIB_DIR} -DSQLITE3_INCLUDE_DIR=${SQLITE_INCLUDE_DIR} -DSQLITE3_LIBRARY=${SQLITE_LIB_DIR}/libsqlite3.${dynamic_lib_ext} ${tiff_arg} .. > $(pwd)/proj.cmake.log 2>&1"
  run_cmd ${cmd}
  run_cmd "cmake --build . > $(pwd)/proj.cmake_build.log 2>&1"
  run_cmd "cmake --build . --target install > $(pwd)/proj.cmake_install.log 2>&1"

fi

# Compile GSL
if [ $COMPILE_GSL -eq 1 ]; then

  if [ ${COMPILER_FAMILY} = "pgi" ]; then
    vrs="1.11"
  else
    vrs="2.7.1"
  fi

  echo
  echo "Compiling GSL_${vrs} at `date`"
  mkdir -p ${LIB_DIR}/gsl
  rm -rf ${LIB_DIR}/gsl/gsl*
  tar -xf ${TAR_DIR}/gsl-${vrs}.tar.gz -C ${LIB_DIR}/gsl
  cd ${LIB_DIR}/gsl/gsl*
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} > $(pwd)/gsl.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > $(pwd)/gsl.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/gsl.make_install.log 2>&1"
fi

# Compile BUFRLIB
if [ $COMPILE_BUFRLIB -eq 1 ]; then

  vrs="v11.6.0"

  echo
  echo "Compiling bufr_${vrs} at `date`"
  mkdir -p ${LIB_DIR}/bufrlib
  rm -rf ${LIB_DIR}/bufrlib/NCEPLIBS-bufr-bufr_${vrs}
  tar -xf ${TAR_DIR}/bufr_${vrs}.tar.gz -C ${LIB_DIR}/bufrlib
  export SOURCE_DIR=${LIB_DIR}/bufrlib/NCEPLIBS-bufr-bufr_${vrs}
  cd $SOURCE_DIR
  echo "cd `pwd`"
  run_cmd "mkdir build"
  export BUILD_DIR=${SOURCE_DIR}/build
  run_cmd "cmake -H${SOURCE_DIR} -B${BUILD_DIR} -DCMAKE_INSTALL_PREFIX=${LIB_DIR} -DCMAKE_BUILD_TYPE=Debug > $(pwd)/bufr.cmake.log 2>&1"
  run_cmd "cd ${BUILD_DIR}"
  run_cmd "make ${MAKE_ARGS} > $(pwd)/bufr.make.log 2>&1"
  run_cmd "ctest > $(pwd)/bufr.ctest.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/bufr.make_install.log 2>&1"
fi


# Compile ZLIB
if [ $COMPILE_ZLIB -eq 1 ]; then
  echo
  echo "Compiling ZLIB at `date`"
  mkdir -p ${LIB_DIR}/zlib
  rm -rf ${LIB_DIR}/zlib/zlib*
  tar -xzf ${TAR_DIR}/zlib*.tar.gz -C ${LIB_DIR}/zlib
  cd ${LIB_DIR}/zlib/zlib*
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} > zlib.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > zlib.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > zlib.make_install.log 2>&1"

  # GPM: why is this removed? Could we add a comment to
  # describe why this is needed?
  echo "rm ${LIB_DIR}/lib/zlib.a"
  rm ${LIB_DIR}/lib/libz.a
fi

# Compile LIBPNG
if [[ $COMPILE_LIBPNG -eq 1 && $HOST != ys* ]]; then
  echo
  echo "Compiling LIBPNG at `date`"
  mkdir -p ${LIB_DIR}/libpng
  rm -rf ${LIB_DIR}/libpng/libpng*
  tar -xzf ${TAR_DIR}/libpng*.tar.gz -C ${LIB_DIR}/libpng
  cd ${LIB_DIR}/libpng/libpng*
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > libpng.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > libpng.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > libpng.make_install.log 2>&1"
fi

# Compile JASPER
if [ $COMPILE_JASPER -eq 1 ]; then

  vrs="2.0.25"
    
  echo
  echo "Compiling JASPER at `date`"
  mkdir -p ${LIB_DIR}/jasper
  rm -rf ${LIB_DIR}/jasper/jasper*
  tar -xf ${TAR_DIR}/jasper-${vrs}.tar.gz -C ${LIB_DIR}/jasper
  cd ${LIB_DIR}/jasper/jasper-version-${vrs}
  export CPPFLAGS="-I${LIB_DIR}/include"
  export SOURCE_DIR=${LIB_DIR}/jasper/jasper-version-${vrs}
  echo "cd `pwd`"
  export BUILD_DIR=${LIB_DIR}/jasper/jasper-version-${vrs}/build
  run_cmd "cmake -G \"Unix Makefiles\" -H${SOURCE_DIR} -B${BUILD_DIR} -DCMAKE_INSTALL_PREFIX=${LIB_DIR} > $(pwd)/jasper.cmake.log 2>&1"
  run_cmd "cd ${BUILD_DIR}"
  run_cmd "make clean all > $(pwd)/jasper.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} test > $(pwd)/jasper.make_test.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/jasper.make_install.log 2>&1"
fi

# Compile G2CLIB
if [ $COMPILE_G2CLIB -eq 1 ]; then

  vrs="1.6.4"

  echo
  echo "Compiling G2CLIB at `date`"
  mkdir -p ${LIB_DIR}/g2clib
  rm -rf ${LIB_DIR}/g2clib/NCEP*
  tar -xf ${TAR_DIR}/g2clib-${vrs}.tar.gz -C ${LIB_DIR}/g2clib
  cd ${LIB_DIR}/g2clib/NCEP*
  echo "cd `pwd`"
  run_cmd "mkdir build; cd build"
  run_cmd "cmake -DCMAKE_INSTALL_PREFIX=${LIB_DIR} -DCMAKE_PREFIX_PATH=${LIB_DIR} .. > $(pwd)/g2c.cmake.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > $(pwd)/g2c.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} test > $(pwd)/g2c.make_test.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/g2c.make_install.log 2>&1"
fi

# Compile ECKIT
if  [ $COMPILE_ECKIT -eq 1 ]; then

  # Need to obtain ecbuild before installing eckit

  vrs="3.5.0"
    
  echo  
  echo "Compiling ECBUILD at `date`"
  mkdir -p ${LIB_DIR}/ecbuild
  rm -rf ${LIB_DIR}/ecbuild/ecbuild*
  tar -xf ${TAR_DIR}/ecbuild-${vrs}.tar.gz -C ${LIB_DIR}/ecbuild
  cd ${LIB_DIR}/ecbuild/ecbuild*
  echo "cd `pwd`"
  run_cmd "mkdir build; cd build"
  run_cmd "cmake ../ -DCMAKE_INSTALL_PREFIX=${LIB_DIR} > $(pwd)/ecbuild.cmake.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/ecbuild.make_install.log 2>&1"
  
  vrs="1.20.2"

  echo
  echo "Compiling ECKIT at `date`"
  mkdir -p ${LIB_DIR}/eckit
  rm -rf ${LIB_DIR}/eckit/eckit*
  tar -xf ${TAR_DIR}/eckit-${vrs}.tar.gz -C ${LIB_DIR}/eckit
  cd ${LIB_DIR}/eckit/eckit*
  echo "cd `pwd`"
  run_cmd "mkdir build; cd build"
  run_cmd "cmake ../ -DCMAKE_INSTALL_PREFIX=${LIB_DIR} -DCMAKE_PREFIX_PATH=${LIB_DIR} > $(pwd)/eckit.cmake.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/eckit.make_install.log 2>&1"

fi

# Compile ATLAS
if [ $COMPILE_ATLAS -eq 1 ]; then

  vrs="0.30.0"

  echo
  echo "Compiling ATLAS at `date`"
  mkdir -p ${LIB_DIR}/atlas
  rm -rf ${LIB_DIR}/atlas/atlas*
  tar -xf ${TAR_DIR}/atlas-${vrs}.tar.gz -C ${LIB_DIR}/atlas
  cd ${LIB_DIR}/atlas/atlas*
  echo "cd `pwd`"
  run_cmd "mkdir build; cd build"
  run_cmd "cmake ../ -DCMAKE_INSTALL_PREFIX=${LIB_DIR} -DCMAKE_PREFIX_PATH=${LIB_DIR} > $(pwd)/atlas.cmake.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > $(pwd)/atlas.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/atlas.make_install.log 2>&1"

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
  rm -rf ${LIB_DIR}/hdf/HDF*
  tar -xf ${TAR_DIR}/HDF4.2*.tar.gz -C ${LIB_DIR}/hdf
  cd ${LIB_DIR}/hdf/HDF*
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} --disable-netcdf --with-jpeg=${LIB_DIR} --with-zlib=${LIB_DIR} CPPFLAGS=-I/usr/include/tirpc LIBS='-lm -ltirpc' > $(pwd)/hdf4.configure.log 2>&1"
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
  run_cmd "make ${MAKE_ARGS} > $(pwd)/hdf4.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/hdf4.make_install.log 2>&1"
fi

# Compile HDFEOS
# Depends on HDF
if [ $COMPILE_HDFEOS -eq 1 ]; then
  echo
  echo "Compiling HDFEOS at `date`"
  mkdir -p ${LIB_DIR}/hdfeos
  rm -rf ${LIB_DIR}/hdfeos/HDF-EOS*
  tar -xzf ${TAR_DIR}/HDF-EOS*.tar.* -C ${LIB_DIR}/hdfeos
  cd ${LIB_DIR}/hdfeos/hdfeos
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} --with-hdf4=${LIB_DIR} --with-jpeg=${LIB_DIR} > $(pwd)/hdf-eos.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > $(pwd)/hed-eos.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/hsf-eos.make_install.log 2>&1"

  cp include/*.h ${LIB_DIR}/include/
fi

# Compile NetCDF
if [ $COMPILE_NETCDF -eq 1 ]; then
  
  echo
  echo "Compiling HDF5 at `date`"
  mkdir -p ${LIB_DIR}/hdf5
  rm -rf ${LIB_DIR}/hdf5/hdf5*
  tar -xzf ${TAR_DIR}/hdf5*.tar.gz -C ${LIB_DIR}/hdf5
  cd ${LIB_DIR}/hdf5/hdf5*
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} --with-zlib=${LIB_Z} CFLAGS=-fPIC CXXFLAGS=-fPIC FFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib:${LIB_Z} CPPFLAGS=-I${LIB_DIR}/include > $(pwd)/hdf5.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/hdf5.make_install.log 2>&1"

  echo
  echo "Compiling NetCDF-C at `date`"
  mkdir -p ${LIB_DIR}/netcdf
  rm -rf ${LIB_DIR}/netcdf/netcdf*
  tar -xzf ${TAR_DIR}/netcdf-4*.tar.gz -C ${LIB_DIR}/netcdf > /dev/null 2>&1 || unzip ${TAR_DIR}/netcdf-4*.zip -d ${LIB_DIR}/netcdf
  cd ${LIB_DIR}/netcdf/netcdf-*
  export FC=''
  export F90=''
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > $(pwd)/netcdf-c.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/netcdf-c.make_install.log 2>&1"
  
  echo
  echo "Compiling NetCDF-CXX at `date`"
  tar -xzf ${TAR_DIR}/netcdf-cxx*.tar.gz -C ${LIB_DIR}/netcdf
  cd ${LIB_DIR}/netcdf/netcdf-cxx*
  echo "cd `pwd`"
  configure_lib_args=""
  if [[ $machine == "Mac" ]]; then
    configure_lib_args="-lhdf5_hl -lhdf5 -lz"
  fi
  run_cmd "./configure --prefix=${LIB_DIR} LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include LIBS=\"${LIBS} ${configure_lib_args}\" > netcdf-cxx.configure.log 2>&1"

  run_cmd "make ${MAKE_ARGS} install > netcdf-cxx.make_install.log 2>&1"
fi

# Compile FREETYPE
if [ $COMPILE_FREETYPE -eq 1 ]; then
  echo
  echo "Compiling FREETYPE at `date`"
  mkdir -p ${LIB_DIR}/freetype
  rm -rf ${LIB_DIR}/freetype/freetype*
  tar -xzf ${TAR_DIR}/freetype*.tar.gz -C ${LIB_DIR}/freetype
  cd ${LIB_DIR}/freetype/freetype*
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} --with-png=yes > $(pwd)/freetype.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > $(pwd)/freetype.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/freetype.make_install.log 2>&1"
fi


# Compile CAIRO
if [ $COMPILE_CAIRO -eq 1 ]; then

  # If on Cray, compile PIXMAN
  if [ ${COMPILER_FAMILY} =  "PrgEnv-intel" ]; then
    echo
    echo "Compiling pixman at `date`"
    mkdir -p  ${LIB_DIR}/pixman
    rm -rf ${LIB_DIR}/pixman/pixman*
    tar -xzf ${TAR_DIR}/pixman*.tar.gz -C ${LIB_DIR}/pixman
    cd ${LIB_DIR}/pixman/pixman*
    echo "cd `pwd`"
    run_cmd "./configure --prefix=${LIB_DIR} > $(pwd)/pixman.configure.log 2>&1"
    run_cmd "make ${MAKE_ARGS} > $(pwd)/pixman.make.log 2>&1"
    run_cmd "make ${MAKE_ARGS} install > $(pwd)/pixman.make_install.log 2>&1"
  fi

  echo
  echo "Compiling CAIRO at `date`"
  mkdir -p ${LIB_DIR}/cairo
  rm -rf ${LIB_DIR}/cairo/cairo*
  tar -xf ${TAR_DIR}/cairo*.tar* -C ${LIB_DIR}/cairo
  cd ${LIB_DIR}/cairo/cairo*
  export PKG_CONFIG=`which pkg-config`
  if [ ${COMPILER_FAMILY} =  "PrgEnv-intel" ]; then
    export PKG_CONFIG_PATH=${LIB_DIR}/lib/pkgconfig/
  fi
  echo "cd `pwd`"
  run_cmd "./configure --prefix=${LIB_DIR} ax_cv_c_float_words_bigendian=no LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > $(pwd)/cairo.configure.log 2>&1"
  run_cmd "make ${MAKE_ARGS} > $(pwd)/cairo.make.log 2>&1"
  run_cmd "make ${MAKE_ARGS} install > $(pwd)/cairo.make_install.log 2>&1"
fi

# Compile MET
if [ $COMPILE_MET -eq 0 ]; then
  echo Skipping MET compilation
  echo "Finished compiling at `date`"
  exit 0
fi

echo
echo "Compiling MET at `date`"
# If using source from a tar file remove everything and unpack the tar file
# FALSE = compiling from github repo and we don't want to overwrite the files
if [ ${USE_MET_TAR_FILE} = "TRUE" ]; then
  rm -rf ${MET_DIR}/MET*
  tar -xzf ${MET_TARBALL} -C ${MET_DIR}
fi
cd ${MET_DIR}/MET*

if [ -z ${MET_BUFRLIB} ]; then
  export MET_BUFRLIB=${LIB_DIR}/lib
  export BUFRLIB_NAME=-lbufr_4
fi

if [ -z ${MET_GRIB2CLIB} ]; then
  export MET_GRIB2CLIB=${LIB_DIR}/lib
  export MET_GRIB2CINC=${LIB_DIR}/include
  export LIB_JASPER=${LIB_DIR}/lib
  export LIB_LIBPNG=${LIB_DIR}/lib
  export LIB_Z=${LIB_DIR}/lib
  export GRIB2CLIB_NAME=-lg2c
fi

if [ -z ${MET_NETCDF} ]; then
  export MET_NETCDF=${LIB_DIR}
  export MET_HDF5=${LIB_DIR}
fi

if [ -z ${MET_GSL} ]; then
  export MET_GSL=${LIB_DIR}
fi

if [ -z ${MET_PROJ} ]; then
  export MET_PROJ=${LIB_DIR}
fi

export MET_PYTHON_BIN_EXE=${MET_PYTHON_BIN_EXE:=${MET_PYTHON}/bin/python3}
export MET_PYTHON_LD
export MET_PYTHON_CC
export LDFLAGS="-Wl,--disable-new-dtags"

if [[ $machine == "Mac" ]]; then
  export LDFLAGS=""
fi

# https://www.gnu.org/software/bash/manual/html_node/Shell-Parameter-Expansion.html
# ${parameter:+word}
# If parameter is null or unset, nothing is substituted, otherwise the expansion of word is substituted.
export LDFLAGS="${LDFLAGS} -Wl,-rpath,${LIB_DIR}/lib:${MET_PROJ:+:$MET_PROJ/lib64}:${LIB_DIR}/lib${MET_NETCDF:+:$MET_NETCDF/lib}${MET_HDF5:+:$MET_HDF5/lib}${MET_BUFRLIB:+:$MET_BUFRLIB}${MET_GRIB2CLIB:+:$MET_GRIB2CLIB}${MET_PYTHON_LIB:+:$MET_PYTHON_LIB}${MET_GSL:+:$MET_GSL/lib}${ADDTL_DIR:+:$ADDTL_DIR}"
export LDFLAGS="${LDFLAGS} -Wl,-rpath,${LIB_JASPER:+$LIB_JASPER}${LIB_LIBPNG:+:$LIB_PNG}${LIB_Z:+$LIB_Z}"
export LDFLAGS="${LDFLAGS} ${LIB_JASPER:+-L$LIB_JASPER} ${LIB_LIBPNG:+-L$LIB_LIBPNG} ${MET_HDF5:+-L$MET_HDF5/lib} ${ADDTL_DIR:+-L$ADDTL_DIR}"
export LIBS="${LIBS} -lhdf5_hl -lhdf5 -lz -ltiff"
export MET_FONT_DIR=${TEST_BASE}/fonts


echo "MET Configuration settings..."
printenv | egrep "^MET_" | sed -r 's/^/export /g'
echo "LDFLAGS = ${LDFLAGS}"
export OPT_ARGS=''
if [[ $COMPILER_FAMILY == "pgi" ]]; then
  export OPT_ARGS="${OPT_ARGS} FFLAGS=-lpgf90"
fi

configure_cmd="./configure --prefix=${MET_INSTALL_DIR} --bindir=${BIN_DIR_PATH}"
configure_cmd="${configure_cmd} BUFRLIB_NAME=${BUFRLIB_NAME}"
configure_cmd="${configure_cmd} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2"
if [[ ! -z ${MET_FREETYPEINC} && ! -z ${MET_FREETYPELIB} && \
      ! -z ${MET_CAIROINC} && ! -z ${MET_CAIROLIB} ]]; then
  configure_cmd="${configure_cmd} --enable-mode_graphics"
fi

if [[ ! -z $MET_ECKIT && ! -z $MET_ATLAS ]]; then
  configure_cmd="${configure_cmd} --enable-ugrid"
fi

if [[ ! -z $MET_HDF && ! -z $MET_HDFEOS ]]; then
  configure_cmd="${configure_cmd} --enable-modis --enable-lidar2nc"
fi

if [[ ! -z ${MET_PYTHON_CC} || ! -z ${MET_PYTHON_LD} ]]; then
  configure_cmd="${configure_cmd} --enable-python"
fi

configure_cmd="${configure_cmd} ${OPT_ARGS}"

echo "cd `pwd`"
run_cmd "${configure_cmd} > $(pwd)/met.configure.log 2>&1"
run_cmd "make ${MAKE_ARGS} > $(pwd)/met.make.log 2>&1"
run_cmd "make ${MAKE_ARGS} install > $(pwd)/met.make_install.log 2>&1"
run_cmd "make ${MAKE_ARGS} test > $(pwd)/met.make_test.log 2>&1"

echo "Finished compiling at `date`"
