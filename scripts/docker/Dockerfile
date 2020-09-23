FROM centos:7
MAINTAINER John Halley Gotway <johnhg@ucar.edu>

#
# This Dockerfile checks out MET from GitHub and compiles the specified branch or tag from source.
#
ARG SOURCE_BRANCH

#
# SOURCE_BRANCH is not defined when built via Docker Hub.
#
RUN if [ "x${SOURCE_BRANCH}" = "x" ]; then \
      echo "ERROR: SOURCE_BRANCH undefined! Rebuild with \"--build-arg SOURCE_BRANCH={branch name}\""; \
      exit 1; \
    else \
      echo "Build Argument SOURCE_BRANCH=${SOURCE_BRANCH}"; \
    fi

ENV MET_GIT_NAME    ${SOURCE_BRANCH}
ENV MET_GIT_URL     https://github.com/dtcenter/MET
ENV MET_DEVELOPMENT true

#
# Define the compilers.
#
ENV CC  /usr/bin/gcc
ENV CXX /usr/bin/g++
ENV FC  /usr/bin/gfortran
ENV F77 /usr/bin/gfortran

#
# Define package URL's.
#
ENV HDF4_URL       http://www.hdfgroup.org/ftp/HDF/releases/HDF4.2r3/src/HDF4.2r3.tar.gz
ENV HDFEOS_URL     https://dtcenter.ucar.edu/dfiles/code/METplus/MET/docker_data/HDF-EOS2.16v1.00.tar.Z

ENV NETCDF4C_URL   https://github.com/Unidata/netcdf-c/archive/v4.4.1.1.zip
ENV NETCDF4CXX_URL https://github.com/Unidata/netcdf-cxx4/archive/v4.3.0.tar.gz

ENV BUFRLIB_URL    https://dtcenter.ucar.edu/dfiles/code/METplus/MET/docker_data/BUFRLIB_v10-2-3.tar
ENV GSFONT_URL     https://dtcenter.ucar.edu/dfiles/code/METplus/MET/docker_data/ghostscript-fonts-std-8.11.tar.gz

#
# Install the required packages.
#
RUN yum -y update \
 && yum -y install file gcc gcc-gfortran gcc-c++ glibc.i686 libgcc.i686 \
                   libpng-devel jasper jasper-devel zlib zlib-devel \
                   cairo-devel freetype-devel epel-release \
                   hostname m4 make tar tcsh ksh time wget which \
                   flex flex-devel bison bison-devel unzip \
 && yum -y install git g2clib-devel hdf5-devel.x86_64 gsl-devel \
 && yum -y install gv ncview wgrib wgrib2 ImageMagick ps2pdf \
 && yum -y install python3 python3-devel python3-pip \
 && pip3 install --upgrade pip \
 && python3 -m pip install numpy xarray

#
# Set the working directory.
#
WORKDIR /met

#
# Setup the environment for interactive bash/csh container shells.
#
RUN echo export MET_BASE=/usr/local/share/met >> /etc/bashrc \
 && echo setenv MET_BASE /usr/local/share/met >> /etc/csh.cshrc \
 && echo export MET_FONT_DIR=/usr/local/share/met/fonts >> /etc/bashrc \
 && echo setenv MET_FONT_DIR /usr/local/share/met/fonts >> /etc/csh.cshrc \
 && echo export RSCRIPTS_BASE=/usr/local/share/met/Rscripts >> /etc/bashrc \
 && echo setenv RSCRIPTS_BASE /usr/local/share/met/Rscripts >> /etc/csh.cshrc \
 && echo export LD_LIBRARY_PATH=/usr/local/lib >> /etc/bashrc \
 && echo setenv LD_LIBRARY_PATH /usr/local/lib >> /etc/csh.cshrc
ENV LD_LIBRARY_PATH /usr/local/lib

#
# Download and install BUFRLIB.
#
RUN mkdir -p /met/logs \
 && mkdir -p /met/external_libs/BUFRLIB \
 && cd /met/external_libs/BUFRLIB \
 && echo "Downloading BUFRLIB from ${BUFRLIB_URL}" \
 && curl -SL ${BUFRLIB_URL} | tar xC /met/external_libs/BUFRLIB \
 && cat preproc.sh | sed 's/cpp /cpp -traditional-cpp /g' > preproc_patch.sh \
 && chmod +x preproc_patch.sh \
 && LOG_FILE=/met/logs/BUFRLIB_build.log \
 && echo "Compiling BUFRLIB and writing log file ${LOG_FILE}" \
 && ./preproc_patch.sh *.F > ${LOG_FILE} \
 && ${CC} -c -DUNDERSCORE *.c >> ${LOG_FILE} \
 && ${FC} -c -fno-second-underscore *.f >> ${LOG_FILE} \
 && ar crv libbufr.a *.o >> ${LOG_FILE} \
 && rm -f /usr/local/lib/libbufr.a \
 && cp  *.a /usr/local/lib \
 && cd /met/external_libs \
 && rm -rf BUFRLIB

#
# Download and install NetCDF4 (C and C++).
#
RUN mkdir -p /met/external_libs/netcdf \
 && cd /met/external_libs/netcdf \
 && echo "Downloading netcdf-c-4.4.1.1 from ${NETCDF4C_URL}" \
 && wget ${NETCDF4C_URL} \
 && unzip v4.4.1.1.zip \
 && cd netcdf-c-4.4.1.1 \
 && LOG_FILE=/met/logs/netcdf-c-4.4.1.1_configure.log \
 && echo "Configuring netcdf-c-4.4.1.1 and writing log file ${LOG_FILE}" \
 && ./configure > ${LOG_FILE} \
 && LOG_FILE=/met/logs/netcdf-c-4.4.1.1_make_install.log \
 && echo "Compiling netcdf-c-4.4.1.1 and writing log file ${LOG_FILE}" \
 && make install > ${LOG_FILE} \
 && echo "Downloading  from ${NETCDF4CXX_URL}" \
 && cd /met/external_libs/netcdf \
 && wget ${NETCDF4CXX_URL} \
 && tar -xzf v4.3.0.tar.gz \
 && cd netcdf-cxx4-4.3.0 \
 && LOG_FILE=/met/logs/netcdf-cxx4-4.3.0_configure.log \
 && echo "Configuring netcdf-cxx4-4.3.0 and writing log file ${LOG_FILE}" \
 && ./configure > ${LOG_FILE} \
 && LOG_FILE=/met/logs/netcdf-cxx4-4.3.0_make_install.log \
 && echo "Compiling netcdf-cxx4-4.3.0 and writing log file ${LOG_FILE}" \
 && make install > ${LOG_FILE} \
 && cd /met/external_libs \
 && rm -rf netcdf

#
# Download and install HDF4 and HDFEOS.
#
RUN echo "Downloading HDF4.2r3 from ${HDF4_URL}" \
 && curl -SL ${HDF4_URL} | tar zxC /met/external_libs \
 && cd /met/external_libs/HDF4.2r3 \
 && LOG_FILE=/met/logs/HDF4.2r3_configure.log \
 && echo "Configuring HDF4.2r3 and writing log file ${LOG_FILE}" \
 && ./configure --prefix=/usr/local --disable-netcdf > ${LOG_FILE} \
 && cat mfhdf/hdiff/Makefile | sed 's/LIBS = -ljpeg -lz/LIBS = -ljpeg -lz -lm/g' > Makefile_NEW \
 && mv -f Makefile_NEW mfhdf/hdiff/Makefile \
 && LOG_FILE=/met/logs/HDF4.2r3_make_install.log \
 && echo "Compiling HDF4.2r3 and writing log file ${LOG_FILE}" \
 && make install > ${LOG_FILE} \
 && echo "Downloading hdfeos from ${HDFEOS_URL}" \
 && curl -SL ${HDFEOS_URL} | tar zxC /met/external_libs \
 && cd /met/external_libs/hdfeos \
 && LOG_FILE=/met/logs/hdfeos_configure.log \
 && echo "Configuring hdfeos and writing log file ${LOG_FILE}" \
 && ./configure --prefix=/usr/local --with-hdf4=/usr/local CC=/usr/local/bin/h4cc > ${LOG_FILE} \
 && LOG_FILE=/met/logs/hdfeos_make_install.log \
 && echo "Compiling hdfeos and writing log file ${LOG_FILE}" \
 && make install > ${LOG_FILE} \
 && cp include/*.h /usr/local/include/. \
 && cd /met/external_libs \
 && rm -rf HDF4.2r3 hdfeos

#
# Download and install MET and GhostScript fonts.
# Delete the MET source code for tagged releases matching "met-"*.
#
RUN echo "Checking out MET ${MET_GIT_NAME} from ${MET_GIT_URL}" \
 && git clone ${MET_GIT_URL} /met/MET-${MET_GIT_NAME} \
 && cd /met/MET-${MET_GIT_NAME}/met \
 && git checkout ${MET_GIT_NAME} \
 && LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_configure.log \
 && echo "Running bootstrap" \
 && ./bootstrap \
 && echo "Configuring MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}" \
 && ./configure --enable-grib2 --enable-mode_graphics --enable-modis --enable-lidar2nc --enable-python \
    MET_FREETYPEINC=/usr/include/freetype2 MET_FREETYPELIB=/usr/lib \
    MET_CAIROINC=/usr/include/cairo MET_CAIROLIB=/usr/lib \
    MET_PYTHON_CC='-I/usr/include/python3.6m' MET_PYTHON_LD='-lpython3.6m' > ${LOG_FILE} \
 && LOG_FILE=/met/MET-${MET_GIT_NAME}/make_install.log \
 && echo "Compiling MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}" \
 && make install > ${LOG_FILE} \
 && LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_test.log \
 && echo "Testing MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}" \
 && make test > ${LOG_FILE} 2>&1 \
 && if [[ $MET_GIT_NAME == "met-"* ]]; then cd /met; rm -rf MET-*; fi \
 && echo "Downloading GhostScript fonts from ${GSFONT_URL}" \
 && curl -SL ${GSFONT_URL} | tar zxC /usr/local/share/met
