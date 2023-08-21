.. _installation:

*********************
Software Installation
*********************

Introduction
============

This section is meant to provide guidance on installing MET. It assumes a beginner user to MET is compiling MET from scratch and will step through the installation process, including listing dependent libraries and how to install them. Installation will require an understanding of the environment and hardware that MET is being installed on as it has options that are dependent on compiler usage, modulefiles, etc.

There are multiple supported methods for installing MET: using a provided compilation script, Docker instances, and Apptainer/Singularity instances. All of these methods will be described below. 
The recommended method is :ref:`compile_script_install` .

.. required_external_libraries_to_build_MET:

Required External Libraries To Build MET
========================================

MET is dependent on several external libraries to function properly. The required libraries are listed below:

* `BUFRLIB <https://emc.ncep.noaa.gov/emc/pages/infrastructure/bufrlib.php>`_ for reading PrepBufr Observation files
* `NetCDF4 <http://www.unidata.ucar.edu/software/netcdf>`_ in C and CXX, for intermediate and output file formats
* `HDF5 <https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/hdf5-1.12.2.tar.gz>`_ is required to support NetCDF 4. HDF5 should be built with `zlib <http://www.zlib.net/>`_.
* `GSL <http://www.gnu.org/software/gsl>`_ GNU Scientific Library Developer's Version for computing confidence intervals (use **GSL-1.11** for **PGI** compilers)
* `GRIB2C <http://www.nco.ncep.noaa.gov/pmb/codes/GRIB2>`_ Library (with dependencies Z, PNG, JASPER), if compiling GRIB2 support.

The following libraries are conditionally required, depending on your intended verification use and compiler language:

* `HDF4 <http://www.hdfgroup.org/products/hdf4>`_ Library, if compiling the MODIS-Regrid or lidar2nc tool.
* `HDF-EOS2 <http://www.hdfgroup.org/hdfeos.html>`_ Library, if compiling the MODIS-Regrid or lidar2nc tool.
* `Cairo <http://cairographics.org/releases>`_ Library, if compiling the MODE-Graphics tool.
* `FreeType <http://www.freetype.org/download.html>`_ Library, if compiling the MODE-Graphics tool.
* `f2c <http://www.netlib.org/f2c>`_ Library for interfacing between Fortran and C (**Not required for most compilers**)

Users can take advantage of the :ref:`compile_script_install` to download and install all of the libraries automatically, both required and conditionally required.

Suggested External Utilities for Use With MET
=============================================

The following utilities have been used with success by other METplus users in their verification processes. They are not required for MET to function, but depending on the user’s intended verification needs, they may be of use:

* `Unified Post Processing System (UPP) <https://dtcenter.org/community-code/unified-post-processor-upp>`_ for preparing model data to be verified
* `copygb utility <http://www.cpc.ncep.noaa.gov/products/wesley/copygb.html>`_ for re-gridding grib data (available with the WRF Post-Processor)
* `Integrated Data Viewer (IDV) <http://www.unidata.ucar.edu/software/idv>`_ for displaying gridded data, including GRIB and NetCDF
* `ncview utility <http://meteora.ucsd.edu/~pierce/ncview_home_page.html>`_ for viewing gridded NetCDF data (ex. the output of pcp_combine)

.. _compile_script_install:

Using the compile_MET_all.sh Script for Installing MET
======================================================

The compile_MET_all.sh script is designed to install MET and, if desired, all of the external library dependencies required for running the system on various platforms. There are some required environment variables that need to be set before running the script and some optional environment variables, both of which are described below. The script relies on the tar_files.tgz, which contains all of the required and optional library packages for MET but not MET itself. A separate command will be used to pull down the latest version of MET in tar.tgz format from GitHub, which the script will then install.

To begin, create and change to a directory where you want to install the latest version of MET. Next, download the script, compile_MET_all.sh, and tar_files.tgz and place both of these in the new directory. These files are available either through using the hyperlinks provided, or by entering the following commands in the terminal while in the directory MET will be installed in:

.. code-block:: ini

  wget https://raw.githubusercontent.com/dtcenter/MET/main_v11.1/internal/scripts/installation/compile_MET_all.sh
  wget https://dtcenter.ucar.edu/dfiles/code/METplus/MET/installation/tar_files.tgz


The tar files will need to be extracted in the MET installation directory:

.. code-block:: ini

  tar -zxf tar_files.tgz

To make the compilation script into an executable, change the permissions to the following:

.. code-block:: ini

  chmod 775 compile_MET_all.sh

Now change directories to the one that was created from expanding the tar files:

.. code-block:: ini

  cd tar_files

The next step will be to identify and download the latest MET release as a tar file (e.g. v11.1.0.tar.gz) and place it in the tar_files directory. The file is available from the Recommended-Components MET section of the `METplus website <https://dtcenter.org/community-code/metplus/download>`_ or by using a wget command while in the tar_files directory:

.. code-block:: ini

  wget https://github.com/dtcenter/MET/archive/refs/tags/v11.1.0.tar.gz


.. _Install_Required-libraries-and:

Environment Variables to Run Script:
------------------------------------

Before running the compilation script, there are five environment variables that are required: 
TEST_BASE, COMPILER, MET_SUBDIR, MET_TARBALL, and USE_MODULES.  
If compiling support for Python embedding, the script will need the following additional environment variables: MET_PYTHON, MET_PYTHON_CC, and MET_PYTHON_LD.

An easy way to set these environment variables is in an environment configuration file 
(for example, install_met_env.<machine_name>). An example environment configuration file to start 
from (install_met_env.generic), as well as environment configuration files used on 
HPCs at NCAR and NOAA, can be found in the MET GitHub repository in the 
`scripts/installation/config 
<https://github.com/dtcenter/MET/tree/main_v11.1/internal/scripts/installation/config>`_ 
directory.

Environment Variable Descriptions:
----------------------------------

REQUIRED:
^^^^^^^^^

**TEST_BASE** – Format is */d1/met/11.1.0*. This is the MET installation directory that was 
created in the first step, and contains **compile_MET_all.sh** script, **tar_files.tgz**, 
and the *tar_files* directory from the untar command.

**COMPILER** – Format is compiler_version (ex. gnu_8.3.0). For the GNU family of compilers, 
use “gnu”; for the Intel family of compilers, use “intel”, “ics”, “ips”, or “PrgEnv-intel”, 
depending on the system. In the past, support was provided for the PGI family of compilers 
through “pgi”. However, this compiler option is no longer actively tested. 

**MET_SUBDIR** – Format is */d1/met/11.1.0*. This is the location where the top-level 
MET subdirectory will be installed and is often set equivalent to TEST_BASE (e.g. ${TEST_BASE}).

**MET_TARBALL** – Format is **v11.1.0.tar.gz**. This is the name of the downloaded MET tarball.

**USE_MODULES** – Format is TRUE or FALSE. Set to FALSE if using a machine that does not use 
modulefiles; set to TRUE if using a machine that does use modulefiles. For more information on 
modulefiles, visit the `wiki page <https://en.wikipedia.org/wiki/Environment_Modules_(software)>`_.

**PYTHON_MODULE** Format is PythonModule_version (ex. python_3.8.6). This environment variable 
is only required if USE_MODULES=TRUE. To set properly, list the Python module to load 
followed by an underscore and version number. For example, setting PYTHON_MODULE=python_3.10.4 
will cause the script to  run "module load python/3.10.4".

REQUIRED IF COMPILING PYTHON EMBEDDING:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**MET_PYTHON** – Format is */usr/local/python3*. This is the location containing the 
bin, include, lib, and share directories for Python.

**MET_PYTHON_CC** - Format is -I followed by the directory containing Python includes 
(ex. -I/usr/local/python3/include/python3.8). This information may be obtained by 
running python3-config --cflags; however, this command can, on certain systems, 
provide too much information.

**MET_PYTHON_LD** - Format is -L followed by the directory containing the Python library 
files then a space, then -l followed by the necessary Python libraries to link to 
(ex. -L/usr/local/python3/lib/\ -lpython3.8\ -lpthread\ -ldl\ -lutil\ -lm). 
The backslashes are necessary in the example shown because of the spaces, which will be 
recognized as the end of the value unless preceded by the “\” character. Alternatively, 
a user can provide the value in quotations 
(ex. export MET_PYTHON_LD="-L/usr/local/python3/lib/ -lpython3.8 -lpthread -ldl -lutil -lm"). 
This information may be obtained by running 
python3-config --ldflags; however, this command can, on certain systems, provide too much information.

OPTIONAL:
^^^^^^^^^

**export MAKE_ARGS=-j #** – If there is a need to install external libraries, or to attempt 
to speed up the MET compilation process, this environmental setting can be added to the 
environment configuration file. Replace the # with the number of cores to use 
as an integer) or simply specify “export MAKE_ARGS=-j” with no integer argument to 
start as many processes in parallel as possible. 

External Library handling in compile_MET_all.sh
-----------------------------------------------

IF THE USER WANTS TO HAVE THE COMPILATION SCRIPT DOWNLOAD THE LIBRARY DEPENDENCIES:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The **compile_MET_all.sh** script will compile and install MET and its required external 
library dependencies required_external_libraries_to_build_MET 
:ref:`required_external_libraries_to_build_MET`, if needed. 
Note that if these libraries are already installed somewhere on the system, 
MET will call and use the libraries that were installed by the script. 

IF THE USER ALREADY HAS THE LIBRARY DEPENDENCIES INSTALLED:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the required external library dependencies have already been installed and don’t 
need to be reinstalled, or if compiling MET on a machine that uses modulefiles and 
the user would like to make use of the existing dependent libraries on that machine, 
there are more environment variables that need to be set to let MET know where those 
library and header files are. The following environment variables need to be added 
to the environment configuration file: 
MET_GRIB2CLIB, MET_GRIB2CINC, GRIB2CLIB_NAME, MET_BUFRLIB, BUFRLIB_NAME, MET_HDF5, 
MET_NETCDF, MET_GSL, LIB_JASPER, LIB_PNG, LIB_Z. 

Generally speaking, for each library there is a set of three environment variables to 
describe the locations: 
$MET_<lib>, $MET_<lib>INC and $MET_<lib>LIB.

The $MET_<lib> environment variable can be used if the external library is 
installed such that there is a main directory which has a subdirectory called 
“lib” containing the library files and another subdirectory called 
“include” containing the include files.

Alternatively, the $MET_<lib>INC and $MET_<lib>LIB environment variables are used if the 
library and include files for an external library are installed in separate locations. 
In this case, both environment variables must be specified and the associated 
$MET_<lib> variable will be ignored.

FINAL NOTE ON EXTERNAL LIBRARIES:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For users wishing to run the Plot-MODE-Field tool, the Ghostscript font data must be 
downloaded into the TEST_BASE directory and set the MET_FONT_DIR environment variable 
in the install_met_env.<machine_name> file  to point to the directory containing those fonts.

Executing the compile_MET_all.sh script
---------------------------------------

With the proper files downloaded and the environment configuration file set to the 
particular system’s needs, MET is ready for installation. 
Simply enter the following into the terminal to execute the script:

.. code-block:: ini

  ./compile_MET_all.sh install_met_env.<machine_name>

To confirm that MET was installed successfully, users are encouraged to run 
the sample test scripts :ref:`sample-test-cases` {link to section below here}.

Due to the highly variable nature of hardware systems, users may encounter issues during 
the installation process that result in MET not being installed. If this occurs please 
first recheck that the locations of all the necessary data files and scripts is correct. 
Next, recheck the environment variables in the environment configuration file and 
ensure there are no spelling errors or improperly set variables. 
After these checks are complete, run the script again.

If there are still errors, users still have options to obtain a successful 
MET installation. Check the `FAQ section of the User’s Guide on topics relevant to installation <https://met.readthedocs.io/en/latest/Users_Guide/appendixA.html#met-won-t-compile>`_. 
Next, review previously asked questions on the installation topic in 
`GitHub Discussions <https://github.com/dtcenter/METplus/discussions/categories/installation>`_. 
Users are welcome to post any questions they might have that have not been asked. 
Finally, consider one of the remaining installation methods for MET, 
as these may prove more successful.

Using Docker for Running MET
----------------------------

Docker is a system that seeks to eliminate some of the complexities associated with 
downloading various software and any library dependencies it might have by allowing 
users to run inside a preset container. Instead of using a hard copy of an application, 
Docker allows users to pull images of the application and run those within the 
Docker environment. This is beneficial to both developers (who no longer have to 
design with every possible system environment in mind) and users (who can skip tracking 
down system environment settings and meet with success faster) alike.

MET has numerous version images for Docker users and continues to be released as 
images at the same interval as system releases. While the advantages of Docker can 
make it an appealing installation route for first time users, it does require 
privileged user access that will result in an unsuccessful installation if not 
available. You should ensure that you have high system access (e.g. admin access) 
before attempting this method.

Installing Docker
-----------------

To begin, you will need to download and install the correct version of Docker 
for your system. The 
`Docker installation webpage <https://www.docker.com/>`_ should detect what 
system you are using to access the webpage and auto select the appropriate version. 
If you require a different version, select the correct version from the dropdown option. 
Follow Docker’s instructions for a successful installation.

Loading the Latest Docker Image of MET
--------------------------------------

Once you have confirmed your installation of Docker was successful, 
all you need to run MET is to download the latest image of MET in Docker. 
To accomplish that, use the pull command:

.. code-block:: ini

  docker pull dtcenter/met

Which will automatically pull the latest Docker image of MET. 
f you encounter an error, try adding the latest version number, for example:

.. code-block:: ini

  docker pull dtcenter/met:11.1.0

Running the Docker version of MET
---------------------------------

All that’s left to do is launch a shell in the Docker container. 
This is accomplished with the command:

.. code-block:: ini

  docker run -it --rm dtcenter/met /bin/bash

Note that the --rm command was added to automatically remove the container created 
from the image once you exit Docker. Simply remove this command if you’d like the 
container to persist after exiting. If there is an error during this run command, 
try adding the latest MET version number the same way you pulled the latest image of MET:

.. code-block:: ini

  docker run -it --rm dtcenter/met:11.1.0 /bin/bash 

If you were successful with the Docker usage of MET, it is highly recommended to move on 
to using the METplus wrappers of the tools, which have their own Docker image. 
Instructions for  obtaining that image are in the 
`METplus Wrappers User Guide <https://metplus.readthedocs.io/en/latest/Users_Guide/getting_started.html#metplus-in-docker>`_.

Using Apptainer for Running MET
===============================

.. _sample-test-cases:

Sample Test Cases
-----------------

Once the MET package has been built successfully, the user is encouraged to run the 
sample test scripts provided. They are run using make test in the top-level directory. 
Execute the following commands:

1. Type ‘make test >& make_test.log &’ to run all of the test scripts in the 
directory. These test scripts use test data supplied with the tarball. For instructions 
on running your own data, please refer to the MET User’s Guide.
2. Type ‘tail -f make_test.log’ to view the execution of the test script.
3. When the test script is finished, type ‘CTRL-C’ to quit the tail. Look in “out” 
to find the output files for these tests. Each tool has a separate, appropriately 
named subdirectory for its output files.
4. In particular, check that the PB2NC tool ran without error. If there was an error, 
run “make clean” then rerun your configure command adding –disable-block4 to your 
configure command line and rebuild MET.

Now that you’ve successfully installed MET, it is highly recommended to next 
install the METplus wrappers to take full advantage of Python integration. 
You can also proceed to the Tutorial and run through the examples that only utilize 
the MET processes 
(METplus wrapper applications and commands will not work unless you have installed METplus wrappers).

Required Libraries and Optional Utilities
=========================================

Three external libraries are required for compiling/building MET and should be downloaded and installed before attempting to install MET. Additional external libraries required for building advanced features in MET are discussed in :numref:`Installation-of-required` :

1. NCEP's BUFRLIB is used by MET to decode point-based observation datasets in PrepBUFR format. BUFRLIB is distributed and supported by NCEP and is freely available for download from `NCEP's BUFRLIB website <https://emc.ncep.noaa.gov/emc/pages/infrastructure/bufrlib.php>`_. BUFRLIB requires C and Fortran-90 compilers that should be from the same family of compilers used when building MET.

2. Several tools within MET use Unidata's NetCDF libraries for writing output NetCDF files. NetCDF libraries are distributed and supported by Unidata and are freely available for download from `Unidata's NetCDF website <http://www.unidata.ucar.edu/software/netcdf>`_. The same family of compilers used to build NetCDF should be used when building MET. MET is now compatible with the enhanced data model provided in NetCDF version 4. The support for NetCDF version 4 requires NetCDF-C, NetCDF-CXX, and HDF5, which is freely available for download on the `HDF5 webpage <https://support.hdfgroup.org/HDF5/>`_.

3. The GNU Scientific Library (GSL) is used by MET when computing confidence intervals. GSL is distributed and supported by the GNU Software Foundation and is freely available for download from the `GNU website <http://www.gnu.org/software/gsl>`_. 

4. The Zlib is used by MET for compression when writing postscript image files from tools (e.g. MODE, Wavelet-Stat, Plot-Data-Plane, and Plot-Point-Obs). Zlib is distributed, supported and is freely available for download from the `Zlib website <http://www.zlib.net>`_. 

Two additional utilities are strongly recommended for use with MET:

1. The Unified Post-Processor is recommended for post-processing the raw WRF model output prior to verifying the model forecasts with MET. The Unified Post-Processor is freely available for `download <https://epic.noaa.gov/unified-post-processor/>`_. MET can read data on a standard, de-staggered grid and on pressure or regular levels in the vertical. The Unified Post-Processor outputs model data in this format from both WRF cores, the NMM and the ARW. However, the Unified Post-Processor is not strictly required as long as the user can produce gridded model output on a standard de-staggered grid on pressure or regular levels in the vertical. Two-dimensional fields (e.g., precipitation amount) are also accepted for some modules.

2. The copygb utility is recommended for re-gridding model and observation datasets in GRIB version 1 format to a common verification grid. The copygb utility is distributed as part of the Unified Post-Processor and is available from other sources as well. While earlier versions of MET required that all gridded data be placed on a common grid, MET version 5.1 added support for automated re-gridding on the fly. After version 5.1, users have the option of running copygb to regrid their GRIB1 data ahead of time or leveraging the automated regridding capability within MET. 

.. _Installation-of-required:

Installation of Required Libraries
==================================

As described in :numref:`Install_Required-libraries-and`, some external libraries are required for building the MET:

1.
NCEP's BUFRLIB is used by the MET to decode point-based observation datasets in PrepBUFR format. Once you have downloaded and unpacked the BUFRLIB tarball, refer to the README_BUFRLIB file. When compiling the library using the GNU C and Fortran compilers, users are strongly encouraged to use the -DUNDERSCORE and -fno-second-underscore options. Compiling the BUFRLIB version 11.3.0 (recommended version) using the GNU compilers consists of the following three steps:

.. code-block:: none
		
  gcc -c -DUNDERSCORE `./getdefflags_C.sh` *.c >> make.log
  gfortran -c -fno-second-underscore -fallow-argument-mismatch `./getdefflags_F.sh` modv*.F moda*.F \
  `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log
  ar crv libbufr.a *.o

Compiling the BUFRLIB using the PGI C and Fortran-90 compilers consists of the following three steps:

.. code-block:: none

  pgcc -c -DUNDERSCORE `./getdefflags_C.sh` *.c >> make.log
  pgf90 -c -Mnosecond_underscore `./getdefflags_F.sh` modv*.F moda*.F \
  `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log
  ar crv libbufr.a *.o

Compiling the BUFRLIB using the Intel icc and ifort compilers consists of the following three steps:

.. code-block:: none
		
  icc -c -DUNDERSCORE `./getdefflags_C.sh` *.c >> make.log
  ifort -c `./getdefflags_F.sh` modv*.F moda*.F \
  `ls -1 *.F *.f | grep -v "mod[av]_"` >> make.log
  ar crv libbufr.a *.o

In the directions above, the static library file that is created will be named libbufr.a. MET will check for the library file named libbufr.a, however in some cases (e.g. where the BUFRLIB is already available on a system) the library file may be named differently (e.g. libbufr_v11.3.0_4_64.a). If the library is named anything other than libbufr.a, users will need to tell MET what library to link with by passing the BUFRLIB_NAME option to MET when running configure (e.g. BUFRLIB_NAME=-lbufr_v11.3.0_4_64).

2. Unidata's NetCDF libraries are used by several tools within MET for writing output NetCDF files. Both `NetCDF-C and NetCDF-CXX <https://www.unidata.ucar.edu/downloads/netcdf/>`_ are required. The same family of compilers used to build NetCDF should be used when building MET. Users may also find some utilities built for NetCDF such as ncdump and ncview useful for viewing the contents of NetCDF files. Support for NetCDF version 4 requires `HDF5 <https://portal.hdfgroup.org/display/HDF5/HDF5>`_.

3. The GNU Scientific Library (GSL) is used by MET for random sampling and normal and binomial distribution computations when estimating confidence intervals. Precompiled binary packages are available for most GNU/Linux distributions and may be installed with root access. When installing GSL from a precompiled package on Debian Linux, the developer's version of GSL must be used; otherwise, use the GSL version available from the `GNU GSL website <http://www.gnu.org/software/gsl/>`_. MET requires access to the GSL source headers and library archive file at build time. 

4. For users wishing to compile MET with GRIB2 file support, `NCEP's GRIB2 Library <http://www.nco.ncep.noaa.gov/pmb/codes/GRIB2>`_ in C (g2clib) must be installed, along with jasperlib, libpng, and zlib. **Please note that compiling the GRIB2C library with the -D__64BIT__ option requires that MET also be configured with CFLAGS="-D__64BIT__". Compiling MET and the GRIB2C library inconsistently may result in a segmentation fault or an "out of memory" error when reading GRIB2 files.** MET looks for the GRIB2C library to be named libgrib2c.a, which may be set in the GRIB2C makefile as LIB=libgrib2c.a. However in some cases, the library file may be named differently (e.g. libg2c_v1.6.0.a). If the library is named anything other than libgrib2c.a, users will need to tell MET what library to link with by passing the GRIB2CLIB_NAME option to MET when running configure (e.g. GRIB2CLIB_NAME=-lg2c_v1.6.0).

5. Users wishing to compile MODIS-regrid and/or lidar2nc will need to install both the `HDF4 <https://portal.hdfgroup.org/display/HDF4/HDF4>`_ and `HDF-EOS2 <http://hdfeos.org/>`_ libraries available from the HDF group websites linked here.

6. The MODE-Graphics utility requires `Cairo <http://cairographics.org/releases>`_ and `FreeType <http://www.freetype.org/download.html>`_. Thus, users who wish to compile this utility must install both libraries. In addition, users will need to download the `Ghostscript font data <http://sourceforge.net/projects/gs-fonts>`_ required at runtime.

.. _Installation-of-optional:

Installation of Optional Utilities
==================================

As described in the introduction to this section, two additional utilities are strongly recommended for use with MET.

1. The `Unified Post-Processor <https://epic.noaa.gov/unified-post-processor/>`_ is recommended for post-processing the raw WRF model output prior to verifying the data with MET. The Unified Post-Processor may be used on WRF output from both the ARW and NMM cores.

2. The copygb utility is recommended for re-gridding model and observation datasets in GRIB format to a common verification grid. The copygb utility is distributed as part of the Unified Post-Processor and is available from other sources as well. Please refer to the "Unified Post-processor" utility mentioned above for information on availability and installation.

.. _met_directory_structure:

MET Directory Structure
=======================

The top-level MET directory consists of  Makefiles, configuration files, and several subdirectories. The top-level Makefile and configuration files control how the entire toolkit is built. Instructions for using these files to build MET can be found in :numref:`Install_Building-the-MET`.

When MET has been successfully built and installed, the installation directory contains two subdirectories. The *bin/* directory contains executables for each module of MET as well as several plotting utilities. The *share/met/* directory contains many subdirectories with data required at runtime and a subdirectory of sample R scripts utilities. The *colortables/*, *map/*, and *ps/* subdirectories contain data used in creating PostScript plots for several MET tools. The *poly/* subdirectory contains predefined lat/lon polyline regions for use in selecting regions over which to verify. The polylines defined correspond to verification regions used by NCEP as described in :numref:`Appendix B, Section %s <appendixB>`. The *config/* directory contains default configuration files for the MET tools. The *python/* subdirectory contains python scripts. The *python/examples* subdirectory contains sample scripts used in Python embedding (:numref:`Appendix F, Section %s <appendixF>`). The *python/pyembed/* subdirectory contains code used in Python embedding (:numref:`Appendix F, Section %s <appendixF>`). The *table_files/* and *tc_data/* subdirectories contain GRIB table definitions and tropical cyclone data, respectively. The *Rscripts/* subdirectory contains a handful of plotting graphic utilities for MET-TC. These are the same Rscripts that reside under the top-level MET *scripts/Rscripts* directory, other than it is the installed location.

The *data/* directory contains several configuration and static data files used by MET. The *sample_fcst/* and *sample_obs/* subdirectories contain sample data used by the test scripts provided in the *scripts/* directory. 

The *docs/* directory contains the Sphinx documentation for MET.

The *out/* directory will be populated with sample output from the test cases described in the next section. 

The *src/* directory contains the source code for each of the tools in MET. 

The *scripts/* directory contains test scripts that are run by make test after MET has been successfully built, and a directory of sample configuration files used in those tests located in the *scripts/config/* subdirectory. The output from the test scripts in this directory will be written to the *out/* directory. Users are encouraged to copy sample configuration files to another location and modify them for their own use.

The *share/met/Rscripts* directory contains a handful of sample R scripts, including plot_tcmpr.R, which provides graphic utilities for MET-TC. For more information on the graphics capabilities, see :numref:`TC-Stat-tool-example` of this User's Guide.

.. _Install_Building-the-MET:

Building the MET Package
========================

Building the MET package consists of three main steps: (1) install the required libraries, (2) configure the environment variables, and (3) configure and execute the build. Users can follow the instructions below or use a sample installation script.  Users can find the script and its instructions under on the `Downloads <https://dtcenter.org/community-code/model-evaluation-tools-met/download>`_ page of the MET website.

Get the MET source code
-----------------------

The MET source code is available for download from the public `MET GitHub repository <https://github.com/dtcenter/MET>`_.

- Open a web browser and go to the `latest stable MET release <https://github.com/dtcenter/MET/releases/latest>`_.

- Click on the `Source code` link (either the *zip* or *tar.gz*) under Assets and when prompted, save it to your machine.

- (Optional) Verify the checksum of the source code download

    - Download the checksum file that corresponds to the source code download link that was used (checksum_zip.txt for the *zip* file and checksum_tar.txt for the *tar.gz* file). Put the checksum file into the same directory as the source code file.
    - Run the *sha256sum* command with the --check argument to verify that the source code download file was not corrupted.

Zip File::

    sha256sum --check checksum_zip.txt

Tar File::

    sha256sum --check checksum_tar.txt

.. note::
   If the source code is downloaded using **wget**, then the filenames will not
   match the filenames listed in the checksum files. If the source code is
   downloaded using **curl**, the *-LJO* flags should be added to the command to
   preserve the expected filenames found in the checksum files.
   
- Uncompress the source code (on Linux/Unix\ *: gunzip* for zip file or *tar xvfz* for the tar.gz file)

Install the Required Libraries
------------------------------

• Please refer to :numref:`Installation-of-required` and :numref:`Installation-of-optional` on how to install the required and optional libraries.

• If installing the required and optional libraries in a non-standard location, the user may need to tell MET where to find them. This can be done by setting or adding to the LD_LIBRARY PATH to include the path to the library files.

Set Environment Variables
-------------------------

The MET build uses environment variables to specify the locations of the needed external libraries. For each library, there is a set of three environment variables to describe the locations: $MET_<lib>, $MET_<lib>INC and $MET_<lib>LIB.

The $MET_<lib> environment variable can be used if the external library is installed such that there is a main directory which has a subdirectory called "lib" containing the library files and another subdirectory called "include" containing the include files. For example, if the NetCDF library files are installed in */opt/netcdf/lib* and the include files are in */opt/netcdf/include*, you can just define the $MET_NETCDF environment variable to be "*/opt/netcdf*".

The $MET_<lib>INC and $MET_<lib>LIB environment variables are used if the library and include files for an external library are installed in separate locations. In this case, both environment variables must be specified and the associated $MET_<lib> variable will be ignored. For example, if the NetCDF include files are installed in */opt/include/netcdf* and the library files are in */opt/lib/netcdf*, then you would set $MET_NETCDFINC to "*/opt/include/netcdf*" and $MET_NETCDFLIB to "*/opt/lib/netcdf*".

The following environment variables should also be set:

* Set $MET_NETCDF to point to the main NetCDF directory, or set $MET_NETCDFINC to point to the directory with the NetCDF include files and set $MET_NETCDFLIB to point to the directory with the NetCDF library files. Note that the files for both NetCDF-C and NetCDF-CXX must be installed in the same include and library directories.

* Set $MET_HDF5 to point to the main HDF5 directory.

* Set $MET_BUFR to point to the main BUFR directory, or set $MET_BUFRLIB to point to the directory with the BUFR library files. Because we don't use any BUFR library include files, you don't need to specify $MET_BUFRINC.

* Set $MET_GSL to point to the main GSL directory, or set $MET_GSLINC to point to the directory with the GSL include files and set $MET_GSLLIB to point to the directory with the GSL library files.

* If compiling support for GRIB2, set $MET_GRIB2CINC and $MET_GRIB2CLIB to point to the main GRIB2C directory which contains both the include and library files. These are used instead of $MET_GRIB2C since the main GRIB2C directory does not contain include and lib subdirectories.

* If compiling support for PYTHON, set $MET_PYTHON_BIN_EXE to specify the desired python executable to be used. Also set $MET_PYTHON_CC, and $MET_PYTHON_LD to specify the compiler (-I) and linker (-L) flags required for python. Set $MET_PYTHON_CC for the directory containing the "Python.h" header file. Set $MET_PYTHON_LD for the directory containing the python library file and indicate the name of that file. For example:

  .. code-block:: none

    MET_PYTHON_BIN_EXE='/usr/bin/python3.6'
    MET_PYTHON_CC='-I/usr/include/python3.6'
    MET_PYTHON_LD='-L/usr/lib/python3.6/config-x86_64-linux-gnu -lpython3.6m'

  Note that this version of Python must include support for a minimum set of required packages. For more information about Python support in MET, including the list of required packages, please refer to :numref:`Appendix F, Section %s <appendixF>`.


* If compiling MODIS-Regrid and/or lidar2nc, set $MET_HDF to point to the main HDF4 directory, or set $MET_HDFINC to point to the directory with the HDF4 include files and set $MET_HDFLIB to point to the directory with the HDF4 library files. Also, set $MET_HDFEOS to point to the main HDF EOS directory, or set $MET_HDFEOSINC to point to the directory with the HDF EOS include files and set $MET_HDFEOSLIB to point to the directory with the HDF EOS library files.

* If compiling MODE Graphics, set $MET_CAIRO to point to the main Cairo directory, or set$MET_CAIROINC to point to the directory with the Cairo include files and set $MET_CAIROLIB to point to the directory with the Cairo library files. Also, set $MET_FREETYPE to point to the main FreeType directory, or set $MET_FREETYPEINC to point to the directory with the FreeType include files and set $MET_FREETYPELIB to point to the directory with the FreeType library files.

*  When running MODE Graphics, set $MET_FONT_DIR to the directory containing font data required at runtime. A link to the tarball containing this font data can be found on the MET website.

For ease of use, you should define these in your .cshrc or equivalent file.

Configure and Execute the Build
-------------------------------

Example: To configure MET to install all of the available tools in the "bin" subdirectory of your current directory, you would use the following commands:

.. code-block:: none

  1. ./configure --prefix=`pwd` --enable-grib2 --enable-python \
                 --enable-modis --enable-mode_graphics --enable-lidar2nc
  2. Type 'make install >& make_install.log &'
  3. Type 'tail -f make_install.log' to view the execution of the make.
  4. When make is finished, type 'CTRL-C' to quit the tail.

If all tools are enabled and the build is successful, the "*<prefix>/bin*" directory (where *<prefix>* is the prefix you specified on your configure command line) will contain the following executables:

.. code-block:: none

   - ascii2nc
   - ensemble_stat
   - gen_ens_prod
   - gen_vx_mask
   - grid_stat
   - gis_dump_dbf
   - gis_dump_shp
   - gis_dump_shx
   - grid_diag
   - gsid2mpr
   - gsidens2orank
   - lidar2nc
   - madis2nc
   - mode
   - mode_analysis
   - modis_regrid
   - mtd
   - pb2nc
   - pcp_combine
   - plot_data_plane
   - plot_mode_field
   - plot_point_obs
   - point2grid
   - point_stat
   - rmw_analysis
   - regrid_data_plane
   - series_analysis
   - shift_data_plane
   - stat_analysis
   - tc_dland
   - tc_gen
   - tc_pairs
   - tc_rmw
   - tc_stat
   - wavelet_stat
   - wwmca_plot
   - wwmca_regrid

NOTE: Several compilation warnings may occur which are expected. If any errors occur, please refer to :numref:`Appendix A, Section %s <Troubleshooting>` on troubleshooting for common problems. 

**-help** and **-version** command line options are available for all of the MET tools. Typing the name of the tool with no command line options also produces the usage statement.

The configure script has command line options to specify where to install MET and which MET utilities to install. Include any of the following options that apply to your system:

.. code-block:: none
		
  --prefix=PREFIX

By default, MET will install all the files in "*/usr/local/bin*". You can specify an installation prefix other than "*/usr/local*" using "--prefix", for instance "--prefix=$HOME" or "--prefix=`pwd`".

.. code-block:: none

  --enable-grib2

Enable compilation of utilities using GRIB2. Requires $MET_GRIB2C.

.. code-block:: none

  --enable-python

Enable compilation of python interface. Requires $MET_PYTHON_CC and $MET_PYTHON_LD.

.. code-block:: none

  --enable-lidar2nc
  
Enable compilation of utilities using the LIDAR2NC tool.

.. code-block:: none

  --enable-modis

Enable compilation of the Modis-Regrid tool. Requires $MET_HDF, $MET_HDFEOSINC, and $MET_HDFEOSLIB.

.. code-block:: none
		
  --enable-mode_graphics

Enable compilation of the MODE-Graphics tool. Requires $MET_CAIRO and $MET_FREETYPE.

.. code-block:: none

  --disable-block4

Disable use of BLOCK4 in the compilation. Use this if you have trouble using PrepBUFR files.

.. code-block:: none

  --disable-openmp

Disable compilation of OpenMP directives within the code which allows some code
regions to benefit from thread-parallel execution. Runtime environment variable
:code:`OMP_NUM_THREADS` controls the number of threads.

Run the configure script with the **-help** argument to see the full list of configuration options.

Make Targets
------------

The autoconf utility provides some standard make targets for the users. In MET, the following standard targets have been implemented and tested:

1. **all** - compile all of the components in the package, but don't install them.

2. **install** - install the components (where is described below). Will also compile if "make all" hasn't been done yet.

3. **clean** - remove all of the temporary files created during the compilation.

4. **uninstall** - remove the installed files. For us, these are the executables and the files in $MET_BASE.

MET also has the following non-standard targets:

5. **test** - runs the *scripts/test_all.sh* script. You must run "make install" before using this target.

.. _sample-test-cases:
   
Sample Test Cases
=================

Once the MET package has been built successfully, the user is encouraged to run the sample test scripts provided. They are run using make test in the top-level directory. Execute the following commands:

1. Type 'make test >& make_test.log &' to run all of the test scripts in the directory. These test scripts use test data supplied with the tarball. For instructions on running your own data, please refer to the MET User's Guide.

2. Type 'tail -f make_test.log' to view the execution of the test script.

3. When the test script is finished, type 'CTRL-C' to quit the tail. Look in "out" to find the output files for these tests. Each tool has a separate, appropriately named subdirectory for its output files. 

4. In particular, check that the PB2NC tool ran without error. If there was an error, run "make clean" then rerun your configure command adding **--disable-block4** to your configure command line and rebuild MET.
