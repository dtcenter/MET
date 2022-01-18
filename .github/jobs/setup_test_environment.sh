#! /bin/bash

######################################################################
# Install tools needed to run MET unit tests in Docker
######################################################################

###
# Install Perl XML Parser
###
yum makecache
yum -y install perl-XML-Parser


###
# Install R
###
#yum -y install epel-release # already installed in MET Docker image
yum -y install R


###
# Install R NetCDF4 library (1.19)
###
yum -y install netcdf-devel.x86_64

# download tarfile into home directory
cd
wget https://cran.r-project.org/src/contrib/ncdf4_1.19.tar.gz
R CMD INSTALL ncdf4_1.19.tar.gz


###
# Install NCO to get ncdiff
###
yum -y install nco


###
# Set environment variables needed to run unit tests
###

export MET_BASE=/usr/local/share/met

export MET_REPO_DIR=/met/MET-${MET_GIT_NAME}
export MET_BUILD_BASE=${MET_REPO_DIR}/met
export MET_TEST_BASE=${MET_REPO_DIR}/test
export PERL5LIB=${MET_TEST_BASE}/lib

export MET_TEST_INPUT=/data/input/MET_test_data/unit_test
export MET_TEST_OUTPUT=/data/output/met_test_output

export MET_TEST_RSCRIPT=/usr/bin/Rscript
export MET_TEST_MET_PYTHON_EXE=/usr/bin/python3
