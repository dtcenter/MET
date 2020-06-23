#!/bin/bash 
#
# Compile and install MET
# (Model Evaluation Tools)
#================================================
#
# The compile_MET_all.sh script will compile and install
# MET and its external library dependencies, if needed.
#
# Turn on compilation of GSL, HDF, HDFEOS, CAIRO, and
# FREETYPE below using the COMPILE_<SOFTWARE> flags below.
# e.g. use COMPILE_GSL = 1 to compile GSL, use
#      COMPILE_GSL = 0 if compilation is not needed
#
# If you don't need to install GRIB2C, BUFR, and NETCDF
# please supply values for the following environment
# variables in the input file (install_met_env.<machine_name>:
# MET_GRIB2CLIB, MET_GRIB2CINC, GRIB2CLIB_NAME, MET_BUFRLIB,
# BUFRLIB_NAME, MET_NETCDF
#
# Required environment variables to set are:
# TEST_BASE, COMPILER, MET_SUBDIR, MET_TARBALL, USE_MODULES
#
# Other variables that may be set:
# SET_D64BIT, MET_PYTHON, MET_PYTHON_CC,
# MET_PYTHON_LD, MET_HDF5, MET_NETCDF
# And, if using modules: LIB_JASPER, LIB_LIBPNG,
# LIB_Z, PYTHON_MODULES
# 
# Usage: compile_MET_all.sh install_met_env.<machine_name>
#
#================================================


if [ -z $1 ]; then
    echo "No environment file provided. Starting compilation with current environment."
else
    if [ ! -f "$1" ]; then
	echo "The file \"$1\" does not exist!"
	exit 1
    else
	source $1
    fi
fi

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
export LD_LIBRARY_PATH=${TEST_BASE}/external_libs/lib:${MET_PYTHON}/lib:${MET_NETCDF}/lib:${MET_HDF5}/lib:${MET_BUFRLIB}:${MET_GRIB2CLIB}:${LIB_JASPER}:${LIB_LIBPNG}:${LIB_Z}:${LD_LIBRARY_PATH}
echo "LD_LIBRARY_PATH = ${LD_LIBRARY_PATH}"

# Constants
if [ -z ${MET_GRIB2CLIB} ]; then
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
COMPILE_GSL=1
COMPILE_HDF=1
COMPILE_HDFEOS=1
COMPILE_FREETYPE=0
COMPILE_CAIRO=0
COMPILE_MET=1

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

echo "USE_MODULES = ${USE_MODULES}"

if [ ${USE_MODULES} == "TRUE" ]; then 
    echo "module load ${COMPILER_FAMILY}/${COMPILER_VERSION}"
    echo ${COMPILER_FAMILY}/${COMPILER_VERSION}

    module load ${COMPILER_FAMILY}/${COMPILER_VERSION}
    if [ ${COMPILER_FAMILY} ==  "PrgEnv-intel" ]; then
	module load craype
	module switch craype craype-sandybridge
    fi
fi

if [ ${COMPILER_FAMILY} == "gnu" ]; then
  export CC=`which gcc`
  export CXX=`which g++`
  export FC=`which gfortran`
  export F77=`which gfortran`
  export F90=`which gfortran`
elif [ ${COMPILER_FAMILY} == "pgi" ]; then
  export CC=`which pgcc`
  export CXX=`which pgc++`
  export FC=`which pgf90`
  export F77=`which pgf90`
  export F90=`which pgf90`
elif [[ ${COMPILER_FAMILY} == "intel" ]] || [[ ${COMPILER_FAMILY} == "ics" ]] || [[ ${COMPILER_FAMILY} == "ips" ]] || [[ ${COMPILER_FAMILY} ==  "PrgEnv-intel" ]]; then
  export CC=`which icc`
  export CXX=`which icc`
  export FC=`which ifort`
  export F77=`which ifort`
  export F90=`which ifort`
else
  echo "ERROR: \${COMPILER} must start with gnu, intel, ics, ips, PrgEnv-intel, or pgi"
  exit
fi

echo "export  CC=${CC}"
echo "export CXX=${CXX}"
echo "export  FC=${FC}"
echo "export F77=${F77}"
echo "export F90=${F90}"


# Load Python module

if [ ${USE_MODULES} == "TRUE" ]; then
    if [ ! -z ${PYTHON_MODULE} ]; then
	PYTHON_NAME=` echo $PYTHON_MODULE | cut -d'_' -f1`
	PYTHON_VERSION_NUM=`echo $PYTHON_MODULE | cut -d'_' -f2`
	echo "module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}"
	echo ${PYTHON_NAME}/${PYTHON_VERSION_NUM} 
	module load ${PYTHON_NAME}/${PYTHON_VERSION_NUM}
    fi
fi

echo "Using python version: "
python --version

# Compile GSL
if [ $COMPILE_GSL -eq 1 ]; then

  if [ ${COMPILER_FAMILY} == "pgi" ]; then
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
    ${FC} -c -fno-second-underscore `./getdefflags_F.sh` modv*.F moda*.F `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log 2>&1
  elif [[ ${COMPILER_FAMILY} == "intel" ]] || [[ ${COMPILER_FAMILY} == "ics" ]] || [[ ${COMPILER_FAMILY} == "ips" ]] || [[ ${COMPILER_FAMILY} == "ips" ]] || [[ ${COMPILER_FAMILY} ==  "PrgEnv-intel" ]]; then
    ${FC} -c `./getdefflags_F.sh` modv*.F moda*.F `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log 2>&1
  elif [[ ${COMPILER_FAMILY} == "pgi" ]]; then
    ${FC} -c -Mnosecond_underscore `./getdefflags_F.sh` modv*.F moda*.F `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log 2>&1
  fi

  ar crv libbufr.a *.o >> make.log 2>&1
  cp *.a ${LIB_DIR}/lib/.
  export BUFRLIB_NAME=-lbufr
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
  echo "./configure --prefix=${LIB_DIR} LDFLAGS=-L/${LIB_DIR}/lib CPPFLAGS=-I/${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} LDFLAGS=-L/${LIB_DIR}/lib CPPFLAGS=-I/${LIB_DIR}/include > configure.log 2>&1
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
  export GRIB2CLIB_NAME=-lgrib2c
fi

# Compile HDF
# Depends on jpeg
# Edit 'mfhdf/hdiff/Makefile' as follows:
#  From: LIBS = -ljpeg -lz
#    To: LIBS = -ljpeg -lz -lm
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
  cat mfhdf/hdiff/Makefile | sed 's/LIBS = -ljpeg -lz/LIBS = -ljpeg -lz -lm/g' > Makefile_NEW
  mv Makefile_NEW mfhdf/hdiff/Makefile
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
  cp include/*.h ${LIB_DIR}/include/.
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
  echo "./configure --prefix=${LIB_DIR} --with-zlib=${LIB_DIR}/lib CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} --with-zlib=${LIB_Z} CFLAGS=-fPIC CXXFLAGS=-fPIC LDFLAGS=-L${LIB_DIR}/lib:{LIB_Z} CPPFLAGS=-I${LIB_DIR}/include > configure.log 2>&1
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
  unzip ${TAR_DIR}/netcdf-4*.zip > /dev/null 2>&1
  cd netcdf-4*
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
    if [ ${COMPILER_FAMILY} ==  "PrgEnv-intel" ]; then
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
  if [ ${COMPILER_FAMILY} ==  "PrgEnv-intel" ]; then
      export PKG_CONFIG_PATH=${LIB_DIR}/lib/pkgconfig/
  fi
  echo "cd `pwd`"
  echo "./configure --prefix=${LIB_DIR} ax_cv_c_float_words_bigendian=no LDFLAGS=-L/${LIB_DIR}/lib CPPFLAGS=-I/${LIB_DIR}/include > configure.log 2>&1"
  ./configure --prefix=${LIB_DIR} ax_cv_c_float_words_bigendian=no LDFLAGS=-L/${LIB_DIR}/lib CPPFLAGS=-I/${LIB_DIR}/include > configure.log 2>&1
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
  if [ ${USE_MET_TAR_FILE} == "TRUE" ]; then
    rm -rf met*
    tar -xzf ${MET_TARBALL}
  fi
  cd met*

  if [ -z ${MET_BUFRLIB} ]; then
      export MET_BUFRLIB=${LIB_DIR}/lib
  else
      export MET_BUFRLIB=${MET_BUFRLIB}
  fi

  if [ -z ${MET_GRIB2CLIB} ]; then
      export MET_GRIB2C=${LIB_DIR}
  else
      export MET_GRIB2CLIB=${MET_GRIB2CLIB}
      export MET_GRIB2CINC=${MET_GRIB2CINC}
  fi
  
  if [ -z ${MET_NETCDF} ]; then
      export MET_NETCDF=${LIB_DIR}
      export MET_HDF5=${LIB_DIR}
  else
      export MET_NETCDF=${MET_NETCDF}
      export MET_HDF5=${MET_HDF5}
  fi
  
  export MET_GSL=${LIB_DIR}
  export MET_HDF=${LIB_DIR}
  export MET_HDFEOS=${LIB_DIR}
  export MET_PYTHON_LD=${MET_PYTHON_LD}
  export MET_PYTHON_CC=${MET_PYTHON_CC}
  export LDFLAGS="-Wl,--disable-new-dtags"
  export LDFLAGS="${LDFLAGS} -Wl,-rpath,${LIB_DIR}/lib:${MET_NETCDF}/lib:${MET_HDF5}/lib:${MET_BUFRLIB}:${MET_GRIB2CLIB}:${MET_PYTHON}/lib"
  export LDFLAGS="${LDFLAGS} -Wl,-rpath,${LIB_JASPER}:${LIB_LIBPNG}:${LIB_Z}"
  export LDFLAGS="${LDFLAGS} -L${LIB_JASPER} -L${MET_HDF5}/lib"
  #export LIBS="${LIBS} -lhdf5_hl -lhdf5 -lz"
  export MET_FONT_DIR=${TEST_BASE}/fonts

  if [ ${SET_D64BIT} == "TRUE" ]; then
      export CFLAGS="-D__64BIT__"
      export CXXFLAGS="-D__64BIT__"
  fi

  if [[ $COMPILE_CAIRO -eq 1 && $COMPILE_FREETYPE -eq 1 ]]; then
      export MET_CAIROINC=${LIB_DIR}/include/cairo
      export MET_CAIROLIB=${LIB_DIR}/lib
      export MET_FREETYPEINC=${LIB_DIR}/include/freetype2
      export MET_FREETYPELIB=${LIB_DIR}/lib
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
      if [[ $COMPILE_CAIRO -eq 1 && $COMPILE_FREETYPE -eq 1 ]]; then
	  echo "./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1"
	  ./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1
      else
	  echo "./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1"
	  ./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc ${OPT_ARGS} > configure.log 2>&1
      fi
  else
      if [[ $COMPILE_CAIRO -eq 1 && $COMPILE_FREETYPE -eq 1 ]]; then
	  echo "./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.log 2>&1"
	  ./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-mode_graphics --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.log 2>&1
      else
	  echo "./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.log 2>&1"
	  ./configure --prefix=${MET_DIR} BUFRLIB_NAME=${BUFRLIB_NAME} GRIB2CLIB_NAME=${GRIB2CLIB_NAME} --enable-grib2 --enable-modis --enable-lidar2nc --enable-python ${OPT_ARGS} > configure.log 2>&1
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

