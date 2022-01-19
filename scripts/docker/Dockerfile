ARG MET_BASE_IMAGE=minimum

FROM dtcenter/met-base:${MET_BASE_IMAGE}
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
# Set the working directory.
#
WORKDIR /met

#
# Download and install MET and GhostScript fonts.
# Delete the MET source code for tagged releases matching "v"*.
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
    MET_HDF=/usr/local/hdf MET_HDFEOS=/usr/local/hdfeos \
    MET_FREETYPEINC=/usr/include/freetype2 MET_FREETYPELIB=/usr/lib \
    MET_CAIROINC=/usr/include/cairo MET_CAIROLIB=/usr/lib \
    MET_PYTHON_CC='-I/usr/include/python3.6m' MET_PYTHON_LD='-lpython3.6m' > ${LOG_FILE} \
 && LOG_FILE=/met/MET-${MET_GIT_NAME}/make_install.log \
 && echo "Compiling MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}" \
 && make install > ${LOG_FILE} \
 && LOG_FILE=/met/logs/MET-${MET_GIT_NAME}_make_test.log \
 && echo "Testing MET ${MET_GIT_NAME} and writing log file ${LOG_FILE}" \
 && make test > ${LOG_FILE} 2>&1 \
 && if [[ $MET_GIT_NAME == "v"* ]]; then cd /met; rm -rf MET-*; fi \
 && echo "Downloading GhostScript fonts from ${GSFONT_URL}" \
 && curl -SL ${GSFONT_URL} | tar zxC /usr/local/share/met
