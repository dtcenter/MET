

###############################################################################

   ##
   ##  Begin Variables to be modified before building
   ##

###############################################################################

# Path to GNU Make command
MAKE         = /usr/bin/make

# Architecture flags
ARCH_FLAGS   = # -DBLOCK4

# Path to the C++ Compiler
# C++ compiler flags
# Any additional required libraries
CXX          = /usr/bin/g++
CXX_FLAGS    = -Wall -Wshadow -static # -g -m32
CXX_LIBS     =

# Path to the Fortran Compiler
# Fortran compiler flags
# Any additional required libraries
FC           = /usr/bin/gfortran
FC_FLAGS     = -Wall -Wshadow -static -ff2c # -g -m32
FC_LIBS      = -lgfortran

# Make print options
#PRINT_OPTS   = --no-print-directory
PRINT_OPTS   =

# Top level directory for the NetCDF library
# NetCDF include directory specified as: -I/your/include/path
# NetCDF library directory specified as: -L/your/library/path
NETCDF_BASE  = /usr/local/netcdf-3.6.3/gcc-4.3.1
NETCDF_INCS  = -I$(NETCDF_BASE)/include
NETCDF_LIBS  = -L$(NETCDF_BASE)/lib

# Top level directory for BUFRLIB
# BUFRLIB include directory specified as: -I/your/include/path
# BUFRLIB library directory specified as: -L/your/library/path
BUFR_BASE    = ../bufrlib/bufrlib
BUFR_INCS    = -I$(BUFR_BASE)
BUFR_LIBS    = -L$(BUFR_BASE)

# Top level directory for the GNU Scientific Library (GSL) if it's not
# installed in a standard location.
# GSL include directory specified as: -I/your/include/path
# GSL library directory specified as: -L/your/library/path
GSL_BASE     = /d1/bullock/kd2/bullock/otherlibs/gsl/gsl-1.12
GSL_INCS     = -I$(GSL_BASE)/include
GSL_LIBS     = -L$(GSL_BASE)/lib

# Top level directory for the F2C or G2C Library if it's not installed in a
# standard location.
# F2C include directory specified as: -I/your/include/path
# F2C library directory containing libf2c.a or libg2c.a and specified as:
# -L/your/library/path
# Name of the library to be used: -lf2c or -lg2c
# NOTE: Only required for the GNU g77 Fortran compiler
F2C_BASE     =
F2C_INCS     =
F2C_LIBS     =
F2C_LIBNAME  =

# Optional flags to disable the compilation of MET tools
# Specify a non-zero value to disable the compilation of the tool
ENABLE_PCP_COMBINE    = 0
ENABLE_GEN_POLY_MASK  = 0
ENABLE_MODE           = 0
ENABLE_GRID_STAT      = 0
ENABLE_PB2NC          = 0
ENABLE_ASCII2NC       = 0
ENABLE_POINT_STAT     = 0
ENABLE_WAVELET_STAT   = 0
ENABLE_ENSEMBLE_STAT  = 0
ENABLE_STAT_ANALYSIS  = 0
ENABLE_MODE_ANALYSIS  = 0
ENABLE_PLOT_POINT_OBS = 0
ENABLE_WWMCA          = 1

###############################################################################

   ##
   ##  End Variables to be modified before building
   ##

###############################################################################

