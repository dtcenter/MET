.. _installation:

Software Installation/Getting Started
=====================================

Introduction
____________

This section describes how to install the MET package. MET has been developed and tested on Linux operating systems. Support for additional platforms and compilers may be added in future releases. The MET package requires many external libraries to be available on the user's computer prior to installation. Required and recommended libraries, how to install MET, the MET directory structure, and sample cases are described in the following sections.

Supported architectures
_______________________

The MET package was developed on Debian Linux using the GNU compilers and the Portland Group (PGI) compilers. The MET package has also been built on several other Linux distributions using the GNU, PGI, and Intel compilers. Past versions of MET have also been ported to IBM machines using the IBM compilers, but we are currently unable to support this option as the development team lacks access to an IBM machine for testing. Other machines may be added to this list in future releases as they are tested. In particular, the goal is to support those architectures supported by the WRF model itself.

The MET tools run on a single processor. Therefore, none of the utilities necessary for running WRF on multiple processors are necessary for running MET. Individual calls to the MET tools have relatively low computing and memory requirements. However users will likely be making many calls to the tools and passing those individual calls to several processors will accomplish the verification task more efficiently.

Programming languages
_____________________

The MET package, including MET-TC, is written primarily in C/C++ in order to be compatible with an extensive verification code base in C/C++ already in existence. In addition, the object-based MODE and MODE-TD verification tools rely heavily on the object-oriented aspects of C++. Knowledge of C/C++ is not necessary to use the MET package. The MET package has been designed to be highly configurable through the use of ASCII configuration files, enabling a great deal of flexibility without the need for source code modifications.

NCEP's BUFRLIB is written entirely in Fortran. The portion of MET that handles the interface to the BUFRLIB for reading PrepBUFR point observation files is also written in Fortran.

The MET package is intended to be a tool for the modeling community to use and adapt. As users make upgrades and improvements to the tools, they are encouraged to offer those upgrades to the broader community by offering feedback to the developers.

Required compilers and scripting languages
__________________________________________

The MET package was developed and tested using the GNU g++/gfortran compilers and the Intel icc/ifort compilers. As additional compilers are successfully tested, they will be added to the list of supported platforms/compilers.

The GNU make utility is used in building all executables and is therefore required.

The MET package consists of a group of command line utilities that are compiled separately. The user may choose to run any subset of these utilities to employ the type of verification methods desired. New tools developed and added to the toolkit will be included as command line utilities.

In order to control the desired flow through MET, users are encouraged to run the tools via a script or consider using the `METplus package <https://dtcenter.org/community-code/metplus>`_. Some sample scripts are provided in the distribution; these examples are written in the Bourne shell. However, users are free to adapt these sample scripts to any scripting language desired.

.. _Install_Required-libraries-and:

Required libraries and optional utilities
_________________________________________

Three external libraries are required for compiling/building MET and should be downloaded and installed before attempting to install MET. Additional external libraries required for building advanced features in MET are discussed in :numref:`Installation-of-required` :

1. NCEP's BUFRLIB is used by MET to decode point-based observation datasets in PrepBUFR format. BUFRLIB is distributed and supported by NCEP and is freely available for download from `NCEP's BUFRLIB website <https://emc.ncep.noaa.gov/emc/pages/infrastructure/bufrlib.php>`_. BUFRLIB requires C and Fortran-90 compilers that should be from the same family of compilers used when building MET.

2. Several tools within MET use Unidata's NetCDF libraries for writing output NetCDF files. NetCDF libraries are distributed and supported by Unidata and are freely available for download from `Unidata's NetCDF website <http://www.unidata.ucar.edu/software/netcdf>`_. The same family of compilers used to build NetCDF should be used when building MET. MET is now compatible with the enhanced data model provided in NetCDF version 4. The support for NetCDF version 4 requires NetCDF-C, NetCDF-CXX, and HDF5, which is freely available for download on the `HDF5 webpage <https://support.hdfgroup.org/HDF5/>`_.

3. The GNU Scientific Library (GSL) is used by MET when computing confidence intervals. GSL is distributed and supported by the GNU Software Foundation and is freely available for download from the `GNU website <http://www.gnu.org/software/gsl>`_. 

4. The Zlib is used by MET for compression when writing postscript image files from tools (e.g. MODE, Wavelet-Stat, Plot-Data-Plane, and Plot-Point-Obs). Zlib is distributed, supported and is freely available for download from the `Zlib website <http://www.zlib.net>`_. 

Two additional utilities are strongly recommended for use with MET:

1. The Unified Post-Processor is recommended for post-processing the raw WRF model output prior to verifying the model forecasts with MET. The Unified Post-Processor is freely available for `download <https://dtcenter.org/community-code/unified-post-processor-upp>`_. MET can read data on a standard, de-staggered grid and on pressure or regular levels in the vertical. The Unified Post-Processor outputs model data in this format from both WRF cores, the NMM and the ARW. However, the Unified Post-Processor is not strictly required as long as the user can produce gridded model output on a standard de-staggered grid on pressure or regular levels in the vertical. Two-dimensional fields (e.g., precipitation amount) are also accepted for some modules.

2. The copygb utility is recommended for re-gridding model and observation datasets in GRIB version 1 format to a common verification grid. The copygb utility is distributed as part of the Unified Post-Processor and is available from other sources as well. While earlier versions of MET required that all gridded data be placed on a common grid, MET version 5.1 added support for automated re-gridding on the fly. After version 5.1, users have the option of running copygb to regrid their GRIB1 data ahead of time or leveraging the automated regridding capability within MET. 

.. _Installation-of-required:

Installation of required libraries
__________________________________

As described in :numref:`Install_Required-libraries-and`, some external libraries are required for building the MET:

1.
NCEP's BUFRLIB is used by the MET to decode point-based observation datasets in PrepBUFR format. Once you have downloaded and unpacked the BUFRLIB tarball, refer to the README_BUFRLIB file. When compiling the library using the GNU C and Fortran compilers, users are strongly encouraged to use the -DUNDERSCORE and -fno-second-underscore options. Compiling the BUFRLIB version 11.3.0 (recommended version) using the GNU compilers consists of the following three steps:

.. code-block:: none
		
  gcc -c -DUNDERSCORE `./getdefflags_C.sh` *.c >> make.log
  gfortran -c -fno-second-underscore `./getdefflags_F.sh` modv*.F moda*.F \
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

Installation of optional utilities
__________________________________

As described in the introduction to this section, two additional utilities are strongly recommended for use with MET.

1. The `Unified Post-Processor <https://dtcenter.org/community-code/unified-post-processor-upp>`_ is recommended for post-processing the raw WRF model output prior to verifying the data with MET. The Unified Post-Processor may be used on WRF output from both the ARW and NMM cores.

2. The copygb utility is recommended for re-gridding model and observation datasets in GRIB format to a common verification grid. The copygb utility is distributed as part of the Unified Post-Processor and is available from other sources as well. Please refer to the "Unified Post-processor" utility mentioned above for information on availability and installation.

.. _met_directory_structure:

MET directory structure
_______________________

The top-level MET directory consists of a README file, Makefiles, configuration files, and several subdirectories. The top-level Makefile and configuration files control how the entire toolkit is built. Instructions for using these files to build MET can be found in :numref:`Install_Building-the-MET`.

When MET has been successfully built and installed, the installation directory contains two subdirectories. The bin/ directory contains executables for each module of MET as well as several plotting utilities. The share/met/ directory contains many subdirectories with data required at runtime and a subdirectory of sample R scripts utilities. The colortables/, map/, and ps/ subdirectories contain data used in creating PostScript plots for several MET tools. The poly/ subdirectory contains predefined lat/lon polyline regions for use in selecting regions over which to verify. The polylines defined correspond to verification regions used by NCEP as described in :numref:`Appendix B, Section %s <appendixB>`. The config/ directory contains default configuration files for the MET tools. The python/ subdirectory contains sample scripts used in Python embedding (:numref:`Appendix F, Section %s <appendixF>`). The table_files/ and tc_data/ subdirectories contain GRIB table definitions and tropical cyclone data, respectively. The Rscripts/ subdirectory contains a handful of plotting graphic utilities for MET-TC. These are the same Rscripts that reside under the top-level MET scripts/Rscripts directory, other than it is the installed location. The wrappers/ subdirectory contains code used in Python embedding (:numref:`Appendix F, Section %s <appendixF>`).

The data/ directory contains several configuration and static data files used by MET. The sample_fcst/ and sample_obs/ subdirectories contain sample data used by the test scripts provided in the scripts/ directory. 

The docs/ directory contains the Sphinx documentation for MET.

The out/ directory will be populated with sample output from the test cases described in the next section. 

The src/ directory contains the source code for each of the tools in MET. 

The scripts/ directory contains test scripts that are run by make test after MET has been successfully built, and a directory of sample configuration files used in those tests located in the scripts/config/ subdirectory. The output from the test scripts in this directory will be written to the out/ directory. Users are encouraged to copy sample configuration files to another location and modify them for their own use.

The share/met/Rscripts directory contains a handful of sample R scripts, including plot_tcmpr.R, which provides graphic utilities for MET-TC. For more information on the graphics capabilities, see :numref:`TC-Stat-tool-example` of this User's Guide.

.. _Install_Building-the-MET:

Building the MET package
________________________

Building the MET package consists of three main steps: (1) install the required libraries, (2) configure the environment variables, and (3) configure and execute the build. Users can follow the instructions below or use a sample installation script.  Users can find the script and its instructions under on the `Downloads <https://dtcenter.org/community-code/model-evaluation-tools-met/download>`_ page of the MET website.

Install the Required Libraries
______________________________

• Please refer to :numref:`Installation-of-required` and :numref:`Installation-of-optional` on how to install the required and optional libraries.

• If installing the required and optional libraries in a non-standard location, the user may need to tell MET where to find them. This can be done by setting or adding to the LD_LIBRARY PATH to include the path to the library files.

Set Environment Variables
~~~~~~~~~~~~~~~~~~~~~~~~~

The MET build uses environment variables to specify the locations of the needed external libraries. For each library, there is a set of three environment variables to describe the locations: $MET_<lib>, $MET_<lib>INC and $MET_<lib>LIB.

The $MET_<lib> environment variable can be used if the external library is installed such that there is a main directory which has a subdirectory called "lib" containing the library files and another subdirectory called "include" containing the include files. For example, if the NetCDF library files are installed in /opt/netcdf/lib and the include files are in /opt/netcdf/include, you can just define the $MET_NETCDF environment variable to be "/opt/netcdf".

The $MET_<lib>INC and $MET_<lib>LIB environment variables are used if the library and include files for an external library are installed in separate locations. In this case, both environment variables must be specified and the associated $MET_<lib> variable will be ignored. For example, if the NetCDF include files are installed in /opt/include/netcdf and the library files are in /opt/lib/netcdf, then you would set $MET_NETCDFINC to "/opt/include/netcdf" and $MET_NETCDFLIB to "/opt/lib/netcdf".

The following environment variables should also be set:

   \- Set $MET_NETCDF to point to the main NetCDF directory, or set $MET_NETCDFINC to point to the directory with the NetCDF include files and set $MET_NETCDFLIB to point to the directory with the NetCDF library files. Note that the files for both NetCDF-C and NetCDF-CXX must be installed in the same include and library directories.

   \- Set $MET_HDF5 to point to the main HDF5 directory.

   \- Set $MET_BUFR to point to the main BUFR directory, or set $MET_BUFRLIB to point to the directory with the BUFR library files. Because we don't use any BUFR library include files, you don't need to specify $MET_BUFRINC.

   \- Set $MET_GSL to point to the main GSL directory, or set $MET_GSLINC to point to the directory with the GSL include files and set $MET_GSLLIB to point to the directory with the GSL library files.

   \- If compiling support for GRIB2, set $MET_GRIB2CINC and $MET_GRIB2CLIB to point to the main GRIB2C directory which contains both the include and library files. These are used instead of $MET_GRIB2C since the main GRIB2C directory does not contain include and lib subdirectories.

   \- If compiling support for PYTHON, set $MET_PYTHON_CC and $MET_PYTHON_LD to specify the compiler (-I) and linker (-L) flags required for python. Set $MET_PYTHON_CC for the directory containing the "Python.h" header file. Set $MET_PYTHON_LD for the directory containing the python library file and indicate the name of that file. For example:

MET_PYTHON_CC='-I/usr/include/python3.6'

MET_PYTHON_LD='-L/usr/lib/python3.6/config-x86_64-linux-gnu -lpython3.6m'

For more information about Python support in MET, please refer to :numref:`Appendix F, Section %s <appendixF>`.

   \- If compiling MODIS-Regrid and/or lidar2nc, set $MET_HDF to point to the main HDF4 directory, or set $MET_HDFINC to point to the directory with the HDF4 include files and set $MET_HDFLIB to point to the directory with the HDF4 library files. Also, set $MET_HDFEOS to point to the main HDF EOS directory, or set $MET_HDFEOSINC to point to the directory with the HDF EOS include files and set $MET_HDFEOSLIB to point to the directory with the HDF EOS library files.

   \- If compiling MODE Graphics, set $MET_CAIRO to point to the main Cairo directory, or set$MET_CAIROINC to point to the directory with the Cairo include files and set $MET_CAIROLIB to point to the directory with the Cairo library files. Also, set $MET_FREETYPE to point to the main FreeType directory, or set $MET_FREETYPEINC to point to the directory with the FreeType include files and set $MET_FREETYPELIB to point to the directory with the FreeType library files.

   \- When running MODE Graphics, set $MET_FONT_DIR to the directory containing font data required at runtime. A link to the tarball containing this font data can be found on the MET website.

For ease of use, you should define these in your .cshrc or equivalent file.

Configure and execute the build
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example: To configure MET to install all of the available tools in the "bin" subdirectory of your current directory, you would use the following commands:

.. code-block:: none

  1. ./configure --prefix=`pwd` --enable-grib2 --enable-python \
                 --enable-modis --enable-mode_graphics --enable-lidar2nc
  2. Type 'make install >& make_install.log &'
  3. Type 'tail -f make_install.log' to view the execution of the make.
  4. When make is finished, type 'CTRL-C' to quit the tail.

If all tools are enabled and the build is successful, the "<prefix>/bin" directory (where <prefix> is the prefix you specified on your configure command line) will contain 36 executables:

.. code-block:: none

   - ascii2nc
   - ensemble_stat
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

**-\\-prefix=PREFIX**

By default, MET will install all the files in "/usr/local/bin". You can specify an installation prefix other than "/usr/local" using "--prefix", for instance "--prefix=$HOME" or "--prefix=`pwd`".

**-\\-enable-grib2**

Enable compilation of utilities using GRIB2. Requires $MET_GRIB2C.

**-\\-enable-python**

Enable compilation of python interface. Requires $MET_PYTHON_CC and $MET_PYTHON_LD.

**-\\-enable-lidar2nc**

Enable compilation of utilities using lidar2nc.

**-\\-enable-modis**

Enable compilation of modis_regrid. Requires $MET_HDF, $MET_HDFEOSINC, and $MET_HDFEOSLIB.

**-\\-enable-mode_graphics**

Enable compilation of mode_graphics. Requires $MET_CAIRO and $MET_FREETYPE.

**-\\-disable-block4**

Disable use of BLOCK4 in the compilation. Use this if you have trouble using PrepBUFR files.

Run the configure script with the --help argument to see the full list of configuration options.

Make Targets
~~~~~~~~~~~~

The autoconf utility provides some standard make targets for the users. In MET, the following standard targets have been implemented and tested:

1. **all** - compile all of the components in the package, but don't install them.

2. **install** - install the components (where is described below). Will also compile if "make all" hasn't been done yet.

3. **clean** - remove all of the temporary files created during the compilation.

4. **uninstall** - remove the installed files. For us, these are the executables and the files in $MET_BASE.

MET also has the following non-standard targets:

5. **test** - runs the scripts/test_all.sh script. You must run "make install" before using this target.

.. _Sample Test cases:
   
Sample test cases
_________________

Once the MET package has been built successfully, the user is encouraged to run the sample test scripts provided. They are run using make test in the top-level directory. Execute the following commands:

1. Type 'make test >& make_test.log &' to run all of the test scripts in the directory. These test scripts use test data supplied with the tarball. For instructions on running your own data, please refer to the MET User's Guide.

2. Type 'tail -f make_test.log' to view the execution of the test script.

3. When the test script is finished, type 'CTRL-C' to quit the tail. Look in "out" to find the output files for these tests. Each tool has a separate, appropriately named subdirectory for its output files. 

4. In particular, check that the PB2NC tool ran without error. If there was an error, run "make clean" then rerun your configure command adding "--disable-block4" to your configure command line and rebuild MET.
